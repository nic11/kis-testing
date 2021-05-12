import re
import time

import mariadb
import requests
import selenium
from selenium.webdriver.remote.webdriver import WebDriver
import pytest

SCREENSHOTS_FOLDER = './screenshots'
BASE_URL = 'http://localhost:8080'
TEST_USER_ID = '1'


@pytest.fixture(scope='session')
def browser():
    return selenium.webdriver.Firefox()


@pytest.fixture(scope='session')
def db():
    conn = mariadb.connect(
        user='root',
        password='admin',
        host='127.0.0.1',
        port=3306,
    )
    return conn


@pytest.fixture(autouse=True)
def screenshoter(browser: WebDriver, request: pytest.FixtureRequest):
    class Screenshoter:
        def save(self):
            if SCREENSHOTS_FOLDER is None:
                return
            assert browser.save_screenshot(f'{SCREENSHOTS_FOLDER}/{time.time_ns()}_{request.node.name}.png')

    instance = Screenshoter()
    request.addfinalizer(lambda: instance.save())
    return instance


def test_main_page(browser: WebDriver):
    browser.get(f'{BASE_URL}')
    time.sleep(2)

    # Test that all needed UI elements are there
    browser.find_element_by_id('get-order-history')
    browser.find_element_by_id('create-order-button')
    browser.find_element_by_id('get-order-input')
    browser.find_element_by_id('get-order-button')


def _create_order(sender, receiver, start_vertex, end_vertex):
    res = requests.post(
        f"{BASE_URL}/create/{sender}?receiver={receiver}",
        json={
            "orderType": "TIME",
            "startVertex": str(start_vertex),
            "endVertex": str(end_vertex),
            "products": [{"weight": 1, "price": 2}],
        },
    )
    res.raise_for_status()
    return res.json()['orderId'][0]


def _change_order_status(cur, order_id, order_status):
    cur.execute(
        '''
        UPDATE dostavim.ORDERS
        SET ORDER_STATUS = ?
        WHERE ID = ?
        ''',
        [order_status, order_id],
    )


def test_create_order(browser: WebDriver, screenshoter, db):
    browser.get(f'{BASE_URL}/create/{TEST_USER_ID}')
    time.sleep(2)

    browser.find_element_by_id('receiver-id').send_keys(TEST_USER_ID)
    browser.find_element_by_id('start-vertex-id').send_keys('1')
    browser.find_element_by_id('end-vertex-id').send_keys('2')

    browser.find_element_by_id('add-product-button').click()

    browser.find_element_by_xpath(f"(//input[@class='weight-input'])[1]").send_keys('1')
    browser.find_element_by_xpath(f"(//input[@class='price-input'])[1]").send_keys('2')
    browser.find_element_by_xpath(f"(//input[@class='weight-input'])[2]").send_keys('3')
    browser.find_element_by_xpath(f"(//input[@class='price-input'])[2]").send_keys('4')
    screenshoter.save()

    browser.find_element_by_id('remove-product-button').click()
    screenshoter.save()

    browser.find_element_by_id('send-order').click()

    m = re.match(r'^.*/order/(\d+)\?clientId=\d+$', browser.current_url)
    assert m
    order_id = int(m.group(1))

    cur = db.cursor()
    cur.execute(
        '''
        SELECT CLIENT_ID, START_VERTEX, END_VERTEX, ORDER_STATUS
        FROM dostavim.ORDERS WHERE ID = ?
        ''',
        (order_id,),
    )
    result = cur.fetchall()[0]

    assert result == (1, 1, 2, 'WAIT_CHANGE')


@pytest.mark.parametrize(
    ['order_status'],
    (
        ['READY'],
        ['COMPLETED'],
    ),
)
def test_order_page(browser: WebDriver, screenshoter, db, order_status):
    start_vertex = '1'
    end_vertex = '2'

    order_id = _create_order(TEST_USER_ID, TEST_USER_ID, start_vertex, end_vertex)
    _change_order_status(db.cursor(), order_id, order_status)
    db.commit()

    browser.get(f'{BASE_URL}/order/{order_id}?clientId={TEST_USER_ID}')
    assert browser.find_element_by_xpath("//div[@class='order_id']/span").text == str(order_id)
    assert browser.find_element_by_xpath("//div[@class='order_start_vertex']/span").text == start_vertex
    assert browser.find_element_by_xpath("//div[@class='order_end_vertex']/span").text == end_vertex
    assert browser.find_element_by_xpath("//div[@class='order_status']/span").text == order_status

    receive_button = browser.find_elements_by_id('receive-order-button')
    if order_status == 'READY':
        assert receive_button
        screenshoter.save()
        receive_button[0].click()
        browser.switch_to.alert.dismiss()
        time.sleep(1)
        assert browser.find_element_by_xpath("//div[@class='order_status']/span").text == 'COMPLETED'
        assert not browser.find_elements_by_id('receive-order-button')
    else:
        assert not receive_button


def _check_order_and_status(table, order_id, status):
    for row in table.find_elements_by_xpath('.//tr'):
        if row.find_elements_by_xpath('.//td')[0].text == str(order_id):
            if status is None:
                assert False, 'this order should not be present in table'
            assert row.find_elements_by_xpath('.//td')[-1].text == status
            return
    if status is not None:
        assert False, 'this order should be present in table'


def test_order_history(browser: WebDriver, db):
    extra_user_id = 2

    order_ids = []
    order_statuses = ['READY', 'COMPLETED', 'COMPLETED']

    for sender, receiver in [
        (TEST_USER_ID, TEST_USER_ID),  # to self
        (TEST_USER_ID, extra_user_id), # to someone else
        (extra_user_id, TEST_USER_ID), # from someone else
    ]:
        order_ids.append(_create_order(sender, receiver, 1, 2))
    
    for order_id, order_status in zip(order_ids, order_statuses):
        _change_order_status(db.cursor(), order_id, order_status)
    db.commit()

    browser.get(f'{BASE_URL}/orders/{TEST_USER_ID}')
    sent_table, received_table = browser.find_elements_by_xpath('//tbody')

    _check_order_and_status(sent_table, order_ids[0], order_statuses[0])
    _check_order_and_status(received_table, order_ids[0], order_statuses[0])

    _check_order_and_status(sent_table, order_ids[1], order_statuses[1])
    _check_order_and_status(received_table, order_ids[1], None)

    _check_order_and_status(sent_table, order_ids[2], None)
    _check_order_and_status(received_table, order_ids[2], order_statuses[2])

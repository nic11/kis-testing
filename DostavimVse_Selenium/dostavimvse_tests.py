import time

import selenium
from selenium.webdriver.remote.webdriver import WebDriver
from selenium.webdriver.common.keys import Keys
import pytest

SCREENSHOTS_FOLDER = './screenshots'
BASE_URL = 'http://localhost:8080'
TEST_USER_ID = '1'


@pytest.fixture(scope='session')
def browser():
    return selenium.webdriver.Firefox()


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


def test_create_order(browser: WebDriver, screenshoter):
    browser.get(f'{BASE_URL}/create/{TEST_USER_ID}')
    time.sleep(2)

    browser.find_element_by_id('receiver-id').send_keys(TEST_USER_ID)
    browser.find_element_by_id('start-vertex-id').send_keys('1')
    browser.find_element_by_id('end-vertex-id').send_keys('2')

    browser.find_element_by_id('add-product-button').click()
    # browser.find_element_by_css_selector('#products-field:nth-child(1) .weight-input').send_keys('1')
    # browser.find_element_by_css_selector('#products-field:nth-child(1) .weight-input').send_keys('1')
    # browser.find_element_by_css_selector('#products-field:nth-child(1) .price-input').send_keys('2')
    # product_selector = "//div[@id='products-field']/div[@class='product']"
    # browser.find_element_by_xpath(f"({product_selector})[1]/input[@class='weight-input']").send_keys('yadaa')

    browser.find_element_by_xpath(f"(//input[@class='weight-input'])[1]").send_keys('1')
    browser.find_element_by_xpath(f"(//input[@class='price-input'])[1]").send_keys('2')
    browser.find_element_by_xpath(f"(//input[@class='weight-input'])[2]").send_keys('3')
    browser.find_element_by_xpath(f"(//input[@class='price-input'])[2]").send_keys('4')
    screenshoter.save()

    browser.find_element_by_id('remove-product-button').click()
    screenshoter.save()

    browser.find_element_by_id('send-order').click()

    # Test that order is in DB
    # TODO

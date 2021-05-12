How to run:
* Prepare server:
  * `git clone https://github.com/akhtyamovpavel/DostavimVse.git && cd DostavimVse`
  * Run DB: `docker run -it --rm -v "$PWD:/docker-entrypoint-initdb.d" -p 127.0.0.1:3306:3306 -e MYSQL_DATABASE=dostavim -e MYSQL_ROOT_PASSWORD=admin mariadb:10.6.0-focal`
  * Run DB preparation: https://github.com/akhtyamovpavel/DostavimVse/blob/master/src/main/java/Main.java
  * Run DostavimVse server: https://github.com/akhtyamovpavel/DostavimVse/blob/master/src/main/java/ru/fivt/dostavimvse/DostavimvseApplication.java
* Prepare and run tests:
  * `apt install libmariadb-dev firefox-geckodriver`
  * `pip install -r requirements.txt`
  * `pytest dostavimvse_tests.py`

#include <Weather.h>
#include <iostream>
int main() {
  Weather weather;
  weather.SetApiKey("Set your api key");

  std::cout << weather.GetTemperature("London") << std::endl;
  std::cout << weather.GetTomorrowTemperature("London") << std::endl;
  std::cout << weather.GetTomorrowDiff("Moscow") << std::endl;
}
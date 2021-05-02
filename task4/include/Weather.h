

#pragma once

#include "json.hpp"
#include "cpr/cpr.h"

using json = nlohmann::json;


class Weather {
 public:
  json GetResponseForCity(const std::string& city, const cpr::Url& url);
  virtual float GetTemperature(const std::string& city);
  virtual float GetTomorrowTemperature(const std::string& city);

  float FindDiffBetweenTwoCities(const std::string& city1, const std::string& city2);

  void SetApiKey(const std::string& api_key);

  std::string GetDifferenceString(const std::string& city1, const std::string& city2);

  std::string GetTomorrowDiff(const std::string& city);
 protected:
  virtual cpr::Response Get(const std::string& city, const cpr::Url& url);
 private:
  const cpr::Url kBaseUrl = cpr::Url{"http://api.openweathermap.org/data/2.5/weather"};
  const cpr::Url kForecastUrl = cpr::Url{"http://api.openweathermap.org/data/2.5/forecast"};
  std::string api_key_;
};




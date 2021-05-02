#pragma once

#include <gmock/gmock.h>
#include <json.hpp>
#include <cpr/cpr.h>

#include <Weather.h>

class WeatherFake : public Weather {
 public:
  WeatherFake(bool forecast_mode, float temp, int status_code)
    : forecast_mode_(forecast_mode)
    , temp_(temp)
    , status_code_(status_code)
  {}

  cpr::Response Get(const std::string& city, const cpr::Url& url) override;

 private:
  bool forecast_mode_;
  float temp_;
  int status_code_;
};

struct PairHash {
  size_t operator()(const std::pair<std::string, std::string>& pair) const {
    return (str_hash(pair.first) >> 32) | (str_hash(pair.second) << 32);
  }

  std::hash<std::string> str_hash{};
};

class WeatherMock : public Weather {
 public:
  typedef std::unordered_map<std::pair<std::string, std::string>, WeatherFake, PairHash> Fakes;

  WeatherMock(bool forecast_mode, float temp, int status_code)
    : fakes_{{{"", ""}, {forecast_mode, temp, status_code}}}
  {}

  WeatherMock(Fakes fakes)
    : fakes_(std::move(fakes))
  {}

  ~WeatherMock();

  MOCK_METHOD(cpr::Response, Get, (const std::string& city, const cpr::Url& url), (override));

  void DelegateToFake();

 private:
  Fakes fakes_;
};

nlohmann::json MakeMainResponse(float temp);

nlohmann::json MakeForecastResponse(float temp_tommorow);

cpr::Response MakeResponse(nlohmann::json json, int status_code);

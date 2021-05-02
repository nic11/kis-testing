//
// Created by Pavel Akhtyamov on 02.05.2020.
//

#include <iostream>
#include "Weather.h"

#include "cpr/cpr.h"


json Weather::GetResponseForCity(const std::string &city, const cpr::Url& url) {
  auto response = Get(city, url);

  if (response.status_code != 200) {
    throw std::invalid_argument("Api error. City is bad");
  }
  json result = json::parse(response.text);
  return result;
}

float Weather::GetTemperature(const std::string& city) {
  json response = GetResponseForCity(city, kBaseUrl);
  return response["main"]["temp"];
}

cpr::Response Weather::Get(const std::string& city, const cpr::Url& url) {
  return cpr::Get(
      url, cpr::Parameters{
          {"q", city},
          {"appid", api_key_},
          {"units", "metric"}
      }
  );
}

std::string Weather::GetDifferenceString(const std::string &city1, const std::string &city2) {
  float diff = FindDiffBetweenTwoCities(city1, city2);
  std::stringstream output;
  output << "Weather in " << city1 << " ";
  if (diff < 0) {
    output << "is colder than in " << city2 << " by " << int(-diff) << " degrees";
  } else {
    output << "is warmer than in " << city2 << " by " << int(diff) << " degrees";
  }

  return output.str();
}

float Weather::FindDiffBetweenTwoCities(const std::string &city1, const std::string& city2) {
  return GetTemperature(city1) - GetTemperature(city2);
}

void Weather::SetApiKey(const std::string &api_key) {
  api_key_ = api_key;
}

float Weather::GetTomorrowTemperature(const std::string& city) {
  json response = GetResponseForCity(city, kForecastUrl);
  return response["list"][7]["main"]["temp"];
}

std::string Weather::GetTomorrowDiff(const std::string &city) {
  float diff = GetTomorrowTemperature(city) - GetTemperature(city);
  std::stringstream output;

  output << "The weather in " << city << " tomorrow will be ";

  std::string response;

  if (diff > 3) {
    response = "much warmer";
  } else if (diff > 0.5) {
    response = "warmer";
  } else if (diff < -3) {
    response = "much colder";
  } else if (diff < -0.5) {
    response = "colder";
  } else {
    response = "the same";
  }

  output << response << " than today.";
  return output.str();
}


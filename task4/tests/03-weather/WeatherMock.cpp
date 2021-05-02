#include "WeatherMock.h"

cpr::Response WeatherFake::Get(const std::string& city, const cpr::Url& url) {
  auto json = forecast_mode_?
      MakeForecastResponse(temp_) : MakeMainResponse(temp_);
  // std::cout << "input " << url << "; " << city << "\noutput: " << json.dump(2) << "\n";
  return MakeResponse(json, status_code_);
}

WeatherMock::~WeatherMock() {
  EXPECT_EQ(fakes_.size(), 0) << "not all requests were caught";
}

void WeatherMock::DelegateToFake() {
  ON_CALL(*this, Get).WillByDefault(
    [this](const std::string& city, const cpr::Url& url) {
      auto it = fakes_.find({city, url});
      if (it == fakes_.end()) {
        it = fakes_.find({"", ""});
      }
      EXPECT_TRUE(it != fakes_.end()) << "too many or unknown request: "
          << city << ", " << url;
      auto result = it->second.Get(city, url);
      fakes_.erase(it);
      return result;
    }
  );
}

nlohmann::json MakeMainResponse(float temp) {
  nlohmann::json json = R"(
    {"main": {"temp": 1337}}
  )"_json;
  json["main"]["temp"] = temp;
  return json;
}

nlohmann::json MakeForecastResponse(float temp_tommorow) {
  auto forecast_entry = MakeMainResponse(temp_tommorow);
  auto root_entry = R"({"list": []})"_json;
  for (int i = 0; i < 7; ++i) {
    root_entry["list"].push_back("{}"_json);
  }
  root_entry["list"].push_back(std::move(forecast_entry));
  return root_entry;
}

cpr::Response MakeResponse(nlohmann::json json, int status_code) {
  cpr::Response res;
  res.status_code = status_code;
  res.text = json.dump();
  return res;
}

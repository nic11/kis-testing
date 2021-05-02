#include "WeatherTestCase.h"

#include "WeatherMock.h"

namespace {

using testing::_;
using testing::Return;

const std::string kWeatherUrl = "http://api.openweathermap.org/data/2.5/weather";
const std::string kForecastUrl = "http://api.openweathermap.org/data/2.5/forecast";

WeatherMock::Fakes kTokyoLondonFakes{
  {{"Tokyo", kWeatherUrl}, {false, 20, 200}},  // 20 degs in Tokyo
  {{"London", kWeatherUrl}, {false, 13, 200}}, // 13 degs in London
};

}

TEST_F(WeatherTestCase, TestMainRequest) {
  // std::cout << MakeMainResponse(32).dump(2) << "\n";
  // std::cout << MakeForecastResponse(32).dump(2) << "\n";

  WeatherMock mock(false, 42, 200);
  mock.DelegateToFake();
  mock.SetApiKey("some_api_key");

  EXPECT_CALL(
    mock,
    Get(
      "Moscow",
      cpr::Url{kWeatherUrl}
    )
  );

  ASSERT_EQ(mock.GetTemperature("Moscow"), 42);
}

TEST_F(WeatherTestCase, TestForecastRequest) {
  WeatherMock mock(true, 42, 200);
  mock.DelegateToFake();

  EXPECT_CALL(
    mock,
    Get(
      "Moscow",
      cpr::Url{kForecastUrl}
    )
  );

  ASSERT_EQ(mock.GetTomorrowTemperature("Moscow"), 42);
}

TEST_F(WeatherTestCase, TestBadRequest) {
  WeatherMock mock(true, 42, 404);
  mock.DelegateToFake();

  EXPECT_CALL(
    mock,
    Get(
      "Moscow",
      cpr::Url{kWeatherUrl}
    )
  );

  ASSERT_THAT(
    [&mock] {
      mock.GetTemperature("Moscow");
    },
    testing::ThrowsMessage<std::invalid_argument>(testing::Eq("Api error. City is bad"))
  );
}

TEST_F(WeatherTestCase, TestCitiesDiffWarmer) {
  WeatherMock mock(kTokyoLondonFakes);
  mock.DelegateToFake();

  EXPECT_CALL(
    mock,
    Get(
      "Tokyo",
      cpr::Url{kWeatherUrl}
    )
  );

  EXPECT_CALL(
    mock,
    Get(
      "London",
      cpr::Url{kWeatherUrl}
    )
  );

  ASSERT_EQ(
    mock.GetDifferenceString("Tokyo", "London"),
    "Weather in Tokyo is warmer than in London by 7 degrees"
  );
}

TEST_F(WeatherTestCase, TestCitiesDiffColder) {
  WeatherMock mock(kTokyoLondonFakes);
  mock.DelegateToFake();

  EXPECT_CALL(
    mock,
    Get(
      "London",
      cpr::Url{kWeatherUrl}
    )
  );

  EXPECT_CALL(
    mock,
    Get(
      "Tokyo",
      cpr::Url{kWeatherUrl}
    )
  );

  ASSERT_EQ(
    mock.GetDifferenceString("London", "Tokyo"),
    "Weather in London is colder than in Tokyo by 7 degrees"
  );
}

struct TomorrowDiffTestInput {
  float temp_today;
  float temp_tomorrow;
  std::string result;
};

const std::vector<TomorrowDiffTestInput> kTomorrowDiffTestInputs{
  {0, 0.499, "The weather in Moscow tomorrow will be the same than today."},
  {0, 2.9, "The weather in Moscow tomorrow will be warmer than today."},
  {0, 3.1, "The weather in Moscow tomorrow will be much warmer than today."},
  {0, -2.9, "The weather in Moscow tomorrow will be colder than today."},
  {0, -3.1, "The weather in Moscow tomorrow will be much colder than today."},
};

class TomorrowDiffTest : public testing::TestWithParam<TomorrowDiffTestInput> {};

TEST_P(TomorrowDiffTest, AllCases) {
  const auto param = GetParam();
  WeatherMock mock(
    {
      {{"Moscow", kWeatherUrl},  {false, param.temp_today,    200}},
      {{"Moscow", kForecastUrl}, {true,  param.temp_tomorrow, 200}},
    }
  );
  mock.DelegateToFake();

  EXPECT_CALL(
    mock,
    Get(
      "Moscow",
      cpr::Url{kWeatherUrl}
    )
  );

  EXPECT_CALL(
    mock,
    Get(
      "Moscow",
      cpr::Url{kForecastUrl}
    )
  );

  ASSERT_EQ(
    mock.GetTomorrowDiff("Moscow"),
    param.result
  );
}

INSTANTIATE_TEST_SUITE_P(
    /* none prefix */, TomorrowDiffTest,
    testing::ValuesIn(kTomorrowDiffTestInputs));

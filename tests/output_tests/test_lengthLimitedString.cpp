#define CATCH_CONFIG_RUNTIME_STATIC_REQUIRE

#include "randomString.h"
#include <catch2/catch_all.hpp>

#include "LengthLimitedString.h"

TEST_CASE("Static lengthLimitedString tests", "[Output][LengthLimitedString]") {
  STATIC_REQUIRE(std::is_same_v<JSOG::LengthLimitedString::LengthLimitedString,
                                std::string>);
  using ConstructType = decltype(JSOG::LengthLimitedString::construct);
  STATIC_REQUIRE(std::is_same_v<ConstructType, std::optional<std::string>(
                                                   const nlohmann::json&)>);

  using RawExportType = decltype(JSOG::LengthLimitedString::rawExport);
  STATIC_REQUIRE(
      std::is_same_v<RawExportType, nlohmann::json(const std::string&)>);

  using ValidateType = decltype(JSOG::LengthLimitedString::validate);
  STATIC_REQUIRE(std::is_same_v<ValidateType, bool(const std::string&)>);
}

TEST_CASE("Check LengthLimitedString::construct",
          "[Output][LengthLimitedString]") {
  SECTION("Valid size strings") {
    std::string str = GENERATE(take(100, randomString(10, 5)));
    INFO(str);
    nlohmann::json json = str;
    auto result = JSOG::LengthLimitedString::construct(json);
    CAPTURE(str);
    REQUIRE(result.has_value());
    CAPTURE(str);
    REQUIRE(result.value() == str);
  }

  SECTION("Invalid size strings") {
    SECTION("Shorter strings") {
      std::string str = GENERATE(take(100, randomString(4, 0)));
      nlohmann::json json = str;
      auto result = JSOG::LengthLimitedString::construct(json);
      CAPTURE(str);
      REQUIRE_FALSE(result.has_value());
    }

    SECTION("Longer strings") {
      std::string str = GENERATE(take(100, randomString(200, 11)));
      nlohmann::json json = str;
      auto result = JSOG::LengthLimitedString::construct(json);
      CAPTURE(str);
      REQUIRE_FALSE(result.has_value());
    }
  }
}

TEST_CASE("Check LengthLimitedString::rawExport",
          "[Output][LengthLimitedString]") {
  std::string str = GENERATE(take(100, randomString(10, 5)));
  auto result = JSOG::LengthLimitedString::rawExport(str);
  CAPTURE(str);
  REQUIRE(result.is_string());
  CAPTURE(str);
  REQUIRE(result == str);
}

TEST_CASE("Check LengthLimitedString::validate",
          "[Output][LengthLimitedString]") {
  std::string str = GENERATE(take(100, randomString(10, 5)));
  CAPTURE(str);
  REQUIRE(JSOG::LengthLimitedString::validate(str));
}

TEST_CASE("LengthLimitedString::json", "[Output][LengthLimitedString]") {
  SECTION("Valid strings") {
    std::string str = GENERATE(take(100, randomString(10, 5)));
    auto jsonopt = JSOG::LengthLimitedString::json(str);
    REQUIRE(jsonopt.has_value());
    auto json = jsonopt.value();
    CAPTURE(str);
    REQUIRE(json.is_string());
    CAPTURE(str);
    REQUIRE(json == str);
  }

  SECTION("Invalid strings") {
    SECTION("Shorter strings") {
      std::string str = GENERATE(take(100, randomString(4, 0)));
      auto jsonopt = JSOG::LengthLimitedString::json(str);
      REQUIRE_FALSE(jsonopt.has_value());
    }
    SECTION("Longer strings") {
      std::string str = GENERATE(take(100, randomString(200, 11)));
      auto jsonopt = JSOG::LengthLimitedString::json(str);
      REQUIRE_FALSE(jsonopt.has_value());
    }
  }
}
#include "RegexMatcherString.h"
#include "randomString.h"
#include <catch2/catch_all.hpp>

TEST_CASE("Static RegexMatcherString tests") {
  STATIC_REQUIRE(std::is_same_v<JSOG::RegexMatcherString::RegexMatcherString,
                                std::string>);
  using ConstructType = decltype(JSOG::RegexMatcherString::construct);
  STATIC_REQUIRE(std::is_same_v<ConstructType, std::optional<std::string>(
                                                   const nlohmann::json&)>);

  using RawExportType = decltype(JSOG::RegexMatcherString::rawExport);
  STATIC_REQUIRE(
      std::is_same_v<RawExportType, nlohmann::json(const std::string&)>);

  using ValidateType = decltype(JSOG::RegexMatcherString::validate);
  STATIC_REQUIRE(std::is_same_v<ValidateType, bool(const std::string&)>);
}

TEST_CASE("Check RegexMatcherString::construct") {
  SECTION("Valid strings") {
    std::string str = GENERATE(take(100, randomString()));
    for (auto& c : str) {
      bool lower = c >= 'a' && c <= 'z';
      bool upper = c >= 'A' && c <= 'Z';
      bool digit = c >= '0' && c <= '9';
      if (!lower && !upper && !digit) {
        c = 'x';
      }
    }
    INFO(str);
    nlohmann::json json = str;
    auto result = JSOG::RegexMatcherString::construct(json);
    CAPTURE(str);
    REQUIRE(result.has_value());
    CAPTURE(str);
    REQUIRE(result.value() == str);
  }

  SECTION("Invalid strings") {
    std::string str = GENERATE(take(100, randomString()));
    str.push_back('!');
    nlohmann::json json = str;
    auto result = JSOG::RegexMatcherString::construct(json);
    CAPTURE(str);
    REQUIRE_FALSE(result.has_value());
  }
}

TEST_CASE("Check RegexMatcherString::rawExport") {
  std::string str = GENERATE(take(100, randomString()));
  auto result = JSOG::RegexMatcherString::rawExport(str);
  CAPTURE(str);
  REQUIRE(result.is_string());
  CAPTURE(str);
  REQUIRE(result == str);
}

TEST_CASE("Check RegexMatcherString::validate") {
  SECTION("Valid strings") {
    std::string str = GENERATE(take(100, randomString()));
    for (auto& c : str) {
      bool lower = c >= 'a' && c <= 'z';
      bool upper = c >= 'A' && c <= 'Z';
      bool digit = c >= '0' && c <= '9';
      if (!lower && !upper && !digit) {
        c = 'x';
      }
    }
    CAPTURE(str);
    REQUIRE(JSOG::RegexMatcherString::validate(str));
  }

  SECTION("Invalid strings") {
    // Realistically, the chance of generating a valid string is very low,
    // less than 1 in 2^200
    std::string str = GENERATE(take(100, randomString(600, 200)));
    CAPTURE(str);
    REQUIRE_FALSE(JSOG::RegexMatcherString::validate(str));
  }
}
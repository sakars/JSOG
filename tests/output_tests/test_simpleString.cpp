
#define CATCH_CONFIG_RUNTIME_STATIC_REQUIRE
#include <catch2/catch_all.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>

#include "SimpleString.h"
#include "randomString.h"
#include <random>
#include <type_traits>

TEST_CASE("Simple String static tests", "[SimpleString]") {
  STATIC_REQUIRE(std::is_same_v<JSOG::SimpleString::SimpleString, std::string>);

  using ConstructType = decltype(JSOG::SimpleString::construct);
  STATIC_REQUIRE(std::is_same_v<ConstructType, std::optional<std::string>(
                                                   const nlohmann::json&)>);

  using RawExportType = decltype(JSOG::SimpleString::rawExport);
  STATIC_REQUIRE(
      std::is_same_v<RawExportType, nlohmann::json(const std::string&)>);

  using ValidateType = decltype(JSOG::SimpleString::validate);
  STATIC_REQUIRE(std::is_same_v<ValidateType, bool(const std::string&)>);
}

TEST_CASE("Check SimpleString::construct", "[SimpleString]") {
  std::string str = GENERATE(take(100, randomString()));
  nlohmann::json json = str;
  auto result = JSOG::SimpleString::construct(json);
  REQUIRE(result.has_value());
  CAPTURE(str);
  REQUIRE(result.value() == str);
}

TEST_CASE("Check SimpleString::rawExport", "[SimpleString]") {
  std::string str = GENERATE(take(100, randomString()));
  auto result = JSOG::SimpleString::rawExport(str);
  CAPTURE(str);
  REQUIRE(result.is_string());
  CAPTURE(str);
  REQUIRE(result == str);
}

TEST_CASE("Check SimpleString::validate", "[SimpleString]") {
  std::string str = GENERATE(take(100, randomString()));
  CAPTURE(str);
  REQUIRE(JSOG::SimpleString::validate(str));
}

TEST_CASE("SimpleString::json", "[SimpleString]") {
  std::string str = GENERATE(take(100, randomString()));
  nlohmann::json json = JSOG::SimpleString::json(str).value();
  CAPTURE(str);
  REQUIRE(json.is_string());
  CAPTURE(str);
  REQUIRE(json == str);
}
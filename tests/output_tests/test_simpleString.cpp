
#define CATCH_CONFIG_RUNTIME_STATIC_REQUIRE
#include <catch2/catch_all.hpp>

#include "SimpleString.h"
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

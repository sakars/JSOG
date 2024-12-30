
#define CATCH_CONFIG_RUNTIME_STATIC_REQUIRE
#include <catch2/catch_all.hpp>

#include "True.h"
#include <type_traits>

TEST_CASE("True static tests", "[Output][True]") {
  STATIC_REQUIRE(std::is_same_v<JSOG::True::True, nlohmann::json>);

  using ConstructType = decltype(JSOG::True::construct);
  STATIC_REQUIRE(std::is_same_v<ConstructType, std::optional<nlohmann::json>(
                                                   const nlohmann::json&)>);

  using RawExportType = decltype(JSOG::True::rawExport);
  STATIC_REQUIRE(
      std::is_same_v<RawExportType, nlohmann::json(const nlohmann::json&)>);

  using ValidateType = decltype(JSOG::True::validate);
  STATIC_REQUIRE(std::is_same_v<ValidateType, bool(const nlohmann::json&)>);
}
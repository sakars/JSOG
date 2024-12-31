
#define CATCH_CONFIG_RUNTIME_STATIC_REQUIRE
#include <catch2/catch_all.hpp>

#include "StringNumberVariant.h"

TEST_CASE("Static StringNumberVariant checks",
          "[Output][StringNumberVariant]") {
  STATIC_CHECK((std::is_same_v<JSOG::StringNumberVariant::StringNumberVariant,
                               std::variant<std::string, double>> ||
                std::is_same_v<JSOG::StringNumberVariant::StringNumberVariant,
                               std::variant<double, std::string>>));

  using ConstructType = decltype(JSOG::StringNumberVariant::construct);
  STATIC_REQUIRE(std::is_same_v<
                 ConstructType,
                 std::optional<JSOG::StringNumberVariant::StringNumberVariant>(
                     const nlohmann::json&)>);

  using RawExportType = decltype(JSOG::StringNumberVariant::rawExport);
  STATIC_REQUIRE(std::is_same_v<
                 RawExportType,
                 nlohmann::json(
                     const JSOG::StringNumberVariant::StringNumberVariant&)>);

  using JsonType = decltype(JSOG::StringNumberVariant::json);
  STATIC_REQUIRE(std::is_same_v<
                 JsonType,
                 std::optional<nlohmann::json>(
                     const JSOG::StringNumberVariant::StringNumberVariant&)>);

  using ValidateType = decltype(JSOG::StringNumberVariant::validate);
  STATIC_REQUIRE(std::is_same_v<
                 ValidateType,
                 bool(const JSOG::StringNumberVariant::StringNumberVariant&)>);
}
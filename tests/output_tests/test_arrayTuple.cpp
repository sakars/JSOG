#include "arrayTuple.h"

// #define CATCH_CONFIG_RUNTIME_STATIC_REQUIRE
#include <catch2/catch_all.hpp>

TEST_CASE("arrayTuple: Static checks", "[Output][arrayTuple]") {
  JSOG::arrayTuple::Array array;
  STATIC_REQUIRE((std::is_same_v<decltype(JSOG::arrayTuple::Array::item0),
                                 std::optional<std::string>>));
  STATIC_REQUIRE((std::is_same_v<decltype(JSOG::arrayTuple::Array::item1),
                                 std::optional<double>>));
  STATIC_REQUIRE((std::is_same_v<decltype(JSOG::arrayTuple::Array::items),
                                 std::vector<unsigned char>>));
  STATIC_REQUIRE(
      std::is_same_v<decltype(array.get<0>()), std::optional<std::string>>);
  STATIC_REQUIRE(
      (std::is_same_v<decltype(array.get<1>()), std::optional<double>>));
  STATIC_REQUIRE((std::is_same_v<decltype(array.get<2>()), unsigned char>));
  STATIC_REQUIRE((std::is_same_v<decltype(array.get<3>()), unsigned char>));
}
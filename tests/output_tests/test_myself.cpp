#include "Myself.h"
#include <catch2/catch_all.hpp>

TEST_CASE("Myself: Static checks", "[myself]") {
  JSOG::Myself::Myself myself;
  myself.object_ = std::make_unique<JSOG::Myself::Myself>();
}
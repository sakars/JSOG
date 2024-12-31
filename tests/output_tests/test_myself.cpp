#include "Myself.h"
#include <catch2/catch_all.hpp>

TEST_CASE("Myself: Static checks for whether self-referential types can exist",
          "[Output][myself]") {
  JSOG::Myself::Myself myself;
  myself.object_ = std::make_unique<JSOG::Myself::Myself>();
  myself.additionalProperties["key"] = "value";
  REQUIRE(myself.object_ != nullptr);
}

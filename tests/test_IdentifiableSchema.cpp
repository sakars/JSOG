#include <catch2/catch_all.hpp>

#include "IdentifiableSchema.h"
#include "LinkedSchema.h"
#include "UriWrapper.h"

TEST_CASE("IdentifiableSchema") {
  SECTION("transition") {
    const auto json = R"(
{
  "$id": "https://example.com/schema",
  "title": "Schema",
  "properties": {
    "a": {
      "type": "string"
    }
  }
})"_json;
    const UriWrapper baseUri("https://example.com/schema");
    const JSONPointer pointer = JSONPointer();
    const Draft draft = Draft::DRAFT_07;
    std::vector<std::unique_ptr<LinkedSchema>> linkedSchemas;
    linkedSchemas.emplace_back(std::make_unique<LinkedSchema>(
        json["properties"]["a"], baseUri, pointer / "properties" / "a", draft,
        std::map<UriWrapper, size_t>{}));
    linkedSchemas.emplace_back(std::make_unique<LinkedSchema>(
        json, baseUri, pointer, draft,
        std::map<UriWrapper, size_t>{
            {baseUri.withPointer(pointer / "properties" / "a"), 0}}));
    auto identifiableSchemas =
        IdentifiableSchema::transition(std::move(linkedSchemas));
    REQUIRE(identifiableSchemas.size() == 2);
    REQUIRE(identifiableSchemas[1].dependencies_.contains(
        baseUri.withPointer(pointer / "properties" / "a")));
    REQUIRE(identifiableSchemas[1].dependencies_.at(
                baseUri.withPointer(pointer / "properties" / "a")) == 0);
    // Identifiers are unique

    REQUIRE(identifiableSchemas[1].identifier_ !=
            identifiableSchemas[0].identifier_);

    REQUIRE(identifiableSchemas[0].identifier_ == "Schema");
    REQUIRE(identifiableSchemas[1].identifier_ == "Schema0");
  }
}
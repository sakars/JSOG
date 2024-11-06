#include <catch2/catch_all.hpp>

#include "IdentifiableSchema.h"
#include "LinkedDraft07.h"
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
    linkedSchemas.push_back(std::make_unique<LinkedDraft07>(
        json["properties"]["a"], baseUri, pointer / "properties" / "a", draft));
    linkedSchemas.push_back(
        std::make_unique<LinkedDraft07>(json, baseUri, pointer, draft));
    linkedSchemas[1]->dependenciesSet_.insert(
        baseUri.withPointer(pointer / "properties" / "a"));
    linkedSchemas[1]->dependencies_.insert(
        {baseUri.withPointer(pointer / "properties" / "a"), *linkedSchemas[0]});
    auto identifiableSchemas =
        IdentifiableSchema::transition(std::move(linkedSchemas));
    REQUIRE(identifiableSchemas.size() == 2);
    REQUIRE(identifiableSchemas[1]->dependencies_.contains(
        baseUri.withPointer(pointer / "properties" / "a")));
    REQUIRE(&identifiableSchemas[1]
                 ->dependencies_
                 .at(baseUri.withPointer(pointer / "properties" / "a"))
                 .get() == identifiableSchemas[0].get());
    // Identifiers are unique

    REQUIRE(identifiableSchemas[1]->identifier_ !=
            identifiableSchemas[0]->identifier_);

    REQUIRE(identifiableSchemas[0]->identifier_ == "Schema");
    REQUIRE(identifiableSchemas[1]->identifier_ == "Schema0");
  }
}
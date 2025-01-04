#include <catch2/catch_all.hpp>

#include "IdentifiableSchema.h"
#include "LinkedSchema.h"
#include "UriWrapper.h"

TEST_CASE("IdentifiableSchema transition", "[IdentifiableSchema]") {
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

TEST_CASE("IdentifiableSchema transition with preferred identifiers",
          "[IdentifiableSchema]") {
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
  std::map<UriWrapper, std::string> preferredIdentifiers = {
      {baseUri.withPointer(pointer), "MySchema"},
      {baseUri.withPointer(pointer / "properties" / "a"), "MySchemaA"}};

  auto identifiableSchemas = IdentifiableSchema::transition(
      std::move(linkedSchemas), preferredIdentifiers);

  REQUIRE(identifiableSchemas.size() == 2);
  REQUIRE((identifiableSchemas[0].dependencies_.contains(
               baseUri.withPointer(pointer / "properties" / "a")) ||
           identifiableSchemas[1].dependencies_.contains(
               baseUri.withPointer(pointer / "properties" / "a"))));

  if (identifiableSchemas[0].dependencies_.contains(
          baseUri.withPointer(pointer / "properties" / "a"))) {
    REQUIRE(identifiableSchemas[0].dependencies_.at(
                baseUri.withPointer(pointer / "properties" / "a")) == 0);
  } else if (identifiableSchemas[1].dependencies_.contains(
                 baseUri.withPointer(pointer / "properties" / "a"))) {
    REQUIRE(identifiableSchemas[1].dependencies_.at(
                baseUri.withPointer(pointer / "properties" / "a")) == 0);
  } else {
    FAIL("Dependency not found");
  }
  // Identifiers are unique

  REQUIRE(identifiableSchemas[1].identifier_ !=
          identifiableSchemas[0].identifier_);

  REQUIRE((identifiableSchemas[0].identifier_ == "MySchema" ||
           identifiableSchemas[1].identifier_ == "MySchema"));
  REQUIRE((identifiableSchemas[1].identifier_ == "MySchemaA" ||
           identifiableSchemas[0].identifier_ == "MySchemaA"));
}

/// @brief Preferred identifiers caused an incident where some schemas were
/// handled before the rest of the schemas, causing swaps in the array, breaking
/// dependencies. This test case ensures that the dependencies are not broken.
TEST_CASE("IdentifiableSchema preferred identifiers don't break dependencies",
          "[IdentifiableSchema]") {
  const auto json = R"(
{
  "$id": "https://example.com/schema",
  "title": "Schema",
  "properties": {
    "a": {
      "type": "string"
    },
    "b": {
      "$ref": "#/properties/a"
    }
  },
  "additionalProperties": false,
  "required": ["a"],
  "type": "object"
})"_json;
  const UriWrapper baseUri("https://example.com/schema");
  const JSONPointer pointer = JSONPointer();
  const Draft draft = Draft::DRAFT_07;
  std::vector<std::unique_ptr<LinkedSchema>> linkedSchemas;
  linkedSchemas.emplace_back(std::make_unique<LinkedSchema>(
      json["properties"]["a"], baseUri, pointer / "properties" / "a", draft,
      std::map<UriWrapper, size_t>{}));
  linkedSchemas.emplace_back(std::make_unique<LinkedSchema>(
      json["properties"]["b"], baseUri, pointer / "properties" / "b", draft,
      std::map<UriWrapper, size_t>{
          {baseUri.withPointer(pointer / "properties" / "a"), 0}}));
  linkedSchemas.emplace_back(std::make_unique<LinkedSchema>(
      json, baseUri, pointer, draft,
      std::map<UriWrapper, size_t>{
          {baseUri.withPointer(pointer / "properties" / "a"), 0},
          {baseUri.withPointer(pointer / "properties" / "b"), 1}}));
  std::map<UriWrapper, std::string> preferredIdentifiers = {
      {baseUri.withPointer(pointer / "properties" / "b"), "MySchemaB"}};

  auto identifiableSchemas = IdentifiableSchema::transition(
      std::move(linkedSchemas), preferredIdentifiers);

  REQUIRE(identifiableSchemas.size() == 3);
  REQUIRE(identifiableSchemas[0].pointer_ == pointer / "properties" / "a");
  REQUIRE(identifiableSchemas[1].pointer_ == pointer / "properties" / "b");
  REQUIRE(identifiableSchemas[2].pointer_ == pointer);
}

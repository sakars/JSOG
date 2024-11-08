#include "UnresolvedSchema.h"
#include <catch2/catch_all.hpp>

TEST_CASE("UnresolvedSchema construction", "[UnresolvedSchema]") {
  SECTION("Construction from atoms") {
    nlohmann::json json = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "Test Schema"
}
)"_json;
    UriWrapper fileUri("file://test.json");
    JSONPointer pointer;
    UnresolvedSchema schema(json, fileUri, pointer);
    REQUIRE(schema.json_.get() == json);
    REQUIRE(schema.baseUri_ == fileUri);
    REQUIRE(schema.pointer_ == pointer);
  }

  SECTION("Construction from DraftRecognisedDocument") {
    nlohmann::json json = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "Test Schema"
}
)"_json;
    UriWrapper fileUri("file://test.json");
    DraftRecognisedDocument draft(json, fileUri);
    UnresolvedSchema schema(draft);
    REQUIRE(schema.json_.get() == json);
    REQUIRE(schema.baseUri_ == fileUri);
    REQUIRE(schema.pointer_ == JSONPointer());
  }

  SECTION("Construction from path") {
    std::filesystem::path path = "samples/document_1.json";
    Document doc(path);
    DraftRecognisedDocument draft(std::move(doc));
    UnresolvedSchema schema(draft);
    REQUIRE(schema.json_.get() == draft.json_);
    REQUIRE(schema.baseUri_ == draft.fileUri_);
    REQUIRE(schema.pointer_ == JSONPointer());
  }
}

TEST_CASE("Unresolved Schema setmap construction", "[UnresolvedSchema]") {
  nlohmann::json json = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "Test Schema"
}
)"_json;
  UriWrapper fileUri("file://test.json");
  JSONPointer pointer;
  std::vector<DraftRecognisedDocument> drafts{
      DraftRecognisedDocument(json, fileUri)};
  auto unresolvedMap = UnresolvedSchema::generateSetMap(drafts);
  auto extracted = unresolvedMap.extract();
  REQUIRE(extracted.size() == 1);
  auto &[keys, value] = extracted[0];
  REQUIRE(keys.size() == 1);
  REQUIRE(keys[0] == fileUri);
  REQUIRE(value->json_.get() == json);
  REQUIRE(value->baseUri_ == fileUri);
  REQUIRE(value->pointer_ == pointer);
}
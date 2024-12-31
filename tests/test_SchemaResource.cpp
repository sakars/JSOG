#include "SchemaResource.h"
#include <catch2/catch_all.hpp>

TEST_CASE("Schema Resource construction",
          "[SchemaResource][DraftRecognisedDocument][Document]") {
  SECTION("Construction from atoms") {
    nlohmann::json json = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "Test Schema"
}
)"_json;
    UriWrapper fileUri("file://test.json");
    JSONPointer pointer;
    SchemaResource schema(json, fileUri, pointer);
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
    DraftRecognisedDocument draft{nlohmann::json(json), UriWrapper(fileUri)};
    SchemaResource schema(draft);
    REQUIRE(schema.json_.get() == json);
    REQUIRE(schema.baseUri_ == fileUri);
    REQUIRE(schema.pointer_ == JSONPointer());
  }

  SECTION("Construction from path") {
    std::filesystem::path path = "samples/document_1.json";
    Document doc(path);
    DraftRecognisedDocument draft(std::move(doc));
    SchemaResource schema(draft);
    REQUIRE(schema.json_.get() == draft.json_);
    REQUIRE(schema.baseUri_ == draft.fileUri_);
    REQUIRE(schema.pointer_ == JSONPointer());
  }
}

TEST_CASE("Schema Resource setmap construction",
          "[SchemaResource][DraftRecognisedDocument]") {
  nlohmann::json json = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "Test Schema"
}
)"_json;
  UriWrapper fileUri("file://test.json");
  JSONPointer pointer;
  std::vector<DraftRecognisedDocument> drafts{
      DraftRecognisedDocument{nlohmann::json(json), UriWrapper(fileUri)}};
  auto resourceMap = SchemaResource::generateSetMap(drafts);
  auto extracted = resourceMap.extract();
  REQUIRE(extracted.size() == 1);
  auto& [keys, value] = extracted[0];
  REQUIRE(keys.size() == 1);
  REQUIRE(keys[0] == fileUri);
  REQUIRE(value->json_.get() == json);
  REQUIRE(value->baseUri_ == fileUri);
  REQUIRE(value->pointer_ == pointer);
}

TEST_CASE("Full pipeline run up to LinkedSchema",
          "[Draft07][LinkedSchema][DraftRecognisedDocument]["
          "Document][SchemaResource]") {
  std::vector<std::filesystem::path> files{"./samples/document_1.json",
                                           "./samples/document_2.json"};
  auto documents = loadDocuments(files);
  auto recognised =
      DraftRecognisedDocument::performDraftRecognition(std::move(documents));
  auto schemaResources = SchemaResource::generateSetMap(std::move(recognised));
  REQUIRE(schemaResources.getSet().size() == 5);
  const auto doesSetContainUri = [&](const UriWrapper& uri) {
    for (const auto& value : schemaResources.getSet()) {
      if (value->baseUri_.withPointer(value->pointer_) == uri) {
        return true;
      }
    }
    return false;
  };
  REQUIRE(doesSetContainUri(UriWrapper("file://samples/document_1.json")));
  REQUIRE(doesSetContainUri(UriWrapper("file://samples/document_2.json")));
  REQUIRE(doesSetContainUri(UriWrapper("file://samples/document_2.json")
                                .withPointer(JSONPointer() / "properties")));
  REQUIRE(doesSetContainUri(
      UriWrapper("file://samples/document_2.json")
          .withPointer(JSONPointer() / "properties" / "name")));
  REQUIRE(doesSetContainUri(
      UriWrapper("file://samples/document_2.json")
          .withPointer(JSONPointer() / "properties" / "ref")));
}
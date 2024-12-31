#include "Document.h"
#include <catch2/catch_all.hpp>

TEST_CASE("Document loads from existing json instance properly", "[Document]") {
  nlohmann::json json = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "Test Schema"
}
)"_json;
  UriWrapper fileUri("file://test.json");
  Document doc(std::move(json), std::move(fileUri));
  REQUIRE(doc.fileUri_ == "file://test.json");
  REQUIRE(doc.json_.is_object());
  REQUIRE(doc.json_["title"] == "Test Schema");
  REQUIRE(doc.json_["$schema"] == "http://json-schema.org/draft-07/schema#");
}

TEST_CASE("Document loads from file properly", "[Document]") {
  std::filesystem::path path = "samples/document_1.json";
  Document doc(path);
  REQUIRE(doc.fileUri_ == "file://" + std::filesystem::absolute(path).string());
  REQUIRE(doc.json_.is_object());
  REQUIRE(doc.json_["title"] == "Test Schema");
  REQUIRE(doc.json_["$schema"] == "http://json-schema.org/draft-07/schema#");
}

TEST_CASE("FileUris are unique", "[Document]") {
  std::filesystem::path path1 = "samples/document_1.json";
  std::filesystem::path path2 = "samples/document_2.json";
  Document doc1(path1);
  Document doc2(path2);
  REQUIRE(doc1.fileUri_ != doc2.fileUri_);
}

TEST_CASE("Load Documents loads documents with unique fileUris", "[Document]") {
  std::vector<std::filesystem::path> paths{"samples/document_1.json",
                                           "samples/document_2.json"};
  auto documents = loadDocuments(paths);
  REQUIRE(documents.size() == 2);
  REQUIRE(documents[0].fileUri_ != documents[1].fileUri_);
}

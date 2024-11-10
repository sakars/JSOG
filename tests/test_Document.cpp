#include "Document.h"
#include <catch2/catch_all.hpp>

TEST_CASE("Document loads from existing json instance properly", "[Document]")
{
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

TEST_CASE("Document loads from file properly", "[Document]")
{
    std::filesystem::path path = "samples/document_1.json";
    Document doc(path);
    REQUIRE(doc.fileUri_ == "file://" + std::filesystem::absolute(path).string());
    REQUIRE(doc.json_.is_object());
    REQUIRE(doc.json_["title"] == "Test Schema");
    REQUIRE(doc.json_["$schema"] == "http://json-schema.org/draft-07/schema#");
}
#include "DraftRecognisedDocument.h"
#include <catch2/catch_all.hpp>

TEST_CASE("DraftRecognisedDocument construction", "[DraftRecognisedDocument]")
{
    nlohmann::json json = R"(
{
"$schema": "http://json-schema.org/draft-07/schema#",
"title": "Test Schema"
}
)"_json;
    UriWrapper fileUri("file://test.json");
    SECTION("Pure construction")
    {
        DraftRecognisedDocument doc(json, fileUri);
        REQUIRE(doc.draft_ == Draft::DRAFT_07);
        REQUIRE(doc.fileUri_ == fileUri);
        REQUIRE(doc.json_ == json);
    }

    SECTION("Construction through Document")
    {
        Document doc(std::move(json), std::move(fileUri));
        DraftRecognisedDocument doc_(std::move(doc));
        REQUIRE(doc_.draft_ == Draft::DRAFT_07);
        REQUIRE(doc_.fileUri_ == "file://test.json");
        REQUIRE(doc_.json_.is_object());
        REQUIRE(doc_.json_["title"] == "Test Schema");
        REQUIRE(doc_.json_["$schema"] == "http://json-schema.org/draft-07/schema#");
    }

    SECTION("Construction through Document with file")
    {
        std::filesystem::path path = "samples/document_1.json";
        Document doc(path);
        DraftRecognisedDocument doc_(std::move(doc));
        REQUIRE(doc_.draft_ == Draft::DRAFT_07);
        REQUIRE(doc_.fileUri_ == "file://" + std::filesystem::absolute(path).string());
        REQUIRE(doc_.json_.is_object());
        REQUIRE(doc_.json_["title"] == "Test Schema");
        REQUIRE(doc_.json_["$schema"] == "http://json-schema.org/draft-07/schema#");
    }
}
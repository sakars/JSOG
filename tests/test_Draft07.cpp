#include "Draft07.h"
#include <catch2/catch_all.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <nlohmann/json.hpp>

TEST_CASE("Issue Locator for Draft07", "[Draft07]") {
  SECTION("Boolean Schema") {
    nlohmann::json json = true;
    UriWrapper baseUri("http://example.com");
    JSONPointer pointer;
    auto issues = issuesWithDraft07Schema(json, baseUri, pointer);
    REQUIRE(issues.size() == 0);
  }
  SECTION("Bad types for whole schema") {
    nlohmann::json json =
        GENERATE(nlohmann::json::array(), "test string", 69, 420.69, nullptr);
    UriWrapper baseUri("http://example.com");
    JSONPointer pointer;
    DYNAMIC_SECTION("Bad schema: " << json.dump()) {
      auto issues = issuesWithDraft07Schema(json, baseUri, pointer);
      REQUIRE(issues.size() == 1);
    }
  }
  SECTION("Bad $id: not a string") {
    nlohmann::json json = {{"$id", 69}};
    UriWrapper baseUri("http://example.com");
    JSONPointer pointer;
    auto issues = issuesWithDraft07Schema(json, baseUri, pointer);
    REQUIRE(issues.size() == 1);
  }
  SECTION("Bad $id: not an absolute URI") {
    nlohmann::json json = {{"$id", "relative.uri.com/with/segments"}};
    UriWrapper baseUri("http://example.com");
    JSONPointer pointer;
    auto issues = issuesWithDraft07Schema(json, baseUri, pointer);
    REQUIRE(issues.size() == 1);
  }
  SECTION("Bad $id: has fragment") {
    nlohmann::json json = {{"$id", "http://example.com/#fragment"}};
    UriWrapper baseUri("http://example.com");
    JSONPointer pointer;
    auto issues = issuesWithDraft07Schema(json, baseUri, pointer);
    REQUIRE(issues.size() == 1);
  }
  SECTION("Good $id") {
    nlohmann::json json = {{"$id", "http://example.com"}};
    UriWrapper baseUri("http://example.com");
    JSONPointer pointer;
    auto issues = issuesWithDraft07Schema(json, baseUri, pointer);
    REQUIRE(issues.size() == 0);
  }
  SECTION("Bad type: not a string") {
    nlohmann::json json = {{"type", 69}};
    UriWrapper baseUri("http://example.com");
    JSONPointer pointer;
    auto issues = issuesWithDraft07Schema(json, baseUri, pointer);
    REQUIRE(issues.size() == 1);
  }
  SECTION("Bad type: not a valid type") {
    nlohmann::json json = {{"type", "invalid"}};
    UriWrapper baseUri("http://example.com");
    JSONPointer pointer;
    auto issues = issuesWithDraft07Schema(json, baseUri, pointer);
    REQUIRE(issues.size() == 1);
  }
  SECTION("Good type:") {
    std::string type = GENERATE("null", "boolean", "object", "array", "number",
                                "string", "integer");
    nlohmann::json json = {{"type", type}};
    UriWrapper baseUri("http://example.com");
    JSONPointer pointer;
    auto issues = issuesWithDraft07Schema(json, baseUri, pointer);
    REQUIRE(issues.size() == 0);
  }
  SECTION("Good type: array of types") {
    nlohmann::json json = {
        {"type", nlohmann::json::array({"null", "boolean"})}};
    UriWrapper baseUri("http://example.com");
    JSONPointer pointer;
    auto issues = issuesWithDraft07Schema(json, baseUri, pointer);
    REQUIRE(issues.size() == 0);
  }
  SECTION("Bad type: duplicate types") {
    nlohmann::json json = {{"type", nlohmann::json::array({"null", "null"})}};
    UriWrapper baseUri("http://example.com");
    JSONPointer pointer;
    auto issues = issuesWithDraft07Schema(json, baseUri, pointer);
    REQUIRE(issues.size() == 1);
  }
  SECTION("Bad type: invalid type in array") {
    nlohmann::json json = {
        {"type", nlohmann::json::array({"null", "invalid"})}};
    UriWrapper baseUri("http://example.com");
    JSONPointer pointer;
    auto issues = issuesWithDraft07Schema(json, baseUri, pointer);
    REQUIRE(issues.size() == 1);
  }
  SECTION("Bad enum: not an array") {
    nlohmann::json json = {
        {"enum", GENERATE(nlohmann::json(69), nlohmann::json("test string"),
                          nlohmann::json(nullptr))}};
    UriWrapper baseUri("http://example.com");
    JSONPointer pointer;
    auto issues = issuesWithDraft07Schema(json, baseUri, pointer);
    REQUIRE(issues.size() == 1);
  }
  SECTION("Good enum") {
    nlohmann::json json = {
        {"enum",
         GENERATE(nlohmann::json::array(), nlohmann::json::array({69, 420.69}),
                  nlohmann::json::array({"test string"}),
                  nlohmann::json::array({nullptr, 420}))}};
    UriWrapper baseUri("http://example.com");
    JSONPointer pointer;
    auto issues = issuesWithDraft07Schema(json, baseUri, pointer);
    REQUIRE(issues.size() == 0);
  }
}
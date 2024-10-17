#include "ResourceIndex.h"
#include <catch2/catch_all.hpp>

TEST_CASE("ResourceIndex correctly adds resources: Simple example",
          "[ResourceIndex]") {
  ResourceIndex index;
  nlohmann::json json = R"(
    {
      "$id": "http://example.com",
      "anchor": "fragment"
    }
  )"_json;
  index.addResource(json, {}, "file:///root.json");

  for (const auto &[key, value] : index) {
    std::cout << key << ": " << value->json.dump(1) << std::endl;
  }

  REQUIRE(index.contains("http://example.com"));
  REQUIRE(index.contains("file:///root.json#fragment"));
  REQUIRE((index["http://example.com"]->json) ==
          (index["file:///root.json#fragment"]->json));
}

TEST_CASE("ResourceIndex correctly adds resources: Draft07 example",
          "[ResourceIndex]") {
  ResourceIndex index;
  nlohmann::json json = R"(
  {
    "$id": "http://example.com/root.json",
    "definitions": {
        "A": { "$id": "#foo" },
        "B": {
            "$id": "other.json",
            "definitions": {
                "X": { "$id": "#bar" },
                "Y": { "$id": "t/inner.json" }
            }
        },
        "C": {
            "$id": "urn:uuid:ee564b8a-7a87-4125-8c96-e9f123d6766f"
        }
    }
  })"_json;
  index.addResource(json, {}, "file:///root.json");

  for (const auto &[key, value] : index) {
    std::cout << key << ": " << value->json.dump(1) << std::endl;
  }

  auto &r = index;

  const auto t = [&](const auto &json, const std::vector<std::string> &keys) {
    for (const auto &key : keys) {
      DYNAMIC_SECTION("Key: " << key) {
        CAPTURE(key, json);
        REQUIRE(r.contains(key));
        REQUIRE(r[key] != nullptr);
        REQUIRE(r[key]->json == json);
      }
    }
  };

  t(json, {"http://example.com/root.json", "http://example.com/root.json#"});

  t(json["definitions"]["A"], {"http://example.com/root.json#foo",
                               "http://example.com/root.json#/definitions/A"});

  t(json["definitions"]["B"],
    {"http://example.com/other.json", "http://example.com/other.json#",
     "http://example.com/root.json#/definitions/B"});

  t(json["definitions"]["B"]["definitions"]["X"],
    {"http://example.com/other.json#bar",
     "http://example.com/other.json#/definitions/X",
     "http://example.com/root.json#/definitions/B/definitions/X"});

  t(json["definitions"]["B"]["definitions"]["Y"],
    {"http://example.com/t/inner.json", "http://example.com/t/inner.json#",
     "http://example.com/root.json#/definitions/B/definitions/Y",
     "http://example.com/other.json#/definitions/Y"});

  t(json["definitions"]["C"], {"urn:uuid:ee564b8a-7a87-4125-8c96-e9f123d6766f",
                               "urn:uuid:ee564b8a-7a87-4125-8c96-e9f123d6766f#",
                               "http://example.com/root.json#/definitions/C"});
}

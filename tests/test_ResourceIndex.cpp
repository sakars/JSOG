#include "ResourceIndex.h"
#include <catch2/catch_all.hpp>

TEST_CASE("ResourceIndex correctly adds resources: Simple example",
          "[ResourceIndex]")
{
  ResourceIndex index;
  nlohmann::json json = R"(
    {
      "$id": "http://example.com",
      "anchor": "fragment"
    }
  )"_json;
  index.addResource(json, {}, "file:///root.json");

  for (const auto &[key, value] : index)
  {
    std::cout << key.toString().value_or("INVALID_URI") << ": " << value->json.dump(1) << std::endl;
  }

  REQUIRE(index.contains("http://example.com"));
  REQUIRE(index.contains("file:///root.json#fragment"));
  REQUIRE((index["http://example.com"]->json) ==
          (index["file:///root.json#fragment"]->json));
}

TEST_CASE("ResourceIndex correctly adds resources: Draft07 example",
          "[ResourceIndex]")
{
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

  for (const auto &[key, value] : index)
  {
    std::cout << key.toString().value_or("Unknown") << ": " << value->json.dump(1) << std::endl;
  }

  const auto t = [&](const auto &json, const std::vector<std::string> &keys)
  {
    for (const auto &key : keys)
    {
      DYNAMIC_SECTION("Key: " << key)
      {
        CAPTURE(key, json);
        REQUIRE(index.contains(key));
        REQUIRE(index[key] != nullptr);
        REQUIRE(index[key]->json == json);
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

TEST_CASE("ResourceIndex correctly builds objects", "[ResourceIndex]")
{
  ResourceIndex index;
  nlohmann::json json = R"(
    {
      "$schema": "http://json-schema.org/draft-07/schema#",
      "$id": "http://example.com/root.json",
      "allOf": [
        { "$id": "http://example.com/root2.json" },
        { "$id": "http://example.com/root3.json" }
      ],
      "random":
      {
        "$id": "http://example.com/root4.json"
      }
    }
  )"_json;

  index.addResource(json, {}, "file:///root.json");
  index.markForBuild("http://example.com/root.json");
  index.build();

  // for (const auto &[key, value] : index) {
  //   std::cout << key << ": " << value->json.dump(1) << std::endl;
  // }
  SECTION("Build stage")
  {
    REQUIRE(index.contains("http://example.com/root.json"));
    REQUIRE(index.contains("http://example.com/root2.json"));
    REQUIRE(index.contains("http://example.com/root3.json"));
    REQUIRE(index.contains("http://example.com/root4.json"));
  }

  index.generateUniqueSchemaNames();
  index.resolveReferences();
  const auto schemas = index.extractSchemas();

  SECTION("Extraction stage")
  {
    // has no random as it is not required to be built by any other schema
    REQUIRE(schemas.size() == 3);
    const auto hasJson = [](const std::vector<std::unique_ptr<Schema>> &schemas,
                            const nlohmann::json &json) -> bool
    {
      for (const auto &schema : schemas)
      {
        if (schema.get()->getJson() == json)
        {
          return true;
        }
      }
      return false;
    };
    REQUIRE(hasJson(schemas, json));
    REQUIRE(hasJson(schemas, json["allOf"][0]));
    REQUIRE(hasJson(schemas, json["allOf"][1]));
  }
}

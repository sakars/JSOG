#include "Draft07.h"
#include "JSONPointer.h"
#include "LinkedSchema.h"
#include <catch2/catch_all.hpp>

SCENARIO("Buildable Schema Draft07 constructs correct deps",
         "[Draft07][LinkedSchema]") {
  // FAIL("Refactors needed");
  WHEN("Schema has a ref in properties") {
    nlohmann::json json = R"(
  {
      "$schema": "http://json-schema.org/draft-07/schema#",
      "title": "Core schema meta-schema",
      "properties": {
        "test": {
          "#ref": "#"
        }
      }
  })"_json;
    UriWrapper fileUri("file://test.json");
    JSONPointer pointer;
    UnresolvedSchema schema(json, fileUri, pointer);
    LinkedSchema buildable(schema, std::map<UriWrapper, size_t>{});
    THEN("It is added to dependencies") {
      auto deps = getDraft07Dependencies(json, fileUri, pointer);
      REQUIRE(deps.size() == 1);
      REQUIRE(deps.count(
                  fileUri.withPointer(pointer / "properties" / "test")) == 1);
    }
  }

  WHEN("Schema has an array of items") {
    nlohmann::json json = R"(
  {
      "$schema": "http://json-schema.org/draft-07/schema#",
      "title": "Core schema meta-schema",
      "items": [
        {
          "$id": "http://example.com/root2.json"
        },
        {
          "$id": "http://example.com/root3.json"
        }
      ]
  })"_json;
    UriWrapper fileUri("file://test.json");
    JSONPointer pointer;
    UnresolvedSchema schema(json, fileUri, pointer);
    LinkedSchema buildable(schema, {});
    THEN("It is added to dependencies") {
      auto deps = getDraft07Dependencies(json, fileUri, pointer);
      REQUIRE(deps.size() == 2);
      REQUIRE(deps.count(fileUri.withPointer(pointer / "items" / "0")) == 1);
      REQUIRE(deps.count(fileUri.withPointer(pointer / "items" / "1")) == 1);
    }
  }

  WHEN("Schema has an object of properties") {
    nlohmann::json json = R"(
  {
      "$schema": "http://json-schema.org/draft-07/schema#",
      "title": "Core schema meta-schema",
      "properties": {
        "test": {
          "$id": "http://example.com/root2.json"
        }
      }
  })"_json;
    UriWrapper fileUri("file://test.json");
    JSONPointer pointer;
    UnresolvedSchema schema(json, fileUri, pointer);
    LinkedSchema buildable(schema, {});
    THEN("It is added to dependencies") {
      auto deps = getDraft07Dependencies(json, fileUri, pointer);
      REQUIRE(deps.size() == 1);
      REQUIRE(deps.count(
                  fileUri.withPointer(pointer / "properties" / "test")) == 1);
    }
  }

  WHEN("Schema has additional properties") {
    nlohmann::json json = R"(
  {
      "$schema": "http://json-schema.org/draft-07/schema#",
      "title": "Core schema meta-schema",
      "additionalProperties": {
        "$id": "http://example.com/root2.json"
      }
  })"_json;
    UriWrapper fileUri("file://test.json");
    JSONPointer pointer;
    UnresolvedSchema schema(json, fileUri, pointer);
    LinkedSchema buildable(schema, {});
    THEN("It is added to dependencies") {
      auto deps = getDraft07Dependencies(json, fileUri, pointer);
      REQUIRE(deps.size() == 1);
      REQUIRE(deps.count(
                  fileUri.withPointer(pointer / "additionalProperties")) == 1);
    }
  }
}

SCENARIO("Multi-file Draft07 reference resolution", "[Draft07][LinkedSchema]") {
  GIVEN("Two schemas with a reference between them") {
    nlohmann::json json1 = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "Core schema meta-schema",
    "properties": {
      "test": {
        "$ref": "file://test2.json"
      }
    }
})"_json;
    UriWrapper fileUri1("file://test.json");
    JSONPointer pointer1;
    UnresolvedSchema schema1(json1, fileUri1, pointer1);
    UnresolvedSchema schema1test(json1.at("properties").at("test"), fileUri1,
                                 pointer1 / "properties" / "test");

    nlohmann::json json2 = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "Core schema meta-schema"
})"_json;
    UriWrapper fileUri2("file://test2.json");
    JSONPointer pointer2;
    UnresolvedSchema schema2(json2, fileUri2, pointer2);

    SetMap<UriWrapper, UnresolvedSchema> setMap;
    setMap.bulkInsert({fileUri1}, std::move(schema1));
    setMap.bulkInsert({fileUri1.withPointer(pointer1 / "properties" / "test")},
                      std::move(schema1test));
    setMap.bulkInsert({fileUri2}, std::move(schema2));

    WHEN("Resolving the references") {
      std::set<UriWrapper> refs{fileUri1};
      auto resolved = resolveDependencies(std::move(setMap), refs);
      THEN("The reference is resolved") {
        REQUIRE(resolved.size() == 3);
        const auto arrayHasSchema = [&resolved](const UriWrapper& uri) {
          return std::any_of(
              resolved.begin(), resolved.end(),
              [&uri](const std::unique_ptr<LinkedSchema>& schema) {
                return schema->baseUri_.withPointer(schema->pointer_) == uri;
              });
        };
        CAPTURE(fileUri1.toString().value(),
                resolved[0]->baseUri_.toString().value(),
                resolved[0]->pointer_.toString(),
                resolved[0]
                    ->baseUri_.withPointer(resolved[0]->pointer_)
                    .toString()
                    .value(),
                resolved[1]->baseUri_.toString().value(),
                resolved[1]->pointer_.toString(),
                resolved[1]
                    ->baseUri_.withPointer(resolved[1]->pointer_)
                    .toString()
                    .value(),
                resolved[2]->baseUri_.toString().value(),
                resolved[2]->pointer_.toString(),
                resolved[2]
                    ->baseUri_.withPointer(resolved[2]->pointer_)
                    .toString()
                    .value());

        REQUIRE(arrayHasSchema(fileUri1));
        REQUIRE(arrayHasSchema(
            fileUri1.withPointer(pointer1 / "properties" / "test")));
        REQUIRE(arrayHasSchema(fileUri2));
      }
    }
  }
}

SCENARIO("Full pipeline run up to LinkedSchema",
         "[Draft07][filesystem][LinkedSchema][DraftRecognisedDocument]["
         "Document]") {
  std::vector<std::filesystem::path> files{"samples/document_1.json",
                                           "samples/document_2.json"};
  auto documents = loadDocuments(files);
  auto recognised = performDraftRecognition(std::move(documents));
  auto unresolvedSchecmas =
      UnresolvedSchema::generateSetMap(std::move(recognised));
  std::set<UriWrapper> refs;
  refs.insert(UriWrapper(
      "file://" +
      std::filesystem::absolute("samples/document_2.json").string()));
  auto resolved = resolveDependencies(std::move(unresolvedSchecmas), refs);
  THEN("The references are resolved") {
    for (const auto& schema : resolved) {
      std::cout << schema->baseUri_.withPointer(schema->pointer_) << std::endl;
    }
    REQUIRE(resolved.size() == 4);
    const auto uri = GENERATE(
        UriWrapper(
            "file://" +
            std::filesystem::absolute("samples/document_1.json").string()),
        UriWrapper(
            "file://" +
            std::filesystem::absolute("samples/document_2.json").string()),
        UriWrapper(
            "file://" +
            std::filesystem::absolute("samples/document_2.json").string())
            .withPointer(JSONPointer() / "properties" / "name"),
        UriWrapper(
            "file://" +
            std::filesystem::absolute("samples/document_2.json").string())
            .withPointer(JSONPointer() / "properties" / "ref"));
    CAPTURE(uri);
    REQUIRE(std::any_of(resolved.begin(), resolved.end(),
                        [uri](const std::unique_ptr<LinkedSchema>& schema) {
                          return schema->baseUri_.withPointer(
                                     schema->pointer_) == uri;
                        }));

    resolved.clear();
  }
}
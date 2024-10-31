#include "ResourceIndex.h"
#include <catch2/catch_all.hpp>

TEST_CASE("ResourceIndex correctly creates object schemas", "[ResourceIndex]")
{
  ResourceIndex index;
  nlohmann::json json = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "$id": "file:///myfile",
    "title": "MyThing",
    "type": "object",
    "properties": {
        "foo": {
            "type": "string"
        },
        "bar": {
            "type": "number"
        }
    }
}
)"_json;

  index.addResource(json, {}, "file:///root.json");
  index.markForBuild("file:///myfile");
  index.build();
  index.generateUniqueSchemaNames();
  index.resolveReferences();
  REQUIRE(index["file:///myfile"]->schema->getIdentifier().value() ==
          "MyThing");
  REQUIRE(index["file:///myfile"]->schema->getTypeName() == "MyThing::Object");
  //   std::cout << (*index["file:///myfile"])->schema->generateDefinition()
  //             << std::endl;
}
#include "ResourceIndex.h"
#include "UriWrapper.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

int main()
{
  nlohmann::json schema = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "title": "Sample Schema",
    "type": "object",
    "properties": {
        "stringType": {
            "type": "string"
        },
        "numberType": {
            "type": "number"
        },
        "integerType": {
            "type": "integer"
        },
        "booleanType": {
            "type": "boolean"
        },
        "arrayType": {
            "type": "array",
            "items": {
                "type": "string"
            }
        },
        "objectType": {
            "type": "object",
            "properties": {
                "property1": {
                    "type": "string"
                },
                "property2": {
                    "type": "number"
                }
            },
            "required": [
                "property1",
                "property2"
            ]
        },
        "nullType": {
            "type": "null"
        }
    },
    "required": [
        "stringType",
        "numberType",
        "integerType",
        "booleanType",
        "arrayType",
        "objectType",
        "nullType"
    ]
})"_json;

  ResourceIndex index;
  index.addResource(schema, {"file:///root.json"}, "file:///root.json");
  for (auto &[uri, resource] : index)
  {
    std::cout << uri << ":" << std::endl
              << (*resource)->json.dump(2) << std::endl;
  }
  index.markForBuild("file:///root.json");
  index.build();
  index.generateUniqueSchemaNames();
  index.resolveReferences();
  // std::ofstream out("myfile.cpp");

  // out << index.generateDefinition(**index["file:///myfile"]);
  // std::cout << index.generateDefinition(**index["file:///myfile"]) << std::endl;
  // out.close();
  // // std::cout << (*index["file:///myfile"])->schema->generateDefinition()
  // //           << std::endl;

  // std::cout << (*index["file:///myfile"])->schema->getTypeName() << std::endl;

  index.generateResources("./outDir");
}
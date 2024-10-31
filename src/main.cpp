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
    "$id": "http://json-schema.org/draft-07/schema#",
    "title": "Core schema meta-schema",
    "properties": {
      "test": {
        "#ref": "#"
      }
    }
})"_json;

  ResourceIndex index;
  index.addResource(schema, {"file:///root.json"}, "file:///root.json");
  for (auto &[uri, resource] : index)
  {
    std::cout << uri << ":" << std::endl
              << resource->json.dump(2) << std::endl;
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
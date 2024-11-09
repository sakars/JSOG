#include "SyncedSchema.h"
#include "UriWrapper.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

// Note: This is the general class flow for the code generation.
// 1. Document - Open a file and read the JSON content into a nlohmann::json
// 2. DraftRecognisedDocument - Identifies the draft version of the JSON schema
// 3. UnresolvedSchema - Represents the immediate JSON object in the document
// 4. LinkedSchema - Represents a schema with all dependencies resolved and
// known.
// 5. IdentifiableSchema - Represents a schema with it's unique identifier.
// 6. DraftXSchemaInterpreter - Interprets the schema according to the draft
// version and outputs an IndexedSyncedSchema.
// 7. IndexedSyncedSchema - Represents a schema that is ready to be generated
// but links aren't connected directly yet.
// 8. SyncedSchema - Represents a schema that is ready to be generated and
// linked to other schemas directly.
// 9. Header and Source files are generated for each SyncedSchema.

int main() {
  SyncedSchema subschemaS("Subs");
  SyncedSchema subschema1("Sub1");
  subschemaS.ref_ = std::ref(subschema1);
  subschema1.properties_.emplace("a", std::ref(subschemaS));
  subschema1.properties_.emplace("b", std::ref(subschemaS));
  subschema1.required_ = {"a"};
  subschema1.minProperties_ = 1;
  subschema1.maxProperties_ = 4;
  subschema1.default_ = "{\"a\": true}"_json;
  SyncedSchema subschema2("Sub2");
  subschema2.type_ = {IndexedSyncedSchema::Type::Boolean};
  SyncedSchema subschema3("Sub3");
  // subschema3.type_ = {SyncedSchema::Type::Number};
  SyncedSchema schema("Test");

  schema.type_ = std::set<IndexedSyncedSchema::Type>();
  schema.type_->insert(IndexedSyncedSchema::Type::Object);
  schema.type_->insert(IndexedSyncedSchema::Type::Array);
  schema.type_->insert(IndexedSyncedSchema::Type::Integer);
  schema.tupleableItems_ = {std::ref(subschema1), std::ref(subschema2)};
  schema.items_ = std::ref(subschema3);
  schema.properties_.emplace("test", std::ref(subschemaS));
  schema.const_ = R"(
{
  "test": "test",
  "test2": "test2",
  "test3": [1, 2, 3],
  "test4": {
    "test5": "test5"
  }
})"_json;

  std::ofstream header(schema.getHeaderFileName());
  header << schema.generateDeclaration().str();
  header.close();
  std::ofstream source(schema.getSourceFileName());
  source << schema.generateDefinition().str();
  source.close();
  std::ofstream sub1header(subschema1.getHeaderFileName());
  sub1header << subschema1.generateDeclaration().str();
  sub1header.close();
  std::ofstream sub1source(subschema1.getSourceFileName());
  sub1source << subschema1.generateDefinition().str();
  sub1source.close();
  std::ofstream sub2header(subschema2.getHeaderFileName());
  sub2header << subschema2.generateDeclaration().str();
  sub2header.close();
  std::ofstream sub2source(subschema2.getSourceFileName());
  sub2source << subschema2.generateDefinition().str();
  sub2source.close();
  std::ofstream sub3header(subschema3.getHeaderFileName());
  sub3header << subschema3.generateDeclaration().str();
  sub3header.close();
  std::ofstream sub3source(subschema3.getSourceFileName());
  sub3source << subschema3.generateDefinition().str();
  sub3source.close();
  std::ofstream subsheader(subschemaS.getHeaderFileName());
  subsheader << subschemaS.generateDeclaration().str();
  subsheader.close();
  std::ofstream subssource(subschemaS.getSourceFileName());
  subssource << subschemaS.generateDefinition().str();
  subssource.close();
  const SyncedSchema& true_ = SyncedSchema::getTrueSchema();
  std::ofstream trueheader(true_.getHeaderFileName());
  trueheader << true_.generateDeclaration().str();
  trueheader.close();
  std::ofstream truesource(true_.getSourceFileName());
  truesource << true_.generateDefinition().str();
  truesource.close();

  //   nlohmann::json schema = R"(
  // {
  //     "$schema": "http://json-schema.org/draft-07/schema#",
  //     "$id": "http://json-schema.org/draft-07/schema#",
  //     "title": "Core schema meta-schema",
  //     "properties": {
  //       "test": {
  //         "#ref": "#"
  //       }
  //     }
  // })"_json;

  // ResourceIndex index;
  // index.addResource(schema, {"file:///root.json"}, "file:///root.json");
  // for (auto &[uri, resource] : index)
  // {
  //   std::cout << uri << ":" << std::endl
  //             << resource->json.dump(2) << std::endl;
  // }
  // index.markForBuild("file:///root.json");
  // index.build();
  // index.generateUniqueSchemaNames();
  // index.resolveReferences();
  // std::ofstream out("myfile.cpp");

  // out << index.generateDefinition(**index["file:///myfile"]);
  // std::cout << index.generateDefinition(**index["file:///myfile"]) <<
  // std::endl; out.close();
  // // std::cout << (*index["file:///myfile"])->schema->generateDefinition()
  // //           << std::endl;

  // std::cout << (*index["file:///myfile"])->schema->getTypeName() <<
  // std::endl;

  // index.generateResources("./outDir");
}
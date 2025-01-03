
#include "Document.h"
#include "DraftInterpreter.h"
#include "DraftRecognisedDocument.h"
#include "IdentifiableSchema.h"
#include "IndexedSyncedSchema.h"
#include "LinkedSchema.h"
#include "SchemaResource.h"
#include "SyncedSchema.h"

#include <catch2/catch_all.hpp>
#include <nlohmann/json.hpp>
#include <vector>

TEST_CASE("Self referencial code generation doesn't crash", "[selfref]") {
  auto selfRef = R"(
{
    "$id": "jsog://SelfRef",
    "title": "SelfRef",
    "properties": {
      "object": {
        "$ref": "#"
      }
    }
})"_json;
  std::vector<DraftRecognisedDocument> documents = {
      DraftRecognisedDocument(Document(std::move(selfRef), "jsog://SelfRef"))};
  auto schemaResources = SchemaResource::generateSetMap(documents);
  REQUIRE(schemaResources.getSet().size() ==
          3); // the schema, properties, object
  auto linkedSchemas = resolveDependencies(
      std::move(schemaResources), std::set<UriWrapper>{"jsog://SelfRef"});
  REQUIRE(linkedSchemas.size() == 2);
  auto identifiableSchemas =
      IdentifiableSchema::transition(std::move(linkedSchemas));
  REQUIRE(identifiableSchemas.size() == 2);
  auto indexedSyncedSchemas = interpretSchemas(identifiableSchemas);
  REQUIRE(indexedSyncedSchemas.size() == 2);
  auto syncedSchemas =
      SyncedSchema::resolveIndexedSchema(std::move(indexedSyncedSchemas));
  SUCCEED();
}
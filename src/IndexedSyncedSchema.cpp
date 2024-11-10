#include "IndexedSyncedSchema.h"
#include <fstream>
#include <vector>

void IndexedSyncedSchema::dumpSchemas(
    const std::vector<IndexedSyncedSchema>& indexedSyncedSchemas) {

  auto isSchema = nlohmann::json::array();
  for (const auto& indexedshcema : indexedSyncedSchemas) {
    auto isDump = nlohmann::json::object();
    isDump["identifier"] = indexedshcema.identifier_;

    if (indexedshcema.definedAsBooleanSchema_.has_value()) {
      isDump["definedAsBooleanSchema"] =
          indexedshcema.definedAsBooleanSchema_.value();
    }

    if (indexedshcema.ref_.has_value()) {
      isDump["ref"] = indexedshcema.ref_.value();
    }

    if (indexedshcema.type_.has_value()) {
      auto& typeDump = isDump["type"];
      for (const auto& type : indexedshcema.type_.value()) {
        switch (type) {
        case IndexedSyncedSchema::Type::Array:
          typeDump.push_back("array");
          break;
        case IndexedSyncedSchema::Type::Boolean:
          typeDump.push_back("boolean");
          break;
        case IndexedSyncedSchema::Type::Integer:
          typeDump.push_back("integer");
          break;
        case IndexedSyncedSchema::Type::Null:
          typeDump.push_back("null");
          break;
        case IndexedSyncedSchema::Type::Number:
          typeDump.push_back("number");
          break;
        case IndexedSyncedSchema::Type::Object:
          typeDump.push_back("object");
          break;
        case IndexedSyncedSchema::Type::String:
          typeDump.push_back("string");
          break;
        default:
          break;
        }
      }
    }

    if (indexedshcema.enum_.has_value()) {
      isDump["enum"] = indexedshcema.enum_.value();
    }

    if (indexedshcema.const_.has_value()) {
      isDump["const"] = indexedshcema.const_.value();
    }

    if (indexedshcema.description_.has_value()) {
      isDump["description"] = indexedshcema.description_.value();
    }

    if (indexedshcema.multipleOf_.has_value()) {
      isDump["multipleOf"] = indexedshcema.multipleOf_.value();
    }

    if (indexedshcema.maximum_.has_value()) {
      isDump["maximum"] = indexedshcema.maximum_.value();
    }

    if (indexedshcema.exclusiveMaximum_.has_value()) {
      isDump["exclusiveMaximum"] = indexedshcema.exclusiveMaximum_.value();
    }

    if (indexedshcema.minimum_.has_value()) {
      isDump["minimum"] = indexedshcema.minimum_.value();
    }

    if (indexedshcema.exclusiveMinimum_.has_value()) {
      isDump["exclusiveMinimum"] = indexedshcema.exclusiveMinimum_.value();
    }

    if (indexedshcema.maxLength_.has_value()) {
      isDump["maxLength"] = indexedshcema.maxLength_.value();
    }

    if (indexedshcema.minLength_.has_value()) {
      isDump["minLength"] = indexedshcema.minLength_.value();
    }

    if (indexedshcema.pattern_.has_value()) {
      isDump["pattern"] = indexedshcema.pattern_.value();
    }

    if (indexedshcema.tupleableItems_.has_value()) {
      auto& tupleableItemsDump = isDump["tupleableItems"];
      for (const auto& tupleableItem : indexedshcema.tupleableItems_.value()) {
        tupleableItemsDump.push_back(tupleableItem);
      }
    }

    if (indexedshcema.items_.has_value()) {
      isDump["items"] = indexedshcema.items_.value();
    }

    if (indexedshcema.maxItems_.has_value()) {
      isDump["maxItems"] = indexedshcema.maxItems_.value();
    }

    if (indexedshcema.minItems_.has_value()) {
      isDump["minItems"] = indexedshcema.minItems_.value();
    }

    if (indexedshcema.uniqueItems_.has_value()) {
      isDump["uniqueItems"] = indexedshcema.uniqueItems_.value();
    }

    if (indexedshcema.contains_.has_value()) {
      isDump["contains"] = indexedshcema.contains_.value();
    }

    if (indexedshcema.maxProperties_.has_value()) {
      isDump["maxProperties"] = indexedshcema.maxProperties_.value();
    }

    if (indexedshcema.minProperties_.has_value()) {
      isDump["minProperties"] = indexedshcema.minProperties_.value();
    }

    if (indexedshcema.required_.has_value()) {
      auto& requiredDump = isDump["required"];
      for (const auto& required : indexedshcema.required_.value()) {
        requiredDump.push_back(required);
      }
    }

    auto& propertiesDump = isDump["properties"];
    for (const auto& [key, value] : indexedshcema.properties_) {
      propertiesDump[key] = value;
    }

    if (indexedshcema.patternProperties_.has_value()) {
      auto& patternPropertiesDump = isDump["patternProperties"];
      for (const auto& [key, value] :
           indexedshcema.patternProperties_.value()) {
        patternPropertiesDump[key] = value;
      }
    }

    if (indexedshcema.additionalProperties_.has_value()) {
      isDump["additionalProperties"] =
          indexedshcema.additionalProperties_.value();
    }

    if (indexedshcema.propertyDependencies_.has_value()) {
      auto& propertyDependenciesDump = isDump["propertyDependencies"];
      for (const auto& [key, value] :
           indexedshcema.propertyDependencies_.value()) {
        propertyDependenciesDump[key] = value;
      }
    }

    if (indexedshcema.schemaDependencies_.has_value()) {
      auto& schemaDependenciesDump = isDump["schemaDependencies"];
      for (const auto& [key, value] :
           indexedshcema.schemaDependencies_.value()) {
        schemaDependenciesDump[key] = value;
      }
    }

    if (indexedshcema.propertyNames_.has_value()) {
      isDump["propertyNames"] = indexedshcema.propertyNames_.value();
    }

    if (indexedshcema.if_.has_value()) {
      isDump["if"] = indexedshcema.if_.value();
    }

    if (indexedshcema.then_.has_value()) {
      isDump["then"] = indexedshcema.then_.value();
    }

    if (indexedshcema.else_.has_value()) {
      isDump["else"] = indexedshcema.else_.value();
    }

    if (indexedshcema.allOf_.has_value()) {
      auto& allOfDump = isDump["allOf"];
      for (const auto& allOf : indexedshcema.allOf_.value()) {
        allOfDump.push_back(allOf);
      }
    }

    if (indexedshcema.anyOf_.has_value()) {
      auto& anyOfDump = isDump["anyOf"];
      for (const auto& anyOf : indexedshcema.anyOf_.value()) {
        anyOfDump.push_back(anyOf);
      }
    }

    if (indexedshcema.oneOf_.has_value()) {
      auto& oneOfDump = isDump["oneOf"];
      for (const auto& oneOf : indexedshcema.oneOf_.value()) {
        oneOfDump.push_back(oneOf);
      }
    }

    if (indexedshcema.not_.has_value()) {
      isDump["not"] = indexedshcema.not_.value();
    }

    if (indexedshcema.format_.has_value()) {
      switch (indexedshcema.format_.value()) {
      case IndexedSyncedSchema::Format::Date:
        isDump["format"] = "date";
        break;
      case IndexedSyncedSchema::Format::Time:
        isDump["format"] = "time";
        break;
      case IndexedSyncedSchema::Format::Email:
        isDump["format"] = "email";
        break;
      case IndexedSyncedSchema::Format::IdnEmail:
        isDump["format"] = "idn-email";
        break;
      case IndexedSyncedSchema::Format::Hostname:
        isDump["format"] = "hostname";
        break;
      case IndexedSyncedSchema::Format::IdnHostname:
        isDump["format"] = "idn-hostname";
        break;
      case IndexedSyncedSchema::Format::IpV4:
        isDump["format"] = "ipv4";
        break;
      case IndexedSyncedSchema::Format::IpV6:
        isDump["format"] = "ipv6";
        break;
      case IndexedSyncedSchema::Format::Uri:
        isDump["format"] = "uri";
        break;
      case IndexedSyncedSchema::Format::UriReference:
        isDump["format"] = "uri-reference";
        break;
      case IndexedSyncedSchema::Format::Iri:
        isDump["format"] = "iri";
        break;
      case IndexedSyncedSchema::Format::IriReference:
        isDump["format"] = "iri-reference";
        break;
      case IndexedSyncedSchema::Format::UriTemplate:
        isDump["format"] = "uri-template";
        break;
      case IndexedSyncedSchema::Format::JsonPointer:
        isDump["format"] = "json-pointer";
        break;
      case IndexedSyncedSchema::Format::RelativeJsonPointer:
        isDump["format"] = "relative-json-pointer";
        break;
      case IndexedSyncedSchema::Format::Regex:
        isDump["format"] = "regex";
        break;
      default:
        break;
      }
    }

    if (indexedshcema.default_.has_value()) {
      isDump["default"] = indexedshcema.default_.value();
    }

    if (indexedshcema.readOnly_.has_value()) {
      isDump["readOnly"] = indexedshcema.readOnly_.value();
    }

    if (indexedshcema.writeOnly_.has_value()) {
      isDump["writeOnly"] = indexedshcema.writeOnly_.value();
    }

    if (indexedshcema.examples_.has_value()) {
      auto& examplesDump = isDump["examples"];
      for (const auto& example : indexedshcema.examples_.value()) {
        examplesDump.push_back(example);
      }
    }

    isSchema.push_back(isDump);
  }
  std::ofstream isDumpFile("indexedSynced.dump.json");
  isDumpFile << isSchema.dump(2);
  isDumpFile.close();
}

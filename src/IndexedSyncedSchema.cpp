#include "IndexedSyncedSchema.h"
#include <fstream>
#include <vector>

void IndexedSyncedSchema::dumpSchemas(
    const std::vector<IndexedSyncedSchema>& indexedSyncedSchemas,
    std::filesystem::path outputDirectory) {

  auto isSchema = nlohmann::json::array();
  for (const auto& indexedshcema : indexedSyncedSchemas) {
    auto isDump = nlohmann::json::object();
    isDump["identifier"] = indexedshcema.identifier_;

    if (indexedshcema.definedAsBooleanSchema_.has_value()) {
      isDump["definedAsBooleanSchema"] =
          indexedshcema.definedAsBooleanSchema_.value();
    }

    if (indexedshcema.reinterpretables_.ref_.has_value()) {
      isDump["ref"] = indexedshcema.reinterpretables_.ref_.value();
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

    if (indexedshcema.numberProperties_.multipleOf_.has_value()) {
      isDump["multipleOf"] =
          indexedshcema.numberProperties_.multipleOf_.value();
    }

    if (indexedshcema.numberProperties_.maximum_.has_value()) {
      isDump["maximum"] = indexedshcema.numberProperties_.maximum_.value();
    }

    if (indexedshcema.numberProperties_.exclusiveMaximum_.has_value()) {
      isDump["exclusiveMaximum"] =
          indexedshcema.numberProperties_.exclusiveMaximum_.value();
    }

    if (indexedshcema.numberProperties_.minimum_.has_value()) {
      isDump["minimum"] = indexedshcema.numberProperties_.minimum_.value();
    }

    if (indexedshcema.numberProperties_.exclusiveMinimum_.has_value()) {
      isDump["exclusiveMinimum"] =
          indexedshcema.numberProperties_.exclusiveMinimum_.value();
    }

    if (indexedshcema.stringProperties_.maxLength_.has_value()) {
      isDump["maxLength"] = indexedshcema.stringProperties_.maxLength_.value();
    }

    if (indexedshcema.stringProperties_.minLength_.has_value()) {
      isDump["minLength"] = indexedshcema.stringProperties_.minLength_.value();
    }

    if (indexedshcema.stringProperties_.pattern_.has_value()) {
      isDump["pattern"] = indexedshcema.stringProperties_.pattern_.value();
    }

    if (indexedshcema.arrayProperties_.tupleableItems_.has_value()) {
      auto& tupleableItemsDump = isDump["tupleableItems"];
      for (const auto& tupleableItem :
           indexedshcema.arrayProperties_.tupleableItems_.value()) {
        tupleableItemsDump.push_back(tupleableItem);
      }
    }

    if (indexedshcema.arrayProperties_.items_.has_value()) {
      isDump["items"] = indexedshcema.arrayProperties_.items_.value();
    }

    if (indexedshcema.arrayProperties_.maxItems_.has_value()) {
      isDump["maxItems"] = indexedshcema.arrayProperties_.maxItems_.value();
    }

    if (indexedshcema.arrayProperties_.minItems_.has_value()) {
      isDump["minItems"] = indexedshcema.arrayProperties_.minItems_.value();
    }

    if (indexedshcema.arrayProperties_.uniqueItems_.has_value()) {
      isDump["uniqueItems"] =
          indexedshcema.arrayProperties_.uniqueItems_.value();
    }

    if (indexedshcema.arrayProperties_.contains_.has_value()) {
      isDump["contains"] = indexedshcema.arrayProperties_.contains_.value();
    }

    if (indexedshcema.objectProperties_.maxProperties_.has_value()) {
      isDump["maxProperties"] =
          indexedshcema.objectProperties_.maxProperties_.value();
    }

    if (indexedshcema.objectProperties_.minProperties_.has_value()) {
      isDump["minProperties"] =
          indexedshcema.objectProperties_.minProperties_.value();
    }

    if (indexedshcema.objectProperties_.required_.has_value()) {
      auto& requiredDump = isDump["required"];
      for (const auto& required :
           indexedshcema.objectProperties_.required_.value()) {
        requiredDump.push_back(required);
      }
    }

    auto& propertiesDump = isDump["properties"];
    for (const auto& [key, value] :
         indexedshcema.objectProperties_.properties_) {
      propertiesDump[key] = value;
    }

    if (indexedshcema.objectProperties_.patternProperties_.has_value()) {
      auto& patternPropertiesDump = isDump["patternProperties"];
      for (const auto& [key, value] :
           indexedshcema.objectProperties_.patternProperties_.value()) {
        patternPropertiesDump[key] = value;
      }
    }

    if (indexedshcema.objectProperties_.additionalProperties_.has_value()) {
      isDump["additionalProperties"] =
          indexedshcema.objectProperties_.additionalProperties_.value();
    }

    if (indexedshcema.objectProperties_.propertyDependencies_.has_value()) {
      auto& propertyDependenciesDump = isDump["propertyDependencies"];
      for (const auto& [key, value] :
           indexedshcema.objectProperties_.propertyDependencies_.value()) {
        propertyDependenciesDump[key] = value;
      }
    }

    if (indexedshcema.objectProperties_.schemaDependencies_.has_value()) {
      auto& schemaDependenciesDump = isDump["schemaDependencies"];
      for (const auto& [key, value] :
           indexedshcema.objectProperties_.schemaDependencies_.value()) {
        schemaDependenciesDump[key] = value;
      }
    }

    if (indexedshcema.objectProperties_.propertyNames_.has_value()) {
      isDump["propertyNames"] =
          indexedshcema.objectProperties_.propertyNames_.value();
    }

    if (indexedshcema.reinterpretables_.if_.has_value()) {
      isDump["if"] = indexedshcema.reinterpretables_.if_.value();
    }

    if (indexedshcema.reinterpretables_.then_.has_value()) {
      isDump["then"] = indexedshcema.reinterpretables_.then_.value();
    }

    if (indexedshcema.reinterpretables_.else_.has_value()) {
      isDump["else"] = indexedshcema.reinterpretables_.else_.value();
    }

    if (indexedshcema.reinterpretables_.allOf_.has_value()) {
      auto& allOfDump = isDump["allOf"];
      for (const auto& allOf : indexedshcema.reinterpretables_.allOf_.value()) {
        allOfDump.push_back(allOf);
      }
    }

    if (indexedshcema.reinterpretables_.anyOf_.has_value()) {
      auto& anyOfDump = isDump["anyOf"];
      for (const auto& anyOf : indexedshcema.reinterpretables_.anyOf_.value()) {
        anyOfDump.push_back(anyOf);
      }
    }

    if (indexedshcema.reinterpretables_.oneOf_.has_value()) {
      auto& oneOfDump = isDump["oneOf"];
      for (const auto& oneOf : indexedshcema.reinterpretables_.oneOf_.value()) {
        oneOfDump.push_back(oneOf);
      }
    }

    if (indexedshcema.reinterpretables_.not_.has_value()) {
      isDump["not"] = indexedshcema.reinterpretables_.not_.value();
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
  std::ofstream isDumpFile(outputDirectory / "indexedSynced.dump.json");
  isDumpFile << isSchema.dump(2);
  isDumpFile.close();
}

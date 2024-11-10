#include "Draft07Interpreter.h"
#include <stdexcept>

IndexedSyncedSchema interpretDraft07IdentifiableSchema(
    const IdentifiableSchema& identifiableSchema) {
  const auto& base = identifiableSchema.baseUri_;

  const auto relUriToDependency = [&](const std::string& relUri) {
    UriWrapper uri(relUri);
    UriWrapper uriApplied = UriWrapper::applyUri(base, uri);
    uriApplied.normalize();
    if (!identifiableSchema.dependencies_.contains(uriApplied)) {
      std::cerr << "base: " << base.toString().value() << std::endl;
      std::cerr << "uri: " << uri.toString().value() << std::endl;
      std::cerr << "relUri: " << relUri << std::endl;
      std::cerr << "uriApplied: " << uriApplied.toString().value() << std::endl
                << std::endl;
      for (const auto& [uri, idx] : identifiableSchema.dependencies_) {
        std::cerr << std::format("{}: {}",
                                 uri.toString().value_or("INVALID_URI"), idx)
                  << std::endl;
      }
      throw std::runtime_error(std::format(
          "{} dependency missing from {}", uriApplied.toString().value(),
          identifiableSchema.baseUri_.withPointer(identifiableSchema.pointer_)
              .toString()
              .value()));
    }
    return identifiableSchema.dependencies_.at(uriApplied);
  };

  IndexedSyncedSchema indexedSyncedSchema;
  indexedSyncedSchema.identifier_ = identifiableSchema.identifier_;
  const auto& json = identifiableSchema.json_.get();
  if (json.is_boolean()) {
    indexedSyncedSchema.definedAsBooleanSchema_ = json.get<bool>();
  } else {
    indexedSyncedSchema.definedAsBooleanSchema_ = std::nullopt;
  }

  if (json.contains("$ref")) {
    indexedSyncedSchema.ref_ =
        relUriToDependency(json.at("$ref").get<std::string>());
  }

  if (json.contains("type")) {
    std::set<IndexedSyncedSchema::Type> types;
    for (const auto& type : json.at("type")) {
      if (type == "null") {
        types.insert(IndexedSyncedSchema::Type::Null);
      } else if (type == "boolean") {
        types.insert(IndexedSyncedSchema::Type::Boolean);
      } else if (type == "object") {
        types.insert(IndexedSyncedSchema::Type::Object);
      } else if (type == "array") {
        types.insert(IndexedSyncedSchema::Type::Array);
      } else if (type == "number") {
        types.insert(IndexedSyncedSchema::Type::Number);
      } else if (type == "string") {
        types.insert(IndexedSyncedSchema::Type::String);
      } else if (type == "integer") {
        types.insert(IndexedSyncedSchema::Type::Integer);
      }
    }
    indexedSyncedSchema.type_ = types;
  }

  if (json.contains("enum")) {
    if (json.at("enum").is_array()) {
      indexedSyncedSchema.enum_ = std::vector<nlohmann::json>();
      for (const auto& item : json.at("enum")) {
        indexedSyncedSchema.enum_->push_back(item);
      }
    }
  }

  if (json.contains("const")) {
    indexedSyncedSchema.const_ = json.at("const");
  }

  if (json.contains("description")) {
    indexedSyncedSchema.description_ =
        json.at("description").get<std::string>();
  }

  if (json.contains("multipleOf")) {
    indexedSyncedSchema.multipleOf_ = json.at("multipleOf").get<double>();
  }

  if (json.contains("maximum")) {
    indexedSyncedSchema.maximum_ = json.at("maximum").get<double>();
  }

  if (json.contains("exclusiveMaximum")) {
    indexedSyncedSchema.exclusiveMaximum_ =
        json.at("exclusiveMaximum").get<double>();
  }

  if (json.contains("minimum")) {
    indexedSyncedSchema.minimum_ = json.at("minimum").get<double>();
  }

  if (json.contains("exclusiveMinimum")) {
    indexedSyncedSchema.exclusiveMinimum_ =
        json.at("exclusiveMinimum").get<double>();
  }

  if (json.contains("maxLength")) {
    indexedSyncedSchema.maxLength_ = json.at("maxLength").get<double>();
  }

  if (json.contains("minLength")) {
    indexedSyncedSchema.minLength_ = json.at("minLength").get<double>();
  }

  if (json.contains("pattern")) {
    indexedSyncedSchema.pattern_ = json.at("pattern").get<std::string>();
  }

  if (json.contains("items")) {
    if (json.at("items").is_object()) {
      // UriWrapper itemsUri(json.at("items").at("$ref").get<std::string>());
      // itemsUri.normalize();
      // UriWrapper itemsUriApplied = UriWrapper::applyUri(base, itemsUri);
      // indexedSyncedSchema.items_ =
      //     identifiableSchema.dependencies_.at(itemsUriApplied);
      JSONPointer itemsPointer = identifiableSchema.pointer_ / "items";
      indexedSyncedSchema.items_ =
          relUriToDependency(itemsPointer.toFragment());
    } else if (json.at("items").is_array()) {
      indexedSyncedSchema.tupleableItems_ = std::vector<size_t>();
      for (size_t i = 0; i < json.at("items").size(); i++) {
        // UriWrapper itemUri(
        //     json.at("items").at(i).at("$ref").get<std::string>());
        // itemUri.normalize();
        // UriWrapper itemUriApplied = UriWrapper::applyUri(base, itemUri);
        // indexedSyncedSchema.tupleableItems_->push_back(
        //     identifiableSchema.dependencies_.at(itemUriApplied));
        JSONPointer itemPointer =
            identifiableSchema.pointer_ / "items" / std::to_string(i);
        indexedSyncedSchema.tupleableItems_->push_back(
            relUriToDependency(itemPointer.toFragment()));
      }
      if (json.contains("additionalItems")) {
        indexedSyncedSchema.items_ = relUriToDependency(
            (identifiableSchema.pointer_ / "additionalItems").toFragment());
      }
    }
  }

  if (json.contains("maxItems")) {
    indexedSyncedSchema.maxItems_ = json.at("maxItems").get<size_t>();
  }

  if (json.contains("minItems")) {
    indexedSyncedSchema.minItems_ = json.at("minItems").get<size_t>();
  }

  if (json.contains("uniqueItems")) {
    indexedSyncedSchema.uniqueItems_ = json.at("uniqueItems").get<bool>();
  }

  if (json.contains("contains")) {
    JSONPointer containsPointer = identifiableSchema.pointer_ / "contains";
    indexedSyncedSchema.contains_ =
        relUriToDependency(containsPointer.toFragment());
  }

  if (json.contains("maxProperties")) {
    indexedSyncedSchema.maxProperties_ = json.at("maxProperties").get<size_t>();
  }

  if (json.contains("minProperties")) {
    indexedSyncedSchema.minProperties_ = json.at("minProperties").get<size_t>();
  }

  if (json.contains("required")) {
    indexedSyncedSchema.required_ = std::set<std::string>();
    for (const auto& item : json.at("required")) {
      indexedSyncedSchema.required_->insert(item.get<std::string>());
    }
  }

  if (json.contains("properties")) {
    indexedSyncedSchema.properties_ = std::map<std::string, size_t>();
    for (const auto& [key, value] : json.at("properties").items()) {
      JSONPointer propertyPointer =
          identifiableSchema.pointer_ / "properties" / key;
      indexedSyncedSchema.properties_.emplace(
          key, relUriToDependency(propertyPointer.toFragment()));
    }
  }

  if (json.contains("patternProperties")) {
    indexedSyncedSchema.patternProperties_ = std::map<std::string, size_t>();
    for (const auto& [key, value] : json.at("patternProperties").items()) {
      JSONPointer patternPropertyPointer =
          identifiableSchema.pointer_ / "patternProperties" / key;
      indexedSyncedSchema.patternProperties_->emplace(
          key, relUriToDependency(patternPropertyPointer.toFragment()));
    }
  }

  if (json.contains("additionalProperties")) {
    JSONPointer additionalPropertiesPointer =
        identifiableSchema.pointer_ / "additionalProperties";
    indexedSyncedSchema.additionalProperties_ =
        relUriToDependency(additionalPropertiesPointer.toFragment());
  }

  if (json.contains("dependencies")) {
    indexedSyncedSchema.propertyDependencies_ =
        std::map<std::string, std::vector<std::string>>();
    indexedSyncedSchema.schemaDependencies_ = std::map<std::string, size_t>();
    for (const auto& [key, value] : json.at("dependencies").items()) {
      if (value.is_array()) {
        std::vector<std::string> dependencies;
        for (const auto& item : value) {
          dependencies.push_back(item.get<std::string>());
        }
        indexedSyncedSchema.propertyDependencies_->emplace(key, dependencies);
      } else {
        JSONPointer dependencyPointer =
            identifiableSchema.pointer_ / "dependencies" / key;
        indexedSyncedSchema.schemaDependencies_->emplace(
            key, relUriToDependency(dependencyPointer.toFragment()));
      }
    }
  }

  if (json.contains("propertyNames")) {
    JSONPointer propertyNamesPointer =
        identifiableSchema.pointer_ / "propertyNames";
    indexedSyncedSchema.propertyNames_ =
        relUriToDependency(propertyNamesPointer.toFragment());
  }

  if (json.contains("if")) {
    JSONPointer ifPointer = identifiableSchema.pointer_ / "if";
    indexedSyncedSchema.if_ = relUriToDependency(ifPointer.toFragment());
  }

  if (json.contains("then")) {
    JSONPointer thenPointer = identifiableSchema.pointer_ / "then";
    indexedSyncedSchema.then_ = relUriToDependency(thenPointer.toFragment());
  }

  if (json.contains("else")) {
    JSONPointer elsePointer = identifiableSchema.pointer_ / "else";
    indexedSyncedSchema.else_ = relUriToDependency(elsePointer.toFragment());
  }

  if (json.contains("allOf")) {
    indexedSyncedSchema.allOf_ = std::vector<size_t>();
    for (size_t i = 0; i < json.at("allOf").size(); i++) {
      JSONPointer allOfPointer =
          identifiableSchema.pointer_ / "allOf" / std::to_string(i);
      indexedSyncedSchema.allOf_->push_back(
          relUriToDependency(allOfPointer.toFragment()));
    }
  }

  if (json.contains("anyOf")) {
    indexedSyncedSchema.anyOf_ = std::vector<size_t>();
    for (size_t i = 0; i < json.at("anyOf").size(); i++) {
      JSONPointer anyOfPointer =
          identifiableSchema.pointer_ / "anyOf" / std::to_string(i);
      indexedSyncedSchema.anyOf_->push_back(
          relUriToDependency(anyOfPointer.toFragment()));
    }
  }

  if (json.contains("oneOf")) {
    indexedSyncedSchema.oneOf_ = std::vector<size_t>();
    for (size_t i = 0; i < json.at("oneOf").size(); i++) {
      JSONPointer oneOfPointer =
          identifiableSchema.pointer_ / "oneOf" / std::to_string(i);
      indexedSyncedSchema.oneOf_->push_back(
          relUriToDependency(oneOfPointer.toFragment()));
    }
  }

  if (json.contains("not")) {
    JSONPointer notPointer = identifiableSchema.pointer_ / "not";
    indexedSyncedSchema.not_ = relUriToDependency(notPointer.toFragment());
  }

  if (json.contains("format")) {
    const std::string& format = json.at("format").get<std::string>();
    if (format == "date-time") {
      indexedSyncedSchema.format_ = IndexedSyncedSchema::Format::DateTime;
    } else if (format == "date") {
      indexedSyncedSchema.format_ = IndexedSyncedSchema::Format::Date;
    } else if (format == "time") {
      indexedSyncedSchema.format_ = IndexedSyncedSchema::Format::Time;
    } else if (format == "email") {
      indexedSyncedSchema.format_ = IndexedSyncedSchema::Format::Email;
    } else if (format == "idn-email") {
      indexedSyncedSchema.format_ = IndexedSyncedSchema::Format::IdnEmail;
    } else if (format == "hostname") {
      indexedSyncedSchema.format_ = IndexedSyncedSchema::Format::Hostname;
    } else if (format == "idn-hostname") {
      indexedSyncedSchema.format_ = IndexedSyncedSchema::Format::IdnHostname;
    } else if (format == "ipv4") {
      indexedSyncedSchema.format_ = IndexedSyncedSchema::Format::IpV4;
    } else if (format == "ipv6") {
      indexedSyncedSchema.format_ = IndexedSyncedSchema::Format::IpV6;
    } else if (format == "uri") {
      indexedSyncedSchema.format_ = IndexedSyncedSchema::Format::Uri;
    } else if (format == "uri-reference") {
      indexedSyncedSchema.format_ = IndexedSyncedSchema::Format::UriReference;
    } else if (format == "iri") {
      indexedSyncedSchema.format_ = IndexedSyncedSchema::Format::Iri;
    } else if (format == "iri-reference") {
      indexedSyncedSchema.format_ = IndexedSyncedSchema::Format::IriReference;
    } else if (format == "uri-template") {
      indexedSyncedSchema.format_ = IndexedSyncedSchema::Format::UriTemplate;
    } else if (format == "json-pointer") {
      indexedSyncedSchema.format_ = IndexedSyncedSchema::Format::JsonPointer;
    } else if (format == "relative-json-pointer") {
      indexedSyncedSchema.format_ =
          IndexedSyncedSchema::Format::RelativeJsonPointer;
    } else if (format == "regex") {
      indexedSyncedSchema.format_ = IndexedSyncedSchema::Format::Regex;
    }
  }

  if (json.contains("default")) {
    indexedSyncedSchema.default_ = json.at("default");
  }

  if (json.contains("readOnly")) {
    indexedSyncedSchema.readOnly_ = json.at("readOnly").get<bool>();
  }

  if (json.contains("writeOnly")) {
    indexedSyncedSchema.writeOnly_ = json.at("writeOnly").get<bool>();
  }

  if (json.contains("examples")) {
    indexedSyncedSchema.examples_ = std::vector<nlohmann::json>();
    for (const auto& item : json.at("examples")) {
      indexedSyncedSchema.examples_->push_back(item);
    }
  }

  return indexedSyncedSchema;
}
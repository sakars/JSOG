
#include "Draft07.h"
#include <format>
#include <regex>

std::set<UriWrapper> getDraft07Dependencies(const nlohmann::json& json,
                                            const UriWrapper& baseUri,
                                            const JSONPointer& pointer) {
  std::set<UriWrapper> dependencies;
  if (json.contains("$ref")) {
    auto ref = json.at("$ref").get<std::string>();
    auto refUri = UriWrapper(ref);
    refUri.normalize();
    refUri = UriWrapper::applyUri(baseUri, refUri);
    dependencies.insert(refUri);
  }
  if (json.contains("items")) {
    auto items = json.at("items");
    if (items.is_object() || items.is_boolean()) {
      auto itemPointer = pointer / "items";
      auto itemUri = baseUri.withPointer(itemPointer);
      dependencies.insert(itemUri);
    } else if (items.is_array()) {
      for (size_t i = 0; i < items.size(); i++) {
        auto itemPointer = pointer / "items" / std::to_string(i);
        auto itemUri = baseUri.withPointer(itemPointer);
        dependencies.insert(itemUri);
      }
      if (json.contains("additionalItems")) {
        auto additionalItems = json.at("additionalItems");
        auto additionalItemsPointer = pointer / "additionalItems";
        auto additionalItemsUri = baseUri.withPointer(additionalItemsPointer);
        dependencies.insert(additionalItemsUri);
      }
    }
  }

  if (json.contains("properties")) {
    auto properties = json.at("properties");
    for (auto& [key, value] : properties.items()) {
      auto propertyPointer = pointer / "properties" / key;
      auto propertyUri = baseUri.withPointer(propertyPointer);
      dependencies.insert(propertyUri);
    }
  }
  if (json.contains("additionalProperties")) {
    auto additionalProperties = json.at("additionalProperties");
    if (additionalProperties.is_object() || additionalProperties.is_boolean()) {
      auto additionalPropertiesPointer = pointer / "additionalProperties";
      auto additionalPropertiesUri =
          baseUri.withPointer(additionalPropertiesPointer);
      dependencies.insert(additionalPropertiesUri);
    }
  }
  if (json.contains("patternProperties")) {
    auto patternProperties = json.at("patternProperties");
    for (auto& [key, value] : patternProperties.items()) {
      auto patternPropertyPointer = pointer / "patternProperties" / key;
      auto patternPropertyUri = baseUri.withPointer(patternPropertyPointer);
      dependencies.insert(patternPropertyUri);
    }
  }
  if (json.contains("dependencies")) {
    auto dependenciesJson = json.at("dependencies");
    for (auto& [key, value] : dependenciesJson.items()) {
      auto dependencyPointer = pointer / "dependencies" / key;
      auto dependencyUri = baseUri.withPointer(dependencyPointer);
      dependencies.insert(dependencyUri);
    }
  }
  if (json.contains("propertyNames")) {
    auto propertyNames = json.at("propertyNames");
    auto propertyNamesPointer = pointer / "propertyNames";
    auto propertyNamesUri = baseUri.withPointer(propertyNamesPointer);
    dependencies.insert(propertyNamesUri);
  }
  if (json.contains("contains")) {
    auto contains = json.at("contains");
    auto containsPointer = pointer / "contains";
    auto containsUri = baseUri.withPointer(containsPointer);
    dependencies.insert(containsUri);
  }
  if (json.contains("not")) {
    auto notValue = json.at("not");
    auto notPointer = pointer / "not";
    auto notUri = baseUri.withPointer(notPointer);
    dependencies.insert(notUri);
  }
  if (json.contains("allOf")) {
    auto allOf = json.at("allOf");
    for (size_t i = 0; i < allOf.size(); i++) {
      auto allOfPointer = pointer / "allOf" / std::to_string(i);
      auto allOfUri = baseUri.withPointer(allOfPointer);
      dependencies.insert(allOfUri);
    }
  }
  if (json.contains("anyOf")) {
    auto anyOf = json.at("anyOf");
    for (size_t i = 0; i < anyOf.size(); i++) {
      auto anyOfPointer = pointer / "anyOf" / std::to_string(i);
      auto anyOfUri = baseUri.withPointer(anyOfPointer);
      dependencies.insert(anyOfUri);
    }
  }
  if (json.contains("oneOf")) {
    auto oneOf = json.at("oneOf");
    for (size_t i = 0; i < oneOf.size(); i++) {
      auto oneOfPointer = pointer / "oneOf" / std::to_string(i);
      auto oneOfUri = baseUri.withPointer(oneOfPointer);
      dependencies.insert(oneOfUri);
    }
  }
  if (json.contains("if")) {
    auto ifValue = json.at("if");
    auto ifPointer = pointer / "if";
    auto ifUri = baseUri.withPointer(ifPointer);
    dependencies.insert(ifUri);
  }
  if (json.contains("then")) {
    auto thenValue = json.at("then");
    auto thenPointer = pointer / "then";
    auto thenUri = baseUri.withPointer(thenPointer);
    dependencies.insert(thenUri);
  }
  if (json.contains("else")) {
    auto elseValue = json.at("else");
    auto elsePointer = pointer / "else";
    auto elseUri = baseUri.withPointer(elsePointer);
    dependencies.insert(elseUri);
  }

  if (json.contains("definitions")) {
    auto definitions = json.at("definitions");
    if (definitions.is_object()) {
      for (auto& [key, value] : definitions.items()) {
        auto definitionPointer = pointer / "definitions" / key;
        auto definitionUri = baseUri.withPointer(definitionPointer);
        dependencies.insert(definitionUri);
      }
    }
  }

  return dependencies;
}

std::vector<std::string> issuesWithDraft07Schema(const LinkedSchema& schema) {
  const auto& baseUri = schema.baseUri_;
  const auto& pointer = schema.pointer_;
  const auto location = [&baseUri](const JSONPointer& pointer) {
    return std::format("{} : {} :\t", baseUri.toString().value(),
                       pointer.toString());
  };
  std::vector<std::string> issues;

  const auto& json = schema.json_.get();
  if (json.is_boolean()) {
    return issues;
  }

  if (!json.is_object()) {
    issues.push_back(std::format("{} Schema is not an object nor boolean",
                                 location(pointer)));
    return issues;
  }

  // $id
  if (json.contains("$id")) {
    const auto& id = json.at("$id");
    if (!id.is_string()) {
      issues.push_back(
          std::format("{} Invalid type of property $id: {}, expected string",
                      location(pointer / "$id"), id.dump()));
    } else {
      UriWrapper idUri(id.get<std::string>());
      if (!idUri.isAbsolute()) {
        issues.push_back(std::format(
            "{} Invalid value of property $id: {}, expected absolute URI",
            location(pointer / "$id"), id.get<std::string>()));
      }
      if (idUri.getFragment().has_value() &&
          !idUri.getFragment().value().empty()) {
        issues.push_back(std::format("{} Invalid value of property $id: {}, "
                                     "expected URI without fragment",
                                     location(pointer / "$id"),
                                     id.get<std::string>()));
      }
    }
  }

  // Type
  if (json.contains("type")) {
    const auto& type = json.at("type");
    if (type.is_string()) {
      if (type != "null" && type != "boolean" && type != "object" &&
          type != "array" && type != "number" && type != "string" &&
          type != "integer") {
        issues.push_back(std::format("{} Invalid type value: {}",
                                     location(pointer / "type"),
                                     type.get<std::string>()));
      }
    } else if (type.is_array()) {
      for (size_t i = 0; i < type.size(); i++) {
        const auto& typeValue = type.at(i);
        if (typeValue != "null" && typeValue != "boolean" &&
            typeValue != "object" && typeValue != "array" &&
            typeValue != "number" && typeValue != "string" &&
            typeValue != "integer") {
          issues.push_back(
              std::format("{} Invalid type value: {}",
                          location(pointer / "type" / std::to_string(i)),
                          typeValue.get<std::string>()));
        }
      }
    } else {
      issues.push_back(
          std::format("{} Invalid type value: {}, expected string or array",
                      location(pointer / "type"), type.dump()));
    }
  }

  // Enum
  if (json.contains("enum")) {
    const auto& enumValue = json.at("enum");
    if (!enumValue.is_array()) {
      issues.push_back(
          std::format("{} Invalid type of property enum: {}, expected array",
                      location(pointer / "enum"), enumValue.dump()));
    }
  }

  // Const: no validation needed

  // MultipleOf
  if (json.contains("multipleOf")) {
    const auto& multipleOf = json.at("multipleOf");
    if (!multipleOf.is_number()) {
      issues.push_back(std::format(
          "{} Invalid type of property multipleOf: {}, expected number",
          location(pointer / "multipleOf"), multipleOf.dump()));
    }
  }

  // Maximum
  if (json.contains("maximum")) {
    const auto& maximum = json.at("maximum");
    if (!maximum.is_number()) {
      issues.push_back(std::format(
          "{} Invalid type of property maximum: {}, expected number",
          location(pointer / "maximum"), maximum.dump()));
    }
  }

  // ExclusiveMaximum
  if (json.contains("exclusiveMaximum")) {
    const auto& exclusiveMaximum = json.at("exclusiveMaximum");
    if (!exclusiveMaximum.is_boolean()) {
      issues.push_back(std::format(
          "{} Invalid type of property exclusiveMaximum: {}, expected boolean",
          location(pointer / "exclusiveMaximum"), exclusiveMaximum.dump()));
    }
  }

  // Minimum
  if (json.contains("minimum")) {
    const auto& minimum = json.at("minimum");
    if (!minimum.is_number()) {
      issues.push_back(std::format(
          "{} Invalid type of property minimum: {}, expected number",
          location(pointer / "minimum"), minimum.dump()));
    }
  }

  // ExclusiveMinimum
  if (json.contains("exclusiveMinimum")) {
    const auto& exclusiveMinimum = json.at("exclusiveMinimum");
    if (!exclusiveMinimum.is_boolean()) {
      issues.push_back(std::format(
          "{} Invalid type of property exclusiveMinimum: {}, expected boolean",
          location(pointer / "exclusiveMinimum"), exclusiveMinimum.dump()));
    }
  }

  // MaxLength
  if (json.contains("maxLength")) {
    const auto& maxLength = json.at("maxLength");
    if (!maxLength.is_number_integer()) {
      issues.push_back(std::format(
          "{} Invalid type of property maxLength: {}, expected integer",
          location(pointer / "maxLength"), maxLength.dump()));
    }
  }

  // MinLength
  if (json.contains("minLength")) {
    const auto& minLength = json.at("minLength");
    if (!minLength.is_number_integer()) {
      issues.push_back(std::format(
          "{} Invalid type of property minLength: {}, expected integer",
          location(pointer / "minLength"), minLength.dump()));
    }
  }

  // Pattern
  if (json.contains("pattern")) {
    const auto& pattern = json.at("pattern");
    if (!pattern.is_string()) {
      issues.push_back(std::format(
          "{} Invalid type of property pattern: {}, expected string",
          location(pointer / "pattern"), pattern.dump()));
    }
    // Check if pattern is a valid regex (ECMA 262)
    try {
      std::regex(pattern.get<std::string>(), std::regex_constants::ECMAScript);
    } catch (const std::regex_error& e) {
      issues.push_back(std::format("{} Invalid regex pattern: {}, error: {}",
                                   location(pointer / "pattern"),
                                   pattern.get<std::string>(), e.what()));
    }
  }

  // Items
  if (json.contains("items")) {
    const auto& items = json.at("items");
    if (!items.is_object() && !items.is_array()) {
      issues.push_back(std::format(
          "{} Invalid type of property items: {}, expected object or array",
          location(pointer / "items"), items.dump()));
    }
  }

  // additionalItems: no validation needed as each schema validates only itself

  // MaxItems
  if (json.contains("maxItems")) {
    const auto& maxItems = json.at("maxItems");
    if (!maxItems.is_number_integer()) {
      issues.push_back(std::format(
          "{} Invalid type of property maxItems: {}, expected integer",
          location(pointer / "maxItems"), maxItems.dump()));
    }
    if (maxItems.get<long long>() < 0) {
      issues.push_back(std::format(
          "{} Invalid value of property maxItems: {}, expected non-negative",
          location(pointer / "maxItems"), maxItems.get<long long>()));
    }
  }

  // MinItems
  if (json.contains("minItems")) {
    const auto& minItems = json.at("minItems");
    if (!minItems.is_number_integer()) {
      issues.push_back(std::format(
          "{} Invalid type of property minItems: {}, expected integer",
          location(pointer / "minItems"), minItems.dump()));
    }
    if (minItems.get<long long>() < 0) {
      issues.push_back(std::format(
          "{} Invalid value of property minItems: {}, expected non-negative",
          location(pointer / "minItems"), minItems.get<long long>()));
    }
  }

  // UniqueItems
  if (json.contains("uniqueItems")) {
    const auto& uniqueItems = json.at("uniqueItems");
    if (!uniqueItems.is_boolean()) {
      issues.push_back(std::format(
          "{} Invalid type of property uniqueItems: {}, expected boolean",
          location(pointer / "uniqueItems"), uniqueItems.dump()));
    }
  }

  // Contains: no validation needed as each schema validates only itself

  // MaxProperties
  if (json.contains("maxProperties")) {
    const auto& maxProperties = json.at("maxProperties");
    if (!maxProperties.is_number_integer()) {
      issues.push_back(std::format(
          "{} Invalid type of property maxProperties: {}, expected integer",
          location(pointer / "maxProperties"), maxProperties.dump()));
    }
    if (maxProperties.get<long long>() < 0) {
      issues.push_back(std::format(
          "{} Invalid value of property maxProperties: {}, expected "
          "non-negative",
          location(pointer / "maxProperties"), maxProperties.get<long long>()));
    }
  }

  // MinProperties
  if (json.contains("minProperties")) {
    const auto& minProperties = json.at("minProperties");
    if (!minProperties.is_number_integer()) {
      issues.push_back(std::format(
          "{} Invalid type of property minProperties: {}, expected integer",
          location(pointer / "minProperties"), minProperties.dump()));
    }
    if (minProperties.get<long long>() < 0) {
      issues.push_back(std::format(
          "{} Invalid value of property minProperties: {}, expected "
          "non-negative",
          location(pointer / "minProperties"), minProperties.get<long long>()));
    }
  }

  // Required
  if (json.contains("required")) {
    const auto& required = json.at("required");
    if (!required.is_array()) {
      issues.push_back(std::format(
          "{} Invalid type of property required: {}, expected array",
          location(pointer / "required"), required.dump()));
    }
    for (size_t i = 0; i < required.size(); i++) {
      const auto& requiredItem = required.at(i);
      if (!requiredItem.is_string()) {
        issues.push_back(std::format(
            "{} Invalid type of property required[{}]: {}, expected string",
            location(pointer / "required" / std::to_string(i)), i,
            requiredItem.dump()));
      }
    }
  }

  // Properties
  if (json.contains("properties")) {
    const auto& properties = json.at("properties");
    if (!properties.is_object()) {
      issues.push_back(std::format(
          "{} Invalid type of property properties: {}, expected object",
          location(pointer / "properties"), properties.dump()));
    }
    // Object properties are schemas themselves, no need to validate here
  }

  // PatternProperties
  if (json.contains("patternProperties")) {
    const auto& patternProperties = json.at("patternProperties");
    if (!patternProperties.is_object()) {
      issues.push_back(std::format(
          "{} Invalid type of property patternProperties: {}, expected object",
          location(pointer / "patternProperties"), patternProperties.dump()));
    }
    // Object properties are schemas themselves, no need to validate here
    // But we do need to validate the keys as regex
    for (const auto& [key, _] : patternProperties.items()) {
      try {
        std::regex(key, std::regex_constants::ECMAScript);
      } catch (const std::regex_error& e) {
        issues.push_back(std::format(
            "{} Invalid regex pattern: {}, error: {}",
            location(pointer / "patternProperties" / key), key, e.what()));
      }
    }
  }

  // AdditionalProperties: no validation needed as each schema validates only
  // itself

  // Dependencies
  if (json.contains("dependencies")) {
    const auto& dependencies = json.at("dependencies");
    if (!dependencies.is_object()) {
      issues.push_back(std::format(
          "{} Invalid type of property dependencies: {}, expected object",
          location(pointer / "dependencies"), dependencies.dump()));
    }
    for (const auto& [key, value] : dependencies.items()) {
      if (value.is_array()) {
        for (size_t i = 0; i < value.size(); i++) {
          const auto& dependency = value.at(i);
          if (!dependency.is_string()) {
            issues.push_back(std::format(
                "{} Invalid type of property dependencies[{}][{}]: {}, "
                "expected string",
                location(pointer / "dependencies" / key / std::to_string(i)),
                key, i, dependency.dump()));
          }
        }
      }
      // if the dependency is a schema, it will be validated by itself
    }
  }

  // PropertyNames: no validation needed as each schema validates only itself

  // If: no validation needed as each schema validates only itself

  // Then: no validation needed as each schema validates only itself

  // Else: no validation needed as each schema validates only itself

  // AllOf
  if (json.contains("allOf")) {
    const auto& allOf = json.at("allOf");
    if (!allOf.is_array()) {
      issues.push_back(
          std::format("{} Invalid type of property allOf: {}, expected array",
                      location(pointer / "allOf"), allOf.dump()));
    }
    // Array items are schemas themselves, no need to validate here
  }

  // AnyOf
  if (json.contains("anyOf")) {
    const auto& anyOf = json.at("anyOf");
    if (!anyOf.is_array()) {
      issues.push_back(
          std::format("{} Invalid type of property anyOf: {}, expected array",
                      location(pointer / "anyOf"), anyOf.dump()));
    }
    // Array items are schemas themselves, no need to validate here
  }

  // OneOf
  if (json.contains("oneOf")) {
    const auto& oneOf = json.at("oneOf");
    if (!oneOf.is_array()) {
      issues.push_back(
          std::format("{} Invalid type of property oneOf: {}, expected array",
                      location(pointer / "oneOf"), oneOf.dump()));
    }
    // Array items are schemas themselves, no need to validate here
  }

  // Not: no validation needed as each schema validates only itself

  // Format
  if (json.contains("format")) {
    const auto& format = json.at("format");
    if (!format.is_string()) {
      issues.push_back(
          std::format("{} Invalid type of property format: {}, expected string",
                      location(pointer / "format"), format.dump()));
    }
  }

  // Default: no validation needed

  // definitions
  if (json.contains("definitions")) {
    const auto& definitions = json.at("definitions");
    if (!definitions.is_object()) {
      issues.push_back(std::format(
          "{} Invalid type of property definitions: {}, expected object",
          location(pointer / "definitions"), definitions.dump()));
    }
    // Object properties are schemas themselves, no need to validate here
  }

  return issues;
}

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
    indexedSyncedSchema.maxLength_ = json.at("maxLength").get<size_t>();
  }

  if (json.contains("minLength")) {
    indexedSyncedSchema.minLength_ = json.at("minLength").get<size_t>();
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

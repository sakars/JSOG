
#include "Draft07.h"

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
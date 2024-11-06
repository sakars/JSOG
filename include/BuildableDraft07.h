#include "BuildableSchema.h"

class BuildableDraft07 : public BuildableSchema {
public:
  BuildableDraft07(const UnresolvedSchema& unresolvedSchema)
      : BuildableSchema(unresolvedSchema) {
    dependenciesSet_ = getDependencies();
  }

  std::set<UriWrapper> getDependencies() const override {
    std::set<UriWrapper> dependencies;
    JSONPointer pointer = pointer_;
    if (json_.get().contains("$ref")) {
      auto ref = json_.get().at("$ref").get<std::string>();
      auto refUri = UriWrapper(ref);
      refUri.normalize();
      refUri = UriWrapper::applyUri(baseUri_, refUri);
      dependencies.insert(refUri);
    }
    if (json_.get().contains("items")) {
      auto items = json_.get().at("items");
      if (items.is_object()) {
        auto itemPointer = pointer / "items";
        auto itemUri = baseUri_.withPointer(itemPointer);
        dependencies.insert(itemUri);
      } else if (items.is_array()) {
        for (size_t i = 0; i < items.size(); i++) {
          auto itemPointer = pointer / "items" / std::to_string(i);
          auto itemUri = baseUri_.withPointer(itemPointer);
          dependencies.insert(itemUri);
        }
      }
    }
    if (json_.get().contains("properties")) {
      auto properties = json_.get().at("properties");
      for (auto& [key, value] : properties.items()) {
        auto propertyPointer = pointer / "properties" / key;
        auto propertyUri = baseUri_.withPointer(propertyPointer);
        dependencies.insert(propertyUri);
      }
    }
    if (json_.get().contains("additionalProperties")) {
      auto additionalProperties = json_.get().at("additionalProperties");
      if (additionalProperties.is_object()) {
        auto additionalPropertiesPointer = pointer / "additionalProperties";
        auto additionalPropertiesUri =
            baseUri_.withPointer(additionalPropertiesPointer);
        dependencies.insert(additionalPropertiesUri);
      }
    }
    if (json_.get().contains("patternProperties")) {
      auto patternProperties = json_.get().at("patternProperties");
      for (auto& [key, value] : patternProperties.items()) {
        auto patternPropertyPointer = pointer / "patternProperties" / key;
        auto patternPropertyUri = baseUri_.withPointer(patternPropertyPointer);
        dependencies.insert(patternPropertyUri);
      }
    }
    if (json_.get().contains("dependencies")) {
      auto dependenciesJson = json_.get().at("dependencies");
      for (auto& [key, value] : dependenciesJson.items()) {
        auto dependencyPointer = pointer / "dependencies" / key;
        auto dependencyUri = baseUri_.withPointer(dependencyPointer);
        dependencies.insert(dependencyUri);
      }
    }
    if (json_.get().contains("propertyNames")) {
      auto propertyNames = json_.get().at("propertyNames");
      auto propertyNamesPointer = pointer / "propertyNames";
      auto propertyNamesUri = baseUri_.withPointer(propertyNamesPointer);
      dependencies.insert(propertyNamesUri);
    }
    if (json_.get().contains("contains")) {
      auto contains = json_.get().at("contains");
      auto containsPointer = pointer / "contains";
      auto containsUri = baseUri_.withPointer(containsPointer);
      dependencies.insert(containsUri);
    }
    if (json_.get().contains("const")) {
      auto constValue = json_.get().at("const");
      auto constPointer = pointer / "const";
      auto constUri = baseUri_.withPointer(constPointer);
      dependencies.insert(constUri);
    }
    if (json_.get().contains("enum")) {
      auto enumValue = json_.get().at("enum");
      auto enumPointer = pointer / "enum";
      auto enumUri = baseUri_.withPointer(enumPointer);
      dependencies.insert(enumUri);
    }
    if (json_.get().contains("not")) {
      auto notValue = json_.get().at("not");
      auto notPointer = pointer / "not";
      auto notUri = baseUri_.withPointer(notPointer);
      dependencies.insert(notUri);
    }
    if (json_.get().contains("allOf")) {
      auto allOf = json_.get().at("allOf");
      for (size_t i = 0; i < allOf.size(); i++) {
        auto allOfPointer = pointer / "allOf" / std::to_string(i);
        auto allOfUri = baseUri_.withPointer(allOfPointer);
        dependencies.insert(allOfUri);
      }
    }
    if (json_.get().contains("anyOf")) {
      auto anyOf = json_.get().at("anyOf");
      for (size_t i = 0; i < anyOf.size(); i++) {
        auto anyOfPointer = pointer / "anyOf" / std::to_string(i);
        auto anyOfUri = baseUri_.withPointer(anyOfPointer);
        dependencies.insert(anyOfUri);
      }
    }
    if (json_.get().contains("oneOf")) {
      auto oneOf = json_.get().at("oneOf");
      for (size_t i = 0; i < oneOf.size(); i++) {
        auto oneOfPointer = pointer / "oneOf" / std::to_string(i);
        auto oneOfUri = baseUri_.withPointer(oneOfPointer);
        dependencies.insert(oneOfUri);
      }
    }
    if (json_.get().contains("if")) {
      auto ifValue = json_.get().at("if");
      auto ifPointer = pointer / "if";
      auto ifUri = baseUri_.withPointer(ifPointer);
      dependencies.insert(ifUri);
    }
    if (json_.get().contains("then")) {
      auto thenValue = json_.get().at("then");
      auto thenPointer = pointer / "then";
      auto thenUri = baseUri_.withPointer(thenPointer);
      dependencies.insert(thenUri);
    }
    if (json_.get().contains("else")) {
      auto elseValue = json_.get().at("else");
      auto elsePointer = pointer / "else";
      auto elseUri = baseUri_.withPointer(elsePointer);
      dependencies.insert(elseUri);
    }
    return dependencies;
  }
};
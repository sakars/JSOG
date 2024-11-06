
#include "SyncedSchema.h"
#include <format>

CodeBlock SyncedSchema::generateDeclaration() const {
  CodeBlock block;
  block << std::format("namespace {} {{", identifier);
  block << CodeBlock::inc;
  {
    block << "struct Object {";
    block << CodeBlock::inc;
    {
      for (const auto& [propertyName, schema] : properties_) {
        if (required_.has_value() &&
            required_.value().count(propertyName) > 0) {
          block << std::format("{} {};", schema.get().generateType(),
                               propertyName);
        } else {
          block << std::format("std::optional<{}> {};",
                               schema.get().generateType(), propertyName);
        }
      }
      if (additionalProperties_.has_value()) {
        block << std::format(
            "std::map<std::string, {}> additionalProperties;",
            additionalProperties_.value().get().generateType());
      }
    }
    block << CodeBlock::dec;
    block << "};";
  }
  block << CodeBlock::dec;
  block << std::format("}} // namespace {}", identifier);
  return block;
}

CodeBlock SyncedSchema::generateDefinition() const {}

std::string SyncedSchema::generateType() const {
  if (definedAsBooleanSchema_.has_value()) {
    return definedAsBooleanSchema_.value() ? "nlohmann::json"
                                           : "std::nullptr_t";
  }

  // If the schema is a reference, return the reference type wrapped in a unique
  // pointer, as there might be issues with the type being incomplete.
  if (ref_.has_value()) {
    return std::format("std::unique_ptr<{}>",
                       ref_.value().get().generateType());
  }

  std::set<Type> types;
  if (type_) {
    types.insert(type_.value().begin(), type_.value().end());
  }
  if (types.size() == 0) {
    types.insert(Type::Object);
    types.insert(Type::Null);
    types.insert(Type::Boolean);
    types.insert(Type::Array);
    types.insert(Type::Number);
    types.insert(Type::String);
  }
  if (types.size() == 1) {
    switch (*types.begin()) {
    case Type::Null:
      return "std::monostate";
    case Type::Boolean:
      return "bool";
    case Type::Object:
      return "Object";
    case Type::Array:
      return "std::vector<Array>";
    case Type::Number:
      return "double";
    case Type::String:
      return "std::string";
    }
  } else {
    std::string type = "std::variant<";
    for (auto it = types.begin(); it != types.end(); it++) {
      switch (*it) {
      case Type::Null:
        type += "std::monostate";
        break;
      case Type::Boolean:
        type += "bool";
        break;
      case Type::Object:
        type += "Object";
        break;
      case Type::Array:
        type += "std::vector<Array>";
        break;
      case Type::Number:
        type += "double";
        break;
      case Type::String:
        type += "std::string";
        break;
      }
      if (std::next(it) != types.end()) {
        type += ", ";
      }
    }
    type += ">";
    return type;
  }
}

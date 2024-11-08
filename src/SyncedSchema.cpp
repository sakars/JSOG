
#include "SyncedSchema.h"
#include <cmath>
#include <cstdint>
#include <format>
#include <limits>

CodeBlock SyncedSchema::generateDeclaration() const {
  CodeBlock block;
  block << "#include <optional>";
  block << "#include <variant>";
  block << "#include <vector>";
  block << "#include <string>";
  block << "#include <map>";
  block << "#include <stdexcept>";
  block << "#include <nlohmann/json.hpp>";
  block << std::format("namespace {} {{", identifier_);
  block << "class Object {";
  block << CodeBlock::inc;
  {
    block << CodeBlock::dec << "public:" << CodeBlock::inc;
    for (const auto& [propertyName, schema] : properties_) {
      if (required_.has_value() && required_.value().count(propertyName) > 0) {
        block << std::format("{} {};", schema.get().getType(), propertyName);
      } else {
        block << std::format("std::optional<{}> {};", schema.get().getType(),
                             propertyName);
      }
    }
    if (additionalProperties_.has_value()) {
      block << std::format("std::map<std::string, {}> additionalProperties;",
                           additionalProperties_.value().get().getType());
    }
  }
  block << CodeBlock::dec;
  block << "};";
  block << "";
  block << "class Array {";
  block << CodeBlock::inc;
  {
    // Tupleable item declaration
    block << CodeBlock::dec << "private:" << CodeBlock::inc;
    if (tupleableItems_.has_value()) {
      for (size_t i = 0; i < tupleableItems_->size(); i++) {
        block << std::format("std::optional<{}> item{};",
                             (*tupleableItems_)[i].get().getType(), i);
      }
    }
    if (items_.has_value()) {
      block << std::format("std::vector<{}> items;", items_->get().getType());
    }
    block << CodeBlock::dec << "public:" << CodeBlock::inc;
    block << "template <size_t N>";
    block << "auto get() const {";
    block << CodeBlock::inc;
    {
      if (tupleableItems_.has_value() && tupleableItems_->size() > 0) {
        for (size_t i = 0; i < tupleableItems_->size(); i++) {
          block << std::format("if constexpr(N == {}) {{", i);
          block << CodeBlock::inc;
          {
            block << std::format("if (item{}.has_value()) {{", i);
            block << CodeBlock::inc;
            block << std::format("return item{}.value();", i);
            block << CodeBlock::dec;
            block << "}";
          }
          block << CodeBlock::dec;
          block << CodeBlock::dis;
          block << "} else ";
        }
        block << "{";
        block << CodeBlock::inc;
        if (items_.has_value()) {
          block << std::format("if(N - {} < items.size()) {{",
                               tupleableItems_->size());
          block << CodeBlock::inc;
          {
            block << std::format("return items[N - {}];",
                                 tupleableItems_->size());
          }
          block << CodeBlock::dec;
          block << "}";
        }
        block << CodeBlock::dec;
        block << "}";
      } else if (items_.has_value()) {
        block << "if(N < items.size()) {";
        block << CodeBlock::inc << "return items[N];" << CodeBlock::dec;
        block << "}";
      }
      block << "throw std::range_error(std::string(\"Item \") + "
               "std::to_string(N) + \" out of range\")";
    }
    block << CodeBlock::dec;
    block << "}";
    if (items_.has_value()) {
      block << std::format("inline {} get(size_t n) {{",
                           items_->get().getType())
            << CodeBlock::inc << "if(n >= items.size()) {"
            << "throw std::range_error(\"Item \" + std::to_string(n) + \" out "
               "of range\");"
            << "}"
            << "return items[n];" << CodeBlock::dec << "}";
    }
  }
  block << CodeBlock::dec;
  block << "};";

  block << std::format("std::optional<{}> construct(const nlohmann::json&);",
                       getType());

  block << std::format("using {} = {};", identifier_, getType());

  block << std::format("}} // namespace {}", identifier_);
  return block;
}

CodeBlock SyncedSchema::generateDefinition() const {
  CodeBlock block;
  block << std::format("#include \"{}\"", getHeaderFileName());
  block << std::format("namespace {} {{", identifier_);
  block << std::format(
      "std::optional<{}> construct(const nlohmann::json& json) {{", getType());
  block << CodeBlock::inc;
  {
    // Boolean schema means that the schema accepts either everything or nothing
    if (definedAsBooleanSchema_.has_value()) {
      if (definedAsBooleanSchema_.value()) {
        block << "return json;";

      } else {
        block << "return std::nullopt;";
      }
    }

    // Reference schema means that the schema is a reference to another schema
    if (ref_.has_value()) {
      block << std::format(
          "return std::make_unique<{0}>({0}::construct(json).value());",
          ref_.value().get().identifier_);
    }

    if (const_.has_value()) {
      std::string constValue = const_.value().dump();
      // escape all characters as hex values
      std::string escapedConstValue = "";
      for (char c : constValue) {
        escapedConstValue += std::format("\\x{:02x}", c);
      }
      block << std::format("if(json == \"{}\"_json) {{", escapedConstValue);
      block << CodeBlock::inc;
      block << "return json;";
      block << CodeBlock::dec;
      block << "}";
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
      types.insert(Type::Integer);
    }

    if (types.contains(Type::Null)) {
      block << "if(json.is_null()) {";
      block << CodeBlock::inc;
      block << std::format("return {}();", getNullType());
      block << CodeBlock::dec;
      block << "}";
    }

    if (types.contains(Type::Boolean)) {
      block << "if(json.is_boolean()) {";
      block << CodeBlock::inc;
      block << std::format("return json.get<{}>();", getBooleanType());
      block << CodeBlock::dec;
      block << "}";
    }

    if (types.contains(Type::Integer)) {
      block << "if(json.is_number_integer()) {";
      block << CodeBlock::inc;
      block << std::format("return json.get<{}>();", getIntegerType());
      block << CodeBlock::dec;
      block << "}";
    }

    if (types.contains(Type::Number)) {
      block << "if(json.is_number()) {";
      block << CodeBlock::inc;
      block << std::format("return json.get<{}>();", getNumberType());
      block << CodeBlock::dec;
      block << "}";
    }

    if (types.contains(Type::String)) {
      block << "if(json.is_string()) {";
      block << CodeBlock::inc;
      block << std::format("return json.get<{}>();", getStringType());
      block << CodeBlock::dec;
      block << "}";
    }
  }
  block << CodeBlock::dec;
  block << "}";
  block << std::format("}} // namespace {}", identifier_);

  return block;
}

CodeBlock SyncedSchema::generateDependencies() const {
  CodeBlock block;
  return block;
}

std::string SyncedSchema::getHeaderFileName() const { return filename_ + ".h"; }

std::string SyncedSchema::getSourceFileName() const {
  return filename_ + ".cpp";
}

std::string SyncedSchema::getType() const {
  // std::nullptr_t is a special case, as it should never actually be used.
  // Any construction attempts will always return an empty optional, not a
  // nullptr. More appropriate would have been to use std::nullopt_t, but that
  // would construct an ill-formed program.
  if (definedAsBooleanSchema_.has_value()) {
    return definedAsBooleanSchema_.value() ? "nlohmann::json"
                                           : "std::nullptr_t";
  }

  // If the schema is a reference, return the reference type wrapped in a unique
  // pointer, as there might be issues with the type being incomplete.
  if (ref_.has_value()) {
    return std::format("std::unique_ptr<{}>", ref_.value().get().getType());
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
    types.insert(Type::Integer);
  }
  if (types.size() == 1) {
    switch (*types.begin()) {
    case Type::Null:
      return getNullType();
    case Type::Boolean:
      return getBooleanType();
    case Type::Object:
      return getObjectType();
    case Type::Array:
      return getArrayType();
    case Type::Number:
      return getNumberType();
    case Type::String:
      return getStringType();
    case Type::Integer:
      return getIntegerType();
    }
  } else {
    std::string type = "std::variant<";
    for (auto it = types.begin(); it != types.end(); it++) {
      switch (*it) {
      case Type::Null:
        type += getNullType();
        break;
      case Type::Boolean:
        type += getBooleanType();
        break;
      case Type::Object:
        type += getObjectType();
        break;
      case Type::Array:
        type += getArrayType();
        break;
      case Type::Number:
        type += getNumberType();
        break;
      case Type::String:
        type += getStringType();
        break;
      case Type::Integer:
        type += getIntegerType();
      }
      if (std::next(it) != types.end()) {
        type += ", ";
      }
    }
    type += ">";
    return type;
  }
}

std::string SyncedSchema::getObjectType() const {
  return std::format("::{}::Object", identifier_);
}

std::string SyncedSchema::getArrayType() const {
  return std::format("::{}::Array", identifier_);
}

std::string SyncedSchema::getNumberType() const { return "double"; }

std::string SyncedSchema::getStringType() const { return "std::string"; }

std::string SyncedSchema::getBooleanType() const { return "bool"; }

std::string SyncedSchema::getNullType() const { return "std::monostate"; }

enum class IntegerType {
  INT8,
  UINT8,
  INT16,
  UINT16,
  INT32,
  UINT32,
  INT64,
  UINT64,
  NONE
};

static IntegerType
smallestIntegerType(std::optional<double> minimum,
                    std::optional<double> maximum,
                    std::optional<double> exclusiveMinimum = std::nullopt,
                    std::optional<double> exclusiveMaximum = std::nullopt) {
  // Initialize effective bounds with provided values or defaults
  double adjustedMin = minimum.value_or(std::numeric_limits<int64_t>::min());
  double adjustedMax = maximum.value_or(std::numeric_limits<int64_t>::max());

  // Adjust bounds based on exclusive values if they are set
  if (exclusiveMinimum && *exclusiveMinimum > adjustedMin) {
    adjustedMin = *exclusiveMinimum;
  }
  if (exclusiveMaximum && *exclusiveMaximum < adjustedMax) {
    adjustedMax = *exclusiveMaximum;
  }

  // Convert adjusted bounds to integers
  int64_t minInt = static_cast<int64_t>(
      std::ceil(adjustedMin)); // Round up for inclusive lower bound
  int64_t maxInt = static_cast<int64_t>(
      std::floor(adjustedMax)); // Round down for inclusive upper bound

  // Determine if signed or unsigned types are needed
  bool needsSigned = minInt < 0;

  // Check each type from smallest to largest
  if (needsSigned) {
    if (minInt >= std::numeric_limits<int8_t>::min() &&
        maxInt <= std::numeric_limits<int8_t>::max())
      return IntegerType::INT8;
    if (minInt >= std::numeric_limits<int16_t>::min() &&
        maxInt <= std::numeric_limits<int16_t>::max())
      return IntegerType::INT16;
    if (minInt >= std::numeric_limits<int32_t>::min() &&
        maxInt <= std::numeric_limits<int32_t>::max())
      return IntegerType::INT32;
    if (minInt >= std::numeric_limits<int64_t>::min() &&
        maxInt <= std::numeric_limits<int64_t>::max())
      return IntegerType::INT64;
  } else {
    if (minInt >= 0 && maxInt <= std::numeric_limits<uint8_t>::max())
      return IntegerType::UINT8;
    if (minInt >= 0 && maxInt <= std::numeric_limits<uint16_t>::max())
      return IntegerType::UINT16;
    if (minInt >= 0 && maxInt <= std::numeric_limits<uint32_t>::max())
      return IntegerType::UINT32;
    if (minInt >= 0 && maxInt <= std::numeric_limits<uint64_t>::max())
      return IntegerType::UINT64;
  }

  // Return NONE if no matching type is found
  return IntegerType::NONE;
}

std::string SyncedSchema::getIntegerType() const {
  IntegerType type = smallestIntegerType(minimum_, maximum_, exclusiveMinimum_,
                                         exclusiveMaximum_);

  switch (type) {
  case IntegerType::INT8:
    return "int8_t";
  case IntegerType::UINT8:
    return "uint8_t";
  case IntegerType::INT16:
    return "int16_t";
  case IntegerType::UINT16:
    return "uint16_t";
  case IntegerType::INT32:
    return "int32_t";
  case IntegerType::UINT32:
    return "uint32_t";
  case IntegerType::INT64:
    return "int64_t";
  case IntegerType::UINT64:
    return "uint64_t";
  case IntegerType::NONE:
    return "int64_t";
  }
  return "int64_t";
}

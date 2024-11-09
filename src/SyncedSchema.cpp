
#include "SyncedSchema.h"
#include <cmath>
#include <cstdint>
#include <format>
#include <limits>

#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif

#define JSOG_DEBUG 1
#if JSOG_DEBUG
#define BLOCK                                                                  \
  block << CodeBlock::dis                                                      \
        << std::format("/*{}:{}*/", __FILE_NAME__, __LINE__);                  \
  block
#else
#define BLOCK block
#endif

static std::string getHeaderDefine(const SyncedSchema& schema) {
  std::string headerDefine = schema.getHeaderFileName();
  for (char& c : headerDefine) {
    if (c == '.') {
      c = '_';
    }
  }
  std::transform(headerDefine.begin(), headerDefine.end(), headerDefine.begin(),
                 ::toupper);
  if (schema.codeProperties.get().define_prefix_.has_value()) {
    headerDefine =
        schema.codeProperties.get().define_prefix_.value() + headerDefine;
  }
  return headerDefine;
}

CodeBlock SyncedSchema::generateDeclaration() const {
  CodeBlock block(codeProperties.get().indent_);
  if (codeProperties.get().headerGuardType_ ==
      CodeProperties::HeaderGuard::Ifndef) {
    std::string upperFileName = getHeaderDefine(*this);
    BLOCK << std::format("#ifndef {}", upperFileName);
    BLOCK << std::format("#define {}", upperFileName);
  } else {
    BLOCK << std::format("#pragma once");
  }
  BLOCK << generateDependencies();

  if (codeProperties.get().globalNamespace_.has_value()) {
    BLOCK << std::format("namespace {} {{",
                         codeProperties.get().globalNamespace_.value());
  }

  BLOCK << std::format("namespace {} {{", identifier_);

  // Forward declarations
  BLOCK << "// Forward declarations" << "class Object;" << "class Array;" << ""
        << "class Object {" << CodeBlock::inc;
  {
    BLOCK << CodeBlock::dec << "public:" << CodeBlock::inc;
    for (const auto& [propertyName, schema] : properties_) {
      if (required_.has_value() && required_.value().count(propertyName) > 0) {
        BLOCK << std::format("{} {};", schema.get().getType(), propertyName);
      } else {
        BLOCK << std::format("std::optional<{}> {};", schema.get().getType(),
                             propertyName);
      }
    }
    if (additionalProperties_.has_value()) {
      BLOCK << std::format("std::map<std::string, {}> additionalProperties;",
                           additionalProperties_.value().get().getType());
    }
  }
  BLOCK << CodeBlock::dec << "};" << "" << "class Array {" << CodeBlock::inc;
  {
    // Tupleable item declaration
    BLOCK << CodeBlock::dec << "private:" << CodeBlock::inc;
    if (tupleableItems_.has_value()) {
      for (size_t i = 0; i < tupleableItems_->size(); i++) {
        BLOCK << std::format("std::optional<{}> item{};",
                             (*tupleableItems_)[i].get().getType(), i);
      }
    }
    BLOCK << std::format("std::vector<{}> items;", items_.get().getType());

    BLOCK << CodeBlock::dec << "public:" << CodeBlock::inc;
    BLOCK << "template <size_t N>";
    BLOCK << "auto get() const {";
    BLOCK << CodeBlock::inc;
    {
      if (tupleableItems_.has_value() && tupleableItems_->size() > 0) {
        for (size_t i = 0; i < tupleableItems_->size(); i++) {
          BLOCK << std::format("if constexpr(N == {}) {{", i) << CodeBlock::inc;
          {
            BLOCK << std::format("if (item{}.has_value()) {{", i)
                  << CodeBlock::inc << std::format("return item{}.value();", i)
                  << CodeBlock::dec << "}";
          }
          BLOCK << CodeBlock::dec << CodeBlock::dis << "} else ";
        }
        BLOCK << "{" << CodeBlock::inc
              << std::format("if(N - {} < items.size()) {{",
                             tupleableItems_->size())
              << CodeBlock::inc;
        {
          BLOCK << std::format("return items[N - {}];",
                               tupleableItems_->size());
        }
        BLOCK << CodeBlock::dec << "}" << CodeBlock::dec << "}";
      } else {
        BLOCK << "if(N < items.size()) {" << CodeBlock::inc
              << "return items[N];" << CodeBlock::dec << "}";
      }
      BLOCK << "throw std::range_error(std::string(\"Item \") + "
               "std::to_string(N) + \" out of range\");";
    }
    BLOCK << CodeBlock::dec << "}";
    BLOCK << std::format("inline {} get(size_t n) {{", items_.get().getType())
          << CodeBlock::inc << "if(n >= items.size()) {"
          << "throw std::range_error(\"Item \" + std::to_string(n) + \" out "
             "of range\");"
          << "}"
          << "return items[n];" << CodeBlock::dec << "}";
  }
  BLOCK << CodeBlock::dec;
  BLOCK << "};";

  BLOCK << std::format("using {} = {};", identifier_, getType())
        << std::format("std::optional<{}> construct(const nlohmann::json&);",
                       getType());

  BLOCK << std::format("}} // namespace {}", identifier_);

  if (codeProperties.get().globalNamespace_.has_value()) {
    BLOCK << std::format("}} // namespace {}",
                         codeProperties.get().globalNamespace_.value());
  }

  if (codeProperties.get().headerGuardType_ ==
      CodeProperties::HeaderGuard::Ifndef) {
    BLOCK << std::format("#endif // {}", getHeaderDefine(*this));
  }
  return block;
}

static std::string escapeJSONString(const std::string& str) {
  std::string escaped;
  for (char c : str) {
    escaped += std::format("\\x{:02x}", c);
  }
  return escaped;
}

/// @brief Escapes a JSON object to a string. It does not wrap quotes around it.
/// @param json The JSON object to escape
/// @details In order to not have to deal with something like R"()" to escape
/// the JSON object, this function will escape all
/// @return The escaped JSON object
static std::string escapeJSON(const nlohmann::json& json) {
  return escapeJSONString(json.dump());
}

CodeBlock SyncedSchema::generateDefinition() const {
  CodeBlock block(codeProperties.get().indent_);
  BLOCK << std::format("#include \"{}\"", getHeaderFileName());

  if (codeProperties.get().globalNamespace_.has_value()) {
    BLOCK << std::format("namespace {} {{",
                         codeProperties.get().globalNamespace_.value());
  }

  BLOCK << std::format("namespace {} {{", identifier_)
        << std::format(
               "std::optional<{}> construct(const nlohmann::json& json) {{",
               getType())
        << CodeBlock::inc;
  {
    // Boolean schema means that the schema accepts either everything or nothing
    if (definedAsBooleanSchema_.has_value()) {
      if (definedAsBooleanSchema_.value()) {
        BLOCK << "return json;";

      } else {
        BLOCK << "return std::nullopt;";
      }
    }

    // Reference schema means that the schema is a reference to another schema
    if (ref_.has_value()) {
      std::string namespace_;
      if (codeProperties.get().globalNamespace_.has_value()) {
        namespace_ = "::" + codeProperties.get().globalNamespace_.value();
      }
      namespace_ += std::format("::{}", ref_.value().get().identifier_);
      BLOCK << std::format(
          "return std::make_unique<{0}::{1}>({0}::construct(json).value());",
          namespace_, ref_.value().get().identifier_);
    } else {

      if (const_.has_value()) {
        BLOCK << std::format("if(json != \"{}\"_json) {{",
                             escapeJSON(const_.value()))
              << CodeBlock::inc << "return std::nullopt;" << CodeBlock::dec
              << "}";
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
        BLOCK << "if(json.is_null()) {" << CodeBlock::inc
              << std::format("return {}();", getNullType()) << CodeBlock::dec
              << "}";
      }

      if (types.contains(Type::Boolean)) {
        BLOCK << "if(json.is_boolean()) {" << CodeBlock::inc
              << std::format("return json.get<{}>();", getBooleanType())
              << CodeBlock::dec << "}";
      }

      if (types.contains(Type::Integer)) {
        BLOCK << "if(json.is_number_integer()) {" << CodeBlock::inc
              << std::format("return json.get<{}>();", getIntegerType())
              << CodeBlock::dec << "}";
      }

      if (types.contains(Type::Number)) {
        BLOCK << "if(json.is_number()) {" << CodeBlock::inc
              << std::format("return json.get<{}>();", getNumberType())
              << CodeBlock::dec << "}";
      }

      if (types.contains(Type::String)) {
        BLOCK << "if(json.is_string()) {" << CodeBlock::inc
              << std::format("return json.get<{}>();", getStringType())
              << CodeBlock::dec << "}";
      }

      if (types.contains(Type::Array)) {
        BLOCK << "if(json.is_array()) {";
        {
          Indent _(block);
          BLOCK << "auto array = Array();";
        }
        BLOCK << "}";
      }
    }

    BLOCK << "return std::nullopt;";
  }
  BLOCK << CodeBlock::dec << "}"
        << std::format("}} // namespace {}", identifier_);

  if (codeProperties.get().globalNamespace_.has_value()) {
    BLOCK << std::format("}} // namespace {}",
                         codeProperties.get().globalNamespace_.value());
  }

  return block;
}

CodeBlock SyncedSchema::generateDependencies() const {
  CodeBlock block(codeProperties.get().indent_);
  BLOCK << "// System dependencies"
        << "#include <optional>"
        << "#include <variant>"
        << "#include <vector>"
        << "#include <string>"
        << "#include <map>"
        << "#include <stdexcept>"
        << "#include <nlohmann/json.hpp>";
  std::set<std::string> dependencies{getHeaderFileName()};
  if (ref_.has_value()) {
    dependencies.insert(ref_.value().get().getHeaderFileName());
  }
  for (const auto& item : tupleableItems_.value_or(
           std::vector<std::reference_wrapper<SyncedSchema>>())) {
    dependencies.insert(item.get().getHeaderFileName());
  }
  dependencies.insert(items_.get().getHeaderFileName());
  if (contains_.has_value()) {
    dependencies.insert(contains_.value().get().getHeaderFileName());
  }
  for (const auto& [_, schema] : properties_) {
    dependencies.insert(schema.get().getHeaderFileName());
  }
  if (patternProperties_.has_value()) {
    for (const auto& [_, schema] : patternProperties_.value()) {
      dependencies.insert(schema.get().getHeaderFileName());
    }
  }
  if (additionalProperties_.has_value()) {
    dependencies.insert(
        additionalProperties_.value().get().getHeaderFileName());
  }
  if (schemaDependencies_.has_value()) {
    for (const auto& [_, schema] : schemaDependencies_.value()) {
      dependencies.insert(schema.get().getHeaderFileName());
    }
  }
  if (propertyNames_.has_value()) {
    dependencies.insert(propertyNames_.value().get().getHeaderFileName());
  }
  if (if_.has_value()) {
    dependencies.insert(if_.value().get().getHeaderFileName());
  }
  if (then_.has_value()) {
    dependencies.insert(then_.value().get().getHeaderFileName());
  }
  if (else_.has_value()) {
    dependencies.insert(else_.value().get().getHeaderFileName());
  }
  if (allOf_.has_value()) {
    for (const auto& schema : allOf_.value()) {
      dependencies.insert(schema.get().getHeaderFileName());
    }
  }
  if (anyOf_.has_value()) {
    for (const auto& schema : anyOf_.value()) {
      dependencies.insert(schema.get().getHeaderFileName());
    }
  }
  if (oneOf_.has_value()) {
    for (const auto& schema : oneOf_.value()) {
      dependencies.insert(schema.get().getHeaderFileName());
    }
  }
  if (not_.has_value()) {
    dependencies.insert(not_.value().get().getHeaderFileName());
  }

  BLOCK << "// Local dependencies";

  for (const auto& dependency : dependencies) {
    if (dependency != "") {
      BLOCK << std::format("#include \"{}\"", dependency);
    }
  }

  BLOCK << "// Forward declarations";
  if (codeProperties.get().globalNamespace_.has_value()) {
    BLOCK << std::format("namespace {} {{",
                         codeProperties.get().globalNamespace_.value());
  }
  for (const auto& dependency : dependencies) {
    if (dependency != "") {
      BLOCK << std::format("namespace {} {{",
                           dependency.substr(0, dependency.size() - 2))
            << std::format("class Object;") << std::format("class Array;")
            << std::format("}} // namespace {}",
                           dependency.substr(0, dependency.size() - 2));
    }
  }
  if (codeProperties.get().globalNamespace_.has_value()) {
    BLOCK << std::format("}} // namespace {}",
                         codeProperties.get().globalNamespace_.value());
  }

  return block;
}

std::string SyncedSchema::getHeaderFileName() const {
  if (filename_.empty()) {
    return "";
  }
  return filename_ + ".h";
}

std::string SyncedSchema::getSourceFileName() const {
  if (filename_.empty()) {
    return "";
  }
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
    // if (codeProperties.get().globalNamespace_.has_value()) {
    //   return std::format("std::unique_ptr<::{0}::{1}::{1}>",
    //                      codeProperties.get().globalNamespace_.value(),
    //                      ref_.value().get().identifier_);
    // }
    // return std::format("std::unique_ptr<::{0}::{0}>",
    //                    ref_.value().get().identifier_);
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
  if (codeProperties.get().globalNamespace_.has_value()) {
    return std::format("::{}::{}::Object",
                       codeProperties.get().globalNamespace_.value(),
                       identifier_);
  }
  return std::format("::{}::Object", identifier_);
}

std::string SyncedSchema::getArrayType() const {
  if (codeProperties.get().globalNamespace_.has_value()) {
    return std::format("::{}::{}::Array",
                       codeProperties.get().globalNamespace_.value(),
                       identifier_);
  }
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

static IntegerType smallestIntegerType(std::optional<double> minimum,
                                       std::optional<double> maximum,
                                       std::optional<double> exclusiveMinimum,
                                       std::optional<double> exclusiveMaximum) {
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

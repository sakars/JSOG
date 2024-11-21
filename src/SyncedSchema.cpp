#include "SyncedSchema.h"
#include "StringUtils.h"
#include <cmath>
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <limits>
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif

#define JSOG_DEBUG 0
#if JSOG_DEBUG
static std::string centerPadString(const std::string& s, size_t width) {
  if (s.size() >= width) {
    return s;
  }
  size_t leftPad = (width - s.size()) / 2;
  size_t rightPad = width - s.size() - leftPad;
  return std::string(leftPad, ' ') + s + std::string(rightPad, ' ');
}
#define BLOCK                                                                  \
  {                                                                            \
    block << CodeBlock::dis                                                    \
          << centerPadString(                                                  \
                 std::format("/*{}:{}*/", __FILE_NAME__, __LINE__), 40);       \
  }                                                                            \
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

// Include guard generation

/// @brief Generates the start of the include guard
/// @param schema The schema to generate the include guard for
/// @return The generated code block
static CodeBlock declarationIncludeGuardStart(const SyncedSchema& schema) {
  CodeBlock block(schema.codeProperties.get().indent_);
#if JSOG_DEBUG
  block << "/*" << schema.identifier_ << " include guard start*/";
#endif
  if (schema.codeProperties.get().headerGuardType_ ==
      CodeProperties::HeaderGuard::Ifndef) {
    std::string upperFileName = getHeaderDefine(schema);
    BLOCK << std::format("#ifndef {}", upperFileName);
    BLOCK << std::format("#define {}", upperFileName);
  } else {
    BLOCK << std::format("#pragma once");
  }
  return block;
}

static CodeBlock declarationIncludeGuardEnd(const SyncedSchema& schema) {
  CodeBlock block(schema.codeProperties.get().indent_);
#if JSOG_DEBUG
  block << "/*" << schema.identifier_ << " include guard end*/";
#endif
  if (schema.codeProperties.get().headerGuardType_ ==
      CodeProperties::HeaderGuard::Ifndef) {
    BLOCK << std::format("#endif // {}", getHeaderDefine(schema));
  }
  return block;
}

// Namespace generation

static CodeBlock namespaceStart(const SyncedSchema& schema) {
  CodeBlock block(schema.codeProperties.get().indent_);
#if JSON_DEBUG
  block << "/*" << schema.identifier_ << " namespace start*/";
#endif
  // Global namespace, if set
  if (schema.codeProperties.get().globalNamespace_.has_value()) {
    BLOCK << std::format("namespace {} {{",
                         schema.codeProperties.get().globalNamespace_.value());
  }
  // Identifier namespace
  BLOCK << std::format("namespace {} {{", schema.identifier_);
  return block;
}

static CodeBlock namespaceEnd(const SyncedSchema& schema) {
  CodeBlock block(schema.codeProperties.get().indent_);
#if JSON_DEBUG
  block << "/*" << schema.identifier_ << " namespace end*/";
#endif
  BLOCK << std::format("}} // namespace {}", schema.identifier_);

  if (schema.codeProperties.get().globalNamespace_.has_value()) {
    BLOCK << std::format("}} // namespace {}",
                         schema.codeProperties.get().globalNamespace_.value());
  }
  return block;
}

static CodeBlock functionDeclarations(const SyncedSchema& schema) {
  CodeBlock block(schema.codeProperties.get().indent_);
#if JSOG_DEBUG
  block << "/*" << schema.identifier_ << " function declarations*/";
#endif
  BLOCK << std::format("using {} = {};", schema.identifier_, schema.getType());
  if (schema.default_.has_value()) {
    BLOCK << std::format(
        "std::optional<{}> construct(const nlohmann::json& = default_);",
        schema.getType());
  } else {
    BLOCK << std::format("std::optional<{}> construct(const nlohmann::json&);",
                         schema.getType());
  }
  BLOCK << std::format("nlohmann::json rawExport(const {}&);",
                       schema.getType());
  BLOCK << std::format("bool validate(const {}&);", schema.getType());
  return block;
}

static CodeBlock classForwardDeclarations(const SyncedSchema& schema) {
#if JSOG_DEBUG
  block << "/*" << schema.identifier_ << " forward declarations*/";
#endif
  CodeBlock block(schema.codeProperties.get().indent_);
  if (schema.type_.contains(IndexedSyncedSchema::Type::Object) ||
      schema.type_.contains(IndexedSyncedSchema::Type::Array)) {
    BLOCK << "// Forward declarations";
    if (schema.type_.contains(IndexedSyncedSchema::Type::Object)) {
      BLOCK << "class Object;";
    }
    if (schema.type_.contains(IndexedSyncedSchema::Type::Array)) {
      BLOCK << "class Array;";
    }
    BLOCK << "";
  }
  return block;
}

static CodeBlock objectClassDefinition(const SyncedSchema& schema) {
  CodeBlock block(schema.codeProperties.get().indent_);
#if JSOG_DEBUG
  block << "/*" << schema.identifier_ << " object class definition*/";
#endif
  if (schema.type_.contains(IndexedSyncedSchema::Type::Object)) {
    // Object declaration
    BLOCK << "class Object {" << CodeBlock::inc;
    {
      // Declare the construct function to be a friend, so it can fill out
      // private members
      BLOCK << std::format(
          "friend std::optional<{}> construct(const nlohmann::json&);",
          schema.getType());
      BLOCK << CodeBlock::dec << "public:" << CodeBlock::inc;
      for (const auto& [propertyName, property] : schema.properties_) {
        if (schema.required_.has_value() &&
            schema.required_.value().count(propertyName) > 0) {
          BLOCK << std::format("{} {};", property.get().getType(),
                               sanitizeString(propertyName) + "_");
        } else {
          BLOCK << std::format("std::optional<{}> {};",
                               property.get().getType(),
                               sanitizeString(propertyName) + "_");
        }
      }
      if (schema.additionalProperties_.get().definedAsBooleanSchema_.value_or(
              true) != false) {
        BLOCK << std::format("std::map<std::string, {}> additionalProperties;",
                             schema.additionalProperties_.get().getType());
      }
    }
    BLOCK << CodeBlock::dec << "};" << "";
  }
  return block;
}

static CodeBlock arrayClassDefinition(const SyncedSchema& schema) {
  CodeBlock block(schema.codeProperties.get().indent_);
#if JSOG_DEBUG
  block << "/*" << schema.identifier_ << " array class definition*/";
#endif
  if (schema.type_.contains(IndexedSyncedSchema::Type::Array)) {
    // Array declaration
    BLOCK << "class Array {" << CodeBlock::inc;
    {
      // Declare the construct function to be a friend, so it can fill out
      // private members
      BLOCK << std::format(
          "friend std::optional<{}> construct(const nlohmann::json&);",
          schema.getType());
      // Tupleable item declaration
      BLOCK << CodeBlock::dec << "public:" << CodeBlock::inc;
      if (schema.tupleableItems_.has_value()) {
        for (size_t i = 0; i < schema.tupleableItems_->size(); i++) {
          BLOCK << std::format("std::optional<{}> item{};",
                               (*schema.tupleableItems_)[i].get().getType(), i);
        }
      }
      BLOCK << std::format("std::vector<{}> items;",
                           schema.items_.get().getType());

      BLOCK << CodeBlock::dec << "public:" << CodeBlock::inc;
      BLOCK << "template <size_t N>";
      BLOCK << "auto get() const {";
      BLOCK << CodeBlock::inc;
      {
        if (schema.tupleableItems_.has_value() &&
            schema.tupleableItems_->size() > 0) {
          for (size_t i = 0; i < schema.tupleableItems_->size(); i++) {
            BLOCK << std::format("if constexpr(N == {}) {{", i)
                  << CodeBlock::inc;
            {
              BLOCK << std::format("if (item{}.has_value()) {{", i)
                    << CodeBlock::inc
                    << std::format("return item{}.value();", i)
                    << CodeBlock::dec << "}";
            }
            BLOCK << CodeBlock::dec << CodeBlock::dis << "} else ";
          }
          BLOCK << "{" << CodeBlock::inc
                << std::format("if(N - {} < items.size()) {{",
                               schema.tupleableItems_->size())
                << CodeBlock::inc;
          {
            BLOCK << std::format("return items[N - {}];",
                                 schema.tupleableItems_->size());
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
      BLOCK << std::format("inline {} get(size_t n) {{",
                           schema.items_.get().getType())
            << CodeBlock::inc << "if(n >= items.size()) {"
            << "throw std::range_error(\"Item \" + std::to_string(n) + \" out "
               "of range\");"
            << "}"
            << "return items[n];" << CodeBlock::dec << "}";
    }
    BLOCK << CodeBlock::dec;
    BLOCK << "};";
  }

  return block;
}

CodeBlock SyncedSchema::generateDeclaration() const {
  CodeBlock block(codeProperties.get().indent_);

  BLOCK << declarationIncludeGuardStart(*this);

  BLOCK << "";

  BLOCK << generateDependencies();

  BLOCK << "";

  BLOCK << namespaceStart(*this);

  if (!definedAsBooleanSchema_.has_value() && !ref_.has_value()) {
    // Forward declarations
    BLOCK << classForwardDeclarations(*this);

    // Default value declaration
    if (default_.has_value()) {
      BLOCK << "extern const nlohmann::json default_;";
    }

    // Object declaration
    BLOCK << objectClassDefinition(*this);

    // Array declaration
    BLOCK << arrayClassDefinition(*this);
  }

  BLOCK << functionDeclarations(*this);

  BLOCK << namespaceEnd(*this);

  BLOCK << declarationIncludeGuardEnd(*this);

  return block;
}

static std::string escapeJSONString(const std::string& str) {
  std::string escaped;
  // Reserve enough space for escaping every char
  escaped.reserve(str.size() * 4);
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

  // Global namespace
  if (codeProperties.get().globalNamespace_.has_value()) {
    BLOCK << std::format("namespace {} {{",
                         codeProperties.get().globalNamespace_.value());
  }

  // Identifier namespace
  BLOCK << std::format("namespace {} {{", identifier_);

  // Default value definition
  if (default_.has_value()) {
    BLOCK << std::format("const nlohmann::json default_ = \"{}\"_json;",
                         escapeJSON(default_.value()));
  }

  // Construct function
  BLOCK << std::format(
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
    } else {
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
        types.insert(type_.begin(), type_.end());

        if (types.size() == 0) {
          types.insert(Type::Object);
          types.insert(Type::Null);
          types.insert(Type::Boolean);
          types.insert(Type::Array);
          types.insert(Type::Number);
          types.insert(Type::String);
          types.insert(Type::Integer);
        }

        BLOCK << std::format("std::optional<{}> outSchema = std::nullopt;",
                             getType());

        if (types.contains(Type::Null)) {
          BLOCK << "if(json.is_null()) {" << CodeBlock::inc
                << std::format("outSchema = {}();", getNullType())
                << CodeBlock::dec << "}";
        }

        if (types.contains(Type::Boolean)) {
          BLOCK << "if(json.is_boolean()) {" << CodeBlock::inc
                << std::format("outSchema = json.get<{}>();", getBooleanType())
                << CodeBlock::dec << "}";
        }

        if (types.contains(Type::Integer)) {
          BLOCK << "if(json.is_number_integer()) {" << CodeBlock::inc
                << std::format("outSchema = json.get<{}>();", getIntegerType())
                << CodeBlock::dec << "}";
        }

        if (types.contains(Type::Number)) {
          BLOCK << "if(json.is_number()) {" << CodeBlock::inc
                << std::format("outSchema = json.get<{}>();", getNumberType())
                << CodeBlock::dec << "}";
        }

        if (types.contains(Type::String)) {
          BLOCK << "if(json.is_string()) {" << CodeBlock::inc
                << std::format("outSchema = json.get<{}>();", getStringType())
                << CodeBlock::dec << "}";
        }

        if (types.contains(Type::Array)) {
          BLOCK << "if(json.is_array()) {";
          {
            Indent _(block);
            BLOCK << "auto array = Array();";
            if (tupleableItems_.has_value()) {
              for (size_t i = 0; i < tupleableItems_->size(); i++) {
                BLOCK << std::format("if(json.size() > {}) {{", i)
                      << CodeBlock::inc
                      << std::format("array.item{} = {}::construct(json[{}]);",
                                     i, (*tupleableItems_)[i].get().identifier_,
                                     i)
                      << CodeBlock::dec << "}";
              }
            }
            BLOCK << std::format(
                "for (size_t i = {}; i < json.size(); i++) {{",
                tupleableItems_
                    .value_or(std::vector<
                              std::reference_wrapper<const SyncedSchema>>())
                    .size());
            {
              Indent _(block);
              BLOCK << std::format(
                  "array.items.push_back({}::construct(json[i]).value());",
                  items_.get().identifier_);
            }
            BLOCK << "}";
            BLOCK << "outSchema = std::move(array);";
          }
          BLOCK << "}";
        }

        if (types.contains(Type::Object)) {
          BLOCK << "if(json.is_object()) {";
          {
            Indent _(block);
            BLOCK << "auto object = Object();";
            bool hasPropertyCountLimits =
                maxProperties_.has_value() || minProperties_.has_value();
            if (hasPropertyCountLimits) {
              BLOCK << "size_t propertyCount = json.size();";
            }
            if (maxProperties_.has_value()) {
              BLOCK << std::format("if(propertyCount > {}) {{",
                                   maxProperties_.value())
                    << CodeBlock::inc << "return std::nullopt;"
                    << CodeBlock::dec << "}";
            }
            if (minProperties_.has_value()) {
              BLOCK << std::format("if(propertyCount < {}) {{",
                                   minProperties_.value())
                    << CodeBlock::inc << "return std::nullopt;"
                    << CodeBlock::dec << "}";
            }
            BLOCK << "static const std::set<std::string> properties = {";
            for (const auto& [propertyName, schema] : properties_) {
              Indent _(block);
              BLOCK << std::format("\"{}\",", propertyName);
            }
            BLOCK << "};";
            for (const auto& [propertyName, schema] : properties_) {
              if (required_.has_value() &&
                  required_.value().contains(propertyName)) {
                BLOCK << std::format("if(!json.contains(\"{}\")) {{",
                                     propertyName);
                {
                  Indent _(block);
                  BLOCK << "return std::nullopt;";
                }
                BLOCK << "}";
                BLOCK << std::format(
                    "object.{} = {}::construct(json[\"{}\"]).value();",
                    sanitizeString(propertyName) + "_",
                    schema.get().identifier_, propertyName);
              } else {
                BLOCK << std::format("if(json.contains(\"{}\")) {{",
                                     propertyName);
                {
                  Indent _(block);
                  BLOCK << std::format(
                      "object.{} = {}::construct(json[\"{}\"]).value();",
                      sanitizeString(propertyName) + "_",
                      schema.get().identifier_, propertyName);
                }
                BLOCK << "}";
              }
            }
            BLOCK << "for (const auto& [key, value] : json.items()) {";
            {
              Indent _(block);
              BLOCK << "if (properties.count(key) == 0) {";
              {
                Indent _(block);
                // If additionalProperties
                if (additionalProperties_.get()
                        .definedAsBooleanSchema_.value_or(true) == false) {
                  BLOCK << "return std::nullopt;";
                } else {
                  BLOCK << std::format("object.additionalProperties[key] = "
                                       "{}::construct(value).value();",
                                       additionalProperties_.get().identifier_);
                }
              }
              BLOCK << "}";
            }
            BLOCK << "}";
            BLOCK << "outSchema = std::move(object);";
          }
          BLOCK << "}";
        }
        BLOCK << "if(outSchema.has_value()) {";
        {
          Indent _(block);
          BLOCK << "if (!validate(outSchema.value())) {";
          {
            Indent _(block);
            BLOCK << "return std::nullopt;";
          }
          BLOCK << "}";
        }
        BLOCK << "}";
        BLOCK << "return outSchema;";
      }

      BLOCK << "throw std::runtime_error(\"Unreachable, likely a bug in the "
               "autogenerated code.\");";
    }
  }
  BLOCK << CodeBlock::dec << "}";

  // export function
  BLOCK << std::format("nlohmann::json rawExport(const {}& schema) {{",
                       getType());
  {
    Indent _(block);

    if (definedAsBooleanSchema_.has_value()) {
      if (definedAsBooleanSchema_.value()) {
        // If the schema is defined as true, then the type is an nllohmann::json
        // object and we can just return it
        BLOCK << "return schema;";
      } else {
        // If the schema is defined as false, then any json conversion should
        // fail, however this is the non-failing conversion function, so
        // we'll just return null
        BLOCK << "return nullptr;";
      }
    } else {

      std::set<Type> types;
      types.insert(type_.begin(), type_.end());

      if (types.size() == 0) {
        types.insert(Type::Object);
        types.insert(Type::Null);
        types.insert(Type::Boolean);
        types.insert(Type::Array);
        types.insert(Type::Number);
        types.insert(Type::String);
        types.insert(Type::Integer);
      }
      if (types.size() > 1) {
        if (types.contains(Type::Null)) {
          BLOCK << "if (std::holds_alternative<std::monostate>(schema)) {";
          {
            Indent _(block);
            BLOCK << "return nullptr;";
          }
          BLOCK << "}";
        }
        if (types.contains(Type::Number)) {
          BLOCK << "if (std::holds_alternative<double>(schema)) {";
          {
            Indent _(block);
            BLOCK << "return std::get<double>(schema);";
          }
          BLOCK << "}";
        }
        if (types.contains(Type::Integer)) {
          BLOCK << std::format("if (std::holds_alternative<{}>(schema)) {{",
                               getIntegerType());
          {
            Indent _(block);
            BLOCK << std::format("return std::get<{}>(schema);",
                                 getIntegerType());
          }
          BLOCK << "}";
        }
        if (types.contains(Type::String)) {
          BLOCK << "if (std::holds_alternative<std::string>(schema)) {";
          {
            Indent _(block);
            BLOCK << "return std::get<std::string>(schema);";
          }
          BLOCK << "}";
        }
        if (types.contains(Type::Boolean)) {
          BLOCK << "if (std::holds_alternative<bool>(schema)) {";
          {
            Indent _(block);
            BLOCK << "return std::get<bool>(schema);";
          }
          BLOCK << "}";
        }
        if (types.contains(Type::Array)) {
          BLOCK << "if (std::holds_alternative<Array>(schema)) {";
          {
            Indent _(block);
            BLOCK << "auto json = nlohmann::json::array();";
            BLOCK << "auto& array = std::get<Array>(schema);";
            if (tupleableItems_.has_value()) {
              for (size_t i = 0; i < tupleableItems_->size(); i++) {
                const auto& item = (*tupleableItems_)[i].get();
                BLOCK << std::format("if(array.item{}.has_value()) {{", i);
                BLOCK << CodeBlock::inc;
                BLOCK << std::format("json.push_back({}::rawExport(array.item{}"
                                     ".value()));",
                                     item.identifier_, i);
                BLOCK << CodeBlock::dec << "}";
              }
            }
            BLOCK << "for (const auto& item : array.items) {";
            {
              Indent _(block);
              BLOCK << std::format("json.push_back({}::rawExport(item));",
                                   items_.get().identifier_);
            }
            BLOCK << "}";
            BLOCK << "return json;";
          }
          BLOCK << "}";
        }
        if (types.contains(Type::Object)) {
          BLOCK << "if (std::holds_alternative<Object>(schema)) {";
          {
            Indent _(block);
            BLOCK << "auto& object = std::get<Object>(schema);";
            BLOCK << "auto json = nlohmann::json::object();";
            for (const auto& [propertyName, schema] : properties_) {
              const auto is_required =
                  required_.has_value() &&
                  required_.value().count(propertyName) > 0;
              if (!is_required) {
                BLOCK << std::format("if (object.{}.has_value()) {{",
                                     sanitizeString(propertyName) + "_")
                      << CodeBlock::inc;
              }
              BLOCK << std::format("json[\"{}\"] = {}::rawExport(object.{}{});",
                                   propertyName, schema.get().identifier_,
                                   sanitizeString(propertyName) + "_",
                                   is_required ? "" : ".value()");

              if (!is_required) {
                BLOCK << CodeBlock::dec << "}";
              }
            }
            if (additionalProperties_.get().definedAsBooleanSchema_.value_or(
                    true) != false) {
              BLOCK << "for (const auto& [key, value] : "
                       "object.additionalProperties) {";
              {
                Indent _(block);
                BLOCK << std::format("json[key] = {}::rawExport(value);",
                                     additionalProperties_.get().identifier_);
              }
              BLOCK << "}";
            }
            BLOCK << "return json;";
          }
          BLOCK << "}";
        }

      } else {
        const auto type = *types.begin();
        if (type == Type::Null) {
          BLOCK << "return nullptr;";
        } else if (type == Type::Number) {
          BLOCK << "return schema;";
        } else if (type == Type::Integer) {
          BLOCK << "return schema;";
        } else if (type == Type::String) {
          BLOCK << "return schema;";
        } else if (type == Type::Boolean) {
          BLOCK << "return schema;";
        } else if (type == Type::Array) {
          BLOCK << "auto json = nlohmann::json::array();";
          BLOCK << "auto& array = schema;";
          if (tupleableItems_.has_value()) {
            for (size_t i = 0; i < tupleableItems_->size(); i++) {
              const auto& item = (*tupleableItems_)[i].get();
              BLOCK << std::format("if(array.item{}.has_value()) {{", i);
              BLOCK << CodeBlock::inc;
              BLOCK << std::format("json.push_back({}::rawExport(array.item{}"
                                   ".value()));",
                                   item.identifier_, i);
              BLOCK << CodeBlock::dec << "}";
            }
          }
          BLOCK << "for (const auto& item : array.items) {";
          {
            Indent _(block);
            BLOCK << std::format("json.push_back({}::rawExport(item));",
                                 items_.get().identifier_);
          }
          BLOCK << "}";
          BLOCK << "return json;";
        } else if (type == Type::Object) {
          BLOCK << "auto& object = schema;";
          BLOCK << "auto json = nlohmann::json::object();";
          for (const auto& [propertyName, schema] : properties_) {
            const auto is_required = required_.has_value() &&
                                     required_.value().count(propertyName) > 0;
            if (!is_required) {
              BLOCK << std::format("if (object.{}.has_value()) {{",
                                   sanitizeString(propertyName) + "_")
                    << CodeBlock::inc;
            }
            BLOCK << std::format("json[\"{}\"] = {}::rawExport(object.{}{});",
                                 propertyName, schema.get().identifier_,
                                 sanitizeString(propertyName) + "_",
                                 is_required ? "" : ".value()");

            if (!is_required) {
              BLOCK << CodeBlock::dec << "}";
            }
          }
          if (additionalProperties_.get().definedAsBooleanSchema_.value_or(
                  true) != false) {
            BLOCK << "for (const auto& [key, value] : "
                     "object.additionalProperties) {";
            {
              Indent _(block);
              BLOCK << std::format("json[key] = {}::rawExport(value);",
                                   additionalProperties_.get().identifier_);
            }
            BLOCK << "}";
          }
          BLOCK << "return json;";
        } else {
          throw std::runtime_error("Unknown type");
        }
      }
      // A throw guard to report invalid function definition
      BLOCK << "throw std::runtime_error(\"Unreachable, likely a bug in the "
               "autogenerated code.\");";
    }
  }
  BLOCK << "}";

  // Validator function
  BLOCK << std::format("bool validate(const {}& schema) {{", getType());
  {
    Indent _(block);
    if (definedAsBooleanSchema_.has_value()) {
      if (definedAsBooleanSchema_.value()) {
        BLOCK << "return true;";
      } else {
        BLOCK << "return false;";
      }
    } else {
      if (const_.has_value()) {
        BLOCK << std::format("if (schema != \"{}\"_json) {{",
                             escapeJSON(const_.value()))
              << CodeBlock::inc << "return false;" << CodeBlock::dec << "}";
      }
      std::set<Type> types;
      types.insert(type_.begin(), type_.end());

      if (types.size() == 0) {
        types.insert(Type::Object);
        types.insert(Type::Null);
        types.insert(Type::Boolean);
        types.insert(Type::Array);
        types.insert(Type::Number);
        types.insert(Type::String);
        types.insert(Type::Integer);
      }
      bool isSingleType = types.size() == 1;
      if (types.contains(Type::Null)) {
        if (!isSingleType) {
          BLOCK << "if (std::holds_alternative<std::monostate>(schema)) {";
          BLOCK << CodeBlock::inc;
          BLOCK << "const auto& nullValue = std::get<std::monostate>(schema);";
        } else {
          BLOCK << "{" << CodeBlock::inc;
          BLOCK << "const auto& nullValue = schema;";
        }
        // Null validation
        // Null doesn't have any properties to validate

        BLOCK << CodeBlock::dec << "}";
      }
      if (types.contains(Type::Boolean)) {
        if (!isSingleType) {
          BLOCK << "if (std::holds_alternative<bool>(schema)) {";
          BLOCK << CodeBlock::inc;
          BLOCK << "const auto& booleanValue = std::get<bool>(schema);";
        } else {
          BLOCK << "{" << CodeBlock::inc;
          BLOCK << "const auto& booleanValue = schema;";
        }
        // Boolean validation
        // Boolean doesn't have any properties to validate
        BLOCK << CodeBlock::dec << "}";
      }
      if (types.contains(Type::Integer) || types.contains(Type::Number)) {
        // Construct a lambda function with auto parameter to be able to
        // validate double and any integer type
        BLOCK
            << "const auto numericValidator = [](auto numericValue) -> bool {";
        {
          Indent _(block);
          // Integer/number validation
          // multipleOf
          if (multipleOf_.has_value()) {
            BLOCK << std::format("if ((std::abs(numericValue / {0}) - "
                                 "static_cast<long>(std::abs(numericValue / "
                                 "{0})) ) < 0.0000001) {{",
                                 multipleOf_.value())
                  << CodeBlock::inc << "return false;" << CodeBlock::dec << "}";
          }

          // maximum
          if (maximum_.has_value()) {
            BLOCK << std::format("if (numericValue > {}) {{", maximum_.value())
                  << CodeBlock::inc << "return false;" << CodeBlock::dec << "}";
          }

          // exclusiveMaximum
          if (exclusiveMaximum_.has_value()) {
            BLOCK << std::format("if (numericValue >= {}) {{",
                                 exclusiveMaximum_.value())
                  << CodeBlock::inc << "return false;" << CodeBlock::dec << "}";
          }

          // minimum
          if (minimum_.has_value()) {
            BLOCK << std::format("if (numericValue < {}) {{", minimum_.value())
                  << CodeBlock::inc << "return false;" << CodeBlock::dec << "}";
          }

          // exclusiveMinimum
          if (exclusiveMinimum_.has_value()) {
            BLOCK << std::format("if (numericValue <= {}) {{",
                                 exclusiveMinimum_.value())
                  << CodeBlock::inc << "return false;" << CodeBlock::dec << "}";
          }
          BLOCK << "return true;";
        }
        BLOCK << "};";
        if (types.contains(Type::Integer)) {
          if (!isSingleType) {
            BLOCK << std::format("if (std::holds_alternative<{}>(schema)) {{",
                                 getIntegerType());
            BLOCK << CodeBlock::inc;
            BLOCK << std::format(
                "const auto& integerValue = std::get<{}>(schema);",
                getIntegerType());
          } else {
            BLOCK << "{" << CodeBlock::inc;
            BLOCK << "const auto& integerValue = schema;";
          }
          BLOCK << "return numericValidator(integerValue);";
          BLOCK << CodeBlock::dec << "}";
        } else {
          if (!isSingleType) {
            BLOCK << "if (std::holds_alternative<double>(schema)) {";
            BLOCK << CodeBlock::inc;
            BLOCK << "const auto& numberValue = std::get<double>(schema);";
          } else {
            BLOCK << "{" << CodeBlock::inc;
            BLOCK << "const auto& numberValue = schema;";
          }
          BLOCK << "return numericValidator(numberValue);";
          BLOCK << CodeBlock::dec << "}";
        }
      }
      // TODO: String validation
      if (types.contains(Type::String)) {
        if (!isSingleType) {
          BLOCK << "if (std::holds_alternative<std::string>(schema)) {";
          BLOCK << CodeBlock::inc;
          BLOCK << "const auto& stringValue = std::get<std::string>(schema);";
        } else {
          BLOCK << "{" << CodeBlock::inc;
          BLOCK << "const auto& stringValue = schema;";
        }
        // MaxLength
        if (maxLength_.has_value()) {
          BLOCK << std::format("if (stringValue.size() > {}) {{",
                               maxLength_.value())
                << CodeBlock::inc << "return false;" << CodeBlock::dec << "}";
        }

        // MinLength
        if (minLength_.has_value()) {
          BLOCK << std::format("if (stringValue.size() < {}) {{",
                               minLength_.value())
                << CodeBlock::inc << "return false;" << CodeBlock::dec << "}";
        }

        // Pattern
        if (pattern_.has_value()) {
          BLOCK << std::format("if (!std::regex_match(stringValue, std::regex("
                               "\"{}\", std::regex_constants::ECMAScript))) {{",
                               escapeJSONString(pattern_.value()))
                << CodeBlock::inc << "return false;" << CodeBlock::dec << "}";
        }
        BLOCK << CodeBlock::dec << "}";
      }
      // TODO: Array validation
      if (types.contains(Type::Array)) {
        if (!isSingleType) {
          BLOCK << "if (std::holds_alternative<Array>(schema)) {";
          BLOCK << CodeBlock::inc;
          BLOCK << "const auto& arrayValue = std::get<Array>(schema);";
        } else {
          BLOCK << "{" << CodeBlock::inc;
          BLOCK << "const auto& arrayValue = schema;";
        }
        // MaxItems
        if (maxItems_.has_value()) {
          BLOCK << std::format("if (arrayValue.items.size() > {}) {{",
                               maxItems_.value())
                << CodeBlock::inc << "return false;" << CodeBlock::dec << "}";
        }

        // MinItems
        if (minItems_.has_value()) {
          BLOCK << std::format("if (arrayValue.items.size() < {}) {{",
                               minItems_.value())
                << CodeBlock::inc << "return false;";
          BLOCK << CodeBlock::dec << "}";
        }

        // UniqueItems
        if (uniqueItems_.has_value() && uniqueItems_.value()) {
          BLOCK << "{";
          {
            Indent _(block);
            BLOCK << "std::set<std::string> uniqueItems;";
            const auto tupleableItems = tupleableItems_.value_or(
                std::vector<std::reference_wrapper<const SyncedSchema>>());
            for (size_t i = 0; i < tupleableItems.size(); i++) {
              BLOCK << std::format("if (arrayValue.item{}.has_value()) {{", i);
              {
                Indent _(block);
                BLOCK << std::format("const std::string item = {}::json("
                                     "arrayValue.item{}.value()).dump(0);",
                                     (*tupleableItems_)[i].get().identifier_,
                                     i);
                BLOCK << "if (uniqueItems.count(item) > 0) {";
                {
                  Indent _(block);
                  BLOCK << "return false;";
                }
                BLOCK << "}";
                BLOCK << "uniqueItems.insert(item);";
              }
              BLOCK << "}";
            }
            if (items_.get().definedAsBooleanSchema_.value_or(true) == false) {
              BLOCK << "for (const auto& item : arrayValue.items) {";
              {
                Indent _(block);
                BLOCK << "const std::string item = "
                         "{}::json(item).dump(0);";
                BLOCK << "if (uniqueItems.count(item) > 0) {";
                {
                  Indent _(block);
                  BLOCK << "return false;";
                }
                BLOCK << "}";
                BLOCK << "uniqueItems.insert(item);";
              }
            }
          }
          BLOCK << "}";
        }

        // Contains
        if (contains_.has_value()) {
          BLOCK << "bool contains = false;";
          BLOCK << "for (const auto& item : arrayValue.items) {";
          {
            Indent _(block);
            BLOCK << "if ({}::validate(item)) {";
            {
              Indent _(block);
              BLOCK << "contains = true;";
              BLOCK << "break;";
            }
            BLOCK << "}";
          }
          BLOCK << "}";
          BLOCK << "if (!contains) {";
          {
            Indent _(block);
            BLOCK << "return false;";
          }
          BLOCK << "}";
        }

        BLOCK << CodeBlock::dec << "}";
      }

      if (types.contains(Type::Object)) {
        if (!isSingleType) {
          BLOCK << "if (std::holds_alternative<Object>(schema)) {";
          BLOCK << CodeBlock::inc;
          BLOCK << "const auto& objectValue = std::get<Object>(schema);";
        } else {
          BLOCK << "{" << CodeBlock::inc;
          BLOCK << "const auto& objectValue = schema;";
        }
        // MaxProperties / MinProperties
        if (maxProperties_.has_value() || minProperties_.has_value()) {
          BLOCK << "{";
          {
            Indent _(block);
            // Count the number of properties
            BLOCK << "size_t propertyCount = schema.properties.size();";
            for (const auto& [propertyName, schema] : properties_) {
              // If the property is required, then a representation of it must
              // have already been created, meaning there is nothing to check
              // and we can simply increment the property count
              if (required_.has_value() &&
                  required_.value().count(propertyName) > 0) {
                BLOCK << "propertyCount++;";
              } else {
                BLOCK << std::format("if (schema.{}.has_value()) {{",
                                     sanitizeString(propertyName) + "_");
                {
                  Indent _(block);
                  BLOCK << "propertyCount++;";
                }
                BLOCK << "}";
              }
            }
            if (maxProperties_.has_value()) {
              BLOCK << std::format("if (propertyCount > {}) {{",
                                   maxProperties_.value())
                    << CodeBlock::inc;
              BLOCK << "return false;";
              BLOCK << CodeBlock::dec << "}";
            }
            if (minProperties_.has_value()) {
              BLOCK << std::format("if (propertyCount < {}) {{",
                                   minProperties_.value())
                    << CodeBlock::inc;
              BLOCK << "return false;";
              BLOCK << CodeBlock::dec << "}";
            }
          }
          BLOCK << "}";
        }

        // Required
        if (required_.has_value()) {
          std::set<std::string> requiredAdditionalProperties =
              required_.value();
          for (const auto& [propertyName, _] : properties_) {
            requiredAdditionalProperties.erase(propertyName);
          }

          for (const auto& propertyName : requiredAdditionalProperties) {
            BLOCK << std::format("if (objectValue.additionalProperties.count("
                                 "\"{}\") == 0) {{",
                                 propertyName)
                  << CodeBlock::inc;
            BLOCK << "return false;";
            BLOCK << CodeBlock::dec << "}";
          }
        }

        // PatternProperties
        if (patternProperties_.has_value()) {
          for (const auto& [pattern, schema] : patternProperties_.value()) {
            BLOCK << "for (const auto& [key, value] : "
                     "objectValue.additionalProperties) {";
            {
              Indent _(block);
              BLOCK << "if (std::regex_match(key, std::regex(\"" +
                           escapeJSONString(pattern) +
                           "\", std::regex_constants::ECMAScript))) {";
              {
                Indent _(block);
                BLOCK << std::format(
                    "const auto rawValue = {}::rawExport(value);",
                    additionalProperties_.get().identifier_);
                BLOCK << std::format("const auto constructedValue = "
                                     "{}::construct(rawValue);",
                                     schema.get().identifier_);
                BLOCK << std::format(
                    "if (!{0}::validate(constructedValue.value())) {{",
                    schema.get().identifier_);
                {
                  Indent _(block);
                  BLOCK << "return false;";
                }
                BLOCK << "}";
              }
              BLOCK << "}";
            }
            BLOCK << "}";
          }
        }

        BLOCK << CodeBlock::dec << "}";
      }

      BLOCK << "return true;";
    }
  }
  BLOCK << "}";

  BLOCK << std::format("}} // namespace {}", identifier_);

  if (codeProperties.get().globalNamespace_.has_value()) {
    BLOCK << std::format("}} // namespace {}",
                         codeProperties.get().globalNamespace_.value());
  }

  return block;
}

CodeBlock SyncedSchema::generateSystemDependencies() const {
  CodeBlock block(codeProperties.get().indent_);
  BLOCK << "#ifndef JSOG_SYS_DEPS";
  BLOCK << "#define JSOG_SYS_DEPS";
  BLOCK << "// System dependencies"
        << "#include <optional>"
        << "#include <variant>"
        << "#include <vector>"
        << "#include <string>"
        << "#include <map>"
        << "#include <stdexcept>"
        << "#include <set>"
        << "#include <regex>"
        << "#include <nlohmann/json.hpp>";
  BLOCK << "#endif // JSOG_SYS_DEPS" << "";
  return block;
}

CodeBlock SyncedSchema::generateDependencies() const {
  CodeBlock block(codeProperties.get().indent_);
  block << generateSystemDependencies();
  std::set<const SyncedSchema*> dependencies;
  if (ref_.has_value()) {
    dependencies.insert(&ref_.value().get());
  }
  for (const auto& item : tupleableItems_.value_or(
           std::vector<std::reference_wrapper<const SyncedSchema>>())) {
    dependencies.insert(&item.get());
  }
  dependencies.insert(&items_.get());
  if (contains_.has_value()) {
    dependencies.insert(&contains_.value().get());
  }
  for (const auto& [_, schema] : properties_) {
    dependencies.insert(&schema.get());
  }
  if (patternProperties_.has_value()) {
    for (const auto& [_, schema] : patternProperties_.value()) {
      dependencies.insert(&schema.get());
    }
  }
  dependencies.insert(&additionalProperties_.get());

  if (schemaDependencies_.has_value()) {
    for (const auto& [_, schema] : schemaDependencies_.value()) {
      dependencies.insert(&schema.get());
    }
  }
  if (propertyNames_.has_value()) {
    dependencies.insert(&propertyNames_.value().get());
  }
  if (if_.has_value()) {
    dependencies.insert(&if_.value().get());
  }
  if (then_.has_value()) {
    dependencies.insert(&then_.value().get());
  }
  if (else_.has_value()) {
    dependencies.insert(&else_.value().get());
  }
  if (allOf_.has_value()) {
    for (const auto& schema : allOf_.value()) {
      dependencies.insert(&schema.get());
    }
  }
  if (anyOf_.has_value()) {
    for (const auto& schema : anyOf_.value()) {
      dependencies.insert(&schema.get());
    }
  }
  if (oneOf_.has_value()) {
    for (const auto& schema : oneOf_.value()) {
      dependencies.insert(&schema.get());
    }
  }
  if (not_.has_value()) {
    dependencies.insert(&not_.value().get());
  }

  BLOCK << "// Local dependencies";

  for (const auto& dependency : dependencies) {
    if (!dependency->getHeaderFileName().empty()) {
      BLOCK << std::format("#include \"{}\"", dependency->getHeaderFileName());
    }
  }
  bool hasForwardDeclarations = false;
  CodeBlock forwardDeclarations;
  forwardDeclarations << "// Forward declarations";
  if (codeProperties.get().globalNamespace_.has_value()) {
    forwardDeclarations << std::format(
        "namespace {} {{", codeProperties.get().globalNamespace_.value());
  }
  for (const auto& dependency : dependencies) {
    bool hasForwardDeclaration = false;
    CodeBlock forwardDeclaration;
    forwardDeclaration << std::format("namespace {} {{",
                                      dependency->identifier_);
    if (!dependency->definedAsBooleanSchema_.has_value() &&
        !dependency->ref_.has_value()) {
      if (dependency->type_.contains(Type::Object)) {
        forwardDeclaration << std::format("class Object;");
        hasForwardDeclaration = true;
      }
      if (dependency->type_.contains(Type::Array)) {
        forwardDeclaration << std::format("class Array;");
        hasForwardDeclaration = true;
      }
    }
    forwardDeclaration << std::format("}} // namespace {}",
                                      dependency->identifier_);
    if (hasForwardDeclaration) {
      hasForwardDeclarations = true;
      forwardDeclarations << forwardDeclaration;
    }
  }
  if (codeProperties.get().globalNamespace_.has_value()) {
    forwardDeclarations << std::format(
        "}} // namespace {}", codeProperties.get().globalNamespace_.value());
  }
  if (hasForwardDeclarations) {
    BLOCK << forwardDeclarations;
  }
  BLOCK << "";
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

  // If the schema is a reference, return the reference type wrapped in a
  // unique pointer, as there might be issues with the type being incomplete.
  if (ref_.has_value()) {
    return std::format("std::unique_ptr<{}>", ref_.value().get().getType());
  }

  std::set<Type> types;
  types.insert(type_.begin(), type_.end());

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

static SyncedSchema::IntegerType
smallestIntegerType(std::optional<double> minimum,
                    std::optional<double> maximum,
                    std::optional<double> exclusiveMinimum,
                    std::optional<double> exclusiveMaximum) {
  using IntegerType = SyncedSchema::IntegerType;
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
  IntegerType type = getIntegerEnum();

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

SyncedSchema::IntegerType SyncedSchema::getIntegerEnum() const {
  return smallestIntegerType(minimum_, maximum_, exclusiveMinimum_,
                             exclusiveMaximum_);
}

std::vector<std::unique_ptr<SyncedSchema>>
SyncedSchema::resolveIndexedSchema(std::vector<IndexedSyncedSchema>&& schemas) {
  try {
    std::vector<std::unique_ptr<SyncedSchema>> syncedSchemas;
    for (auto& schema : schemas) {
      std::unique_ptr<SyncedSchema> syncedSchema =
          std::make_unique<SyncedSchema>(schema.identifier_);
      syncedSchemas.emplace_back(std::move(syncedSchema));
    }
    for (size_t i = 0; i < schemas.size(); i++) {
      auto& schema = schemas[i];
      auto& syncedSchema = *syncedSchemas[i];
      syncedSchema.definedAsBooleanSchema_ = schema.definedAsBooleanSchema_;
      if (schema.ref_.has_value()) {
        syncedSchema.ref_ = *syncedSchemas[schema.ref_.value()];
      } else {
        syncedSchema.ref_ = std::nullopt;
      }
      syncedSchema.type_ = schema.type_.value_or(std::set<Type>{
          Type::Object, Type::Array, Type::Boolean, Type::Integer, Type::Null,
          Type::Number, Type::String});
      syncedSchema.enum_ = schema.enum_;
      syncedSchema.const_ = schema.const_;
      syncedSchema.description_ = schema.description_;
      syncedSchema.multipleOf_ = schema.multipleOf_;
      syncedSchema.maximum_ = schema.maximum_;
      syncedSchema.exclusiveMaximum_ = schema.exclusiveMaximum_;
      syncedSchema.minimum_ = schema.minimum_;
      syncedSchema.exclusiveMinimum_ = schema.exclusiveMinimum_;
      syncedSchema.maxLength_ = schema.maxLength_;
      syncedSchema.minLength_ = schema.minLength_;
      syncedSchema.pattern_ = schema.pattern_;
      if (schema.tupleableItems_.has_value()) {
        const auto& indices = schema.tupleableItems_.value();
        syncedSchema.tupleableItems_ =
            std::vector<std::reference_wrapper<const SyncedSchema>>();
        for (size_t index : indices) {
          syncedSchema.tupleableItems_.value().push_back(*syncedSchemas[index]);
        }
      }
      if (schema.items_.has_value()) {
        syncedSchema.items_ = *syncedSchemas[schema.items_.value()];
      } else {
        syncedSchema.items_ = getTrueSchema();
      }
      syncedSchema.maxItems_ = schema.maxItems_;
      syncedSchema.minItems_ = schema.minItems_;
      syncedSchema.uniqueItems_ = schema.uniqueItems_;
      if (schema.contains_.has_value()) {
        syncedSchema.contains_ = *syncedSchemas[schema.contains_.value()];
      } else {
        syncedSchema.contains_ = std::nullopt;
      }
      syncedSchema.maxProperties_ = schema.maxProperties_;
      syncedSchema.minProperties_ = schema.minProperties_;
      syncedSchema.required_ = schema.required_;
      for (const auto& [propertyName, index] : schema.properties_) {
        syncedSchema.properties_.emplace(propertyName,
                                         std::cref(*syncedSchemas[index]));
      }
      if (schema.patternProperties_.has_value()) {
        syncedSchema.patternProperties_ =
            std::map<std::string, std::reference_wrapper<const SyncedSchema>>();
        for (const auto& [pattern, index] : schema.patternProperties_.value()) {
          syncedSchema.patternProperties_.value().emplace(
              pattern, *syncedSchemas[index]);
        }
      }
      if (schema.additionalProperties_.has_value()) {
        syncedSchema.additionalProperties_ =
            *syncedSchemas[schema.additionalProperties_.value()];
      } else {
        syncedSchema.additionalProperties_ = getTrueSchema();
      }
      syncedSchema.propertyDependencies_ = schema.propertyDependencies_;
      // syncedSchema.schemaDependencies_ = schema.schemaDependencies_;
      if (schema.schemaDependencies_.has_value()) {
        syncedSchema.schemaDependencies_ =
            std::map<std::string, std::reference_wrapper<const SyncedSchema>>();
        for (const auto& [propertyName, index] :
             schema.schemaDependencies_.value()) {
          syncedSchema.schemaDependencies_.value().emplace(
              propertyName, *syncedSchemas[index]);
        }
      }
      // syncedSchema.propertyNames_ = schema.propertyNames_;
      if (schema.propertyNames_.has_value()) {
        syncedSchema.propertyNames_ =
            *syncedSchemas[schema.propertyNames_.value()];
      } else {
        syncedSchema.propertyNames_ = std::nullopt;
      }
      // syncedSchema.if_ = schema.if_;
      if (schema.if_.has_value()) {
        syncedSchema.if_ = *syncedSchemas[schema.if_.value()];
      } else {
        syncedSchema.if_ = std::nullopt;
      }
      if (schema.then_.has_value()) {
        syncedSchema.then_ = *syncedSchemas[schema.then_.value()];
      } else {
        syncedSchema.then_ = std::nullopt;
      }
      if (schema.else_.has_value()) {
        syncedSchema.else_ = *syncedSchemas[schema.else_.value()];
      } else {
        syncedSchema.else_ = std::nullopt;
      }
      if (schema.allOf_.has_value()) {
        syncedSchema.allOf_ =
            std::vector<std::reference_wrapper<const SyncedSchema>>();
        for (size_t index : schema.allOf_.value()) {
          syncedSchema.allOf_.value().push_back(*syncedSchemas[index]);
        }
      }
      if (schema.anyOf_.has_value()) {
        syncedSchema.anyOf_ =
            std::vector<std::reference_wrapper<const SyncedSchema>>();
        for (size_t index : schema.anyOf_.value()) {
          syncedSchema.anyOf_.value().push_back(*syncedSchemas[index]);
        }
      }
      if (schema.oneOf_.has_value()) {
        syncedSchema.oneOf_ =
            std::vector<std::reference_wrapper<const SyncedSchema>>();
        for (size_t index : schema.oneOf_.value()) {
          syncedSchema.oneOf_.value().push_back(*syncedSchemas[index]);
        }
      }
      if (schema.not_.has_value()) {
        syncedSchema.not_ = *syncedSchemas[schema.not_.value()];
      } else {
        syncedSchema.not_ = std::nullopt;
      }
      syncedSchema.format_ = schema.format_;
      syncedSchema.default_ = schema.default_;
    }
    syncedSchemas.emplace_back(std::make_unique<SyncedSchema>(getTrueSchema()));
    return syncedSchemas;
  } catch (const std::exception& e) {
    std::cerr << "Error transitioning Indexed Synced to Synced: ";
    throw e;
  }
}

void SyncedSchema::dumpSchemas(
    std::vector<std::unique_ptr<SyncedSchema>>& schemas) {
  auto schemasDump = nlohmann::json::array();
  for (const auto& schema : schemas) {
    auto schemaDump = nlohmann::json::object();
    schemaDump["identifier"] = schema->identifier_;
    if (schema->definedAsBooleanSchema_.has_value()) {
      schemaDump["definedAsBooleanSchema"] =
          schema->definedAsBooleanSchema_.value();
    }
    if (schema->ref_.has_value()) {
      schemaDump["ref"] = schema->ref_.value().get().identifier_;
    }
    auto typeDump = nlohmann::json::array();
    for (const auto& type : schema->type_) {
      typeDump.push_back(static_cast<int>(type));
    }
    schemaDump["type"] = typeDump;

    if (schema->enum_.has_value()) {
      schemaDump["enum"] = schema->enum_.value();
    }
    if (schema->const_.has_value()) {
      schemaDump["const"] = schema->const_.value();
    }
    if (schema->description_.has_value()) {
      schemaDump["description"] = schema->description_.value();
    }
    if (schema->multipleOf_.has_value()) {
      schemaDump["multipleOf"] = schema->multipleOf_.value();
    }
    if (schema->maximum_.has_value()) {
      schemaDump["maximum"] = schema->maximum_.value();
    }
    if (schema->exclusiveMaximum_.has_value()) {
      schemaDump["exclusiveMaximum"] = schema->exclusiveMaximum_.value();
    }
    if (schema->minimum_.has_value()) {
      schemaDump["minimum"] = schema->minimum_.value();
    }
    if (schema->exclusiveMinimum_.has_value()) {
      schemaDump["exclusiveMinimum"] = schema->exclusiveMinimum_.value();
    }
    if (schema->maxLength_.has_value()) {
      schemaDump["maxLength"] = schema->maxLength_.value();
    }
    if (schema->minLength_.has_value()) {
      schemaDump["minLength"] = schema->minLength_.value();
    }
    if (schema->pattern_.has_value()) {
      schemaDump["pattern"] = schema->pattern_.value();
    }
    if (schema->tupleableItems_.has_value()) {
      auto tupleableItemsDump = nlohmann::json::array();
      for (auto& index : schema->tupleableItems_.value()) {
        tupleableItemsDump.push_back(index.get().identifier_);
      }
      schemaDump["tupleableItems"] = tupleableItemsDump;
    }
    schemaDump["items"] = schema->items_.get().identifier_;

    if (schema->maxItems_.has_value()) {
      schemaDump["maxItems"] = schema->maxItems_.value();
    }
    if (schema->minItems_.has_value()) {
      schemaDump["minItems"] = schema->minItems_.value();
    }
    if (schema->uniqueItems_.has_value()) {
      schemaDump["uniqueItems"] = schema->uniqueItems_.value();
    }
    if (schema->contains_.has_value()) {
      schemaDump["contains"] = schema->contains_.value().get().identifier_;
    }
    if (schema->maxProperties_.has_value()) {
      schemaDump["maxProperties"] = schema->maxProperties_.value();
    }
    if (schema->minProperties_.has_value()) {
      schemaDump["minProperties"] = schema->minProperties_.value();
    }
    if (schema->required_.has_value()) {
      auto requiredDump = nlohmann::json::array();
      for (const auto& required : schema->required_.value()) {
        requiredDump.push_back(required);
      }
      schemaDump["required"] = requiredDump;
    }
    auto propertiesDump = nlohmann::json::object();
    for (const auto& [propertyName, index] : schema->properties_) {
      propertiesDump[propertyName] = index.get().identifier_;
    }
    schemaDump["properties"] = propertiesDump;
    if (schema->patternProperties_.has_value()) {
      auto patternPropertiesDump = nlohmann::json::object();
      for (const auto& [pattern, index] : schema->patternProperties_.value()) {
        patternPropertiesDump[pattern] = index.get().identifier_;
      }
      schemaDump["patternProperties"] = patternPropertiesDump;
    }
    schemaDump["additionalProperties"] =
        schema->additionalProperties_.get().identifier_;
    if (schema->propertyDependencies_.has_value()) {
      auto propertyDependenciesDump = nlohmann::json::object();
      for (const auto& [propertyName, index] :
           schema->propertyDependencies_.value()) {

        propertyDependenciesDump[propertyName] = nlohmann::json::array();
        for (const auto& dependency : index) {
          propertyDependenciesDump[propertyName].push_back(dependency);
        }
      }
    }
    if (schema->propertyDependencies_.has_value()) {
      auto propertyDependenciesDump = nlohmann::json::object();
      for (const auto& [propertyName, index] :
           schema->propertyDependencies_.value()) {
        propertyDependenciesDump[propertyName] = nlohmann::json::array();
        for (const auto& dependency : index) {
          propertyDependenciesDump[propertyName].push_back(dependency);
        }
      }
      schemaDump["propertyDependencies"] = propertyDependenciesDump;
    }
    if (schema->schemaDependencies_.has_value()) {
      auto schemaDependenciesDump = nlohmann::json::object();
      for (const auto& [propertyName, index] :
           schema->schemaDependencies_.value()) {
        schemaDependenciesDump[propertyName] = index.get().identifier_;
      }
      schemaDump["schemaDependencies"] = schemaDependenciesDump;
    }
    if (schema->propertyNames_.has_value()) {
      schemaDump["propertyNames"] =
          schema->propertyNames_.value().get().identifier_;
    }
    if (schema->if_.has_value()) {
      schemaDump["if"] = schema->if_.value().get().identifier_;
    }
    if (schema->then_.has_value()) {
      schemaDump["then"] = schema->then_.value().get().identifier_;
    }
    if (schema->else_.has_value()) {
      schemaDump["else"] = schema->else_.value().get().identifier_;
    }
    if (schema->allOf_.has_value()) {
      auto allOfDump = nlohmann::json::array();
      for (const auto& index : schema->allOf_.value()) {
        allOfDump.push_back(index.get().identifier_);
      }
      schemaDump["allOf"] = allOfDump;
    }
    if (schema->anyOf_.has_value()) {
      auto anyOfDump = nlohmann::json::array();
      for (const auto& index : schema->anyOf_.value()) {
        anyOfDump.push_back(index.get().identifier_);
      }
      schemaDump["anyOf"] = anyOfDump;
    }
    if (schema->oneOf_.has_value()) {
      auto oneOfDump = nlohmann::json::array();
      for (const auto& index : schema->oneOf_.value()) {
        oneOfDump.push_back(index.get().identifier_);
      }
      schemaDump["oneOf"] = oneOfDump;
    }
    if (schema->not_.has_value()) {
      schemaDump["not"] = schema->not_.value().get().identifier_;
    }
    if (schema->format_.has_value()) {
      schemaDump["format"] = static_cast<size_t>(schema->format_.value());
    }
    if (schema->default_.has_value()) {
      schemaDump["default"] = schema->default_.value();
    }
    schemasDump.push_back(schemaDump);
  }
  std::ofstream ofs("synced.dump.json");
  ofs << schemasDump.dump(2);
  ofs.close();
}

#undef BLOCK

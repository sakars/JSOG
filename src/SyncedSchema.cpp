#include "SyncedSchema.h"
#include "StringUtils.h"
#include <cmath>
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <limits>

static std::string getHeaderDefine(const SyncedSchema& schema) {
  std::string headerDefine = schema.getHeaderFileName();
  for (char& c : headerDefine) {
    if (c == '.') {
      c = '_';
    }
  }
  std::transform(headerDefine.begin(), headerDefine.end(), headerDefine.begin(),
                 ::toupper);
  if (schema.codeProperties.get().definePrefix_.has_value()) {
    headerDefine =
        schema.codeProperties.get().definePrefix_.value() + headerDefine;
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
  // hoi hoi
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
  BLOCK << std::format("std::optional<nlohmann::json> json(const {}&);",
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

CodeBlock SyncedSchema::generateDeclaration() const {
  CodeBlock block(codeProperties.get().indent_);

  BLOCK << declarationIncludeGuardStart(*this);

  BLOCK << "";

  BLOCK << generateDependencies();

  BLOCK << "";

  BLOCK << namespaceStart(*this);

  if (!definedAsBooleanSchema_.has_value() &&
      !reinterpretables_.ref_.has_value()) {
    // Forward declarations
    BLOCK << classForwardDeclarations(*this);

    // Default value declaration
    if (default_.has_value()) {
      BLOCK << "extern const nlohmann::json default_;";
    }

    // Object declaration

    if (type_.contains(IndexedSyncedSchema::Type::Object)) {
      BLOCK << objectProperties_.objectClassDefinition(codeProperties,
                                                       getType());
    }

    // Array declaration
    if (type_.contains(IndexedSyncedSchema::Type::Array)) {
      BLOCK << arrayProperties_.arrayClassDefinition(codeProperties, getType());
    }
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

static CodeBlock jsonExportFunction(const SyncedSchema& schema) {
  CodeBlock block(schema.codeProperties.get().indent_);
  BLOCK << std::format("std::optional<nlohmann::json> json(const {}& schema) "
                       "{{",
                       schema.getType());
  {
    Indent _(block);
    BLOCK << "if (!validate(schema)) {";
    {
      Indent _(block);
      BLOCK << "return std::nullopt;";
    }
    BLOCK << "}";
    BLOCK << "return rawExport(schema);";
  }
  BLOCK << "}";
  return block;
}

static CodeBlock booleanSchemaConstruct(const SyncedSchema& schema) {
  CodeBlock block(schema.codeProperties.get().indent_);
  // Boolean schema means that the schema accepts either everything or nothing
  if (schema.definedAsBooleanSchema_.has_value()) {
    if (schema.definedAsBooleanSchema_.value()) {
      BLOCK << "return json;";

    } else {
      BLOCK << "return std::nullopt;";
    }
  }
  return block;
}

static CodeBlock refConstruct(const SyncedSchema& schema) {
  CodeBlock block(schema.codeProperties.get().indent_);
  if (schema.reinterpretables_.ref_.has_value()) {
    std::string namespace_;
    if (schema.codeProperties.get().globalNamespace_.has_value()) {
      namespace_ = "::" + schema.codeProperties.get().globalNamespace_.value();
    }
    namespace_ += std::format(
        "::{}", schema.reinterpretables_.ref_.value().get().identifier_);
    BLOCK << std::format(
        "return std::make_unique<{0}::{1}>({0}::construct(json).value());",
        namespace_, schema.reinterpretables_.ref_.value().get().identifier_);
  }
  return block;
}

static CodeBlock constConstruct(const SyncedSchema& schema) {
  CodeBlock block(schema.codeProperties.get().indent_);
  if (schema.const_.has_value()) {
    BLOCK << std::format("if(json != \"{}\"_json) {{",
                         escapeJSON(schema.const_.value()))
          << CodeBlock::inc << "return std::nullopt;" << CodeBlock::dec << "}";
  }
  return block;
}

static CodeBlock nullConstructor(const SyncedSchema& schema,
                                 const std::string& inputJsonVariableName,
                                 const std::string& outSchemaVariableName) {
  CodeBlock block(schema.codeProperties.get().indent_);
  if (schema.type_.contains(SyncedSchema::Type::Null)) {
    BLOCK << std::format("if({}.is_null()) {{", inputJsonVariableName)
          << CodeBlock::inc;
    BLOCK << std::format("{} = {}();", outSchemaVariableName,
                         schema.nullProperties_.getNullType());
    BLOCK << CodeBlock::dec << "}";
  }
  return block;
}

static CodeBlock stringConstructor(const SyncedSchema& schema,
                                   const std::string& inputJsonVariableName,
                                   const std::string& outSchemaVariableName) {
  CodeBlock block(schema.codeProperties.get().indent_);
  if (schema.type_.contains(SyncedSchema::Type::String)) {
    BLOCK << std::format("if({}.is_string()) {{", inputJsonVariableName)
          << CodeBlock::inc;
    BLOCK << std::format("{} = {}.get<{}>();", outSchemaVariableName,
                         inputJsonVariableName,
                         schema.stringProperties_.getStringType());
    BLOCK << CodeBlock::dec << "}";
  }
  return block;
}

static CodeBlock validateConstructed(const SyncedSchema& schema,
                                     const std::string& outSchemaVariableName) {
  CodeBlock block(schema.codeProperties.get().indent_);
  BLOCK << std::format("if({}.has_value()) {{", outSchemaVariableName);
  {
    Indent _(block);
    BLOCK << std::format("if (!validate({}.value())) {{",
                         outSchemaVariableName);
    {
      Indent _(block);
      BLOCK << "return std::nullopt;";
    }
    BLOCK << "}";
  }
  BLOCK << "}";
  return block;
}

static CodeBlock constructFunction(const SyncedSchema& schema) {
  CodeBlock block(schema.codeProperties.get().indent_);
  // Construct function
  std::string outSchemaVariableName = "outSchema";
  std::string inputJsonVariableName = "json";
  BLOCK << std::format(
               "std::optional<{}> construct(const nlohmann::json& {}) {{",
               schema.getType(), inputJsonVariableName)
        << CodeBlock::inc;
  {
    BLOCK << booleanSchemaConstruct(schema);
    if (!schema.definedAsBooleanSchema_.has_value()) {
      // Reference schema means that the schema is a reference to another schema
      BLOCK << refConstruct(schema);

      if (!schema.reinterpretables_.ref_.has_value()) {

        BLOCK << constConstruct(schema);

        if (schema.type_.size() == 0) {
          assert(false);
        }

        BLOCK << std::format("std::optional<{}> {} = std::nullopt;",
                             schema.getType(), outSchemaVariableName);

        BLOCK << nullConstructor(schema, inputJsonVariableName,
                                 outSchemaVariableName);

        if (schema.type_.contains(SyncedSchema::Type::Boolean)) {
          BLOCK << schema.boolProperties_.booleanConstructor(
              schema.codeProperties, inputJsonVariableName,
              outSchemaVariableName);
        }

        if (schema.type_.contains(SyncedSchema::Type::Integer)) {
          BLOCK << schema.numberProperties_.integerConstructor(
              schema.codeProperties, inputJsonVariableName,
              outSchemaVariableName);
        }

        if (schema.type_.contains(SyncedSchema::Type::Number)) {
          BLOCK << schema.numberProperties_.numberConstructor(
              schema.codeProperties, inputJsonVariableName,
              outSchemaVariableName);
        }

        if (schema.type_.contains(SyncedSchema::Type::String)) {
          BLOCK << stringConstructor(schema, inputJsonVariableName,
                                     outSchemaVariableName);
        }

        if (schema.type_.contains(SyncedSchema::Type::Array)) {
          BLOCK << schema.arrayProperties_.arrayConstructor(
              schema.codeProperties, inputJsonVariableName,
              outSchemaVariableName);
        }

        if (schema.type_.contains(SyncedSchema::Type::Object)) {
          BLOCK << schema.objectProperties_.objectConstructor(
              schema.codeProperties, inputJsonVariableName,
              outSchemaVariableName);
        }

        BLOCK << validateConstructed(schema, outSchemaVariableName);

        BLOCK << std::format("return {};", outSchemaVariableName);
      }

      BLOCK << "throw std::runtime_error(\"Unreachable, likely a bug in the "
               "autogenerated code.\");";
    }
  }
  BLOCK << CodeBlock::dec << "}";
  return block;
}

CodeBlock rawExportDefinition(const SyncedSchema& schema) {
  CodeBlock block(schema.codeProperties.get().indent_);
  BLOCK << std::format("nlohmann::json rawExport(const {}& schema) {{",
                       schema.getType());
  {
    Indent _(block);
    if (schema.reinterpretables_.ref_.has_value()) {
      BLOCK << std::format(
          "return {}::rawExport(*schema);",
          schema.reinterpretables_.ref_.value().get().getNamespace());
    } else if (schema.definedAsBooleanSchema_.has_value()) {
      if (schema.definedAsBooleanSchema_.value()) {
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

      assert(schema.type_.size() > 0);

      if (schema.type_.size() > 1) {
        if (schema.type_.contains(SyncedSchema::Type::Null)) {
          BLOCK << "if (std::holds_alternative<std::monostate>(schema)) {";
          {
            Indent _(block);
            BLOCK << "return nullptr;";
          }
          BLOCK << "}";
        }
        if (schema.type_.contains(SyncedSchema::Type::Number)) {
          BLOCK << "if (std::holds_alternative<double>(schema)) {";
          {
            Indent _(block);
            BLOCK << "return std::get<double>(schema);";
          }
          BLOCK << "}";
        }
        if (schema.type_.contains(SyncedSchema::Type::Integer)) {
          BLOCK << std::format("if (std::holds_alternative<{}>(schema)) {{",
                               schema.numberProperties_.getIntegerType());
          {
            Indent _(block);
            BLOCK << std::format("return std::get<{}>(schema);",
                                 schema.numberProperties_.getIntegerType());
          }
          BLOCK << "}";
        }
        if (schema.type_.contains(SyncedSchema::Type::String)) {
          BLOCK << "if (std::holds_alternative<std::string>(schema)) {";
          {
            Indent _(block);
            BLOCK << "return std::get<std::string>(schema);";
          }
          BLOCK << "}";
        }
        if (schema.type_.contains(SyncedSchema::Type::Boolean)) {
          BLOCK << "if (std::holds_alternative<bool>(schema)) {";
          {
            Indent _(block);
            BLOCK << "return std::get<bool>(schema);";
          }
          BLOCK << "}";
        }
        if (schema.type_.contains(SyncedSchema::Type::Array)) {
          BLOCK << "if (std::holds_alternative<Array>(schema)) {";
          {
            Indent _(block);
            BLOCK << "auto json = nlohmann::json::array();";
            BLOCK << "auto& array = std::get<Array>(schema);";
            if (schema.arrayProperties_.tupleableItems_.has_value()) {
              for (size_t i = 0;
                   i < schema.arrayProperties_.tupleableItems_->size(); i++) {
                const auto& item =
                    (*schema.arrayProperties_.tupleableItems_)[i].get();
                if (schema.codeProperties.get()
                        .minItemsMakeTupleableRequired_ &&
                    schema.arrayProperties_.minItems_.has_value() &&
                    i < schema.arrayProperties_.minItems_.value()) {
                  BLOCK << std::format(
                      "json.push_back({}::rawExport(array.item{}"
                      "));",
                      item.identifier_, i);
                } else {
                  BLOCK << std::format("if(array.item{}.has_value()) {{", i);
                  BLOCK << CodeBlock::inc;
                  BLOCK << std::format(
                      "json.push_back({}::rawExport(array.item{}"
                      ".value()));",
                      item.identifier_, i);
                  BLOCK << CodeBlock::dec << "}";
                }
              }
            }
            BLOCK << "for (const auto& item : array.items) {";
            {
              Indent _(block);
              BLOCK << std::format(
                  "json.push_back({}::rawExport(item));",
                  schema.arrayProperties_.items_.get().identifier_);
            }
            BLOCK << "}";
            BLOCK << "return json;";
          }
          BLOCK << "}";
        }
        if (schema.type_.contains(SyncedSchema::Type::Object)) {
          BLOCK << "if (std::holds_alternative<Object>(schema)) {";
          {
            Indent _(block);
            BLOCK << "auto& object = std::get<Object>(schema);";
            BLOCK << "auto json = nlohmann::json::object();";
            for (const auto& [propertyName, propertySchema] :
                 schema.objectProperties_.properties_) {
              const auto is_required =
                  schema.objectProperties_.required_.has_value() &&
                  schema.objectProperties_.required_.value().count(
                      propertyName) > 0;
              if (!is_required) {
                BLOCK << std::format("if (object.{}.has_value()) {{",
                                     sanitizeString(propertyName) + "_")
                      << CodeBlock::inc;
              }
              BLOCK << std::format("json[\"{}\"] = {}::rawExport(object.{}{});",
                                   propertyName,
                                   propertySchema.get().identifier_,
                                   sanitizeString(propertyName) + "_",
                                   is_required ? "" : ".value()");

              if (!is_required) {
                BLOCK << CodeBlock::dec << "}";
              }
            }
            if (schema.objectProperties_.additionalProperties_.get()
                    .definedAsBooleanSchema_.value_or(true) != false) {
              BLOCK << "for (const auto& [key, value] : "
                       "object.additionalProperties) {";
              {
                Indent _(block);
                BLOCK << std::format(
                    "json[key] = {}::rawExport(value);",
                    schema.objectProperties_.additionalProperties_.get()
                        .identifier_);
              }
              BLOCK << "}";
            }
            BLOCK << "return json;";
          }
          BLOCK << "}";
        }

      } else {
        const auto type = *schema.type_.begin();
        if (type == SyncedSchema::Type::Null) {
          BLOCK << "return nullptr;";
        } else if (type == SyncedSchema::Type::Number) {
          BLOCK << "return schema;";
        } else if (type == SyncedSchema::Type::Integer) {
          BLOCK << "return schema;";
        } else if (type == SyncedSchema::Type::String) {
          BLOCK << "return schema;";
        } else if (type == SyncedSchema::Type::Boolean) {
          BLOCK << "return schema;";
        } else if (type == SyncedSchema::Type::Array) {
          BLOCK << "auto json = nlohmann::json::array();";
          BLOCK << "auto& array = schema;";
          if (schema.arrayProperties_.tupleableItems_.has_value()) {
            for (size_t i = 0;
                 i < schema.arrayProperties_.tupleableItems_->size(); i++) {
              const auto& item =
                  (*schema.arrayProperties_.tupleableItems_)[i].get();
              if (schema.codeProperties.get().minItemsMakeTupleableRequired_ &&
                  schema.arrayProperties_.minItems_.has_value() &&
                  i < schema.arrayProperties_.minItems_.value()) {
                BLOCK << std::format(
                    "json.push_back({}::rawExport(array.item{}));",
                    item.identifier_, i);
              } else {
                BLOCK << std::format("if(array.item{}.has_value()) {{", i);
                BLOCK << CodeBlock::inc;
                BLOCK << std::format(
                    "json.push_back({}::rawExport(array.item{}.value()));",
                    item.identifier_, i);
                BLOCK << CodeBlock::dec << "}";
              }
            }
          }
          BLOCK << "for (const auto& item : array.items) {";
          {
            Indent _(block);
            BLOCK << std::format(
                "json.push_back({}::rawExport(item));",
                schema.arrayProperties_.items_.get().identifier_);
          }
          BLOCK << "}";
          BLOCK << "return json;";
        } else if (type == SyncedSchema::Type::Object) {
          BLOCK << "auto& object = schema;";
          BLOCK << "auto json = nlohmann::json::object();";
          for (const auto& [propertyName, propertySchema] :
               schema.objectProperties_.properties_) {
            const auto is_required =
                schema.objectProperties_.required_.has_value() &&
                schema.objectProperties_.required_.value().count(propertyName) >
                    0;
            if (!is_required) {
              BLOCK << std::format("if (object.{}.has_value()) {{",
                                   sanitizeString(propertyName) + "_")
                    << CodeBlock::inc;
            }
            BLOCK << std::format("json[\"{}\"] = {}::rawExport(object.{}{});",
                                 propertyName, propertySchema.get().identifier_,
                                 sanitizeString(propertyName) + "_",
                                 is_required ? "" : ".value()");

            if (!is_required) {
              BLOCK << CodeBlock::dec << "}";
            }
          }
          if (schema.objectProperties_.additionalProperties_.get()
                  .definedAsBooleanSchema_.value_or(true) != false) {
            BLOCK << "for (const auto& [key, value] : "
                     "object.additionalProperties) {";
            {
              Indent _(block);
              BLOCK << std::format(
                  "json[key] = {}::rawExport(value);",
                  schema.objectProperties_.additionalProperties_.get()
                      .identifier_);
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
  return block;
}

static CodeBlock validateDefinition(const SyncedSchema& schema) {
  using Type = SyncedSchema::Type;
  CodeBlock block(schema.codeProperties.get().indent_);

  BLOCK << std::format("bool validate(const {}& schema) {{", schema.getType());
  {
    Indent _(block);
    if (schema.reinterpretables_.ref_.has_value()) {
      BLOCK << std::format(
          "return {}::validate(*schema);",
          schema.reinterpretables_.ref_.value().get().getNamespace());
    } else if (schema.definedAsBooleanSchema_.has_value()) {
      if (schema.definedAsBooleanSchema_.value()) {
        BLOCK << "return true;";
      } else {
        BLOCK << "return false;";
      }
    } else {
      if (schema.const_.has_value()) {
        BLOCK << std::format("if (schema != \"{}\"_json) {{",
                             escapeJSON(schema.const_.value()))
              << CodeBlock::inc << "return false;" << CodeBlock::dec << "}";
      }

      assert(schema.type_.size() > 0);

      bool isSingleType = schema.type_.size() == 1;
      if (schema.type_.contains(Type::Null)) {
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
      if (schema.type_.contains(Type::Boolean)) {
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
      if (schema.type_.contains(Type::Integer) ||
          schema.type_.contains(Type::Number)) {
        // Construct a lambda function with auto parameter to be able to
        // validate double and any integer type
        BLOCK << "const auto numericValidator = [](auto numericValue) -> "
                 "bool {";
        {
          Indent _(block);
          // Integer/number validation
          // multipleOf
          if (schema.numberProperties_.multipleOf_.has_value()) {
            BLOCK << std::format("if ((std::abs(numericValue / {0}) - "
                                 "static_cast<long>(std::abs(numericValue / "
                                 "{0})) ) < 0.0000001) {{",
                                 schema.numberProperties_.multipleOf_.value())
                  << CodeBlock::inc << "return false;" << CodeBlock::dec << "}";
          }

          // maximum
          if (schema.numberProperties_.maximum_.has_value()) {
            BLOCK << std::format("if (numericValue > {}) {{",
                                 schema.numberProperties_.maximum_.value())
                  << CodeBlock::inc << "return false;" << CodeBlock::dec << "}";
          }

          // exclusiveMaximum
          if (schema.numberProperties_.exclusiveMaximum_.has_value()) {
            BLOCK << std::format(
                         "if (numericValue >= {}) {{",
                         schema.numberProperties_.exclusiveMaximum_.value())
                  << CodeBlock::inc << "return false;" << CodeBlock::dec << "}";
          }

          // minimum
          if (schema.numberProperties_.minimum_.has_value()) {
            BLOCK << std::format("if (numericValue < {}) {{",
                                 schema.numberProperties_.minimum_.value())
                  << CodeBlock::inc << "return false;" << CodeBlock::dec << "}";
          }

          // exclusiveMinimum
          if (schema.numberProperties_.exclusiveMinimum_.has_value()) {
            BLOCK << std::format(
                         "if (numericValue <= {}) {{",
                         schema.numberProperties_.exclusiveMinimum_.value())
                  << CodeBlock::inc << "return false;" << CodeBlock::dec << "}";
          }
          BLOCK << "return true;";
        }
        BLOCK << "};";
        if (schema.type_.contains(Type::Integer)) {
          if (!isSingleType) {
            BLOCK << std::format("if (std::holds_alternative<{}>(schema)) {{",
                                 schema.numberProperties_.getIntegerType());
            BLOCK << CodeBlock::inc;
            BLOCK << std::format(
                "const auto& integerValue = std::get<{}>(schema);",
                schema.numberProperties_.getIntegerType());
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
      if (schema.type_.contains(Type::String)) {
        if (!isSingleType) {
          BLOCK << "if (std::holds_alternative<std::string>(schema)) {";
          BLOCK << CodeBlock::inc;
          BLOCK << "const auto& stringValue = std::get<std::string>(schema);";
        } else {
          BLOCK << "{" << CodeBlock::inc;
          BLOCK << "const auto& stringValue = schema;";
        }
        // MaxLength
        if (schema.stringProperties_.maxLength_.has_value()) {
          BLOCK << std::format("if (stringValue.size() > {}) {{",
                               schema.stringProperties_.maxLength_.value())
                << CodeBlock::inc << "return false;" << CodeBlock::dec << "}";
        }

        // MinLength
        if (schema.stringProperties_.minLength_.has_value()) {
          BLOCK << std::format("if (stringValue.size() < {}) {{",
                               schema.stringProperties_.minLength_.value())
                << CodeBlock::inc << "return false;" << CodeBlock::dec << "}";
        }

        // Pattern
        if (schema.stringProperties_.pattern_.has_value()) {
          BLOCK << std::format("if (!std::regex_match(stringValue, std::regex("
                               "\"{}\", std::regex_constants::ECMAScript))) {{",
                               escapeJSONString(
                                   schema.stringProperties_.pattern_.value()))
                << CodeBlock::inc << "return false;" << CodeBlock::dec << "}";
        }
        BLOCK << CodeBlock::dec << "}";
      }
      if (schema.type_.contains(Type::Array)) {
        // TODO: Array needs tupleable items validation
        if (!isSingleType) {
          BLOCK << "if (std::holds_alternative<Array>(schema)) {";
          BLOCK << CodeBlock::inc;
          BLOCK << "const auto& arrayValue = std::get<Array>(schema);";
        } else {
          BLOCK << "{" << CodeBlock::inc;
          BLOCK << "const auto& arrayValue = schema;";
        }
        // MaxItems
        if (schema.arrayProperties_.maxItems_.has_value()) {
          BLOCK << std::format("if (arrayValue.items.size() > {}) {{",
                               schema.arrayProperties_.maxItems_.value())
                << CodeBlock::inc << "return false;" << CodeBlock::dec << "}";
        }

        // MinItems
        if (schema.arrayProperties_.minItems_.has_value()) {
          BLOCK << std::format("if (arrayValue.items.size() < {}) {{",
                               schema.arrayProperties_.minItems_.value())
                << CodeBlock::inc << "return false;";
          BLOCK << CodeBlock::dec << "}";
        }

        // UniqueItems
        if (schema.arrayProperties_.uniqueItems_.has_value() &&
            schema.arrayProperties_.uniqueItems_.value()) {
          BLOCK << "{";
          {
            Indent _(block);
            BLOCK << "std::set<std::string> uniqueItems;";
            const auto tupleableItems =
                schema.arrayProperties_.tupleableItems_.value_or(
                    std::vector<std::reference_wrapper<const SyncedSchema>>());
            for (size_t i = 0; i < tupleableItems.size(); i++) {
              BLOCK << std::format("if (arrayValue.item{}.has_value()) {{", i);
              {
                Indent _(block);
                BLOCK << std::format(
                    "const std::string item = {}::json("
                    "arrayValue.item{}.value()).dump(0);",
                    (*schema.arrayProperties_.tupleableItems_)[i]
                        .get()
                        .identifier_,
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
            if (schema.arrayProperties_.items_.get()
                    .definedAsBooleanSchema_.value_or(true) == false) {
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
        if (schema.arrayProperties_.contains_.has_value()) {
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

      if (schema.type_.contains(Type::Object)) {
        if (!isSingleType) {
          BLOCK << "if (std::holds_alternative<Object>(schema)) {";
          BLOCK << CodeBlock::inc;
          BLOCK << "const auto& objectValue = std::get<Object>(schema);";
        } else {
          BLOCK << "{" << CodeBlock::inc;
          BLOCK << "const auto& objectValue = schema;";
        }
        // MaxProperties / MinProperties
        if (schema.objectProperties_.maxProperties_.has_value() ||
            schema.objectProperties_.minProperties_.has_value()) {
          BLOCK << "{";
          {
            Indent _(block);
            // Count the number of properties
            BLOCK << "size_t propertyCount = schema.properties.size();";
            for (const auto& [propertyName, propertySchema] :
                 schema.objectProperties_.properties_) {
              // If the property is required, then a representation of it must
              // have already been created, meaning there is nothing to check
              // and we can simply increment the property count
              if (schema.objectProperties_.required_.has_value() &&
                  schema.objectProperties_.required_.value().count(
                      propertyName) > 0) {
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
            if (schema.objectProperties_.maxProperties_.has_value()) {
              BLOCK << std::format(
                           "if (propertyCount > {}) {{",
                           schema.objectProperties_.maxProperties_.value())
                    << CodeBlock::inc;
              BLOCK << "return false;";
              BLOCK << CodeBlock::dec << "}";
            }
            if (schema.objectProperties_.minProperties_.has_value()) {
              BLOCK << std::format(
                           "if (propertyCount < {}) {{",
                           schema.objectProperties_.minProperties_.value())
                    << CodeBlock::inc;
              BLOCK << "return false;";
              BLOCK << CodeBlock::dec << "}";
            }
          }
          BLOCK << "}";
        }

        // Required
        if (schema.objectProperties_.required_.has_value()) {
          std::set<std::string> requiredAdditionalProperties =
              schema.objectProperties_.required_.value();
          for (const auto& [propertyName, _] :
               schema.objectProperties_.properties_) {
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
        if (schema.objectProperties_.patternProperties_.has_value()) {
          for (const auto& [pattern, patternSchema] :
               schema.objectProperties_.patternProperties_.value()) {
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
                    schema.objectProperties_.additionalProperties_.get()
                        .identifier_);
                BLOCK << std::format("const auto constructedValue = "
                                     "{}::construct(rawValue);",
                                     patternSchema.get().identifier_);
                BLOCK << std::format(
                    "if (!{0}::validate(constructedValue.value())) {{",
                    patternSchema.get().identifier_);
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

  return block;
}

CodeBlock SyncedSchema::generateDefinition() const {
  CodeBlock block(codeProperties.get().indent_);
  BLOCK << std::format("#include \"{}\"", getHeaderFileName());

  BLOCK << namespaceStart(*this);

  // Default value definition
  if (default_.has_value()) {
    BLOCK << std::format("const nlohmann::json default_ = \"{}\"_json;",
                         escapeJSON(default_.value()));
  }

  // Construct function
  BLOCK << constructFunction(*this);

  // export function
  BLOCK << rawExportDefinition(*this);

  BLOCK << jsonExportFunction(*this);

  // Validator function
  BLOCK << validateDefinition(*this);

  BLOCK << namespaceEnd(*this);

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
  if (reinterpretables_.ref_.has_value()) {
    dependencies.insert(&reinterpretables_.ref_.value().get());
  }
  for (const auto& item : arrayProperties_.tupleableItems_.value_or(
           std::vector<std::reference_wrapper<const SyncedSchema>>())) {
    dependencies.insert(&item.get());
  }
  dependencies.insert(&arrayProperties_.items_.get());
  if (arrayProperties_.contains_.has_value()) {
    dependencies.insert(&arrayProperties_.contains_.value().get());
  }
  for (const auto& [_, schema] : objectProperties_.properties_) {
    dependencies.insert(&schema.get());
  }
  if (objectProperties_.patternProperties_.has_value()) {
    for (const auto& [_, schema] :
         objectProperties_.patternProperties_.value()) {
      dependencies.insert(&schema.get());
    }
  }
  dependencies.insert(&objectProperties_.additionalProperties_.get());

  if (objectProperties_.schemaDependencies_.has_value()) {
    for (const auto& [_, schema] :
         objectProperties_.schemaDependencies_.value()) {
      dependencies.insert(&schema.get());
    }
  }
  if (objectProperties_.propertyNames_.has_value()) {
    dependencies.insert(&objectProperties_.propertyNames_.value().get());
  }
  if (reinterpretables_.if_.has_value()) {
    dependencies.insert(&reinterpretables_.if_.value().get());
  }
  if (reinterpretables_.then_.has_value()) {
    dependencies.insert(&reinterpretables_.then_.value().get());
  }
  if (reinterpretables_.else_.has_value()) {
    dependencies.insert(&reinterpretables_.else_.value().get());
  }
  if (reinterpretables_.allOf_.has_value()) {
    for (const auto& schema : reinterpretables_.allOf_.value()) {
      dependencies.insert(&schema.get());
    }
  }
  if (reinterpretables_.anyOf_.has_value()) {
    for (const auto& schema : reinterpretables_.anyOf_.value()) {
      dependencies.insert(&schema.get());
    }
  }
  if (reinterpretables_.oneOf_.has_value()) {
    for (const auto& schema : reinterpretables_.oneOf_.value()) {
      dependencies.insert(&schema.get());
    }
  }
  if (reinterpretables_.not_.has_value()) {
    dependencies.insert(&reinterpretables_.not_.value().get());
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
        !dependency->reinterpretables_.ref_.has_value()) {
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

std::string SyncedSchema::getNamespace() const {
  std::string namespaceString = "::";
  if (codeProperties.get().globalNamespace_.has_value()) {
    namespaceString += codeProperties.get().globalNamespace_.value() + "::";
  }
  namespaceString += identifier_;
  return namespaceString;
}

std::string SyncedSchema::getHeaderFileName() const {
  if (identifier_.empty()) {
    return "";
  }
  return identifier_ + ".h";
}

std::string SyncedSchema::getSourceFileName() const {
  if (identifier_.empty()) {
    return "";
  }
  return identifier_ + ".cpp";
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
  if (reinterpretables_.ref_.has_value()) {
    return std::format("std::unique_ptr<{}>",
                       reinterpretables_.ref_.value().get().getType());
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
      return nullProperties_.getNullType();
    case Type::Boolean:
      return getBooleanType();
    case Type::Object:
      return objectProperties_.getObjectType(getNamespaceLocation());
    case Type::Array:
      return arrayProperties_.getArrayType(getNamespaceLocation());
    case Type::Number:
      return numberProperties_.getNumberType();
    case Type::String:
      return stringProperties_.getStringType();
    case Type::Integer:
      return numberProperties_.getIntegerType();
    }
  }
  std::string type = "std::variant<";
  for (auto it = types.begin(); it != types.end(); it++) {
    switch (*it) {
    case Type::Null:
      type += nullProperties_.getNullType();
      break;
    case Type::Boolean:
      type += getBooleanType();
      break;
    case Type::Object:
      type += objectProperties_.getObjectType(getNamespaceLocation());
      break;
    case Type::Array:
      type += arrayProperties_.getArrayType(getNamespaceLocation());
      break;
    case Type::Number:
      type += numberProperties_.getNumberType();
      break;
    case Type::String:
      type += stringProperties_.getStringType();
      break;
    case Type::Integer:
      type += numberProperties_.getIntegerType();
    }
    if (std::next(it) != types.end()) {
      type += ", ";
    }
  }
  type += ">";
  return type;
}

std::string SyncedSchema::getNamespaceLocation() const {
  if (codeProperties.get().globalNamespace_.has_value()) {
    return std::format(
        "::{}::{}", codeProperties.get().globalNamespace_.value(), identifier_);
  }
  return std::format("::{}", identifier_);
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
      if (schema.reinterpretables_.ref_.has_value()) {
        syncedSchema.reinterpretables_.ref_ =
            *syncedSchemas[schema.reinterpretables_.ref_.value()];
      } else {
        syncedSchema.reinterpretables_.ref_ = std::nullopt;
      }
      syncedSchema.type_ = schema.type_.value_or(std::set<Type>{
          Type::Object, Type::Array, Type::Boolean, Type::Integer, Type::Null,
          Type::Number, Type::String});
      syncedSchema.enum_ = schema.enum_;
      syncedSchema.const_ = schema.const_;
      syncedSchema.description_ = schema.description_;
      syncedSchema.numberProperties_ = schema.numberProperties_;
      syncedSchema.stringProperties_ = schema.stringProperties_;
      syncedSchema.arrayProperties_ =
          ArrayProperties(schema.arrayProperties_, syncedSchemas);
      syncedSchema.objectProperties_.maxProperties_ =
          schema.objectProperties_.maxProperties_;
      syncedSchema.objectProperties_.minProperties_ =
          schema.objectProperties_.minProperties_;
      syncedSchema.objectProperties_.required_ =
          schema.objectProperties_.required_;
      for (const auto& [propertyName, index] :
           schema.objectProperties_.properties_) {
        syncedSchema.objectProperties_.properties_.emplace(
            propertyName, std::cref(*syncedSchemas[index]));
      }
      if (schema.objectProperties_.patternProperties_.has_value()) {
        syncedSchema.objectProperties_.patternProperties_ =
            std::map<std::string, std::reference_wrapper<const SyncedSchema>>();
        for (const auto& [pattern, index] :
             schema.objectProperties_.patternProperties_.value()) {
          syncedSchema.objectProperties_.patternProperties_.value().emplace(
              pattern, *syncedSchemas[index]);
        }
      }
      if (schema.objectProperties_.additionalProperties_.has_value()) {
        syncedSchema.objectProperties_.additionalProperties_ =
            *syncedSchemas[schema.objectProperties_.additionalProperties_
                               .value()];
      } else {
        syncedSchema.objectProperties_.additionalProperties_ = getTrueSchema();
      }
      syncedSchema.objectProperties_.propertyDependencies_ =
          schema.objectProperties_.propertyDependencies_;
      // syncedSchema.objectProperties_.schemaDependencies_ =
      // schema.schemaDependencies_;
      if (schema.objectProperties_.schemaDependencies_.has_value()) {
        syncedSchema.objectProperties_.schemaDependencies_ =
            std::map<std::string, std::reference_wrapper<const SyncedSchema>>();
        for (const auto& [propertyName, index] :
             schema.objectProperties_.schemaDependencies_.value()) {
          syncedSchema.objectProperties_.schemaDependencies_.value().emplace(
              propertyName, *syncedSchemas[index]);
        }
      }
      // syncedSchema.objectProperties_.propertyNames_ = schema.propertyNames_;
      if (schema.objectProperties_.propertyNames_.has_value()) {
        syncedSchema.objectProperties_.propertyNames_ =
            *syncedSchemas[schema.objectProperties_.propertyNames_.value()];
      } else {
        syncedSchema.objectProperties_.propertyNames_ = std::nullopt;
      }
      // syncedSchema.if_ = schema.if_;
      if (schema.reinterpretables_.if_.has_value()) {
        syncedSchema.reinterpretables_.if_ =
            *syncedSchemas[schema.reinterpretables_.if_.value()];
      } else {
        syncedSchema.reinterpretables_.if_ = std::nullopt;
      }
      if (schema.reinterpretables_.then_.has_value()) {
        syncedSchema.reinterpretables_.then_ =
            *syncedSchemas[schema.reinterpretables_.then_.value()];
      } else {
        syncedSchema.reinterpretables_.then_ = std::nullopt;
      }
      if (schema.reinterpretables_.else_.has_value()) {
        syncedSchema.reinterpretables_.else_ =
            *syncedSchemas[schema.reinterpretables_.else_.value()];
      } else {
        syncedSchema.reinterpretables_.else_ = std::nullopt;
      }
      if (schema.reinterpretables_.allOf_.has_value()) {
        syncedSchema.reinterpretables_.allOf_ =
            std::vector<std::reference_wrapper<const SyncedSchema>>();
        for (size_t index : schema.reinterpretables_.allOf_.value()) {
          syncedSchema.reinterpretables_.allOf_.value().push_back(
              *syncedSchemas[index]);
        }
      }
      if (schema.reinterpretables_.anyOf_.has_value()) {
        syncedSchema.reinterpretables_.anyOf_ =
            std::vector<std::reference_wrapper<const SyncedSchema>>();
        for (size_t index : schema.reinterpretables_.anyOf_.value()) {
          syncedSchema.reinterpretables_.anyOf_.value().push_back(
              *syncedSchemas[index]);
        }
      }
      if (schema.reinterpretables_.oneOf_.has_value()) {
        syncedSchema.reinterpretables_.oneOf_ =
            std::vector<std::reference_wrapper<const SyncedSchema>>();
        for (size_t index : schema.reinterpretables_.oneOf_.value()) {
          syncedSchema.reinterpretables_.oneOf_.value().push_back(
              *syncedSchemas[index]);
        }
      }
      if (schema.reinterpretables_.not_.has_value()) {
        syncedSchema.reinterpretables_.not_ =
            *syncedSchemas[schema.reinterpretables_.not_.value()];
      } else {
        syncedSchema.reinterpretables_.not_ = std::nullopt;
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
    std::vector<std::unique_ptr<SyncedSchema>>& schemas,
    std::filesystem::path outputDirectory) {
  auto schemasDump = nlohmann::json::array();
  for (const auto& schema : schemas) {
    auto schemaDump = nlohmann::json::object();
    schemaDump["identifier"] = schema->identifier_;
    if (schema->definedAsBooleanSchema_.has_value()) {
      schemaDump["definedAsBooleanSchema"] =
          schema->definedAsBooleanSchema_.value();
    }
    if (schema->reinterpretables_.ref_.has_value()) {
      schemaDump["ref"] =
          schema->reinterpretables_.ref_.value().get().identifier_;
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
    if (schema->numberProperties_.multipleOf_.has_value()) {
      schemaDump["multipleOf"] = schema->numberProperties_.multipleOf_.value();
    }
    if (schema->numberProperties_.maximum_.has_value()) {
      schemaDump["maximum"] = schema->numberProperties_.maximum_.value();
    }
    if (schema->numberProperties_.exclusiveMaximum_.has_value()) {
      schemaDump["exclusiveMaximum"] =
          schema->numberProperties_.exclusiveMaximum_.value();
    }
    if (schema->numberProperties_.minimum_.has_value()) {
      schemaDump["minimum"] = schema->numberProperties_.minimum_.value();
    }
    if (schema->numberProperties_.exclusiveMinimum_.has_value()) {
      schemaDump["exclusiveMinimum"] =
          schema->numberProperties_.exclusiveMinimum_.value();
    }
    if (schema->stringProperties_.maxLength_.has_value()) {
      schemaDump["maxLength"] = schema->stringProperties_.maxLength_.value();
    }
    if (schema->stringProperties_.minLength_.has_value()) {
      schemaDump["minLength"] = schema->stringProperties_.minLength_.value();
    }
    if (schema->stringProperties_.pattern_.has_value()) {
      schemaDump["pattern"] = schema->stringProperties_.pattern_.value();
    }
    if (schema->arrayProperties_.tupleableItems_.has_value()) {
      auto tupleableItemsDump = nlohmann::json::array();
      for (auto& index : schema->arrayProperties_.tupleableItems_.value()) {
        tupleableItemsDump.push_back(index.get().identifier_);
      }
      schemaDump["tupleableItems"] = tupleableItemsDump;
    }
    schemaDump["items"] = schema->arrayProperties_.items_.get().identifier_;

    if (schema->arrayProperties_.maxItems_.has_value()) {
      schemaDump["maxItems"] = schema->arrayProperties_.maxItems_.value();
    }
    if (schema->arrayProperties_.minItems_.has_value()) {
      schemaDump["minItems"] = schema->arrayProperties_.minItems_.value();
    }
    if (schema->arrayProperties_.uniqueItems_.has_value()) {
      schemaDump["uniqueItems"] = schema->arrayProperties_.uniqueItems_.value();
    }
    if (schema->arrayProperties_.contains_.has_value()) {
      schemaDump["contains"] =
          schema->arrayProperties_.contains_.value().get().identifier_;
    }
    if (schema->objectProperties_.maxProperties_.has_value()) {
      schemaDump["maxProperties"] =
          schema->objectProperties_.maxProperties_.value();
    }
    if (schema->objectProperties_.minProperties_.has_value()) {
      schemaDump["minProperties"] =
          schema->objectProperties_.minProperties_.value();
    }
    if (schema->objectProperties_.required_.has_value()) {
      auto requiredDump = nlohmann::json::array();
      for (const auto& required : schema->objectProperties_.required_.value()) {
        requiredDump.push_back(required);
      }
      schemaDump["required"] = requiredDump;
    }
    auto propertiesDump = nlohmann::json::object();
    for (const auto& [propertyName, index] :
         schema->objectProperties_.properties_) {
      propertiesDump[propertyName] = index.get().identifier_;
    }
    schemaDump["properties"] = propertiesDump;
    if (schema->objectProperties_.patternProperties_.has_value()) {
      auto patternPropertiesDump = nlohmann::json::object();
      for (const auto& [pattern, index] :
           schema->objectProperties_.patternProperties_.value()) {
        patternPropertiesDump[pattern] = index.get().identifier_;
      }
      schemaDump["patternProperties"] = patternPropertiesDump;
    }
    schemaDump["additionalProperties"] =
        schema->objectProperties_.additionalProperties_.get().identifier_;
    if (schema->objectProperties_.propertyDependencies_.has_value()) {
      auto propertyDependenciesDump = nlohmann::json::object();
      for (const auto& [propertyName, index] :
           schema->objectProperties_.propertyDependencies_.value()) {

        propertyDependenciesDump[propertyName] = nlohmann::json::array();
        for (const auto& dependency : index) {
          propertyDependenciesDump[propertyName].push_back(dependency);
        }
      }
    }
    if (schema->objectProperties_.propertyDependencies_.has_value()) {
      auto propertyDependenciesDump = nlohmann::json::object();
      for (const auto& [propertyName, index] :
           schema->objectProperties_.propertyDependencies_.value()) {
        propertyDependenciesDump[propertyName] = nlohmann::json::array();
        for (const auto& dependency : index) {
          propertyDependenciesDump[propertyName].push_back(dependency);
        }
      }
      schemaDump["propertyDependencies"] = propertyDependenciesDump;
    }
    if (schema->objectProperties_.schemaDependencies_.has_value()) {
      auto schemaDependenciesDump = nlohmann::json::object();
      for (const auto& [propertyName, index] :
           schema->objectProperties_.schemaDependencies_.value()) {
        schemaDependenciesDump[propertyName] = index.get().identifier_;
      }
      schemaDump["schemaDependencies"] = schemaDependenciesDump;
    }
    if (schema->objectProperties_.propertyNames_.has_value()) {
      schemaDump["propertyNames"] =
          schema->objectProperties_.propertyNames_.value().get().identifier_;
    }
    if (schema->reinterpretables_.if_.has_value()) {
      schemaDump["if"] =
          schema->reinterpretables_.if_.value().get().identifier_;
    }
    if (schema->reinterpretables_.then_.has_value()) {
      schemaDump["then"] =
          schema->reinterpretables_.then_.value().get().identifier_;
    }
    if (schema->reinterpretables_.else_.has_value()) {
      schemaDump["else"] =
          schema->reinterpretables_.else_.value().get().identifier_;
    }
    if (schema->reinterpretables_.allOf_.has_value()) {
      auto allOfDump = nlohmann::json::array();
      for (const auto& index : schema->reinterpretables_.allOf_.value()) {
        allOfDump.push_back(index.get().identifier_);
      }
      schemaDump["allOf"] = allOfDump;
    }
    if (schema->reinterpretables_.anyOf_.has_value()) {
      auto anyOfDump = nlohmann::json::array();
      for (const auto& index : schema->reinterpretables_.anyOf_.value()) {
        anyOfDump.push_back(index.get().identifier_);
      }
      schemaDump["anyOf"] = anyOfDump;
    }
    if (schema->reinterpretables_.oneOf_.has_value()) {
      auto oneOfDump = nlohmann::json::array();
      for (const auto& index : schema->reinterpretables_.oneOf_.value()) {
        oneOfDump.push_back(index.get().identifier_);
      }
      schemaDump["oneOf"] = oneOfDump;
    }
    if (schema->reinterpretables_.not_.has_value()) {
      schemaDump["not"] =
          schema->reinterpretables_.not_.value().get().identifier_;
    }
    if (schema->format_.has_value()) {
      schemaDump["format"] = static_cast<size_t>(schema->format_.value());
    }
    if (schema->default_.has_value()) {
      schemaDump["default"] = schema->default_.value();
    }
    schemasDump.push_back(schemaDump);
  }
  std::ofstream ofs(outputDirectory / "synced.dump.json");
  ofs << schemasDump.dump(2);
  ofs.close();
}

#include "SyncedSchema/ObjectProperties.h"
#include "CodeBlock.h"
#include "CodeProperties.h"
#include "StringUtils.h"
#include "SyncedSchema.h"
#include <format>

std::string
ObjectProperties::getObjectType(std::string namespaceLocation) const {
  return std::format("{}::Object", namespaceLocation);
}

CodeBlock ObjectProperties::objectConstructor(
    const CodeProperties& codeProperties,
    const std::string& inputJsonVariableName,
    const std::string& outSchemaVariableName) const {
  CodeBlock block(codeProperties.indent_);
  BLOCK << std::format("if({}.is_object()) {{", inputJsonVariableName);
  {
    Indent _(block);
    BLOCK << "auto object = Object();";
    bool hasPropertyCountLimits =
        maxProperties_.has_value() || minProperties_.has_value();
    if (hasPropertyCountLimits) {
      BLOCK << std::format("size_t propertyCount = {}.size();",
                           inputJsonVariableName);
    }
    if (maxProperties_.has_value()) {
      BLOCK << std::format("if(propertyCount > {}) {{", maxProperties_.value())
            << CodeBlock::inc << "return std::nullopt;" << CodeBlock::dec
            << "}";
    }
    if (minProperties_.has_value()) {
      BLOCK << std::format("if(propertyCount < {}) {{", minProperties_.value())
            << CodeBlock::inc << "return std::nullopt;" << CodeBlock::dec
            << "}";
    }
    BLOCK << "static const std::set<std::string> properties = {";
    for (const auto& [propertyName, schema] : properties_) {
      Indent _(block);
      BLOCK << std::format("\"{}\",", propertyName);
    }
    BLOCK << "};";
    for (const auto& [propertyName, propSchema] : properties_) {
      if (required_.has_value() && required_.value().contains(propertyName)) {
        BLOCK << std::format("if(!{}.contains(\"{}\")) {{",
                             inputJsonVariableName, propertyName);
        {
          Indent _(block);
          BLOCK << "return std::nullopt;";
        }
        BLOCK << "}";
        BLOCK << std::format("object.{} = {}::construct({}[\"{}\"]).value();",
                             sanitizeString(propertyName) + "_",
                             propSchema.get().identifier_,
                             inputJsonVariableName, propertyName);
      } else {
        BLOCK << std::format("if({}.contains(\"{}\")) {{",
                             inputJsonVariableName, propertyName);
        {
          Indent _(block);
          BLOCK << std::format("object.{} = {}::construct({}[\"{}\"]).value();",
                               sanitizeString(propertyName) + "_",
                               propSchema.get().identifier_,
                               inputJsonVariableName, propertyName);
        }
        BLOCK << "}";
      }
    }
    BLOCK << std::format("for (const auto& [key, value] : {}.items()) {{",
                         inputJsonVariableName);
    {
      Indent _(block);
      BLOCK << "if (properties.count(key) == 0) {";
      {
        Indent _(block);
        // If additionalProperties
        if (additionalProperties_.get().definedAsBooleanSchema_.value_or(
                true) == false) {
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
    BLOCK << std::format("{} = std::move(object);", outSchemaVariableName);
  }
  BLOCK << "}";

  return block;
}

CodeBlock
ObjectProperties::objectClassDefinition(const CodeProperties& codeProperties,
                                        std::string schemaType) const {
  CodeBlock block(codeProperties.indent_);
#if JSOG_DEBUG
  block << "/*" << schema.identifier_ << " object class definition*/";
#endif
  // Object declaration
  BLOCK << "class Object {" << CodeBlock::inc;
  {
    // Declare the construct function to be a friend, so it can fill out
    // private members
    BLOCK << std::format(
        "friend std::optional<{}> construct(const nlohmann::json&);",
        schemaType);
    BLOCK << CodeBlock::dec << "public:" << CodeBlock::inc;
    for (const auto& [propertyName, property] : properties_) {
      if (required_.has_value() && required_.value().count(propertyName) > 0) {
        BLOCK << std::format("{} {};", property.get().getType(),
                             sanitizeString(propertyName) + "_");
      } else {
        BLOCK << std::format("std::optional<{}> {};", property.get().getType(),
                             sanitizeString(propertyName) + "_");
      }
    }
    if (additionalProperties_.get().definedAsBooleanSchema_.value_or(true) !=
        false) {
      BLOCK << std::format("std::map<std::string, {}> additionalProperties;",
                           additionalProperties_.get().getType());
    }
  }
  BLOCK << CodeBlock::dec << "};" << "";

  return block;
}
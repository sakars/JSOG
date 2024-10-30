#ifndef DRAFT07_H
#define DRAFT07_H

#include "JSONPointer.h"
#include "ResourceIndex.h"
#include "Schema.h"
#include "UriWrapper.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <regex>
#include <set>
#include <variant>

class Draft07 : public Schema
{
  /// @brief An internal representation of a boolean schema
  /// @warning This class is not a representation of a schema that describes
  /// boolean values, but rather a schema that is a boolean value itself.
  struct BooleanSchema
  {
    bool value;
  };

  struct SchemaInternals
  {
    enum class Type
    {
      STRING,
      NUMBER,
      INTEGER,
      BOOLEAN,
      OBJECT,
      ARRAY,
      NULL_
    };
    SchemaInternals(const nlohmann::json &, JSONPointer);
    static std::optional<Type> stringToType(const std::string &s)
    {
      if (s == "string")
      {
        return Type::STRING;
      }
      else if (s == "number")
      {
        return Type::NUMBER;
      }
      else if (s == "integer")
      {
        return Type::INTEGER;
      }
      else if (s == "boolean")
      {
        return Type::BOOLEAN;
      }
      else if (s == "object")
      {
        return Type::OBJECT;
      }
      else if (s == "array")
      {
        return Type::ARRAY;
      }
      else if (s == "null")
      {
        return Type::NULL_;
      }
      return std::nullopt;
    }
    static std::string typeToString(Type t)
    {
      switch (t)
      {
      case Type::STRING:
        return "string";
      case Type::NUMBER:
        return "number";
      case Type::INTEGER:
        return "integer";
      case Type::BOOLEAN:
        return "boolean";
      case Type::OBJECT:
        return "object";
      case Type::ARRAY:
        return "array";
      case Type::NULL_:
        return "null";
      }
      throw std::runtime_error("Unknown type");
    }
    std::optional<std::set<Type>> type;
    std::optional<std::vector<nlohmann::json>> enum_;
    std::optional<nlohmann::json> const_;
    std::optional<std::reference_wrapper<Schema>> if_;
    std::optional<std::reference_wrapper<Schema>> then_;
    std::optional<std::reference_wrapper<Schema>> else_;
    std::optional<std::vector<std::reference_wrapper<Schema>>> allOf;
    std::optional<std::vector<std::reference_wrapper<Schema>>> anyOf;
    std::optional<std::vector<std::reference_wrapper<Schema>>> oneOf;
    std::optional<std::reference_wrapper<Schema>> not_;
    std::optional<std::map<std::string, std::reference_wrapper<Schema>>>
        definitions;
    std::optional<std::string> title;
    std::optional<std::string> description;
    std::optional<nlohmann::json> default_;
    std::optional<bool> readOnly;  // no plans to implement this
    std::optional<bool> writeOnly; // no plans to implement this
    std::optional<double> multipleOf;
    std::optional<double> maximum;
    std::optional<double> exclusiveMaximum;
    std::optional<double> minimum;
    std::optional<double> exclusiveMinimum;
    /// pattern in standard ecmascript regex aka ECMA-262
    /// @warning Implementations MUST NOT consider the pattern as anchored.
    ///
    /// Excerpt from Draft-07 validation spec 4.3 RegEx:
    ///
    /// Finally, implementations MUST NOT
    /// take regular expressions to be anchored, neither at the beginning nor at
    /// the end. This means, for instance, the pattern "es" matches
    /// "expression".
    std::optional<std::regex> pattern;
    std::optional<int> maxLength;
    std::optional<int> minLength;
    std::optional<std::variant<std::reference_wrapper<Schema>,
                               std::vector<std::reference_wrapper<Schema>>>>
        items;
    std::optional<std::reference_wrapper<Schema>> additionalItems;
    std::optional<unsigned long> maxItems;
    std::optional<unsigned long> minItems;
    std::optional<bool> uniqueItems;
    std::optional<std::reference_wrapper<Schema>> contains;
    std::optional<unsigned long> maxProperties;
    std::optional<unsigned long> minProperties;
    std::optional<std::set<std::string>> required;
    std::optional<std::map<
        std::string, std::variant<JSONPointer, std::reference_wrapper<Schema>>>>
        properties;
    std::optional<
        std::vector<std::tuple<std::regex, std::reference_wrapper<Schema>>>>
        patternProperties;
    std::optional<std::reference_wrapper<Schema>> additionalProperties;
    std::optional<
        std::map<std::string, std::variant<std::reference_wrapper<Schema>,
                                           std::set<std::string>>>>
        dependencies;
    std::optional<std::reference_wrapper<Schema>> propertyNames;
  };

  using Internals =
      std::variant<std::monostate, BooleanSchema, SchemaInternals, std::string>;
  Internals internals;

public:
  Draft07(const nlohmann::json &source, std::string baseUri,
          JSONPointer pointer)
      : Schema(source, baseUri, pointer)
  {
    if (source.is_boolean())
    {
      internals = BooleanSchema{source.get<bool>()};
    }
    else if (source.is_object())
    {
      if (source.contains("$ref"))
      {
        internals = source["$ref"].get<std::string>();
      }
      else
      {
        internals = SchemaInternals(source, pointer);
      }
    }
  }

  std::set<std::string> getDeps() const override
  {
    std::set<std::string> deps;
    UriWrapper uri(getBaseUri());
    JSONPointer pointer =
        JSONPointer::fromJSONString(getPointer().toFragment().substr(1));
    const auto addPtr = [&uri, &deps](const JSONPointer &pointer)
    {
      UriWrapper depUri(uri);
      depUri.setFragment(pointer.toFragment(), true);
      deps.insert(depUri.toString().value());
    };
    const auto addStr = [&uri, &deps](const std::string &str)
    {
      UriWrapper depUri(uri);
      UriWrapper strUri(str);
      depUri = UriWrapper::applyUri(uri, strUri);
      deps.insert(depUri.toString().value());
    };

    const nlohmann::json &json = getJson();
    if (json.contains("$ref"))
    {
      deps.insert(json["$ref"].get<std::string>());
      return deps;
    }

    if (json.contains("if") &&
        (json["if"].is_object() || json["if"].is_boolean()))
    {
      addPtr(pointer / "if");
    }

    if (json.contains("then") &&
        (json["then"].is_object() || json["then"].is_boolean()))
    {
      addPtr(pointer / "then");
    }

    if (json.contains("else") &&
        (json["else"].is_object() || json["else"].is_boolean()))
    {
      addPtr(pointer / "else");
    }

    if (json.contains("allOf") && json["allOf"].is_array())
    {
      for (size_t i = 0; i < json["allOf"].size(); i++)
      {
        addPtr(pointer / "allOf" / std::to_string(i));
      }
    }

    if (json.contains("anyOf") && json["anyOf"].is_array())
    {
      for (size_t i = 0; i < json["anyOf"].size(); i++)
      {
        addPtr(pointer / "anyOf" / std::to_string(i));
      }
    }

    if (json.contains("oneOf") && json["oneOf"].is_array())
    {
      for (size_t i = 0; i < json["oneOf"].size(); i++)
      {
        addPtr(pointer / "oneOf" / std::to_string(i));
      }
    }

    if (json.contains("not") &&
        (json["not"].is_object() || json["not"].is_boolean()))
    {
      addPtr(pointer / "not");
    }

    if (json.contains("items"))
    {
      if (json["items"].is_object() || json["items"].is_boolean())
      {
        addPtr(pointer / "items");
      }
      else if (json["items"].is_array())
      {
        for (size_t i = 0; i < json["items"].size(); i++)
        {
          addPtr(pointer / "items" / std::to_string(i));
        }
      }
    }

    if (json.contains("additionalItems") &&
        (json["additionalItems"].is_object() ||
         json["additionalItems"].is_boolean()))
    {
      addPtr(pointer / "additionalItems");
    }

    if (json.contains("contains") &&
        (json["contains"].is_object() || json["contains"].is_boolean()))
    {
      addPtr(pointer / "contains");
    }

    if (json.contains("properties") && (json["properties"].is_object()))
    {
      for (auto &[key, value] : json["properties"].items())
      {
        addPtr(pointer / "properties" / key);
      }
    }

    if (json.contains("patternProperties") &&
        json["patternProperties"].is_object())
    {
      for (auto &[key, value] : json["patternProperties"].items())
      {
        addPtr(pointer / "patternProperties" / key);
      }
    }

    if (json.contains("additionalProperties") &&
        (json["additionalProperties"].is_object() ||
         json["additionalProperties"].is_boolean()))
    {
      addPtr(pointer / "additionalProperties");
    }

    if (json.contains("dependencies") && json["dependencies"].is_object())
    {
      for (auto &[key, value] : json["dependencies"].items())
      {
        if (value.is_object() || value.is_boolean())
        {
          addPtr(pointer / "dependencies" / key);
        }
      }
    }

    if (json.contains("propertyNames") &&
        (json["propertyNames"].is_object() ||
         json["propertyNames"].is_boolean()))
    {
      addPtr(pointer / "propertyNames");
    }

    return deps;
  }

  std::string generateDefinition() const override;
  std::string generateStructs() const;
  std::string createArrayStruct() const;
  std::string createObjectStruct() const;

  std::string getTypeName() const override;

  void resolveReferences(
      std::function<std::optional<std::reference_wrapper<Schema>>(std::string)>
          resolve) override
  {
    if (std::holds_alternative<std::string>(internals))
    {
      // TODO: This is really fucking scuffed, as right now string holds both
      // json pointers and the typename
      auto &str = std::get<std::string>(internals);
      str = resolve(str).value().get().getTypeName();
    }
    else if (std::holds_alternative<SchemaInternals>(internals))
    {
      auto &schemaInternals = std::get<SchemaInternals>(internals);
      if (schemaInternals.properties.has_value())
      {
        auto &properties = schemaInternals.properties.value();
        for (auto &[key, value] : properties)
        {
          if (std::holds_alternative<JSONPointer>(value))
          {
            const auto &ptr = std::get<JSONPointer>(value);
            const auto typeName = resolve("#" + ptr.toString());
            if (typeName.has_value())
            {
              properties[key] = typeName.value();
            }
          }
        }
      }
    }
  }
};

#endif // DRAFT07_H
#include "Draft07.h"
#include "JSONPointer.h"
#include "StringUtils.h"
#include <format>

Draft07::SchemaInternals::SchemaInternals(const nlohmann::json &json,
                                          JSONPointer pointer)
{
  if (json.contains("type"))
  {
    if (json["type"].is_string())
    {
      auto type = stringToType(json["type"].get<std::string>());
      if (type.has_value())
      {
        this->type = std::set<Type>{type.value()};
      }
    }
    else if (json["type"].is_array())
    {
      std::set<Type> types;
      for (auto &type : json["type"])
      {
        if (type.is_string())
        {
          auto type_ = stringToType(type.get<std::string>());
          if (type_.has_value())
          {
            types.insert(type_.value());
          }
        }
      }
      if (!types.empty())
      {
        this->type = types;
      }
    }
  }
  else
  {
    this->type =
        std::set<Type>{Type::ARRAY, Type::BOOLEAN, Type::INTEGER, Type::NULL_,
                       Type::NUMBER, Type::OBJECT, Type::STRING};
  }

  if (json.contains("properties") && json["properties"].is_object())
  {
    properties =
        std::map<std::string,
                 std::variant<JSONPointer, std::reference_wrapper<Schema>>>{};
    const auto &props = json["properties"];
    for (const auto &[key, _] : props.items())
    {
      // properties[key] =
      // TODO: This is pizdec and something to think about. I need to reference
      // these schemas. Maybe need to figure out how to reference relative json
      // properties as even if i passed base uri, this doesn't let me reasonably
      // specify where we are now. I think I need to pass a JSON Pointer from
      // the base URI.

      // TODO: Continue this function
      properties.value()[key] = pointer / "properties" / key;
    }
  }
}

std::string Draft07::createArrayStruct() const
{
  std::string arrStruct = "struct Array {\n";
  const auto &schema = std::get<SchemaInternals>(internals);
  const auto &items = schema.items;
  if (items.has_value())
  {
    if (items.value().index() == 0)
    {
      arrStruct += "  std::vector<" +
                   std::get<std::reference_wrapper<Schema>>(items.value())
                       .get()
                       .getIdentifier()
                       .value() +
                   "> items;\n";
    }
    else
    {
      const auto &items = std::get<std::vector<std::reference_wrapper<Schema>>>(
          schema.items.value());
      arrStruct += "  std::tuple<";
      for (auto &item : items)
      {
        arrStruct += item.get().getTypeName() + ", ";
      }
      arrStruct.pop_back();
      arrStruct.pop_back();
      arrStruct += "> items;";
    }
  }
  // TODO: add additionalItems.
  arrStruct += "};\n";
  return arrStruct;
}

/// @brief Format string taking key, typeName, cxxTypeToJSONType(typeName)
// constexpr auto objectKeyPrimitiveAdder = R"(
// if (json.contains("{0}") && json["{0}"].is_{2}()) {{
//   obj._{0} = json["{0}"].get<{1}>();
// }}
// )";

/// @brief Format string taking key, typeName
constexpr auto objectKeyRequiredSchemaAdder = R"(
if (json.contains("{0}")) {{
  obj._{0} = {1}::create(json["{0}"]).value();
}}
)";

constexpr auto objectKeyOptionalSchemaAdder = R"(
if (json.contains("{0}")) {{
  obj._{0} = {1}::create(json["{0}"]).value();
}} else {{
  obj._{0} = std::nullopt;
}}
)";

std::string Draft07::createObjectStruct() const
{
  std::string objectStruct = "struct Object {\n";
  const auto &schema = std::get<SchemaInternals>(internals);
  const auto &properties = schema.properties;
  if (properties.has_value())
  {
    for (const auto &[key, schema] : properties.value())
    {
      objectStruct +=
          std::get<std::reference_wrapper<Schema>>(schema).get().getTypeName() +
          " ";
      objectStruct += normalizeString(key) + ";\n";
    }
  }
  // TODO: add other schema properties.

  // Add the factory function
  objectStruct += "static std::optional<Object> create(const nlohmann::json &json) {\n";

  objectStruct += "Object obj;\n";
  // Check if the json is an object
  objectStruct += "if (!json.is_object()) {\n";
  objectStruct += "return std::nullopt;\n";
  objectStruct += "}\n";

  if (properties.has_value())
  {
    for (const auto &[key, schema] : properties.value())
    {
      const auto identifier = std::get<std::reference_wrapper<Schema>>(schema).get().getIdentifier().value();
      // if (iscxxTypeJSONPrimitive(typeName))
      // {
      //   objectStruct += std::format(
      //       objectKeyPrimitiveAdder,
      //       key, typeName, cxxTypeToJSONType(typeName));
      // }
      // else
      // {
      objectStruct += std::format(
          objectKeyRequiredSchemaAdder,
          key, identifier);
      // }
    }
  }
  objectStruct += "return obj;\n";
  objectStruct += "}\n";

  objectStruct += "};\n";

  return objectStruct;
}

std::string Draft07::generateDefinition() const
{
  // If it's a boolean schema, we don't generate a definition as it will be
  // either a monostate or a json
  if (std::holds_alternative<BooleanSchema>(internals))
  {
    return "";
  }
  // If it's a reference, we don't generate a definition
  if (std::holds_alternative<std::string>(internals))
  {
    return "";
  }

  const auto &schemaInternals = std::get<SchemaInternals>(internals);
  // Should be initialized by the constructor
  const auto &types = schemaInternals.type.value();

  std::string definition = "namespace " + getIdentifier().value() + " {\n";

  definition += createArrayStruct();
  definition += createObjectStruct();

  // Add the factory function
  definition += "std::optional<" + getTypeName() + "> create(const nlohmann::json &json) {\n";
  for (auto &type : types)
  {
    const auto typeName = SchemaInternals::typeToString(type);
    if (type == SchemaInternals::Type::OBJECT)
    {
      definition += "if (json.is_object()) {\n";
      definition += "return Object::create(json);\n";
      definition += "}\n";
    }
    else if (type == SchemaInternals::Type::ARRAY)
    {
      definition += "if (json.is_array()) {\n";
      definition += "return Array::create(json);\n";
      definition += "}\n";
    }
    else if (type == SchemaInternals::Type::STRING)
    {
      definition += "if (json.is_string()) {\n";
      definition += "return json.get<std::string>();\n";
      definition += "}\n";
    }
    else if (type == SchemaInternals::Type::NUMBER)
    {
      definition += "if (json.is_number()) {\n";
      definition += "return json.get<double>();\n";
      definition += "}\n";
    }
    else if (type == SchemaInternals::Type::INTEGER)
    {
      definition += "if (json.is_number_integer()) {\n";
      definition += "return json.get<int>();\n";
      definition += "}\n";
    }
    else if (type == SchemaInternals::Type::BOOLEAN)
    {
      definition += "if (json.is_boolean()) {\n";
      definition += "return json.get<bool>();\n";
      definition += "}\n";
    }
    else if (type == SchemaInternals::Type::NULL_)
    {
      definition += "if (json.is_null()) {\n";
      definition += "return std::monostate();\n";
      definition += "}\n";
    }
  }
  definition += "return std::nullopt;\n";
  definition += "}\n";

  definition += "} // namespace" + getIdentifier().value() + "\n";
  return definition;
}

std::string Draft07::getTypeName() const
{
  // If it's just a bool, then it's type is simple
  if (std::holds_alternative<BooleanSchema>(internals))
  {
    const auto &schema = std::get<BooleanSchema>(internals);
    return schema.value ? "nlohmann::json" : "std::monostate";
  }
  // If it's a reference, we don't generate a definition
  if (std::holds_alternative<std::string>(internals))
  {
    return std::get<std::string>(internals);
  }

  // Otherwise
  const auto &schemaInternals = std::get<SchemaInternals>(internals);
  // Should be initialized by the constructor
  // TODO: rework this to not have to trust this exists.
  const auto &type = schemaInternals.type.value();
  std::string typeName = "";
  if (type.size() == 1)
  {
    switch (*type.begin())
    {
    case SchemaInternals::Type::STRING:
      typeName += "std::string";
      break;
    case SchemaInternals::Type::NUMBER:
      typeName += "double";
      break;
    case SchemaInternals::Type::INTEGER:
      typeName += "int";
      break;
    case SchemaInternals::Type::BOOLEAN:
      typeName += "bool";
      break;
    case SchemaInternals::Type::OBJECT:
      typeName += getIdentifier().value() + "::" + "Object";
      break;
    case SchemaInternals::Type::ARRAY:
      typeName += getIdentifier().value() + "::" + "Array";
      break;
    case SchemaInternals::Type::NULL_:
      typeName += "std::monostate";
      break;
    }
  }
  else
  {
    typeName += "std::variant<";
    for (auto &type : type)
    {
      switch (type)
      {
      case SchemaInternals::Type::NULL_:
        typeName += "std::monostate, ";
        break;
      case SchemaInternals::Type::STRING:
        typeName += "std::string, ";
        break;
      case SchemaInternals::Type::NUMBER:
        typeName += "double, ";
        break;
      case SchemaInternals::Type::INTEGER:
        typeName += "int, ";
        break;
      case SchemaInternals::Type::BOOLEAN:
        typeName += "bool, ";
        break;
      case SchemaInternals::Type::OBJECT:
        typeName += getIdentifier().value() + "::" + "Object, ";
        break;
      case SchemaInternals::Type::ARRAY:
        typeName += getIdentifier().value() + "::" + "Array, ";
        break;
      }
    }
    typeName.pop_back();
    typeName.pop_back();
    typeName += ">";
  }
  return typeName;
}
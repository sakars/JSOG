#include "ResourceIndex.h"
#include "Schema.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <regex>
#include <set>
#include <variant>

class Draft07 : public Schema {
  /// @brief An internal representation of a boolean schema
  /// @warning This class is not a representation of a schema that describes
  /// boolean values, but rather a schema that is a boolean value itself.
  struct BooleanSchema {
    bool value;
  };

  struct SchemaInternals {
    enum class Type { STRING, NUMBER, INTEGER, BOOLEAN, OBJECT, ARRAY, NULL_ };
    static std::optional<Type> stringToType(const std::string &s) {
      if (s == "string") {
        return Type::STRING;
      } else if (s == "number") {
        return Type::NUMBER;
      } else if (s == "integer") {
        return Type::INTEGER;
      } else if (s == "boolean") {
        return Type::BOOLEAN;
      } else if (s == "object") {
        return Type::OBJECT;
      } else if (s == "array") {
        return Type::ARRAY;
      } else if (s == "null") {
        return Type::NULL_;
      }
      return std::nullopt;
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
    std::optional<std::map<std::string, std::reference_wrapper<Schema>>>
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
  Draft07(const nlohmann::json &source) : Schema(source) {}

  void
  initialize(const std::function<Schema &(const nlohmann::json &)> &factory,
             ResourceIndex &index) override {
    const auto &c = content.get();
    // Initialize the schema
    // Check if the json is a boolean
    if (c.is_boolean()) {
      internals = BooleanSchema{c.get<bool>()};
      return;
    } else if (c.contains("$ref") && c["$ref"].is_string()) {
      internals = c["$ref"].get<std::string>();
      return;
    }
    internals = SchemaInternals{};
    // Step1: Check for the presence of an "$id" keyword.
    // In this case, we construct a new resource and add it to the index.
    // Root and parent won't change in this case.
    if (c.contains("$id") && c["$id"].is_string()) {
      index.addResource(c["$id"].get<std::string>(), *this);
    }

    // Step2: Fill out the internals
    auto &internalsRef = std::get<SchemaInternals>(internals);
    if (c.contains("type")) {
      if (c["type"].is_string()) {
        auto type = SchemaInternals::stringToType(c["type"].get<std::string>());
        if (type.has_value()) {
          internalsRef.type = std::set<SchemaInternals::Type>{type.value()};
        }
      } else if (c["type"].is_array()) {
        std::set<SchemaInternals::Type> types;
        for (const auto &type : c["type"]) {
          if (type.is_string()) {
            auto t = SchemaInternals::stringToType(type.get<std::string>());
            if (t.has_value()) {
              types.insert(t.value());
            }
          }
        }
        internalsRef.type = types;
      }
    } else {
      internalsRef.type = std::set<SchemaInternals::Type>{
          SchemaInternals::Type::STRING, SchemaInternals::Type::NUMBER,
          SchemaInternals::Type::INTEGER, SchemaInternals::Type::BOOLEAN,
          SchemaInternals::Type::OBJECT, SchemaInternals::Type::ARRAY,
          SchemaInternals::Type::NULL_};
    }

    
  }
};
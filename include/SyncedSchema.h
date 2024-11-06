#include "CodeBlock.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <set>
#include <string>
#include <vector>

/// @brief A class to represent a schema that holds information to generate code
/// @details This class is used to represent a schema that holds information to
/// generate code that is not dependent on the Draft version of the schema.
class SyncedSchema {
public:
  std::string identifier;

  // Non-type specific properties

  /// @brief If set, the schema is interpreted as a boolean schema, regardless
  /// of the other properties set.
  std::optional<bool> definedAsBooleanSchema_;

  /// @brief If set, the schema is interpreted as a reference to another schema
  std::optional<std::reference_wrapper<SyncedSchema>> ref_;

  enum class Type { Null, Boolean, Object, Array, Number, String };

  std::optional<std::set<Type>> type_;

  std::optional<std::vector<nlohmann::json>> enum_;

  std::optional<nlohmann::json> const_;

  std::optional<std::string> description_;

  // Number and Integer properties
  std::optional<double> multipleOf_;
  std::optional<double> maximum_;
  std::optional<double> exclusiveMaximum_;
  std::optional<double> minimum_;
  std::optional<double> exclusiveMinimum_;

  // String properties
  std::optional<double> maxLength_;
  std::optional<double> minLength_;
  std::optional<std::string> pattern_;

  // Array properties
  /// @brief The first items in the array
  std::optional<std::vector<std::reference_wrapper<SyncedSchema>>>
      tupleableItems_;
  /// @brief The object that matches non-tupleable items in the array
  std::optional<std::reference_wrapper<SyncedSchema>> items_;
  std::optional<size_t> maxItems_;
  std::optional<size_t> minItems_;
  std::optional<bool> uniqueItems_;
  std::optional<std::reference_wrapper<SyncedSchema>> contains_;

  // Object initialization properties
  std::optional<size_t> maxProperties_;
  std::optional<size_t> minProperties_;
  std::optional<std::set<std::string>> required_;
  std::map<std::string, std::reference_wrapper<SyncedSchema>> properties_;
  std::optional<std::map<std::string, std::reference_wrapper<SyncedSchema>>>
      patternProperties_;
  std::optional<std::reference_wrapper<SyncedSchema>> additionalProperties_;

  std::optional<std::map<std::string, std::vector<std::string>>>
      propertyDependencies_;
  std::optional<std::map<std::string, std::reference_wrapper<SyncedSchema>>>
      schemaDependencies_;

  std::optional<std::reference_wrapper<SyncedSchema>> propertyNames_;

  std::optional<std::reference_wrapper<SyncedSchema>> if_;
  std::optional<std::reference_wrapper<SyncedSchema>> then_;
  std::optional<std::reference_wrapper<SyncedSchema>> else_;

  std::optional<std::vector<std::reference_wrapper<SyncedSchema>>> allOf_;
  std::optional<std::vector<std::reference_wrapper<SyncedSchema>>> anyOf_;
  std::optional<std::vector<std::reference_wrapper<SyncedSchema>>> oneOf_;
  std::optional<std::reference_wrapper<SyncedSchema>> not_;

  enum class Format {
    DateTime,
    Date,
    Time,
    Email,
    IdnEmail,
    Hostname,
    IdnHostname,
    IpV4,
    IpV6,
    Uri,
    UriReference,
    Iri,
    IriReference,
    UriTemplate,
    JsonPointer,
    RelativeJsonPointer,
    Regex
  };

  std::optional<Format> format_;
  std::optional<nlohmann::json> default_;

  std::optional<bool> readOnly_;
  std::optional<bool> writeOnly_;

  std::optional<std::vector<nlohmann::json>> examples_;

  SyncedSchema(const std::string& identifier) : identifier(identifier) {}

  /// @brief Generates the declaration of the schema
  CodeBlock generateDeclaration() const;
  CodeBlock generateDefinition() const;
  std::string generateType() const;
};
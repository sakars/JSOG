#ifndef INDEXED_SYNCED_SCHEMA_H
#define INDEXED_SYNCED_SCHEMA_H

#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <set>
#include <string>
#include <vector>

/// @brief A fully instantiated instance of IndexedSyncedSchema fully represents
/// an interpreted JSON schema regardless of its Draft version.
class IndexedSyncedSchema {
public:
  std::string identifier_;
  std::optional<bool> definedAsBooleanSchema_;
  std::optional<size_t> ref_;

  enum class Type { Null, Boolean, Object, Array, Number, String, Integer };
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

  std::optional<std::set<Type>> type_;
  std::optional<std::vector<nlohmann::json>> enum_;
  std::optional<nlohmann::json> const_;
  std::optional<std::string> description_;
  std::optional<double> multipleOf_;
  std::optional<double> maximum_;
  std::optional<double> exclusiveMaximum_;
  std::optional<double> minimum_;
  std::optional<double> exclusiveMinimum_;
  std::optional<size_t> maxLength_;
  std::optional<size_t> minLength_;
  std::optional<std::string> pattern_;
  std::optional<std::vector<size_t>> tupleableItems_;
  std::optional<size_t> items_;
  std::optional<size_t> maxItems_;
  std::optional<size_t> minItems_;
  std::optional<bool> uniqueItems_;
  std::optional<size_t> contains_;
  std::optional<size_t> maxProperties_;
  std::optional<size_t> minProperties_;
  std::optional<std::set<std::string>> required_;
  std::map<std::string, size_t> properties_;
  std::optional<std::map<std::string, size_t>> patternProperties_;
  std::optional<size_t> additionalProperties_;
  std::optional<std::map<std::string, std::vector<std::string>>>
      propertyDependencies_;
  std::optional<std::map<std::string, size_t>> schemaDependencies_;
  std::optional<size_t> propertyNames_;
  std::optional<size_t> if_;
  std::optional<size_t> then_;
  std::optional<size_t> else_;
  std::optional<std::vector<size_t>> allOf_;
  std::optional<std::vector<size_t>> anyOf_;
  std::optional<std::vector<size_t>> oneOf_;
  std::optional<size_t> not_;
  std::optional<Format> format_;
  std::optional<nlohmann::json> default_;
  std::optional<bool> readOnly_;
  std::optional<bool> writeOnly_;
  std::optional<std::vector<nlohmann::json>> examples_;
  static void
  dumpSchemas(const std::vector<IndexedSyncedSchema>& indexedSyncedSchemas,
              std::filesystem::path outputDirectory = ".");
};

#endif // INDEXED_SYNCED_SCHEMA_H
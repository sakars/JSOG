#ifndef INDEXED_SYNCED_SCHEMA_H
#define INDEXED_SYNCED_SCHEMA_H

#include "IndexedSyncedSchema/IndexedArrayProperties.h"
#include "IndexedSyncedSchema/IndexedObjectProperties.h"
#include "IndexedSyncedSchema/IndexedReinterpretables.h"
#include "SyncedSchema/NumberProperties.h"
#include "SyncedSchema/StringProperties.h"
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

  NumberProperties numberProperties_;

  StringProperties stringProperties_;

  IndexedArrayProperties arrayProperties_;

  IndexedObjectProperties objectProperties_;

  IndexedReinterpretables reinterpretables_;

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
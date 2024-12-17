#ifndef SYNCEDSCHEMA_H
#define SYNCEDSCHEMA_H

#include "CodeBlock.h"
#include "IndexedSyncedSchema.h"
#include "SyncedSchema/ArrayProperties.h"
#include "SyncedSchema/BoolProperties.h"
#include "SyncedSchema/NullProperties.h"
#include "SyncedSchema/NumberProperties.h"
#include "SyncedSchema/ObjectProperties.h"
#include "SyncedSchema/StringProperties.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <set>
#include <string>
#include <vector>

class SyncedSchema;

struct Reinterpretables {
  std::optional<std::reference_wrapper<SyncedSchema>> ref_;
  std::optional<std::reference_wrapper<const SyncedSchema>> if_;
  std::optional<std::reference_wrapper<const SyncedSchema>> then_;
  std::optional<std::reference_wrapper<const SyncedSchema>> else_;

  std::optional<std::vector<std::reference_wrapper<const SyncedSchema>>> allOf_;
  std::optional<std::vector<std::reference_wrapper<const SyncedSchema>>> anyOf_;
  std::optional<std::vector<std::reference_wrapper<const SyncedSchema>>> oneOf_;
  std::optional<std::reference_wrapper<const SyncedSchema>> not_;
};

/// @brief A class to represent a schema that holds information to generate code
/// @details This class is used to represent a schema that holds information to
/// generate code that is not dependent on the Draft version of the schema.
class SyncedSchema {
public:
  using Type = IndexedSyncedSchema::Type;
  using Format = IndexedSyncedSchema::Format;
  std::reference_wrapper<const CodeProperties> codeProperties =
      std::cref(getDefaultCodeProperties());

  std::string identifier_;
  // Non-type specific properties

  /// @brief If set, the schema is interpreted as a boolean schema, regardless
  /// of the other properties set.
  std::optional<bool> definedAsBooleanSchema_;

  std::set<IndexedSyncedSchema::Type> type_;

  std::optional<std::vector<nlohmann::json>> enum_;

  std::optional<nlohmann::json> const_;

  std::optional<std::string> description_;

  NullProperties nullProperties_;

  BoolProperties boolProperties_;

  // Number and Integer properties
  NumberProperties numberProperties_;

  // String properties
  StringProperties stringProperties_;

  // Array properties
  ArrayProperties arrayProperties_;

  // Object initialization properties
  ObjectProperties objectProperties_;

  // Reinterpretables
  Reinterpretables reinterpretables_;

  std::optional<IndexedSyncedSchema::Format> format_;
  std::optional<nlohmann::json> default_;

  std::optional<bool> readOnly_;
  std::optional<bool> writeOnly_;

  std::optional<std::vector<nlohmann::json>> examples_;

  SyncedSchema(const std::string& identifier)
      : identifier_(identifier), arrayProperties_(getTrueSchema()),
        objectProperties_(getTrueSchema()) {}

private:
  /// @brief Private constructor to create default schemas
  /// without causing undefined behavior
  SyncedSchema()
      : identifier_(""), arrayProperties_(*this), objectProperties_(*this) {}

public:
  /// @brief Generates the declaration of the schema
  CodeBlock generateDeclaration() const;
  CodeBlock generateDefinition() const;
  CodeBlock generateSystemDependencies() const;
  CodeBlock generateDependencies() const;

  std::string getNamespace() const;
  std::string getHeaderFileName() const;
  std::string getSourceFileName() const;
  std::string getType() const;

  std::string getBooleanType() const { return "bool"; }

  std::string getNamespaceLocation() const;

  static const CodeProperties& getDefaultCodeProperties() {
    static CodeProperties properties;
    return properties;
  }

  static const SyncedSchema& getTrueSchema() {
    static SyncedSchema schema;
    schema.identifier_ = "True";
    schema.definedAsBooleanSchema_ = true;
    return schema;
  }

  static std::vector<std::unique_ptr<SyncedSchema>>
  resolveIndexedSchema(std::vector<IndexedSyncedSchema>&& schemas);

  static void dumpSchemas(std::vector<std::unique_ptr<SyncedSchema>>& schemas,
                          std::filesystem::path outputDirectory = ".");
};

#endif // SYNCEDSCHEMA_H
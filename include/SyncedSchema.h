#include "CodeBlock.h"
#include "IndexedSyncedSchema.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <set>
#include <string>
#include <vector>

struct CodeProperties {
  enum class HeaderGuard { Pragma, Ifndef };
  HeaderGuard headerGuardType_ = HeaderGuard::Ifndef;
  std::string indent_ = "  ";
  std::optional<std::string> globalNamespace_ = "JSOG";
  std::optional<std::string> define_prefix_ = "JSOG_";

  // Feature flags

  /// @brief If set, whenever a schema is defined as an array,
  /// tupleableItems_ is set, and minItems_ is set to at least 1,
  /// the generated code will make itemN required, not optional.
  /// @todo This feature is not yet tested
  bool minItemsMakeTupleableRequired_ = false;
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
  std::string filename_;

  // Non-type specific properties

  /// @brief If set, the schema is interpreted as a boolean schema, regardless
  /// of the other properties set.
  std::optional<bool> definedAsBooleanSchema_;

  /// @brief If set, the schema is interpreted as a reference to another schema
  std::optional<std::reference_wrapper<SyncedSchema>> ref_;

  std::set<IndexedSyncedSchema::Type> type_;

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
  std::optional<size_t> maxLength_;
  std::optional<size_t> minLength_;
  std::optional<std::string> pattern_;

  // Array properties
  /// @brief The first items in the array
  std::optional<std::vector<std::reference_wrapper<const SyncedSchema>>>
      tupleableItems_;
  /// @brief The object that matches non-tupleable items in the array
  std::reference_wrapper<const SyncedSchema> items_;
  std::optional<size_t> maxItems_;
  std::optional<size_t> minItems_;
  std::optional<bool> uniqueItems_;
  std::optional<std::reference_wrapper<const SyncedSchema>> contains_;

  // Object initialization properties
  std::optional<size_t> maxProperties_;
  std::optional<size_t> minProperties_;
  std::optional<std::set<std::string>> required_;
  std::map<std::string, std::reference_wrapper<const SyncedSchema>> properties_;
  std::optional<
      std::map<std::string, std::reference_wrapper<const SyncedSchema>>>
      patternProperties_;
  std::reference_wrapper<const SyncedSchema> additionalProperties_;

  std::optional<std::map<std::string, std::vector<std::string>>>
      propertyDependencies_;
  std::optional<
      std::map<std::string, std::reference_wrapper<const SyncedSchema>>>
      schemaDependencies_;

  std::optional<std::reference_wrapper<const SyncedSchema>> propertyNames_;

  // Conditional properties
  std::optional<std::reference_wrapper<const SyncedSchema>> if_;
  std::optional<std::reference_wrapper<const SyncedSchema>> then_;
  std::optional<std::reference_wrapper<const SyncedSchema>> else_;

  std::optional<std::vector<std::reference_wrapper<const SyncedSchema>>> allOf_;
  std::optional<std::vector<std::reference_wrapper<const SyncedSchema>>> anyOf_;
  std::optional<std::vector<std::reference_wrapper<const SyncedSchema>>> oneOf_;
  std::optional<std::reference_wrapper<const SyncedSchema>> not_;

  std::optional<IndexedSyncedSchema::Format> format_;
  std::optional<nlohmann::json> default_;

  std::optional<bool> readOnly_;
  std::optional<bool> writeOnly_;

  std::optional<std::vector<nlohmann::json>> examples_;

  SyncedSchema(const std::string& identifier)
      : identifier_(identifier), filename_(identifier), items_(getTrueSchema()),
        additionalProperties_(getTrueSchema()) {}

private:
  /// @brief Private constructor to create default schemas
  /// without causing undefined behavior
  SyncedSchema()
      : identifier_(""), filename_(""), items_(*this),
        additionalProperties_(*this) {}

public:
  enum class IntegerType {
    INT8,
    UINT8,
    INT16,
    UINT16,
    INT32,
    UINT32,
    INT64,
    UINT64
  };
  /// @brief Generates the declaration of the schema
  CodeBlock generateDeclaration() const;
  CodeBlock generateDefinition() const;
  CodeBlock generateSystemDependencies() const;
  CodeBlock generateDependencies() const;

  std::string getHeaderFileName() const;
  std::string getSourceFileName() const;
  std::string getType() const;
  std::string getObjectType() const;
  std::string getArrayType() const;
  std::string getNumberType() const;
  std::string getStringType() const;
  std::string getBooleanType() const;
  std::string getNullType() const;
  std::string getIntegerType() const;
  IntegerType getIntegerEnum() const;

  static const CodeProperties& getDefaultCodeProperties() {
    static CodeProperties properties;
    return properties;
  }

  static const SyncedSchema& getTrueSchema() {
    static SyncedSchema schema;
    schema.identifier_ = "True";
    schema.filename_ = "True";
    schema.definedAsBooleanSchema_ = true;
    return schema;
  }

  static std::vector<std::unique_ptr<SyncedSchema>>
  resolveIndexedSchema(std::vector<IndexedSyncedSchema>&& schemas);

  static void dumpSchemas(std::vector<std::unique_ptr<SyncedSchema>>& schemas,
                          std::filesystem::path outputDirectory = ".");
};
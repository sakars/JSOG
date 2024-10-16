#ifndef SCHEMA_H
#define SCHEMA_H

#include <functional>
#include <nlohmann/json.hpp>
#include <optional>

class ResourceIndex;
class Resource;

/**
 * @brief Represents a JSON schema
 * @details This class represents a JSON schema. It is a base class for
 * different schema versions, such as Draft07.
 *
 * Schema creation is a multi-step process:
 * 1. The schema is initialized with the JSON content. In this stage,
 *   the schema may create a new resource if it has an "$id" keyword and
 *   generate its subschemas, which are also sent through the initialization
 *   step.
 * 2. Schemas perform reference resolution as all schemas are initialized.
 *   If a schema cannot be resolved, attempt
 *
 *
 */
class Schema {

public:
  std::reference_wrapper<const nlohmann::json> content;
  std::map<std::string, std::reference_wrapper<Schema>> subschemas;
  std::reference_wrapper<Schema> root;
  std::reference_wrapper<Schema> parent;
  std::optional<std::reference_wrapper<Resource>> resource = std::nullopt;

  Schema(const nlohmann::json &source,
         std::optional<std::reference_wrapper<Schema>> root = std::nullopt,
         std::optional<std::reference_wrapper<Schema>> parent = std::nullopt)
      : content(source), root(root.value_or(*this)),
        parent(parent.value_or(root.value_or(*this))) {
    // Clone the default resource from the root schema
    resource = this->root.get().resource;
  }

  virtual void
  initialize(const std::function<Schema &(const nlohmann::json &)> &factory,
             ResourceIndex &index) = 0;
};

#endif // SCHEMA_H
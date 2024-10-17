#ifndef SCHEMA_H
#define SCHEMA_H

#include "ResourceIndex.h"
#include <functional>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

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
  std::string baseUri;

  Schema(const nlohmann::json &source, const std::string &baseUri)
      : content(source), baseUri(baseUri) {}

  virtual void initialize(
      const std::function<Schema &(const nlohmann::json &)> &schema_generator,
      ResourceIndex &index) = 0;
};

#endif // SCHEMA_H
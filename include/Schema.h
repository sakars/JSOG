#ifndef SCHEMA_H
#define SCHEMA_H

#include <functional>
#include <nlohmann/json.hpp>
#include <optional>
#include <set>
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
  std::reference_wrapper<const nlohmann::json> json_;
  std::string baseUri_;

  Schema(const nlohmann::json &json, std::string baseUri)
      : json_(json), baseUri_(baseUri) {}

  virtual std::set<std::string> getDeps() const = 0;
};

#endif // SCHEMA_H
#ifndef SCHEMA_H
#define SCHEMA_H

#include <functional>
#include <nlohmann/json.hpp>
#include <optional>
#include <set>
#include <string>

enum class Draft { DRAFT_07, UNKNOWN };

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
  std::set<std::string> customNames = {};
  std::optional<std::string> realName;

  Schema(const nlohmann::json &json, std::string baseUri)
      : json_(json), baseUri_(baseUri) {}

  void addName(const std::string &name) { customNames.emplace(name); }

  virtual std::set<std::string> getDeps() const = 0;

private:
  /// @brief Sanitize this string that can be practically anything, to a
  /// typename that is valid in C++
  /// @param str The string to sanitize
  /// @return A sanitized string
  static std::string sanitize(const std::string &str) {
    std::string sanitized = str;
    std::replace(sanitized.begin(), sanitized.end(), '-', '_');
    std::replace(sanitized.begin(), sanitized.end(), '.', '_');
    std::replace(sanitized.begin(), sanitized.end(), '/', '_');
    std::replace(sanitized.begin(), sanitized.end(), ':', '_');
    std::replace(sanitized.begin(), sanitized.end(), '#', '_');
    std::replace(sanitized.begin(), sanitized.end(), ' ', '_');
    return sanitized;
  }

protected:
  virtual std::string getTypeName() const {
    if (realName.has_value()) {
      return *realName;
    }
    if (json_.get().contains("title")) {
      return sanitize(json_.get()["title"].get<std::string>());
    }
    return "Schema";
  }

public:
  void generateRealName() {
    if (!realName.has_value()) {
      realName = getTypeName();
    }
  }
  void clearRealName() { realName = std::nullopt; }

  void overrideRealName(std::string name) { realName = sanitize(name); }

  std::string getRealName() const { return getTypeName(); }
};

#endif // SCHEMA_H
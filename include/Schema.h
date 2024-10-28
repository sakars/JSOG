#ifndef SCHEMA_H
#define SCHEMA_H

#include "JSONPointer.h"
#include <functional>
#include <nlohmann/json.hpp>
#include <optional>
#include <set>
#include <string>
#include <variant>

enum class Draft { DRAFT_07, UNKNOWN };

// TODO: Rewrite Schema Interface to have seperate stage interfaces.
// Currently a variant with stages where each stage has the previous stage moved
// inside it sounds pretty good. A bit complicated, sure, but not bad by any
// measure.

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
  struct Stage1 {
    /// @brief The json content of the schema. This does not own the JSON
    /// content, and the JSON content must outlive this object.
    std::reference_wrapper<const nlohmann::json> json_;
    /// @brief Pointer to the current schema from baseUri_
    JSONPointer pointer_;
    /// @brief The base URI of the schema
    std::string baseUri_;

    Stage1(const nlohmann::json &json, JSONPointer pointer, std::string baseUri)
        : json_(json), pointer_(pointer), baseUri_(baseUri) {}

    virtual std::string getPreferredIdentifier() const {
      if (json_.get().contains("title")) {
        return json_.get()["title"].get<std::string>();
      }
      return "Schema";
    }

    std::optional<std::string> getIdentifier() const { return std::nullopt; }
    std::string getBaseUri() const { return baseUri_; }
    const nlohmann::json &getJson() const { return json_; }
    const JSONPointer &getPointer() const { return pointer_; }
  };

  struct Stage2 {
    Stage1 stage1_;
    std::string uniqueIdentifier_;

    std::optional<std::string> getIdentifier() const {
      return uniqueIdentifier_;
    }
    std::string getBaseUri() const { return stage1_.baseUri_; }
    const nlohmann::json &getJson() const { return stage1_.json_; }
    const JSONPointer &getPointer() const { return stage1_.pointer_; }
  };

public:
  // std::reference_wrapper<const nlohmann::json> json_;
  // std::string baseUri_;
  // std::optional<std::string> realName;

  std::variant<Stage1, Stage2> stage_;

  Schema(const nlohmann::json &json, std::string baseUri, JSONPointer pointer)
      : stage_(Stage1(json, pointer, baseUri)) {}

  virtual std::set<std::string> getDeps() const = 0;

public:
  void transitionToStage2(std::string uniqueIdentifier) {
    if (!std::holds_alternative<Stage1>(stage_)) {
      throw std::runtime_error("Cannot transition to Stage2 from non-Stage1");
    }
    auto &stage1 = std::get<Stage1>(stage_);
    stage_ = Stage2(std::move(stage1), uniqueIdentifier);
  }

  std::optional<std::string> getIdentifier() const {
    return std::visit([](auto &&arg) { return arg.getIdentifier(); }, stage_);
  }

  std::string getBaseUri() const {
    return std::visit([](auto &&arg) { return arg.getBaseUri(); }, stage_);
  }

  const nlohmann::json &getJson() const {
    return std::visit(
        [](auto &&arg) -> const nlohmann::json & { return arg.getJson(); },
        stage_);
  }

  const JSONPointer &getPointer() const {
    return std::visit(
        [](auto &&arg) -> const JSONPointer & { return arg.getPointer(); },
        stage_);
  }

  virtual std::string getTypeName() const = 0;

  virtual std::string generateDefinition() const { return ""; }

  // TODO: implement in Draft07
  /// @brief Resolves references in the schema
  /// @details This function resolves references in the schema. Before calling
  /// this function, a schema may not know which other schemas it is
  /// referencing, only knowing their URIs. After calling this function, the
  /// schema should have references to the actual schema objects that it
  /// references.
  virtual void resolveReferences(
      std::function<
          std::optional<std::reference_wrapper<Schema>>(std::string)>) = 0;
};

#endif // SCHEMA_H
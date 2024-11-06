#ifndef BUILDABLESCHEMA_H
#define BUILDABLESCHEMA_H

#include "DraftRecognisedDocument.h"
#include "JSONPointer.h"
#include "SetMap.h"
#include "StringUtils.h"
#include "UnresolvedSchema.h"
#include "UriWrapper.h"
#include <memory>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <vector>

class BuildableSchema {
public:
  const std::reference_wrapper<const nlohmann::json> json_;
  const UriWrapper baseUri_;
  const JSONPointer pointer_;
  Draft draft_;

  std::map<UriWrapper, std::reference_wrapper<const BuildableSchema>>
      dependencies_;
  std::set<UriWrapper> dependenciesSet_;

  BuildableSchema(const UnresolvedSchema& unresolvedSchema)
      : json_(unresolvedSchema.json_), baseUri_(unresolvedSchema.baseUri_),
        pointer_(unresolvedSchema.pointer_), draft_(unresolvedSchema.draft_) {}

protected:
  virtual std::set<UriWrapper> getDependencies() const = 0;

public:
  BuildableSchema(const BuildableSchema&) = delete;
  virtual ~BuildableSchema() = default;

  /// @brief Returns the preferred identifier for the schema.
  /// @return The preferred identifier for the schema.
  /// @warning This function has to return a valid C++ identifier. Use
  /// normalizeString to ensure that.
  std::string getPreferredIdentifier() const {
    if (json_.get().contains("title")) {
      auto title = json_.get()["title"].get<std::string>();
      if (title != "") {
        return sanitizeString(title);
      }
    }
    return "Schema";
  }
};

std::unique_ptr<BuildableSchema> construct(const UnresolvedSchema& schema);

std::vector<std::unique_ptr<BuildableSchema>>
resolveDependencies(SetMap<UriWrapper, UnresolvedSchema>&& setMap,
                    const std::set<UriWrapper>& requiredReferences);

#endif // BUILDABLESCHEMA_H
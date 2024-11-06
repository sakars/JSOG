#include "BuildableSchema.h"
#include "BuildableDraft07.h"

std::unique_ptr<BuildableSchema> construct(const UnresolvedSchema& schema) {
  if (schema.draft_ == Draft::DRAFT_07) {
    return std::make_unique<BuildableDraft07>(schema);
  }
  throw std::runtime_error("Unrecognized schema draft");
}

static void
addDependencies(const std::set<UriWrapper>& dependencies,
                const SetMap<UriWrapper, UnresolvedSchema>& setMap,
                std::set<const UnresolvedSchema*>& buildableRequiredSchemas,
                const std::set<const UnresolvedSchema*>& builtRequiredSchemas) {
  for (const auto& dep : dependencies) {
    if (setMap.contains(dep)) {
      auto& schema = setMap[dep];
      bool isToBeBuilt = buildableRequiredSchemas.find(&schema) !=
                         buildableRequiredSchemas.end();
      bool isBuilt =
          builtRequiredSchemas.find(&schema) != builtRequiredSchemas.end();

      if (!isToBeBuilt && !isBuilt) {
        buildableRequiredSchemas.emplace(&schema);
      }
    }
  }
}

std::vector<std::unique_ptr<BuildableSchema>>
resolveDependencies(SetMap<UriWrapper, UnresolvedSchema>&& setMap,
                    const std::set<UriWrapper>& requiredReferences) {
  SetMap<UriWrapper, BuildableSchema> resolvedSchemas;
  std::set<const UnresolvedSchema*> buildableRequiredSchemas;
  std::set<const UnresolvedSchema*> builtRequiredSchemas;
  // Set up the initial required schemas
  for (const auto& ref : requiredReferences) {
    if (setMap.contains(ref)) {
      buildableRequiredSchemas.insert(&setMap[ref]);
    } else {
      std::cerr << "Warning: Required reference not found in setMap: " << ref
                << std::endl;
    }
  }

  // Build each dependency, adding new dependencies as they are found
  while (!buildableRequiredSchemas.empty()) {
    auto schema = *buildableRequiredSchemas.begin();
    buildableRequiredSchemas.erase(schema);
    builtRequiredSchemas.insert(schema);
    auto buildableSchema = construct(*schema);
    auto& deps = buildableSchema->dependenciesSet_;
    addDependencies(deps, setMap, buildableRequiredSchemas,
                    builtRequiredSchemas);
    std::set<UriWrapper> schemaKeys = setMap.keysOfValue(*schema);
    resolvedSchemas.bulkInsert(schemaKeys, std::move(buildableSchema));
  }

  // Fills in the dependency map of a schema
  const auto fillDepMap = [&resolvedSchemas = std::as_const(resolvedSchemas)](
                              BuildableSchema& schema) {
    for (const auto& depUri : schema.dependenciesSet_) {
      if (resolvedSchemas.contains(depUri)) {
        schema.dependencies_.emplace(depUri,
                                     std::cref(resolvedSchemas[depUri]));
      } else {
        std::cerr << "Warning: Dependency not found in resolvedSchemas: "
                  << depUri << std::endl;
      }
    }
  };

  // Fill in the dependency map of each schema
  for (const auto [_, schemaRef] : resolvedSchemas) {
    fillDepMap(schemaRef);
  }
  // Extract the schemas from setMap
  auto extracted = resolvedSchemas.extract();
  std::vector<std::unique_ptr<BuildableSchema>> schemas;
  for (auto& [keys, value] : extracted) {
    schemas.push_back(std::move(value));
  }
  return schemas;
}
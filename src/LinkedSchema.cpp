#include "LinkedSchema.h"
#include "Draft07.h"
std::map<Draft, std::set<UriWrapper> (*)(const nlohmann::json&,
                                         const UriWrapper&, const JSONPointer&)>
    linkers{
        {Draft::DRAFT_07, getDraft07Dependencies},
    };

std::vector<std::unique_ptr<LinkedSchema>>
resolveDependencies(SetMap<UriWrapper, UnresolvedSchema>&& setMap,
                    const std::set<UriWrapper>& requiredReferences) {
  // set up a more useful data structure for dependency resolution
  auto map = setMap.extract();
  std::vector<std::unique_ptr<UnresolvedSchema>> unresolvedSchemas;
  std::map<UriWrapper, size_t> schemaIndices;
  for (auto& [keys, value] : map) {
    for (const auto& key : keys) {
      schemaIndices[key] = unresolvedSchemas.size();
    }
    unresolvedSchemas.push_back(std::move(value));
  }

  std::set<size_t> buildableRequiredSchemas;

  for (const auto& ref : requiredReferences) {
    if (schemaIndices.contains(ref)) {
      buildableRequiredSchemas.insert(schemaIndices[ref]);
    } else {
      std::cerr << "Warning: Required reference not found in opened documents: "
                << ref << std::endl;
    }
  }

  std::set<size_t> builtRequiredSchemas;
  std::vector<std::map<UriWrapper, size_t>> dependencyList(
      unresolvedSchemas.size());

  while (!buildableRequiredSchemas.empty()) {
    auto schemaIdx = *buildableRequiredSchemas.begin();
    auto& schema = *unresolvedSchemas[schemaIdx];
    buildableRequiredSchemas.erase(schemaIdx);
    builtRequiredSchemas.insert(schemaIdx);
    const auto dependencies = linkers[unresolvedSchemas[schemaIdx]->draft_](
        schema.json_.get(), schema.baseUri_, schema.pointer_);
    std::map<UriWrapper, size_t> depIndices_;
    for (const auto& dep : dependencies) {
      if (schemaIndices.contains(dep)) {
        depIndices_.emplace(dep, schemaIndices.at(dep));
      } else {
        std::cerr << "Warning: Dependency not found in opened documents: "
                  << dep << std::endl;
      }
    }
    auto& depIndices = std::as_const(depIndices_);
    std::set<size_t> newDeps;
    for (const auto& [_, idx] : depIndices) {
      newDeps.insert(idx);
    }
    for (const auto idx : newDeps) {
      if (!builtRequiredSchemas.contains(idx)) {
        buildableRequiredSchemas.insert(idx);
      }
    }
    dependencyList[schemaIdx] = depIndices;
  }

  std::map<size_t, size_t> shrinkMap;
  for (const auto builtIdx : builtRequiredSchemas) {
    shrinkMap.emplace(builtIdx, shrinkMap.size());
  }

  std::vector<std::map<UriWrapper, size_t>> shrunkDependencyList;
  for (const auto builtIdx : builtRequiredSchemas) {
    auto& deps = dependencyList[builtIdx];
    std::map<UriWrapper, size_t> shrunkDeps;
    for (const auto& [uri, idx] : deps) {
      shrunkDeps.emplace(uri, shrinkMap.at(idx));
    }
    shrunkDependencyList.push_back(shrunkDeps);
  }

  std::vector<std::unique_ptr<UnresolvedSchema>> shrunkSchemas;
  for (const auto builtIdx : builtRequiredSchemas) {
    shrunkSchemas.push_back(std::move(unresolvedSchemas[builtIdx]));
  }

  std::vector<std::unique_ptr<LinkedSchema>> linkedSchemas;
  for (size_t i = 0; i < shrunkSchemas.size(); i++) {
    const auto& schema = *shrunkSchemas[i];
    auto& shrunkDependency = shrunkDependencyList[i];
    linkedSchemas.emplace_back(
        std::make_unique<LinkedSchema>(schema, shrunkDependency));
  }

  return linkedSchemas;

  // Set up the initial required schemas

  // SetMap<UriWrapper, LinkedSchema> resolvedSchemas;
  // std::set<const UnresolvedSchema*> buildableRequiredSchemas;
  // std::set<const UnresolvedSchema*> builtRequiredSchemas;
  // // Set up the initial required schemas
  // for (const auto& ref : requiredReferences) {
  //   if (setMap.contains(ref)) {
  //     buildableRequiredSchemas.insert(&setMap[ref]);
  //   } else {
  //     std::cerr << "Warning: Required reference not found in setMap: " << ref
  //               << std::endl;
  //   }
  // }

  // // Build each dependency, adding new dependencies as they are found
  // while (!buildableRequiredSchemas.empty()) {
  //   auto schema = *buildableRequiredSchemas.begin();
  //   buildableRequiredSchemas.erase(schema);
  //   builtRequiredSchemas.insert(schema);
  //   auto buildableSchema = construct(*schema);
  //   auto& deps = buildableSchema->dependenciesSet_;
  //   addDependencies(deps, setMap, buildableRequiredSchemas,
  //                   builtRequiredSchemas);
  //   std::set<UriWrapper> schemaKeys = setMap.keysOfValue(*schema);
  //   resolvedSchemas.bulkInsert(schemaKeys, std::move(buildableSchema));
  // }

  // std::vector<LinkedSchema*> schemaPtrs;
  // for (const auto& schema : resolvedSchemas.getSet()) {
  //   schemaPtrs.push_back(schema.get());
  // }

  // // Fills in the dependency map of a schema
  // const auto fillDepMap =
  //     [&resolvedSchemas = std::as_const(resolvedSchemas),
  //      &schemaPtrs = std::as_const(schemaPtrs)](LinkedSchema& schema) {
  //       for (const auto& depUri : schema.dependenciesSet_) {
  //         if (resolvedSchemas.contains(depUri)) {
  //           const auto depSchemaPtr = &resolvedSchemas[depUri];
  //           size_t index = std::distance(
  //               schemaPtrs.begin(),
  //               std::find(schemaPtrs.begin(), schemaPtrs.end(),
  //               depSchemaPtr));
  //           schema.dependencies_.emplace(depUri, index);
  //           // schema.dependencies_.emplace(depUri,
  //           // std::cref(resolvedSchemas[depUri]));
  //         } else {
  //           std::cerr << "Warning: Dependency not found in resolvedSchemas: "
  //                     << depUri << std::endl;
  //         }
  //       }
  //     };

  // // Fill in the dependency map of each schema
  // for (const auto [_, schemaRef] : resolvedSchemas) {
  //   fillDepMap(schemaRef);
  // }

  // // Extract the schemas from setMap
  // auto extracted = resolvedSchemas.extract();

  // std::vector<std::unique_ptr<LinkedSchema>> schemas;
  // schemas.resize(extracted.size());
  // for (auto& [keys, value] : extracted) {
  //   const auto index = std::distance(
  //       schemaPtrs.begin(),
  //       std::find(schemaPtrs.begin(), schemaPtrs.end(), value.get()));
  //   schemas[index] = std::move(value);
  // }
  // return schemas;
}
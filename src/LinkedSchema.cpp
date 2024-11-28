#include "LinkedSchema.h"
#include "Draft07.h"
std::map<Draft, std::set<UriWrapper> (*)(const nlohmann::json&,
                                         const UriWrapper&, const JSONPointer&)>
    linkers{
        {Draft::DRAFT_07, getDraft07Dependencies},
    };

std::map<Draft,
         std::vector<std::string> (*)(const nlohmann::json&, const UriWrapper&,
                                      const JSONPointer&)>
    issueCheckers{
        {Draft::DRAFT_07, issuesWithDraft07Schema},
    };

std::vector<std::unique_ptr<LinkedSchema>>
resolveDependencies(SetMap<UriWrapper, UnresolvedSchema>&& setMap,
                    const std::set<UriWrapper>& requiredReferences) {
  try {
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
        std::cerr
            << "Warning: Required reference not found in opened documents: "
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
  } catch (const std::exception& e) {
    std::cerr << "Error caught in resolveDependencies:\n";
    throw e;
  }
}

std::vector<std::string> LinkedSchema::generateIssuesList(
    const std::vector<std::unique_ptr<LinkedSchema>>& schemas) {
  try {
    std::vector<std::string> issues;
    for (const auto& schema : schemas) {
      const auto& issueChecker = issueCheckers.at(schema->draft_);
      const auto schemaIssues =
          issueChecker(schema->json_.get(), schema->baseUri_, schema->pointer_);
      issues.insert(issues.end(), schemaIssues.begin(), schemaIssues.end());
    }
    return issues;
  } catch (const std::exception& e) {
    std::cerr << "Error caught in LinkedSchema::generateIssuesList:\n";
    throw e;
  }
}

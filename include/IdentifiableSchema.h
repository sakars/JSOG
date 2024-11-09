#include "LinkedSchema.h"
#include <map>

class IdentifiableSchema {

public:
  std::map<UriWrapper, std::reference_wrapper<IdentifiableSchema>>
      dependencies_;

  const std::string identifier_;
  const std::reference_wrapper<const nlohmann::json> json_;

  const UriWrapper baseUri_;
  const JSONPointer pointer_;
  const Draft draft_;

  IdentifiableSchema(const nlohmann::json& json, const UriWrapper& baseUri,
                     const JSONPointer& pointer, Draft draft,
                     const std::string& identifier)
      : identifier_(identifier), json_(json), baseUri_(baseUri),
        pointer_(pointer), draft_(draft) {}

  static std::vector<std::unique_ptr<IdentifiableSchema>>
  transition(std::vector<std::unique_ptr<LinkedSchema>>&& linkedSchemas) {
    // Perform schema link indexing.
    std::vector<std::map<UriWrapper, size_t>> linkedSchemaDependencyIndices;
    for (const auto& linkedSchema : linkedSchemas) {
      std::map<UriWrapper, size_t> dependencyIndices;
      for (const auto& [uri, dep] : linkedSchema->dependencies_) {
        auto it =
            std::find_if(linkedSchemas.begin(), linkedSchemas.end(),
                         [&dep](const std::unique_ptr<LinkedSchema>& schema) {
                           return &dep.get() == schema.get();
                         });
        if (it != linkedSchemas.end()) {
          dependencyIndices[uri] = std::distance(linkedSchemas.begin(), it);
        } else {
          throw std::runtime_error("Dependency not found in linkedSchemas");
        }
      }
      linkedSchemaDependencyIndices.push_back(dependencyIndices);
    }

    // Construct the identifiable schemas
    // Note: We add True to the identifiers to avoid conflicts with the
    // predefined True schema.
    std::set<std::string> identifiers{"True", "False"};
    std::vector<std::unique_ptr<IdentifiableSchema>> identifiableSchemas;
    for (size_t i = 0; i < linkedSchemas.size(); i++) {
      const auto& linkedSchema = linkedSchemas[i];
      const std::string preferred_identifier =
          linkedSchema->getPreferredIdentifier();
      std::string identifier = preferred_identifier;
      size_t j = 0;
      while (identifiers.count(identifier) > 0) {
        bool preferred_identifier_has_number_at_end =
            preferred_identifier.back() >= '0' &&
            preferred_identifier.back() <= '9';
        identifier = preferred_identifier +
                     (preferred_identifier_has_number_at_end ? "_" : "") +
                     std::to_string(j);
        j++;
      }
      identifiers.insert(identifier);
      auto schema = std::make_unique<IdentifiableSchema>(
          linkedSchema->json_, linkedSchema->baseUri_, linkedSchema->pointer_,
          linkedSchema->draft_, identifier);
      identifiableSchemas.push_back(std::move(schema));
    }

    // Relink the schemas
    for (size_t i = 0; i < identifiableSchemas.size(); i++) {
      auto& schema = identifiableSchemas[i];
      for (const auto& [uri, index] : linkedSchemaDependencyIndices[i]) {
        schema->dependencies_.emplace(uri,
                                      std::ref(*identifiableSchemas[index]));
      }
    }
    return identifiableSchemas;
  }
};

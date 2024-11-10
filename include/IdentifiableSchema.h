#ifndef IDENTIFIABLESCHEMA_H
#define IDENTIFIABLESCHEMA_H

#include "LinkedSchema.h"
#include <map>

class IdentifiableSchema {

public:
  std::map<UriWrapper, size_t> dependencies_;

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

  static std::vector<IdentifiableSchema>
  transition(std::vector<std::unique_ptr<LinkedSchema>>&& linkedSchemas) {

    // Construct the identifiable schemas
    // Note: We add reserved identifiers to the identifiers to avoid conflicts
    // with the predefined schemas.
    std::set<std::string> identifiers{"True", "False"};
    std::vector<IdentifiableSchema> identifiableSchemas;
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
      auto schema = IdentifiableSchema(
          linkedSchema->json_, linkedSchema->baseUri_, linkedSchema->pointer_,
          linkedSchema->draft_, identifier);
      schema.dependencies_ = linkedSchema->dependencies_;
      identifiableSchemas.push_back(std::move(schema));
    }
    return identifiableSchemas;
  }

  static void
  dumpSchemas(std::vector<IdentifiableSchema>& identifiableSchemas) {
    auto iDump = R"({"dependencies":{}})"_json;
    for (const auto& iSchema : identifiableSchemas) {
      auto& uriDump =
          iDump["dependencies"][iSchema.baseUri_.toString().value()];
      auto& ptrDump = uriDump[iSchema.pointer_.toFragment()];
      for (const auto& [uri, idx] : iSchema.dependencies_) {
        const auto& idxSchema = identifiableSchemas[idx];
        ptrDump.push_back(nlohmann::json::array(
            {uri.toString().value(),
             idxSchema.baseUri_.withPointer(idxSchema.pointer_)
                 .toString()
                 .value()}));
      }
    }

    std::ofstream iDumpFile("identifiable.dump.json");
    iDumpFile << iDump.dump(2);
    iDumpFile.close();
  }
};

#endif // IDENTIFIABLESCHEMA_H
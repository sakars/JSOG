#ifndef DRAFT_RECOGNISER_H
#define DRAFT_RECOGNISER_H

#include <nlohmann/json.hpp>
#include "UriWrapper.h"

enum class Draft
{
  DRAFT_07
};

// TODO: Document and move to a cpp file.
inline Draft identifySchemaDraft(const nlohmann::json &json)
{
  // Check $schema
  if (json.contains("$schema") && json["$schema"].is_string())
  {
    // normalize schema field.
    auto schemaField = UriWrapper(json["$schema"].get<std::string>());
    schemaField.normalize();
    auto draft07SchemaUri =
        UriWrapper("http://json-schema.org/draft-07/schema#");
    schemaField.normalize();
    if (schemaField == draft07SchemaUri)
    {
      return Draft::DRAFT_07;
    }
    else
    {
      std::cerr << "Unknown Schema attempted resolution. Schema field: "
                << schemaField << "draft07 uri\n"
                << draft07SchemaUri << std::endl;
    }
  }

  // TODO: Add other detection methods to know in case schema field is
  // missing or doesn't match recognized schemas.
  return Draft::DRAFT_07;
}

#endif // DRAFT_RECOGNISER_H
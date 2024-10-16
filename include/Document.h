#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "Schema.h"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

class ResourceIndex;

class Document {
public:
  std::vector<std::unique_ptr<Schema>> schemas;
  nlohmann::json json;
  std::reference_wrapper<ResourceIndex> index;

  Document(const std::filesystem::path &path, ResourceIndex &index);

  Schema &makeSchema(const nlohmann::json &source);
};

#endif // DOCUMENT_H
#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "UriWrapper.h"
#include "Schema.h"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <format>

/// @brief Represents a JSON document
/// @details This class represents a JSON document. It is a wrapper around the
/// nlohmann::json class that provides additional functionality.
class Document
{
public:
  nlohmann::json json;
  UriWrapper fileUri;

  Document(const nlohmann::json &json, const UriWrapper &fileUri)
      : json(json), fileUri(fileUri) {}

  Document(const std::filesystem::path &path)
  {
    std::ifstream file(path);
    if (!file.is_open())
    {
      throw std::runtime_error(std::format("Could not open file {}", path.string()));
    }
    file >> json;
    const auto absPath = std::filesystem::absolute(path);
    const auto uri = "file://" + absPath.string();
    fileUri = UriWrapper(uri);
  }
};

#endif // DOCUMENT_H
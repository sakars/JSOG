#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "UriWrapper.h"
#include <filesystem>
#include <format>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

/// @brief Represents a JSON document
/// @details This class represents a JSON document. It is a wrapper around the
/// nlohmann::json class that provides additional functionality.
class Document {
public:
  nlohmann::json json_;
  UriWrapper fileUri_;

  Document(nlohmann::json&& json, UriWrapper&& fileUri)
      : json_(std::move(json)), fileUri_(std::move(fileUri)) {
    fileUri_.normalize();
  }

  Document(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
      throw std::runtime_error(
          std::format("Error: Could not open file {}", path.string()));
    }
    try {
      file >> json_;
    } catch (const nlohmann::json::parse_error& e) {
      throw std::runtime_error(
          std::format("Error: Could not parse JSON in file {}: {}",
                      path.string(), e.what()));
    }
    const auto absPath = std::filesystem::absolute(path);
    const auto uri = "file://" + absPath.string();
    fileUri_ = UriWrapper(uri);
  }
};

inline std::vector<Document>
loadDocuments(const std::vector<std::filesystem::path>& paths) {
  std::vector<Document> documents;
  documents.reserve(paths.size());
  for (const auto& path : paths) {
    documents.emplace_back(path);
  }
  return documents;
}

#endif // DOCUMENT_H
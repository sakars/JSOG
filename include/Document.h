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
  nlohmann::json json;

  Document(const std::filesystem::path &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
      throw std::runtime_error("Could not open file");
    }
    file >> json;
  }
};

#endif // DOCUMENT_H
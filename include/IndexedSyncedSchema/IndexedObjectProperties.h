#ifndef INDEXED_OBJECT_PROPERTIES_H
#define INDEXED_OBJECT_PROPERTIES_H

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

struct IndexedObjectProperties {
  std::optional<size_t> maxProperties_;
  std::optional<size_t> minProperties_;
  std::optional<std::set<std::string>> required_;
  std::map<std::string, size_t> properties_;
  std::optional<std::map<std::string, size_t>> patternProperties_;
  std::optional<size_t> additionalProperties_;
  std::optional<std::map<std::string, std::vector<std::string>>>
      propertyDependencies_;
  std::optional<std::map<std::string, size_t>> schemaDependencies_;
  std::optional<size_t> propertyNames_;
};

#endif // INDEXED_OBJECT_PROPERTIES_H
#ifndef INDEXED_ARRAY_PROPERTIES_H
#define INDEXED_ARRAY_PROPERTIES_H

#include <optional>
#include <vector>

struct IndexedArrayProperties {
  std::optional<std::vector<size_t>> tupleableItems_;
  std::optional<size_t> items_;
  std::optional<size_t> maxItems_;
  std::optional<size_t> minItems_;
  std::optional<bool> uniqueItems_;
  std::optional<size_t> contains_;
};

#endif // INDEXED_ARRAY_PROPERTIES_H
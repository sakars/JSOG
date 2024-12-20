#ifndef INDEXED_REINTERPRETABLES_H
#define INDEXED_REINTERPRETABLES_H

#include <optional>
#include <vector>

struct IndexedReinterpretables {
  std::optional<size_t> ref_;

  std::optional<size_t> if_;
  std::optional<size_t> then_;
  std::optional<size_t> else_;
  std::optional<std::vector<size_t>> allOf_;
  std::optional<std::vector<size_t>> anyOf_;
  std::optional<std::vector<size_t>> oneOf_;
  std::optional<size_t> not_;
};

#endif // INDEXED_REINTERPRETABLES_H
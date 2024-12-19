
#include <optional>
#include <vector>

class SyncedSchema;

struct Reinterpretables {
  std::optional<std::reference_wrapper<SyncedSchema>> ref_;
  std::optional<std::reference_wrapper<const SyncedSchema>> if_;
  std::optional<std::reference_wrapper<const SyncedSchema>> then_;
  std::optional<std::reference_wrapper<const SyncedSchema>> else_;

  std::optional<std::vector<std::reference_wrapper<const SyncedSchema>>> allOf_;
  std::optional<std::vector<std::reference_wrapper<const SyncedSchema>>> anyOf_;
  std::optional<std::vector<std::reference_wrapper<const SyncedSchema>>> oneOf_;
  std::optional<std::reference_wrapper<const SyncedSchema>> not_;
};
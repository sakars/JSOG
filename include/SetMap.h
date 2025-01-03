#ifndef SETMAP_H
#define SETMAP_H

#include <map>
#include <memory>
#include <set>

/// @brief A container that allows for multiple keys to reference the same value
/// and a reverse lookup of keys for a value. Useful if a map is required with
/// frequent lookup of keys for a value.
/// @tparam K The key value type
/// @tparam V The value type
template <typename K, typename V> class SetMap {
  std::map<K, std::reference_wrapper<V>> map;
  std::map<const V*, std::set<K>> reverseMap;
  std::set<std::unique_ptr<V>> set;

public:
  const std::set<std::unique_ptr<V>>& getSet() const { return set; }
  class iterator {
    friend class SetMap;
    using InternalIterator = std::set<std::unique_ptr<V>>::iterator;

    InternalIterator it_;
    SetMap& setMap_;

    iterator(InternalIterator it, SetMap& map) : it_(it), setMap_(map) {}

  public:
    std::tuple<std::reference_wrapper<const std::set<K>>,
               std::reference_wrapper<V>>
    operator*() {
      return {setMap_.keysOfValue(*it_->get()), std::ref(*it_->get())};
    }

    iterator& operator++() {
      ++it_;
      return *this;
    }

    bool operator==(const iterator& other) const { return it_ == other.it_; }
    bool operator!=(const iterator& other) const { return it_ != other.it_; }
  };

  iterator begin() { return iterator(set.begin(), *this); }
  iterator end() { return iterator(set.end(), *this); }

  V& operator[](const K& key) { return map.at(key); }

  const V& operator[](const K& key) const { return map.at(key); }

  bool contains(const K& key) const { return map.contains(key); }

  std::set<K>& keysOfValue(const V& value) { return reverseMap.at(&value); }
  std::set<K> equivalentKeys(const K& key) { return keysOfValue(map.at(key)); }

  /// @brief Inserts a key set for a value into the map
  void bulkInsert(std::set<K> keys, V&& value) {
    auto up = std::make_unique<V>(std::move(value));
    bulkInsert(keys, std::move(up));
  }

  /// @brief Inserts a key set for a value into the map
  void bulkInsert(std::set<K> keys, const V& value) {
    auto up = std::make_unique<V>(value);
    bulkInsert(keys, std::move(up));
  }

  /// @brief Inserts a key set for a value into the map
  void bulkInsert(std::set<K> keys, std::unique_ptr<V> value) {
    reverseMap[value.get()] = keys;
    for (auto& key : keys) {
      map.emplace(key, std::ref<V>(*value));
    }
    set.emplace(std::move(value));
  }

  /// @brief Clears the map and the reverse map and returns the values into
  /// a vector of tuples containing the keys and the values
  std::vector<std::tuple<std::vector<K>, std::unique_ptr<V>>> extract() {
    std::vector<std::tuple<std::vector<K>, std::unique_ptr<V>>> extracted;
    while (!set.empty()) {
      std::vector<K> keys;
      std::unique_ptr<V> value = std::move(set.extract(set.begin()).value());
      for (const auto& [key, val] : map) {
        if (&val.get() == value.get()) {
          keys.emplace_back(key);
        }
      }
      auto tuple = std::make_tuple(keys, std::move(value));
      extracted.emplace_back(std::move(tuple));
    }

    set.clear();
    map.clear();
    reverseMap.clear();
    return extracted;
  }
};

#endif // SETMAP_H
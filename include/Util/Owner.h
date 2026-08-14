#pragma once

// GSL_OWNER documents a raw handle that must be released by the containing
// type. Keep a local fallback so the project does not require the full
// Guidelines Support Library.
#ifndef GSL_OWNER
#define GSL_OWNER
#endif

#include <algorithm>
#include <memory>
#include <vector>

namespace z8 {
template <typename T> class Owner {
  std::unique_ptr<T> Own;

public:
  Owner() = default;
  Owner(const Owner &) = delete;
  Owner &operator=(const Owner &) = delete;
  Owner(Owner &&) noexcept = default;
  Owner &operator=(Owner &&) noexcept = default;

  template <typename U = T, typename... ArgTys,
            typename = std::enable_if_t<std::is_base_of_v<T, U>>>
  U *set(ArgTys &&...args) {
    auto Ptr = std::make_unique<U>(std::forward<ArgTys>(args)...);
    auto *Ref = Ptr.get();
    Own = std::move(Ptr);
    return Ref;
  }

  template <typename U = T,
            typename = std::enable_if_t<std::is_base_of_v<T, U>>>
  U *set(std::unique_ptr<U> Ptr) {
    auto *Ref = Ptr.get();
    Own = std::move(Ptr);
    return Ref;
  }

  T *get() { return Own.get(); }
  const T *get() const { return Own.get(); }
  void reset() noexcept { Own.reset(); }

  [[nodiscard]]
  explicit operator bool() const noexcept {
    return static_cast<bool>(Own);
  }
};

template <typename T> class OwnerArray {
public:
  using value_type = T;
  using pointer = T *;
  using reference = T &;
  using const_pointer = const T *;
  using const_reference = const T &;

private:
  std::vector<std::unique_ptr<T>> Owns;
  std::vector<T *> Refs;

public:
  OwnerArray() = default;
  OwnerArray(const OwnerArray &) = delete;
  OwnerArray &operator=(const OwnerArray &) = delete;
  OwnerArray(OwnerArray &&) noexcept = default;
  OwnerArray &operator=(OwnerArray &&) noexcept = default;

  template <typename U = T, typename... ArgTys,
            typename = std::enable_if_t<std::is_base_of_v<T, U>>>
  U *add(ArgTys &&...args) {
    auto Own = std::make_unique<U>(std::forward<ArgTys>(args)...);
    auto *Ref = Own.get();
    Owns.push_back(std::move(Own));
    Refs.push_back(Ref);
    return Ref;
  }

  template <typename U = T,
            typename = std::enable_if_t<std::is_base_of_v<T, U>>>
  U *add(std::unique_ptr<U> Own) {
    auto *Ref = Own.get();
    Owns.push_back(std::move(Own));
    Refs.push_back(Ref);
    return Ref;
  }

  T &operator[](std::size_t Index) { return *Refs[Index]; }
  const T &operator[](std::size_t Index) const { return *Refs[Index]; }

  T &at(std::size_t Index) { return *Refs.at(Index); }
  const T &at(std::size_t Index) const { return *Refs.at(Index); }

  T &front() { return *Refs.front(); }
  const T &front() const { return *Refs.front(); }

  T &back() { return *Refs.back(); }
  const T &back() const { return *Refs.back(); }

  [[nodiscard]]
  std::size_t size() const noexcept {
    return Refs.size();
  }

  [[nodiscard]]
  bool empty() const noexcept {
    return Refs.empty();
  }

  void reserve(std::size_t Size) {
    Owns.reserve(Size);
    Refs.reserve(Size);
  }

  void clear() noexcept {
    Refs.clear();
    Owns.clear();
  }

  [[nodiscard]]
  const std::vector<T *> &get() const noexcept {
    return Refs;
  }

  auto begin() noexcept { return Refs.begin(); }
  auto end() noexcept { return Refs.end(); }

  auto begin() const noexcept { return Refs.begin(); }
  auto end() const noexcept { return Refs.end(); }

  auto cbegin() const noexcept { return Refs.cbegin(); }
  auto cend() const noexcept { return Refs.cend(); }
};
} // namespace z8

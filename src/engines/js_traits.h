#pragma once

#include <memory>
#include <type_traits>

// NOLINTBEGIN(readability-identifier-naming)
template <typename T>
struct raw_ptr_type {
  using type = T*;
};

template <typename T>
struct is_shared_ptr_helper : std::false_type {};

template <typename T>
struct is_shared_ptr_helper<std::shared_ptr<T>> : std::true_type {};

template <typename T>
struct is_shared_ptr : is_shared_ptr_helper<std::remove_cv_t<T>> {};

// C++14/17 versions
template <typename T>
constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;

template <typename T>
struct shared_ptr_inner {
private:
  // Remove cv-qualifiers first
  using raw_type = std::remove_cv_t<T>;

  // Check if it's a shared_ptr
  template <typename U>
  static std::false_type test(...);

  template <typename U>
  static auto test(U*)
      -> std::enable_if_t<std::is_same_v<U, std::shared_ptr<typename U::element_type>>,
                          std::true_type>;

public:
  using type = std::
      conditional_t<decltype(test<raw_type>(nullptr))::value, typename raw_type::element_type, T>;
};

template <typename T>
using shared_ptr_inner_t = typename shared_ptr_inner<T>::type;

// NOLINTEND(readability-identifier-naming)

//==----------- annotated_arg.hpp - SYCL annotated_arg extension -----------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#pragma once

#include <cstddef>
#include <type_traits>

#include <sycl/detail/stl_type_traits.hpp>
#include <sycl/exception.hpp>
#include <sycl/ext/oneapi/annotated_arg/properties.hpp>
#include <sycl/ext/oneapi/properties/properties.hpp>

#ifdef __SYCL_DEVICE_ONLY__
#define __SYCL_HOST_NOT_SUPPORTED(Op)
#else
#define __SYCL_HOST_NOT_SUPPORTED(Op)                                          \
  throw sycl::exception(                                                       \
      sycl::make_error_code(sycl::errc::feature_not_supported),                \
      Op " is not supported on host device.");
#endif

namespace sycl {
__SYCL_INLINE_VER_NAMESPACE(_V1) {
namespace ext {
namespace oneapi {
namespace experimental {

template <typename T, typename PropertyListT = detail::empty_properties_t>
class annotated_arg {
  // This should always fail when instantiating the unspecialized version.
  static_assert(is_property_list<PropertyListT>::value,
                "Property list is invalid.");
};

// Partial specialization to make PropertyListT visible as a parameter pack
// of properties.
template <typename T, typename... Props>
class __SYCL_SPECIAL_CLASS annotated_arg<T, detail::properties_t<Props...>> {

  using property_list_t = detail::properties_t<Props...>;

  T obj;
  #ifdef __SYCL_DEVICE_ONLY__
    void __init(
      [[__sycl_detail__::add_ir_attributes_kernel_parameter(
          detail::PropertyMetaInfo<Props>::name...,
          detail::PropertyMetaInfo<Props>::value...
      )]]
      T _obj) {
        obj = _obj;
    }
  #endif

public:
  // T should be trivially copy constructible to be device copyable
  static_assert(std::is_trivially_copy_constructible<T>::value,
                "Type T must be trivially copy constructable.");
  static_assert(std::is_trivially_destructible<T>::value,
                "Type T must be trivially destructible.");
  static_assert(is_property_list<property_list_t>::value,
                "Property list is invalid.");

  // Check compability of each property values in the property list
  static_assert(check_property_list<T, Props...>::value,
                "property list contains invalid property.");

  annotated_arg() = default;
  annotated_arg(const annotated_arg&) = default;
  annotated_arg(const T& _obj) : obj(_obj) {};

  operator T&() {
    __SYCL_HOST_NOT_SUPPORTED("Implicit conversion of device_global to T")
    return obj;
  }
  operator const T&() const {
    __SYCL_HOST_NOT_SUPPORTED("Implicit conversion of device_global to T")
    return obj;
  }

  inline T& get() {
    __SYCL_HOST_NOT_SUPPORTED("get()")
    return obj;
  }
  inline const T& get() const {
    __SYCL_HOST_NOT_SUPPORTED("get()")
    return obj;
  }

  template <typename PropertyT> static constexpr bool has_property() {
    return property_list_t::template has_property<PropertyT>();
  }

  template <typename PropertyT> static constexpr auto get_property() {
    return property_list_t::template get_property<PropertyT>();
  }
};

} // namespace experimental
} // namespace oneapi
} // namespace ext
} // __SYCL_INLINE_VER_NAMESPACE(_V1)
} // namespace sycl

#undef __SYCL_HOST_NOT_SUPPORTED

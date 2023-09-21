//==-------- alloc_device.hpp - SYCL annotated usm device allocation -------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#pragma once

#include <sycl/ext/oneapi/experimental/annotated_usm/alloc_base.hpp>

namespace sycl {
inline namespace _V1 {
namespace ext {
namespace oneapi {
namespace experimental {

template <typename T, typename ListA, typename ListB>
using CheckDevicePtrTAndPropLists =
    CheckTAndPropListsWithUsmKind<alloc::device, T, ListA, ListB>;

template <typename PropertyListT>
using GetAnnotatedDevicePtrProperties =
    GetAnnotatedPtrPropertiesWithUsmKind<alloc::device, PropertyListT>;

////
//  "aligned_alloc_device_annotated": aligned device USM allocation functions with properties support
//
//  This the base form of all the annotated USM device allocation functions, which are implemented by
//  calling the more generic "aligned_alloc_annotated" functions with the USM kind as an argument. 
//  Note that when calling "aligned_alloc_annotated", the template parameter `propertyListA` should
//  include the `usm_kind<alloc::device>` property to make it appear on the returned annotated_ptr
//  of "aligned_alloc_annotated"
////
template <typename propertyListA = detail::empty_properties_t,
          typename propertyListB =
              typename GetAnnotatedDevicePtrProperties<propertyListA>::type>
std::enable_if_t<
    CheckDevicePtrTAndPropLists<void, propertyListA, propertyListB>::value,
    annotated_ptr<void, propertyListB>>
aligned_alloc_device_annotated(size_t alignment, size_t numBytes,
                               const device &syclDevice,
                               const context &syclContext,
                               const propertyListA &propList = properties{}) {
  VALIDATE_PROPERTIES(void);
  return aligned_alloc_annotated<MergeUsmKind<alloc::device, propertyListA>>(alignment, numBytes, syclDevice, syclContext, alloc::device);
}

template <typename T, typename propertyListA = detail::empty_properties_t,
          typename propertyListB =
              typename GetAnnotatedDevicePtrProperties<propertyListA>::type>
std::enable_if_t<
    CheckDevicePtrTAndPropLists<T, propertyListA, propertyListB>::value,
    annotated_ptr<T, propertyListB>>
aligned_alloc_device_annotated(size_t alignment, size_t count,
                               const device &syclDevice,
                               const context &syclContext,
                               const propertyListA &propList = properties{}) {
  VALIDATE_PROPERTIES(T);
  return aligned_alloc_annotated<T, MergeUsmKind<alloc::device, propertyListA>>(alignment, count, syclDevice, syclContext, alloc::device);
}

template <typename propertyListA = detail::empty_properties_t,
          typename propertyListB =
              typename GetAnnotatedDevicePtrProperties<propertyListA>::type>
std::enable_if_t<
    CheckDevicePtrTAndPropLists<void, propertyListA, propertyListB>::value,
    annotated_ptr<void, propertyListB>>
aligned_alloc_device_annotated(size_t alignment, size_t numBytes,
                               const queue &syclQueue,
                               const propertyListA &propList = properties{}) {
  return aligned_alloc_device_annotated<propertyListA>(alignment, numBytes,
                                        syclQueue.get_device(),
                                        syclQueue.get_context());
}

template <typename T, typename propertyListA = detail::empty_properties_t,
          typename propertyListB =
              typename GetAnnotatedDevicePtrProperties<propertyListA>::type>
std::enable_if_t<
    CheckDevicePtrTAndPropLists<T, propertyListA, propertyListB>::value,
    annotated_ptr<T, propertyListB>>
aligned_alloc_device_annotated(size_t alignment, size_t count,
                               const queue &syclQueue,
                               const propertyListA &propList = properties{}) {
  return aligned_alloc_device_annotated<T, propertyListA>(alignment, count,
                                           syclQueue.get_device(),
                                           syclQueue.get_context());
}

////
//  "malloc_device_annotated": device USM allocation functions with properties support
//
//  Note: "malloc_device_annotated" functions call "aligned_alloc_device_annotated"
//  with alignment 0
////
template <typename propertyListA = detail::empty_properties_t,
          typename propertyListB =
              typename GetAnnotatedDevicePtrProperties<propertyListA>::type>
std::enable_if_t<
    CheckDevicePtrTAndPropLists<void, propertyListA, propertyListB>::value,
    annotated_ptr<void, propertyListB>>
malloc_device_annotated(size_t numBytes, const device &syclDevice,
                        const context &syclContext,
                        const propertyListA &propList = properties{}) {
  VALIDATE_PROPERTIES(void);
  return aligned_alloc_device_annotated<propertyListA>(0, numBytes, syclDevice, syclContext);
}

template <typename T, typename propertyListA = detail::empty_properties_t,
          typename propertyListB =
              typename GetAnnotatedDevicePtrProperties<propertyListA>::type>
std::enable_if_t<
    CheckDevicePtrTAndPropLists<T, propertyListA, propertyListB>::value,
    annotated_ptr<T, propertyListB>>
malloc_device_annotated(size_t count, const device &syclDevice,
                        const context &syclContext,
                        const propertyListA &propList = properties{}) {
  VALIDATE_PROPERTIES(T);
  return malloc_device_annotated<T, propertyListA>(count, syclDevice, syclContext);
}

template <typename propertyListA = detail::empty_properties_t,
          typename propertyListB =
              typename GetAnnotatedDevicePtrProperties<propertyListA>::type>
std::enable_if_t<
    CheckDevicePtrTAndPropLists<void, propertyListA, propertyListB>::value,
    annotated_ptr<void, propertyListB>>
malloc_device_annotated(size_t numBytes, const queue &syclQueue,
                        const propertyListA &propList = properties{}) {
  return malloc_device_annotated<propertyListA>(numBytes, syclQueue.get_device(),
                                 syclQueue.get_context());
}

template <typename T, typename propertyListA = detail::empty_properties_t,
          typename propertyListB =
              typename GetAnnotatedDevicePtrProperties<propertyListA>::type>
std::enable_if_t<
    CheckDevicePtrTAndPropLists<T, propertyListA, propertyListB>::value,
    annotated_ptr<T, propertyListB>>
malloc_device_annotated(size_t count, const queue &syclQueue,
                        const propertyListA &propList = properties{}) {
  return malloc_device_annotated<T, propertyListA>(count, syclQueue.get_device(),
                                    syclQueue.get_context());
}

} // namespace experimental
} // namespace oneapi
} // namespace ext
} // namespace _V1
} // namespace sycl
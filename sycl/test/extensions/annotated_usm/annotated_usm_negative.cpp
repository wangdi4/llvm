// RUN: %clangxx -fsycl -fsycl-targets=%sycl_triple -fsyntax-only -Xclang -verify -Xclang -verify-ignore-unexpected=note %s

// Expected failed tests for annotated USM allocation:
// 1. Given properties contain invalid runtime property
// 2. Given properties contain invalid compile-time property
// 3. usm_kind in the property list conflicts with the function name
// 4. required usm_kind is not provided in the property list

#include <sycl/sycl.hpp>

#include "fake_properties.hpp"

#define TEST(f, args...)                                                       \
  { auto ap = f(args); }

using namespace sycl::ext::oneapi::experimental;
using namespace sycl::ext::intel::experimental;
using alloc = sycl::usm::alloc;

constexpr int N = 10;

// clang-format off

void testInvalidRuntimeProperty(sycl::queue &q) {
  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_util.hpp:* {{static assertion failed due to requirement {{.+}}: Found invalid runtime property in the property list.}}
  TEST((malloc_device_annotated<int>), N, q, properties{conduit, cache_config{large_slm}})

  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_util.hpp:* {{static assertion failed due to requirement {{.+}}: Found invalid runtime property in the property list.}}
  TEST((malloc_shared_annotated<int>), N, q, properties{conduit, foo{foo_enum::b}})

  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_util.hpp:* {{static assertion failed due to requirement {{.+}}: Found invalid runtime property in the property list.}}
  TEST((malloc_host_annotated<int>), N, q, properties{conduit, foz{0, 1}})

  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_util.hpp:* {{static assertion failed due to requirement {{.+}}: Found invalid runtime property in the property list.}}
  TEST((malloc_annotated<int>), N, q, alloc::device, properties{conduit, foo{foo_enum::a}, foz{0, 1}})
}

// Expect error when the property list contain invalid comile-time property
// Note that two errors are raised for each case, during:
// 1. validating malloc input properties
// 2. validating annotated_ptr properties
void testInvalidCompileTimeProperty(sycl::queue &q) {
  // expected-error-re@sycl/ext/oneapi/experimental/common_annotated_properties/properties.hpp:* {{static assertion failed due to requirement {{.+}}: Property is invalid for the given type.}}
  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_util.hpp:* {{static assertion failed due to requirement {{.+}}: Found invalid compile-time property in the property list.}}
  TEST(malloc_device_annotated, N, q, properties{conduit, bar})

  // expected-error-re@sycl/ext/oneapi/experimental/common_annotated_properties/properties.hpp:* {{static assertion failed due to requirement {{.+}}: Property is invalid for the given type.}}
  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_util.hpp:* {{static assertion failed due to requirement {{.+}}: Found invalid compile-time property in the property list.}}
  TEST(malloc_device_annotated<T>, N, q, properties{conduit, bar})

  // expected-error-re@sycl/ext/oneapi/experimental/common_annotated_properties/properties.hpp:* {{static assertion failed due to requirement {{.+}}: Property is invalid for the given type.}}
  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_util.hpp:* {{static assertion failed due to requirement {{.+}}: Found invalid compile-time property in the property list.}}
  TEST(aligned_alloc_device_annotated, 1, N, q, properties{conduit, bar})

  // expected-error-re@sycl/ext/oneapi/experimental/common_annotated_properties/properties.hpp:* {{static assertion failed due to requirement {{.+}}: Property is invalid for the given type.}}
  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_util.hpp:* {{static assertion failed due to requirement {{.+}}: Found invalid compile-time property in the property list.}}
  TEST(aligned_alloc_device_annotated<T>, 1, N, q, properties{conduit, bar})

  // expected-error-re@sycl/ext/oneapi/experimental/common_annotated_properties/properties.hpp:* {{static assertion failed due to requirement {{.+}}: Property is invalid for the given type.}}
  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_util.hpp:* {{static assertion failed due to requirement {{.+}}: Found invalid compile-time property in the property list.}}
  TEST(malloc_shared_annotated, N, q, properties{conduit, baz<1>})

  // expected-error-re@sycl/ext/oneapi/experimental/common_annotated_properties/properties.hpp:* {{static assertion failed due to requirement {{.+}}: Property is invalid for the given type.}}
  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_util.hpp:* {{static assertion failed due to requirement {{.+}}: Found invalid compile-time property in the property list.}}
  TEST(malloc_shared_annotated<T>, N, q, properties{conduit, baz<1>})

  // expected-error-re@sycl/ext/oneapi/experimental/common_annotated_properties/properties.hpp:* {{static assertion failed due to requirement {{.+}}: Property is invalid for the given type.}}
  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_util.hpp:* {{static assertion failed due to requirement {{.+}}: Found invalid compile-time property in the property list.}}
  TEST(aligned_alloc_shared_annotated, N, q, properties{conduit, baz<1>})

  // expected-error-re@sycl/ext/oneapi/experimental/common_annotated_properties/properties.hpp:* {{static assertion failed due to requirement {{.+}}: Property is invalid for the given type.}}
  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_util.hpp:* {{static assertion failed due to requirement {{.+}}: Found invalid compile-time property in the property list.}}
  TEST(aligned_alloc_shared_annotated<T>, N, q, properties{conduit, baz<1>})

  // expected-error-re@sycl/ext/oneapi/experimental/common_annotated_properties/properties.hpp:* {{static assertion failed due to requirement {{.+}}: Property is invalid for the given type.}}
  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_util.hpp:* {{static assertion failed due to requirement {{.+}}: Found invalid compile-time property in the property list.}}
  TEST(malloc_host_annotated, N, q, properties{conduit, boo<double>})

  // expected-error-re@sycl/ext/oneapi/experimental/common_annotated_properties/properties.hpp:* {{static assertion failed due to requirement {{.+}}: Property is invalid for the given type.}}
  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_util.hpp:* {{static assertion failed due to requirement {{.+}}: Found invalid compile-time property in the property list.}}
  TEST(malloc_host_annotated<T>, N, q, properties{conduit, boo<double>})

  // expected-error-re@sycl/ext/oneapi/experimental/common_annotated_properties/properties.hpp:* {{static assertion failed due to requirement {{.+}}: Property is invalid for the given type.}}
  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_util.hpp:* {{static assertion failed due to requirement {{.+}}: Found invalid compile-time property in the property list.}}
  TEST(aligned_alloc_host_annotated, N, q, properties{conduit, boo<double>})

  // expected-error-re@sycl/ext/oneapi/experimental/common_annotated_properties/properties.hpp:* {{static assertion failed due to requirement {{.+}}: Property is invalid for the given type.}}
  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_util.hpp:* {{static assertion failed due to requirement {{.+}}: Found invalid compile-time property in the property list.}}
  TEST(aligned_alloc_host_annotated<T>, N, q, properties{conduit, boo<double>})

  // expected-error-re@sycl/ext/oneapi/experimental/common_annotated_properties/properties.hpp:* {{static assertion failed due to requirement {{.+}}: Property is invalid for the given type.}}
  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_util.hpp:* {{static assertion failed due to requirement {{.+}}: Found invalid compile-time property in the property list.}}
  TEST(malloc_annotated, N, q, alloc::device, properties{conduit, bar, baz<1>})

  // expected-error-re@sycl/ext/oneapi/experimental/common_annotated_properties/properties.hpp:* {{static assertion failed due to requirement {{.+}}: Property is invalid for the given type.}}
  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_util.hpp:* {{static assertion failed due to requirement {{.+}}: Found invalid compile-time property in the property list.}}
  TEST(malloc_annotated<T>, N, q, alloc::device, properties{work_group_size})
  TEST(aligned_alloc_annotated<T>, N, q, properties{usm_kind_device, ready_latency})
  TEST(aligned_alloc_annotated, N, q, properties{usm_kind_device, device_image_scope})
}

void testMissingUsmKind(sycl::queue &q) {
  // missing usm kind in property list when it is required
  properties InP{};
  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_base.hpp:* {{static assertion failed due to requirement {{.+}}: USM kind is not specified. Please specify it as an argument or in the input property list.}}
  TEST(malloc_annotated, N, q, InP)
  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_base.hpp:* {{static assertion failed due to requirement {{.+}}: USM kind is not specified. Please specify it as an argument or in the input property list.}}
  TEST((malloc_annotated<int>), N, q, InP)
}


void testConflictingUsmKind(sycl::queue &q) {
  // Conflict usm kinds between function name and property list
  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_util.hpp:* {{static assertion failed due to requirement {{.+}}: Input property list contains conflicting USM kind.}}
  // expected-error@+1 {{no matching function for call to 'malloc_shared_annotated'}}
  TEST((malloc_shared_annotated<int>), N, q, properties{usm_kind_host});

  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_util.hpp:* {{static assertion failed due to requirement {{.+}}: Input property list contains conflicting USM kind.}}
  // expected-error@+1 {{no matching function for call to 'malloc_device_annotated'}}
  TEST((malloc_device_annotated<int>), N, q, properties{usm_kind_host});

  // expected-error-re@sycl/ext/oneapi/experimental/annotated_usm/alloc_util.hpp:* {{static assertion failed due to requirement {{.+}}: Input property list contains conflicting USM kind.}}
  // expected-error@+1 {{no matching function for call to 'malloc_host_annotated'}}
  TEST((malloc_host_annotated<int>), N, q, properties{usm_kind_device});

  // expected-error-re@sycl/ext/oneapi/properties/properties.hpp:* {{static assertion failed due to requirement {{.+}}: Duplicate properties in property list.}}
  properties InvalidPropList{usm_kind_device, usm_kind_host};
}


// clang-format on
int main() {
  sycl::queue q;

  testInvalidRuntimeProperty(q);
  testInvalidCompileTimeProperty(q);
  testMissingUsmKind(q);
  testConflictingUsmKind(q);
  return 0;
}

// RUN: %clangxx -fsycl -fsycl-targets=%sycl_triple -fsyntax-only -Xclang -verify -Xclang -verify-ignore-unexpected=note %s
// expected-no-diagnostics

// Compile-time tests for annotated USM allocation functions

#include "sycl/sycl.hpp"
#include <iostream>

// clang-format on
namespace sycl {
namespace ext::oneapi::experimental {

// Define fake runtime property `foo` to test the malloc support for runtime properties
enum class foo_enum : std::uint16_t { a, b, c };

struct foo {
  foo(foo_enum v) : value(v) {}
  foo_enum value;
};

using foo_key = foo;

template <>
struct is_property_key<foo_key>
    : std::true_type {};

namespace detail {
template <> struct IsRuntimeProperty<foo_key> : std::true_type {};
} // namespace detail

} // namespace ext::oneapi::experimental
} // namespace sycl

// Single test instance
#define TEST(f, args...) {  \
  auto ap = f(args);   \
  APVec.push_back(ap);  \ // type check `APVec.push_back()`
  free(ap, q);}

// Test all the use cases for single allocation function, including:
// 1. specify input properties `InP1` as argument. The template parameter `input properties` and `output properties`
// are automatically inferred. This is the most common use case
// 2. specify input properties `InP1` in the template parameters
// The template parameter `output properties` is automatically inferred
// 3. fully specify all the template parameters: `input properties` and `output properties`
#define TEST_GROUP_VOID(func_name, args...) \
  TEST((func_name), args, InP);  \
  TEST((func_name<decltype(InP)>), args);  \
  TEST((func_name<decltype(InP)>), args, InP);  \
  TEST((func_name<decltype(InP), decltype(OutP)>), args, InP); \
  TEST((func_name<decltype(InP), decltype(OutP)>), args);

// Test all the use cases for single allocation function (with element type T), including:
// 1. specify allocated data type `T` in template parameter, and property
// list as argument. The template parameter `input properties` and `output properties`
// are automatically inferred. This is the most common use case
// 2. specify allocated data type `T` and input properties `InP1` in the template parameters
// The template parameter `output properties` is automatically inferred
// 3. fully specify all the template parameters: `T`, `input properties` and `output properties`
#define TEST_GROUP_T(func_name, args...) \
  TEST((func_name<T>), args, InP);  \
  TEST((func_name<T, decltype(InP)>), args);  \
  TEST((func_name<T, decltype(InP)>), args, InP);  \
  TEST((func_name<T, decltype(InP), decltype(OutP)>), args, InP); \
  TEST((func_name<T, decltype(InP), decltype(OutP)>), args);

using namespace sycl::ext::oneapi::experimental;
using namespace sycl::ext::intel::experimental;
using alloc = sycl::usm::alloc;

constexpr int N = 10;

template <typename T>
void testAlloc() {
  sycl::queue q;
  const sycl::context &Ctx = q.get_context();
  auto Dev = q.get_device();

  // Test device allocation
  {
    // All compile-time properties in the input properties appear on
    // the returned annotated_ptr, and runtime properties do not appear on the
    // returned annotated_ptr (e.g. `cache_config`)
    // properties InP{conduit, buffer_location<5>, foo{foo_enum::a}};
    properties InP{conduit, buffer_location<5>};
    properties OutP{conduit, buffer_location<5>, usm_kind<alloc::device>};

    // Test device allocation in bytes
    {
      std::vector<annotated_ptr<void, decltype(OutP)>> APVec;
      TEST_GROUP_VOID(malloc_device_annotated, N, q);
      TEST_GROUP_VOID(malloc_device_annotated, N, Dev, Ctx);
      TEST_GROUP_VOID(aligned_alloc_device_annotated, 1, N, q);
      TEST_GROUP_VOID(aligned_alloc_device_annotated, 1, N, Dev, Ctx);
    }

    // Test device allocation in elements of T
    {
      std::vector<annotated_ptr<T, decltype(OutP)>> APVec;
      TEST_GROUP_T(malloc_device_annotated, N, q);
      TEST_GROUP_T(malloc_device_annotated, N, Dev, Ctx);
      TEST_GROUP_T(aligned_alloc_device_annotated, 1, N, q);
      TEST_GROUP_T(aligned_alloc_device_annotated, 1, N, Dev, Ctx);
    }
  }

  // Test host allocation
  {
    properties InP{conduit, buffer_location<5>};
    properties OutP{conduit, buffer_location<5>, usm_kind<alloc::host>};
    // subtest: host allocation in bytes
    {
      std::vector<annotated_ptr<void, decltype(OutP)>> APVec;
      TEST_GROUP_VOID(malloc_host_annotated, N, q);
      TEST_GROUP_VOID(malloc_host_annotated, N, Ctx);
      TEST_GROUP_VOID(aligned_alloc_host_annotated, 1, N, q);
      TEST_GROUP_VOID(aligned_alloc_host_annotated, 1, N, Ctx);
    }

    // subtest: host allocation in elements of T
    {
      std::vector<annotated_ptr<T, decltype(OutP)>> APVec;
      TEST_GROUP_T(malloc_host_annotated, N, q);
      TEST_GROUP_T(malloc_host_annotated, N, Ctx);
      TEST_GROUP_T(aligned_alloc_host_annotated, 1, N, q);
      TEST_GROUP_T(aligned_alloc_host_annotated, 1, N, Ctx);
    }
  }

  // Test shared allocation
  {
    properties InP{conduit, buffer_location<5>};
    properties OutP{conduit, buffer_location<5>, usm_kind<alloc::shared>};

    {
      std::vector<annotated_ptr<void, decltype(OutP)>> APVec;
      TEST_GROUP_VOID(malloc_shared_annotated, N, q);
      TEST_GROUP_VOID(malloc_shared_annotated, N, Dev, Ctx);
      TEST_GROUP_VOID(aligned_alloc_shared_annotated, 1, N, q);
      TEST_GROUP_VOID(aligned_alloc_shared_annotated, 1, N, Dev, Ctx);
    }

    // Test shared allocation in elements of T
    {
      std::vector<annotated_ptr<T, decltype(OutP)>> APVec;
      TEST_GROUP_T(malloc_shared_annotated, N, q);
      TEST_GROUP_T(malloc_shared_annotated, N, Dev, Ctx);
      TEST_GROUP_T(aligned_alloc_shared_annotated, 1, N, q);
      TEST_GROUP_T(aligned_alloc_shared_annotated, 1, N, Dev, Ctx);
    }
  }

  // Test alloc functions with usm_kind argument and no usm_kind compile-time
  // property, usm_kind does not appear on the returned annotated_ptr
  {
    properties InP{conduit, buffer_location<5>};
    properties OutP{conduit, buffer_location<5>};

    // Test allocation in bytes
    {
      std::vector<annotated_ptr<void, decltype(OutP)>> APVec;
      TEST_GROUP_VOID(malloc_annotated, N, q, alloc::device);
      TEST_GROUP_VOID(malloc_annotated, N, Dev, Ctx, alloc::device);
      TEST_GROUP_VOID(aligned_alloc_annotated, 1, N, q, alloc::device);
      TEST_GROUP_VOID(aligned_alloc_annotated, 1, N, Dev, Ctx, alloc::host);
    }

    // Test allocation in elements of T
    {
      std::vector<annotated_ptr<T, decltype(OutP)>> APVec;
      TEST_GROUP_T(malloc_annotated, N, q, alloc::host);
      TEST_GROUP_T(malloc_annotated, N, Dev, Ctx, alloc::host);
      TEST_GROUP_T(aligned_alloc_annotated, 1, N, q, alloc::shared);
      TEST_GROUP_T(aligned_alloc_annotated, 1, N, Dev, Ctx, alloc::shared);
    }
  }

  // Test alloc functions where usm_kind property is required in the input
  // property list. usm_kind appears on the returned annotated_ptr
  {
    properties InP{conduit, buffer_location<5>, usm_kind<alloc::device>};
    properties OutP{conduit, buffer_location<5>, usm_kind<alloc::device>};

    // Test allocation in bytes
    {
      std::vector<annotated_ptr<void, decltype(OutP)>> APVec;
      TEST((malloc_annotated), N, q, InP);
      TEST((malloc_annotated), N, Dev, Ctx, InP);
      TEST((malloc_annotated<decltype(InP)>), N, q, InP);
      TEST((malloc_annotated<decltype(InP)>), N, Dev, Ctx, InP);
      TEST((malloc_annotated<decltype(InP), decltype(OutP)>), N, q, InP);
      TEST((malloc_annotated<decltype(InP), decltype(OutP)>), N, Dev, Ctx, InP);
    }

    // Test allocation in elements of T
    {
      std::vector<annotated_ptr<T, decltype(OutP)>> APVec;
      TEST((malloc_annotated<T>), N, q, InP);
      TEST((malloc_annotated<T>), N, Dev, Ctx, InP);
      TEST((malloc_annotated<T, decltype(InP)>), N, q, InP);
      TEST((malloc_annotated<T, decltype(InP)>), N, Dev, Ctx, InP);
      TEST((malloc_annotated<T, decltype(InP), decltype(OutP)>), N, q, InP);
      TEST((malloc_annotated<T, decltype(InP), decltype(OutP)>), N, Dev, Ctx, InP);
    }
  }

  // Test alloc functions with empty property list
  {
    // Test allocation in bytes
    {
      std::vector<annotated_ptr<void>> APVec;
      TEST((malloc_annotated), N, q, alloc::device);
      TEST((malloc_annotated), N, Dev, Ctx, alloc::device);
    }

    // Test allocation in elements of T
    {
      std::vector<annotated_ptr<T>> APVec;
      TEST((malloc_annotated<T>), N, q, alloc::host);
      TEST((malloc_annotated<T>), N, Dev, Ctx, alloc::shared);
    }
  }
}

int main() {
  testAlloc<double>();
  testAlloc<std::complex<double>>();
  return 0;
}

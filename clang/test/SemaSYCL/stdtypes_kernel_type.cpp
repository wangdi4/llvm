// RUN: %clang_cc1 -fsycl -fsycl-is-device -sycl-std=2020 -DCHECK_ERROR -verify %s

// INTEL_CUSTOMIZATION comments should be removed once patch is upstreamed to intel/llvm.

#include "Inputs/sycl.hpp"

namespace std {
typedef long unsigned int size_t;
typedef long int ptrdiff_t;
typedef decltype(nullptr) nullptr_t;
class T;
class U;
class Foo; // INTEL
} // namespace std

template <typename T>
struct Templated_kernel_name;

// INTEL_CUSTOMIZATION
template <typename T>
struct Templated_kernel_name2;
// end INTEL_CUSTOMIZATION

template <typename T, typename... Args> class TemplParamPack;

using namespace cl::sycl;
queue q;

int main() {
#ifdef CHECK_ERROR
  // INTEL_CUSTOMIZATION
  // expected-error@Inputs/sycl.hpp:220 6 {{kernel name cannot be or contain a type in the "std" namespace}}
  // END INTEL_CUSTOMIZATION
  q.submit([&](handler &h) {
    // INTEL_CUSTOMIZATION
    // expected-note@Inputs/sycl.hpp:220 {{Invalid kernel name is 'nullptr_t'}}
    // END INTEL_CUSTOMIZATION
    // expected-note@+1{{in instantiation of function template specialization}}
    h.single_task<std::nullptr_t>([=] {});
  });
  q.submit([&](handler &h) {
    // INTEL_CUSTOMIZATION
    // expected-note@Inputs/sycl.hpp:220{{Invalid kernel name is 'std::T'}}
    // END INTEL_CUSTOMIZATION
    // expected-note@+1{{in instantiation of function template specialization}}
    h.single_task<std::T>([=] {});
  });
  q.submit([&](handler &h) {
    // INTEL_CUSTOMIZATION
    // expected-note@Inputs/sycl.hpp:220{{Invalid kernel name is 'Templated_kernel_name<nullptr_t>'}}
    // END INTEL_CUSTOMIZATION
    // expected-note@+1{{in instantiation of function template specialization}}
    h.single_task<Templated_kernel_name<std::nullptr_t>>([=] {});
  });
  q.submit([&](handler &h) {
    // INTEL_CUSTOMIZATION
    // expected-note@Inputs/sycl.hpp:220{{Invalid kernel name is 'Templated_kernel_name<std::U>'}}
    // END INTEL_CUSTOMIZATION
    // expected-note@+1{{in instantiation of function template specialization}}
    h.single_task<Templated_kernel_name<std::U>>([=] {});
  });
  // INTEL_CUSTOMIZATION
  q.submit([&](handler &cgh) {
    // expected-note@Inputs/sycl.hpp:220{{Invalid kernel name is 'Templated_kernel_name2<Templated_kernel_name<std::Foo>>'}}
    // expected-note@+1{{in instantiation of function template specialization}}
    cgh.single_task<Templated_kernel_name2<Templated_kernel_name<std::Foo>>>([]() {});
  });
  // END INTEL_CUSTOMIZATION
  q.submit([&](handler &cgh) {
    // INTEL_CUSTOMIZATION
    // expected-note@Inputs/sycl.hpp:220{{Invalid kernel name is 'TemplParamPack<int, float, nullptr_t, double>'}}
    // END INTEL_CUSTOMIZATION
    // expected-note@+1{{in instantiation of function template specialization}}
    cgh.single_task<TemplParamPack<int, float, std::nullptr_t, double>>([]() {});
  });
#endif

  // Although in the std namespace, these resolve to builtins such as `int` that are allowed in kernel names
  q.submit([&](handler &h) {
    h.single_task<std::size_t>([=] {});
  });
  q.submit([&](handler &h) {
    h.single_task<std::ptrdiff_t>([=] {});
  });

  return 0;
}

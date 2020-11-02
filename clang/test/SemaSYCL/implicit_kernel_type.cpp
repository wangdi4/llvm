// RUN: %clang_cc1 -fsycl -fsycl-is-device -internal-isystem %S/Inputs -fsycl-int-header=%t.h -fsyntax-only -sycl-std=2020 -verify %s -Werror=sycl-strict -DERROR
// RUN: %clang_cc1 -fsycl -fsycl-is-device -internal-isystem %S/Inputs -fsycl-int-header=%t.h -fsyntax-only -sycl-std=2020 -verify %s  -Wsycl-strict -DWARN
// RUN: %clang_cc1 -fsycl -fsycl-is-device -internal-isystem %S/Inputs -fsycl-int-header=%t.h -fsycl-unnamed-lambda -fsyntax-only -sycl-std=2020 -verify %s  -Werror=sycl-strict

// INTEL_CUSTOMIZATION comments should be removed once patch is upstreamed to intel/llvm.

#include "sycl.hpp"

#ifdef __SYCL_UNNAMED_LAMBDA__
// expected-no-diagnostics
#endif

using namespace cl::sycl;

// user-defined function
void function() {
}

// user-defined struct
struct myWrapper {
};

// user-declared class
class myWrapper2;

int main() {
  queue q;

#if defined(WARN)
  // expected-error@Inputs/sycl.hpp:220 {{kernel needs to have a globally-visible name}}
  // INTEL_CUSTOMIZATION
  // expected-note@Inputs/sycl.hpp:220 {{Invalid kernel name is 'InvalidKernelName1'}}
  // expected-note@+11 {{InvalidKernelName1 declared here}}
  // expected-note@+12 {{in instantiation of function template specialization}}
  // end INTEL_CUSTOMIZATION
#elif defined(ERROR)
  // expected-error@Inputs/sycl.hpp:220 {{kernel needs to have a globally-visible name}}
  // INTEL_CUSTOMIZATION
  // expected-note@Inputs/sycl.hpp:220 {{Invalid kernel name is 'InvalidKernelName1'}}
  // end INTEL_CUSTOMIZATION
  // expected-note@+3 {{InvalidKernelName1 declared here}}
  // expected-note@+4 {{in instantiation of function template specialization}}
#endif
  class InvalidKernelName1 {};
  q.submit([&](handler &h) {
    h.single_task<InvalidKernelName1>([]() {});
  });

#if defined(WARN)
  // expected-warning@Inputs/sycl.hpp:220 {{SYCL 1.2.1 specification requires an explicit forward declaration for a kernel type name; your program may not be portable}}
#elif defined(ERROR)
  // expected-error@Inputs/sycl.hpp:220 {{SYCL 1.2.1 specification requires an explicit forward declaration for a kernel type name; your program may not be portable}}
#endif

  q.submit([&](handler &h) {
#ifndef __SYCL_UNNAMED_LAMBDA__
  // expected-note@+3 {{fake_kernel declared here}}
  // expected-note@+2 {{in instantiation of function template specialization}}
#endif
    h.single_task<class fake_kernel>([]() { function(); });
  });

#if defined(WARN)
  // expected-warning@Inputs/sycl.hpp:220 {{SYCL 1.2.1 specification requires an explicit forward declaration for a kernel type name; your program may not be portable}}
#elif defined(ERROR)
  // expected-error@Inputs/sycl.hpp:220 {{SYCL 1.2.1 specification requires an explicit forward declaration for a kernel type name; your program may not be portable}}
#endif

  q.submit([&](handler &h) {
#ifndef __SYCL_UNNAMED_LAMBDA__
  // expected-note@+3 {{fake_kernel2 declared here}}
  // expected-note@+2 {{in instantiation of function template specialization}}
#endif
    h.single_task<class fake_kernel2>([]() {
      auto l = [](auto f) { f(); };
    });
  });

  q.submit([&](handler &h) {
    h.single_task<class myWrapper>([]() { function(); });
  });

  q.submit([&](handler &h) {
    h.single_task<class myWrapper2>([]() { function(); });
  });
  return 0;
}

// INTEL_CUSTOMIZATION comments should be removed once patch is upstreamed to intel/llvm.

// RUN: %clang_cc1 -fsycl -fsycl-is-device -fsyntax-only -Wno-sycl-2017-compat -verify %s
// RUN: %clang_cc1 -fsycl -fsycl-is-device -fsycl-unnamed-lambda -fsyntax-only -Wno-sycl-2017-compat -verify %s
#include "Inputs/sycl.hpp"

#ifdef __SYCL_UNNAMED_LAMBDA__
// expected-no-diagnostics
#endif

namespace namespace1 {
template <typename T>
class KernelName;
}

namespace std {
typedef struct {
} max_align_t;
} // namespace std

// INTEL_CUSTOMIZATION
template <typename T>
struct Templated_kernel_name;

template <typename T>
struct Templated_kernel_name2;
// end INTEL_CUSTOMIZATION

struct MyWrapper {
private:
  class InvalidKernelName0 {};
  class InvalidKernelName3 {};
  class InvalidKernelName4 {};
  class InvalidKernelName5 {};

public:
  void test() {
    cl::sycl::queue q;

#ifndef __SYCL_UNNAMED_LAMBDA__
    // expected-error@Inputs/sycl.hpp:220 {{kernel needs to have a globally-visible name}}
    // INTEL_CUSTOMIZATION
    // expected-note@Inputs/sycl.hpp:220 {{Invalid kernel name is 'InvalidKernelName1'}}
    // end INTEL_CUSTOMIZATION
    // expected-note@+3 {{InvalidKernelName1 declared here}}
    // expected-note@+4{{in instantiation of function template specialization}}
#endif
    class InvalidKernelName1 {};
    q.submit([&](cl::sycl::handler &h) {
      h.single_task<InvalidKernelName1>([] {});
    });

#ifndef __SYCL_UNNAMED_LAMBDA__
    // expected-error@Inputs/sycl.hpp:220 {{kernel needs to have a globally-visible name}}
    // INTEL_CUSTOMIZATION
    // expected-note@Inputs/sycl.hpp:220 {{Invalid kernel name is 'namespace1::KernelName<InvalidKernelName2>'}}
    // end INTEL_CUSTOMIZATION
    // expected-note@+3 {{InvalidKernelName2 declared here}}
    // expected-note@+4{{in instantiation of function template specialization}}
#endif
    class InvalidKernelName2 {};
    q.submit([&](cl::sycl::handler &h) {
      h.single_task<namespace1::KernelName<InvalidKernelName2>>([] {});
    });

#ifndef __SYCL_UNNAMED_LAMBDA__
    // expected-error@Inputs/sycl.hpp:220 {{kernel needs to have a globally-visible name}}
    // INTEL_CUSTOMIZATION
    // expected-note@Inputs/sycl.hpp:220 {{Invalid kernel name is 'MyWrapper::InvalidKernelName0'}}
    // expected-note@31 {{InvalidKernelName0 declared here}}
    // end INTEL_CUSTOMIZATION
    // expected-note@+3{{in instantiation of function template specialization}}
#endif
    q.submit([&](cl::sycl::handler &h) {
      h.single_task<InvalidKernelName0>([] {});
    });

#ifndef __SYCL_UNNAMED_LAMBDA__
    // expected-error@Inputs/sycl.hpp:220 {{kernel needs to have a globally-visible name}}
    // INTEL_CUSTOMIZATION
    // expected-note@Inputs/sycl.hpp:220 {{Invalid kernel name is 'namespace1::KernelName<MyWrapper::InvalidKernelName3>'}}
    // expected-note@32 {{InvalidKernelName3 declared here}}
    // end INTEL_CUSTOMIZATION
    // expected-note@+3{{in instantiation of function template specialization}}
#endif
    q.submit([&](cl::sycl::handler &h) {
      h.single_task<namespace1::KernelName<InvalidKernelName3>>([] {});
    });

    using ValidAlias = MyWrapper;
    q.submit([&](cl::sycl::handler &h) {
      h.single_task<ValidAlias>([] {});
    });

#ifndef __SYCL_UNNAMED_LAMBDA__
    // INTEL_CUSTOMIZATION
    // expected-error@Inputs/sycl.hpp:220 {{kernel name cannot be or contain a type in the "std" namespace}}
    // expected-note@Inputs/sycl.hpp:220 {{Invalid kernel name is 'std::max_align_t'}}
    // expected-note@+4{{in instantiation of function template specialization}}
    // end INTEL_CUSTOMIZATION
#endif
    q.submit([&](cl::sycl::handler &h) {
      h.single_task<std::max_align_t>([] {});
    });

    using InvalidAlias = InvalidKernelName4;
#ifndef __SYCL_UNNAMED_LAMBDA__
    // expected-error@Inputs/sycl.hpp:220 {{kernel needs to have a globally-visible name}}
    // INTEL_CUSTOMIZATION
    // expected-note@Inputs/sycl.hpp:220 {{Invalid kernel name is 'MyWrapper::InvalidKernelName4'}}
    // expected-note@33 {{InvalidKernelName4 declared here}}
    // end INTEL_CUSTOMIZATION
    // expected-note@+3{{in instantiation of function template specialization}}
#endif
    q.submit([&](cl::sycl::handler &h) {
      h.single_task<InvalidAlias>([] {});
    });

    using InvalidAlias1 = InvalidKernelName5;
#ifndef __SYCL_UNNAMED_LAMBDA__
    // expected-error@Inputs/sycl.hpp:220 {{kernel needs to have a globally-visible name}}
    // INTEL_CUSTOMIZATION
    // expected-note@Inputs/sycl.hpp:220 {{Invalid kernel name is 'namespace1::KernelName<MyWrapper::InvalidKernelName5>'}}
    // expected-note@34 {{InvalidKernelName5 declared here}}
    // end INTEL_CUSTOMIZATION
    // expected-note@+3{{in instantiation of function template specialization}}
#endif
    q.submit([&](cl::sycl::handler &h) {
      h.single_task<namespace1::KernelName<InvalidAlias1>>([] {});
    });

    // INTEL_CUSTOMIZATION
#ifndef __SYCL_UNNAMED_LAMBDA__
    // expected-error@Inputs/sycl.hpp:220 {{kernel needs to have a globally-visible name}}
    // expected-note@Inputs/sycl.hpp:220 {{Invalid kernel name is 'Templated_kernel_name2<Templated_kernel_name<InvalidKernelName1>>'}}
    // expected-note@48 {{InvalidKernelName1 declared here}}
    // expected-note@+3{{in instantiation of function template specialization}}
#endif
    q.submit([&](cl::sycl::handler &h) {
      h.single_task<Templated_kernel_name2<Templated_kernel_name<InvalidKernelName1>>>([] {});
    });
    // end INTEL_CUSTOMIZATION
  }
};

int main() {
  cl::sycl::queue q;
#ifndef __SYCL_UNNAMED_LAMBDA__
  // expected-error@Inputs/sycl.hpp:220 {{kernel name is missing}}
  // INTEL_CUSTOMIZATION
  // expected-note-re@Inputs/sycl.hpp:220 {{Invalid kernel name is '(lambda at {{.*}}unnamed-kernel.cpp{{.*}}'}}
  // end INTEL_CUSTOMIZATION
  // expected-note@+2{{in instantiation of function template specialization}}
#endif
  q.submit([&](cl::sycl::handler &h) { h.single_task([] {}); });

  return 0;
}

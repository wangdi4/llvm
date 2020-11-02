// RUN: %clang_cc1 -fsycl -fsycl-is-device -fsycl-int-header=%t.h -DCHECK_ERROR -verify %s
// RUN: %clang_cc1 -fsycl -fsycl-is-device -triple spir64-unknown-unknown-sycldevice -fsycl-int-header=%t.h %s
// RUN: FileCheck -input-file=%t.h %s
//
// INTEL_CUSTOMIZATION comments should be removed once patch is upstreamed to intel/llvm.
//
// CHECK: #include <CL/sycl/detail/defines_elementary.hpp>
// CHECK-NEXT: #include <CL/sycl/detail/kernel_desc.hpp>
//
// CHECK: static constexpr
// CHECK-NEXT: const char* const kernel_names[] = {
// CHECK-NEXT:   "_ZTSm",
// CHECK-NEXT:   "_ZTSl"
// CHECK-NEXT: };
//
// CHECK: static constexpr
// CHECK-NEXT: const kernel_param_desc_t kernel_signatures[] = {
// CHECK-NEXT:   //--- _ZTSm
// CHECK-EMPTY:
// CHECK-NEXT:   //--- _ZTSl
// CHECK-EMPTY:
// CHECK-NEXT: };

// CHECK: template <> struct KernelInfo<unsigned long> {
// CHECK: template <> struct KernelInfo<long> {

void usage() {
}

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

template <typename name, typename Func>
__attribute__((sycl_kernel)) void kernel_single_task(Func kernelFunc) {
  kernelFunc();
}

int main() {
#ifdef CHECK_ERROR
  // INTEL_CUSTOMIZATION
  // expected-error@+2 {{kernel name cannot be or contain a type in the "std" namespace}}
  // expected-note@+1 {{Invalid kernel name is 'nullptr_t'}}
  kernel_single_task<std::nullptr_t>([=]() {});
  // expected-error@+2 {{kernel name cannot be or contain a type in the "std" namespace}}
  // expected-note@+1 {{Invalid kernel name is 'std::T'}}
  kernel_single_task<std::T>([=]() {});
  // expected-error@+2 {{kernel name cannot be or contain a type in the "std" namespace}}
  // expected-note@+1 {{Invalid kernel name is 'Templated_kernel_name<nullptr_t>'}}
  kernel_single_task<Templated_kernel_name<std::nullptr_t>>([=]() {});
  // expected-error@+2 {{kernel name cannot be or contain a type in the "std" namespace}}
  // expected-note@+1 {{Invalid kernel name is 'Templated_kernel_name<std::U>'}}
  kernel_single_task<Templated_kernel_name<std::U>>([=]() {});
  // expected-error@+2 {{kernel name cannot be or contain a type in the "std" namespace}}
  // expected-note@+1 {{Invalid kernel name is 'Templated_kernel_name2<Templated_kernel_name<std::Foo>>'}}
  kernel_single_task<Templated_kernel_name2<Templated_kernel_name<std::Foo>>>([=]() {});
  // end INTEL_CUSTOMIZATION
#endif

  // Although in the std namespace, these resolve to builtins such as `int` that are allowed in kernel names
  kernel_single_task<std::size_t>([=]() {});
  kernel_single_task<std::ptrdiff_t>([=]() {});

  return 0;
}

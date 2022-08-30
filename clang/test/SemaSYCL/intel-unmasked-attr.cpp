// RUN: %clang_cc1 -fsycl-is-device -fsycl-allow-func-ptr -DINTEL_CUSTOMIZATION -sycl-std=2020 -internal-isystem %S/Inputs -fsyntax-only -verify %s
// RUN: %clang_cc1 -fsycl-is-host -fsyntax-only -DSYCL_HOST -verify %s
// RUN: not %clang_cc1 -fsycl-is-device -fsycl-allow-func-ptr -ast-dump %s | FileCheck %s
// RUN: %clang_cc1 -fsyntax-only -verify %s

#ifdef __SYCL_DEVICE_ONLY__

#include "sycl.hpp"

using namespace sycl;
queue q;

[[intel::unmasked]] // expected-warning {{'unmasked' attribute only applies to functions}}
int N;

[[intel::unmasked(3)]] // expected-error {{'unmasked' attribute takes no arguments}}
void
bar() {}

[[intel::unmasked]] // expected-error {{'unmasked' attribute cannot be applied to a static function or function in an anonymous namespace}}
static void
func1(int) {}

namespace {
[[intel::unmasked]] // expected-error {{'unmasked' attribute cannot be applied to a static function or function in an anonymous namespace}}
void
func2(int) {}
} // namespace

// expected-warning@+1 {{function with 'unmasked' attribute must have at least one parameter}}
[[intel::unmasked]] void bar1() {}

[[intel::unmasked]] void bar2(int) {}

class A {
public:
  // expected-warning@+1 {{function with 'unmasked' attribute must have at least one parameter}}
  [[intel::unmasked]] int func() {}

  [[intel::unmasked]] int func1(int) { return 1; }

  [[intel::unmasked]] int operator()(int) { return 1; }
};

class KernelName;

int main() {
  q.submit([&](handler &h) {
    h.single_task<class KernelName>(
        [=]() {    // expected-note 2{{called by 'operator()'}}
          bar2(1); // expected-warning {{function with 'unmasked' attribute can be called only by 'non_uniform_sub_group::invoke_unmasked()'}}
          A Obj;
          Obj.func();
          Obj.func1(1); // expected-warning {{function with 'unmasked' attribute can be called only by 'non_uniform_sub_group::invoke_unmasked()'}}

          ext::intel::non_uniform_sub_group G;
          G.invoke_unmasked(Obj);
          G.invoke_unmasked(bar2);

        });
  });
  return 0;
}

#elif defined(SYCL_HOST)
[[intel::unmasked]] int func1(int) { return 1; }
// expected-no-diagnostics
#else
[[intel::unmasked]] // expected-warning {{'unmasked' attribute ignored}}
void
baz() {}
#endif // __SYCL_DEVICE_ONLY__

// CHECK: FunctionDecl {{.*}} bar2
// CHECK: SYCLUnmaskedAttr

// CHECK: CXXMethodDecl {{.*}} func1
// CHECK: SYCLUnmaskedAttr
//
// CHECK: CXXMethodDecl {{.*}} operator()
// CHECK: SYCLUnmaskedAttr

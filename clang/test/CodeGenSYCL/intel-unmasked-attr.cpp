// RUN: %clang_cc1 -fsycl-is-device -fsycl-allow-func-ptr -DINTEL_CUSTOMIZATION -internal-isystem %S/Inputs -disable-llvm-passes -triple spir64-unknown-unknown-sycldevice -emit-llvm -o - %s | FileCheck %s

#include "sycl.hpp"

using namespace sycl;
queue q;

[[intel::device_indirectly_callable]] int bar10(int a) { return a + 20; }
[[intel::device_indirectly_callable]] int bar20(int a) { return a + 20; }

[[intel::unmasked]] void bar2(int) {bar10(1);}
// CHECK-DAG: define dso_local spir_func void @_Z4bar2i(i32 {{[^,]*}}%0) #[[ATTRS_UNMASKED:[0-9]+]]
// CHECK-DAG: define dso_local spir_func {{[^,]*}}i32 @_Z5bar10i(i32 {{[^,]*}}%a) #[[ATTRS_NOT_UNMASKED:[0-9]+]]

class A {
public:
  // CHECK-DAG: define linkonce_odr spir_func void @_ZN1AclEi(%class.A addrspace(4)* {{[^,]*}}%this, i32 {{[^,]*}}%i) #[[ATTRS_UNMASKED]]
  // CHECK-DAG: define dso_local spir_func {{[^,]*}}i32 @_Z5bar20i(i32 {{[^,]*}}%a) #[[ATTRS_NOT_UNMASKED]]
  [[intel::unmasked]] void operator()(int i) { bar20(i); }
};

class KernelName;

int main() {
  q.submit([&](handler &h) {

    h.single_task<class KernelName>(
        [=]() {
          A Obj;
          ext::intel::non_uniform_sub_group G;
          G.invoke_unmasked(Obj);
          G.invoke_unmasked(bar10);
        });
  });
  return 0;
}

// CHECK: attributes #[[ATTRS_UNMASKED]] = { {{.*}} noinline {{.*}} "referenced-indirectly" {{.*}} "unmasked"
// CHECK-NOT: attributes #[[ATTRS_NOT_UNMASKED]] = { {{.*}} noinline {{.*}} "unmasked"

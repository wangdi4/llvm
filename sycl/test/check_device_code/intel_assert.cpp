// RUN: %clangxx -fsycl-device-only -Xclang -emit-llvm -Xclang -no-enable-noundef-analysis %s -o - | FileCheck %s
// RUN: %clangxx -c %fsycl-host-only -Xclang -emit-llvm -Xclang -no-enable-noundef-analysis %s -Xclang -opaque-pointers -o - | FileCheck %s -check-prefix CHECK-HOST
// REQUIRES: linux

#include <CL/sycl.hpp>

int main() {
  sycl::queue Q;

  Q.single_task([] {
    // CHECK-NOT: noreturn
    // CHECK-NOT: unreachable
    assert(false);
  });
  Q.wait();

  // CHECK-HOST: call void @__assert_fail
  // CHECK-HOST-NEXT: unreachable
  // CHECK-HOST: declare dso_local void @__assert_fail(ptr, ptr, i32, ptr) #[[ATTR:[0-9]+]]
  // CHECK-HOST: attributes #[[ATTR]] = {
  // CHECK-HOST-SAME: noreturn
  assert(false);

  return 0;
}

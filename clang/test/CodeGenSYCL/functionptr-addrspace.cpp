// RUN: %clang_cc1 -fsycl-is-device -emit-llvm -triple spir64 -verify -emit-llvm %s -o - | FileCheck %s

// expected-no-diagnostics

template <typename Name, typename Func>
__attribute__((sycl_kernel)) void kernel_single_task(const Func &kernelFunc) {
  kernelFunc();
}

<<<<<<< HEAD
// INTEL: added nocapture
// CHECK: define dso_local spir_func{{.*}}invoke_function{{.*}}(i32 ()* nocapture %fptr, i32 addrspace(4)* nocapture %ptr)
// INTEL
=======
// CHECK: define dso_local spir_func{{.*}}invoke_function{{.*}}(i32 ()* nocapture noundef %fptr, i32 addrspace(4)* nocapture noundef %ptr)
>>>>>>> 17b69935e3c78f89a6dc1ab54a70fa8f082ccc62
void invoke_function(int (*fptr)(), int *ptr) {}

int f() { return 0; }

int main() {
  kernel_single_task<class fake_kernel>([=]() {
    int (*p)() = f;
    int (&r)() = *p;
    int a = 10;
    invoke_function(p, &a);
    invoke_function(r, &a);
    invoke_function(f, &a);
  });
  return 0;
}

// RUN: %clang_cc1 -fsycl-is-device %s -no-opaque-pointers -emit-llvm -sycl-std=2017 -triple spir64-unknown-unknown -o - | FileCheck %s
// INTEL_CUSTOMIZATION
// INTEL_FEATURE_CSA
// INTEL - Disable test for CSA compiler as it is flaky.
// UNSUPPORTED: csa-registered-target
// end INTEL_FEATURE_CSA
// end INTEL_CUSTOMIZATION

template <typename name, typename Func>
__attribute__((sycl_kernel)) void kernel(const Func &kernelFunc) {
  kernelFunc();
}

int main() {
  int *a;
  int *b;
  int *c;
  kernel<class kernel_restrict>(
      [ a, b, c ]() [[intel::kernel_args_restrict]] { c[0] = a[0] + b[0]; });
  // INTEL CHECK: define {{.*}}spir_kernel {{.*}}kernel_restrict(i32 addrspace(1)* noalias nocapture noundef readonly align 4 %{{.*}}, i32 addrspace(1)* noalias nocapture noundef readonly align 4 %{{.*}}, i32 addrspace(1)* noalias nocapture noundef writeonly align 4 %{{.*}})

  int *d;
  int *e;
  int *f;

  kernel<class kernel_norestrict>(
      [d, e, f]() { f[0] = d[0] + e[0]; });
  // INTEL CHECK: define {{.*}}spir_kernel {{.*}}kernel_norestrict(i32 addrspace(1)* nocapture noundef readonly align 4 %{{.*}}, i32 addrspace(1)* nocapture noundef readonly align 4 %{{.*}}, i32 addrspace(1)* nocapture noundef writeonly align 4 %{{.*}})

  int g = 42;
  kernel<class kernel_restrict_other_types>(
      [ a, b, c, g ]() [[intel::kernel_args_restrict]] { c[0] = a[0] + b[0] + g; });
  // INTEL CHECK: define {{.*}}spir_kernel {{.*}}kernel_restrict_other_types(i32 addrspace(1)* noalias nocapture noundef readonly align 4 %{{.*}}, i32 addrspace(1)* noalias nocapture noundef readonly align 4 %{{.*}}, i32 addrspace(1)* noalias nocapture noundef writeonly align 4 %{{.*}}, i32 noundef %{{.*}})
}

// RUN: %clang_cc1 -fsycl-is-device %s -emit-llvm -sycl-std=2017 -triple spir64-unknown-unknown -o - | FileCheck %s
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
<<<<<<< HEAD
  // INTEL CHECK: define {{.*}}spir_kernel {{.*}}kernel_restrict(i32 addrspace(1)* noalias nocapture noundef readonly %{{.*}}, i32 addrspace(1)* noalias nocapture noundef readonly %{{.*}}, i32 addrspace(1)* noalias nocapture noundef writeonly %{{.*}})
=======
  // CHECK: define {{.*}}spir_kernel {{.*}}kernel_restrict(i32 addrspace(1)* noalias noundef %{{.*}}, i32 addrspace(1)* noalias noundef %{{.*}}, i32 addrspace(1)* noalias noundef %{{.*}})
>>>>>>> 0a1e6d9cafbcbe81d4bd7972cac5d8790124de34

  int *d;
  int *e;
  int *f;

  kernel<class kernel_norestrict>(
      [d, e, f]() { f[0] = d[0] + e[0]; });
<<<<<<< HEAD
  // INTEL CHECK: define {{.*}}spir_kernel {{.*}}kernel_norestrict(i32 addrspace(1)* nocapture noundef readonly %{{.*}}, i32 addrspace(1)* nocapture noundef readonly %{{.*}}, i32 addrspace(1)* nocapture noundef writeonly %{{.*}})
=======
  // CHECK: define {{.*}}spir_kernel {{.*}}kernel_norestrict(i32 addrspace(1)* noundef %{{.*}}, i32 addrspace(1)* noundef %{{.*}}, i32 addrspace(1)* noundef %{{.*}})
>>>>>>> 0a1e6d9cafbcbe81d4bd7972cac5d8790124de34

  int g = 42;
  kernel<class kernel_restrict_other_types>(
      [ a, b, c, g ]() [[intel::kernel_args_restrict]] { c[0] = a[0] + b[0] + g; });
<<<<<<< HEAD
  // INTEL CHECK: define {{.*}}spir_kernel {{.*}}kernel_restrict_other_types(i32 addrspace(1)* noalias nocapture noundef readonly %{{.*}}, i32 addrspace(1)* noalias nocapture noundef readonly %{{.*}}, i32 addrspace(1)* noalias nocapture noundef writeonly %{{.*}}, i32 noundef %{{.*}})
=======
  // CHECK: define {{.*}}spir_kernel {{.*}}kernel_restrict_other_types(i32 addrspace(1)* noalias noundef %{{.*}}, i32 addrspace(1)* noalias noundef %{{.*}}, i32 addrspace(1)* noalias noundef %{{.*}}, i32 noundef %{{.*}})
>>>>>>> 0a1e6d9cafbcbe81d4bd7972cac5d8790124de34
}

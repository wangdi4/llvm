// RUN: %clang_cc1 %s -o - -O0 -emit-llvm \
// RUN:  -fintel-compatibility \
// RUN:  -triple spir64-unknown-unknown   \
// RUN:  -aux-triple x86_64-unknown-linux-gnu \
// RUN:  -fsycl-is-device -finclude-default-header\
// RUN:  -debug-info-kind=limited -gno-column-info \
// RUN:  \
// RUN:  | FileCheck %s

// This test is cloned from the test debug-info-indirect-param-var.cpp, and uses the -fintel-compatibility flag in addition.
// This is to minimize the effect of INTEL_CUSTOMIZATION changes on community tests.

#define KERNEL __attribute__((sycl_kernel))

template <typename KernelName, typename KernelType>
KERNEL void parallel_for(const KernelType &KernelFunc) {
  KernelFunc();
}

struct S {
  int i, j;
};

void my_kernel(S s) {
  s.i = 10;
}

int my_host() {
  S a;
  parallel_for<class K>([=]() { my_kernel(a); });
  return 0;
}

// CHECK: define {{.*}} spir_func void @_Z9my_kernel1S(
// CHECK-SAME: !dbg [[MY_KERNEL:![0-9]+]]
// CHECK-SAME: {
// CHECK: %[[AS_CAST:.*]] = addrspacecast ptr {{.*}} to ptr addrspace(4)
// CHECK:   call void @llvm.dbg.declare(
// CHECK-SAME: metadata ptr addrspace(4) %[[AS_CAST]],
// CHECK-SAME: metadata [[MY_PARAM:![0-9]+]],
// INTEL_CUSTOMIZATION
// CHECK-SAME: metadata !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef, DW_OP_deref)
// end INTEL_CUSTOMIZATION
// CHECK-SAME: )
// CHECK: }

// CHECK:      [[MY_KERNEL]] = distinct !DISubprogram(
// CHECK-SAME: name: "my_kernel"
// CHECK-SAME: )
// CHECK:      [[MY_PARAM]] = !DILocalVariable(
// CHECK-SAME: name: "s"
// CHECK-SAME: arg: 1
// CHECK-SAME: scope:  [[MY_KERNEL]]
// CHECK-SAME: )

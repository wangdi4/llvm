// CQ#366562

// RUN: %clang_cc1 -fintel-compatibility -verify %s -DZERO_OK
// RUN: %clang_cc1 -fintel-compatibility -fintel-compatibility-disable=UnrollZero -verify %s
// RUN: %clang_cc1 -fintel-compatibility -fintel-compatibility-enable=UnrollZero -DZERO_OK -emit-llvm %s -o - | FileCheck --check-prefix=CHECK-ZERO_OK %s

int main(void) {
  int i = 0, s = 0;

  // Unroll disabled.

#ifdef ZERO_OK
#pragma unroll(0) // expected-no-diagnostics
#else
#pragma unroll(0) // expected-error{{invalid value '0'; must be positive}}
#endif
  for (i = 0; i < 10; ++i)
    s = s + i;
    // CHECK-ZERO_OK-DAG: !llvm.loop [[LOOP_1:!.+]]

#pragma nounroll
  for (i = 0; i < 10; ++i)
    s = s + i;
    // CHECK: !llvm.loop [[LOOP_2:!.+]]

#pragma unroll
  for (i = 0; i < 10; ++i)
    s = s + i;
  // CHECK: !llvm.loop [[LOOP_3:!.+]]

  return s;
  // CHECK: [[LOOP_1]] = distinct !{[[LOOP_1]], [[ATTR_1:!.+]]}
  // CHECK: [[ATTR_1]] = !{!"llvm.loop.unroll.disable"}
  // CHECK: [[LOOP_2]] = distinct !{[[LOOP_2]], [[ATTR_1]]}
  // CHECK: [[LOOP_3]] = distinct !{[[LOOP_3]], [[ATTR_2:!.+]]}
  // CHECK: [[ATTR_2]] = !{!"llvm.loop.unroll.enable"}
}

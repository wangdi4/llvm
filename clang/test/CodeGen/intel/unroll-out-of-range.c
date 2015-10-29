// CQ#366562
// RUN: %clang_cc1 -fintel-compatibility -emit-llvm %s -o - | FileCheck %s
// REQUIRES: llvm-backend

int main(void) {
  int i = 0, s = 0;

  // Unroll disabled.
  #pragma unroll(0)
  for (i = 0; i < 10; ++i)
    s = s + i;
  // CHECK: !llvm.loop [[LOOP_1:!.+]]

  #pragma nounroll
  for (i = 0; i < 10; ++i)
    s = s + i;
  // CHECK: !llvm.loop [[LOOP_2:!.+]]

  // Unroll enabled.
  #pragma unroll(-2)
  for (i = 0; i < 10; ++i)
    s = s + i;
  // CHECK: !llvm.loop [[LOOP_3:!.+]]
  
  #pragma unroll(1844674407370955161)
  for (i = 0; i < 10; ++i)
    s = s + i;
  // CHECK: !llvm.loop [[LOOP_4:!.+]]

  #pragma unroll
  for (i = 0; i < 10; ++i)
    s = s + i;
  // CHECK: !llvm.loop [[LOOP_5:!.+]]

  return s;
  // CHECK: [[LOOP_1]] = distinct !{[[LOOP_1]], [[ATTR_1:!.+]]}
  // CHECK: [[ATTR_1]] = !{!"llvm.loop.unroll.disable"}
  // CHECK: [[LOOP_2]] = distinct !{[[LOOP_2]], [[ATTR_1]]}
  // CHECK: [[LOOP_3]] = distinct !{[[LOOP_3]], [[ATTR_2:!.+]]}
  // CHECK: [[ATTR_2]] = !{!"llvm.loop.unroll.enable"}
  // CHECK: [[LOOP_4]] = distinct !{[[LOOP_4]], [[ATTR_2]]}
  // CHECK: [[LOOP_5]] = distinct !{[[LOOP_5]], [[ATTR_2]]}
}

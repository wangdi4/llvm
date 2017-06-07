// RUN: %clang_cc1 -triple=x86_64-unknown-unknown -fintel-compatibility -std=c++11 %s -emit-llvm -o - | FileCheck %s

typedef unsigned int uint;
void foo()
{
  #pragma unroll
  for (uint level = 0; level < 6; level++)
  {
    #pragma unroll(4)
    for (uint j = 0; j < 63; j+=2)
    {
    }
    // CHECK: !llvm.loop [[LOOP_INNER:!.+]]
  }
  // CHECK: !llvm.loop [[LOOP_OUTER:!.+]]
}
// CHECK: [[LOOP_INNER]] = distinct !{[[LOOP_INNER]], [[LOOP_IATTR:!.+]]}
// CHECK: [[LOOP_IATTR]] = !{!"llvm.loop.unroll.count", i32 4}
// CHECK: [[LOOP_OUTER]] = distinct !{[[LOOP_OUTER]], [[LOOP_OATTR:!.+]]}
// CHECK: [[LOOP_OATTR]] = !{!"llvm.loop.unroll.enable"}

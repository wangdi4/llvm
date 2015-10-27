// RUN: %clang_cc1 -triple=x86_64-unknown-unknown -fintel-compatibility -std=c++11 %s -emit-llvm -o - | FileCheck %s
// REQUIRES: llvm-backend

int main() {
  int i = 0, x  = 0;
  // Usual case - #pragma unroll precedes a loop.
  #pragma unroll (4)
  for (i = 0; i < 800; ++i)
    ++x;
  // CHECK: !llvm.loop [[LOOP_1:!.+]]

  // #pragma unroll precedes a non-loop statement - applied to the closest loop.
  #pragma unroll (8)
  x = 0;
  for (i = 0; i < 8000; ++i)
    --x;
  // CHECK: !llvm.loop [[LOOP_2:!.+]]

  // Several #pragma unroll statements - the last setting must be applied.
  #pragma unroll (2)
  x = 100;
  #pragma unroll (16)
  if (x > 0) x = 0;
  i = 0;
  while (i < 8000) { ++x; ++i; }
  // CHECK: !llvm.loop [[LOOP_3:!.+]]

  // Check that pending unroll attribute is added to other loop hint attributes.
  #pragma unroll (4)
  if (x > 0) x = 0;
  int arr[400];
  #pragma clang loop interleave_count(4)
  for (int j : arr)
    ++x;
  // CHECK: !llvm.loop [[LOOP_4:!.+]]

  // Check that pending #pragma unroll will be applied to the closest loop.
  #pragma unroll (8)
  x = 0;
  struct S {
    void f() {
      int ii = 0, xx = 0;
      // #pragma unroll must be applied here (attributes for LOOP_6).
      for (ii = 0; ii < 800; ++ii)
        ++xx;
    }
  };
  S s;
  s.f();
  // This loop must have only inteterleave_count attribute (LOOP_5).
  #pragma clang loop interleave_count(4)
  for (i = 0; i < 800; ++i)
    ++x;
  // CHECK: !llvm.loop [[LOOP_5:!.+]]
}

// CHECK: _ZZ4mainEN1S1fEv
// CHECK: !llvm.loop [[LOOP_6:!.+]]

int g() {
  // Check that local #pragma unroll has effect on exit from its scope.
  int i = 0, x;
  #pragma nounroll
  x = 0;
  struct S {
    int f(int N) {
      #pragma unroll (16)
      return N;
    }
  };
  for (i = 0; i < 400; ++i)
    ++x;

  // If multiple unroll attributes are given, the last must take effect.
  #pragma unroll (2)
  x = 0;
  #pragma nounroll
  #pragma unroll (4)
  for (i = 0; i < 400; ++i)
    ++x;

  return x;
}

// CHECK: _Z1gv
// CHECK: !llvm.loop [[LOOP_7:!.+]]
// CHECK: !llvm.loop [[LOOP_8:!.+]]


// CHECK: [[LOOP_1]] = distinct !{[[LOOP_1]], [[ATTR_1:!.+]]}
// CHECK: [[ATTR_1]] = !{!"llvm.loop.unroll.count", i32 4}
// CHECK: [[LOOP_2]] = distinct !{[[LOOP_2]], [[ATTR_2:!.+]]}
// CHECK: [[ATTR_2]] = !{!"llvm.loop.unroll.count", i32 8}
// CHECK: [[LOOP_3]] = distinct !{[[LOOP_3]], [[ATTR_3:!.+]]}
// CHECK: [[ATTR_3]] = !{!"llvm.loop.unroll.count", i32 16}
// CHECK: [[LOOP_4]] = distinct !{[[LOOP_4]], [[ATTR_4:!.+]], [[ATTR_1]]}
// CHECK: [[ATTR_4]] = !{!"llvm.loop.interleave.count", i32 4}
// CHECK: [[LOOP_5]] = distinct !{[[LOOP_5]], [[ATTR_4]]}
// CHECK: [[LOOP_6]] = distinct !{[[LOOP_6]], [[ATTR_2]]}
// CHECK: [[LOOP_7]] = distinct !{[[LOOP_7]], [[ATTR_3]]}
// CHECK: [[LOOP_8]] = distinct !{[[LOOP_8]], [[ATTR_1]]}

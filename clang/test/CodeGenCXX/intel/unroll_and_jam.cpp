// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -o - %s | FileCheck %s
// expected-no-diagnostics

void bar();

void foo() {
  int i, a[10], n = 10, j;

  i = 0;
  #pragma nounroll_and_jam
  for (i = 0; i < n; ++i) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_1:.*]]
    a[i] += 5;
  }
  #pragma nounroll_and_jam
  for (i = 0; i < n; ++i) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_2:.*]]
    for (j = 0; j < n; ++j) {
      // CHECK-NOT: br label %{{.*}}, !llvm.loop !{{.*}}
      a[i] += a[j];
    }
    bar();
  }
  for (i = 0; i < n; ++i) {
    // CHECK-NOT: br label %{{.*}}, !llvm.loop !{{.*}}
    #pragma nounroll_and_jam
    for (j = 0; j < n; ++j) {
      // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_3:.*]]
      a[i] += a[j];
    }
    bar();
  }
  #pragma unroll_and_jam
  for (i = 0; i < n; ++i) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_4:.*]]
    a[i] += 5;
  }
  #pragma unroll_and_jam (5)
  for (i = 0; i < n; ++i) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_5:.*]]
    a[i] += a[j];
  }
}

// CHECK: ![[LOOP_1]] = distinct !{![[LOOP_1]], ![[NOUNROLLANDJAM:.*]]}
// CHECK: ![[NOUNROLLANDJAM]] = !{!"llvm.loop.unroll_and_jam.disable"}
// CHECK: ![[LOOP_2]] = distinct !{![[LOOP_2]], ![[NOUNROLLANDJAM]]}
// CHECK: ![[LOOP_3]] = distinct !{![[LOOP_3]], ![[NOUNROLLANDJAM]]}
// CHECK: ![[LOOP_4]] = distinct !{![[LOOP_4]], ![[UNROLLANDJAM:.*]]}
// CHECK: ![[UNROLLANDJAM]] = !{!"llvm.loop.unroll_and_jam.enable"}
// CHECK: ![[LOOP_5]] = distinct !{![[LOOP_5]], ![[UNROLLANDJAMCOUNT:.*]]}
// CHECK: ![[UNROLLANDJAMCOUNT]] = !{!"llvm.loop.unroll_and_jam.count", i32 5}

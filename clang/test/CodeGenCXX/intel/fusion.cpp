// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -o - %s | FileCheck %s
// expected-no-diagnostics

void bar();

void foo() {
  int i, a[10], n = 10, j;

  i = 0;
  #pragma fusion
  while (i < n) {
    // CHECK: br label {{.*}}, !llvm.loop ![[LOOP_1:.*]]
    a[i] += 3;
  }
  i = 0;
  #pragma fusion
  do {
    // CHECK: br i1 %{{.*}}, !llvm.loop ![[LOOP_2:.*]]
    a[i] += 4;
  } while (i < n);
  #pragma fusion
  for (i = 0; i < n; ++i) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_3:.*]]
    a[i] += 5;
  }
  #pragma fusion
  for (auto j: a) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_4:.*]]
    j += 6;
  }
  #pragma fusion
  for (i = 0; i < n; ++i) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_5:.*]]
    for (j = 0; j < n; ++j) {
      // CHECK-NOT: br label %{{.*}}, !llvm.loop !{{.*}}
      a[i] += a[j];
    }
    bar();
  }
  for (i = 0; i < n; ++i) {
    // CHECK-NOT: br label %{{.*}}, !llvm.loop !{{.*}}
    #pragma fusion
    for (j = 0; j < n; ++j) {
      // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_8:.*]]
      a[i] += a[j];
    }
    bar();
  }
}

// CHECK: ![[LOOP_1]] = distinct !{![[LOOP_1]], ![[FUSION:.*]]}
// CHECK: ![[FUSION]] = !{!"llvm.loop.fusion.enable"}
// CHECK: ![[LOOP_2]] = distinct !{![[LOOP_2]], ![[FUSION]]}
// CHECK: ![[LOOP_3]] = distinct !{![[LOOP_3]], ![[FUSION]]}
// CHECK: ![[LOOP_4]] = distinct !{![[LOOP_4]], ![[FUSION]]}
// CHECK: ![[LOOP_5]] = distinct !{![[LOOP_5]], ![[FUSION]]}
// CHECK: ![[LOOP_8]] = distinct !{![[LOOP_8]], ![[FUSION]]}

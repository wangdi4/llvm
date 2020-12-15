// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -fhls -emit-llvm -o - %s | FileCheck %s
// expected-no-diagnostics

void bar();

void foo() {
  int i, a[10], n = 10, j;

  i = 0;
  #pragma nofusion
  while (i < n) {
    // CHECK: br label {{.*}}, !llvm.loop ![[LOOP_1:.*]]
    a[i] += 3;
  }
  i = 0;
  #pragma nofusion
  do {
    // CHECK: br i1 %{{.*}}, !llvm.loop ![[LOOP_2:.*]]
    a[i] += 4;
  } while (i < n);
  #pragma nofusion
  for (i = 0; i < n; ++i) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_3:.*]]
    a[i] += 5;
  }
  #pragma nofusion
  for (auto j: a) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_4:.*]]
    j += 6;
  }
  #pragma nofusion
  for (i = 0; i < n; ++i) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_5:.*]]
    for (j = 0; j < n; ++j) {
      // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_6:.*]]
      a[i] += a[j];
    }
    bar();
  }
  for (i = 0; i < n; ++i) {
    // CHECK-NOT: br label %{{.*}}, !llvm.loop !{{.*}}
    #pragma nofusion
    for (j = 0; j < n; ++j) {
      // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_8:.*]]
      a[i] += a[j];
    }
    bar();
  }
}

// CHECK: ![[LOOP_1]] = distinct !{![[LOOP_1]], ![[LOOP_MUSTPROGRESS:[0-9]+]], ![[NOFUSION:.*]]}
// CHECK: ![[NOFUSION]] = !{!"llvm.loop.fusion.disable"}
// CHECK: ![[LOOP_2]] = distinct !{![[LOOP_2]], ![[LOOP_MUSTPROGRESS]], ![[NOFUSION]]}
// CHECK: ![[LOOP_3]] = distinct !{![[LOOP_3]], ![[LOOP_MUSTPROGRESS]], ![[NOFUSION]]}
// CHECK: ![[LOOP_4]] = distinct !{![[LOOP_4]], ![[NOFUSION]]}
// CHECK: ![[LOOP_5]] = distinct !{![[LOOP_5]], ![[LOOP_MUSTPROGRESS]]}
// CHECK: ![[LOOP_6]] = distinct !{![[LOOP_6]], ![[LOOP_MUSTPROGRESS]], ![[NOFUSION]]}
// CHECK: ![[LOOP_8]] = distinct !{![[LOOP_8]], ![[LOOP_MUSTPROGRESS]], ![[NOFUSION]]}

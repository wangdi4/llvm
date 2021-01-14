// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -o - %s | FileCheck %s
// expected-no-diagnostics

void bar();

void foo() {
  int i, a[10], n = 10, j;

  #pragma ivdep
  for (i = 0; i < n; ++i) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_1:.*]]
    a[i] += 5;
  }
  #pragma ivdep loop
  for (i = 0; i < n; ++i) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_2:.*]]
    a[i] += 5;
  }
  #pragma ivdep back
  for (i = 0; i < n; ++i) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_3:.*]]
    a[i] += 5;
  }
  #pragma ivdep
  for (i = 0; i < n; ++i) {
    // CHECK: call void @{{.*}}bar{{.*}}()
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_4:.*]]
    for (j = 0; j < n; ++j) {
      // CHECK-NOT: br label %{{.*}}, !llvm.loop !{{.*}}
      a[i] += a[j];
    }
    bar();
  }
}

// CHECK: ![[LOOP_1]] = distinct !{![[LOOP_1]], ![[LOOP_MUSTPROGRESS:[0-9]+]], ![[IVDEPBACK:.*]]}
// CHECK: ![[IVDEPBACK]] = !{!"llvm.loop.vectorize.ivdep_back"}
// CHECK: ![[LOOP_2]] = distinct !{![[LOOP_2]], ![[LOOP_MUSTPROGRESS]], ![[IVDEPLOOP:.*]]}
// CHECK: ![[IVDEPLOOP]] = !{!"llvm.loop.vectorize.ivdep_loop"}
// CHECK: ![[LOOP_3]] = distinct !{![[LOOP_3]], ![[LOOP_MUSTPROGRESS]], ![[IVDEPBACK]]}
// CHECK: ![[LOOP_4]] = distinct !{![[LOOP_4]], ![[LOOP_MUSTPROGRESS]], ![[IVDEPBACK]]}

// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -triple i686-windows -o - %s | FileCheck %s
// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -triple x86_64-windows-msvc -o - %s | FileCheck %s
// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -triple i686-unknown-windows-msvc -o - %s | FileCheck %s
// RUN: %clang_cc1 -fintel-compatibility-enable=PragmaNoVector -fintel-compatibility-enable=PragmaVector -emit-llvm -o - %s | FileCheck %s
// expected-no-diagnostics

void foo() {
  int i, j, a[10], n = 10;
  i = 0;
  #pragma novector
  while (i < n) {
    // CHECK: br label {{.*}}, !llvm.loop ![[LOOP_2:.*]]
    a[i] += 3;
  }
  i = 0;
  #pragma novector
  do {
    // CHECK: br i1 %{{.*}}, !llvm.loop ![[LOOP_4:.*]]
    a[i] += 4;
  } while (i < n);
  #pragma novector
  for (i = 0; i < n; ++i) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_5:.*]]
    a[i] += 5;
  }
  #pragma novector
  for (auto j: a) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_6:.*]]
    j += 6;
  }
}

void bar_vector() {
  int i, j, a[10], n = 10;
  i = 0;
  #pragma vector
  while (i < n) {
    // CHECK: br label {{.*}}, !llvm.loop ![[LOOP_12:.*]]
    a[i] += 3;
  }
  i = 0;
  #pragma vector
  do {
    // CHECK: br i1 %{{.*}}, !llvm.loop ![[LOOP_14:.*]]
    a[i] += 4;
  } while (i < n);
  #pragma vector
  for (i = 0; i < n; ++i) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_15:.*]]
    a[i] += 5;
  }
  #pragma vector
  for (auto j: a) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_16:.*]]
    j += 6;
  }
}

void bar_vectoralways() {
  int i, j, a[10], n = 10;
  i = 0;
  #pragma vector always
  while (i < n) {
    // CHECK: br label {{.*}}, !llvm.loop ![[LOOP_22:.*]]
    a[i] += 3;
  }
  i = 0;
  #pragma vector always
  do {
    // CHECK: br i1 %{{.*}}, !llvm.loop ![[LOOP_24:.*]]
    a[i] += 4;
  } while (i < n);
  #pragma vector always
  for (i = 0; i < n; ++i) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_25:.*]]
    a[i] += 5;
  }
  #pragma vector always
  for (auto j: a) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_26:.*]]
    j += 6;
  }
}
// CHECK: define {{.*}}foo1
void foo1()
{
  int n = 10,j = 0, i = 0;
  #pragma novector
  for (i = 0; i < n; ++i) {
    for (j = 0; j < n; ++j) {
      i++;
    }
    foo();
    // CHECK: call {{.*}}foo
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_7:.*]]
  }
}
// CHECK: define {{.*}}foo2
void foo2()
{
  int n = 10,j = 0, i = 0;
  for (i = 0; i < n; ++i) {
    #pragma novector
    for (j = 0; j < n; ++j) {
      i++;
      // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_8:.*]]
    }
    // CHECK: call {{.*}}foo1
    foo1();
  }
}
// CHECK: ![[LOOP_2]] = distinct !{![[LOOP_2]], ![[VENABLE:.*]], ![[VECTORIZE:.*]]}
// CHECK: ![[VENABLE]] = !{!"llvm.loop.vectorize.enable", i1 true}
// CHECK: ![[VECTORIZE]] = !{!"llvm.loop.vectorize.width", i32 1}
// CHECK: ![[LOOP_4]] = distinct !{![[LOOP_4]], ![[VENABLE]], ![[VECTORIZE]]}
// CHECK: ![[LOOP_5]] = distinct !{![[LOOP_5]], ![[VENABLE]], ![[VECTORIZE]]}
// CHECK: ![[LOOP_6]] = distinct !{![[LOOP_6]], ![[VENABLE]], ![[VECTORIZE]]}
// CHECK: ![[LOOP_12]] = distinct !{![[LOOP_12]], ![[VENABLE]]}
// CHECK: ![[LOOP_14]] = distinct !{![[LOOP_14]], ![[VENABLE]]}
// CHECK: ![[LOOP_15]] = distinct !{![[LOOP_15]], ![[VENABLE]]}
// CHECK: ![[LOOP_16]] = distinct !{![[LOOP_16]], ![[VENABLE]]}
// CHECK: ![[LOOP_22]] = distinct !{![[LOOP_22]], ![[VALWAYS:.*]], ![[VENABLE]]}
// CHECK: ![[VALWAYS]] = !{!"llvm.loop.vectorize.ignore_profitability"}
// CHECK: ![[LOOP_24]] = distinct !{![[LOOP_24]], ![[VALWAYS]], ![[VENABLE]]}
// CHECK: ![[LOOP_25]] = distinct !{![[LOOP_25]], ![[VALWAYS]], ![[VENABLE]]}
// CHECK: ![[LOOP_26]] = distinct !{![[LOOP_26]], ![[VALWAYS]], ![[VENABLE]]}
// CHECK: ![[LOOP_7]] = distinct !{![[LOOP_7]], ![[VENABLE]], ![[VECTORIZE]]}
// CHECK: ![[LOOP_8]] = distinct !{![[LOOP_8]], ![[VENABLE]], ![[VECTORIZE]]}

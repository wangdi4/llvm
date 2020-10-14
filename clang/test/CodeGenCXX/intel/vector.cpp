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
    // CHECK: br label {{.*}}, !llvm.loop ![[LOOP_2:[0-9]+]]
    a[i] += 3;
  }
  i = 0;
  #pragma novector
  do {
    // CHECK: br i1 %{{.*}}, !llvm.loop ![[LOOP_4:[0-9]+]]
    a[i] += 4;
  } while (i < n);
  #pragma novector
  for (i = 0; i < n; ++i) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_5:[0-9]+]]
    a[i] += 5;
  }
  #pragma novector
  for (auto j: a) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_6:[0-9]+]]
    j += 6;
  }
}

void bar_vector() {
  int i, j, a[10], n = 10;
  i = 0;
  #pragma vector
  while (i < n) {
    // CHECK-NOT: br label {{.*}}, !llvm.loop ![[LOOP_12:[0-9]+]]
    a[i] += 3;
  }
  i = 0;
  #pragma vector
  do {
    // CHECK-NOT: br i1 %{{.*}}, !llvm.loop ![[LOOP_14:[0-9]+]]
    a[i] += 4;
  } while (i < n);
  #pragma vector
  for (i = 0; i < n; ++i) {
    // CHECK-NOT: br label %{{.*}}, !llvm.loop ![[LOOP_15:[0-9]+]]
    a[i] += 5;
  }
  #pragma vector
  for (auto j: a) {
    // CHECK-NOT: br label %{{.*}}, !llvm.loop ![[LOOP_16:[0-9]+]]
    j += 6;
  }
}

void bar_vectoralways() {
  int i, j, a[10], n = 10;
  i = 0;
  #pragma vector always
  while (i < n) {
    // CHECK: br label {{.*}}, !llvm.loop ![[LOOP_22:[0-9]+]]
    a[i] += 3;
  }
  i = 0;
  #pragma vector always
  do {
    // CHECK: br i1 %{{.*}}, !llvm.loop ![[LOOP_24:[0-9]+]]
    a[i] += 4;
  } while (i < n);
  #pragma vector always
  for (i = 0; i < n; ++i) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_25:[0-9]+]]
    a[i] += 5;
  }
  #pragma vector always
  for (auto j: a) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_26:[0-9]+]]
    j += 6;
  }
}
void bar_vectoraligned() {
  int i, j, a[10], n = 10;
  i = 0;
  #pragma vector aligned
  while (i < n) {
    // CHECK: br label {{.*}}, !llvm.loop ![[LOOP_27:[0-9]+]]
    a[i] += 3;
  }
  i = 0;
  #pragma vector aligned
  do {
    // CHECK: br i1 %{{.*}}, !llvm.loop ![[LOOP_28:[0-9]+]]
    a[i] += 4;
  } while (i < n);
  #pragma vector aligned
  for (i = 0; i < n; ++i) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_29:[0-9]+]]
    a[i] += 5;
  }
  #pragma vector aligned
  for (auto j: a) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_30:[0-9]+]]
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
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_7:[0-9]+]]
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
      // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_8:[0-9]+]]
    }
    // CHECK: call {{.*}}foo1
    foo1();
  }
}

// CHECK: define {{.*}}foo3
void foo3() {
  int n = 10;
  #pragma vector always aligned
  for(int i = 0;i < n; ++i) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_31:[0-9]+]]
  }
}

void foo4() {
  int n = 10;
  #pragma vector always
  #pragma vector aligned
  for(int i = 0;i < n; ++i) {
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_32:[0-9]+]]
  }
}

// CHECK: ![[LOOP_2]] = distinct !{![[LOOP_2]], ![[NOVECTOR:[0-9]+]]}
// CHECK: ![[NOVECTOR]] = !{!"llvm.loop.vectorize.width", i32 1}
// CHECK: ![[LOOP_4]] = distinct !{![[LOOP_4]], ![[NOVECTOR]]}
// CHECK: ![[LOOP_5]] = distinct !{![[LOOP_5]], ![[NOVECTOR]]}
// CHECK: ![[LOOP_6]] = distinct !{![[LOOP_6]], ![[NOVECTOR]]}
// CHECK: ![[LOOP_22]] = distinct !{![[LOOP_22]], ![[VALWAYS:[0-9]+]], ![[VENABLE:[0-9]+]]}
// CHECK: ![[VALWAYS]] = !{!"llvm.loop.vectorize.ignore_profitability"}
// CHECK: ![[VENABLE]] = !{!"llvm.loop.vectorize.enable", i1 true}
// CHECK: ![[LOOP_24]] = distinct !{![[LOOP_24]], ![[VALWAYS]], ![[VENABLE]]}
// CHECK: ![[LOOP_25]] = distinct !{![[LOOP_25]], ![[VALWAYS]], ![[VENABLE]]}
// CHECK: ![[LOOP_26]] = distinct !{![[LOOP_26]], ![[VALWAYS]], ![[VENABLE]]}
// CHECK: ![[LOOP_27]] = distinct !{![[LOOP_27]], ![[VALIGNED:[0-9]+]]}
// CHECK: ![[VALIGNED]] = !{!"llvm.loop.intel.vector.aligned"}
// CHECK: ![[LOOP_28]] = distinct !{![[LOOP_28]], ![[VALIGNED]]}
// CHECK: ![[LOOP_29]] = distinct !{![[LOOP_29]], ![[VALIGNED]]}
// CHECK: ![[LOOP_30]] = distinct !{![[LOOP_30]], ![[VALIGNED]]}
// CHECK: ![[LOOP_7]] = distinct !{![[LOOP_7]], ![[NOVECTOR]]}
// CHECK: ![[LOOP_8]] = distinct !{![[LOOP_8]], ![[NOVECTOR]]}
// CHECK: ![[LOOP_31]] = distinct !{![[LOOP_31]], ![[VALWAYS]], ![[VENABLE]], ![[VALIGNED]]}
// CHECK: ![[LOOP_32]] = distinct !{![[LOOP_32]], ![[VALWAYS]], ![[VENABLE]], ![[VALIGNED]]}

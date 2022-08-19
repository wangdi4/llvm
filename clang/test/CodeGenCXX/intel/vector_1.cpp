// RUN: %clang_cc1 -fintel-compatibility -std=c++11 -emit-llvm -o - \
// RUN:  %s | FileCheck %s

// expected-no-diagnostics

struct S {
  int a;
  S();
  ~S();
  template <int t>
  void apply() {
#pragma vector temporal
    for (int i = 0; i < 1024; i++) {
      // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_2:[0-9]+]]
      a++;
    }
#pragma vector nontemporal
    for (int i = 0; i < 1024; i++) {
      // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_5:[0-9]+]]
      a++;
    }
#pragma vector vectorlength(t, 2)
    for (int i = 0; i < 1024; i++) {
      // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_7:[0-9]+]]
      a++;
    }
  }

};
template<typename T>
void foo(T x) {

#pragma vector temporal
  for (int i = 0; i < 1024; i++)
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_9:[0-9]+]]
    x++;
#pragma vector nontemporal
  for (int i = 0; i < 1024; i++)
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_10:[0-9]+]]
    x++;
#pragma vector vectorlength(2,4)
  for (int i = 0; i < 1024; i++)
    // CHECK: br label %{{.*}}, !llvm.loop ![[LOOP_11:[0-9]+]]
    x++;
}
void use_template() {
   S obj;
   obj.apply<10>();
   foo(10);
}
// CHECK: ![[LOOP_2]] = distinct !{![[LOOP_2]], ![[LOOP_MUSTPROGRESS:[0-9]+]], ![[TEMPORAL:[0-9]+]]}
// CHECK: ![[LOOP_MUSTPROGRESS]] = !{!"llvm.loop.mustprogress"}
// CHECK: ![[TEMPORAL]] = !{!"llvm.loop.intel.vector.temporal"}
// CHECK: ![[LOOP_5]] = distinct !{![[LOOP_5]], ![[LOOP_MUSTPROGRESS]], ![[NONTEMPORAL:[0-9]+]]}
// CHECK: ![[NONTEMPORAL]] = !{!"llvm.loop.intel.vector.nontemporal"}
// CHECK: ![[LOOP_7]] = distinct !{![[LOOP_7]], ![[LOOP_MUSTPROGRESS]], ![[VECTORLENGTH:[0-9]+]]}
// CHECK: ![[VECTORLENGTH]] = !{!"llvm.loop.intel.vector.vectorlength", i32 10, i32 2}
// CHECK: ![[LOOP_9]] = distinct !{![[LOOP_9]], ![[LOOP_MUSTPROGRESS]], ![[TEMPORAL]]}
// CHECK: ![[LOOP_10]] = distinct !{![[LOOP_10]], ![[LOOP_MUSTPROGRESS]], ![[NONTEMPORAL]]}
// CHECK: ![[LOOP_11]] = distinct !{![[LOOP_11]], ![[LOOP_MUSTPROGRESS]], ![[VECTORLENGTH1:[0-9]+]]}
// CHECK: ![[VECTORLENGTH1]] =  !{!"llvm.loop.intel.vector.vectorlength", i32 2, i32 4}

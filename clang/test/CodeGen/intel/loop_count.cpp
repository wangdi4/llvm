//RUN: %clang_cc1 -fintel-compatibility -emit-llvm %s -o - | FileCheck %s
//RUN: %clang_cc1 -fintel-compatibility -debug-info-kind=limited -emit-llvm -o %t %s

void bar(int i);
int ibar(int i);

#define min(a,b) (((a) < (b)) ? (a) : (b))

//CHECK-LABEL: foo
void foo()
{
  //CHECK: br{{.*}}!llvm.loop [[LOOP_1:![0-9]+]]
  #pragma loop_count(1,2)
  for (int i = 0; i < 10; ++i) {
    bar(i);
  }
  //CHECK: br{{.*}}!llvm.loop [[LOOP_2:![0-9]+]]
  #pragma loop_count=3,4
  for (int i = 0; i < 10; ++i) {
    bar(i);
  }
  //CHECK: br{{.*}}!llvm.loop [[LOOP_3:![0-9]+]]
  #pragma loop_count=1,2
  for (int i = 0; i < 10; ++i) {
    bar(i);
  }
  //CHECK: br{{.*}}!llvm.loop [[LOOP_4:![0-9]+]]
  #pragma loop_count(1) min(min(4,10)) max(10) avg(2)
  for (int i = 0; i < 10; ++i) {
    bar(i);
  }
  //CHECK: br{{.*}}!llvm.loop [[LOOP_5:![0-9]+]]
  #pragma loop_count (1,2,3)
  #pragma loop_count min=1
  #pragma loop_count max=10
  #pragma loop_count avg=3
  for (int i = 0; i < 10; ++i) {
    bar(i);
  }
}


// CHECK: [[LOOP_1]] = distinct !{[[LOOP_1]], ![[LOOP_MUSTPROGRESS:[0-9]+]], [[ATTR_1:!.+]]}
// CHECK: [[ATTR_1]] = !{!"llvm.loop.intel.loopcount", i32 1, i32 2}
// CHECK: [[LOOP_2]] = distinct !{[[LOOP_2]], ![[LOOP_MUSTPROGRESS]], [[ATTR_11:!.+]]}
// CHECK: [[ATTR_11]] = !{!"llvm.loop.intel.loopcount", i32 3, i32 4}
// CHECK: [[LOOP_4]] = distinct !{[[LOOP_4]], ![[LOOP_MUSTPROGRESS]], [[ATTR_1:!.+]], [[ATTR_42:!.+]], [[ATTR_43:!.+]], [[ATTR_44:!.+]]}
// CHECK: [[ATTR_42]] = !{!"llvm.loop.intel.loopcount_minimum", i32 4}
// CHECK: [[ATTR_43]] = !{!"llvm.loop.intel.loopcount_maximum", i32 10}
// CHECK: [[ATTR_44]] = !{!"llvm.loop.intel.loopcount_average", i32 2}
// CHECK: [[LOOP_5]] = distinct !{[[LOOP_5]], ![[LOOP_MUSTPROGRESS]], [[ATTR_51:!.+]], [[ATTR_52:!.+]], [[ATTR_43:!.+]], [[ATTR_54:!.+]]}
// CHECK: [[ATTR_51]] = !{!"llvm.loop.intel.loopcount", i32 1, i32 2, i32 3}
// CHECK: [[ATTR_52]] = !{!"llvm.loop.intel.loopcount_minimum", i32 1}
// CHECK: [[ATTR_54]] = !{!"llvm.loop.intel.loopcount_average", i32 3}

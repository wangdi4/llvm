// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple i386-windows-pc -opaque-pointers -emit-dtrans-info -emit-llvm %s -o - | FileCheck %s

// Ensure that we properly handle 'Expand' parameters,
// which seemingly only happen on i386 windows.
struct Simple {
  int *IPtr;
};

void SimpleFunc(Simple s) {}
// CHECK: define {{.*}}void @"?SimpleFunc@@YAXUSimple@@@Z"(ptr "intel_dtrans_func_index"="1" %{{.+}}){{.*}} !intel.dtrans.func.type ![[SIMPLE_FUNC:[0-9]+]]

struct HasArrays {
  int IArr[3];
};

// Note: It is not clear how to get an array to be an 'expand', so it isn't 
// really clear how to reproduce this, but adding a test for completeness.
void HasArraysFunc(HasArrays a){}
// CHECK: define {{.*}}void @"?HasArraysFunc@@YAXUHasArrays@@@Z"(ptr noundef byval(%"struct..?AUHasArrays@@.HasArrays"){{.*}} "intel_dtrans_func_index"="1" %{{.+}}){{.*}} !intel.dtrans.func.type ![[HASARRAYS_FUNC:[0-9]+]]

struct HasComplex {
  _Complex int Cplx;
};

// Complex cant be pointers, so elements will never result in DTrans info, but
// make sure we don't crash.
void HasComplexFunc(HasComplex a){}
// CHECK: define {{.*}}void @"?HasComplexFunc@@YAXUHasComplex@@@Z"(i32 %{{.+}}, i32 %{{.+}})


struct HasPointerAndStuff {
  int *P;
  float F;
  int I;
};

// CHECK: define {{.*}}void @"?HasPointerAndStuffFunc@@YAXUHasPointerAndStuff@@@Z"(ptr "intel_dtrans_func_index"="1" %{{.+}}, float %{{.+}}, i32 %{{.+}}){{.*}} !intel.dtrans.func.type ![[HASSTUFF_FUNC:[0-9]+]]
void HasPointerAndStuffFunc(HasPointerAndStuff a){}

// CHECK: !intel.dtrans.types = !{![[SIMPLE_TY:[0-9]+]], ![[HASARRAYS_TY:[0-9]+]], ![[HASCOMPLEX_TY:[0-9]+]], ![[HASSTUFF_TY:[0-9]+]]}

// CHECK: ![[SIMPLE_TY]] = !{!"S", %"struct..?AUSimple@@.Simple" zeroinitializer, i32 1, ![[INTPTR:[0-9]+]]}
// CHECK: ![[INTPTR]] = !{i32 0, i32 1}
// CHECK: ![[HASARRAYS_TY]] = !{!"S", %"struct..?AUHasArrays@@.HasArrays" zeroinitializer, i32 1, ![[INT3_ARR:[0-9]+]]
// CHECK: ![[INT3_ARR]] = !{!"A", i32 3, ![[INT:[0-9]+]]}
// CHECK: ![[INT]] = !{i32 0, i32 0}
// CHECK: ![[HASCOMPLEX_TY]] = !{!"S", %"struct..?AUHasComplex@@.HasComplex" zeroinitializer, i32 1, ![[CPLX_ELT:[0-9]+]]}
// CHECK: ![[CPLX_ELT]] = !{![[CPLX:[0-9]+]], i32 0}
// CHECK: ![[CPLX]] = !{!"L", i32 2, ![[INT]], ![[INT]]}
// CHECK: ![[HASSTUFF_TY]] = !{!"S", %"struct..?AUHasPointerAndStuff@@.HasPointerAndStuff" zeroinitializer, i32 3, ![[INTPTR]], ![[FLOAT:[0-9]+]], ![[INT]]}
// CHECK: ![[FLOAT]] = !{float 0{{.*}}, i32 0}

// CHECK: ![[SIMPLE_FUNC]] = distinct !{![[INTPTR]]}
// CHECK: ![[HASARRAYS_FUNC]] = distinct !{![[HASARRAYS_PTR:[0-9]+]]}
// CHECK: ![[HASARRAYS_PTR]] = !{%"struct..?AUHasArrays@@.HasArrays" zeroinitializer, i32 1}
// CHECK: ![[HASSTUFF_FUNC]] = distinct !{![[INTPTR]]}

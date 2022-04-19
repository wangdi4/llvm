// RUN: %clang_cc1 -DTEST1 -O1 -triple x86_64-unknown-unknown -disable-llvm-passes -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s
// RUN: %clang_cc1 -DTEST2 -O1 -triple x86_64-unknown-unknown -disable-llvm-passes -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s
// RUN: %clang_cc1 -DTEST3 -O1 -triple x86_64-unknown-unknown -disable-llvm-passes -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s
// RUN: %clang_cc1 -DTEST4 -O1 -triple x86_64-unknown-unknown -disable-llvm-passes -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s
// RUN: %clang_cc1 -DTEST5 -O1 -triple x86_64-unknown-unknown -disable-llvm-passes -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s

#ifdef TEST1
#define TEST *rp = *rq
#endif

#ifdef TEST2
#define TEST *(rp+1) = *rq
#endif

#ifdef TEST3
#define TEST *rp = *(rq+1)
#endif

#ifdef TEST4
#define TEST rp[10] = *rq
#endif

#ifdef TEST5
#define TEST *rp = rq[10]
#endif

void foo(int *p, int *q) {
  {
    int *restrict rp = p;
    int *restrict rq = q;
// CHECK: %rp = alloca i32*
// CHECK: %rq = alloca i32*

    TEST;
// CHECK: load {{.*}},{{.*}} !alias.scope [[scope0:!.*]], !noalias [[scope1:!.*]]
// CHECK: store {{.*}},{{.*}} !alias.scope [[scope1]], !noalias [[scope0]]
  }
}

// CHECK-DAG: [[domain:!.*]] = distinct !{[[domain]], !"foo"}
//
// CHECK-DAG: [[scope0]] = !{[[scope0_0:!.*]]}
// CHECK-DAG: [[scope0_0]] = distinct !{[[scope0_0]], [[domain]], !"foo: rq"}
//
// CHECK-DAG: [[scope1]] = !{[[scope1_0:!.*]]}
// CHECK-DAG: [[scope1_0]] = distinct !{[[scope1_0]], [[domain]], !"foo: rp"}


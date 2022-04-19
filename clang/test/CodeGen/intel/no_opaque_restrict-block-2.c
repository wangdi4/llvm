// RUN: %clang_cc1 -DTEST1 -O1 -triple x86_64-unknown-unknown -disable-llvm-passes -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s

void foo(int *p, int *q, int *r) {
  {
    int *restrict rp = p;
    int *restrict rq = q;
// CHECK: %rp = alloca i32*
// CHECK: %rq = alloca i32*
// CHECK: %rr = alloca i32*

    *rp = *rq;
// CHECK: load {{.*}},{{.*}} !alias.scope [[scope0:!.*]], !noalias [[scope1:!.*]]
// CHECK: store {{.*}},{{.*}} !alias.scope [[scope1]], !noalias [[scope0]]

    {
      int *restrict rr = r;

      *rr = *rp;
// CHECK: load {{.*}},{{.*}} !alias.scope [[scope1]], !noalias [[non_rp:!.*]]
// CHECK: store {{.*}},{{.*}} !alias.scope [[scope2:!.*]], !noalias [[non_rr:!.*]]
    }
  }
}

// CHECK-DAG: [[domain:!.*]] = distinct !{[[domain]], !"foo"}
//
// CHECK-DAG: [[scope0]] = !{[[scope0_0:!.*]]}
// CHECK-DAG: [[scope0_0]] = distinct !{[[scope0_0]], [[domain]], !"foo: rq"}
//
// CHECK-DAG: [[scope1]] = !{[[scope1_0:!.*]]}
// CHECK-DAG: [[scope1_0]] = distinct !{[[scope1_0]], [[domain]], !"foo: rp"}
//
// CHECK-DAG: [[scope2]] = !{[[scope2_0:!.*]]}
// CHECK-DAG: [[scope2_0]] = distinct !{[[scope2_0]], [[domain]], !"foo: rr"}
//
// CHECK-DAG: [[non_rp]] = !{[[scope0_0]], [[scope2_0]]}
// CHECK-DAG: [[non_rr]] = !{[[scope1_0]], [[scope0_0]]}


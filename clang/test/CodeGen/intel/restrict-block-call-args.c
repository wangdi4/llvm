// RUN: %clang_cc1 -O1 -triple x86_64-unknown-unknown -disable-llvm-passes -emit-llvm %s -o - | FileCheck %s

void bar(int *p, int *q) {
  *p = *q;
}

void foo(int *p, int *q) {
  {
    int *restrict rp = p;
    int *restrict rq = q;
// CHECK: %rp = alloca i32*
// CHECK: %rq = alloca i32*

    bar(rp, rq);
// CHECK: call void @bar(i32* noundef %4, i32* noundef %5), !intel.args.alias.scope [[SCOPE_LIST:!.*]]
  }
}

// CHECK-DAG: [[SCOPE_LIST]] = !{[[scope0:!.*]], [[scope1:!.*]]}
//
// CHECK-DAG: [[scope0]] = !{[[scope0_0:!.*]]}
// CHECK-DAG: [[scope1]] = !{[[scope1_0:!.*]]}
//
// CHECK-DAG: [[domain:!.*]] = distinct !{[[domain]], !"foo"}
//
// CHECK-DAG: [[scope0_0]] = distinct !{[[scope0_0]], [[domain]], !"foo: rp"}
// CHECK-DAG: [[scope1_0]] = distinct !{[[scope1_0]], [[domain]], !"foo: rq"}


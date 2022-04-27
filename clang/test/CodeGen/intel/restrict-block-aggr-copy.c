// RUN: %clang_cc1 -DTEST1 -O1 -triple x86_64-unknown-unknown -disable-llvm-passes -emit-llvm -opaque-pointers %s -o - | FileCheck %s

typedef struct _A {
  int x;
  float y;
} A;

void foo(A *p, A *q) {
  {
    A *restrict rp = p;
    A *restrict rq = q;
// CHECK: %rp = alloca ptr 
// CHECK: %rq = alloca ptr 

    *rp = *rq;
// CHECK: call void @llvm.memcpy
// CHECK-SAME: !alias.scope [[scopes:![0-9]*]]
  }
}

// CHECK: [[scopes]] = !{[[scope0:!.*]], [[scope1:!.*]]}
//
// CHECK-DAG: [[domain:!.*]] = distinct !{[[domain]], !"foo"}
//
// CHECK-DAG: [[scope0]] = distinct !{[[scope0]], [[domain]], !"foo: rq"}
// CHECK-DAG: [[scope1]] = distinct !{[[scope1]], [[domain]], !"foo: rp"}


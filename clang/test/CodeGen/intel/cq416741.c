// RUN: %clang_cc1 -O1 -triple x86_64-unknown-unknown -fintel-compatibility %s -emit-llvm -o - | FileCheck %s

struct test_node { int a; };
struct test_node n0[2];
struct test_node *n1;

int check() {
    n0[1].a = 10;
    n1->a = 20;
    return n0[1].a;
}
// CHECK-NOT: array@_ZTSA2_9test_node

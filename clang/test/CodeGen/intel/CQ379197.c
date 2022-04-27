// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fintel-compatibility %s -opaque-pointers -emit-llvm -o - -verify | FileCheck %s
// expected-no-diagnostics

signed int a, b, aa, bb;
char *c, *d, *cc, *dd;
// CHECK-LABEL: @main
int main() {
// CHECK: load i32, ptr @b, align 4
// CHECK: load i32, ptr %.atomictmp{{[0-9]*}}, align 4
// CHECK: cmpxchg weak ptr @a, i32 %{{[0-9]*}}, i32 %{{[0-9]*}} monotonic monotonic
__atomic_compare_exchange_weak_explicit(&a, &b, 0, 0, 0);
// CHECK: load i64, ptr @d, align 8
// CHECK: cmpxchg weak ptr @c, i64 %{{[0-9]*}}, i64 %{{[0-9]*}} monotonic monotonic
__atomic_compare_exchange_weak_explicit(&c, &d, 0, 0, 0);
// CHECK: load i32, ptr @bb, align 4
// CHECK: load i32, ptr %.atomictmp{{[0-9]*}}, align 4
// CHECK: cmpxchg ptr @aa, i32 %{{[0-9]*}}, i32 %{{[0-9]*}} monotonic monotonic
__atomic_compare_exchange_strong_explicit(&aa, &bb, 0, 0, 0);
// CHECK: load i64, ptr @dd, align 8 
// CHECK: cmpxchg ptr @cc, i64 %{{[0-9]*}}, i64 %{{[0-9]*}} monotonic monotonic
__atomic_compare_exchange_strong_explicit(&cc, &dd, 0, 0, 0); 
}


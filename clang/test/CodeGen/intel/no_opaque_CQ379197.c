// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fintel-compatibility %s -emit-llvm -no-opaque-pointers -o - -verify | FileCheck %s
// expected-no-diagnostics

signed int a, b, aa, bb;
char *c, *d, *cc, *dd;
// CHECK-LABEL: @main
int main() {
// CHECK: load i32, i32* @b, align 4
// CHECK: load i32, i32* %.atomictmp{{[0-9]*}}, align 4
// CHECK: cmpxchg weak i32* @a, i32 %{{[0-9]*}}, i32 %{{[0-9]*}} monotonic monotonic
__atomic_compare_exchange_weak_explicit(&a, &b, 0, 0, 0);
// CHECK: load i64, i64* bitcast (i8** @d to i64*), align 8
// CHECK: cmpxchg weak i64* bitcast (i8** @c to i64*), i64 %{{[0-9]*}}, i64 %{{[0-9]*}} monotonic monotonic
__atomic_compare_exchange_weak_explicit(&c, &d, 0, 0, 0);
// CHECK: load i32, i32* @bb, align 4
// CHECK: load i32, i32* %.atomictmp{{[0-9]*}}, align 4
// CHECK: cmpxchg i32* @aa, i32 %{{[0-9]*}}, i32 %{{[0-9]*}} monotonic monotonic
__atomic_compare_exchange_strong_explicit(&aa, &bb, 0, 0, 0);
// CHECK: load i64, i64* bitcast (i8** @dd to i64*), align 8 
// CHECK: cmpxchg i64* bitcast (i8** @cc to i64*), i64 %{{[0-9]*}}, i64 %{{[0-9]*}} monotonic monotonic
__atomic_compare_exchange_strong_explicit(&cc, &dd, 0, 0, 0); 
}


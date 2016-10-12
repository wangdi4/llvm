// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fintel-compatibility %s -emit-llvm -o - -verify | FileCheck %s
// expected-no-diagnostics

signed int a, b, aa, bb;
char *c, *d, *cc, *dd;
// CHECK-LABEL: @main
int main() {
// CHECK: %0 = load i32, i32* @b, align 4
// CHECK: %1 = load i32, i32* %.atomictmp, align 4
// CHECK: cmpxchg weak i32* @a, i32 %0, i32 %1 monotonic monotonic
__atomic_compare_exchange_weak_explicit(&a, &b, 0, 0, 0);
// CHECK: %7 = load i64, i64* bitcast (i8** @d to i64*), align 8
// CHECK: %9 = cmpxchg weak i64* bitcast (i8** @c to i64*), i64 %7, i64 %8 monotonic monotonic
__atomic_compare_exchange_weak_explicit(&c, &d, 0, 0, 0);
// CHECK: %13 = load i32, i32* @bb, align 4
// CHECK: %14 = load i32, i32* %.atomictmp7, align 4
// CHECK: cmpxchg i32* @aa, i32 %13, i32 %14 monotonic monotonic
__atomic_compare_exchange_strong_explicit(&aa, &bb, 0, 0, 0);
// CHECK: %20 = load i64, i64* bitcast (i8** @dd to i64*), align 8 
// CHECK: %22 = cmpxchg i64* bitcast (i8** @cc to i64*), i64 %20, i64 %21 monotonic monotonic
__atomic_compare_exchange_strong_explicit(&cc, &dd, 0, 0, 0); 
}


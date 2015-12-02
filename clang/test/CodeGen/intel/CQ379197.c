// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fintel-compatibility %s -emit-llvm -o - -verify | FileCheck %s
// expected-no-diagnostics

signed int a, b;
char *c, *d;
// CHECK-LABEL: @main
int main() {
// CHECK: call i32 @__atomic_compare_exchange_weak_explicit_4(i32* {{[^,]+}}, i32* {{[^,]+}}, i32 0, i32 0, i32 0)
__atomic_compare_exchange_weak_explicit(&a, &b, 0, 0, 0);
// CHECK: call i32 @__atomic_compare_exchange_weak_explicit_8(i64* {{[^,]+}}, i64* {{[^,]+}}, i64 0, i32 0, i32 0)
__atomic_compare_exchange_weak_explicit(&c, &d, 0, 0, 0);
// CHECK: call i32 @__atomic_compare_exchange_strong_explicit_4(i32* {{[^,]+}}, i32* {{[^,]+}}, i32 0, i32 0, i32 0)
__atomic_compare_exchange_strong_explicit(&a, &b, 0, 0, 0);
// CHECK: call i32 @__atomic_compare_exchange_strong_explicit_8(i64* {{[^,]+}}, i64* {{[^,]+}}, i64 0, i32 0, i32 0)
__atomic_compare_exchange_strong_explicit(&c, &d, 0, 0, 0);
}


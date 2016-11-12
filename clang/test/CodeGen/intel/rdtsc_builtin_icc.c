// RUN: %clang_cc1 -triple x86_64-unknown-unknown -emit-llvm -o - -fintel-compatibility %s | FileCheck %s
int test_rdtsc() {
  return _rdtsc();
// CHECK: @test_rdtsc
// CHECK: call i64 @llvm.x86.rdtsc
}

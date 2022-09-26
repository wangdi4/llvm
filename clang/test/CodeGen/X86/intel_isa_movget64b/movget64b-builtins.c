// REQUIRES: intel_feature_isa_movget64b
// RUN: %clang_cc1 -no-opaque-pointers %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +movget64b \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

unsigned long long test_movget64b_u64(const __m512i *__A) {
  // CHECK-LABEL: @test_movget64b_u64(
  // CHECK: call i64 @llvm.x86.movget64b64(i8* %{{.*}})
  return _movget64b_u64(__A);
}

unsigned long long test_movget64b_u32(const __m512i *__A) {
  // CHECK-LABEL: @test_movget64b_u32(
  // CHECK: call i32 @llvm.x86.movget64b32(i8* %{{.*}})
  return _movget64b_u32(__A);
}

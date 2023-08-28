// REQUIRES: intel_feature_isa_gpr_movget
// RUN: %clang_cc1 -no-opaque-pointers %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +gprmovget \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <x86gprintrin.h>
#include <stddef.h>

unsigned long long test_movget_u64(const unsigned long long *__A) {
  // CHECK-LABEL: @test_movget_u64(
  // CHECK: call i64 @llvm.x86.movget64(i8* %{{.*}})
  return _movget_u64(__A);
}

unsigned int test_movget_u32(const unsigned int *__A) {
  // CHECK-LABEL: @test_movget_u32(
  // CHECK: call i32 @llvm.x86.movget32(i8* %{{.*}})
  return _movget_u32(__A);
}

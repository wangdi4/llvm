// REQUIRES: intel_feature_isa_rao_int
// RUN: %clang_cc1 -no-opaque-pointers %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +raoint \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <stddef.h>
#include <x86gprintrin.h>

void test_aadd_si32(int *__A, int __B) {
  // CHECK-LABEL: @test_aadd_si32(
  // CHECK: call void @llvm.x86.aadd32(i8* %{{.*}}, i32 %{{.*}})
  _aadd_si32(__A, __B);
}

void test_aand_si32(int *__A, int __B) {
  // CHECK-LABEL: @test_aand_si32(
  // CHECK: call void @llvm.x86.aand32(i8* %{{.*}}, i32 %{{.*}})
  _aand_si32(__A, __B);
}

void test_aor_si32(int *__A, int __B) {
  // CHECK-LABEL: @test_aor_si32(
  // CHECK: call void @llvm.x86.aor32(i8* %{{.*}}, i32 %{{.*}})
  _aor_si32(__A, __B);
}

void test_axor_si32(int *__A, int __B) {
  // CHECK-LABEL: @test_axor_si32(
  // CHECK: call void @llvm.x86.axor32(i8* %{{.*}}, i32 %{{.*}})
  _axor_si32(__A, __B);
}

void test_aadd_si64(long long *__A, long long __B) {
  // CHECK-LABEL: @test_aadd_si64(
  // CHECK: call void @llvm.x86.aadd64(i8* %{{.*}}, i64 %{{.*}})
  _aadd_si64(__A, __B);
}

void test_aand_si64(long long *__A, long long __B) {
  // CHECK-LABEL: @test_aand_si64(
  // CHECK: call void @llvm.x86.aand64(i8* %{{.*}}, i64 %{{.*}})
  _aand_si64(__A, __B);
}

void test_aor_si64(long long *__A, long long __B) {
  // CHECK-LABEL: @test_aor_si64(
  // CHECK: call void @llvm.x86.aor64(i8* %{{.*}}, i64 %{{.*}})
  _aor_si64(__A, __B);
}

void test_axor_si64(long long *__A, long long __B) {
  // CHECK-LABEL: @test_axor_si64(
  // CHECK: call void @llvm.x86.axor64(i8* %{{.*}}, i64 %{{.*}})
  _axor_si64(__A, __B);
}

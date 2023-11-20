// REQUIRES: intel_feature_isa_avx256p_unsupported
// RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-apple-darwin -target-feature +avx256p -emit-llvm -o - -Wall -Werror -Wsign-conversion | FileCheck %s
// RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-apple-darwin -target-feature +avx256p -fno-signed-char -emit-llvm -o - -Wall -Werror -Wsign-conversion | FileCheck %s


#include <immintrin.h>

__mmask32 test_knot_mask32(__mmask32 a) {
  // CHECK-LABEL: @test_knot_mask32
  // CHECK: [[IN:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[NOT:%.*]] = xor <32 x i1> [[IN]], <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  return _knot_mask32(a);
}

__mmask32 test_kand_mask32(__mmask32 a, __mmask32 b) {
  // CHECK-LABEL: @test_kand_mask32
  // CHECK: [[LHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RES:%.*]] = and <32 x i1> [[LHS]], [[RHS]]
  return _kand_mask32(a, b);
}

__mmask32 test_kandn_mask32(__mmask32 a, __mmask32 b) {
  // CHECK-LABEL: @test_kandn_mask32
  // CHECK: [[LHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[NOT:%.*]] = xor <32 x i1> [[LHS]], <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  // CHECK: [[RES:%.*]] = and <32 x i1> [[NOT]], [[RHS]]
  return _kandn_mask32(a, b);
}

__mmask32 test_kor_mask32(__mmask32 a, __mmask32 b) {
  // CHECK-LABEL: @test_kor_mask32
  // CHECK: [[LHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RES:%.*]] = or <32 x i1> [[LHS]], [[RHS]]
  return _kor_mask32(a, b);
}

__mmask32 test_kxnor_mask32(__mmask32 a, __mmask32 b) {
  // CHECK-LABEL: @test_kxnor_mask32
  // CHECK: [[LHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[NOT:%.*]] = xor <32 x i1> [[LHS]], <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  // CHECK: [[RES:%.*]] = xor <32 x i1> [[NOT]], [[RHS]]
  return _kxnor_mask32(a, b);
}

__mmask32 test_kxor_mask32(__mmask32 a, __mmask32 b) {
  // CHECK-LABEL: @test_kxor_mask32
  // CHECK: [[LHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RES:%.*]] = xor <32 x i1> [[LHS]], [[RHS]]
  return _kxor_mask32(a, b);
}

unsigned char test_kortestz_mask32_u8(__mmask32 a, __mmask32 b) {
  // CHECK-LABEL: @test_kortestz_mask32_u8
  // CHECK: [[LHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[OR:%.*]] = or <32 x i1> [[LHS]], [[RHS]]
  // CHECK: [[CAST:%.*]] = bitcast <32 x i1> [[OR]] to i32
  // CHECK: [[CMP:%.*]] = icmp eq i32 [[CAST]], 0
  // CHECK: [[ZEXT:%.*]] = zext i1 [[CMP]] to i32
  // CHECK: trunc i32 [[ZEXT]] to i8
  return _kortestz_mask32_u8(a, b);
}

unsigned char test_kortestc_mask32_u8(__mmask32 a, __mmask32 b) {
  // CHECK-LABEL: @test_kortestc_mask32_u8
  // CHECK: [[LHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[OR:%.*]] = or <32 x i1> [[LHS]], [[RHS]]
  // CHECK: [[CAST:%.*]] = bitcast <32 x i1> [[OR]] to i32
  // CHECK: [[CMP:%.*]] = icmp eq i32 [[CAST]], -1
  // CHECK: [[ZEXT:%.*]] = zext i1 [[CMP]] to i32
  // CHECK: trunc i32 [[ZEXT]] to i8
  return _kortestc_mask32_u8(a, b);
}

unsigned char test_kortest_mask32_u8(__mmask32 a, __mmask32 b, unsigned char *CF) {
  // CHECK-LABEL: @test_kortest_mask32_u8
  // CHECK: [[LHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[OR:%.*]] = or <32 x i1> [[LHS]], [[RHS]]
  // CHECK: [[CAST:%.*]] = bitcast <32 x i1> [[OR]] to i32
  // CHECK: [[CMP:%.*]] = icmp eq i32 [[CAST]], -1
  // CHECK: [[ZEXT:%.*]] = zext i1 [[CMP]] to i32
  // CHECK: trunc i32 [[ZEXT]] to i8
  // CHECK: [[LHS2:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RHS2:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[OR2:%.*]] = or <32 x i1> [[LHS2]], [[RHS2]]
  // CHECK: [[CAST2:%.*]] = bitcast <32 x i1> [[OR2]] to i32
  // CHECK: [[CMP2:%.*]] = icmp eq i32 [[CAST2]], 0
  // CHECK: [[ZEXT2:%.*]] = zext i1 [[CMP2]] to i32
  // CHECK: trunc i32 [[ZEXT2]] to i8
  return _kortest_mask32_u8(a, b, CF);
}

unsigned char test_ktestz_mask32_u8(__mmask32 a, __mmask32 b) {
  // CHECK-LABEL: @test_ktestz_mask32_u8
  // CHECK: [[LHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RES:%.*]] = call i32 @llvm.x86.avx512.ktestz.d(<32 x i1> [[LHS]], <32 x i1> [[RHS]])
  // CHECK: trunc i32 [[RES]] to i8
  return _ktestz_mask32_u8(a, b);
}

unsigned char test_ktestc_mask32_u8(__mmask32 a, __mmask32 b) {
  // CHECK-LABEL: @test_ktestc_mask32_u8
  // CHECK: [[LHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RES:%.*]] = call i32 @llvm.x86.avx512.ktestc.d(<32 x i1> [[LHS]], <32 x i1> [[RHS]])
  // CHECK: trunc i32 [[RES]] to i8
  return _ktestc_mask32_u8(a, b);
}

unsigned char test_ktest_mask32_u8(__mmask32 a, __mmask32 b, unsigned char *CF) {
  // CHECK-LABEL: @test_ktest_mask32_u8
  // CHECK: [[LHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RES:%.*]] = call i32 @llvm.x86.avx512.ktestc.d(<32 x i1> [[LHS]], <32 x i1> [[RHS]])
  // CHECK: trunc i32 [[RES]] to i8
  // CHECK: [[LHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RES:%.*]] = call i32 @llvm.x86.avx512.ktestz.d(<32 x i1> [[LHS]], <32 x i1> [[RHS]])
  // CHECK: trunc i32 [[RES]] to i8
  return _ktest_mask32_u8(a, b, CF);
}

__mmask32 test_kadd_mask32(__mmask32 a, __mmask32 b) {
  // CHECK-LABEL: @test_kadd_mask32
  // CHECK: [[LHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RES:%.*]] = call <32 x i1> @llvm.x86.avx512.kadd.d(<32 x i1> [[LHS]], <32 x i1> [[RHS]])
  return _kadd_mask32(a, b);
}

__mmask32 test_kshiftli_mask32(__mmask32 a) {
  // CHECK-LABEL: @test_kshiftli_mask32
  // CHECK: [[VAL:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RES:%.*]] = shufflevector <32 x i1> zeroinitializer, <32 x i1> [[VAL]], <32 x i32> <i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32>
  return _kshiftli_mask32(a, 31);
}

__mmask32 test_kshiftri_mask32(__mmask32 a) {
  // CHECK-LABEL: @test_kshiftri_mask32
  // CHECK: [[VAL:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK: [[RES:%.*]] = shufflevector <32 x i1> [[VAL]], <32 x i1> zeroinitializer, <32 x i32> <i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47, i32 48, i32 49, i32 50, i32 51, i32 52, i32 53, i32 54, i32 55, i32 56, i32 57, i32 58, i32 59, i32 60, i32 61, i32 62>
  return _kshiftri_mask32(a, 31);
}

__mmask32 test_cvtu32_mask32(unsigned int a) {
  // CHECK-LABEL: @test_cvtu32_mask32
  return _cvtu32_mask32(a);
}

__mmask32 test_load_mask32(__mmask32 *A) {
  // CHECK-LABEL: @test_load_mask32
  // CHECK: [[LOAD:%.*]] = load i32, ptr %{{.*}}
  return _load_mask32(A);
}

void test_store_mask32(__mmask32 *A, __mmask32 a) {
  // CHECK-LABEL: @test_store_mask32
  // CHECK: store i32 %{{.*}}, ptr %{{.*}}
  _store_mask32(A, a);
}
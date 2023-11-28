// REQUIRES: intel_feature_isa_avx256p_unsupported
// RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-apple-darwin -target-feature +avx256p -emit-llvm -o - -Wall -Werror | FileCheck %s


#include <immintrin.h>

__mmask8 test_knot_mask8(__mmask8 a) {
  // CHECK-LABEL: @test_knot_mask8
  // CHECK: [[IN:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[NOT:%.*]] = xor <8 x i1> [[IN]], <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  return _knot_mask8(a);
}

__mmask8 test_kand_mask8(__mmask8 a, __mmask8 b) {
  // CHECK-LABEL: @test_kand_mask8
  // CHECK: [[LHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RES:%.*]] = and <8 x i1> [[LHS]], [[RHS]]
  return _kand_mask8(a, b);
}

__mmask8 test_kandn_mask8(__mmask8 a, __mmask8 b) {
  // CHECK-LABEL: @test_kandn_mask8
  // CHECK: [[LHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[NOT:%.*]] = xor <8 x i1> [[LHS]], <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  // CHECK: [[RES:%.*]] = and <8 x i1> [[NOT]], [[RHS]]
  return _kandn_mask8(a, b);
}

__mmask8 test_kor_mask8(__mmask8 a, __mmask8 b) {
  // CHECK-LABEL: @test_kor_mask8
  // CHECK: [[LHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RES:%.*]] = or <8 x i1> [[LHS]], [[RHS]]
  return _kor_mask8(a, b);
}

__mmask8 test_kxnor_mask8(__mmask8 a, __mmask8 b) {
  // CHECK-LABEL: @test_kxnor_mask8
  // CHECK: [[LHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[NOT:%.*]] = xor <8 x i1> [[LHS]], <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  // CHECK: [[RES:%.*]] = xor <8 x i1> [[NOT]], [[RHS]]
  return _kxnor_mask8(a, b);
}

__mmask8 test_kxor_mask8(__mmask8 a, __mmask8 b) {
  // CHECK-LABEL: @test_kxor_mask8
  // CHECK: [[LHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RES:%.*]] = xor <8 x i1> [[LHS]], [[RHS]]
  return _kxor_mask8(a, b);
}

unsigned char test_kortestz_mask8_u8(__mmask8 a, __mmask8 b) {
  // CHECK-LABEL: @test_kortestz_mask8_u8
  // CHECK: [[LHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[OR:%.*]] = or <8 x i1> [[LHS]], [[RHS]]
  // CHECK: [[CAST:%.*]] = bitcast <8 x i1> [[OR]] to i8
  // CHECK: [[CMP:%.*]] = icmp eq i8 [[CAST]], 0
  // CHECK: [[ZEXT:%.*]] = zext i1 [[CMP]] to i32
  // CHECK: trunc i32 [[ZEXT]] to i8
  return _kortestz_mask8_u8(a, b);
}

unsigned char test_kortestc_mask8_u8(__mmask8 a, __mmask8 b) {
  // CHECK-LABEL: @test_kortestc_mask8_u8
  // CHECK: [[LHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[OR:%.*]] = or <8 x i1> [[LHS]], [[RHS]]
  // CHECK: [[CAST:%.*]] = bitcast <8 x i1> [[OR]] to i8
  // CHECK: [[CMP:%.*]] = icmp eq i8 [[CAST]], -1
  // CHECK: [[ZEXT:%.*]] = zext i1 [[CMP]] to i32
  // CHECK: trunc i32 [[ZEXT]] to i8
  return _kortestc_mask8_u8(a, b);
}

unsigned char test_kortest_mask8_u8(__mmask8 a, __mmask8 b, unsigned char *CF) {
  // CHECK-LABEL: @test_kortest_mask8_u8
  // CHECK: [[LHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[OR:%.*]] = or <8 x i1> [[LHS]], [[RHS]]
  // CHECK: [[CAST:%.*]] = bitcast <8 x i1> [[OR]] to i8
  // CHECK: [[CMP:%.*]] = icmp eq i8 [[CAST]], -1
  // CHECK: [[ZEXT:%.*]] = zext i1 [[CMP]] to i32
  // CHECK: trunc i32 [[ZEXT]] to i8
  // CHECK: [[LHS2:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RHS2:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[OR2:%.*]] = or <8 x i1> [[LHS2]], [[RHS2]]
  // CHECK: [[CAST2:%.*]] = bitcast <8 x i1> [[OR2]] to i8
  // CHECK: [[CMP2:%.*]] = icmp eq i8 [[CAST2]], 0
  // CHECK: [[ZEXT2:%.*]] = zext i1 [[CMP2]] to i32
  // CHECK: trunc i32 [[ZEXT2]] to i8
  return _kortest_mask8_u8(a, b, CF);
}

unsigned char test_ktestz_mask8_u8(__mmask8 a, __mmask8 b) {
  // CHECK-LABEL: @test_ktestz_mask8_u8
  // CHECK: [[LHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RES:%.*]] = call i32 @llvm.x86.avx512.ktestz.b(<8 x i1> [[LHS]], <8 x i1> [[RHS]])
  // CHECK: trunc i32 [[RES]] to i8
  return _ktestz_mask8_u8(a, b);
}

unsigned char test_ktestc_mask8_u8(__mmask8 a, __mmask8 b) {
  // CHECK-LABEL: @test_ktestc_mask8_u8
  // CHECK: [[LHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RES:%.*]] = call i32 @llvm.x86.avx512.ktestc.b(<8 x i1> [[LHS]], <8 x i1> [[RHS]])
  // CHECK: trunc i32 [[RES]] to i8
  return _ktestc_mask8_u8(a, b);
}

unsigned char test_ktest_mask8_u8(__mmask8 a, __mmask8 b, unsigned char *CF) {
  // CHECK-LABEL: @test_ktest_mask8_u8
  // CHECK: [[LHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RES:%.*]] = call i32 @llvm.x86.avx512.ktestc.b(<8 x i1> [[LHS]], <8 x i1> [[RHS]])
  // CHECK: trunc i32 [[RES]] to i8
  // CHECK: [[LHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RES:%.*]] = call i32 @llvm.x86.avx512.ktestz.b(<8 x i1> [[LHS]], <8 x i1> [[RHS]])
  // CHECK: trunc i32 [[RES]] to i8
  return _ktest_mask8_u8(a, b, CF);
}

unsigned char test_ktestz_mask16_u8(__mmask16 a, __mmask16 b) {
  // CHECK-LABEL: @test_ktestz_mask16_u8
  // CHECK: [[LHS:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK: [[RES:%.*]] = call i32 @llvm.x86.avx512.ktestz.w(<16 x i1> [[LHS]], <16 x i1> [[RHS]])
  // CHECK: trunc i32 [[RES]] to i8
  return _ktestz_mask16_u8(a, b);
}

unsigned char test_ktestc_mask16_u8(__mmask16 a, __mmask16 b) {
  // CHECK-LABEL: @test_ktestc_mask16_u8
  // CHECK: [[LHS:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK: [[RES:%.*]] = call i32 @llvm.x86.avx512.ktestc.w(<16 x i1> [[LHS]], <16 x i1> [[RHS]])
  // CHECK: trunc i32 [[RES]] to i8
  return _ktestc_mask16_u8(a, b);
}

unsigned char test_ktest_mask16_u8(__mmask16 a, __mmask16 b, unsigned char *CF) {
  // CHECK-LABEL: @test_ktest_mask16_u8
  // CHECK: [[LHS:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK: [[RES:%.*]] = call i32 @llvm.x86.avx512.ktestc.w(<16 x i1> [[LHS]], <16 x i1> [[RHS]])
  // CHECK: trunc i32 [[RES]] to i8
  // CHECK: [[LHS:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK: [[RES:%.*]] = call i32 @llvm.x86.avx512.ktestz.w(<16 x i1> [[LHS]], <16 x i1> [[RHS]])
  // CHECK: trunc i32 [[RES]] to i8
  return _ktest_mask16_u8(a, b, CF);
}

__mmask8 test_kadd_mask8(__mmask8 a, __mmask8 b) {
  // CHECK-LABEL: @test_kadd_mask8
  // CHECK: [[LHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RES:%.*]] = call <8 x i1> @llvm.x86.avx512.kadd.b(<8 x i1> [[LHS]], <8 x i1> [[RHS]])
  return _kadd_mask8(a, b);
}

__mmask16 test_kadd_mask16(__mmask16 a, __mmask16 b) {
  // CHECK-LABEL: @test_kadd_mask16
  // CHECK: [[LHS:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK: [[RHS:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK: [[RES:%.*]] = call <16 x i1> @llvm.x86.avx512.kadd.w(<16 x i1> [[LHS]], <16 x i1> [[RHS]])
  return _kadd_mask16(a, b);
}

__mmask8 test_kshiftli_mask8(__mmask8 a) {
  // CHECK-LABEL: @test_kshiftli_mask8
  // CHECK: [[VAL:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RES:%.*]] = shufflevector <8 x i1> zeroinitializer, <8 x i1> [[VAL]], <8 x i32> <i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13>
  return _kshiftli_mask8(a, 2);
}

__mmask8 test_kshiftri_mask8(__mmask8 a) {
  // CHECK-LABEL: @test_kshiftri_mask8
  // CHECK: [[VAL:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK: [[RES:%.*]] = shufflevector <8 x i1> [[VAL]], <8 x i1> zeroinitializer, <8 x i32> <i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9>
  return _kshiftri_mask8(a, 2);
}

unsigned int test_cvtmask8_u32(__mmask8 a) {
  // CHECK-LABEL: @test_cvtmask8_u32
  // CHECK: zext i8 %{{.*}} to i32
  return _cvtmask8_u32(a);
}

__mmask8 test_cvtu32_mask8(unsigned int a) {
  // CHECK-LABEL: @test_cvtu32_mask8
  // CHECK: trunc i32 %{{.*}} to i8
  return _cvtu32_mask8(a);
}

__mmask8 test_load_mask8(__mmask8 *A) {
  // CHECK-LABEL: @test_load_mask8
  // CHECK: [[LOAD:%.*]] = load i8, ptr %{{.*}}
  return _load_mask8(A);
}

void test_store_mask8(__mmask8 *A, __mmask8 a) {
  // CHECK-LABEL: @test_store_mask8
  // CHECK: store i8 %{{.*}}, ptr %{{.*}}
  _store_mask8(A, a);
}

__m128d test_mm512_range_round_sd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm512_range_round_sd
  // CHECK: @llvm.x86.avx512.mask.range.sd
  return _mm_range_round_sd(__A, __B, 4, 8); 
}

__m128d test_mm512_mask_range_round_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: test_mm512_mask_range_round_sd
  // CHECK: @llvm.x86.avx512.mask.range.sd
  return _mm_mask_range_round_sd(__W, __U, __A, __B, 4, 8); 
}

__m128d test_mm512_maskz_range_round_sd(__mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm512_maskz_range_round_sd
  // CHECK: @llvm.x86.avx512.mask.range.sd
  return _mm_maskz_range_round_sd(__U, __A, __B, 4, 8); 
}

__m128 test_mm512_range_round_ss(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm512_range_round_ss
  // CHECK: @llvm.x86.avx512.mask.range.ss
  return _mm_range_round_ss(__A, __B, 4, 8); 
}

__m128 test_mm512_mask_range_round_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm512_mask_range_round_ss
  // CHECK: @llvm.x86.avx512.mask.range.ss
  return _mm_mask_range_round_ss(__W, __U, __A, __B, 4, 8); 
}

__m128 test_mm512_maskz_range_round_ss(__mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm512_maskz_range_round_ss
  // CHECK: @llvm.x86.avx512.mask.range.ss
  return _mm_maskz_range_round_ss(__U, __A, __B, 4, 8); 
}

__m128d test_mm_range_sd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_range_sd
  // CHECK: @llvm.x86.avx512.mask.range.sd
  return _mm_range_sd(__A, __B, 4); 
}

__m128d test_mm_mask_range_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: test_mm_mask_range_sd
  // CHECK: @llvm.x86.avx512.mask.range.sd
  return _mm_mask_range_sd(__W, __U, __A, __B, 4); 
}

__m128d test_mm_maskz_range_sd(__mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_maskz_range_sd
  // CHECK: @llvm.x86.avx512.mask.range.sd
  return _mm_maskz_range_sd(__U, __A, __B, 4); 
}

__m128 test_mm_range_ss(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_range_ss
  // CHECK: @llvm.x86.avx512.mask.range.ss
  return _mm_range_ss(__A, __B, 4); 
}

__m128 test_mm_mask_range_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_mask_range_ss
  // CHECK: @llvm.x86.avx512.mask.range.ss
  return _mm_mask_range_ss(__W, __U, __A, __B, 4); 
}

__m128 test_mm_maskz_range_ss(__mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_maskz_range_ss
  // CHECK: @llvm.x86.avx512.mask.range.ss
  return _mm_maskz_range_ss(__U, __A, __B, 4); 
}

__m128 test_mm_reduce_ss(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_reduce_ss
  // CHECK: @llvm.x86.avx512.mask.reduce.ss
  return _mm_reduce_ss(__A, __B, 4);
}

__m128 test_mm_mask_reduce_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_mask_reduce_ss
  // CHECK: @llvm.x86.avx512.mask.reduce.ss
  return _mm_mask_reduce_ss(__W, __U, __A, __B, 4);
}

__m128 test_mm_maskz_reduce_ss(__mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_maskz_reduce_ss
  // CHECK: @llvm.x86.avx512.mask.reduce.ss
  return _mm_maskz_reduce_ss(__U, __A, __B, 4);
}

__m128 test_mm_reduce_round_ss(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_reduce_round_ss
  // CHECK: @llvm.x86.avx512.mask.reduce.ss
  return _mm_reduce_round_ss(__A, __B, 4, 8);
}

__m128 test_mm_mask_reduce_round_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_mask_reduce_round_ss
  // CHECK: @llvm.x86.avx512.mask.reduce.ss
  return _mm_mask_reduce_round_ss(__W, __U, __A, __B, 4, 8);
}

__m128 test_mm_maskz_reduce_round_ss(__mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_maskz_reduce_round_ss
  // CHECK: @llvm.x86.avx512.mask.reduce.ss
  return _mm_maskz_reduce_round_ss(__U, __A, __B, 4, 8);
}

__m128d test_mm_reduce_sd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_reduce_sd
  // CHECK: @llvm.x86.avx512.mask.reduce.sd
  return _mm_reduce_sd(__A, __B, 4);
}

__m128d test_mm_mask_reduce_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_mask_reduce_sd
  // CHECK: @llvm.x86.avx512.mask.reduce.sd
  return _mm_mask_reduce_sd(__W, __U, __A, __B, 4);
}

__m128d test_mm_maskz_reduce_sd(__mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_maskz_reduce_sd
  // CHECK: @llvm.x86.avx512.mask.reduce.sd
  return _mm_maskz_reduce_sd(__U, __A, __B, 4);
}

__m128d test_mm_reduce_round_sd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_reduce_round_sd
  // CHECK: @llvm.x86.avx512.mask.reduce.sd
  return _mm_reduce_round_sd(__A, __B, 4, 8);
}

__m128d test_mm_mask_reduce_round_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_mask_reduce_round_sd
  // CHECK: @llvm.x86.avx512.mask.reduce.sd
  return _mm_mask_reduce_round_sd(__W, __U, __A, __B, 4, 8);
}

__m128d test_mm_maskz_reduce_round_sd(__mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_maskz_reduce_round_sd
  // CHECK: @llvm.x86.avx512.mask.reduce.sd
  return _mm_maskz_reduce_round_sd(__U, __A, __B, 4, 8);
}

__mmask8 test_mm_fpclass_sd_mask(__m128d __A)  { 
  // CHECK-LABEL: @test_mm_fpclass_sd_mask
  // CHECK: @llvm.x86.avx512.mask.fpclass.sd
 return _mm_fpclass_sd_mask (__A, 2);
}

__mmask8 test_mm_mask_fpclass_sd_mask(__mmask8 __U, __m128d __A)  {
 // CHECK-LABEL: @test_mm_mask_fpclass_sd_mask
 // CHECK: @llvm.x86.avx512.mask.fpclass.sd
 return _mm_mask_fpclass_sd_mask (__U,  __A, 2);
}

__mmask8 test_mm_fpclass_ss_mask(__m128 __A)  { 
 // CHECK-LABEL: @test_mm_fpclass_ss_mask
 // CHECK: @llvm.x86.avx512.mask.fpclass.ss
 return _mm_fpclass_ss_mask ( __A, 2);
}

__mmask8 test_mm_mask_fpclass_ss_mask(__mmask8 __U, __m128 __A)  {
 // CHECK-LABEL: @test_mm_mask_fpclass_ss_mask
 // CHECK: @llvm.x86.avx512.mask.fpclass.ss
 return _mm_mask_fpclass_ss_mask (__U, __A, 2);
}


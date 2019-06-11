// REQUIRES: intel_feature_isa_fp16
// RUN: %clang_cc1 -ffreestanding %s -triple=x86_64-unknown-unknown -target-feature +avx512vl -target-feature +avx512fp16 -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <immintrin.h>

_Float16 test_mm_cvtsh_h(__m128h __A) {
  // CHECK-LABEL: @test_mm_cvtsh_h
  // CHECK: extractelement <8 x half> %{{.*}}, i32 0
  return _mm_cvtsh_h(__A);
}

__m128h test_mm_set_sh(_Float16 __h)
{
  // CHECK-LABEL: @test_mm_set_sh
  // CHECK: insertelement <8 x half> {{.*}}, i32 0
  // CHECK: insertelement <8 x half> %{{.*}}, half 0xH0000, i32 1
  // CHECK: insertelement <8 x half> %{{.*}}, half 0xH0000, i32 2
  // CHECK: insertelement <8 x half> %{{.*}}, half 0xH0000, i32 3
  // CHECK: insertelement <8 x half> %{{.*}}, half 0xH0000, i32 4
  // CHECK: insertelement <8 x half> %{{.*}}, half 0xH0000, i32 5
  // CHECK: insertelement <8 x half> %{{.*}}, half 0xH0000, i32 6
  // CHECK: insertelement <8 x half> %{{.*}}, half 0xH0000, i32 7
  return _mm_set_sh(__h);
}

__m128h test_mm_set1_ph(_Float16 h)
{
  // CHECK-LABEL: @test_mm_set1_ph
  // CHECK: insertelement <8 x half> {{.*}}, i32 0
  // CHECK: insertelement <8 x half> {{.*}}, i32 1
  // CHECK: insertelement <8 x half> {{.*}}, i32 2
  // CHECK: insertelement <8 x half> {{.*}}, i32 3
  // CHECK: insertelement <8 x half> {{.*}}, i32 4
  // CHECK: insertelement <8 x half> {{.*}}, i32 5
  // CHECK: insertelement <8 x half> {{.*}}, i32 6
  // CHECK: insertelement <8 x half> {{.*}}, i32 7
  return _mm_set1_ph(h);
}

__m256h test_mm256_set1_ph(_Float16 h)
{
  // CHECK-LABEL: @test_mm256_set1_ph
  // CHECK: insertelement <16 x half> {{.*}}, i32 0
  // CHECK: insertelement <16 x half> {{.*}}, i32 1
  // CHECK: insertelement <16 x half> {{.*}}, i32 2
  // CHECK: insertelement <16 x half> {{.*}}, i32 3
  // CHECK: insertelement <16 x half> {{.*}}, i32 4
  // CHECK: insertelement <16 x half> {{.*}}, i32 5
  // CHECK: insertelement <16 x half> {{.*}}, i32 6
  // CHECK: insertelement <16 x half> {{.*}}, i32 7
  // CHECK: insertelement <16 x half> {{.*}}, i32 8
  // CHECK: insertelement <16 x half> {{.*}}, i32 9
  // CHECK: insertelement <16 x half> {{.*}}, i32 10
  // CHECK: insertelement <16 x half> {{.*}}, i32 11
  // CHECK: insertelement <16 x half> {{.*}}, i32 12
  // CHECK: insertelement <16 x half> {{.*}}, i32 13
  // CHECK: insertelement <16 x half> {{.*}}, i32 14
  // CHECK: insertelement <16 x half> {{.*}}, i32 15
  return _mm256_set1_ph(h);
}
__m256h test_mm256_add_ph(__m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_add_ph
  // CHECK: %{{.*}} = fadd <16 x half> %{{.*}}, %{{.*}}
  return _mm256_add_ph(__A, __B);
}

__m256h test_mm256_mask_add_ph(__m256h __W, __mmask32 __U, __m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_mask_add_ph
  // CHECK: %{{.*}} = fadd <16 x half> %{{.*}}, %{{.*}}
  // CHECK: select <16 x i1> %{{.*}}, <16 x half> %{{.*}}, <16 x half> %{{.*}}
  return (__m256h)_mm256_mask_add_ph(__W, __U, __A, __B);
}

__m256h test_mm256_maskz_add_ph(__mmask32 __U, __m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_maskz_add_ph
  // CHECK: %{{.*}} = fadd <16 x half> %{{.*}}, %{{.*}}
  // CHECK: select <16 x i1> %{{.*}}, <16 x half> %{{.*}}, <16 x half> %{{.*}}
  return _mm256_maskz_add_ph(__U, __A, __B);
}

__m128h test_mm_add_ph(__m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_add_ph
  // CHECK: %{{.*}} = fadd <8 x half> %{{.*}}, %{{.*}}
  return _mm_add_ph(__A, __B);
}

__m128h test_mm_mask_add_ph(__m128h __W, __mmask32 __U, __m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_mask_add_ph
  // CHECK: %{{.*}} = fadd <8 x half> %{{.*}}, %{{.*}}
  // CHECK: select <8 x i1> %{{.*}}, <8 x half> %{{.*}}, <8 x half> %{{.*}}
  return (__m128h)_mm_mask_add_ph(__W, __U, __A, __B);
}

__m128h test_mm_maskz_add_ph(__mmask32 __U, __m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_maskz_add_ph
  // CHECK: %{{.*}} = fadd <8 x half> %{{.*}}, %{{.*}}
  // CHECK: select <8 x i1> %{{.*}}, <8 x half> %{{.*}}, <8 x half> %{{.*}}
  return _mm_maskz_add_ph(__U, __A, __B);
}

__m256h test_mm256_sub_ph(__m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_sub_ph
  // CHECK: %{{.*}} = fsub <16 x half> %{{.*}}, %{{.*}}
  return _mm256_sub_ph(__A, __B);
}

__m256h test_mm256_mask_sub_ph(__m256h __W, __mmask32 __U, __m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_mask_sub_ph
  // CHECK: %{{.*}} = fsub <16 x half> %{{.*}}, %{{.*}}
  // CHECK: select <16 x i1> %{{.*}}, <16 x half> %{{.*}}, <16 x half> %{{.*}}
  return (__m256h)_mm256_mask_sub_ph(__W, __U, __A, __B);
}

__m256h test_mm256_maskz_sub_ph(__mmask32 __U, __m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_maskz_sub_ph
  // CHECK: %{{.*}} = fsub <16 x half> %{{.*}}, %{{.*}}
  // CHECK: select <16 x i1> %{{.*}}, <16 x half> %{{.*}}, <16 x half> %{{.*}}
  return _mm256_maskz_sub_ph(__U, __A, __B);
}

__m128h test_mm_sub_ph(__m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_sub_ph
  // CHECK: %{{.*}} = fsub <8 x half> %{{.*}}, %{{.*}}
  return _mm_sub_ph(__A, __B);
}

__m128h test_mm_mask_sub_ph(__m128h __W, __mmask32 __U, __m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_mask_sub_ph
  // CHECK: %{{.*}} = fsub <8 x half> %{{.*}}, %{{.*}}
  // CHECK: select <8 x i1> %{{.*}}, <8 x half> %{{.*}}, <8 x half> %{{.*}}
  return (__m128h)_mm_mask_sub_ph(__W, __U, __A, __B);
}

__m128h test_mm_maskz_sub_ph(__mmask32 __U, __m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_maskz_sub_ph
  // CHECK: %{{.*}} = fsub <8 x half> %{{.*}}, %{{.*}}
  // CHECK: select <8 x i1> %{{.*}}, <8 x half> %{{.*}}, <8 x half> %{{.*}}
  return _mm_maskz_sub_ph(__U, __A, __B);
}

__m256h test_mm256_mul_ph(__m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_mul_ph
  // CHECK: %{{.*}} = fmul <16 x half> %{{.*}}, %{{.*}}
  return _mm256_mul_ph(__A, __B);
}

__m256h test_mm256_mask_mul_ph(__m256h __W, __mmask32 __U, __m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_mask_mul_ph
  // CHECK: %{{.*}} = fmul <16 x half> %{{.*}}, %{{.*}}
  // CHECK: select <16 x i1> %{{.*}}, <16 x half> %{{.*}}, <16 x half> %{{.*}}
  return (__m256h)_mm256_mask_mul_ph(__W, __U, __A, __B);
}

__m256h test_mm256_maskz_mul_ph(__mmask32 __U, __m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_maskz_mul_ph
  // CHECK: %{{.*}} = fmul <16 x half> %{{.*}}, %{{.*}}
  // CHECK: select <16 x i1> %{{.*}}, <16 x half> %{{.*}}, <16 x half> %{{.*}}
  return _mm256_maskz_mul_ph(__U, __A, __B);
}

__m128h test_mm_mul_ph(__m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_mul_ph
  // CHECK: %{{.*}} = fmul <8 x half> %{{.*}}, %{{.*}}
  return _mm_mul_ph(__A, __B);
}

__m128h test_mm_mask_mul_ph(__m128h __W, __mmask32 __U, __m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_mask_mul_ph
  // CHECK: %{{.*}} = fmul <8 x half> %{{.*}}, %{{.*}}
  // CHECK: select <8 x i1> %{{.*}}, <8 x half> %{{.*}}, <8 x half> %{{.*}}
  return (__m128h)_mm_mask_mul_ph(__W, __U, __A, __B);
}

__m128h test_mm_maskz_mul_ph(__mmask32 __U, __m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_maskz_mul_ph
  // CHECK: %{{.*}} = fmul <8 x half> %{{.*}}, %{{.*}}
  // CHECK: select <8 x i1> %{{.*}}, <8 x half> %{{.*}}, <8 x half> %{{.*}}
  return _mm_maskz_mul_ph(__U, __A, __B);
}

__m256h test_mm256_div_ph(__m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_div_ph
  // CHECK: %{{.*}} = fdiv <16 x half> %{{.*}}, %{{.*}}
  return _mm256_div_ph(__A, __B);
}

__m256h test_mm256_mask_div_ph(__m256h __W, __mmask32 __U, __m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_mask_div_ph
  // CHECK: %{{.*}} = fdiv <16 x half> %{{.*}}, %{{.*}}
  // CHECK: select <16 x i1> %{{.*}}, <16 x half> %{{.*}}, <16 x half> %{{.*}}
  return (__m256h)_mm256_mask_div_ph(__W, __U, __A, __B);
}

__m256h test_mm256_maskz_div_ph(__mmask32 __U, __m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_maskz_div_ph
  // CHECK: %{{.*}} = fdiv <16 x half> %{{.*}}, %{{.*}}
  // CHECK: select <16 x i1> %{{.*}}, <16 x half> %{{.*}}, <16 x half> %{{.*}}
  return _mm256_maskz_div_ph(__U, __A, __B);
}

__m128h test_mm_div_ph(__m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_div_ph
  // CHECK: %{{.*}} = fdiv <8 x half> %{{.*}}, %{{.*}}
  return _mm_div_ph(__A, __B);
}

__m128h test_mm_mask_div_ph(__m128h __W, __mmask32 __U, __m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_mask_div_ph
  // CHECK: %{{.*}} = fdiv <8 x half> %{{.*}}, %{{.*}}
  // CHECK: select <8 x i1> %{{.*}}, <8 x half> %{{.*}}, <8 x half> %{{.*}}
  return (__m128h)_mm_mask_div_ph(__W, __U, __A, __B);
}

__m128h test_mm_maskz_div_ph(__mmask32 __U, __m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_maskz_div_ph
  // CHECK: %{{.*}} = fdiv <8 x half> %{{.*}}, %{{.*}}
  // CHECK: select <8 x i1> %{{.*}}, <8 x half> %{{.*}}, <8 x half> %{{.*}}
  return _mm_maskz_div_ph(__U, __A, __B);
}

__m256h test_mm256_min_ph(__m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_min_ph
  // CHECK: @llvm.x86.avx512fp16.min.ph.256
  return _mm256_min_ph(__A, __B);
}

__m256h test_mm256_mask_min_ph(__m256h __W, __mmask32 __U, __m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_mask_min_ph
  // CHECK: @llvm.x86.avx512fp16.min.ph.256
  return (__m256h)_mm256_mask_min_ph(__W, __U, __A, __B);
}

__m256h test_mm256_maskz_min_ph(__mmask32 __U, __m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_maskz_min_ph
  // CHECK: @llvm.x86.avx512fp16.min.ph.256
  return _mm256_maskz_min_ph(__U, __A, __B);
}

__m128h test_mm_min_ph(__m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_min_ph
  // CHECK: @llvm.x86.avx512fp16.min.ph.128
  return _mm_min_ph(__A, __B);
}

__m128h test_mm_mask_min_ph(__m128h __W, __mmask32 __U, __m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_mask_min_ph
  // CHECK: @llvm.x86.avx512fp16.min.ph.128
  return (__m128h)_mm_mask_min_ph(__W, __U, __A, __B);
}

__m128h test_mm_maskz_min_ph(__mmask32 __U, __m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_maskz_min_ph
  // CHECK: @llvm.x86.avx512fp16.min.ph.128
  return _mm_maskz_min_ph(__U, __A, __B);
}

__m256h test_mm256_max_ph(__m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_max_ph
  // CHECK: @llvm.x86.avx512fp16.max.ph.256
  return _mm256_max_ph(__A, __B);
}

__m256h test_mm256_mask_max_ph(__m256h __W, __mmask32 __U, __m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_mask_max_ph
  // CHECK: @llvm.x86.avx512fp16.max.ph.256
  return (__m256h)_mm256_mask_max_ph(__W, __U, __A, __B);
}

__m256h test_mm256_maskz_max_ph(__mmask32 __U, __m256h __A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_maskz_max_ph
  // CHECK: @llvm.x86.avx512fp16.max.ph.256
  return _mm256_maskz_max_ph(__U, __A, __B);
}

__m128h test_mm_max_ph(__m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_max_ph
  // CHECK: @llvm.x86.avx512fp16.max.ph.128
  return _mm_max_ph(__A, __B);
}

__m128h test_mm_mask_max_ph(__m128h __W, __mmask32 __U, __m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_mask_max_ph
  // CHECK: @llvm.x86.avx512fp16.max.ph.128
  return (__m128h)_mm_mask_max_ph(__W, __U, __A, __B);
}

__m128h test_mm_maskz_max_ph(__mmask32 __U, __m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_maskz_max_ph
  // CHECK: @llvm.x86.avx512fp16.max.ph.128
  return _mm_maskz_max_ph(__U, __A, __B);
}

__mmask16 test_mm256_cmp_ph_mask_eq_oq(__m256h a, __m256h b) {
  // CHECK-LABEL: @test_mm256_cmp_ph_mask_eq_oq
  // CHECK: fcmp oeq <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_EQ_OQ);
}

__mmask16 test_mm256_cmp_ph_mask_lt_os(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_lt_os
  // CHECK: fcmp olt <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_LT_OS);
}

__mmask16 test_mm256_cmp_ph_mask_le_os(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_le_os
  // CHECK: fcmp ole <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_LE_OS);
}

__mmask16 test_mm256_cmp_ph_mask_unord_q(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_unord_q
  // CHECK: fcmp uno <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_UNORD_Q);
}

__mmask16 test_mm256_cmp_ph_mask_neq_uq(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_neq_uq
  // CHECK: fcmp une <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_NEQ_UQ);
}

__mmask16 test_mm256_cmp_ph_mask_nlt_us(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_nlt_us
  // CHECK: fcmp uge <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_NLT_US);
}

__mmask16 test_mm256_cmp_ph_mask_nle_us(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_nle_us
  // CHECK: fcmp ugt <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_NLE_US);
}

__mmask16 test_mm256_cmp_ph_mask_ord_q(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_ord_q
  // CHECK: fcmp ord <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_ORD_Q);
}

__mmask16 test_mm256_cmp_ph_mask_eq_uq(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_eq_uq
  // CHECK: fcmp ueq <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_EQ_UQ);
}

__mmask16 test_mm256_cmp_ph_mask_nge_us(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_nge_us
  // CHECK: fcmp ult <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_NGE_US);
}

__mmask16 test_mm256_cmp_ph_mask_ngt_us(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_ngt_us
  // CHECK: fcmp ule <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_NGT_US);
}

__mmask16 test_mm256_cmp_ph_mask_false_oq(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_false_oq
  // CHECK: fcmp false <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_FALSE_OQ);
}

__mmask16 test_mm256_cmp_ph_mask_neq_oq(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_neq_oq
  // CHECK: fcmp one <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_NEQ_OQ);
}

__mmask16 test_mm256_cmp_ph_mask_ge_os(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_ge_os
  // CHECK: fcmp oge <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_GE_OS);
}

__mmask16 test_mm256_cmp_ph_mask_gt_os(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_gt_os
  // CHECK: fcmp ogt <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_GT_OS);
}

__mmask16 test_mm256_cmp_ph_mask_true_uq(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_true_uq
  // CHECK: fcmp true <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_TRUE_UQ);
}

__mmask16 test_mm256_cmp_ph_mask_eq_os(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_eq_os
  // CHECK: fcmp oeq <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_EQ_OS);
}

__mmask16 test_mm256_cmp_ph_mask_lt_oq(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_lt_oq
  // CHECK: fcmp olt <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_LT_OQ);
}

__mmask16 test_mm256_cmp_ph_mask_le_oq(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_le_oq
  // CHECK: fcmp ole <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_LE_OQ);
}

__mmask16 test_mm256_cmp_ph_mask_unord_s(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_unord_s
  // CHECK: fcmp uno <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_UNORD_S);
}

__mmask16 test_mm256_cmp_ph_mask_neq_us(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_neq_us
  // CHECK: fcmp une <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_NEQ_US);
}

__mmask16 test_mm256_cmp_ph_mask_nlt_uq(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_nlt_uq
  // CHECK: fcmp uge <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_NLT_UQ);
}

__mmask16 test_mm256_cmp_ph_mask_nle_uq(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_nle_uq
  // CHECK: fcmp ugt <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_NLE_UQ);
}

__mmask16 test_mm256_cmp_ph_mask_ord_s(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_ord_s
  // CHECK: fcmp ord <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_ORD_S);
}

__mmask16 test_mm256_cmp_ph_mask_eq_us(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_eq_us
  // CHECK: fcmp ueq <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_EQ_US);
}

__mmask16 test_mm256_cmp_ph_mask_nge_uq(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_nge_uq
  // CHECK: fcmp ult <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_NGE_UQ);
}

__mmask16 test_mm256_cmp_ph_mask_ngt_uq(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_ngt_uq
  // CHECK: fcmp ule <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_NGT_UQ);
}

__mmask16 test_mm256_cmp_ph_mask_false_os(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_false_os
  // CHECK: fcmp false <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_FALSE_OS);
}

__mmask16 test_mm256_cmp_ph_mask_neq_os(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_neq_os
  // CHECK: fcmp one <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_NEQ_OS);
}

__mmask16 test_mm256_cmp_ph_mask_ge_oq(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_ge_oq
  // CHECK: fcmp oge <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_GE_OQ);
}

__mmask16 test_mm256_cmp_ph_mask_gt_oq(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_gt_oq
  // CHECK: fcmp ogt <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_GT_OQ);
}

__mmask16 test_mm256_cmp_ph_mask_true_us(__m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_cmp_ph_mask_true_us
  // CHECK: fcmp true <16 x half> %{{.*}}, %{{.*}}
  return _mm256_cmp_ph_mask(a, b, _CMP_TRUE_US);
}

__mmask16 test_mm256_mask_cmp_ph_mask_eq_oq(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: @test_mm256_mask_cmp_ph_mask_eq_oq
  // CHECK: [[CMP:%.*]] = fcmp oeq <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_EQ_OQ);
}

__mmask16 test_mm256_mask_cmp_ph_mask_lt_os(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_lt_os
  // CHECK: [[CMP:%.*]] = fcmp olt <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_LT_OS);
}

__mmask16 test_mm256_mask_cmp_ph_mask_le_os(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_le_os
  // CHECK: [[CMP:%.*]] = fcmp ole <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_LE_OS);
}

__mmask16 test_mm256_mask_cmp_ph_mask_unord_q(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_unord_q
  // CHECK: [[CMP:%.*]] = fcmp uno <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_UNORD_Q);
}

__mmask16 test_mm256_mask_cmp_ph_mask_neq_uq(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_neq_uq
  // CHECK: [[CMP:%.*]] = fcmp une <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_NEQ_UQ);
}

__mmask16 test_mm256_mask_cmp_ph_mask_nlt_us(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_nlt_us
  // CHECK: [[CMP:%.*]] = fcmp uge <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_NLT_US);
}

__mmask16 test_mm256_mask_cmp_ph_mask_nle_us(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_nle_us
  // CHECK: [[CMP:%.*]] = fcmp ugt <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_NLE_US);
}

__mmask16 test_mm256_mask_cmp_ph_mask_ord_q(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_ord_q
  // CHECK: [[CMP:%.*]] = fcmp ord <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_ORD_Q);
}

__mmask16 test_mm256_mask_cmp_ph_mask_eq_uq(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_eq_uq
  // CHECK: [[CMP:%.*]] = fcmp ueq <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_EQ_UQ);
}

__mmask16 test_mm256_mask_cmp_ph_mask_nge_us(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_nge_us
  // CHECK: [[CMP:%.*]] = fcmp ult <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_NGE_US);
}

__mmask16 test_mm256_mask_cmp_ph_mask_ngt_us(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_ngt_us
  // CHECK: [[CMP:%.*]] = fcmp ule <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_NGT_US);
}

__mmask16 test_mm256_mask_cmp_ph_mask_false_oq(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_false_oq
  // CHECK: [[CMP:%.*]] = fcmp false <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_FALSE_OQ);
}

__mmask16 test_mm256_mask_cmp_ph_mask_neq_oq(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_neq_oq
  // CHECK: [[CMP:%.*]] = fcmp one <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_NEQ_OQ);
}

__mmask16 test_mm256_mask_cmp_ph_mask_ge_os(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_ge_os
  // CHECK: [[CMP:%.*]] = fcmp oge <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_GE_OS);
}

__mmask16 test_mm256_mask_cmp_ph_mask_gt_os(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_gt_os
  // CHECK: [[CMP:%.*]] = fcmp ogt <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_GT_OS);
}

__mmask16 test_mm256_mask_cmp_ph_mask_true_uq(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_true_uq
  // CHECK: [[CMP:%.*]] = fcmp true <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_TRUE_UQ);
}

__mmask16 test_mm256_mask_cmp_ph_mask_eq_os(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_eq_os
  // CHECK: [[CMP:%.*]] = fcmp oeq <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_EQ_OS);
}

__mmask16 test_mm256_mask_cmp_ph_mask_lt_oq(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_lt_oq
  // CHECK: [[CMP:%.*]] = fcmp olt <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_LT_OQ);
}

__mmask16 test_mm256_mask_cmp_ph_mask_le_oq(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_le_oq
  // CHECK: [[CMP:%.*]] = fcmp ole <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_LE_OQ);
}

__mmask16 test_mm256_mask_cmp_ph_mask_unord_s(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_unord_s
  // CHECK: [[CMP:%.*]] = fcmp uno <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_UNORD_S);
}

__mmask16 test_mm256_mask_cmp_ph_mask_neq_us(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_neq_us
  // CHECK: [[CMP:%.*]] = fcmp une <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_NEQ_US);
}

__mmask16 test_mm256_mask_cmp_ph_mask_nlt_uq(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_nlt_uq
  // CHECK: [[CMP:%.*]] = fcmp uge <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_NLT_UQ);
}

__mmask16 test_mm256_mask_cmp_ph_mask_nle_uq(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_nle_uq
  // CHECK: [[CMP:%.*]] = fcmp ugt <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_NLE_UQ);
}

__mmask16 test_mm256_mask_cmp_ph_mask_ord_s(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_ord_s
  // CHECK: [[CMP:%.*]] = fcmp ord <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_ORD_S);
}

__mmask16 test_mm256_mask_cmp_ph_mask_eq_us(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_eq_us
  // CHECK: [[CMP:%.*]] = fcmp ueq <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_EQ_US);
}

__mmask16 test_mm256_mask_cmp_ph_mask_nge_uq(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_nge_uq
  // CHECK: [[CMP:%.*]] = fcmp ult <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_NGE_UQ);
}

__mmask16 test_mm256_mask_cmp_ph_mask_ngt_uq(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_ngt_uq
  // CHECK: [[CMP:%.*]] = fcmp ule <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_NGT_UQ);
}

__mmask16 test_mm256_mask_cmp_ph_mask_false_os(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_false_os
  // CHECK: [[CMP:%.*]] = fcmp false <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_FALSE_OS);
}

__mmask16 test_mm256_mask_cmp_ph_mask_neq_os(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_neq_os
  // CHECK: [[CMP:%.*]] = fcmp one <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_NEQ_OS);
}

__mmask16 test_mm256_mask_cmp_ph_mask_ge_oq(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_ge_oq
  // CHECK: [[CMP:%.*]] = fcmp oge <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_GE_OQ);
}

__mmask16 test_mm256_mask_cmp_ph_mask_gt_oq(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_gt_oq
  // CHECK: [[CMP:%.*]] = fcmp ogt <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_GT_OQ);
}

__mmask16 test_mm256_mask_cmp_ph_mask_true_us(__mmask16 m, __m256h a, __m256h b) {
  // CHECK-LABEL: test_mm256_mask_cmp_ph_mask_true_us
  // CHECK: [[CMP:%.*]] = fcmp true <16 x half> %{{.*}}, %{{.*}}
  // CHECK: and <16 x i1> [[CMP]], {{.*}}
  return _mm256_mask_cmp_ph_mask(m, a, b, _CMP_TRUE_US);
}

__mmask8 test_mm_cmp_ph_mask_eq_oq(__m128h a, __m128h b) {
  // CHECK-LABEL: @test_mm_cmp_ph_mask_eq_oq
  // CHECK: fcmp oeq <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_EQ_OQ);
}

__mmask8 test_mm_cmp_ph_mask_lt_os(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_lt_os
  // CHECK: fcmp olt <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_LT_OS);
}

__mmask8 test_mm_cmp_ph_mask_le_os(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_le_os
  // CHECK: fcmp ole <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_LE_OS);
}

__mmask8 test_mm_cmp_ph_mask_unord_q(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_unord_q
  // CHECK: fcmp uno <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_UNORD_Q);
}

__mmask8 test_mm_cmp_ph_mask_neq_uq(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_neq_uq
  // CHECK: fcmp une <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_NEQ_UQ);
}

__mmask8 test_mm_cmp_ph_mask_nlt_us(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_nlt_us
  // CHECK: fcmp uge <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_NLT_US);
}

__mmask8 test_mm_cmp_ph_mask_nle_us(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_nle_us
  // CHECK: fcmp ugt <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_NLE_US);
}

__mmask8 test_mm_cmp_ph_mask_ord_q(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_ord_q
  // CHECK: fcmp ord <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_ORD_Q);
}

__mmask8 test_mm_cmp_ph_mask_eq_uq(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_eq_uq
  // CHECK: fcmp ueq <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_EQ_UQ);
}

__mmask8 test_mm_cmp_ph_mask_nge_us(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_nge_us
  // CHECK: fcmp ult <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_NGE_US);
}

__mmask8 test_mm_cmp_ph_mask_ngt_us(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_ngt_us
  // CHECK: fcmp ule <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_NGT_US);
}

__mmask8 test_mm_cmp_ph_mask_false_oq(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_false_oq
  // CHECK: fcmp false <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_FALSE_OQ);
}

__mmask8 test_mm_cmp_ph_mask_neq_oq(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_neq_oq
  // CHECK: fcmp one <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_NEQ_OQ);
}

__mmask8 test_mm_cmp_ph_mask_ge_os(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_ge_os
  // CHECK: fcmp oge <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_GE_OS);
}

__mmask8 test_mm_cmp_ph_mask_gt_os(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_gt_os
  // CHECK: fcmp ogt <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_GT_OS);
}

__mmask8 test_mm_cmp_ph_mask_true_uq(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_true_uq
  // CHECK: fcmp true <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_TRUE_UQ);
}

__mmask8 test_mm_cmp_ph_mask_eq_os(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_eq_os
  // CHECK: fcmp oeq <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_EQ_OS);
}

__mmask8 test_mm_cmp_ph_mask_lt_oq(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_lt_oq
  // CHECK: fcmp olt <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_LT_OQ);
}

__mmask8 test_mm_cmp_ph_mask_le_oq(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_le_oq
  // CHECK: fcmp ole <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_LE_OQ);
}

__mmask8 test_mm_cmp_ph_mask_unord_s(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_unord_s
  // CHECK: fcmp uno <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_UNORD_S);
}

__mmask8 test_mm_cmp_ph_mask_neq_us(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_neq_us
  // CHECK: fcmp une <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_NEQ_US);
}

__mmask8 test_mm_cmp_ph_mask_nlt_uq(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_nlt_uq
  // CHECK: fcmp uge <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_NLT_UQ);
}

__mmask8 test_mm_cmp_ph_mask_nle_uq(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_nle_uq
  // CHECK: fcmp ugt <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_NLE_UQ);
}

__mmask8 test_mm_cmp_ph_mask_ord_s(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_ord_s
  // CHECK: fcmp ord <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_ORD_S);
}

__mmask8 test_mm_cmp_ph_mask_eq_us(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_eq_us
  // CHECK: fcmp ueq <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_EQ_US);
}

__mmask8 test_mm_cmp_ph_mask_nge_uq(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_nge_uq
  // CHECK: fcmp ult <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_NGE_UQ);
}

__mmask8 test_mm_cmp_ph_mask_ngt_uq(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_ngt_uq
  // CHECK: fcmp ule <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_NGT_UQ);
}

__mmask8 test_mm_cmp_ph_mask_false_os(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_false_os
  // CHECK: fcmp false <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_FALSE_OS);
}

__mmask8 test_mm_cmp_ph_mask_neq_os(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_neq_os
  // CHECK: fcmp one <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_NEQ_OS);
}

__mmask8 test_mm_cmp_ph_mask_ge_oq(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_ge_oq
  // CHECK: fcmp oge <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_GE_OQ);
}

__mmask8 test_mm_cmp_ph_mask_gt_oq(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_gt_oq
  // CHECK: fcmp ogt <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_GT_OQ);
}

__mmask8 test_mm_cmp_ph_mask_true_us(__m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_cmp_ph_mask_true_us
  // CHECK: fcmp true <8 x half> %{{.*}}, %{{.*}}
  return _mm_cmp_ph_mask(a, b, _CMP_TRUE_US);
}

__mmask8 test_mm_mask_cmp_ph_mask_eq_oq(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: @test_mm_mask_cmp_ph_mask_eq_oq
  // CHECK: [[CMP:%.*]] = fcmp oeq <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_EQ_OQ);
}

__mmask8 test_mm_mask_cmp_ph_mask_lt_os(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_lt_os
  // CHECK: [[CMP:%.*]] = fcmp olt <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_LT_OS);
}

__mmask8 test_mm_mask_cmp_ph_mask_le_os(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_le_os
  // CHECK: [[CMP:%.*]] = fcmp ole <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_LE_OS);
}

__mmask8 test_mm_mask_cmp_ph_mask_unord_q(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_unord_q
  // CHECK: [[CMP:%.*]] = fcmp uno <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_UNORD_Q);
}

__mmask8 test_mm_mask_cmp_ph_mask_neq_uq(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_neq_uq
  // CHECK: [[CMP:%.*]] = fcmp une <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_NEQ_UQ);
}

__mmask8 test_mm_mask_cmp_ph_mask_nlt_us(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_nlt_us
  // CHECK: [[CMP:%.*]] = fcmp uge <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_NLT_US);
}

__mmask8 test_mm_mask_cmp_ph_mask_nle_us(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_nle_us
  // CHECK: [[CMP:%.*]] = fcmp ugt <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_NLE_US);
}

__mmask8 test_mm_mask_cmp_ph_mask_ord_q(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_ord_q
  // CHECK: [[CMP:%.*]] = fcmp ord <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_ORD_Q);
}

__mmask8 test_mm_mask_cmp_ph_mask_eq_uq(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_eq_uq
  // CHECK: [[CMP:%.*]] = fcmp ueq <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_EQ_UQ);
}

__mmask8 test_mm_mask_cmp_ph_mask_nge_us(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_nge_us
  // CHECK: [[CMP:%.*]] = fcmp ult <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_NGE_US);
}

__mmask8 test_mm_mask_cmp_ph_mask_ngt_us(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_ngt_us
  // CHECK: [[CMP:%.*]] = fcmp ule <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_NGT_US);
}

__mmask8 test_mm_mask_cmp_ph_mask_false_oq(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_false_oq
  // CHECK: [[CMP:%.*]] = fcmp false <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_FALSE_OQ);
}

__mmask8 test_mm_mask_cmp_ph_mask_neq_oq(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_neq_oq
  // CHECK: [[CMP:%.*]] = fcmp one <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_NEQ_OQ);
}

__mmask8 test_mm_mask_cmp_ph_mask_ge_os(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_ge_os
  // CHECK: [[CMP:%.*]] = fcmp oge <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_GE_OS);
}

__mmask8 test_mm_mask_cmp_ph_mask_gt_os(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_gt_os
  // CHECK: [[CMP:%.*]] = fcmp ogt <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_GT_OS);
}

__mmask8 test_mm_mask_cmp_ph_mask_true_uq(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_true_uq
  // CHECK: [[CMP:%.*]] = fcmp true <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_TRUE_UQ);
}

__mmask8 test_mm_mask_cmp_ph_mask_eq_os(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_eq_os
  // CHECK: [[CMP:%.*]] = fcmp oeq <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_EQ_OS);
}

__mmask8 test_mm_mask_cmp_ph_mask_lt_oq(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_lt_oq
  // CHECK: [[CMP:%.*]] = fcmp olt <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_LT_OQ);
}

__mmask8 test_mm_mask_cmp_ph_mask_le_oq(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_le_oq
  // CHECK: [[CMP:%.*]] = fcmp ole <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_LE_OQ);
}

__mmask8 test_mm_mask_cmp_ph_mask_unord_s(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_unord_s
  // CHECK: [[CMP:%.*]] = fcmp uno <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_UNORD_S);
}

__mmask8 test_mm_mask_cmp_ph_mask_neq_us(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_neq_us
  // CHECK: [[CMP:%.*]] = fcmp une <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_NEQ_US);
}

__mmask8 test_mm_mask_cmp_ph_mask_nlt_uq(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_nlt_uq
  // CHECK: [[CMP:%.*]] = fcmp uge <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_NLT_UQ);
}

__mmask8 test_mm_mask_cmp_ph_mask_nle_uq(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_nle_uq
  // CHECK: [[CMP:%.*]] = fcmp ugt <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_NLE_UQ);
}

__mmask8 test_mm_mask_cmp_ph_mask_ord_s(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_ord_s
  // CHECK: [[CMP:%.*]] = fcmp ord <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_ORD_S);
}

__mmask8 test_mm_mask_cmp_ph_mask_eq_us(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_eq_us
  // CHECK: [[CMP:%.*]] = fcmp ueq <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_EQ_US);
}

__mmask8 test_mm_mask_cmp_ph_mask_nge_uq(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_nge_uq
  // CHECK: [[CMP:%.*]] = fcmp ult <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_NGE_UQ);
}

__mmask8 test_mm_mask_cmp_ph_mask_ngt_uq(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_ngt_uq
  // CHECK: [[CMP:%.*]] = fcmp ule <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_NGT_UQ);
}

__mmask8 test_mm_mask_cmp_ph_mask_false_os(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_false_os
  // CHECK: [[CMP:%.*]] = fcmp false <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_FALSE_OS);
}

__mmask8 test_mm_mask_cmp_ph_mask_neq_os(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_neq_os
  // CHECK: [[CMP:%.*]] = fcmp one <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_NEQ_OS);
}

__mmask8 test_mm_mask_cmp_ph_mask_ge_oq(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_ge_oq
  // CHECK: [[CMP:%.*]] = fcmp oge <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_GE_OQ);
}

__mmask8 test_mm_mask_cmp_ph_mask_gt_oq(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_gt_oq
  // CHECK: [[CMP:%.*]] = fcmp ogt <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_GT_OQ);
}

__mmask8 test_mm_mask_cmp_ph_mask_true_us(__mmask8 m, __m128h a, __m128h b) {
  // CHECK-LABEL: test_mm_mask_cmp_ph_mask_true_us
  // CHECK: [[CMP:%.*]] = fcmp true <8 x half> %{{.*}}, %{{.*}}
  // CHECK: and <8 x i1> [[CMP]], {{.*}}
  return _mm_mask_cmp_ph_mask(m, a, b, _CMP_TRUE_US);
}

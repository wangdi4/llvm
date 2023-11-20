// REQUIRES: intel_feature_isa_avx256p_unsupported
//  RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-apple-darwin \
//  RUN:            -target-feature +avx256p -emit-llvm -o - -Wall -Werror \
//  RUN:            | FileCheck %s

#include <immintrin.h>

float test_mm_cvtsbh_ss(__bf16 A) {
  // CHECK-LABEL: @test_mm_cvtsbh_ss
  // CHECK: fpext bfloat %{{.*}} to float
  // CHECK: ret float %{{.*}}
  return _mm_cvtsbh_ss(A);
}
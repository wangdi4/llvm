// RUN: %clang_cc1 -fexperimental-new-pass-manager -ffreestanding %s -O0 -triple=x86_64-apple-darwin -target-cpu skylake-avx512 -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <immintrin.h>

short test_mm_reduce_max_epi16(__m128i __W){
// CHECK-LABEL: test_mm_reduce_max_epi16
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp sgt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp sgt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp sgt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  extractelement <8 x i16> %{{.*}}, i32 0
  return _mm_reduce_max_epi16(__W);
}

short test_mm_reduce_min_epi16(__m128i __W){
// CHECK-LABEL: test_mm_reduce_min_epi16
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp slt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp slt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp slt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  extractelement <8 x i16> %{{.*}}, i32 0
  return _mm_reduce_min_epi16(__W);
}

unsigned short test_mm_reduce_max_epu16(__m128i __W){
// CHECK-LABEL: test_mm_reduce_max_epu16
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ugt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ugt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ugt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  extractelement <8 x i16> %{{.*}}, i32 0
  return _mm_reduce_max_epu16(__W);
}

unsigned short test_mm_reduce_min_epu16(__m128i __W){
// CHECK-LABEL: test_mm_reduce_min_epu16
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ult <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ult <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ult <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  extractelement <8 x i16> %{{.*}}, i32 0
  return _mm_reduce_min_epu16(__W);
}

short test_mm_mask_reduce_max_epi16(__mmask8 __M, __m128i __W){
// CHECK-LABEL: test_mm_mask_reduce_max_epi16
// CHECK:  store i16 -32768, i16* %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp sgt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp sgt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp sgt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  extractelement <8 x i16> %{{.*}}, i32 0
  return _mm_mask_reduce_max_epi16(__M, __W);
}

short test_mm_mask_reduce_min_epi16(__mmask8 __M, __m128i __W){
// CHECK-LABEL: test_mm_mask_reduce_min_epi16
// CHECK:  store i16 32767, i16* %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp slt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp slt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp slt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  extractelement <8 x i16> %{{.*}}, i32 0
  return _mm_mask_reduce_min_epi16(__M, __W);
}

unsigned short test_mm_mask_reduce_max_epu16(__mmask8 __M, __m128i __W){
// CHECK-LABEL: test_mm_mask_reduce_max_epu16
// CHECK:  store <2 x i64> zeroinitializer, <2 x i64>* %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ugt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ugt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ugt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  extractelement <8 x i16> %{{.*}}, i32 0
  return _mm_mask_reduce_max_epu16(__M, __W);
}

unsigned short test_mm_mask_reduce_min_epu16(__mmask8 __M, __m128i __W){
// CHECK-LABEL: test_mm_mask_reduce_min_epu16
// CHECK:  store i16 -1, i16* %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ult <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ult <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ult <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  extractelement <8 x i16> %{{.*}}, i32 0
  return _mm_mask_reduce_min_epu16(__M, __W);
}

short test_mm256_reduce_max_epi16(__m256i __W){
// CHECK-LABEL: test_mm256_reduce_max_epi16
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:  icmp sgt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp sgt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp sgt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp sgt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  extractelement <8 x i16> %{{.*}}, i32 0
  return _mm256_reduce_max_epi16(__W);
}

short test_mm256_reduce_min_epi16(__m256i __W){
// CHECK-LABEL: test_mm256_reduce_min_epi16
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:  icmp slt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp slt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp slt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp slt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  extractelement <8 x i16> %{{.*}}, i32 0
  return _mm256_reduce_min_epi16(__W);
}

unsigned short test_mm256_reduce_max_epu16(__m256i __W){
// CHECK-LABEL: test_mm256_reduce_max_epu16
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:  icmp ugt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ugt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ugt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ugt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  extractelement <8 x i16> %{{.*}}, i32 0
  return _mm256_reduce_max_epu16(__W);
}

short test_mm256_mask_reduce_max_epi16(__mmask16 __M, __m256i __W){
// CHECK-LABEL: test_mm256_mask_reduce_max_epi16
// CHECK:  store i16 -32768, i16* %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i16> %{{.*}}, <16 x i16> %{{.*}}
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:  icmp sgt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp sgt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp sgt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp sgt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  extractelement <8 x i16> %{{.*}}, i32 0
  return _mm256_mask_reduce_max_epi16(__M, __W);
}

short test_mm256_mask_reduce_min_epi16(__mmask16 __M, __m256i __W){
// CHECK-LABEL: test_mm256_mask_reduce_min_epi16
// CHECK:  store i16 32767, i16* %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i16> %{{.*}}, <16 x i16> %{{.*}}
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:  icmp slt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp slt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp slt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp slt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  extractelement <8 x i16> %{{.*}}, i32 0
  return _mm256_mask_reduce_min_epi16(__M, __W);
}

unsigned short test_mm256_mask_reduce_max_epu16(__mmask16 __M, __m256i __W){
// CHECK-LABEL: test_mm256_mask_reduce_max_epu16
// CHECK:  store <4 x i64> zeroinitializer, <4 x i64>* %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i16> %{{.*}}, <16 x i16> %{{.*}}
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:  icmp ugt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ugt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ugt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ugt <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  extractelement <8 x i16> %{{.*}}, i32 0
  return _mm256_mask_reduce_max_epu16(__M, __W);
}

unsigned short test_mm256_mask_reduce_min_epu16(__mmask16 __M, __m256i __W){
// CHECK-LABEL: test_mm256_mask_reduce_min_epu16
// CHECK:  store i16 -1, i16* %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i16> %{{.*}}, <16 x i16> %{{.*}}
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:  icmp ult <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ult <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ult <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ult <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  extractelement <8 x i16> %{{.*}}, i32 0
  return _mm256_mask_reduce_min_epu16(__M, __W);
}

unsigned short test_mm256_reduce_min_epu16(__m256i __W){
// CHECK-LABEL: test_mm256_reduce_min_epu16
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:  icmp ult <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ult <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ult <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:  icmp ult <8 x i16> %{{.*}}, %{{.*}}
// CHECK:  select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:  extractelement <8 x i16> %{{.*}}, i32 0
  return _mm256_reduce_min_epu16(__W);
}

signed char test_mm_reduce_max_epi8(__m128i __W){
// CHECK-LABEL: test_mm_reduce_max_epi8
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp sgt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp sgt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp sgt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp sgt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  extractelement <16 x i8> %{{.*}}, i32 0
  return _mm_reduce_max_epi8(__W);
}

signed char test_mm_reduce_min_epi8(__m128i __W){
// CHECK-LABEL: test_mm_reduce_min_epi8
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp slt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp slt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp slt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp slt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  extractelement <16 x i8> %{{.*}}, i32 0
  return _mm_reduce_min_epi8(__W);
}

unsigned char test_mm_reduce_max_epu8(__m128i __W){
// CHECK-LABEL: test_mm_reduce_max_epu8
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ugt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ugt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ugt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ugt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  extractelement <16 x i8> %{{.*}}, i32 0
  return _mm_reduce_max_epu8(__W);
}

unsigned char test_mm_reduce_min_epu8(__m128i __W){
// CHECK-LABEL: test_mm_reduce_min_epu8
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ult <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ult <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ult <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ult <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  extractelement <16 x i8> %{{.*}}, i32 0
  return _mm_reduce_min_epu8(__W);
}

signed char test_mm_mask_reduce_max_epi8(__mmask16 __M, __m128i __W){
// CHECK-LABEL: test_mm_mask_reduce_max_epi8
// CHECK:  store i8 -128, i8* %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp sgt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp sgt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp sgt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp sgt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  extractelement <16 x i8> %{{.*}}, i32 0
  return _mm_mask_reduce_max_epi8(__M, __W);
}

signed char test_mm_mask_reduce_min_epi8(__mmask16 __M, __m128i __W){
// CHECK-LABEL: test_mm_mask_reduce_min_epi8
// CHECK:  store i8 127, i8* %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp slt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp slt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp slt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp slt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  extractelement <16 x i8> %{{.*}}, i32 0
  return _mm_mask_reduce_min_epi8(__M, __W);
}

unsigned char test_mm_mask_reduce_max_epu8(__mmask16 __M, __m128i __W){
// CHECK-LABEL: test_mm_mask_reduce_max_epu8
// CHECK:  store <2 x i64> zeroinitializer, <2 x i64>* %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ugt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ugt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ugt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ugt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  extractelement <16 x i8> %{{.*}}, i32 0
  return _mm_mask_reduce_max_epu8(__M, __W);
}

unsigned char test_mm_mask_reduce_min_epu8(__mmask16 __M, __m128i __W){
// CHECK-LABEL: test_mm_mask_reduce_min_epu8
// CHECK:  store i8 -1, i8* %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ult <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ult <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ult <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ult <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  extractelement <16 x i8> %{{.*}}, i32 0
  return _mm_mask_reduce_min_epu8(__M, __W);
}

signed char test_mm256_reduce_max_epi8(__m256i __W){
// CHECK-LABEL: test_mm256_reduce_max_epi8
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:  icmp sgt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp sgt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp sgt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp sgt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp sgt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  extractelement <16 x i8> %{{.*}}, i32 0
  return _mm256_reduce_max_epi8(__W);
}

signed char test_mm256_reduce_min_epi8(__m256i __W){
// CHECK-LABEL: test_mm256_reduce_min_epi8
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:  icmp slt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp slt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp slt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp slt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp slt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  extractelement <16 x i8> %{{.*}}, i32 0
  return _mm256_reduce_min_epi8(__W);
}

unsigned char test_mm256_reduce_max_epu8(__m256i __W){
// CHECK-LABEL: test_mm256_reduce_max_epu8
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:  icmp ugt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ugt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ugt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ugt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ugt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  extractelement <16 x i8> %{{.*}}, i32 0
  return _mm256_reduce_max_epu8(__W);
}

unsigned char test_mm256_reduce_min_epu8(__m256i __W){
// CHECK-LABEL: test_mm256_reduce_min_epu8
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:  icmp ult <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ult <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ult <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ult <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ult <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  extractelement <16 x i8> %{{.*}}, i32 0
  return _mm256_reduce_min_epu8(__W);
}

signed char test_mm256_mask_reduce_max_epi8(__mmask32 __M, __m256i __W){
// CHECK-LABEL: test_mm256_mask_reduce_max_epi8
// CHECK:  store i8 -128, i8* %{{.*}}
// CHECK:  select <32 x i1> %{{.*}}, <32 x i8> %{{.*}}, <32 x i8> %{{.*}}
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:  icmp sgt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp sgt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp sgt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp sgt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp sgt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  extractelement <16 x i8> %{{.*}}, i32 0
  return _mm256_mask_reduce_max_epi8(__M, __W);
}

signed char test_mm256_mask_reduce_min_epi8(__mmask32 __M, __m256i __W){
// CHECK-LABEL: test_mm256_mask_reduce_min_epi8
// CHECK:  store i8 127, i8* %{{.*}}
// CHECK:  select <32 x i1> %{{.*}}, <32 x i8> %{{.*}}, <32 x i8> %{{.*}}
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:  icmp slt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp slt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp slt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp slt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp slt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  extractelement <16 x i8> %{{.*}}, i32 0
  return _mm256_mask_reduce_min_epi8(__M, __W);
}

unsigned char test_mm256_mask_reduce_max_epu8(__mmask32 __M, __m256i __W){
// CHECK-LABEL: test_mm256_mask_reduce_max_epu8
// CHECK:  store <4 x i64> zeroinitializer, <4 x i64>* %{{.*}}
// CHECK:  select <32 x i1> %{{.*}}, <32 x i8> %{{.*}}, <32 x i8> %{{.*}}
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:  icmp ugt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ugt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ugt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ugt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ugt <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  extractelement <16 x i8> %{{.*}}, i32 0
  return _mm256_mask_reduce_max_epu8(__M, __W);
}

unsigned char test_mm256_mask_reduce_min_epu8(__mmask32 __M, __m256i __W){
// CHECK-LABEL: test_mm256_mask_reduce_min_epu8
// CHECK:  store i8 -1, i8* %{{.*}}
// CHECK:  select <32 x i1> %{{.*}}, <32 x i8> %{{.*}}, <32 x i8> %{{.*}}
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:  shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:  icmp ult <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ult <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ult <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ult <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:  icmp ult <16 x i8> %{{.*}}, %{{.*}}
// CHECK:  select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:  extractelement <16 x i8> %{{.*}}, i32 0
  return _mm256_mask_reduce_min_epu8(__M, __W);
}

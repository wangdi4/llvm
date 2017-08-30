// FIXME: We should not be testing with -O2 (ie, a dependency on the entire IR optimizer).

// RUN: %clang_cc1 -ffreestanding %s -O2 -triple=x86_64-apple-darwin -target-cpu skylake-avx512 -emit-llvm -o - -Wall -Werror |opt -instnamer -S |FileCheck %s

#include <immintrin.h>

long long test_mm512_reduce_max_epi64(__m512i __W){
  // CHECK: %shuffle1.i = shufflevector <8 x i64> %__W, <8 x i64> undef, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef>
  // CHECK: [[T:%.+]] = icmp slt <8 x i64> %shuffle1.i, %__W ;INTEL
  // CHECK: [[T1:%.+]] = select <8 x i1> [[T]], <8 x i64> %__W, <8 x i64> %shuffle1.i ;INTEL
  // CHECK: %shuffle3.i = shufflevector <8 x i64> [[T1]], <8 x i64> undef, <8 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T2:%.+]] = icmp sgt <8 x i64> [[T1]], %shuffle3.i ;INTEL
  // CHECK: [[T3:%.+]] = select <8 x i1> [[T2]], <8 x i64> [[T1]], <8 x i64> %shuffle3.i ;INTEL
  // CHECK: %shuffle6.i = shufflevector <8 x i64> [[T3]], <8 x i64> undef, <8 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T4:%.+]] = icmp sgt <8 x i64> [[T3]], %shuffle6.i ;INTEL
  // CHECK: %.elt.i = extractelement <8 x i1> [[T4]], i32 0 ;INTEL
  // CHECK: %.elt20.i = extractelement <8 x i64> [[T3]], i32 0 ;INTEL
  // CHECK: %shuffle6.elt.i = extractelement <8 x i64> [[T3]], i32 1 ;INTEL
  // CHECK: %vecext.i = select i1 %.elt.i, i64 %.elt20.i, i64 %shuffle6.elt.i
  // CHECK: ret i64 %vecext.i
  return _mm512_reduce_max_epi64(__W);
}

unsigned long long test_mm512_reduce_max_epu64(__m512i __W){
  // CHECK: %shuffle1.i = shufflevector <8 x i64> %__W, <8 x i64> undef, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef>
  // CHECK: [[T:%.+]] = icmp ult <8 x i64> %shuffle1.i, %__W ;INTEL
  // CHECK: [[T1:%.+]] = select <8 x i1> [[T]], <8 x i64> %__W, <8 x i64> %shuffle1.i ;INTEL
  // CHECK: %shuffle3.i = shufflevector <8 x i64> [[T1]], <8 x i64> undef, <8 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T2:%.+]] = icmp ugt <8 x i64> [[T1]], %shuffle3.i ;INTEL
  // CHECK: [[T3:%.+]] = select <8 x i1> [[T2]], <8 x i64> [[T1]], <8 x i64> %shuffle3.i ;INTEL
  // CHECK: %shuffle6.i = shufflevector <8 x i64> [[T3]], <8 x i64> undef, <8 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T4:%.+]] = icmp ugt <8 x i64> [[T3]], %shuffle6.i ;INTEL
  // CHECK: %.elt.i = extractelement <8 x i1> [[T4]], i32 0 ;INTEL
  // CHECK: %.elt20.i = extractelement <8 x i64> [[T3]], i32 0 ;INTEL
  // CHECK: %shuffle6.elt.i = extractelement <8 x i64> [[T3]], i32 1 ;INTEL
  // CHECK: %vecext.i = select i1 %.elt.i, i64 %.elt20.i, i64 %shuffle6.elt.i
  // CHECK: ret i64 %vecext.i
  return _mm512_reduce_max_epu64(__W); 
}

double test_mm512_reduce_max_pd(__m512d __W){
  // CHECK: %shuffle1.i = shufflevector <8 x double> %__W, <8 x double> undef, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef>
  // CHECK: [[T:%.+]] = tail call <8 x double> @llvm.x86.avx512.mask.max.pd.512(<8 x double> %__W, <8 x double> %shuffle1.i, <8 x double> zeroinitializer, i8 -1, i32 4) ;INTEL
  // CHECK: %shuffle3.i = shufflevector <8 x double> [[T]], <8 x double> undef, <8 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T1:%.+]] = tail call <8 x double> @llvm.x86.avx512.mask.max.pd.512(<8 x double> [[T]], <8 x double> %shuffle3.i, <8 x double> zeroinitializer, i8 -1, i32 4) ;INTEL
  // CHECK: %shuffle6.i = shufflevector <8 x double> [[T1]], <8 x double> undef, <8 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T2:%.+]] = tail call <8 x double> @llvm.x86.avx512.mask.max.pd.512(<8 x double> [[T1]], <8 x double> %shuffle6.i, <8 x double> zeroinitializer, i8 -1, i32 4) ;INTEL
  // CHECK: %vecext.i = extractelement <8 x double> [[T2]], i32 0 ;INTEL
  // CHECK: ret double %vecext.i
  return _mm512_reduce_max_pd(__W); 
}

long long test_mm512_reduce_min_epi64(__m512i __W){
  // CHECK: %shuffle1.i = shufflevector <8 x i64> %__W, <8 x i64> undef, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef>
  // CHECK: [[T:%.+]] = icmp slt <8 x i64> %shuffle1.i, %__W ;INTEL
  // CHECK: [[T1:%.+]] = select <8 x i1> [[T]], <8 x i64> %__W, <8 x i64> %shuffle1.i ;INTEL
  // CHECK: %shuffle3.i = shufflevector <8 x i64> [[T1]], <8 x i64> undef, <8 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T2:%.+]] = icmp sgt <8 x i64> [[T1]], %shuffle3.i ;INTEL
  // CHECK: [[T3:%.+]] = select <8 x i1> [[T2]], <8 x i64> [[T1]], <8 x i64> %shuffle3.i ;INTEL
  // CHECK: %shuffle6.i = shufflevector <8 x i64> [[T3]], <8 x i64> undef, <8 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T4:%.+]] = icmp sgt <8 x i64> [[T3]], %shuffle6.i ;INTEL
  // CHECK: %.elt.i = extractelement <8 x i1> [[T4]], i32 0 ;INTEL
  // CHECK: %.elt20.i = extractelement <8 x i64> [[T3]], i32 0 ;INTEL
  // CHECK: %shuffle6.elt.i = extractelement <8 x i64> [[T3]], i32 1 ;INTEL
  // CHECK: %vecext.i = select i1 %.elt.i, i64 %.elt20.i, i64 %shuffle6.elt.i
  // CHECK: ret i64 %vecext.i
  return _mm512_reduce_max_epi64(__W);
}

unsigned long long test_mm512_reduce_min_epu64(__m512i __W){
  // CHECK: %shuffle1.i = shufflevector <8 x i64> %__W, <8 x i64> undef, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef>
  // CHECK: [[T:%.+]] = icmp ult <8 x i64> %shuffle1.i, %__W ;INTEL
  // CHECK: [[T1:%.+]] = select <8 x i1> [[T]], <8 x i64> %__W, <8 x i64> %shuffle1.i ;INTEL
  // CHECK: %shuffle3.i = shufflevector <8 x i64> [[T1]], <8 x i64> undef, <8 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T2:%.+]] = icmp ugt <8 x i64> [[T1]], %shuffle3.i ;INTEL
  // CHECK: [[T3:%.+]] = select <8 x i1> [[T2]], <8 x i64> [[T1]], <8 x i64> %shuffle3.i ;INTEL
  // CHECK: %shuffle6.i = shufflevector <8 x i64> [[T3]], <8 x i64> undef, <8 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T4:%.+]] = icmp ugt <8 x i64> [[T3]], %shuffle6.i ;INTEL
  // CHECK: %.elt.i = extractelement <8 x i1> [[T4]], i32 0 ;INTEL
  // CHECK: %.elt20.i = extractelement <8 x i64> [[T3]], i32 0 ;INTEL
  // CHECK: %shuffle6.elt.i = extractelement <8 x i64> [[T3]], i32 1 ;INTEL
  // CHECK: %vecext.i = select i1 %.elt.i, i64 %.elt20.i, i64 %shuffle6.elt.i
  // CHECK: ret i64 %vecext.i
  return _mm512_reduce_max_epu64(__W); 
}

double test_mm512_reduce_min_pd(__m512d __W){
  // CHECK: %shuffle1.i = shufflevector <8 x double> %__W, <8 x double> undef, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef>
  // CHECK: [[T:%.+]] = tail call <8 x double> @llvm.x86.avx512.mask.min.pd.512(<8 x double> %__W, <8 x double> %shuffle1.i, <8 x double> zeroinitializer, i8 -1, i32 4) ;INTEL
  // CHECK: %shuffle3.i = shufflevector <8 x double> [[T]], <8 x double> undef, <8 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T1:%.+]] = tail call <8 x double> @llvm.x86.avx512.mask.min.pd.512(<8 x double> [[T]], <8 x double> %shuffle3.i, <8 x double> zeroinitializer, i8 -1, i32 4) ;INTEL
  // CHECK: %shuffle6.i = shufflevector <8 x double> [[T1]], <8 x double> undef, <8 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T2:%.+]] = tail call <8 x double> @llvm.x86.avx512.mask.min.pd.512(<8 x double> [[T1]], <8 x double> %shuffle6.i, <8 x double> zeroinitializer, i8 -1, i32 4) ;INTEL
  // CHECK: %vecext.i = extractelement <8 x double> [[T2]], i32 0 ;INTEL
  // CHECK: ret double %vecext.i
  return _mm512_reduce_min_pd(__W); 
}

long long test_mm512_mask_reduce_max_epi64(__mmask8 __M, __m512i __W){
  // CHECK: [[T:%.+]] = bitcast i8 %__M to <8 x i1> ;INTEL
  // CHECK: [[T1:%.+]] = select <8 x i1> [[T]], <8 x i64> %__W, <8 x i64> <i64 -9223372036854775808, i64 -9223372036854775808, i64 -9223372036854775808, i64 -9223372036854775808, i64 -9223372036854775808, i64 -9223372036854775808, i64 -9223372036854775808, i64 -9223372036854775808> ;INTEL
  // CHECK: %shuffle1.i = shufflevector <8 x i64> [[T1]], <8 x i64> undef, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T2:%.+]] = icmp sgt <8 x i64> [[T1]], %shuffle1.i ;INTEL
  // CHECK: [[T3:%.+]] = select <8 x i1> [[T2]], <8 x i64> [[T1]], <8 x i64> %shuffle1.i ;INTEL
  // CHECK: %shuffle4.i = shufflevector <8 x i64> [[T3]], <8 x i64> undef, <8 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T4:%.+]] = icmp sgt <8 x i64> [[T3]], %shuffle4.i ;INTEL
  // CHECK: [[T5:%.+]] = select <8 x i1> [[T4]], <8 x i64> [[T3]], <8 x i64> %shuffle4.i ;INTEL
  // CHECK: %shuffle7.i = shufflevector <8 x i64> [[T5]], <8 x i64> undef, <8 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T6:%.+]] = icmp sgt <8 x i64> [[T5]], %shuffle7.i ;INTEL
  // CHECK: %.elt.i = extractelement <8 x i1> [[T6]], i32 0 ;INTEL
  // CHECK: %.elt22.i = extractelement <8 x i64> [[T5]], i32 0 ;INTEL
  // CHECK: %shuffle7.elt.i = extractelement <8 x i64> [[T5]], i32 1 ;INTEL
  // CHECK: %vecext.i = select i1 %.elt.i, i64 %.elt22.i, i64 %shuffle7.elt.i
  // CHECK: ret i64 %vecext.i
  return _mm512_mask_reduce_max_epi64(__M, __W); 
}

unsigned long test_mm512_mask_reduce_max_epu64(__mmask8 __M, __m512i __W){
  // CHECK: [[T:%.+]] = bitcast i8 %__M to <8 x i1> ;INTEL
  // CHECK: [[T1:%.+]] = select <8 x i1> [[T]], <8 x i64> %__W, <8 x i64> zeroinitializer ;INTEL
  // CHECK: %shuffle1.i = shufflevector <8 x i64> [[T1]], <8 x i64> undef, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T2:%.+]] = icmp ugt <8 x i64> [[T1]], %shuffle1.i ;INTEL
  // CHECK: [[T3:%.+]] = select <8 x i1> [[T2]], <8 x i64> [[T1]], <8 x i64> %shuffle1.i ;INTEL
  // CHECK: %shuffle4.i = shufflevector <8 x i64> [[T3]], <8 x i64> undef, <8 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T4:%.+]] = icmp ugt <8 x i64> [[T3]], %shuffle4.i ;INTEL
  // CHECK: [[T5:%.+]] = select <8 x i1> [[T4]], <8 x i64> [[T3]], <8 x i64> %shuffle4.i ;INTEL
  // CHECK: %shuffle7.i = shufflevector <8 x i64> [[T5]], <8 x i64> undef, <8 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T6:%.+]] = icmp ugt <8 x i64> [[T5]], %shuffle7.i ;INTEL
  // CHECK: %.elt.i = extractelement <8 x i1> [[T6]], i32 0 ;INTEL
  // CHECK: %.elt22.i = extractelement <8 x i64> [[T5]], i32 0 ;INTEL
  // CHECK: %shuffle7.elt.i = extractelement <8 x i64> [[T5]], i32 1 ;INTEL
  // CHECK: %vecext.i = select i1 %.elt.i, i64 %.elt22.i, i64 %shuffle7.elt.i
  // CHECK: ret i64 %vecext.i
  return _mm512_mask_reduce_max_epu64(__M, __W); 
}

long long test_mm512_mask_reduce_max_pd(__mmask8 __M, __m512d __W){
  // CHECK: [[T:%.+]] = bitcast i8 %__M to <8 x i1> ;INTEL
  // CHECK: [[T1:%.+]] = select <8 x i1> [[T]], <8 x double> %__W, <8 x double> <double 0xFFF0000000000000, double 0xFFF0000000000000, double 0xFFF0000000000000, double 0xFFF0000000000000, double 0xFFF0000000000000, double 0xFFF0000000000000, double 0xFFF0000000000000, double 0xFFF0000000000000> ;INTEL
  // CHECK: %shuffle1.i = shufflevector <8 x double> [[T1]], <8 x double> undef, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T2:%.+]] = tail call <8 x double> @llvm.x86.avx512.mask.max.pd.512(<8 x double> [[T1]], <8 x double> %shuffle1.i, <8 x double> zeroinitializer, i8 -1, i32 4) ;INTEL
  // CHECK: %shuffle4.i = shufflevector <8 x double> [[T2]], <8 x double> undef, <8 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T3:%.+]] = tail call <8 x double> @llvm.x86.avx512.mask.max.pd.512(<8 x double> [[T2]], <8 x double> %shuffle4.i, <8 x double> zeroinitializer, i8 -1, i32 4) ;INTEL
  // CHECK: %shuffle7.i = shufflevector <8 x double> [[T3]], <8 x double> undef, <8 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T4:%.+]] = tail call <8 x double> @llvm.x86.avx512.mask.max.pd.512(<8 x double> [[T3]], <8 x double> %shuffle7.i, <8 x double> zeroinitializer, i8 -1, i32 4) ;INTEL
  // CHECK: %vecext.i = extractelement <8 x double> [[T4]], i32 0 ;INTEL
  // CHECK: %conv = fptosi double %vecext.i to i64
  // CHECK: ret i64 %conv
  return _mm512_mask_reduce_max_pd(__M, __W); 
}

long long test_mm512_mask_reduce_min_epi64(__mmask8 __M, __m512i __W){
  // CHECK: [[T:%.+]] = bitcast i8 %__M to <8 x i1> ;INTEL
  // CHECK: [[T1:%.+]] = select <8 x i1> [[T]], <8 x i64> %__W, <8 x i64> <i64 9223372036854775807, i64 9223372036854775807, i64 9223372036854775807, i64 9223372036854775807, i64 9223372036854775807, i64 9223372036854775807, i64 9223372036854775807, i64 9223372036854775807> ;INTEL
  // CHECK: %shuffle1.i = shufflevector <8 x i64> [[T1]], <8 x i64> undef, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T2:%.+]] = icmp slt <8 x i64> [[T1]], %shuffle1.i ;INTEL
  // CHECK: [[T3:%.+]] = select <8 x i1> [[T2]], <8 x i64> [[T1]], <8 x i64> %shuffle1.i ;INTEL
  // CHECK: %shuffle4.i = shufflevector <8 x i64> [[T3]], <8 x i64> undef, <8 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T4:%.+]] = icmp slt <8 x i64> [[T3]], %shuffle4.i ;INTEL
  // CHECK: [[T5:%.+]] = select <8 x i1> [[T4]], <8 x i64> [[T3]], <8 x i64> %shuffle4.i ;INTEL
  // CHECK: %shuffle7.i = shufflevector <8 x i64> [[T5]], <8 x i64> undef, <8 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T6:%.+]] = icmp slt <8 x i64> [[T5]], %shuffle7.i ;INTEL
  // CHECK: %.elt.i = extractelement <8 x i1> [[T6]], i32 0 ;INTEL
  // CHECK: %.elt22.i = extractelement <8 x i64> [[T5]], i32 0 ;INTEL
  // CHECK: %shuffle7.elt.i = extractelement <8 x i64> [[T5]], i32 1 ;INTEL
  // CHECK: %vecext.i = select i1 %.elt.i, i64 %.elt22.i, i64 %shuffle7.elt.i
  // CHECK: ret i64 %vecext.i
  return _mm512_mask_reduce_min_epi64(__M, __W); 
}

long long test_mm512_mask_reduce_min_epu64(__mmask8 __M, __m512i __W){
  // CHECK: [[T:%.+]] = bitcast i8 %__M to <8 x i1> ;INTEL
  // CHECK: [[T1:%.+]] = select <8 x i1> [[T]], <8 x i64> %__W, <8 x i64> zeroinitializer ;INTEL
  // CHECK: %shuffle1.i = shufflevector <8 x i64> [[T1]], <8 x i64> undef, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T2:%.+]] = icmp ugt <8 x i64> [[T1]], %shuffle1.i ;INTEL
  // CHECK: [[T3:%.+]] = select <8 x i1> [[T2]], <8 x i64> [[T1]], <8 x i64> %shuffle1.i ;INTEL
  // CHECK: %shuffle4.i = shufflevector <8 x i64> [[T3]], <8 x i64> undef, <8 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T4:%.+]] = icmp ugt <8 x i64> [[T3]], %shuffle4.i ;INTEL
  // CHECK: [[T5:%.+]] = select <8 x i1> [[T4]], <8 x i64> [[T3]], <8 x i64> %shuffle4.i ;INTEL
  // CHECK: %shuffle7.i = shufflevector <8 x i64> [[T5]], <8 x i64> undef, <8 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T6:%.+]] = icmp ugt <8 x i64> [[T5]], %shuffle7.i ;INTEL
  // CHECK: %.elt.i = extractelement <8 x i1> [[T6]], i32 0 ;INTEL
  // CHECK: %.elt22.i = extractelement <8 x i64> [[T5]], i32 0 ;INTEL
  // CHECK: %shuffle7.elt.i = extractelement <8 x i64> [[T5]], i32 1 ;INTEL
  // CHECK: %vecext.i = select i1 %.elt.i, i64 %.elt22.i, i64 %shuffle7.elt.i
  // CHECK: ret i64 %vecext.i
  return _mm512_mask_reduce_max_epu64(__M, __W); 
}

double test_mm512_mask_reduce_min_pd(__mmask8 __M, __m512d __W){
  // CHECK: [[T:%.+]] = bitcast i8 %__M to <8 x i1> ;INTEL
  // CHECK: [[T1:%.+]] = select <8 x i1> [[T]], <8 x double> %__W, <8 x double> <double 0x7FF0000000000000, double 0x7FF0000000000000, double 0x7FF0000000000000, double 0x7FF0000000000000, double 0x7FF0000000000000, double 0x7FF0000000000000, double 0x7FF0000000000000, double 0x7FF0000000000000> ;INTEL
  // CHECK: %shuffle1.i = shufflevector <8 x double> [[T1]], <8 x double> undef, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T2:%.+]] = tail call <8 x double> @llvm.x86.avx512.mask.min.pd.512(<8 x double> [[T1]], <8 x double> %shuffle1.i, <8 x double> zeroinitializer, i8 -1, i32 4) ;INTEL
  // CHECK: %shuffle4.i = shufflevector <8 x double> [[T2]], <8 x double> undef, <8 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T3:%.+]] = tail call <8 x double> @llvm.x86.avx512.mask.min.pd.512(<8 x double> [[T2]], <8 x double> %shuffle4.i, <8 x double> zeroinitializer, i8 -1, i32 4) ;INTEL
  // CHECK: %shuffle7.i = shufflevector <8 x double> [[T3]], <8 x double> undef, <8 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T4:%.+]] = tail call <8 x double> @llvm.x86.avx512.mask.min.pd.512(<8 x double> [[T3]], <8 x double> %shuffle7.i, <8 x double> zeroinitializer, i8 -1, i32 4) ;INTEL
  // CHECK: %vecext.i = extractelement <8 x double> [[T4]], i32 0 ;INTEL
  // CHECK: ret double %vecext.i
  return _mm512_mask_reduce_min_pd(__M, __W); 
}

int test_mm512_reduce_max_epi32(__m512i __W){
  // CHECK: [[T:%.+]] = bitcast <8 x i64> %__W to <16 x i32> ;INTEL
  // CHECK: %shuffle1.i = shufflevector <16 x i32> [[T]], <16 x i32> undef, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T1:%.+]] = icmp slt <16 x i32> %shuffle1.i, [[T]] ;INTEL
  // CHECK: [[T2:%.+]] = select <16 x i1> [[T1]], <16 x i32> [[T]], <16 x i32> %shuffle1.i ;INTEL
  // CHECK: %shuffle3.i = shufflevector <16 x i32> [[T2]], <16 x i32> undef, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T3:%.+]] = icmp sgt <16 x i32> [[T2]], %shuffle3.i ;INTEL
  // CHECK: [[T4:%.+]] = select <16 x i1> [[T3]], <16 x i32> [[T2]], <16 x i32> %shuffle3.i ;INTEL
  // CHECK: %shuffle6.i = shufflevector <16 x i32> [[T4]], <16 x i32> undef, <16 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T5:%.+]] = icmp sgt <16 x i32> [[T4]], %shuffle6.i ;INTEL
  // CHECK: [[T6:%.+]] = select <16 x i1> [[T5]], <16 x i32> [[T4]], <16 x i32> %shuffle6.i ;INTEL
  // CHECK: %shuffle9.i = shufflevector <16 x i32> [[T6]], <16 x i32> undef, <16 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T7:%.+]] = icmp sgt <16 x i32> [[T6]], %shuffle9.i ;INTEL
  // CHECK: [[T8:%.+]] = select <16 x i1> [[T7]], <16 x i32> [[T6]], <16 x i32> %shuffle9.i ;INTEL
  // CHECK: [[T9:%.+]] = bitcast <16 x i32> [[T8]] to <8 x i64> ;INTEL
  // CHECK: %vecext.i = extractelement <8 x i64> [[T9]], i32 0 ;INTEL
  // CHECK: %conv.i = trunc i64 %vecext.i to i32
  // CHECK: ret i32 %conv.i
  return _mm512_reduce_max_epi32(__W);
}

unsigned int test_mm512_reduce_max_epu32(__m512i __W){
  // CHECK: [[T:%.+]] = bitcast <8 x i64> %__W to <16 x i32> ;INTEL
  // CHECK: %shuffle1.i = shufflevector <16 x i32> [[T]], <16 x i32> undef, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T1:%.+]] = icmp ult <16 x i32> %shuffle1.i, [[T]] ;INTEL
  // CHECK: [[T2:%.+]] = select <16 x i1> [[T1]], <16 x i32> [[T]], <16 x i32> %shuffle1.i ;INTEL
  // CHECK: %shuffle3.i = shufflevector <16 x i32> [[T2]], <16 x i32> undef, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T3:%.+]] = icmp ugt <16 x i32> [[T2]], %shuffle3.i ;INTEL
  // CHECK: [[T4:%.+]] = select <16 x i1> [[T3]], <16 x i32> [[T2]], <16 x i32> %shuffle3.i ;INTEL
  // CHECK: %shuffle6.i = shufflevector <16 x i32> [[T4]], <16 x i32> undef, <16 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T5:%.+]] = icmp ugt <16 x i32> [[T4]], %shuffle6.i ;INTEL
  // CHECK: [[T6:%.+]] = select <16 x i1> [[T5]], <16 x i32> [[T4]], <16 x i32> %shuffle6.i ;INTEL
  // CHECK: %shuffle9.i = shufflevector <16 x i32> [[T6]], <16 x i32> undef, <16 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T7:%.+]] = icmp ugt <16 x i32> [[T6]], %shuffle9.i ;INTEL
  // CHECK: [[T8:%.+]] = select <16 x i1> [[T7]], <16 x i32> [[T6]], <16 x i32> %shuffle9.i ;INTEL
  // CHECK: [[T9:%.+]] = bitcast <16 x i32> [[T8]] to <8 x i64> ;INTEL
  // CHECK: %vecext.i = extractelement <8 x i64> [[T9]], i32 0 ;INTEL
  // CHECK: %conv.i = trunc i64 %vecext.i to i32
  // CHECK: ret i32 %conv.i
  return _mm512_reduce_max_epu32(__W); 
}

float test_mm512_reduce_max_ps(__m512 __W){
  // CHECK: %shuffle1.i = shufflevector <16 x float> %__W, <16 x float> undef, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  // CHECK: [[T:%.+]] = tail call <16 x float> @llvm.x86.avx512.mask.max.ps.512(<16 x float> %__W, <16 x float> %shuffle1.i, <16 x float> zeroinitializer, i16 -1, i32 4) ;INTEL
  // CHECK: %shuffle3.i = shufflevector <16 x float> [[T]], <16 x float> undef, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T1:%.+]] = tail call <16 x float> @llvm.x86.avx512.mask.max.ps.512(<16 x float> [[T]], <16 x float> %shuffle3.i, <16 x float> zeroinitializer, i16 -1, i32 4) ;INTEL
  // CHECK: %shuffle6.i = shufflevector <16 x float> [[T1]], <16 x float> undef, <16 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T2:%.+]] = tail call <16 x float> @llvm.x86.avx512.mask.max.ps.512(<16 x float> [[T1]], <16 x float> %shuffle6.i, <16 x float> zeroinitializer, i16 -1, i32 4) ;INTEL
  // CHECK: %shuffle9.i = shufflevector <16 x float> [[T2]], <16 x float> undef, <16 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T3:%.+]] = tail call <16 x float> @llvm.x86.avx512.mask.max.ps.512(<16 x float> [[T2]], <16 x float> %shuffle9.i, <16 x float> zeroinitializer, i16 -1, i32 4) ;INTEL
  // CHECK: %vecext.i = extractelement <16 x float> [[T3]], i32 0 ;INTEL
  // CHECK: ret float %vecext.i
  return _mm512_reduce_max_ps(__W); 
}

int test_mm512_reduce_min_epi32(__m512i __W){
  // CHECK: [[T:%.+]] = bitcast <8 x i64> %__W to <16 x i32> ;INTEL
  // CHECK: %shuffle1.i = shufflevector <16 x i32> [[T]], <16 x i32> undef, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T1:%.+]] = icmp sgt <16 x i32> %shuffle1.i, [[T]] ;INTEL
  // CHECK: [[T2:%.+]] = select <16 x i1> [[T1]], <16 x i32> [[T]], <16 x i32> %shuffle1.i ;INTEL
  // CHECK: %shuffle3.i = shufflevector <16 x i32> [[T2]], <16 x i32> undef, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T3:%.+]] = icmp slt <16 x i32> [[T2]], %shuffle3.i ;INTEL
  // CHECK: [[T4:%.+]] = select <16 x i1> [[T3]], <16 x i32> [[T2]], <16 x i32> %shuffle3.i ;INTEL
  // CHECK: %shuffle6.i = shufflevector <16 x i32> [[T4]], <16 x i32> undef, <16 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T5:%.+]] = icmp slt <16 x i32> [[T4]], %shuffle6.i ;INTEL
  // CHECK: [[T6:%.+]] = select <16 x i1> [[T5]], <16 x i32> [[T4]], <16 x i32> %shuffle6.i ;INTEL
  // CHECK: %shuffle9.i = shufflevector <16 x i32> [[T6]], <16 x i32> undef, <16 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T7:%.+]] = icmp slt <16 x i32> [[T6]], %shuffle9.i ;INTEL
  // CHECK: [[T8:%.+]] = select <16 x i1> [[T7]], <16 x i32> [[T6]], <16 x i32> %shuffle9.i ;INTEL
  // CHECK: [[T9:%.+]] = bitcast <16 x i32> [[T8]] to <8 x i64> ;INTEL
  // CHECK: %vecext.i = extractelement <8 x i64> [[T9]], i32 0 ;INTEL
  // CHECK: %conv.i = trunc i64 %vecext.i to i32
  // CHECK: ret i32 %conv.i
  return _mm512_reduce_min_epi32(__W);
}

unsigned int test_mm512_reduce_min_epu32(__m512i __W){
  // CHECK: [[T:%.+]] = bitcast <8 x i64> %__W to <16 x i32> ;INTEL
  // CHECK: %shuffle1.i = shufflevector <16 x i32> [[T]], <16 x i32> undef, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T1:%.+]] = icmp ugt <16 x i32> %shuffle1.i, [[T]] ;INTEL
  // CHECK: [[T2:%.+]] = select <16 x i1> [[T1]], <16 x i32> [[T]], <16 x i32> %shuffle1.i ;INTEL
  // CHECK: %shuffle3.i = shufflevector <16 x i32> [[T2]], <16 x i32> undef, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T3:%.+]] = icmp ult <16 x i32> [[T2]], %shuffle3.i ;INTEL
  // CHECK: [[T4:%.+]] = select <16 x i1> [[T3]], <16 x i32> [[T2]], <16 x i32> %shuffle3.i ;INTEL
  // CHECK: %shuffle6.i = shufflevector <16 x i32> [[T4]], <16 x i32> undef, <16 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T5:%.+]] = icmp ult <16 x i32> [[T4]], %shuffle6.i ;INTEL
  // CHECK: [[T6:%.+]] = select <16 x i1> [[T5]], <16 x i32> [[T4]], <16 x i32> %shuffle6.i ;INTEL
  // CHECK: %shuffle9.i = shufflevector <16 x i32> [[T6]], <16 x i32> undef, <16 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T7:%.+]] = icmp ult <16 x i32> [[T6]], %shuffle9.i ;INTEL
  // CHECK: [[T8:%.+]] = select <16 x i1> [[T7]], <16 x i32> [[T6]], <16 x i32> %shuffle9.i ;INTEL
  // CHECK: [[T9:%.+]] = bitcast <16 x i32> [[T8]] to <8 x i64> ;INTEL
  // CHECK: %vecext.i = extractelement <8 x i64> [[T9]], i32 0 ;INTEL
  // CHECK: %conv.i = trunc i64 %vecext.i to i32
  // CHECK: ret i32 %conv.i
  return _mm512_reduce_min_epu32(__W); 
}

float test_mm512_reduce_min_ps(__m512 __W){
  // CHECK: %shuffle1.i = shufflevector <16 x float> %__W, <16 x float> undef, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  // CHECK: [[T:%.+]] = tail call <16 x float> @llvm.x86.avx512.mask.min.ps.512(<16 x float> %__W, <16 x float> %shuffle1.i, <16 x float> zeroinitializer, i16 -1, i32 4) ;INTEL
  // CHECK: %shuffle3.i = shufflevector <16 x float> [[T]], <16 x float> undef, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T1:%.+]] = tail call <16 x float> @llvm.x86.avx512.mask.min.ps.512(<16 x float> [[T]], <16 x float> %shuffle3.i, <16 x float> zeroinitializer, i16 -1, i32 4) ;INTEL
  // CHECK: %shuffle6.i = shufflevector <16 x float> [[T1]], <16 x float> undef, <16 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T2:%.+]] = tail call <16 x float> @llvm.x86.avx512.mask.min.ps.512(<16 x float> [[T1]], <16 x float> %shuffle6.i, <16 x float> zeroinitializer, i16 -1, i32 4) ;INTEL
  // CHECK: %shuffle9.i = shufflevector <16 x float> [[T2]], <16 x float> undef, <16 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T3:%.+]] = tail call <16 x float> @llvm.x86.avx512.mask.min.ps.512(<16 x float> [[T2]], <16 x float> %shuffle9.i, <16 x float> zeroinitializer, i16 -1, i32 4) ;INTEL
  // CHECK: %vecext.i = extractelement <16 x float> [[T3]], i32 0 ;INTEL
  // CHECK: ret float %vecext.i
  return _mm512_reduce_min_ps(__W); 
}

int test_mm512_mask_reduce_max_epi32(__mmask16 __M, __m512i __W){
  // CHECK: [[T:%.+]] = bitcast <8 x i64> %__W to <16 x i32> ;INTEL
  // CHECK: [[T1:%.+]] = bitcast i16 %__M to <16 x i1> ;INTEL
  // CHECK: [[T2:%.+]] = select <16 x i1> [[T1]], <16 x i32> [[T]], <16 x i32> <i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648> ;INTEL
  // CHECK: %shuffle1.i = shufflevector <16 x i32> [[T2]], <16 x i32> undef, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T3:%.+]] = icmp sgt <16 x i32> [[T2]], %shuffle1.i ;INTEL
  // CHECK: [[T4:%.+]] = select <16 x i1> [[T3]], <16 x i32> [[T2]], <16 x i32> %shuffle1.i ;INTEL
  // CHECK: %shuffle4.i = shufflevector <16 x i32> [[T4]], <16 x i32> undef, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T5:%.+]] = icmp sgt <16 x i32> [[T4]], %shuffle4.i ;INTEL
  // CHECK: [[T6:%.+]] = select <16 x i1> [[T5]], <16 x i32> [[T4]], <16 x i32> %shuffle4.i ;INTEL
  // CHECK: %shuffle7.i = shufflevector <16 x i32> [[T6]], <16 x i32> undef, <16 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T7:%.+]] = icmp sgt <16 x i32> [[T6]], %shuffle7.i ;INTEL
  // CHECK: [[T8:%.+]] = select <16 x i1> [[T7]], <16 x i32> [[T6]], <16 x i32> %shuffle7.i ;INTEL
  // CHECK: %shuffle10.i = shufflevector <16 x i32> [[T8]], <16 x i32> undef, <16 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T9:%.+]] = icmp sgt <16 x i32> [[T8]], %shuffle10.i ;INTEL
  // CHECK: [[T10:%.+]] = select <16 x i1> [[T9]], <16 x i32> [[T8]], <16 x i32> %shuffle10.i ;INTEL
  // CHECK: [[T11:%.+]] = bitcast <16 x i32> [[T10]] to <8 x i64> ;INTEL
  // CHECK: %vecext.i = extractelement <8 x i64> [[T11]], i32 0 ;INTEL
  // CHECK: %conv.i = trunc i64 %vecext.i to i32
  // CHECK: ret i32 %conv.i
  return _mm512_mask_reduce_max_epi32(__M, __W); 
}

unsigned int test_mm512_mask_reduce_max_epu32(__mmask16 __M, __m512i __W){
  // CHECK: [[T:%.+]] = bitcast <8 x i64> %__W to <16 x i32> ;INTEL
  // CHECK: [[T1:%.+]] = bitcast i16 %__M to <16 x i1> ;INTEL
  // CHECK: [[T2:%.+]] = select <16 x i1> [[T1]], <16 x i32> [[T]], <16 x i32> zeroinitializer ;INTEL
  // CHECK: %shuffle1.i = shufflevector <16 x i32> [[T2]], <16 x i32> undef, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T3:%.+]] = icmp ugt <16 x i32> [[T2]], %shuffle1.i ;INTEL
  // CHECK: [[T4:%.+]] = select <16 x i1> [[T3]], <16 x i32> [[T2]], <16 x i32> %shuffle1.i ;INTEL
  // CHECK: %shuffle4.i = shufflevector <16 x i32> [[T4]], <16 x i32> undef, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T5:%.+]] = icmp ugt <16 x i32> [[T4]], %shuffle4.i ;INTEL
  // CHECK: [[T6:%.+]] = select <16 x i1> [[T5]], <16 x i32> [[T4]], <16 x i32> %shuffle4.i ;INTEL
  // CHECK: %shuffle7.i = shufflevector <16 x i32> [[T6]], <16 x i32> undef, <16 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T7:%.+]] = icmp ugt <16 x i32> [[T6]], %shuffle7.i ;INTEL
  // CHECK: [[T8:%.+]] = select <16 x i1> [[T7]], <16 x i32> [[T6]], <16 x i32> %shuffle7.i ;INTEL
  // CHECK: %shuffle10.i = shufflevector <16 x i32> [[T8]], <16 x i32> undef, <16 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T9:%.+]] = icmp ugt <16 x i32> [[T8]], %shuffle10.i ;INTEL
  // CHECK: [[T10:%.+]] = select <16 x i1> [[T9]], <16 x i32> [[T8]], <16 x i32> %shuffle10.i ;INTEL
  // CHECK: [[T11:%.+]] = bitcast <16 x i32> [[T10]] to <8 x i64> ;INTEL
  // CHECK: %vecext.i = extractelement <8 x i64> [[T11]], i32 0 ;INTEL
  // CHECK: %conv.i = trunc i64 %vecext.i to i32
  // CHECK: ret i32 %conv.i
  return _mm512_mask_reduce_max_epu32(__M, __W); 
}

float test_mm512_mask_reduce_max_ps(__mmask16 __M, __m512 __W){
  // CHECK: [[T:%.+]] = bitcast i16 %__M to <16 x i1> ;INTEL
  // CHECK: [[T1:%.+]] = select <16 x i1> [[T]], <16 x float> %__W, <16 x float> <float 0xFFF0000000000000, float 0xFFF0000000000000, float 0xFFF0000000000000, float 0xFFF0000000000000, float 0xFFF0000000000000, float 0xFFF0000000000000, float 0xFFF0000000000000, float 0xFFF0000000000000, float 0xFFF0000000000000, float 0xFFF0000000000000, float 0xFFF0000000000000, float 0xFFF0000000000000, float 0xFFF0000000000000, float 0xFFF0000000000000, float 0xFFF0000000000000, float 0xFFF0000000000000> ;INTEL
  // CHECK: %shuffle1.i = shufflevector <16 x float> [[T1]], <16 x float> undef, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T2:%.+]] = tail call <16 x float> @llvm.x86.avx512.mask.max.ps.512(<16 x float> [[T1]], <16 x float> %shuffle1.i, <16 x float> zeroinitializer, i16 -1, i32 4) ;INTEL
  // CHECK: %shuffle4.i = shufflevector <16 x float> [[T2]], <16 x float> undef, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T3:%.+]] = tail call <16 x float> @llvm.x86.avx512.mask.max.ps.512(<16 x float> [[T2]], <16 x float> %shuffle4.i, <16 x float> zeroinitializer, i16 -1, i32 4) ;INTEL
  // CHECK: %shuffle7.i = shufflevector <16 x float> [[T3]], <16 x float> undef, <16 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T4:%.+]] = tail call <16 x float> @llvm.x86.avx512.mask.max.ps.512(<16 x float> [[T3]], <16 x float> %shuffle7.i, <16 x float> zeroinitializer, i16 -1, i32 4) ;INTEL
  // CHECK: %shuffle10.i = shufflevector <16 x float> [[T4]], <16 x float> undef, <16 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T5:%.+]] = tail call <16 x float> @llvm.x86.avx512.mask.max.ps.512(<16 x float> [[T4]], <16 x float> %shuffle10.i, <16 x float> zeroinitializer, i16 -1, i32 4) ;INTEL
  // CHECK: %vecext.i = extractelement <16 x float> [[T5]], i32 0 ;INTEL
  // CHECK: ret float %vecext.i
  return _mm512_mask_reduce_max_ps(__M, __W); 
}

int test_mm512_mask_reduce_min_epi32(__mmask16 __M, __m512i __W){
  // CHECK: [[T:%.+]] = bitcast <8 x i64> %__W to <16 x i32> ;INTEL
  // CHECK: [[T1:%.+]] = bitcast i16 %__M to <16 x i1> ;INTEL
  // CHECK: [[T2:%.+]] = select <16 x i1> [[T1]], <16 x i32> [[T]], <16 x i32> <i32 2147483647, i32 2147483647, i32 2147483647, i32 2147483647, i32 2147483647, i32 2147483647, i32 2147483647, i32 2147483647, i32 2147483647, i32 2147483647, i32 2147483647, i32 2147483647, i32 2147483647, i32 2147483647, i32 2147483647, i32 2147483647> ;INTEL
  // CHECK: %shuffle1.i = shufflevector <16 x i32> [[T2]], <16 x i32> undef, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T3:%.+]] = icmp slt <16 x i32> [[T2]], %shuffle1.i ;INTEL
  // CHECK: [[T4:%.+]] = select <16 x i1> [[T3]], <16 x i32> [[T2]], <16 x i32> %shuffle1.i ;INTEL
  // CHECK: %shuffle4.i = shufflevector <16 x i32> [[T4]], <16 x i32> undef, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T5:%.+]] = icmp slt <16 x i32> [[T4]], %shuffle4.i ;INTEL
  // CHECK: [[T6:%.+]] = select <16 x i1> [[T5]], <16 x i32> [[T4]], <16 x i32> %shuffle4.i ;INTEL
  // CHECK: %shuffle7.i = shufflevector <16 x i32> [[T6]], <16 x i32> undef, <16 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T7:%.+]] = icmp slt <16 x i32> [[T6]], %shuffle7.i ;INTEL
  // CHECK: [[T8:%.+]] = select <16 x i1> [[T7]], <16 x i32> [[T6]], <16 x i32> %shuffle7.i ;INTEL
  // CHECK: %shuffle10.i = shufflevector <16 x i32> [[T8]], <16 x i32> undef, <16 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T9:%.+]] = icmp slt <16 x i32> [[T8]], %shuffle10.i ;INTEL
  // CHECK: [[T10:%.+]] = select <16 x i1> [[T9]], <16 x i32> [[T8]], <16 x i32> %shuffle10.i ;INTEL
  // CHECK: [[T11:%.+]] = bitcast <16 x i32> [[T10]] to <8 x i64> ;INTEL
  // CHECK: %vecext.i = extractelement <8 x i64> [[T11]], i32 0 ;INTEL
  // CHECK: %conv.i = trunc i64 %vecext.i to i32
  // CHECK: ret i32 %conv.i
  return _mm512_mask_reduce_min_epi32(__M, __W); 
}

unsigned int test_mm512_mask_reduce_min_epu32(__mmask16 __M, __m512i __W){
  // CHECK: [[T:%.+]] = bitcast <8 x i64> %__W to <16 x i32> ;INTEL
  // CHECK: [[T1:%.+]] = bitcast i16 %__M to <16 x i1> ;INTEL
  // CHECK: [[T2:%.+]] = select <16 x i1> [[T1]], <16 x i32> [[T]], <16 x i32> <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1> ;INTEL
  // CHECK: %shuffle1.i = shufflevector <16 x i32> [[T2]], <16 x i32> undef, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T3:%.+]] = icmp ult <16 x i32> [[T2]], %shuffle1.i ;INTEL
  // CHECK: [[T4:%.+]] = select <16 x i1> [[T3]], <16 x i32> [[T2]], <16 x i32> %shuffle1.i ;INTEL
  // CHECK: %shuffle4.i = shufflevector <16 x i32> [[T4]], <16 x i32> undef, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T5:%.+]] = icmp ult <16 x i32> [[T4]], %shuffle4.i ;INTEL
  // CHECK: [[T6:%.+]] = select <16 x i1> [[T5]], <16 x i32> [[T4]], <16 x i32> %shuffle4.i ;INTEL
  // CHECK: %shuffle7.i = shufflevector <16 x i32> [[T6]], <16 x i32> undef, <16 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T7:%.+]] = icmp ult <16 x i32> [[T6]], %shuffle7.i ;INTEL
  // CHECK: [[T8:%.+]] = select <16 x i1> [[T7]], <16 x i32> [[T6]], <16 x i32> %shuffle7.i ;INTEL
  // CHECK: %shuffle10.i = shufflevector <16 x i32> [[T8]], <16 x i32> undef, <16 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T9:%.+]] = icmp ult <16 x i32> [[T8]], %shuffle10.i ;INTEL
  // CHECK: [[T10:%.+]] = select <16 x i1> [[T9]], <16 x i32> [[T8]], <16 x i32> %shuffle10.i ;INTEL
  // CHECK: [[T11:%.+]] = bitcast <16 x i32> [[T10]] to <8 x i64> ;INTEL
  // CHECK: %vecext.i = extractelement <8 x i64> [[T11]], i32 0 ;INTEL
  // CHECK: %conv.i = trunc i64 %vecext.i to i32
  // CHECK: ret i32 %conv.i
  return _mm512_mask_reduce_min_epu32(__M, __W); 
}

float test_mm512_mask_reduce_min_ps(__mmask16 __M, __m512 __W){
  // CHECK: [[T:%.+]] = bitcast i16 %__M to <16 x i1> ;INTEL
  // CHECK: [[T1:%.+]] = select <16 x i1> [[T]], <16 x float> %__W, <16 x float> <float 0x7FF0000000000000, float 0x7FF0000000000000, float 0x7FF0000000000000, float 0x7FF0000000000000, float 0x7FF0000000000000, float 0x7FF0000000000000, float 0x7FF0000000000000, float 0x7FF0000000000000, float 0x7FF0000000000000, float 0x7FF0000000000000, float 0x7FF0000000000000, float 0x7FF0000000000000, float 0x7FF0000000000000, float 0x7FF0000000000000, float 0x7FF0000000000000, float 0x7FF0000000000000> ;INTEL
  // CHECK: %shuffle1.i = shufflevector <16 x float> [[T1]], <16 x float> undef, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T2:%.+]] = tail call <16 x float> @llvm.x86.avx512.mask.min.ps.512(<16 x float> [[T1]], <16 x float> %shuffle1.i, <16 x float> zeroinitializer, i16 -1, i32 4) ;INTEL
  // CHECK: %shuffle4.i = shufflevector <16 x float> [[T2]], <16 x float> undef, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T3:%.+]] = tail call <16 x float> @llvm.x86.avx512.mask.min.ps.512(<16 x float> [[T2]], <16 x float> %shuffle4.i, <16 x float> zeroinitializer, i16 -1, i32 4) ;INTEL
  // CHECK: %shuffle7.i = shufflevector <16 x float> [[T3]], <16 x float> undef, <16 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T4:%.+]] = tail call <16 x float> @llvm.x86.avx512.mask.min.ps.512(<16 x float> [[T3]], <16 x float> %shuffle7.i, <16 x float> zeroinitializer, i16 -1, i32 4) ;INTEL
  // CHECK: %shuffle10.i = shufflevector <16 x float> [[T4]], <16 x float> undef, <16 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef> ;INTEL
  // CHECK: [[T5:%.+]] = tail call <16 x float> @llvm.x86.avx512.mask.min.ps.512(<16 x float> [[T4]], <16 x float> %shuffle10.i, <16 x float> zeroinitializer, i16 -1, i32 4) ;INTEL
  // CHECK: %vecext.i = extractelement <16 x float> [[T5]], i32 0 ;INTEL
  // CHECK: ret float %vecext.i
  return _mm512_mask_reduce_min_ps(__M, __W); 
}


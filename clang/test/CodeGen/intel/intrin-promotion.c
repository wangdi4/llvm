// RUN: %clang_cc1 %s -emit-llvm -o - -triple x86_64-linux-pc -fintel-compatibility -mintrinsic-promote -internal-isystem %S/Inputs | FileCheck %s

#include <intrin-promotion.h>

// Should be promoted for usage of the builtin.
void uses_builtin_directly(__m128d A){
  (void)__builtin_ia32_vcvtsd2si32(A, 4);
}
// CHECK: define{{.*}}void @uses_builtin_directly(<2 x double> %A) #[[MVWADDAVX512F:[0-9]+]]

// Should be promoted for usage of an intrinsic function.
void uses_intrin_function(__m128d A){
  (void)A;
  avx512f_intrin();
}
// CHECK: define{{.*}}void @uses_intrin_function(<2 x double> %A) #[[ADDAVX512F_NO_MIN:[0-9]+]]

void uses_intrin_macro(__m128d A) {
  (void) _mm_cvt_roundsd_si32(A, 4);
}
// CHECK: define{{.*}}void @uses_intrin_macro(<2 x double> %A) #[[MVWADDAVX512F]]

// Should be promoted and get feature for all.
void uses_multiple_intrin_function(__m128d A){
  (void) _mm_cvt_roundsd_si32(A, 4);
  bmi_intrin();
  lzcnt_intrin();
}
// CHECK: define{{.*}}void @uses_multiple_intrin_function(<2 x double> %A) #[[ADDAVX512F_BMI_LZCNT:[0-9]+]]

// Should be promoted and get feature for all, reversed to show is deterministic.
void uses_multiple_intrin_function_reverse_order(__m128d A){
  lzcnt_intrin();
  bmi_intrin();
  (void) _mm_cvt_roundsd_si32(A, 4);
}
// CHECK: define{{.*}}void @uses_multiple_intrin_function_reverse_order(<2 x double> %A) #[[ADDAVX512F_BMI_LZCNT]]

void calls_or_builtin(__v4sf A, __v4sf B, __v4sf C) {
  (void) __builtin_ia32_vfmaddps(A, B, C);
}
// CHECK: define{{.*}}void @calls_or_builtin(<4 x float> %A, <4 x float> %B, <4 x float> %C) #[[ADDFMA:[0-9]+]]

void calls_fma4_intrin(__m128 A, __m128 B, __m128 C) {
  (void)_mm_macc_ps(A,B,C);
}
// CHECK: define{{.*}}void @calls_fma4_intrin(<4 x float> %A, <4 x float> %B, <4 x float> %C) #[[ADDFMA4:[0-9]+]]

// CHECK:#[[MVWADDAVX512F]] = {{{[^}]*}}"min-legal-vector-width"="128"{{.*}}"target-features"="+avx,+avx2,+avx512f,+cx8,+f16c,+fma,+mmx,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave"
// CHECK:#[[ADDAVX512F_NO_MIN]] = {{{[^}]*}}"target-features"="+avx,+avx2,+avx512f,+cx8,+f16c,+fma,+mmx,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave"
// CHECK:#[[ADDAVX512F_BMI_LZCNT]] = {{{[^}]*}}"target-features"="+avx,+avx2,+avx512f,+bmi,+cx8,+f16c,+fma,+lzcnt,+mmx,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave"
// CHECK:#[[ADDFMA]] = {{{[^}]*}}"target-features"="+avx,+cx8,+fma,+mmx,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave"
// CHECK:#[[ADDFMA4]] = {{{[^}]*}}"target-features"="+avx,+cx8,+fma4,+mmx,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+sse4a,+ssse3,+x87,+xsave"

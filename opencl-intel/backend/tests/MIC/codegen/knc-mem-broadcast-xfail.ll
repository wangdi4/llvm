; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s
;

target datalayout = "e-p:64:64"

define <16 x i32> @broadcastpv16i32(<16 x i32>* %pv) nounwind readnone ssp {
; CHECK: vbroadcastss 36(%rdi), %zmm0
  %v = load <16 x i32>* %pv
  %i = extractelement <16 x i32> %v, i32 9 
  %1 = insertelement <16 x i32> undef, i32 %i, i32 0
  %2 = insertelement <16 x i32> %1, i32 %i, i32 1
  %3 = insertelement <16 x i32> %2, i32 %i, i32 2
  %4 = insertelement <16 x i32> %3, i32 %i, i32 3
  %5 = insertelement <16 x i32> %4, i32 %i, i32 4
  %6 = insertelement <16 x i32> %5, i32 %i, i32 5
  %7 = insertelement <16 x i32> %6, i32 %i, i32 6
  %8 = insertelement <16 x i32> %7, i32 %i, i32 7
  %9 = insertelement <16 x i32> %8, i32 %i, i32 8
  %10 = insertelement <16 x i32> %9, i32 %i, i32 9
  %11 = insertelement <16 x i32> %10, i32 %i, i32 10
  %12 = insertelement <16 x i32> %11, i32 %i, i32 11
  %13 = insertelement <16 x i32> %12, i32 %i, i32 12
  %14 = insertelement <16 x i32> %13, i32 %i, i32 13
  %15 = insertelement <16 x i32> %14, i32 %i, i32 14
  %16 = insertelement <16 x i32> %15, i32 %i, i32 15
  ret <16 x i32> %16
}

define <8 x i64> @broadcastv8i64(<8 x i64>* %pv) nounwind readnone ssp {
; CHECK: vbroadcastsd 48(%rdi), %zmm0
  %v = load <8 x i64>* %pv
  %e = extractelement <8 x i64> %v, i32 6
  %1 = insertelement <8 x i64> undef, i64 %e, i32 0
  %2 = insertelement <8 x i64> %1, i64 %e, i32 1
  %3 = insertelement <8 x i64> %2, i64 %e, i32 2
  %4 = insertelement <8 x i64> %3, i64 %e, i32 3
  %5 = insertelement <8 x i64> %4, i64 %e, i32 4
  %6 = insertelement <8 x i64> %5, i64 %e, i32 5
  %7 = insertelement <8 x i64> %6, i64 %e, i32 6
  %8 = insertelement <8 x i64> %7, i64 %e, i32 7
  ret <8 x i64> %8
}

define <16 x float> @broadcastpv16f32(<16 x float>* %pv) nounwind readnone ssp {
; CHECK: vbroadcastss 8(%rdi), %zmm0
  %v = load <16 x float>* %pv
  %e = extractelement <16 x float> %v, i32 2 
  %1 = insertelement <16 x float> undef, float %e, i32 0
  %2 = insertelement <16 x float> %1, float %e, i32 1
  %3 = insertelement <16 x float> %2, float %e, i32 2
  %4 = insertelement <16 x float> %3, float %e, i32 3
  %5 = insertelement <16 x float> %4, float %e, i32 4
  %6 = insertelement <16 x float> %5, float %e, i32 5
  %7 = insertelement <16 x float> %6, float %e, i32 6
  %8 = insertelement <16 x float> %7, float %e, i32 7
  %9 = insertelement <16 x float> %8, float %e, i32 8
  %10 = insertelement <16 x float> %9, float %e, i32 9
  %11 = insertelement <16 x float> %10, float %e, i32 10
  %12 = insertelement <16 x float> %11, float %e, i32 11
  %13 = insertelement <16 x float> %12, float %e, i32 12
  %14 = insertelement <16 x float> %13, float %e, i32 13
  %15 = insertelement <16 x float> %14, float %e, i32 14
  %16 = insertelement <16 x float> %15, float %e, i32 15
  ret <16 x float> %16
}

define <8 x double> @broadcastpv8f64(<8 x double>* %pv) nounwind readnone ssp {
; CHECK: vbroadcastsd 32(%rdi), %zmm0
  %v = load <8 x double>* %pv
  %e = extractelement <8 x double> %v, i32 4 
  %1 = insertelement <8 x double> undef, double %e, i32 0
  %2 = insertelement <8 x double> %1, double %e, i32 1
  %3 = insertelement <8 x double> %2, double %e, i32 2
  %4 = insertelement <8 x double> %3, double %e, i32 3
  %5 = insertelement <8 x double> %4, double %e, i32 4
  %6 = insertelement <8 x double> %5, double %e, i32 5
  %7 = insertelement <8 x double> %6, double %e, i32 6
  %8 = insertelement <8 x double> %7, double %e, i32 7
  ret <8 x double> %8
}


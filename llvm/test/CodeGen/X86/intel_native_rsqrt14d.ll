; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+avx512vl --show-mc-encoding | FileCheck %s --check-prefixes=CHECK,AVX512

declare double @llvm.sqrt.f64(double)
declare <2 x double> @llvm.sqrt.v2f64(<2 x double>)
declare <4 x double> @llvm.sqrt.v4f64(<4 x double>)
declare <8 x double> @llvm.sqrt.v8f64(<8 x double>)

define double @test_native_rsqrt14_sd(double %data) #0 {
; AVX512-LABEL: test_native_rsqrt14_sd:
; AVX512:       # %bb.0:
; AVX512-NEXT:    vrsqrt14sd %xmm0, %xmm0
; AVX512-NEXT:    retq
  %sqrt = tail call double @llvm.sqrt.f64(double %data)
  %div = fdiv fast double 1.0, %sqrt
  ret double %div
}

define <2 x double> @test_native_rsqrt14_pd_128(<2 x double> %data) #0 {
; AVX512-LABEL: test_native_rsqrt14_pd_128:
; AVX512:       # %bb.0:
; AVX512-NEXT:    vrsqrt14pd %xmm0, %xmm0
; AVX512-NEXT:    retq
  %sqrt = tail call <2 x double> @llvm.sqrt.v2f64(<2 x double> %data)
  %div = fdiv fast <2 x double> <double 1.0, double 1.0>, %sqrt
  ret <2 x double> %div
}

define <4 x double> @test_native_rsqrt14_pd_256(<4 x double> %data) #0 {
; AVX512-LABEL: test_native_rsqrt14_pd_256:
; AVX512:       # %bb.0:
; AVX512-NEXT:    vrsqrt14pd %ymm0, %ymm0
; AVX512-NEXT:    retq
  %sqrt = tail call <4 x double> @llvm.sqrt.v4f64(<4 x double> %data)
  %div = fdiv fast <4 x double> <double 1.0, double 1.0, double 1.0, double 1.0>, %sqrt
  ret <4 x double> %div
}

define <8 x double> @test_native_rsqrt14_pd_512(<8 x double> %data) #0 {
; AVX512-LABEL: test_native_rsqrt14_pd_512:
; AVX512:       # %bb.0:
; AVX512-NEXT:    vrsqrt14pd %zmm0, %zmm0
; AVX512-NEXT:    retq
  %sqrt = tail call <8 x double> @llvm.sqrt.v8f64(<8 x double> %data)
  %div = fdiv fast <8 x double> <double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0>, %sqrt
  ret <8 x double> %div
}

attributes #0 = { "reciprocal-estimates"="sqrtd:0,vec-sqrtd:0" }

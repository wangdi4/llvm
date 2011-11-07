; RUN: llc < %s -march=x86-64 -mcpu=sandybridge | FileCheck %s

declare double @__ocl_svml_d1(double)

declare <4 x double> @__ocl_svml_d4(<4 x double>)

declare <8 x double> @__ocl_svml_d8(<8 x double>)

declare <16 x double> @__ocl_svml_d16(<16 x double>)

declare double @__ocl_svml_d1_d1(double, double)

declare <4 x double> @__ocl_svml_d4_d4(<4 x double>, <4 x double>)

declare <8 x double> @__ocl_svml_d8_d8(<8 x double>, <8 x double>)

declare <16 x double> @__ocl_svml_d16_d16(<16 x double>, <16 x double>)


define double @testd1(double %x, double %y) nounwind {
entry:
;CHECK: testd1
;CHECK: vaddsd {{[^,]*}}, %xmm0
  %x1 = fadd  double  %x, %y
  %call = tail call x86_svmlcc double @__ocl_svml_d1(double %x1) nounwind
  ret double %call
}

define <4 x double> @testd4(<4 x double> %x, <4 x double> %y) nounwind {
entry:
;CHECK: testd4
;CHECK: vaddpd {{[^,]*}}, %ymm0
  %x1 = fadd  <4 x double>  %x, %y
  %call = tail call x86_svmlcc <4 x double> @__ocl_svml_d4(<4 x double> %x1) nounwind
  ret <4 x double> %call
}

define <8 x double> @testd8(<8 x double> %x, <8 x double> %y) nounwind {
entry:
;CHECK: testd8
;CHECK: vaddpd {{[^,]*}}, %ymm0
  %x1 = fadd  <8 x double>  %x, %y
  %call = tail call x86_svmlcc <8 x double> @__ocl_svml_d8(<8 x double> %x1) nounwind
  ret <8 x double> %call
}

define <16 x double> @testd16(<16 x double> %x, <16 x double> %y) nounwind {
entry:
;CHECK: testd16
;CHECK: vaddpd  {{.*}}, %ymm0
;CHECK: vaddpd  {{.*}}, %ymm1
;CHECK: vaddpd  {{.*}}, %ymm2
;CHECK: vaddpd  {{.*}}, %ymm3
;CHECK: callq
  %x1 = fadd  <16 x double>  %x, %y
  %call = tail call x86_svmlcc <16 x double> @__ocl_svml_d16(<16 x double> %x1) nounwind
  %y1 = fsub  <16 x double>  %call, %y
  ret <16 x double> %y1
}

define double @testd1_d1(double %x, double %y) nounwind {
entry:
;CHECK: testd1_d1
;CHECK: vaddsd {{[^,]*}}, %xmm0
  %x1 = fadd  double  %x, %y
  %call = tail call x86_svmlcc double @__ocl_svml_d1_d1(double %x1, double %y) nounwind
  ret double %call
}
; ( result((ymm0)) parameters((ymm0),(ymm1))  )
define <4 x double> @testd4_d4(<4 x double> %x, <4 x double> %y) nounwind {
entry:
;CHECK: testd4_d4
;CHECK: vaddpd  {{.*}}, %ymm0
  %x1 = fadd  <4 x double>  %x, %y
  %call = tail call x86_svmlcc <4 x double> @__ocl_svml_d4_d4(<4 x double> %x1, <4 x double> %y) nounwind
  ret <4 x double> %call
}

; ( result((ymm0 ymm1)) parameters((ymm0 ymm1),(ymm2 ymm3))  )
define <8 x double> @testd8_d8(<8 x double> %x, <8 x double> %y) nounwind {
entry:
;CHECK: testd8_d8
;CHECK: vaddpd  {{.*}}, %ymm0
;CHECK: vaddpd  {{.*}}, %ymm2
;CHECK: vaddpd  {{.*}}, %ymm1
;CHECK: vaddpd  {{.*}}, %ymm3
  %x1 = fadd  <8 x double>  %x, %y
  %y1 = fadd  <8 x double>  %x1, %y
  %call = tail call x86_svmlcc <8 x double> @__ocl_svml_d8_d8(<8 x double> %x1, <8 x double> %y1) nounwind
  %res = fsub <8 x double> %x, %call
  ret <8 x double> %res
}


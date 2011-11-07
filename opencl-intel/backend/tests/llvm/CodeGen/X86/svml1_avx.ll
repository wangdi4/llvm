; RUN: llc < %s -march=x86-64 -mcpu=sandybridge | FileCheck %s

declare float @__ocl_svml_f1(float)

declare <4 x float> @__ocl_svml_f4(<4 x float>)

declare <8 x float> @__ocl_svml_f8(<8 x float>)

declare <16 x float> @__ocl_svml_f16(<16 x float>)

declare float @__ocl_svml_f1_f1(float, float)

declare <4 x float> @__ocl_svml_f4_f4(<4 x float>, <4 x float>)

declare <8 x float> @__ocl_svml_f8_f8(<8 x float>, <8 x float>)

declare <16 x float> @__ocl_svml_f16_f16(<16 x float>, <16 x float>)


define float @testf1(float %x, float %y) nounwind {
entry:
;CHECK: testf1
;CHECK: vaddss {{.*}}, %xmm0
  %x1 = fadd  float  %x, %y
  %call = tail call x86_svmlcc float @__ocl_svml_f1(float %x1) nounwind
  ret float %call
}

define <4 x float> @testf4(<4 x float> %x, <4 x float> %y) nounwind {
entry:
;CHECK: testf4
;CHECK: vaddps {{.*}}, %xmm0
  %x1 = fadd  <4 x float>  %x, %y
  %call = tail call x86_svmlcc <4 x float> @__ocl_svml_f4(<4 x float> %x1) nounwind
  ret <4 x float> %call
}

define <8 x float> @testf8(<8 x float> %x, <8 x float> %y) nounwind {
entry:
;CHECK: testf8
;CHECK: vaddps {{.*}}, %ymm0
  %x1 = fadd  <8 x float>  %x, %y
  %call = tail call x86_svmlcc <8 x float> @__ocl_svml_f8(<8 x float> %x1) nounwind
  ret <8 x float> %call
}

define <16 x float> @testf16(<16 x float> %x, <16 x float> %y) nounwind {
entry:
;CHECK: testf16
;CHECK: vaddps {{.*}}, %ymm0
;CHECK: vaddps {{.*}}, %ymm1
;CHECK: callq
;CHECK: vsubps {{.*}}, %ymm0
;CHECK: vsubps {{.*}}, %ymm1
  %x1 = fadd  <16 x float>  %x, %y
  %call = tail call x86_svmlcc <16 x float> @__ocl_svml_f16(<16 x float> %x1) nounwind
  %y1 = fsub  <16 x float>  %call, %y
  ret <16 x float> %y1
}

define float @testf1_f1(float %x, float %y) nounwind {
entry:
;CHECK: testf1_f1
;CHECK: vaddss {{.*}}, %xmm0
  %x1 = fadd  float  %x, %y
  %call = tail call x86_svmlcc float @__ocl_svml_f1_f1(float %x1, float %y) nounwind
  ret float %call
}

define <4 x float> @testf4_f4(<4 x float> %x, <4 x float> %y) nounwind {
entry:
;CHECK: testf4_f4
;CHECK: vaddps  {{.*}}, %xmm0
  %x1 = fadd  <4 x float>  %x, %y
  %call = tail call x86_svmlcc <4 x float> @__ocl_svml_f4_f4(<4 x float> %x1, <4 x float> %y) nounwind
  ret <4 x float> %call
}

define <8 x float> @testf8_f8(<8 x float> %x, <8 x float> %y) nounwind {
entry:
;CHECK: testf8_f8
;CHECK: vaddps  {{.*}}, %ymm0

  %x1 = fadd  <8 x float>  %x, %y
  %call = tail call x86_svmlcc <8 x float> @__ocl_svml_f8_f8(<8 x float> %x1, <8 x float> %y) nounwind
  ret <8 x float> %call
}

; ( result((ymm0 ymm1)) parameters((ymm0 ymm1),(ymm2 ymm3))  )
define <16 x float> @testf16_f16(<16 x float> %x, <16 x float> %y) nounwind {
entry:
;CHECK: testf16_f16
;CHECK: vaddps {{.*}}, %ymm2
;CHECK: vaddps {{.*}}, %ymm3
;CHECK: callq
;CHECK: vsubps {{.*}}, %ymm0
;CHECK: vsubps {{.*}}, %ymm1
  %x1 = fadd  <16 x float>  %x, %y
  %y1 = fadd  <16 x float>  %y, %x1
  %call = tail call x86_svmlcc <16 x float> @__ocl_svml_f16_f16(<16 x float> %x1, <16 x float> %y1) nounwind
  %y2 = fsub  <16 x float>  %call, %y
  ret <16 x float> %y2
}

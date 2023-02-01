; Ensure math function calls are only vectorized if they have the same
; IMF attributes, and these attributes are preserved in vectorized call.

; RUN: opt -passes=inject-tli-mappings,slp-vectorizer -vector-library=SVML -S %s | FileCheck %s

; CHECK-LABEL: define {{[^@]+}}@same_attributes(
; CHECK: call fast <4 x float> @llvm.sqrt.v4f32(<4 x float> %{{.*}}) #[[ATTR_LOW_PREC:[0-9]+]]

define <4 x float> @same_attributes(<4 x float>* %a) {
entry:
  %0 = load <4 x float>, <4 x float>* %a, align 16
  %vecext = extractelement <4 x float> %0, i32 0
  %1 = tail call fast float @sqrtf(float %vecext) #0
  %vecins = insertelement <4 x float> undef, float %1, i32 0
  %vecext.1 = extractelement <4 x float> %0, i32 1
  %2 = tail call fast float @sqrtf(float %vecext.1) #0
  %vecins.1 = insertelement <4 x float> %vecins, float %2, i32 1
  %vecext.2 = extractelement <4 x float> %0, i32 2
  %3 = tail call fast float @sqrtf(float %vecext.2) #0
  %vecins.2 = insertelement <4 x float> %vecins.1, float %3, i32 2
  %vecext.3 = extractelement <4 x float> %0, i32 3
  %4 = tail call fast float @sqrtf(float %vecext.3) #0
  %vecins.3 = insertelement <4 x float> %vecins.2, float %4, i32 3
  ret <4 x float> %vecins.3
}

; CHECK-LABEL: define {{[^@]+}}@different_imf_attributes(
; CHECK: call fast float @sqrtf(float %{{.*}}) #[[ATTR_LOW_PREC_INJECTED:[0-9]+]]
; CHECK: call fast <2 x float> @llvm.sqrt.v2f32(<2 x float> %{{.*}}) #[[ATTR_LOW_PREC]]
; CHECK: call fast float @sqrtf(float %{{.*}}) #[[ATTR_HIGH_PREC_INJECTED:[0-9]+]]

define <4 x float> @different_imf_attributes(<4 x float>* %a) {
entry:
  %0 = load <4 x float>, <4 x float>* %a, align 16
  %vecext = extractelement <4 x float> %0, i32 0
  %1 = tail call fast float @sqrtf(float %vecext) #0
  %vecins = insertelement <4 x float> undef, float %1, i32 0
  %vecext.1 = extractelement <4 x float> %0, i32 1
  %2 = tail call fast float @sqrtf(float %vecext.1) #0
  %vecins.1 = insertelement <4 x float> %vecins, float %2, i32 1
  %vecext.2 = extractelement <4 x float> %0, i32 2
  %3 = tail call fast float @sqrtf(float %vecext.2) #0
  %vecins.2 = insertelement <4 x float> %vecins.1, float %3, i32 2
  %vecext.3 = extractelement <4 x float> %0, i32 3
  %4 = tail call fast float @sqrtf(float %vecext.3) #1
  %vecins.3 = insertelement <4 x float> %vecins.2, float %4, i32 3
  ret <4 x float> %vecins.3
}

; Difference in other attributes are ignored.
; CHECK-LABEL: define {{[^@]+}}@different_other_attributes(
; CHECK: call fast <2 x float> @llvm.sqrt.v2f32(<2 x float> %{{.*}}) #[[ATTR_MEDIUM_PREC:[0-9]+]]
; CHECK: call fast <2 x float> @llvm.sqrt.v2f32(<2 x float> %{{.*}})

define <4 x float> @different_other_attributes(<4 x float>* %a) {
entry:
  %0 = load <4 x float>, <4 x float>* %a, align 16
  %vecext = extractelement <4 x float> %0, i32 0
  %1 = tail call fast float @sqrtf(float %vecext) #3
  %vecins = insertelement <4 x float> undef, float %1, i32 0
  %vecext.1 = extractelement <4 x float> %0, i32 1
  %2 = tail call fast float @sqrtf(float %vecext.1) #4
  %vecins.1 = insertelement <4 x float> %vecins, float %2, i32 1
  %vecext.2 = extractelement <4 x float> %0, i32 2
  %3 = tail call fast float @sqrtf(float %vecext.2)
  %vecins.2 = insertelement <4 x float> %vecins.1, float %3, i32 2
  %vecext.3 = extractelement <4 x float> %0, i32 3
  %4 = tail call fast float @sqrtf(float %vecext.3) #5
  %vecins.3 = insertelement <4 x float> %vecins.2, float %4, i32 3
  ret <4 x float> %vecins.3
}

declare float @sqrtf(float) readonly nounwind willreturn

; CHECK: attributes #[[ATTR_LOW_PREC]] = {{.*}}"imf-precision"="low"
; CHECK: attributes #[[ATTR_LOW_PREC_INJECTED]] = {{.*}}"imf-precision"="low"
; CHECK: attributes #[[ATTR_HIGH_PREC_INJECTED]] = {{.*}}"imf-precision"="high"
; CHECK: attributes #[[ATTR_MEDIUM_PREC]] = {{.*}}"imf-precision"="medium"

attributes #0 = { "imf-precision"="low" }
attributes #1 = { "imf-precision"="high" }
attributes #3 = { "imf-precision"="medium" }
attributes #4 = { "imf-precision"="medium" nofree }
attributes #5 = { nofree }

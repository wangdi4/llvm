; RUN: opt -passes=sycl-kernel-relaxed-math -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-relaxed-math -S %s | FileCheck %s


define void @check_not_replace_float(float %f1, <2 x float> %f2, <3 x float> %f3, <4 x float> %f4, <8 x float> %f8, <16 x float> %f16) nounwind {
entry:
    %call01 = call float @_Z5log10f(float %f1)
    %call02 = call <2 x float> @_Z5log10Dv2_f(<2 x float> %f2)
    %call03 = call <3 x float> @_Z5log10Dv3_f(<3 x float> %f3)
    %call04 = call <4 x float> @_Z5log10Dv4_f(<4 x float> %f4)
    %call05 = call <8 x float> @_Z5log10Dv8_f(<8 x float> %f8)
    %call06 = call <16 x float> @_Z5log10Dv16_f(<16 x float> %f16)
    %call07 = call float @_Z4powrff(float %f1, float %f1)
    %call08 = call <2 x float> @_Z4powrDv2_fS_(<2 x float> %f2, <2 x float> %f2)
    %call09 = call <3 x float> @_Z4powrDv3_fS_(<3 x float> %f3, <3 x float> %f3)
    %call10 = call <4 x float> @_Z4powrDv4_fS_(<4 x float> %f4, <4 x float> %f4)
    %call11 = call <8 x float> @_Z4powrDv8_fS_(<8 x float> %f8, <8 x float> %f8)
    %call12 = call <16 x float> @_Z4powrDv16_fS_(<16 x float> %f16, <16 x float> %f16)
    %call13 = call float @_Z5rsqrtf(float %f1)
    %call14 = call <2 x float> @_Z5rsqrtDv2_f(<2 x float> %f2)
    %call15 = call <3 x float> @_Z5rsqrtDv3_f(<3 x float> %f3)
    %call16 = call <4 x float> @_Z5rsqrtDv4_f(<4 x float> %f4)
    %call17 = call <8 x float> @_Z5rsqrtDv8_f(<8 x float> %f8)
    %call18 = call <16 x float> @_Z5rsqrtDv16_f(<16 x float> %f16)
    %call19 = call float @_Z4sqrtf(float %f1)
    %call20 = call <2 x float> @_Z4sqrtDv2_f(<2 x float> %f2)
    %call21 = call <3 x float> @_Z4sqrtDv3_f(<3 x float> %f3)
    %call22 = call <4 x float> @_Z4sqrtDv4_f(<4 x float> %f4)
    %call23 = call <8 x float> @_Z4sqrtDv8_f(<8 x float> %f8)
    %call24 = call <16 x float> @_Z4sqrtDv16_f(<16 x float> %f16)
    ret void
}

declare float @_Z5log10f(float)
declare <2 x float> @_Z5log10Dv2_f(<2 x float>)
declare <3 x float> @_Z5log10Dv3_f(<3 x float>)
declare <4 x float> @_Z5log10Dv4_f(<4 x float>)
declare <8 x float> @_Z5log10Dv8_f(<8 x float>)
declare <16 x float> @_Z5log10Dv16_f(<16 x float>)

declare float @_Z4powrff(float, float)
declare <2 x float> @_Z4powrDv2_fS_(<2 x float>, <2 x float>)
declare <3 x float> @_Z4powrDv3_fS_(<3 x float>, <3 x float>)
declare <4 x float> @_Z4powrDv4_fS_(<4 x float>, <4 x float>)
declare <8 x float> @_Z4powrDv8_fS_(<8 x float>, <8 x float>)
declare <16 x float> @_Z4powrDv16_fS_(<16 x float>, <16 x float>)

declare float @_Z5rsqrtf(float)
declare <2 x float> @_Z5rsqrtDv2_f(<2 x float>)
declare <3 x float> @_Z5rsqrtDv3_f(<3 x float>)
declare <4 x float> @_Z5rsqrtDv4_f(<4 x float>)
declare <8 x float> @_Z5rsqrtDv8_f(<8 x float>)
declare <16 x float> @_Z5rsqrtDv16_f(<16 x float>)

declare float @_Z4sqrtf(float)
declare <2 x float> @_Z4sqrtDv2_f(<2 x float>)
declare <3 x float> @_Z4sqrtDv3_f(<3 x float>)
declare <4 x float> @_Z4sqrtDv4_f(<4 x float>)
declare <8 x float> @_Z4sqrtDv8_f(<8 x float>)
declare <16 x float> @_Z4sqrtDv16_f(<16 x float>)

!opencl.ocl.version = !{!0}
!0 = !{i32 2, i32 0}

; CHECK:        define void @check_not_replace_float
; CHECK:        entry:
; CHECK-NEXT:   %call01 = call float @_Z5log10f(float %f1)
; CHECK-NEXT:   %call02 = call <2 x float> @_Z5log10Dv2_f(<2 x float> %f2)
; CHECK-NEXT:   %call03 = call <3 x float> @_Z5log10Dv3_f(<3 x float> %f3)
; CHECK-NEXT:   %call04 = call <4 x float> @_Z5log10Dv4_f(<4 x float> %f4)
; CHECK-NEXT:   %call05 = call <8 x float> @_Z5log10Dv8_f(<8 x float> %f8)
; CHECK-NEXT:   %call06 = call <16 x float> @_Z5log10Dv16_f(<16 x float> %f16)
; CHECK-NEXT:   %call07 = call float @_Z4powrff(float %f1, float %f1)
; CHECK-NEXT:   %call08 = call <2 x float> @_Z4powrDv2_fS_(<2 x float> %f2, <2 x float> %f2)
; CHECK-NEXT:   %call09 = call <3 x float> @_Z4powrDv3_fS_(<3 x float> %f3, <3 x float> %f3)
; CHECK-NEXT:   %call10 = call <4 x float> @_Z4powrDv4_fS_(<4 x float> %f4, <4 x float> %f4)
; CHECK-NEXT:   %call11 = call <8 x float> @_Z4powrDv8_fS_(<8 x float> %f8, <8 x float> %f8)
; CHECK-NEXT:   %call12 = call <16 x float> @_Z4powrDv16_fS_(<16 x float> %f16, <16 x float> %f16)
; CHECK-NEXT:   %call13 = call float @_Z5rsqrtf(float %f1)
; CHECK-NEXT:   %call14 = call <2 x float> @_Z5rsqrtDv2_f(<2 x float> %f2)
; CHECK-NEXT:   %call15 = call <3 x float> @_Z5rsqrtDv3_f(<3 x float> %f3)
; CHECK-NEXT:   %call16 = call <4 x float> @_Z5rsqrtDv4_f(<4 x float> %f4)
; CHECK-NEXT:   %call17 = call <8 x float> @_Z5rsqrtDv8_f(<8 x float> %f8)
; CHECK-NEXT:   %call18 = call <16 x float> @_Z5rsqrtDv16_f(<16 x float> %f16)
; CHECK-NEXT:   %call19 = call float @_Z4sqrtf(float %f1)
; CHECK-NEXT:   %call20 = call <2 x float> @_Z4sqrtDv2_f(<2 x float> %f2)
; CHECK-NEXT:   %call21 = call <3 x float> @_Z4sqrtDv3_f(<3 x float> %f3)
; CHECK-NEXT:   %call22 = call <4 x float> @_Z4sqrtDv4_f(<4 x float> %f4)
; CHECK-NEXT:   %call23 = call <8 x float> @_Z4sqrtDv8_f(<8 x float> %f8)
; CHECK-NEXT:   %call24 = call <16 x float> @_Z4sqrtDv16_f(<16 x float> %f16)
; CHECK-NEXT:   ret void

; CHECK:        declare float @_Z5log10f(float)
; CHECK:        declare <2 x float> @_Z5log10Dv2_f(<2 x float>)
; CHECK:        declare <3 x float> @_Z5log10Dv3_f(<3 x float>)
; CHECK:        declare <4 x float> @_Z5log10Dv4_f(<4 x float>)
; CHECK:        declare <8 x float> @_Z5log10Dv8_f(<8 x float>)
; CHECK:        declare <16 x float> @_Z5log10Dv16_f(<16 x float>)

; CHECK:        declare float @_Z4powrff(float, float)
; CHECK:        declare <2 x float> @_Z4powrDv2_fS_(<2 x float>, <2 x float>)
; CHECK:        declare <3 x float> @_Z4powrDv3_fS_(<3 x float>, <3 x float>)
; CHECK:        declare <4 x float> @_Z4powrDv4_fS_(<4 x float>, <4 x float>)
; CHECK:        declare <8 x float> @_Z4powrDv8_fS_(<8 x float>, <8 x float>)
; CHECK:        declare <16 x float> @_Z4powrDv16_fS_(<16 x float>, <16 x float>)

; CHECK:        declare float @_Z5rsqrtf(float)
; CHECK:        declare <2 x float> @_Z5rsqrtDv2_f(<2 x float>)
; CHECK:        declare <3 x float> @_Z5rsqrtDv3_f(<3 x float>)
; CHECK:        declare <4 x float> @_Z5rsqrtDv4_f(<4 x float>)
; CHECK:        declare <8 x float> @_Z5rsqrtDv8_f(<8 x float>)
; CHECK:        declare <16 x float> @_Z5rsqrtDv16_f(<16 x float>)

; CHECK:        declare float @_Z4sqrtf(float)
; CHECK:        declare <2 x float> @_Z4sqrtDv2_f(<2 x float>)
; CHECK:        declare <3 x float> @_Z4sqrtDv3_f(<3 x float>)
; CHECK:        declare <4 x float> @_Z4sqrtDv4_f(<4 x float>)
; CHECK:        declare <8 x float> @_Z4sqrtDv8_f(<8 x float>)
; CHECK:        declare <16 x float> @_Z4sqrtDv16_f(<16 x float>)

; DEBUGIFY-NOT: WARNING

; RUN: opt -passes=sycl-kernel-relaxed-math -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-relaxed-math -S %s | FileCheck %s


define void @check_fdim_float(float %f1, <2 x float> %f2, <3 x float> %f3, <4 x float> %f4, <8 x float> %f8, <16 x float> %f16) nounwind {
entry:
    %call01 = call float @_Z4fdimff(float %f1, float %f1)
    %call02 = call <2 x float> @_Z4fdimDv2_fS_(<2 x float> %f2, <2 x float> %f2)
    %call03 = call <3 x float> @_Z4fdimDv3_fS_(<3 x float> %f3, <3 x float> %f3)
    %call04 = call <4 x float> @_Z4fdimDv4_fS_(<4 x float> %f4, <4 x float> %f4)
    %call05 = call <8 x float> @_Z4fdimDv8_fS_(<8 x float> %f8, <8 x float> %f8)
    %call06 = call <16 x float> @_Z4fdimDv16_fS_(<16 x float> %f16, <16 x float> %f16)
    ret void
}

declare float @_Z4fdimff(float, float)
declare <2 x float> @_Z4fdimDv2_fS_(<2 x float>, <2 x float>)
declare <3 x float> @_Z4fdimDv3_fS_(<3 x float>, <3 x float>)
declare <4 x float> @_Z4fdimDv4_fS_(<4 x float>, <4 x float>)
declare <8 x float> @_Z4fdimDv8_fS_(<8 x float>, <8 x float>)
declare <16 x float> @_Z4fdimDv16_fS_(<16 x float>, <16 x float>)

; CHECK:        define void @check_fdim_float
; CHECK:        entry:
; CHECK-NEXT:   %call01 = call float @_Z11native_fdimff(float %f1, float %f1)
; CHECK-NEXT:   %call02 = call <2 x float> @_Z11native_fdimDv2_fS_(<2 x float> %f2, <2 x float> %f2)
; CHECK-NEXT:   %call03 = call <3 x float> @_Z11native_fdimDv3_fS_(<3 x float> %f3, <3 x float> %f3)
; CHECK-NEXT:   %call04 = call <4 x float> @_Z11native_fdimDv4_fS_(<4 x float> %f4, <4 x float> %f4)
; CHECK-NEXT:   %call05 = call <8 x float> @_Z11native_fdimDv8_fS_(<8 x float> %f8, <8 x float> %f8)
; CHECK-NEXT:   %call06 = call <16 x float> @_Z11native_fdimDv16_fS_(<16 x float> %f16, <16 x float> %f16)
; CHECK-NEXT:   ret void

; CHECK:        declare float @_Z11native_fdimff(float, float)
; CHECK:        declare <2 x float> @_Z11native_fdimDv2_fS_(<2 x float>, <2 x float>)
; CHECK:        declare <3 x float> @_Z11native_fdimDv3_fS_(<3 x float>, <3 x float>)
; CHECK:        declare <4 x float> @_Z11native_fdimDv4_fS_(<4 x float>, <4 x float>)
; CHECK:        declare <8 x float> @_Z11native_fdimDv8_fS_(<8 x float>, <8 x float>)
; CHECK:        declare <16 x float> @_Z11native_fdimDv16_fS_(<16 x float>, <16 x float>)

; DEBUGIFY-NOT: WARNING

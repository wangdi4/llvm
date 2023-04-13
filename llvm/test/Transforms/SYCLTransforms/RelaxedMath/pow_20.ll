; RUN: opt -passes=sycl-kernel-relaxed-math -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-relaxed-math -S %s | FileCheck %s


define void @check_pow_float(float %f1, <2 x float> %f2, <3 x float> %f3, <4 x float> %f4, <8 x float> %f8, <16 x float> %f16) nounwind {
entry:
    %call01 = call float @_Z3powff(float %f1, float %f1)
    %call02 = call <2 x float> @_Z3powDv2_fS_(<2 x float> %f2, <2 x float> %f2)
    %call03 = call <3 x float> @_Z3powDv3_fS_(<3 x float> %f3, <3 x float> %f3)
    %call04 = call <4 x float> @_Z3powDv4_fS_(<4 x float> %f4, <4 x float> %f4)
    %call05 = call <8 x float> @_Z3powDv8_fS_(<8 x float> %f8, <8 x float> %f8)
    %call06 = call <16 x float> @_Z3powDv16_fS_(<16 x float> %f16, <16 x float> %f16)
    ret void
}

declare float @_Z3powff(float, float)
declare <2 x float> @_Z3powDv2_fS_(<2 x float>, <2 x float>)
declare <3 x float> @_Z3powDv3_fS_(<3 x float>, <3 x float>)
declare <4 x float> @_Z3powDv4_fS_(<4 x float>, <4 x float>)
declare <8 x float> @_Z3powDv8_fS_(<8 x float>, <8 x float>)
declare <16 x float> @_Z3powDv16_fS_(<16 x float>, <16 x float>)

!opencl.ocl.version = !{!0}

!0 = !{i32 2, i32 0}

; CHECK:        define void @check_pow_float
; CHECK:        entry:
; CHECK-NEXT:   %call01 = call float @_Z6pow_rmff(float %f1, float %f1)
; CHECK-NEXT:   %call02 = call <2 x float> @_Z6pow_rmDv2_fS_(<2 x float> %f2, <2 x float> %f2)
; CHECK-NEXT:   %call03 = call <3 x float> @_Z6pow_rmDv3_fS_(<3 x float> %f3, <3 x float> %f3)
; CHECK-NEXT:   %call04 = call <4 x float> @_Z6pow_rmDv4_fS_(<4 x float> %f4, <4 x float> %f4)
; CHECK-NEXT:   %call05 = call <8 x float> @_Z6pow_rmDv8_fS_(<8 x float> %f8, <8 x float> %f8)
; CHECK-NEXT:   %call06 = call <16 x float> @_Z6pow_rmDv16_fS_(<16 x float> %f16, <16 x float> %f16)
; CHECK-NEXT:   ret void

; CHECK:        declare float @_Z6pow_rmff(float, float)
; CHECK:        declare <2 x float> @_Z6pow_rmDv2_fS_(<2 x float>, <2 x float>)
; CHECK:        declare <3 x float> @_Z6pow_rmDv3_fS_(<3 x float>, <3 x float>)
; CHECK:        declare <4 x float> @_Z6pow_rmDv4_fS_(<4 x float>, <4 x float>)
; CHECK:        declare <8 x float> @_Z6pow_rmDv8_fS_(<8 x float>, <8 x float>)
; CHECK:        declare <16 x float> @_Z6pow_rmDv16_fS_(<16 x float>, <16 x float>)

; DEBUGIFY-NOT: WARNING

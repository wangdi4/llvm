; RUN: opt -relaxed-funcs -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

define void @check_log2_float(float %f1, <2 x float> %f2, <3 x float> %f3, <4 x float> %f4, <8 x float> %f8, <16 x float> %f16) nounwind {
entry:
    %call01 = call float @_Z4log2f(float %f1)
    %call02 = call <2 x float> @_Z4log2Dv2_f(<2 x float> %f2)
    %call03 = call <3 x float> @_Z4log2Dv3_f(<3 x float> %f3)
    %call04 = call <4 x float> @_Z4log2Dv4_f(<4 x float> %f4)
    %call05 = call <8 x float> @_Z4log2Dv8_f(<8 x float> %f8)
    %call06 = call <16 x float> @_Z4log2Dv16_f(<16 x float> %f16)
    ret void
}

declare float @_Z4log2f(float)
declare <2 x float> @_Z4log2Dv2_f(<2 x float>)
declare <3 x float> @_Z4log2Dv3_f(<3 x float>)
declare <4 x float> @_Z4log2Dv4_f(<4 x float>)
declare <8 x float> @_Z4log2Dv8_f(<8 x float>)
declare <16 x float> @_Z4log2Dv16_f(<16 x float>)

!opencl.compiler.options = !{!0}

!0 = !{!"-cl-std=CL2.0"}

; CHECK:        define void @check_log2_float
; CHECK:        entry:
; CHECK-NEXT:   %call01 = call float @_Z7log2_rmf(float %f1)
; CHECK-NEXT:   %call02 = call <2 x float> @_Z7log2_rmDv2_f(<2 x float> %f2)
; CHECK-NEXT:   %call03 = call <3 x float> @_Z7log2_rmDv3_f(<3 x float> %f3)
; CHECK-NEXT:   %call04 = call <4 x float> @_Z7log2_rmDv4_f(<4 x float> %f4)
; CHECK-NEXT:   %call05 = call <8 x float> @_Z7log2_rmDv8_f(<8 x float> %f8)
; CHECK-NEXT:   %call06 = call <16 x float> @_Z7log2_rmDv16_f(<16 x float> %f16)
; CHECK-NEXT:   ret void

; CHECK:        declare float @_Z7log2_rmf(float)
; CHECK:        declare <2 x float> @_Z7log2_rmDv2_f(<2 x float>)
; CHECK:        declare <3 x float> @_Z7log2_rmDv3_f(<3 x float>)
; CHECK:        declare <4 x float> @_Z7log2_rmDv4_f(<4 x float>)
; CHECK:        declare <8 x float> @_Z7log2_rmDv8_f(<8 x float>)
; CHECK:        declare <16 x float> @_Z7log2_rmDv16_f(<16 x float>)

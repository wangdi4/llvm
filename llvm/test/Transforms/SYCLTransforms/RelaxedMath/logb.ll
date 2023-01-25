; RUN: opt -passes=sycl-kernel-relaxed-math -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-relaxed-math -S %s | FileCheck %s


define void @check_logb(float %f1, <2 x float> %f2, <3 x float> %f3, <4 x float> %f4, <8 x float> %f8, <16 x float> %f16) nounwind {
entry:
    %call01 = call float @_Z4logbf(float %f1)
    %call02 = call <2 x float> @_Z4logbDv2_f(<2 x float> %f2)
    %call03 = call <3 x float> @_Z4logbDv3_f(<3 x float> %f3)
    %call04 = call <4 x float> @_Z4logbDv4_f(<4 x float> %f4)
    %call05 = call <8 x float> @_Z4logbDv8_f(<8 x float> %f8)
    %call06 = call <16 x float> @_Z4logbDv16_f(<16 x float> %f16)
    ret void
}

declare float @_Z4logbf(float)
declare <2 x float> @_Z4logbDv2_f(<2 x float>)
declare <3 x float> @_Z4logbDv3_f(<3 x float>)
declare <4 x float> @_Z4logbDv4_f(<4 x float>)
declare <8 x float> @_Z4logbDv8_f(<8 x float>)
declare <16 x float> @_Z4logbDv16_f(<16 x float>)

; CHECK:        define void @check_logb
; CHECK:        entry:
; CHECK-NEXT:   %call01 = call float @_Z11native_logbf(float %f1)
; CHECK-NEXT:   %call02 = call <2 x float> @_Z11native_logbDv2_f(<2 x float> %f2)
; CHECK-NEXT:   %call03 = call <3 x float> @_Z11native_logbDv3_f(<3 x float> %f3)
; CHECK-NEXT:   %call04 = call <4 x float> @_Z11native_logbDv4_f(<4 x float> %f4)
; CHECK-NEXT:   %call05 = call <8 x float> @_Z11native_logbDv8_f(<8 x float> %f8)
; CHECK-NEXT:   %call06 = call <16 x float> @_Z11native_logbDv16_f(<16 x float> %f16)
; CHECK-NEXT:   ret void

; CHECK:        declare float @_Z11native_logbf(float)
; CHECK:        declare <2 x float> @_Z11native_logbDv2_f(<2 x float>)
; CHECK:        declare <3 x float> @_Z11native_logbDv3_f(<3 x float>)
; CHECK:        declare <4 x float> @_Z11native_logbDv4_f(<4 x float>)
; CHECK:        declare <8 x float> @_Z11native_logbDv8_f(<8 x float>)
; CHECK:        declare <16 x float> @_Z11native_logbDv16_f(<16 x float>)

; DEBUGIFY-NOT: WARNING

; RUN: opt -passes=dpcpp-kernel-relaxed-math -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-relaxed-math -S %s | FileCheck %s

; RUN: opt -enable-new-pm=0 -dpcpp-kernel-relaxed-math -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -enable-new-pm=0 -dpcpp-kernel-relaxed-math -S %s | FileCheck %s

define void @check_acospi_float(float %f1, <2 x float> %f2, <3 x float> %f3, <4 x float> %f4, <8 x float> %f8, <16 x float> %f16) nounwind {
entry:
    %call01 = call float @_Z6acospif(float %f1)
    %call02 = call <2 x float> @_Z6acospiDv2_f(<2 x float> %f2)
    %call03 = call <3 x float> @_Z6acospiDv3_f(<3 x float> %f3)
    %call04 = call <4 x float> @_Z6acospiDv4_f(<4 x float> %f4)
    %call05 = call <8 x float> @_Z6acospiDv8_f(<8 x float> %f8)
    %call06 = call <16 x float> @_Z6acospiDv16_f(<16 x float> %f16)
    ret void
}

define void @check_acospi_double(double %d1, <2 x double> %d2, <3 x double> %d3, <4 x double> %d4, <8 x double> %d8, <16 x double> %d16) nounwind {
entry:
    %call01 = call double @_Z6acospid(double %d1)
    %call02 = call <2 x double> @_Z6acospiDv2_d(<2 x double> %d2)
    %call03 = call <3 x double> @_Z6acospiDv3_d(<3 x double> %d3)
    %call04 = call <4 x double> @_Z6acospiDv4_d(<4 x double> %d4)
    %call05 = call <8 x double> @_Z6acospiDv8_d(<8 x double> %d8)
    %call06 = call <16 x double> @_Z6acospiDv16_d(<16 x double> %d16)
    ret void
}

declare float @_Z6acospif(float)
declare <2 x float> @_Z6acospiDv2_f(<2 x float>)
declare <3 x float> @_Z6acospiDv3_f(<3 x float>)
declare <4 x float> @_Z6acospiDv4_f(<4 x float>)
declare <8 x float> @_Z6acospiDv8_f(<8 x float>)
declare <16 x float> @_Z6acospiDv16_f(<16 x float>)

declare double @_Z6acospid(double)
declare <2 x double> @_Z6acospiDv2_d(<2 x double>)
declare <3 x double> @_Z6acospiDv3_d(<3 x double>)
declare <4 x double> @_Z6acospiDv4_d(<4 x double>)
declare <8 x double> @_Z6acospiDv8_d(<8 x double>)
declare <16 x double> @_Z6acospiDv16_d(<16 x double>)

; CHECK:        define void @check_acospi_float
; CHECK:        entry:
; CHECK-NEXT:   %call01 = call float @_Z13native_acospif(float %f1)
; CHECK-NEXT:   %call02 = call <2 x float> @_Z13native_acospiDv2_f(<2 x float> %f2)
; CHECK-NEXT:   %call03 = call <3 x float> @_Z13native_acospiDv3_f(<3 x float> %f3)
; CHECK-NEXT:   %call04 = call <4 x float> @_Z13native_acospiDv4_f(<4 x float> %f4)
; CHECK-NEXT:   %call05 = call <8 x float> @_Z13native_acospiDv8_f(<8 x float> %f8)
; CHECK-NEXT:   %call06 = call <16 x float> @_Z13native_acospiDv16_f(<16 x float> %f16)
; CHECK-NEXT:   ret void

; CHECK:        define void @check_acospi_double
; CHECK:        entry:
; CHECK-NEXT:   %call01 = call double @_Z13native_acospid(double %d1)
; CHECK-NEXT:   %call02 = call <2 x double> @_Z13native_acospiDv2_d(<2 x double> %d2)
; CHECK-NEXT:   %call03 = call <3 x double> @_Z13native_acospiDv3_d(<3 x double> %d3)
; CHECK-NEXT:   %call04 = call <4 x double> @_Z13native_acospiDv4_d(<4 x double> %d4)
; CHECK-NEXT:   %call05 = call <8 x double> @_Z13native_acospiDv8_d(<8 x double> %d8)
; CHECK-NEXT:   %call06 = call <16 x double> @_Z13native_acospiDv16_d(<16 x double> %d16)
; CHECK-NEXT:   ret void

; CHECK:        declare float @_Z13native_acospif(float)
; CHECK:        declare <2 x float> @_Z13native_acospiDv2_f(<2 x float>)
; CHECK:        declare <3 x float> @_Z13native_acospiDv3_f(<3 x float>)
; CHECK:        declare <4 x float> @_Z13native_acospiDv4_f(<4 x float>)
; CHECK:        declare <8 x float> @_Z13native_acospiDv8_f(<8 x float>)
; CHECK:        declare <16 x float> @_Z13native_acospiDv16_f(<16 x float>)

; CHECK:        declare double @_Z13native_acospid(double)
; CHECK:        declare <2 x double> @_Z13native_acospiDv2_d(<2 x double>)
; CHECK:        declare <3 x double> @_Z13native_acospiDv3_d(<3 x double>)
; CHECK:        declare <4 x double> @_Z13native_acospiDv4_d(<4 x double>)
; CHECK:        declare <8 x double> @_Z13native_acospiDv8_d(<8 x double>)
; CHECK:        declare <16 x double> @_Z13native_acospiDv16_d(<16 x double>)

; DEBUGIFY-NOT: WARNING

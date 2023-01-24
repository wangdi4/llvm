; RUN: opt -passes=sycl-kernel-relaxed-math -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-relaxed-math -S %s | FileCheck %s


define void @check_powr_float(float %f1, <2 x float> %f2, <3 x float> %f3, <4 x float> %f4, <8 x float> %f8, <16 x float> %f16) nounwind {
entry:
    %call01 = call float @_Z4powrff(float %f1, float %f1)
    %call02 = call <2 x float> @_Z4powrDv2_fS_(<2 x float> %f2, <2 x float> %f2)
    %call03 = call <3 x float> @_Z4powrDv3_fS_(<3 x float> %f3, <3 x float> %f3)
    %call04 = call <4 x float> @_Z4powrDv4_fS_(<4 x float> %f4, <4 x float> %f4)
    %call05 = call <8 x float> @_Z4powrDv8_fS_(<8 x float> %f8, <8 x float> %f8)
    %call06 = call <16 x float> @_Z4powrDv16_fS_(<16 x float> %f16, <16 x float> %f16)
    ret void
}

define void @check_powr_double(double %d1, <2 x double> %d2, <3 x double> %d3, <4 x double> %d4, <8 x double> %d8, <16 x double> %d16) nounwind {
entry:
    %call01 = call double @_Z4powrdd(double %d1, double %d1)
    %call02 = call <2 x double> @_Z4powrDv2_dS_(<2 x double> %d2, <2 x double> %d2)
    %call03 = call <3 x double> @_Z4powrDv3_dS_(<3 x double> %d3, <3 x double> %d3)
    %call04 = call <4 x double> @_Z4powrDv4_dS_(<4 x double> %d4, <4 x double> %d4)
    %call05 = call <8 x double> @_Z4powrDv8_dS_(<8 x double> %d8, <8 x double> %d8)
    %call06 = call <16 x double> @_Z4powrDv16_dS_(<16 x double> %d16, <16 x double> %d16)
    ret void
}

declare float @_Z4powrff(float, float)
declare <2 x float> @_Z4powrDv2_fS_(<2 x float>, <2 x float>)
declare <3 x float> @_Z4powrDv3_fS_(<3 x float>, <3 x float>)
declare <4 x float> @_Z4powrDv4_fS_(<4 x float>, <4 x float>)
declare <8 x float> @_Z4powrDv8_fS_(<8 x float>, <8 x float>)
declare <16 x float> @_Z4powrDv16_fS_(<16 x float>, <16 x float>)

declare double @_Z4powrdd(double, double)
declare <2 x double> @_Z4powrDv2_dS_(<2 x double>, <2 x double>)
declare <3 x double> @_Z4powrDv3_dS_(<3 x double>, <3 x double>)
declare <4 x double> @_Z4powrDv4_dS_(<4 x double>, <4 x double>)
declare <8 x double> @_Z4powrDv8_dS_(<8 x double>, <8 x double>)
declare <16 x double> @_Z4powrDv16_dS_(<16 x double>, <16 x double>)

; CHECK:        define void @check_powr_float
; CHECK:        entry:
; CHECK-NEXT:   %call01 = call float @_Z11native_powrff(float %f1, float %f1)
; CHECK-NEXT:   %call02 = call <2 x float> @_Z11native_powrDv2_fS_(<2 x float> %f2, <2 x float> %f2)
; CHECK-NEXT:   %call03 = call <3 x float> @_Z11native_powrDv3_fS_(<3 x float> %f3, <3 x float> %f3)
; CHECK-NEXT:   %call04 = call <4 x float> @_Z11native_powrDv4_fS_(<4 x float> %f4, <4 x float> %f4)
; CHECK-NEXT:   %call05 = call <8 x float> @_Z11native_powrDv8_fS_(<8 x float> %f8, <8 x float> %f8)
; CHECK-NEXT:   %call06 = call <16 x float> @_Z11native_powrDv16_fS_(<16 x float> %f16, <16 x float> %f16)
; CHECK-NEXT:   ret void

; CHECK:        define void @check_powr_double
; CHECK:        entry:
; CHECK-NEXT:   %call01 = call double @_Z11native_powrdd(double %d1, double %d1)
; CHECK-NEXT:   %call02 = call <2 x double> @_Z11native_powrDv2_dS_(<2 x double> %d2, <2 x double> %d2)
; CHECK-NEXT:   %call03 = call <3 x double> @_Z11native_powrDv3_dS_(<3 x double> %d3, <3 x double> %d3)
; CHECK-NEXT:   %call04 = call <4 x double> @_Z11native_powrDv4_dS_(<4 x double> %d4, <4 x double> %d4)
; CHECK-NEXT:   %call05 = call <8 x double> @_Z11native_powrDv8_dS_(<8 x double> %d8, <8 x double> %d8)
; CHECK-NEXT:   %call06 = call <16 x double> @_Z11native_powrDv16_dS_(<16 x double> %d16, <16 x double> %d16)
; CHECK-NEXT:   ret void

; CHECK:        declare float @_Z11native_powrff(float, float)
; CHECK:        declare <2 x float> @_Z11native_powrDv2_fS_(<2 x float>, <2 x float>)
; CHECK:        declare <3 x float> @_Z11native_powrDv3_fS_(<3 x float>, <3 x float>)
; CHECK:        declare <4 x float> @_Z11native_powrDv4_fS_(<4 x float>, <4 x float>)
; CHECK:        declare <8 x float> @_Z11native_powrDv8_fS_(<8 x float>, <8 x float>)
; CHECK:        declare <16 x float> @_Z11native_powrDv16_fS_(<16 x float>, <16 x float>)

; CHECK:        declare double @_Z11native_powrdd(double, double)
; CHECK:        declare <2 x double> @_Z11native_powrDv2_dS_(<2 x double>, <2 x double>)
; CHECK:        declare <3 x double> @_Z11native_powrDv3_dS_(<3 x double>, <3 x double>)
; CHECK:        declare <4 x double> @_Z11native_powrDv4_dS_(<4 x double>, <4 x double>)
; CHECK:        declare <8 x double> @_Z11native_powrDv8_dS_(<8 x double>, <8 x double>)
; CHECK:        declare <16 x double> @_Z11native_powrDv16_dS_(<16 x double>, <16 x double>)

; DEBUGIFY-NOT: WARNING

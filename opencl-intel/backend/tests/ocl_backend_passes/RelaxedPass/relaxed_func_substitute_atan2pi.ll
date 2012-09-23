; RUN: opt -relaxed-funcs -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

define void @check_atan2pi_float(float %f1, <2 x float> %f2, <3 x float> %f3, <4 x float> %f4, <8 x float> %f8, <16 x float> %f16) nounwind {
entry:
    %call01 = call float @_Z7atan2piff(float %f1, float %f1)
    %call02 = call <2 x float> @_Z7atan2piDv2_fS_(<2 x float> %f2, <2 x float> %f2)
    %call03 = call <3 x float> @_Z7atan2piDv3_fS_(<3 x float> %f3, <3 x float> %f3)
    %call04 = call <4 x float> @_Z7atan2piDv4_fS_(<4 x float> %f4, <4 x float> %f4)
    %call05 = call <8 x float> @_Z7atan2piDv8_fS_(<8 x float> %f8, <8 x float> %f8)
    %call06 = call <16 x float> @_Z7atan2piDv16_fS_(<16 x float> %f16, <16 x float> %f16)
    ret void
}

define void @check_atan2pi_double(double %f1, <2 x double> %f2, <3 x double> %f3, <4 x double> %f4, <8 x double> %f8, <16 x double> %f16) nounwind {
entry:
    %call01 = call double @_Z7atan2pidd(double %f1, double %f1)
    %call02 = call <2 x double> @_Z7atan2piDv2_dS_(<2 x double> %f2, <2 x double> %f2)
    %call03 = call <3 x double> @_Z7atan2piDv3_dS_(<3 x double> %f3, <3 x double> %f3)
    %call04 = call <4 x double> @_Z7atan2piDv4_dS_(<4 x double> %f4, <4 x double> %f4)
    %call05 = call <8 x double> @_Z7atan2piDv8_dS_(<8 x double> %f8, <8 x double> %f8)
    %call06 = call <16 x double> @_Z7atan2piDv16_dS_(<16 x double> %f16, <16 x double> %f16)
    ret void
}

declare float @_Z7atan2piff(float, float)
declare <2 x float> @_Z7atan2piDv2_fS_(<2 x float>, <2 x float>)
declare <3 x float> @_Z7atan2piDv3_fS_(<3 x float>, <3 x float>)
declare <4 x float> @_Z7atan2piDv4_fS_(<4 x float>, <4 x float>)
declare <8 x float> @_Z7atan2piDv8_fS_(<8 x float>, <8 x float>)
declare <16 x float> @_Z7atan2piDv16_fS_(<16 x float>, <16 x float>)

declare double @_Z7atan2pidd(double, double)
declare <2 x double> @_Z7atan2piDv2_dS_(<2 x double>, <2 x double>)
declare <3 x double> @_Z7atan2piDv3_dS_(<3 x double>, <3 x double>)
declare <4 x double> @_Z7atan2piDv4_dS_(<4 x double>, <4 x double>)
declare <8 x double> @_Z7atan2piDv8_dS_(<8 x double>, <8 x double>)
declare <16 x double> @_Z7atan2piDv16_dS_(<16 x double>, <16 x double>)

; CHECK:        define void @check_atan2pi_float
; CHECK:        entry:
; CHECK-NEXT:   %call01 = call float @_Z14native_atan2piff(float %f1, float %f1)
; CHECK-NEXT:   %call02 = call <2 x float> @_Z14native_atan2piDv2_fS_(<2 x float> %f2, <2 x float> %f2)
; CHECK-NEXT:   %call03 = call <3 x float> @_Z14native_atan2piDv3_fS_(<3 x float> %f3, <3 x float> %f3)
; CHECK-NEXT:   %call04 = call <4 x float> @_Z14native_atan2piDv4_fS_(<4 x float> %f4, <4 x float> %f4)
; CHECK-NEXT:   %call05 = call <8 x float> @_Z14native_atan2piDv8_fS_(<8 x float> %f8, <8 x float> %f8)
; CHECK-NEXT:   %call06 = call <16 x float> @_Z14native_atan2piDv16_fS_(<16 x float> %f16, <16 x float> %f16)
; CHECK-NEXT:   ret void

; CHECK:        define void @check_atan2pi_double
; CHECK:        entry:
; CHECK-NEXT:   %call01 = call double @_Z14native_atan2pidd(double %f1, double %f1)
; CHECK-NEXT:   %call02 = call <2 x double> @_Z14native_atan2piDv2_dS_(<2 x double> %f2, <2 x double> %f2)
; CHECK-NEXT:   %call03 = call <3 x double> @_Z14native_atan2piDv3_dS_(<3 x double> %f3, <3 x double> %f3)
; CHECK-NEXT:   %call04 = call <4 x double> @_Z14native_atan2piDv4_dS_(<4 x double> %f4, <4 x double> %f4)
; CHECK-NEXT:   %call05 = call <8 x double> @_Z14native_atan2piDv8_dS_(<8 x double> %f8, <8 x double> %f8)
; CHECK-NEXT:   %call06 = call <16 x double> @_Z14native_atan2piDv16_dS_(<16 x double> %f16, <16 x double> %f16)
; CHECK-NEXT:   ret void

; CHECK:        declare float @_Z14native_atan2piff(float, float)
; CHECK:        declare <2 x float> @_Z14native_atan2piDv2_fS_(<2 x float>, <2 x float>)
; CHECK:        declare <3 x float> @_Z14native_atan2piDv3_fS_(<3 x float>, <3 x float>)
; CHECK:        declare <4 x float> @_Z14native_atan2piDv4_fS_(<4 x float>, <4 x float>)
; CHECK:        declare <8 x float> @_Z14native_atan2piDv8_fS_(<8 x float>, <8 x float>)
; CHECK:        declare <16 x float> @_Z14native_atan2piDv16_fS_(<16 x float>, <16 x float>)

; CHECK:        declare double @_Z14native_atan2pidd(double, double)
; CHECK:        declare <2 x double> @_Z14native_atan2piDv2_dS_(<2 x double>, <2 x double>)
; CHECK:        declare <3 x double> @_Z14native_atan2piDv3_dS_(<3 x double>, <3 x double>)
; CHECK:        declare <4 x double> @_Z14native_atan2piDv4_dS_(<4 x double>, <4 x double>)
; CHECK:        declare <8 x double> @_Z14native_atan2piDv8_dS_(<8 x double>, <8 x double>)
; CHECK:        declare <16 x double> @_Z14native_atan2piDv16_dS_(<16 x double>, <16 x double>)

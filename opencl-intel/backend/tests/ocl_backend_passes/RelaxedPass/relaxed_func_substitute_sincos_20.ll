; RUN: opt -relaxed-funcs -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

define void @check_sincos_float(float %f1, <2 x float> %f2, <3 x float> %f3, <4 x float> %f4, <8 x float> %f8, <16 x float> %f16,
                                float addrspace(1)* %pf1, <2 x float> addrspace(1)*  %pf2,
                                <3 x float> addrspace(1)* %pf3, <4 x float> addrspace(1)* %pf4,
                                <8 x float>  addrspace(1)* %pf8, <16 x float> addrspace(1)*  %pf16) nounwind {
entry:

    %call01 = call float @_Z6sincosfPU3AS1f(float %f1, float addrspace(1)* %pf1)
    %call02 = call <2 x float> @_Z6sincosDv2_fPU3AS1S_(<2 x float> %f2, <2 x float> addrspace(1)* %pf2)
    %call03 = call <3 x float> @_Z6sincosDv3_fPU3AS1S_(<3 x float> %f3, <3 x float> addrspace(1)* %pf3)
    %call04 = call <4 x float> @_Z6sincosDv4_fPU3AS1S_(<4 x float> %f4, <4 x float> addrspace(1)* %pf4)
    %call05 = call <8 x float> @_Z6sincosDv8_fPU3AS1S_(<8 x float> %f8, <8 x float> addrspace(1)* %pf8)
    %call06 = call <16 x float> @_Z6sincosDv16_fPU3AS1S_(<16 x float> %f16, <16 x float> addrspace(1)* %pf16)
    ret void
}


declare float @_Z6sincosfPU3AS1f(float, float addrspace(1)*)
declare <2 x float> @_Z6sincosDv2_fPU3AS1S_(<2 x float>, <2 x float> addrspace(1)*)
declare <3 x float> @_Z6sincosDv3_fPU3AS1S_(<3 x float>, <3 x float> addrspace(1)*)
declare <4 x float> @_Z6sincosDv4_fPU3AS1S_(<4 x float>, <4 x float> addrspace(1)*)
declare <8 x float> @_Z6sincosDv8_fPU3AS1S_(<8 x float>, <8 x float> addrspace(1)*)
declare <16 x float> @_Z6sincosDv16_fPU3AS1S_(<16 x float>, <16 x float> addrspace(1)*)

!opencl.compiler.options = !{!0}

!0 = !{!"-cl-std=CL2.0"}

; CHECK:        define void @check_sincos_float
; CHECK:        entry:
; CHECK-NEXT:   %call01 = call float @_Z9sincos_rmfPU3AS1f(float %f1, float addrspace(1)* %pf1)
; CHECK-NEXT:   %call02 = call <2 x float> @_Z9sincos_rmDv2_fPU3AS1S_(<2 x float> %f2, <2 x float> addrspace(1)* %pf2)
; CHECK-NEXT:   %call03 = call <3 x float> @_Z9sincos_rmDv3_fPU3AS1S_(<3 x float> %f3, <3 x float> addrspace(1)* %pf3)
; CHECK-NEXT:   %call04 = call <4 x float> @_Z9sincos_rmDv4_fPU3AS1S_(<4 x float> %f4, <4 x float> addrspace(1)* %pf4)
; CHECK-NEXT:   %call05 = call <8 x float> @_Z9sincos_rmDv8_fPU3AS1S_(<8 x float> %f8, <8 x float> addrspace(1)* %pf8)
; CHECK-NEXT:   %call06 = call <16 x float> @_Z9sincos_rmDv16_fPU3AS1S_(<16 x float> %f16, <16 x float> addrspace(1)* %pf16)
; CHECK-NEXT:   ret void


; CHECK:        declare float @_Z9sincos_rmfPU3AS1f(float, float addrspace(1)*)
; CHECK:        declare <2 x float> @_Z9sincos_rmDv2_fPU3AS1S_(<2 x float>, <2 x float> addrspace(1)*)
; CHECK:        declare <3 x float> @_Z9sincos_rmDv3_fPU3AS1S_(<3 x float>, <3 x float> addrspace(1)*)
; CHECK:        declare <4 x float> @_Z9sincos_rmDv4_fPU3AS1S_(<4 x float>, <4 x float> addrspace(1)*)
; CHECK:        declare <8 x float> @_Z9sincos_rmDv8_fPU3AS1S_(<8 x float>, <8 x float> addrspace(1)*)
; CHECK:        declare <16 x float> @_Z9sincos_rmDv16_fPU3AS1S_(<16 x float>, <16 x float> addrspace(1)*)

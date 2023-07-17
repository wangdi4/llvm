; RUN: opt -passes=sycl-kernel-relaxed-math -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-relaxed-math -S %s | FileCheck %s

define void @check_fract_float(float %f1, <2 x float> %f2, <3 x float> %f3, <4 x float> %f4, <8 x float> %f8, <16 x float> %f16) nounwind {
entry:
    %ptr1 = alloca float
    %ptr2 = alloca <2 x float>
    %ptr3 = alloca <3 x float>
    %ptr4 = alloca <4 x float>
    %ptr8 = alloca <8 x float>
    %ptr16 = alloca <16 x float>
    
    %call01 = call float @_Z5fractfPf(float %f1, ptr %ptr1)
    %call02 = call <2 x float> @_Z5fractDv2_fPS_(<2 x float> %f2, ptr %ptr2)
    %call03 = call <3 x float> @_Z5fractDv3_fPS_(<3 x float> %f3, ptr %ptr3)
    %call04 = call <4 x float> @_Z5fractDv4_fPS_(<4 x float> %f4, ptr %ptr4)
    %call05 = call <8 x float> @_Z5fractDv8_fPS_(<8 x float> %f8, ptr %ptr8)
    %call06 = call <16 x float> @_Z5fractDv16_fPS_(<16 x float> %f16, ptr %ptr16)
    ret void
}

declare float @_Z5fractfPf(float, ptr)
declare <2 x float> @_Z5fractDv2_fPS_(<2 x float>, ptr)
declare <3 x float> @_Z5fractDv3_fPS_(<3 x float>, ptr)
declare <4 x float> @_Z5fractDv4_fPS_(<4 x float>, ptr)
declare <8 x float> @_Z5fractDv8_fPS_(<8 x float>, ptr)
declare <16 x float> @_Z5fractDv16_fPS_(<16 x float>, ptr)

; CHECK:        define void @check_fract_float
; CHECK:        entry:
; CHECK-NEXT:   %ptr1 = alloca float
; CHECK-NEXT:   %ptr2 = alloca <2 x float>
; CHECK-NEXT:   %ptr3 = alloca <3 x float>
; CHECK-NEXT:   %ptr4 = alloca <4 x float>
; CHECK-NEXT:   %ptr8 = alloca <8 x float>
; CHECK-NEXT:   %ptr16 = alloca <16 x float>
; CHECK-NEXT:   %call01 = call float @_Z12native_fractfPf(float %f1, ptr %ptr1)
; CHECK-NEXT:   %call02 = call <2 x float> @_Z12native_fractDv2_fPS_(<2 x float> %f2, ptr %ptr2)
; CHECK-NEXT:   %call03 = call <3 x float> @_Z12native_fractDv3_fPS_(<3 x float> %f3, ptr %ptr3)
; CHECK-NEXT:   %call04 = call <4 x float> @_Z12native_fractDv4_fPS_(<4 x float> %f4, ptr %ptr4)
; CHECK-NEXT:   %call05 = call <8 x float> @_Z12native_fractDv8_fPS_(<8 x float> %f8, ptr %ptr8)
; CHECK-NEXT:   %call06 = call <16 x float> @_Z12native_fractDv16_fPS_(<16 x float> %f16, ptr %ptr16)
; CHECK-NEXT:   ret void

; CHECK:        declare float @_Z12native_fractfPf(float, ptr)
; CHECK:        declare <2 x float> @_Z12native_fractDv2_fPS_(<2 x float>, ptr)
; CHECK:        declare <3 x float> @_Z12native_fractDv3_fPS_(<3 x float>, ptr)
; CHECK:        declare <4 x float> @_Z12native_fractDv4_fPS_(<4 x float>, ptr)
; CHECK:        declare <8 x float> @_Z12native_fractDv8_fPS_(<8 x float>, ptr)
; CHECK:        declare <16 x float> @_Z12native_fractDv16_fPS_(<16 x float>, ptr)

; DEBUGIFY-NOT: WARNING

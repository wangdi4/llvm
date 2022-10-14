; REQUIRES: asserts
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-outofboundsok=false -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; This test verifies that BadCasting is not set for parent struct
; (i.e %DOMDocumentImpl) even though expected type (%DOMNode*) and used
; type (%DOMElement*) of %arg1 don't match. The parent class is not involved
; in any BadCasting in this test as it is used only to get address of a field.
; Only either %arg or %i is involved in BadCasting. Skip BadCasting for
; parent struct only when expected type is at zero offset of used type.
; (Special case).

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%DOMDocumentImpl = type <{ i64, ptr }>
%DOMNode = type { ptr }
%DOMElement = type { %DOMNode }

define void @_ZN11xercesc_2_715DOMDocumentImpl12insertBeforeEPNS_7DOMNodeES2_(ptr "intel_dtrans_func_index"="1" %arg, ptr "intel_dtrans_func_index"="2" %arg1) !intel.dtrans.func.type !9 {
bb:
  %i = getelementptr inbounds %DOMDocumentImpl, ptr %arg, i64 0, i32 1
  store ptr %arg1, ptr %i, align 8
  ret void
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %DOMDocumentImpl = type <{ i64, ptr }>
; CHECK: Safety data: Mismatched element access{{ *$}}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %DOMElement = type { %DOMNode }
; CHECK: Safety data: Mismatched element access | Unsafe pointer store | Contains nested structure{{ *$}}

; CHECK: DTRANS_StructInfo:
; CHECK:  LLVMType: %DOMNode = type { ptr }
; CHECK: Safety data: Bad casting | Mismatched element access | Unsafe pointer store | Nested structure | Has vtable{{ *$}}


!intel.dtrans.types = !{!0, !4, !7}

!0 = !{!"S", %DOMNode zeroinitializer, i32 1, !1}
!1 = !{!2, i32 2}
!2 = !{!"F", i1 true, i32 0, !3}
!3 = !{i32 0, i32 0}
!4 = !{!"S", %DOMDocumentImpl zeroinitializer, i32 2, !5, !6}
!5 = !{i64 0, i32 0}
!6 = !{%DOMElement zeroinitializer, i32 1}
!7 = !{!"S", %DOMElement zeroinitializer, i32 1, !8}
!8 = !{%DOMNode zeroinitializer, i32 0}
!9 = distinct !{!10, !11}
!10 = !{%DOMDocumentImpl zeroinitializer, i32 1}
!11 = !{%DOMNode zeroinitializer, i32 1}

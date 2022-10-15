; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test a case where there are multiple aggregate type pointers into the select
; statement, but the merge is safe because they represent aliases of the same
; base type due to element zero of the derived type being the base type.

%class.test01.base = type { i32, %class.test01.base* }
%class.test01.derived = type { %class.test01.base, i32 }
define internal void @test01(%class.test01.derived* "intel_dtrans_func_index"="1" %pDerived) !intel.dtrans.func.type !5 {
  %pField = getelementptr %class.test01.derived, %class.test01.derived* %pDerived, i64 0, i32 0, i32 1
  %pField.as.pDerived = bitcast %class.test01.base** %pField to %class.test01.derived**
  %pDerived2 = load %class.test01.derived*, %class.test01.derived** %pField.as.pDerived

  ; %pDerived2 will have both the base type and the derived type associated with
  ; it. For the purpose of the merge it is safe, because both operands have the
  ; same dominant type, so there will not be an 'Unsafe pointer merge'. However,
  ; the load will have triggered safety violations.
  %merge = select i1 undef, %class.test01.derived* %pDerived, %class.test01.derived* %pDerived2
  call void @test01helper(%class.test01.derived* %merge)
  ret void
}

define internal void @test01helper(%class.test01.derived* "intel_dtrans_func_index"="1" %pDerived) !intel.dtrans.func.type !6 {
  %pField = getelementptr %class.test01.derived, %class.test01.derived* %pDerived, i64 0, i32 1
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %class.test01.base
; CHECK: Safety data: Bad casting | Mismatched element access | Nested structure{{ *$}}
; CHECK: End LLVMType: %class.test01.base

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %class.test01.derived
; CHECK: Safety data: Bad casting | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %class.test01.derived


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%class.test01.base zeroinitializer, i32 1}  ; %class.test01.base*
!3 = !{%class.test01.base zeroinitializer, i32 0}  ; %class.test01.base
!4 = !{%class.test01.derived zeroinitializer, i32 1}  ; %class.test01.derived*
!5 = distinct !{!4}
!6 = distinct !{!4}
!7 = !{!"S", %class.test01.base zeroinitializer, i32 2, !1, !2} ; { i32, %class.test01.base* }
!8 = !{!"S", %class.test01.derived zeroinitializer, i32 2, !3, !1} ; { %class.test01.base, i32 }

!intel.dtrans.types = !{!7, !8}

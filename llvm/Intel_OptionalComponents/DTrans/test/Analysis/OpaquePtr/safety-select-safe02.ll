; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test a case where there are multiple aggregate type pointers into the select
; statement, but the merge is safe because they represent aliases of the same
; base type due to element zero of the derived type being the base type.

%class.test01.base = type { i32, %class.test01.base* }
%class.test01.derived = type { %class.test01.base, i32 }
define internal void @test01(%class.test01.derived* %pDerived) !dtrans_type !4 {
  %pField = getelementptr %class.test01.derived, %class.test01.derived* %pDerived, i64 0, i32 0, i32 1
  %pField.as.pDerived = bitcast %class.test01.base** %pField to %class.test01.derived**
  %pDerived2 = load %class.test01.derived*, %class.test01.derived** %pField.as.pDerived

  ; %pDerived2 will have both the base type and the derived type associated with it.
  ; For the purpose of the merge it is safe, because both operands have the same
  ; dominant type.
  %merge = select i1 undef, %class.test01.derived* %pDerived, %class.test01.derived* %pDerived2
  call void @test01helper(%class.test01.derived* %merge)
  ret void
}

define internal void @test01helper(%class.test01.derived* %pDerived) !dtrans_type !4 {
  %pField = getelementptr %class.test01.derived, %class.test01.derived* %pDerived, i64 0, i32 1
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: class.test01.base
; CHECK: Safety data: Nested structure

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: class.test01.derived
; CHECK: Safety data: Contains nested structure


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!3, i32 1}  ; %class.test01.base*
!3 = !{!"R", %class.test01.base zeroinitializer, i32 0}  ; %class.test01.base
!4 = !{!"F", i1 false, i32 1, !5, !6}  ; void (%class.test01.derived*)
!5 = !{!"void", i32 0}  ; void
!6 = !{!7, i32 1}  ; %class.test01.derived*
!7 = !{!"R", %class.test01.derived zeroinitializer, i32 0}  ; %class.test01.derived
!8 = !{!"S", %class.test01.base zeroinitializer, i32 2, !1, !2} ; { i32, %class.test01.base* }
!9 = !{!"S", %class.test01.derived zeroinitializer, i32 2, !3, !1} ; { %class.test01.base, i32 }

!dtrans_types = !{!8, !9}

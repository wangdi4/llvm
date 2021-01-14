; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Calling a function using an alias to the function which appears to be a
; properly typed pointer to the structure. However, this case is not safe
; for DTrans because the alias is defined using a weak alias linkage.
%struct.test01 = type { i32, i32 }
@f01_alias = weak alias void (%struct.test01*), void (%struct.test01*)* @f01

define internal void @f01(%struct.test01* %s) !dtrans_type !2 {
  %p = getelementptr %struct.test01, %struct.test01* %s, i64 0, i32 0
  %i = load i32, i32* %p
  ret void
}

define void @test01(%struct.test01* %s) !dtrans_type !2 {
  call void @f01_alias(%struct.test01* %s)
  ret void
}

; TODO: This case should be marked as "Address taken". However, the
; PointerTypeAnalyzer will mark the arguments to a weakly defined alias as
; "Unhandled", so this will currently be marked as "Unhandled use". We don't
; expect to see "weak aliases" when we have whole program, so the important
; thing is just that they not be treated as safe, if we do.

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: Unhandled use{{ *$}}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !3, !4}  ; void (%struct.test01*)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!6}

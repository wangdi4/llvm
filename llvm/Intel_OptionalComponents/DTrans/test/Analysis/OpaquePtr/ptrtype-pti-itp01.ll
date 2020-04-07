; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type recovery for "ptrtoint" and "inttoptr" instructions

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; Simple test of type propagation tracking through "ptrtoint" and "inttoptr"
; instructions.
%struct.test01 = type { i32, i32}
define internal void @test01(%struct.test01* %in) !dtrans_type !1 {
  %i = ptrtoint %struct.test01* %in to i64
  %p = inttoptr i64 %i to %struct.test01*
  ret void
}
; CHECK-LABEL: void @test01
; CHECK-CUR:  %i = ptrtoint %struct.test01* %in to i64
; CHECK-FUT:  %i = ptrtoint p0 %in to i64
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   %struct.test01*
; CHECK-NEXT: No element pointees.

; CHECK-CUR:  %p = inttoptr i64 %i to %struct.test01*
; CHECK-FUT:  %p = inttoptr i64 %i to p0
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   %struct.test01*
; CHECK-NEXT: No element pointees.


; Test of type propagation for a case that safety analysis will later
; need to detect as unsafe.
%struct.test02 = type { i32, i32}
define internal void @test02(%struct.test02* %in) !dtrans_type !5 {
  ; The Conversion results in truncation of value. For the purpose of type
  ; recovery, we will track it as the original pointer type, and leave it
  ; to the safety analysis to mark it as unsafe.
  %i = ptrtoint %struct.test02* %in to i8
  %p = inttoptr i8 %i to %struct.test02*
  ret void
}
; CHECK-LABEL: void @test02
; CHECK-CUR:  %i = ptrtoint %struct.test02* %in to i8
; CHECK-FUT:  %i = ptrtoint p0 %in to i8
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:    %struct.test02*
; CHECK-NEXT: No element pointees.

; CHECK-CUR:  %p = inttoptr i8 %i to %struct.test02*
; CHECK-FUT:  %p = inttoptr i8 %i to p0
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   %struct.test02*
; CHECK-NEXT: No element pointees.


; Test a case that should be detected as UNHANDLED with the current
; analysis because only simple backtracking is done for the integer
; value of an "inttoptr" instruction.
%struct.test03 = type { i32, i32}
define internal void @test03(%struct.test03* %in) !dtrans_type !8 {
  %i = ptrtoint %struct.test03* %in to i64
  %r8 = ashr i64 %i, 8
  %l8 = shl i64 %r8, 8
  %a8 = add i64 %l8, 8

  ; For this case, we mark the result as unhandled. More tracking of the
  ; source of %a8 would be needed to handle it.
  %p = inttoptr i64 %a8 to %struct.test03*

  ret void
}
; CHECK-LABEL: void @test03
; CHECK-CUR:  %i = ptrtoint %struct.test03* %in to i64
; CHECK-FUT:  %i = ptrtoint p0 %in to i64
; CHECK:     LocalPointerInfo:
; CHECK:       Aliased types:
; CHECK-NEXT:   %struct.test03*
; CHECK-NEXT: No element pointees.

; CHECK-CUR:  %p = inttoptr i64 %a8 to %struct.test03*
; CHECK-FUT:  %p = inttoptr i64 %a8 to p0
; CHECK:     LocalPointerInfo:
; CHECK-SAME: <UNHANDLED>


!1 = !{!"F", i1 false, i32 1, !2, !3}  ; void (%struct.test01*)
!2 = !{!"void", i32 0}  ; void
!3 = !{!4, i32 1}  ; %struct.test01*
!4 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!5 = !{!"F", i1 false, i32 1, !2, !6}  ; void (%struct.test02*)
!6 = !{!7, i32 1}  ; %struct.test02*
!7 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!8 = !{!"F", i1 false, i32 1, !2, !9}  ; void (%struct.test03*)
!9 = !{!10, i32 1}  ; %struct.test03*
!10 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!11 = !{i32 0, i32 0}  ; i32
!12 = !{!"S", %struct.test01 zeroinitializer, i32 2, !11, !11} ; { i32, i32}
!13 = !{!"S", %struct.test02 zeroinitializer, i32 2, !11, !11} ; { i32, i32}
!14 = !{!"S", %struct.test03 zeroinitializer, i32 2, !11, !11} ; { i32, i32}

!dtrans_types = !{!12, !13, !14}

; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery on getelementptr instructions

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

%struct.test01 = type { i64, %struct.test01* }

define void @test01(%struct.test01* %in)  !dtrans_type !1 {
  %f0 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 0
  %v0 = load i64, i64* %f0

  %f1 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
  %v1 = load %struct.test01*, %struct.test01** %f1

  ret void
}

; TODO: These currently report UNHANDLED because getelementptr analysis
; is not implemented yet. This should go away after that analysis is
; implemented.

; CHECK-LABEL: Input Parameters: test01
; CHECK-CUR: Arg 0: %struct.test01* %in
; CHECK-FUT: Arg 0: p0 %in
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   %struct.test01*
; CHECK-NEXT: No element pointees.

; CHECK-CUR: %f0 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 0
; CHECK-FUT: %f0 = getelementptr %struct.test01, p0 %in, i64 0, i32 0
; CHECK:    LocalPointerInfo:
; CHECK-SAME: UNHANDLED

; CHECK-CUR: %v0 = load i64, i64* %f0
; CHECK-FUT: %v0 = load i64, p0 %f0
; CHECK-CUR: %f1 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
; CHECK-FUT: %f1 = getelementptr %struct.test01, p0 %in, i64 0, i32 1
; CHECK:    LocalPointerInfo:
; CHECK-SAME: UNHANDLED

; CHECK-CUR: %v1 = load %struct.test01*, %struct.test01** %f1
; CHECK-FUT: %v1 = load p0, p0 %f1
; CHECK:    LocalPointerInfo:
; CHECK-SAME: DEPENDS ON UNHANDLED


!1 = !{!"F", i1 false, i32 1, !2, !3}  ; void (%struct.test01*)
!2 = !{!"void", i32 0}  ; void
!3 = !{!4, i32 1}  ; %struct.test01*
!4 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!5 = !{i64 0, i32 0}  ; i64
!6 = !{!"S", %struct.test01  zeroinitializer, i32 2, !5, !3} ; { i64, %struct.test01* }

!dtrans_types = !{!6}

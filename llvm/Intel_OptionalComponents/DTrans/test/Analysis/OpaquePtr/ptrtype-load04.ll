; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type recovery on load of a pointer to a structure
; done as a pointer-sized integer.

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; This test requires the data layout to establish i64 as the
; type to use for a pointer-sized integer.
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test01 = type { i64, %struct.test01* }
define internal void @test01(%struct.test01* %in) !dtrans_type !4 {
  %f0 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 0
  ; This is a pointer-sized int that does NOT involve a pointer alias.
  %v0 = load i64, i64* %f0

  %f1 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
  %bc = bitcast %struct.test01** %f1 to i64*
  ; This is a pointer-sized int that does involve a pointer alias.
  %v1 = load i64, i64* %bc
  ret void
}
; CHECK-LABEL: define internal void @test01

; We do not expect pointer info to be emitted for this load
; because there is not a pointer or aggregate type alias.
; CHECK-CUR: %v0 = load i64, i64* %f0
; CHECK-FUT: %v0 = load i64, p0 %f0
; CHECK-NOT:    LocalPointerInfo:
; CHECK-CUR: %f1 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
; CHECK-FUT: %f1 = getelementptr %struct.test01, p0 %in, i64 0, i32 1

; CHECK-CUR:  %bc = bitcast %struct.test01** %f1 to i64*
; CHECK-FUT:  %bc = bitcast p0 %f1 to p0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01**{{ *$}}
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test01 @ 1

; This case should have info reported because there is a pointer alias.
; CHECK-CUR:  %v1 = load i64, i64* %bc
; CHECK-FUT:  %v1 = load i64, p0 %bc
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01*{{ *$}}
; CHECK-NEXT: No element pointees.


!1 = !{i64 0, i32 0}  ; i64
!2 = !{!3, i32 1}  ; %struct.test01*
!3 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!4 = !{!"F", i1 false, i32 1, !5, !2}  ; void (%struct.test01*)
!5 = !{!"void", i32 0}  ; void
!6 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i64, %struct.test01* }

!dtrans_types = !{!6}

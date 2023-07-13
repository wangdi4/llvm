; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery on load of a pointer to a structure
; done as a pointer-sized integer.


; This test requires the data layout to establish i64 as the
; type to use for a pointer-sized integer.
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test01 = type { i64, ptr }
define internal void @test01(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !3 {
  %f0 = getelementptr %struct.test01, ptr %in, i64 0, i32 0
  ; This is a pointer-sized int that does NOT involve a pointer alias.
  %v0 = load i64, ptr %f0

  %f1 = getelementptr %struct.test01, ptr %in, i64 0, i32 1
  %bc = bitcast ptr %f1 to ptr
  ; This is a pointer-sized int that does involve a pointer alias.
  %v1 = load i64, ptr %bc
  ret void
}
; CHECK-LABEL: define internal void @test01

; We do not expect pointer info to be emitted for this load
; because there is not a pointer or aggregate type alias.
; CHECK: %v0 = load i64, ptr %f0
; CHECK-NOT:    LocalPointerInfo:
; CHECK: %f1 = getelementptr %struct.test01, ptr %in, i64 0, i32 1

; CHECK:  %bc = bitcast ptr %f1 to ptr
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01**{{ *$}}
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test01 @ 1

; This case should have info reported because there is a pointer alias.
; CHECK:  %v1 = load i64, ptr %bc
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01*{{ *$}}
; CHECK-NEXT: No element pointees.


!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i64, %struct.test01* }

!intel.dtrans.types = !{!4}

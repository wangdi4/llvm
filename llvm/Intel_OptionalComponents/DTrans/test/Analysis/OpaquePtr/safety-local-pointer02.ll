; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test of processing alloca instruction with vector types. DTrans safety checking
; does not model vector instructions, so these cases are just to ensure that they
; do not cause compilation failures.

; Allocating a vector of pointers to structures is currently unhandled.
%struct.test01 = type { i32, i32 }
define void @test01() {
  %local = alloca <2 x %struct.test01*>, !intel_dtrans_type !2
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Unhandled use
; CHECK: End LLVMType: %struct.test01

; Allocating an array of a vector of pointers to structures is currently
; unhandled.
%struct.test02 = type { i32, i32 }
define void @test02() {
  %local = alloca [4 x <2 x %struct.test02*>], !intel_dtrans_type !4
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Unhandled use
; CHECK: End LLVMType: %struct.test02


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"V", i32 2, !3}  ; <2 x %struct.test01*>
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = !{!"A", i32 4, !5}  ; [4 x <2 x %struct.test02*>]
!5 = !{!"V", i32 2, !6}  ; <2 x %struct.test02*>
!6 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!7 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!8 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!7, !8}

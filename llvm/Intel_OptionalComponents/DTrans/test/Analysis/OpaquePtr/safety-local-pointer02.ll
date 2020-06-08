; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test of processing alloca instruction with vector types. DTrans safety checking
; does not model vector instructions, so these cases are just to ensure that they
; do not cause compilation failures.

; Allocating a vector of pointers to structures is currently unhandled.
%struct.test01 = type { i32, i32 }
define void @test01() {
  %local = alloca <2 x %struct.test01*>, !dtrans_type !2
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: Unhandled use

; Allocating an array of a vector of pointers to structures is currently
; unhandled.
%struct.test02 = type { i32, i32 }
define void @test02() {
  %local = alloca [4 x <2 x %struct.test02*>], !dtrans_type !5
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Unhandled use


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"V", i32 2, !3}  ; <2 x %struct.test01*>
!3 = !{!4, i32 1}  ; %struct.test01*
!4 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!5 = !{!"A", i32 4, !6}  ; [4 x <2 x %struct.test02*>]
!6 = !{!"V", i32 2, !7}  ; <2 x %struct.test02*>
!7 = !{!8, i32 1}  ; %struct.test02*
!8 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!9 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!10 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!9, !10}

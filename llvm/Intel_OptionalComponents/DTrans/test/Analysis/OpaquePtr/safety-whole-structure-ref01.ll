; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test detection of 'Whole structure reference' safety condition on 'load'
; instructions.

; Test whole structure load of a member field
%struct.test01a = type { %struct.test01b }
%struct.test01b = type { i32, i32 }
define void @test01(%struct.test01a* %p) !dtrans_type !3 {
  %nested = getelementptr %struct.test01a, %struct.test01a* %p, i64 0, i32 0
  %t = load %struct.test01b, %struct.test01b* %nested
  ret void
}
; The "whole structure reference" should not propagate to the outer structure.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: 0)Field LLVM Type: %struct.test01b
; CHECK: Field info: Read{{ *$}}
; CHECK: Safety data: Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Read{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Read{{ *$}}
; CHECK: Safety data: Whole structure reference | Nested structure{{ *$}}


; Test whole structure load of a pointer that is not for a member field
%struct.test02 = type { i32, i32 }
define void @test02(%struct.test02* %p) !dtrans_type !7 {
  %t = load %struct.test02, %struct.test02* %p
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Read{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Read{{ *$}}
; CHECK: Safety data: Whole structure reference{{ *$}}


!1 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!"F", i1 false, i32 1, !4, !5}  ; void (%struct.test01a*)
!4 = !{!"void", i32 0}  ; void
!5 = !{!6, i32 1}  ; %struct.test01a*
!6 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!7 = !{!"F", i1 false, i32 1, !4, !8}  ; void (%struct.test02*)
!8 = !{!9, i32 1}  ; %struct.test02*
!9 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!10 = !{!"S", %struct.test01a zeroinitializer, i32 1, !1} ; { %struct.test01b }
!11 = !{!"S", %struct.test01b zeroinitializer, i32 2, !2, !2} ; { i32, i32 }
!12 = !{!"S", %struct.test02 zeroinitializer, i32 2, !2, !2} ; { i32, i32 }

!dtrans_types = !{!10, !11, !12}

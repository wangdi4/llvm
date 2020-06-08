; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test setting of "Local instance" safety bit


; This case should set "Local instance" on the allocated type, and the nested
; types, but not on the pointed-to types.
%struct.test01a = type { i32*, %struct.test01b, %struct.test01d* }
%struct.test01b = type { i32, %struct.test01c }
%struct.test01c = type { i32 }
%struct.test01d = type { i32, %struct.test01e }
%struct.test01e = type { i32 }
define void @test01() {
  %local = alloca %struct.test01a
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Contains nested structure | Local instance{{ *$}}

; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Nested structure | Contains nested structure | Local instance{{ *$}}

; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01c
; CHECK: Safety data: Nested structure | Local instance{{ *$}}

; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01d
; CHECK: Safety data: Contains nested structure{{ *$}}

; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01e
; CHECK: Safety data: Nested structure{{ *$}}


; Allocation of array of structures should be "Local instance"
%struct.test02 = type { i32, i32 }
define void @test02() {
  %local = alloca [4 x %struct.test02]
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Local instance{{ *$}}


!1 = !{i32 0, i32 1}  ; i32*
!2 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!3 = !{!4, i32 1}  ; %struct.test01d*
!4 = !{!"R", %struct.test01d zeroinitializer, i32 0}  ; %struct.test01d
!5 = !{i32 0, i32 0}  ; i32
!6 = !{!"R", %struct.test01c zeroinitializer, i32 0}  ; %struct.test01c
!7 = !{!"R", %struct.test01e zeroinitializer, i32 0}  ; %struct.test01e
!8 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !2, !3} ; { i32*, %struct.test01b, %struct.test01d* }
!9 = !{!"S", %struct.test01b zeroinitializer, i32 2, !5, !6} ; { i32, %struct.test01c }
!10 = !{!"S", %struct.test01c zeroinitializer, i32 1, !5} ; { i32 }
!11 = !{!"S", %struct.test01d zeroinitializer, i32 2, !5, !7} ; { i32, %struct.test01e }
!12 = !{!"S", %struct.test01e zeroinitializer, i32 1, !5} ; { i32 }
!13 = !{!"S", %struct.test02 zeroinitializer, i32 2, !5, !5} ; { i32, i32 }

!dtrans_types = !{!8, !9, !10, !11, !12, !13}

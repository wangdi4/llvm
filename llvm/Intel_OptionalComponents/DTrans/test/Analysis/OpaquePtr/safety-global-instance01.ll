; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test of setting the "Global instance" safety flag.

%struct.test01a = type { i32*, %struct.test01b, %struct.test01d* }
%struct.test01b = type { i32, %struct.test01c }
%struct.test01c = type { i32 }
%struct.test01d = type { i32, %struct.test01e }
%struct.test01e = type { i32 }

@globalStruct = internal global %struct.test01a zeroinitializer

; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Global instance | Contains nested structure{{ *$}}

; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Global instance | Nested structure | Contains nested structure{{ *$}}

; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01c
; CHECK: Safety data: Global instance | Nested structure{{ *$}}

; TODO: Even though there is a pointer of this type in a global instantiation,
; this is not treated as having the "Global pointer" safety flag because
; a direct instantiation of a pointer to the type is not seen. This should
; be changed in the future to record that there is effectively a global
; pointer to this type.
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01d
; CHECK: Safety data: Contains nested structure{{ *$}}

; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01e
; CHECK: Safety data: Nested structure{{ *$}}


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

!dtrans_types = !{!8, !9, !10, !11, !12}

; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test setting of "Global pointer" safety bit.

%struct.test01 = type { i32, i32 }
@global_ptr_to_struct = internal global %struct.test01* zeroinitializer, !dtrans_type !2
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: Global pointer

%struct.test02 = type { i32, i32 }
@global_ptr_to_ptr_to_struct = internal global %struct.test02** zeroinitializer, !dtrans_type !4

; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Global pointer

%struct.test03a = type { %struct.test03b, %struct.test03c* }
%struct.test03b = type { i32, i32 }
%struct.test03c = type { i32, i32 }
@global_ptr_struct3 =  internal global %struct.test03a* zeroinitializer, !dtrans_type !9
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test03a
; CHECK: Safety data: Global pointer

; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test03b
; CHECK: Safety data: Global pointer

; "Global pointer" is not pointer-carried to referenced types.
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test03c
; CHECK: Safety data: No issues found

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!3, i32 1}  ; %struct.test01*
!3 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!4 = !{!5, i32 2}  ; %struct.test02**
!5 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!6 = !{!"R", %struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!7 = !{!8, i32 1}  ; %struct.test03c*
!8 = !{!"R", %struct.test03c zeroinitializer, i32 0}  ; %struct.test03c
!9 = !{!10, i32 1}  ; %struct.test03a*
!10 = !{!"R", %struct.test03a zeroinitializer, i32 0}  ; %struct.test03a
!11 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!12 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!13 = !{!"S", %struct.test03a zeroinitializer, i32 2, !6, !7} ; { %struct.test03b, %struct.test03c* }
!14 = !{!"S", %struct.test03b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!15 = !{!"S", %struct.test03c zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!11, !12, !13, !14, !15}

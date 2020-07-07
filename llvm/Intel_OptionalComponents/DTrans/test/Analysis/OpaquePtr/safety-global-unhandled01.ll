; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; These are cases that the safety analyzer currently does not handle to ensure
; they are detected. As more support is developed, some of these cases may be
; removed from this test.

; Vector types are not supported
%struct.test01 = type { i32, i32 }
@global_array_of_vector_ptrs = internal global [16 x <2 x %struct.test01*>] zeroinitializer, !dtrans_type !2
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data:{{.*}}Unhandled use{{.*}}

; Thread locals variables are conservatively marked as unhandled.
%struct.test02 = type { i32, i32 }
@global_thread_local = thread_private global %struct.test02 zeroinitializer
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data:{{.*}}Unhandled use{{.*}}

; Non-local linkage variables should not get transformed.
%struct.test03 = type { i32, i32 }
@global_weak = weak global %struct.test03 zeroinitializer
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: Safety data:{{.*}}Unhandled use{{.*}}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"A", i32 16, !3}  ; [16 x <2 x %struct.test01*>]
!3 = !{!"V", i32 2, !4}  ; <2 x %struct.test01*>
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!7 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!8 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!6, !7, !8}

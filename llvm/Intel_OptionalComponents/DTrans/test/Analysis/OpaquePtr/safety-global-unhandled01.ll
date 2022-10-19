; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; These are cases that the safety analyzer currently does not handle to ensure
; they are detected. As more support is developed, some of these cases may be
; removed from this test.

; Vector types are not supported
%struct.test01 = type { i32, i32 }
@global_array_of_vector_ptrs = internal global [16 x <2 x %struct.test01*>] zeroinitializer, !intel_dtrans_type !2
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data:{{.*}}Unhandled use{{.*}}
; CHECK: End LLVMType: %struct.test01

; Thread locals variables are conservatively marked as unhandled.
%struct.test02 = type { i32, i32 }
@global_thread_local = thread_private global %struct.test02 zeroinitializer
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data:{{.*}}Unhandled use{{.*}}
; CHECK: End LLVMType: %struct.test02

; Non-local linkage variables should not get transformed.
%struct.test03 = type { i32, i32 }
@global_weak = weak global %struct.test03 zeroinitializer
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data:{{.*}}Unhandled use{{.*}}
; CHECK: End LLVMType: %struct.test03

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"A", i32 16, !3}  ; [16 x <2 x %struct.test01*>]
!3 = !{!"V", i32 2, !4}  ; <2 x %struct.test01*>
!4 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!5 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!6 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!7 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!5, !6, !7}

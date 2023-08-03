; REQUIRES: asserts
; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-outofboundsok=false -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; This test verifies that BadCasting is not set for other pointer
; fields of a parent class when a pointer field of the parent class
; violates BadCasting safety check.
; 3rd field in %struct.outer involves in BadCasting safety check
; with "store ptr %ph, ptr %field" instruction. This test makes sure
; the following:
;  1. Set BadCasting on parent (%struct.outer)
;  2. Set BadCasting on other non-pointer field (%struct.nested)
;  3. Set BadCasting on other structs that are involved (%struct.id)
;  4. BadCasting is NOT set on other pointer fields (%struct.inner)

; CHECK: LLVMType: %struct.id = type { i32, i32 }
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}
; CHECK: LLVMType: %struct.inner = type { i32, i32 }
; CHECK: Safety data: No issues found
; CHECK: LLVMType: %struct.nested = type { i32 }
; CHECK: Safety data: Bad casting | Nested structure{{ *$}}
; CHECK: LLVMType: %struct.outer = type { %struct.nested, i32, ptr, ptr }
; CHECK: Safety data: Bad casting | Mismatched element access | Contains nested structure{{ *$}}


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.nested = type { i32 }
%struct.inner = type { i32, i32 }
%struct.outer = type { %struct.nested, i32, ptr, ptr }
%struct.id = type { i32, i32 }
%struct.s = type { i32, ptr }

define void @bar(ptr %outer, ptr %p, ptr %sp) !intel.dtrans.func.type !11 {
bb0:
%f1 = getelementptr %struct.s, ptr %sp, i64 0, i32 1
%l1 = load ptr, ptr %f1
br label %bb2

bb1:
%f3 = getelementptr %struct.outer, ptr %outer, i64 0, i32 2
%l2 = load ptr, ptr %f3
br label %bb2

bb2:
%ph = phi ptr [%l1, %bb0], [%l2, %bb1]
%field = getelementptr %struct.outer, ptr %outer, i64 0, i32 2
store ptr %ph, ptr %field
ret void
}

!intel.dtrans.types = !{!0, !2, !5, !9}

!0 = !{!"S", %struct.inner zeroinitializer, i32 2, !1, !1} ; %struct.inner = type { i32, i32 }
!1 = !{i32 0, i32 0}
!2 = !{!"S", %struct.outer zeroinitializer, i32 4, !13, !1, !3, !4} ; %struct.outer = type { struct.nested, i32, i8*, %struct.inner* }
!3 = !{i8 0, i32 1}
!4 = !{%struct.inner zeroinitializer, i32 1}
!5 = !{!"S", %struct.id zeroinitializer, i32 2, !1, !1} ; %struct.id = type { i32, i32 }
!6 = distinct !{!7, !3}
!7 = !{%struct.outer zeroinitializer, i32 1}
!8 = !{%struct.id zeroinitializer, i32 1}
!9 = !{!"S", %struct.s zeroinitializer, i32 2, !1, !8} ; %struct.s = type { i32, %struct.id* }
!10 = !{%struct.s zeroinitializer, i32 1}
!11 = distinct !{!7, !3, !10}
!12 = !{!"S", %struct.nested zeroinitializer, i32 1, !1} ; %struct.nested = type { i32 }
!13 = !{%struct.nested zeroinitializer, i32 0}

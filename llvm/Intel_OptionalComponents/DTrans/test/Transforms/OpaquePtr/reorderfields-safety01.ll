; This test verifies dtrans field reordering transformation doesn't select
; a structure as candidate due to safety conditions.

; REQUIRES: asserts
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-reorderfieldsop -debug-only=dtrans-reorderfieldsop -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-reorderfieldsop -debug-only=dtrans-reorderfieldsop -disable-output 2>&1 | FileCheck %s

; CHECK: Rejecting struct.test based on safety data

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

define i32 @main() {
entry:
  %call = tail call noalias ptr @calloc(i64 10, i64 48)
  tail call void @foo(ptr %call)
  %h = getelementptr inbounds i8, ptr %call, i64 20
  tail call void @foo1(ptr nonnull %h)
  ret i32 0
}

declare !intel.dtrans.func.type !5 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64)
declare !intel.dtrans.func.type !7 void @foo(ptr "intel_dtrans_func_index"="1")
declare !intel.dtrans.func.type !9 void @foo1(ptr "intel_dtrans_func_index"="1")

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i8 0, i32 1}  ; i8*
!5 = distinct !{!4}
!6 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!7 = distinct !{!6}
!8 = !{i32 0, i32 1}  ; i32*
!9 = distinct !{!8}
!10 = !{!"S", %struct.test zeroinitializer, i32 7, !1, !2, !1, !1, !3, !2, !2} ; { i32, i64, i32, i32, i16, i64, i64 }

!intel.dtrans.types = !{!10}


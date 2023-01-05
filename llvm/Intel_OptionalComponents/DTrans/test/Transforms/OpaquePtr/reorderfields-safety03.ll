; This test verifies that struct.test is not selected as candidate
; due to reordering restrictions since struct.test has vector type.

;  RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-reorderfieldsop -S 2>&1 | FileCheck %s

; Note: Rejecting struct.test because reorder-field pass won't support any
; struct with vector type inside.

; CHECK: %struct.test = type { i32, i64, i32, i32, <2 x i8>, i64, i64 }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test = type { i32, i64, i32, i32, <2 x i8>, i64, i64 }

define void @foo(ptr "intel_dtrans_func_index"="1" %tp) !intel.dtrans.func.type !6 {
entry:
  %i = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 0
  %0 = load i32, ptr %i, align 8
  %add = add nsw i32 %0, 20
  %conv = sext i32 %add to i64
  %c = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 1
  store i64 %conv, ptr %c, align 8
  %add2 = add i32 %0, 40
  %t = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 2
  store i32 %add2, ptr %t, align 8
  ret void
}

define i32 @main() {
entry:
  %call = tail call noalias ptr @calloc(i64 10, i64 48)
  store i32 10, ptr %call, align 8
  tail call void @foo(ptr %call)
  ret i32 0
}

declare !intel.dtrans.func.type !8  "intel_dtrans_func_index"="1" ptr @calloc(i64, i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{!"V", i32 2, !4}  ; <2 x i8>
!4 = !{i8 0, i32 0}  ; i8
!5 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!6 = distinct !{!5}
!7 = !{i8 0, i32 1}  ; i8*
!8 = distinct !{!7}
!9 = !{!"S", %struct.test zeroinitializer, i32 7, !1, !2, !1, !1, !3, !2, !2} ; { i32, i64, i32, i32, <2 x i8>, i64, i64 }

!intel.dtrans.types = !{!9}


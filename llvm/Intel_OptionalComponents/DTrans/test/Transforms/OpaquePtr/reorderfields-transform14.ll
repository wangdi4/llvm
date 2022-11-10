; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-reorderfieldsop -S  < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; This test verifies that ReorderFieldsOP pass doesn't crash while processing
; "sub" instruction of %struct.test1 pointers when ReorderFields is
; not triggered for test1. ReorderFieldsOP is triggered for test2 but not
; for test1 due to safety issues.

; CHECK: define i32 @main
; CHECK: call void @foo(ptr %call) 

@G =  global i32 20, align 4
%struct.test1 = type { i32, i64, i32, i32, i16, i64, i64 }
%struct.test2 = type { i32, i64, i32, i32, i16, i64, i64 }

define i32 @main() {
entry:
  %call = tail call noalias ptr @calloc(i64 10, i64 48)
  %a4 = getelementptr %struct.test1, ptr %call, i64 4
  %ld = load i32, ptr @G, align 4
  %conv = sext i32 %ld to i64
  %mul = mul nsw i64 %conv, 48
  %p5 = ptrtoint ptr %a4 to i64
  %p6 = ptrtoint ptr %call to i64
  %diff2 = sub i64  %p6, %p5
  %num2 = sdiv i64 %diff2, %mul
  call void @foo(ptr %call)

  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %i = getelementptr %struct.test2, ptr %call1, i64 0, i32 0
  store i32 10, ptr %i, align 8
  ret i32 0
}

declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0
declare !intel.dtrans.func.type !10 void @foo(ptr "intel_dtrans_func_index"="1")

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!intel.dtrans.types = !{!9, !11}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{%struct.test1 zeroinitializer, i32 1}  ; %struct.test1*
!5 = distinct !{!4, !4}
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = distinct !{!6}
!9 = !{!"S", %struct.test1 zeroinitializer, i32 7, !1, !2, !1, !1, !3, !2, !2} ; { i32, i64, i32, i32, i16, i64, i64 }
!10 = distinct !{!4}
!11 = !{!"S", %struct.test2 zeroinitializer, i32 7, !1, !2, !1, !1, !3, !2, !2} ; { i32, i64, i32, i32, i16, i64, i64 }

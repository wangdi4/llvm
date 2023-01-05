; This test verifies that the field reordering transformation is applied
; correctly to malloc/calloc with constant and non-constant sizes related
; to %struct.test.

;  RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -S -passes=dtrans-reorderfieldsop | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@G =  global i32 20, align 4
%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

define i32 @main() {
entry:
  %ld = load i32, ptr @G, align 4
  %conv = sext i32 %ld to i64
  %mul = mul nsw i64 %conv, 48
  %call = tail call noalias ptr @malloc(i64 %mul)
; CHECK:   %0 = sdiv exact i64 %mul, 48
; CHECK:   %1 = mul i64 %0, 40
; CHECK:   %call = tail call noalias ptr @malloc(i64 %1)
; CHECK-NOT: malloc(i64 %mul)

  %j1 = getelementptr %struct.test, ptr %call, i64 0, i32 1
  store i64 0, ptr %j1

  %call1 = tail call noalias ptr @malloc(i64 48)
; CHECK:   %call1 = tail call noalias ptr @malloc(i64 40)

  %k1 = getelementptr %struct.test, ptr %call1, i64 0, i32 1
  store i64 0, ptr %k1
  %call2 = tail call noalias ptr @calloc(i64 10, i64 48)
; CHECK:   %call2 = tail call noalias ptr @calloc(i64 10, i64 40)

  %l1 = getelementptr %struct.test, ptr %call2, i64 0, i32 1
  store i64 0, ptr %l1
  %call3 = tail call noalias ptr @calloc(i64 48, i64 20)
; CHECK:   %call3 = tail call noalias ptr @calloc(i64 40, i64 20)

  %m1 = getelementptr %struct.test, ptr %call3, i64 0, i32 1
  store i64 0, ptr %m1
  %call4 = tail call noalias ptr @calloc(i64 %mul, i64 20)
; CHECK:   %2 = sdiv exact i64 %mul, 48
; CHECK:   %3 = mul i64 %2, 40
; CHECK:   %call4 = tail call noalias ptr @calloc(i64 %3, i64 20)

  %n1 = getelementptr %struct.test, ptr %call4, i64 0, i32 1
  store i64 0, ptr %n1
  %call5 = tail call noalias ptr @calloc(i64 10, i64 %mul)
; CHECK:   %4 = sdiv exact i64 %mul, 48
; CHECK:   %5 = mul i64 %4, 40
; CHECK:   %call5 = tail call noalias ptr @calloc(i64 10, i64 %5)

  %o1 = getelementptr %struct.test, ptr %call5, i64 0, i32 1
  store i64 0, ptr %o1

  %call6 = tail call noalias ptr @malloc(i64 480)
; CHECK: malloc(i64 400)

  %p1 = getelementptr %struct.test, ptr %call6, i64 0, i32 1
  store i64 0, ptr %p1

  ret i32 0
}

declare !intel.dtrans.func.type !5 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0
declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i8 0, i32 1}  ; i8*
!5 = distinct !{!4}
!6 = distinct !{!4}
!7 = !{!"S", %struct.test zeroinitializer, i32 7, !1, !2, !1, !1, !3, !2, !2} ; { i32, i64, i32, i32, i16, i64, i64 }

!intel.dtrans.types = !{!7}

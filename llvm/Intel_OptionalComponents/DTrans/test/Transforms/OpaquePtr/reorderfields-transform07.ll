; This test verifies that the field reordering transformation is applied
; correctly to realloc calls with constant and non-constant sizes related
; to %struct.test.

;  RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -S -dtrans-reorderfieldsop | FileCheck %s
;  RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -S -passes=dtrans-reorderfieldsop | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@G =  global i32 20, align 4
%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

define i32 @foo(ptr "intel_dtrans_func_index"="1" %p1, i64 %num2) !intel.dtrans.func.type !5 {
entry:
  %ld = load i32, ptr @G, align 4
  %conv = sext i32 %ld to i64
  %mul = mul nsw i64 %conv, 48

; realloc with constant size
  %call = tail call noalias ptr @realloc(ptr %p1, i64 48)
  %q2 = getelementptr %struct.test, ptr %call, i64 0, i32 2
  store i32 0, ptr %q2
; CHECK:  %call = tail call noalias ptr @realloc(ptr %p1, i64 40)

; realloc with non-constant size without sext/zext
  %call1 = tail call noalias ptr @realloc(ptr %call, i64 %mul)
  %w2 = getelementptr %struct.test, ptr %call1, i64 0, i32 2
  store i32 0, ptr %w2
; CHECK:  %[[SDIV:[0-9]+]] = sdiv exact i64 %mul, 48
; CHECK:  %[[MUL:[0-9]+]] = mul i64 %[[SDIV]], 40
; CHECK:  %call1 = tail call noalias ptr @realloc(ptr %call, i64 %[[MUL]])

; realloc with non-constant size with zext
  %mul1 = mul i32 %ld, 48
  %tmp1 = zext i32 %mul1 to i64
  %mul2 = mul i64 %tmp1, %num2
  %call2 = tail call noalias ptr @realloc(ptr %call, i64 %mul2)
  %e2 = getelementptr %struct.test, ptr %call2, i64 0, i32 2
  store i32 0, ptr %e2
; CHECK:  %[[SDIV:[0-9]+]] = sdiv exact i64 %mul2, 48
; CHECK:  %[[MUL:[0-9]+]] = mul i64 %[[SDIV]], 40
; CHECK:  %call2 = tail call noalias ptr @realloc(ptr %call, i64 %[[MUL]])

; realloc with non-constant size with sext
  %mul3 = mul i32 %ld, 48
  %tmp2 = sext i32 %mul3 to i64
  %mul4 = mul i64 %tmp2, %num2
  %call3 = tail call noalias ptr @realloc(ptr %call2, i64 %mul4)
  %r2 = getelementptr %struct.test, ptr %call3, i64 0, i32 2
  store i32 0, ptr %r2
; CHECK:  %[[SDIV:[0-9]+]] = sdiv exact i64 %mul4, 48
; CHECK:  %[[MUL:[0-9]+]] = mul i64 %[[SDIV]], 40
; CHECK:  %call3 = tail call noalias ptr @realloc(ptr %call2, i64 %[[MUL]])

  ret i32 0
}

; Function Attrs: nounwind
declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" ptr @realloc(ptr "intel_dtrans_func_index"="2", i64) #0

attributes #0 = { allockind("realloc") allocsize(1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!5 = distinct !{!4}
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6, !6}
!8 = !{!"S", %struct.test zeroinitializer, i32 7, !1, !2, !1, !1, !3, !2, !2} ; { i32, i64, i32, i32, i16, i64, i64 }

!intel.dtrans.types = !{!8}

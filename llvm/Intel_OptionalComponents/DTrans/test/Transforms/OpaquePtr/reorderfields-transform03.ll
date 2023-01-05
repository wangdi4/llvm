; This test verifies that the field reordering transformation is applied
; correctly to sdiv and udiv instructions with constant and non-constant
; sizes related to %struct.test.

;  RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -S -passes=dtrans-reorderfieldsop | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@G =  global i32 20, align 4
%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

define i32 @main() {
entry:
  %call = tail call noalias ptr @calloc(i64 10, i64 48)
  %a4 = getelementptr %struct.test, ptr %call, i64 4

  %ld = load i32, ptr @G, align 4
  %conv = sext i32 %ld to i64
  %mul = mul nsw i64 %conv, 48
  %p5 = ptrtoint ptr %a4 to i64
  %p6 = ptrtoint ptr %call to i64
  %diff2 = sub i64  %p6, %p5
  %num2 = sdiv i64 %diff2, %mul
; CHECK: %[[SDIV:[0-9]+]] = sdiv exact i64 %mul, 48
; CHECK: %[[MUL:[0-9]+]] = mul i64 %[[SDIV]], 40
; CHECK: %num2 = sdiv i64 %diff2, %[[MUL]]

  %diff3 = sub i64  %p6, %p5
  %num3 = udiv i64 %diff3, %mul
; CHECK: %[[SDIV:[0-9]+]] = sdiv exact i64 %mul, 48
; CHECK: %[[MUL:[0-9]+]] = mul i64 %[[SDIV]], 40
; CHECK: %num3 = udiv i64 %diff3, %[[MUL]]
  ret i32 0
}

define void @foo(ptr "intel_dtrans_func_index"="1" %tp, ptr "intel_dtrans_func_index"="2" %tp2) !intel.dtrans.func.type !5 {
entry:
  %p1 = ptrtoint ptr %tp2 to i64
  %p2 = ptrtoint ptr %tp to i64
  %diff = sub i64  %p2, %p1
  %num = sdiv i64 %diff, 48
; CHECK: %num = sdiv i64 %diff, 40

  %p3 = ptrtoint ptr %tp2 to i64
  %p4 = ptrtoint ptr %tp to i64
  %diff1 = sub i64  %p3, %p4
  %num1 = udiv i64 %diff1, 48
; CHECK: %num1 = udiv i64 %diff1, 40

  ret void
}

declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #1
declare !intel.dtrans.func.type !8 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!5 = distinct !{!4, !4}
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = distinct !{!6}
!9 = !{!"S", %struct.test zeroinitializer, i32 7, !1, !2, !1, !1, !3, !2, !2} ; { i32, i64, i32, i32, i16, i64, i64 }

!intel.dtrans.types = !{!9}

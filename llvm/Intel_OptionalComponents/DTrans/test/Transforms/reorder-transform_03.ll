; This test verifies that Field-reordering transformation applied
; correctly to sdiv and udiv instructions with constant and non-constant
; sizes related to %struct.test.

;  RUN: opt  -whole-program-assume < %s -S -dtrans-reorderfields | FileCheck %s
;  RUN: opt  -whole-program-assume < %s -S -passes=dtrans-reorderfields | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@G =  global i32 20, align 4
%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

define i32 @main() {
entry:
  %call = tail call noalias i8* @calloc(i64 10, i64 48)
  %0 = bitcast i8* %call to %struct.test*
  %1 = bitcast i8* %call to %struct.test*

  %ld = load i32, i32* @G, align 4
  %conv = sext i32 %ld to i64
  %mul = mul nsw i64 %conv, 48
  %p5 = ptrtoint %struct.test* %1 to i64
  %p6 = ptrtoint %struct.test* %0 to i64
  %diff2 = sub i64  %p6, %p5
  %num2 = sdiv i64 %diff2, %mul
; CHECK: %2 = sdiv exact i64 %mul, 48
; CHECK: %3 = mul i64 %2, 40
; CHECK: %num2 = sdiv i64 %diff2, %3

  %diff3 = sub i64  %p6, %p5
  %num3 = udiv i64 %diff3, %mul
; CHECK: %4 = sdiv exact i64 %mul, 48
; CHECK: %5 = mul i64 %4, 40
; CHECK: %num3 = udiv i64 %diff3, %5
  ret i32 0
}

define void @foo(%struct.test* %tp, %struct.test* %tp2) {
entry:
  %p1 = ptrtoint %struct.test* %tp2 to i64
  %p2 = ptrtoint %struct.test* %tp to i64
  %diff = sub i64  %p2, %p1
  %num = sdiv i64 %diff, 48
; CHECK: %num = sdiv i64 %diff, 40

  %p3 = ptrtoint %struct.test* %tp2 to i64
  %p4 = ptrtoint %struct.test* %tp to i64
  %diff1 = sub i64  %p3, %p4
  %num1 = udiv i64 %diff1, 48
; CHECK: %num1 = udiv i64 %diff1, 40

  ret void
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
declare dso_local noalias i8* @malloc(i64)

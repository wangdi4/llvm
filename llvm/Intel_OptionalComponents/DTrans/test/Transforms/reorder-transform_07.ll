; This test verifies that Field-reordering transformation applied
; correctly to realloc with constant and non-constant sizes with and
; without sext/zext related to %struct.test.

;  RUN: opt  < %s -whole-program-assume -S -dtrans-reorderfields | FileCheck %s
;  RUN: opt  < %s -whole-program-assume -S -passes=dtrans-reorderfields | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@G =  global i32 20, align 4
%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

define i32 @foo(%struct.test* %p1, i64 %num2) {
entry:
  %ld = load i32, i32* @G, align 4
  %conv = sext i32 %ld to i64
  %mul = mul nsw i64 %conv, 48
  %p2 = bitcast %struct.test* %p1 to i8*
; realloc with constant size
  %call = tail call noalias i8* @realloc(i8* %p2, i64 48)
; CHECK:  %call = tail call noalias i8* @realloc(i8* %p2, i64 40)

  %l = bitcast i8* %call to %struct.test*
; realloc with non-constant size without sext/zext
  %call1 = tail call noalias i8* @realloc(i8* %call, i64 %mul)
; CHECK:  %0 = sdiv exact i64 %mul, 48
; CHECK:  %1 = mul i64 %0, 40
; CHECK:  %call1 = tail call noalias i8* @realloc(i8* %call, i64 %1)

  %k = bitcast i8* %call1 to %struct.test*
; realloc with non-constant size with zext
  %mul1 = mul i32 %ld, 48
  %tmp1 = zext i32 %mul1 to i64
  %mul2 = mul i64 %tmp1, %num2
  %call2 = tail call noalias i8* @realloc(i8* %call, i64 %mul2)
; CHECK:  %2 = sdiv exact i64 %mul2, 48
; CHECK:  %3 = mul i64 %2, 40
; CHECK:  %call2 = tail call noalias i8* @realloc(i8* %call, i64 %3)

  %m = bitcast i8* %call2 to %struct.test*
; realloc with non-constant size with sext
  %mul3 = mul i32 %ld, 48
  %tmp2 = sext i32 %mul3 to i64
  %mul4 = mul i64 %tmp2, %num2
  %call3 = tail call noalias i8* @realloc(i8* %call2, i64 %mul4)
; CHECK:  %4 = sdiv exact i64 %mul4, 48
; CHECK:  %5 = mul i64 %4, 40
; CHECK:  %call3 = tail call noalias i8* @realloc(i8* %call2, i64 %5)

  %n = bitcast i8* %call3 to %struct.test*
  ret i32 0
}

; Function Attrs: nounwind
declare noalias i8* @realloc(i8*, i64)

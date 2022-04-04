; This test verifies that Field-reordering transformation applied
; correctly to malloc/calloc with constant and non-constant sizes related
; to %struct.test.

;  RUN: opt  -whole-program-assume < %s -S -dtrans-reorderfields | FileCheck %s
;  RUN: opt  -whole-program-assume < %s -S -passes=dtrans-reorderfields | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@G =  global i32 20, align 4
%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

define i32 @main() {
entry:
  %ld = load i32, i32* @G, align 4
  %conv = sext i32 %ld to i64
  %mul = mul nsw i64 %conv, 48
  %call = tail call noalias i8* @malloc(i64 %mul)
; CHECK:   %0 = sdiv exact i64 %mul, 48
; CHECK:   %1 = mul i64 %0, 40
; CHECK:   %call = tail call noalias i8* @malloc(i64 %1)
; CHECK-NOT: malloc(i64 %mul)

  %j = bitcast i8* %call to %struct.test*
  %call1 = tail call noalias i8* @malloc(i64 48)
; CHECK:   %call1 = tail call noalias i8* @malloc(i64 40)

  %k = bitcast i8* %call1 to %struct.test*
  %call2 = tail call noalias i8* @calloc(i64 10, i64 48)
; CHECK:   %call2 = tail call noalias i8* @calloc(i64 10, i64 40)

  %l = bitcast i8* %call2 to %struct.test*
  %call3 = tail call noalias i8* @calloc(i64 48, i64 20)
; CHECK:   %call3 = tail call noalias i8* @calloc(i64 40, i64 20)

  %m = bitcast i8* %call3 to %struct.test*
  %call4 = tail call noalias i8* @calloc(i64 %mul, i64 20)
; CHECK:   %2 = sdiv exact i64 %mul, 48
; CHECK:   %3 = mul i64 %2, 40
; CHECK:   %call4 = tail call noalias i8* @calloc(i64 %3, i64 20)

  %n = bitcast i8* %call4 to %struct.test*
  %call5 = tail call noalias i8* @calloc(i64 10, i64 %mul)
; CHECK:   %4 = sdiv exact i64 %mul, 48
; CHECK:   %5 = mul i64 %4, 40
; CHECK:   %call5 = tail call noalias i8* @calloc(i64 10, i64 %5)

  %o = bitcast i8* %call5 to %struct.test*

  %call6 = tail call noalias i8* @malloc(i64 480)
; CHECK: malloc(i64 400)

  %p = bitcast i8* %call6 to %struct.test*

  ret i32 0
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64)
declare noalias i8* @calloc(i64, i64)

; This test verifies that Field-reordering transformation applied
; correctly to malloc with non-constant size  related to %struct.test.

;  RUN: opt < %s -S -dtrans-reorderfields | FileCheck %s
;  RUN: opt < %s -S -passes=dtrans-reorderfields | FileCheck %s

; CHECK:   %0 = sdiv i64 %mul, 48
; CHECK:   %1 = mul i64 %0, 40
; CHECK:  %call = tail call noalias i8* @malloc(i64 %1)

; CHECK-NOT: malloc(i64 %mul)


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@G =  global i32 20, align 4
%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

define void @foo(%struct.test* %tp) {
entry:
  %i = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 0
  %0 = load i32, i32* %i, align 8
  %add = add nsw i32 %0, 20
  %conv = sext i32 %add to i64
  %c = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 1
  store i64 %conv, i64* %c, align 8
  %add2 = add i32 %0, 40
  %t = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 2
  store i32 %add2, i32* %t, align 8
  ret void
}

define i32 @main() {
entry:
  %ld = load i32, i32* @G, align 4
  %conv = sext i32 %ld to i64
  %mul = mul nsw i64 %conv, 48
  %call = tail call noalias i8* @malloc(i64 %mul)
  %j = bitcast i8* %call to %struct.test*
  %i = bitcast i8* %call to i32*
  store i32 10, i32* %i, align 8
  tail call void @foo(%struct.test* %j)
  ret i32 0
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64)

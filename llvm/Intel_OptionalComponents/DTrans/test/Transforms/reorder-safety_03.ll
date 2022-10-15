; UNSUPPORTED: enable-opaque-pointers

; This test verifies that struct.test is not selected as candidate
; due to reordering restrictions since struct.test has vector type.

;  RUN: opt  -whole-program-assume -intel-libirc-allowed < %s -dtrans-reorderfields   -S 2>&1 | FileCheck %s
;  RUN: opt  -whole-program-assume -intel-libirc-allowed < %s -passes=dtrans-reorderfields  -S 2>&1 | FileCheck %s

; Note: Rejecting struct.test because reorder-field pass won't support any struct with vector type inside.

; CHECK: %struct.test = type { i32, i64, i32, i32, <2 x i8>, i64, i64 }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test = type { i32, i64, i32, i32, <2 x i8>, i64, i64 }

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
  %call = tail call noalias i8* @calloc(i64 10, i64 48)
  %0 = bitcast i8* %call to %struct.test*
  %i = bitcast i8* %call to i32*
  store i32 10, i32* %i, align 8
  tail call void @foo(%struct.test* %0)
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)

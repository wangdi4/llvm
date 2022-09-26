; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; This test verifies dtrans field reordering transformation doesn't select
; struct.test as candidate due to safety conditions.

;  RUN: opt  -whole-program-assume < %s -dtrans-reorderfields -debug-only=dtrans-reorderfields -disable-output 2>&1 | FileCheck %s
;  RUN: opt  -whole-program-assume < %s -passes=dtrans-reorderfields -debug-only=dtrans-reorderfields -disable-output 2>&1 | FileCheck %s

; CHECK: Rejecting struct.test based on safety data


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test = type { i32, i64, i32, i32, i16, %struct.test1, i64 }
%struct.test1 = type { i64 }

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
  %add5 = add i32 %0, 70
  %h = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 3
  store i32 %add5, i32* %h, align 4
  %1 = trunc i32 %0 to i16
  %conv8 = add i16 %1, 110
  %d = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 4
  store i16 %conv8, i16* %d, align 8
  %conv10 = sext i16 %conv8 to i64
  %f = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 5, i32 0
  %2 = insertelement <2 x i64> undef, i64 %conv10, i32 0
  %3 = shufflevector <2 x i64> %2, <2 x i64> undef, <2 x i32> zeroinitializer
  %4 = add nsw <2 x i64> %3, <i64 50, i64 110>
  %5 = bitcast i64* %f to <2 x i64>*
  store <2 x i64> %4, <2 x i64>* %5, align 8
  ret void
}

define i32 @main() {
entry:
  ret i32 0
}

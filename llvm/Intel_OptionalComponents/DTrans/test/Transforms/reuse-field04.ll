; This test verifies foldToSameValue.
; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -enable-intel-advanced-opts -disable-output -mtriple=i686-- -mattr=+avx2 -whole-program-assume < %s -dtrans-reusefield -debug-only=dtrans-reusefield -disable-output 2>&1 | FileCheck %s
; RUN: opt -enable-intel-advanced-opts -disable-output -mtriple=i686-- -mattr=+avx2 -whole-program-assume < %s -passes=dtrans-reusefield -debug-only=dtrans-reusefield -disable-output 2>&1 | FileCheck %s

; CHECK:Fold following stores with same value:
; CHECK-NEXT:          store i64 %load0, i64* %f, align 8
; CHECK-NEXT:          store i64 %load1, i64* %o, align 8
; CHECK:Reused structure: %struct.test
; CHECK-NEXT:    Field mapping are (From:To): { 3:3 4:3 }
; CHECK-NEXT:GEP (before):
; CHECK-NEXT:  %g = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 4
; CHECK-NEXT:GEP (after):
; CHECK-NEXT:  %g = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 3

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test = type { i32, i64, i32, i64, i64 }

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
  %load0 = load i64, i64* %c, align 8
  %f = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 3
  store i64 %load0, i64* %f, align 8
  %load1 = load i64, i64* %c, align 8
  %o = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 4
  store i64 %load1, i64* %o, align 8
  ret void
}

define i64 @cal(%struct.test* %tp) {
entry:
  %f = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 3
  %a = load i64, i64* %f, align 8
  %g = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 4
  %b = load i64, i64* %g, align 8
  %c = add i64 %a, %b
  ret i64 %c
}

define i64 @main() {
entry:
  %call = tail call noalias i8* @calloc(i64 10, i64 40)
  %0 = bitcast i8* %call to %struct.test*
  %i = getelementptr %struct.test, %struct.test* %0, i64 0, i32 0
  store i32 10, i32* %i, align 8
  tail call void @foo(%struct.test* %0)
  %res = tail call i64 @cal(%struct.test* %0)
  ret i64 %res
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)

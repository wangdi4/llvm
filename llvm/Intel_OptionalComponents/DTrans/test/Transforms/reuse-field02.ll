; This test verifies that reuse field transformation applied to the 3-th and 4-th field of struct.test
; and replace it to 1.
; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume < %s -dtrans-reusefield -debug-only=dtrans-reusefield -disable-output 2>&1 | FileCheck %s
; RUN: opt -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume < %s -passes=dtrans-reusefield -debug-only=dtrans-reusefield -disable-output 2>&1 | FileCheck %s

; CHECK:Reused structure: %struct.test
; CHECK-NEXT:    Field mapping are (From:To): { 1:1 3:1 4:1 }
; CHECK-NEXT:GEP (before):
; CHECK-NEXT:  %f = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 3
; CHECK-NEXT:GEP (after):
; CHECK-NEXT:  %f = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 1
; CHECK-NEXT:GEP (before):
; CHECK-NEXT:  %g = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 4
; CHECK-NEXT:GEP (after):
; CHECK-NEXT:  %g = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 1

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
  %f = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 3
  store i64 %conv, i64* %f, align 8
  %o = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 4
  store i64 %conv, i64* %o, align 8
  ret void
}

define i64 @cal(%struct.test* %tp) {
entry:
  %f = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 3
  %a = load i64, i64* %f, align 8
  %g = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 4
  %b = load i64, i64* %g, align 8
  %h = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 1
  %d = load i64, i64* %h, align 8
  %e = add i64 %a, %b
  %ret = add i64 %d, %e
  ret i64 %ret
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

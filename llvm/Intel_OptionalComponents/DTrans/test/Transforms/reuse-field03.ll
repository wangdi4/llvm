; This test try to check fields intersect function, the 1-th field's same value fields should be { 1 3 4 }, but 3-th and 4-th field same value fields should be { 3, 4 }, so the result of intersect should be { 3, 4 }.
; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume < %s -dtrans-reusefield -debug-only=dtrans-reusefield -disable-output 2>&1 | FileCheck %s
; RUN: opt -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume < %s -passes=dtrans-reusefield -debug-only=dtrans-reusefield -disable-output 2>&1 | FileCheck %s

; CHECK:Reused structure: %struct.test
; CHECK-NEXT:    Field mapping are (From:To): { 3:3 4:3 }
; CHECK-NEXT:GEP (before):
; CHECK-NEXT:  %g = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 4
; CHECK-NEXT:New GEP (after):
; CHECK-NEXT:  %0 = getelementptr %struct.test, %struct.test* %tp, i64 0, i32 3
; CHECK-NEXT:Load:
; CHECK-NEXT:  %b = load i64, i64* %0, align 8
; CHECK-NEXT:GEP (before):
; CHECK-NEXT:  %g = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 4
; CHECK-NEXT:GEP (after):
; CHECK-NEXT:  %g = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 3
; CHECK-NEXT:GEP (before):
; CHECK-NEXT:  %g = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 4
; CHECK-NEXT:GEP (after):
; CHECK-NEXT:  %g = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 3

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test = type { i32, i64, i32, i64, i64 }

define void @foo_0(%struct.test* %tp) {
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

define i64 @cal_0(%struct.test* %tp) {
entry:
  %f = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 3
  %a = load i64, i64* %f, align 8
  %g = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 4
  %b = load i64, i64* %g, align 8
  %h = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 1
  %d = load i64, i64* %g, align 8
  %e = add i64 %a, %b
  %ret = add i64 %d, %e
  ret i64 %ret
}

define void @foo_1(%struct.test* %tp, i64 %num) {
entry:
  %add11 = add nsw i64 %num, 50
  %f = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 3
  store i64 %add11, i64* %f, align 8
  %add14 = add nsw i64 %num, 110
  %o = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 4
  store i64 %add11, i64* %o, align 8
  ret void
}

define i64 @cal_1(%struct.test* %tp) {
entry:
  %f = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 1
  %a = load i64, i64* %f, align 8
  %g = getelementptr inbounds %struct.test, %struct.test* %tp, i64 0, i32 4
  %b = load i64, i64* %g, align 8
  %c = sub i64 %a, %b
  ret i64 %c
}

define i64 @main() {
entry:
  %call = tail call noalias i8* @calloc(i64 10, i64 40)
  %0 = bitcast i8* %call to %struct.test*
  %i = getelementptr %struct.test, %struct.test* %0, i64 0, i32 0
  store i32 10, i32* %i, align 8
  tail call void @foo_0(%struct.test* %0)
  %res_0 = tail call i64 @cal_0(%struct.test* %0)
  tail call void @foo_1(%struct.test* %0, i64 101)
  %res_1 = tail call i64 @cal_1(%struct.test* %0)
  %res = add i64 %res_0, %res_1
  ret i64 %res
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)

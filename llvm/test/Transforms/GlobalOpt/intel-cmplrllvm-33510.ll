; RUN: opt < %s -S -globalopt | FileCheck %s
; RUN: opt < %s -S -passes="globalopt" | FileCheck %s

; This test case makes sure that the global optimizer doesn't
; assert when trying to fold the Load instruction with type
; x86_mmx.

; CHECK: define dso_local i32 @foo() local_unnamed_addr {
; CHECK-NEXT:  ret i32 0
; CHECK-NEXT: }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@m64 = internal global <1 x i64> zeroinitializer

define dso_local i32 @foo() {
  %temp = load x86_mmx, x86_mmx* bitcast (<1 x i64>* @m64 to x86_mmx*)
  ret i32 0
}

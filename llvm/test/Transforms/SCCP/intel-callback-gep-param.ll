; This test case checks that the interprocedural sparse conditional constant
; propagation works when the actual parameter is a GEP to a multidimensional
; array that is constant.

; RUN: opt -passes=ipsccp -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@globArray = internal global [4 x [2 x i32]] zeroinitializer

define internal void @foo(ptr %a, i8 %b) {
entry:
  %tmp1 = icmp eq i8 %b, 1
  br i1 %tmp1, label %recbr, label %retbr

recbr:
  call void @foo(ptr %a, i8 %b)
  br label %retbr

retbr:
  ret void
}

define void @bar() {
entry:
  store i32 1, ptr @globArray
  call void @foo(ptr @globArray, i8 1)
  ret void
}


; Check that the GEP was propagated correctly
; CHECK: define internal void @foo(ptr %a, i8 %b) {
; CHECK-NEXT: entry:
; CHECK-NEXT:  br label %recbr

; CHECK: recbr:                                            ; preds = %entry
; CHECK-NEXT:  call void @foo(ptr @globArray, i8 1)
; CHECK-NEXT:  br label %retbr

; CHECK: retbr:                                            ; preds = %recbr
; CHECK-NEXT:  ret void
; CHECK-NEXT: }

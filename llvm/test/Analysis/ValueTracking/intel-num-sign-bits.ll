; RUN: opt < %s -passes='print<num-sign-bits>' -disable-output 2>&1 | FileCheck %s
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @scalar.rec() {
; CHECK-LABEL: @scalar.rec
entry:
  br label %header

header:
  %iv = phi i64 [0, %entry], [%iv.next, %header]
; CHECK: %iv = phi i64 [ 0, %entry ], [ %iv.next, %header ]: 60
  %iv.next = add i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 16
  br i1 %exitcond, label %exit, label %header

exit:
  ret void
}

define void @scalar.rec.mul() {
; CHECK-LABEL: @scalar.rec.mul
entry:
  br label %header

header:
  %phi = phi i64 [0, %entry], [%phi.next, %header]
; CHECK: %phi = phi i64 [ 0, %entry ], [ %phi.next, %header ]: 1
  %phi.next = mul i64 %phi, 2
  %exitcond = icmp eq i64 %phi.next, 16
  br i1 %exitcond, label %exit, label %header

exit:
  ret void
}

define void @vector.rec() {
; CHECK-LABEL: @vector.rec
entry:
  br label %header

header:
  %iv = phi i64 [0, %entry], [%iv.next, %header]
  %vec.iv = phi <2 x i64> [<i64 0, i64 1>, %entry], [%vec.iv.next, %header]
; CHECK:  %vec.iv = phi <2 x i64> [ <i64 0, i64 1>, %entry ], [ %vec.iv.next, %header ]: 58
  %vec.iv.next = add <2 x i64> %vec.iv, <i64 2, i64 2>
  %iv.next = add i64 %iv, 2
  %exitcond = icmp eq i64 %iv.next, 16
  br i1 %exitcond, label %exit, label %header

exit:
  ret void
}

define void @vector.rec.mul() {
; CHECK-LABEL: @vector.rec.mul
entry:
  br label %header

header:
  %iv = phi i64 [0, %entry], [%iv.next, %header]
  %vec.iv = phi <2 x i64> [<i64 0, i64 1>, %entry], [%vec.iv.next, %header]
; CHECK:  %vec.iv = phi <2 x i64> [ <i64 0, i64 1>, %entry ], [ %vec.iv.next, %header ]: 1
  %vec.iv.next = mul <2 x i64> %vec.iv, <i64 2, i64 2>
  %iv.next = add i64 %iv, 2
  %exitcond = icmp eq i64 %iv.next, 16
  br i1 %exitcond, label %exit, label %header

exit:
  ret void
}

define void @bcast(i16 %x) {
; CHECK-LABEL: @bcast
  %ext = sext i16 %x to i64
  %insert = insertelement <2 x i64> undef, i64 %ext, i64 0
  %bcast = shufflevector <2 x i64> %insert, <2 x i64> undef, <2 x i32><i32 0, i32 0>
; CHECK:   %bcast = shufflevector <2 x i64> %insert, <2 x i64> undef, <2 x i32> zeroinitializer: 49
  ret void
}

declare i1 @cond(i64)
define void @vector.rec.uncountable() {
; CHECK-LABEL: @vector.rec.uncountable
entry:
  br label %header

header:
  %iv = phi i64 [0, %entry], [%iv.next, %header]
  %vec.iv = phi <2 x i64> [<i64 0, i64 1>, %entry], [%vec.iv.next, %header]
; CHECK:  %vec.iv = phi <2 x i64> [ <i64 0, i64 1>, %entry ], [ %vec.iv.next, %header ]: 1
  %vec.iv.next = add <2 x i64> %vec.iv, <i64 2, i64 2>
  %iv.next = add i64 %iv, 2
  %exitcond = call i1 @cond(i64 %iv)
  br i1 %exitcond, label %exit, label %header

exit:
  ret void
}

define void @scalar.rec.uncountable() {
; CHECK-LABEL: @scalar.rec.uncountable
entry:
  br label %header

header:
  %iv = phi i64 [0, %entry], [%iv.next, %header]
; CHECK: %iv = phi i64 [ 0, %entry ], [ %iv.next, %header ]: 1
  %iv.next = add i64 %iv, 1
  %exitcond = call i1 @cond(i64 %iv)
  br i1 %exitcond, label %exit, label %header

exit:
  ret void
}

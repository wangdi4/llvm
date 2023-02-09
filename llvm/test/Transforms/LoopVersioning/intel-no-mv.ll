; CMPLRLLVM-32566
; https://bugs.llvm.org/show_bug.cgi?id=52426
; LoopLoadElim asks LoopVersioning if it can optionally add a runtime check,
; but LV will assert if the runtime check is not needed.

; RUN: opt -opaque-pointers=0 -passes="loop-load-elim" -S %s | FileCheck %s

; CHECK-LABEL: bb
; CHECK: tmp6 = lshr
; CHECK-NEXT: load i32, i32* %tmp1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

$_ZZ = comdat any

declare dso_local i32 @spam(...)

define dso_local void @foo() local_unnamed_addr #0 comdat($_ZZ) align 2 personality i8* bitcast (i32 (...)* @spam to i8*) {
bb:
  %tmp = tail call noalias nonnull i8* @foo.1()
  %tmp1 = bitcast i8* %tmp to i32*
  %tmp2 = getelementptr i32, i32* %tmp1, i64 3
  %tmp3 = ptrtoint i32* %tmp2 to i64
  %tmp4 = ptrtoint i8* %tmp to i64
  %tmp5 = sub i64 %tmp3, %tmp4
  %tmp6 = lshr i64 %tmp5, 2
  br label %bb7

bb7:                                              ; preds = %bb7, %bb
  %tmp8 = phi i64 [ 1, %bb ], [ %tmp16, %bb7 ]
  %tmp9 = phi i32 [ 1, %bb ], [ %tmp15, %bb7 ]
  %tmp10 = add i32 %tmp9, -1
  %tmp11 = zext i32 %tmp10 to i64
  %tmp12 = getelementptr inbounds i32, i32* %tmp1, i64 %tmp11
  %tmp13 = load i32, i32* %tmp12, align 4
  %tmp14 = getelementptr inbounds i32, i32* %tmp1, i64 %tmp8
  store i32 undef, i32* %tmp14, align 4
  %tmp15 = add i32 %tmp9, 1
  %tmp16 = zext i32 %tmp15 to i64
  %tmp17 = icmp ugt i64 %tmp6, %tmp16
  br i1 %tmp17, label %bb7, label %bb18

bb18:                                             ; preds = %bb7
  unreachable
}

declare dso_local i8* @foo.1() local_unnamed_addr #0

attributes #0 = { "unsafe-fp-math"="true" }


; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -enable-new-pm=0 -hir-locality-analysis -hir-spatial-locality | FileCheck %s
; RUN: opt < %s -passes=hir-ssa-deconstruction | opt -passes="print<hir-locality-analysis>" -hir-spatial-locality -disable-output 2>&1 | FileCheck %s

; Verify that locality analysis does not assert on (%ptr)[0] which has non-linear stride.

; HIR-
; + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; |   %stride = (%stride_ptr)[0:0:8(i64*:0)];
; |   %ld = (%ptr)[0:0:%stride(i64*:0)];
; + END LOOP

; CHECK: Locality Info for Loop level: 1     NumCacheLines: 20       SpatialCacheLines: 19    TempInvCacheLines: 1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i64* %ptr, i64* %stride_ptr, i64 %n) {
alloca_0:
  %rel = icmp slt i64 %n, 1
  br i1 %rel, label %bb1, label %loop.preheader

loop.preheader:
  br label %loop

loop:                                             ; preds = %loop, %loop
  %indvars.iv = phi i64 [ 1, %loop.preheader ], [ %indvars.iv.next, %loop ]
  %stride = load i64, i64* %stride_ptr, align 1
  %subs = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 0, i64 0, i64 %stride, i64* elementtype(i64) %ptr, i64 0)
  %ld = load i64, i64* %subs
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  %rel52 = icmp sgt i64 %indvars.iv.next, %n
  br i1 %rel52, label %bb1.loopexit, label %loop

bb1.loopexit:                                     ; preds = %loop
  br label %bb1

bb1:                                              ; preds = %bb1.loopexit, %alloca_0
  ret void
}

declare i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8, i64, i64, i64*, i64) #1



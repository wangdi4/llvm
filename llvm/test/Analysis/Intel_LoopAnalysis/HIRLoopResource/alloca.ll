; RUN: opt -passes="hir-ssa-deconstruction,print<hir-loop-resource>" < %s -disable-output 2>&1 | FileCheck %s

; Verify that we can successfully compute loop resource for loop containing
; alloca instruction.
; Note that alloca is considered to have cost of 1.

; + DO i1 = 0, zext.i32.i64((-1 + %N)), 1   <DO_LOOP>
; |   %0 = alloca %size;
; |   (%allocs)[i1] = &((%0)[0]);
; + END LOOP

; CHECK: + DO i1 = 0, sext.i32.i64((-1 + %N)), 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:    Integer Operations: 3
; CHECK:    Integer Operations Cost: 3
; CHECK:    Integer Memory Writes: 1
; CHECK:    Memory Operations Cost: 4
; CHECK:    Total Cost: 7
; CHECK:    Memory Bound
; CHECK: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32 %N, i32 %size, ptr nocapture %allocs) {
entry:
  %cmp.5 = icmp sgt i32 %N, 0
  br i1 %cmp.5, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %conv = sext i32 %size to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %0 = alloca i8, i64 %conv, align 1
  %arrayidx = getelementptr inbounds ptr, ptr %allocs, i64 %indvars.iv
  store ptr %0, ptr %arrayidx, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %N
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}


; RUN: opt < %s -hir-ssa-deconstruction -analyze -hir-array-section-analysis | FileCheck %s
; R_UN: opt < %s -aa-pipeline=basic-aa -passes="hir-ssa-deconstruction,print<hir-array-section-analysis>" -disable-output 2>&1 | FileCheck %s

; Check that because of non-linear access in the innermost
; dimension (%p)[i1][%shl] the result is * for innermost dimension.

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   %shl = i1  <<  %n;
;       |   (%p)[i1][%shl] = i1;
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { }
; CHECK:    + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:    %p: (DEF) L: [ 0 ][ * ], U: [ 99 ][ * ]
; CHECK:    + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo([100 x i32]* nocapture %p, i32 %n) {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = trunc i64 %indvars.iv to i32
  %shl = shl i32 %0, %n
  %idxprom1 = sext i32 %shl to i64
  %arrayidx2 = getelementptr inbounds [100 x i32], [100 x i32]* %p, i64 %indvars.iv, i64 %idxprom1
  store i32 %0, i32* %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}


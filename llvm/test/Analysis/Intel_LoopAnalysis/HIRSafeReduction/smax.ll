; RUN: opt < %s  -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-safe-reduction-analysis>" -disable-output 2>&1 | FileCheck %s

; This test case checks that the llvm.smax intrinsic is marked as safe
; reduction. It was created from the following test case:

; int foo(int *a, int n) {
;   int max = 0;
;   for (int i = 0; i < n; i++) {
;     max = a[i] > max ? a[i] : max;
;   }
;   return max;
; }

; HIR generated

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   %max.013 = @llvm.smax.i32((%a)[i1],  %max.013);
;       + END LOOP
; END REGION

; Check safe reduction

; CHECK:  + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:  |   <Safe Reduction> Red Op: select <Has Unsafe Algebra- No> <Conditional- No>
; CHECK:  |   %max.013 = @llvm.smax.i32((%a)[i1],  %max.013); <Safe Reduction>
; CHECK:  + END LOOP

;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local noundef i32 @_Z3fooPii(i32* nocapture noundef readonly %a, i32 noundef %n) {
entry:
  %cmp12 = icmp sgt i32 %n, 0
  br i1 %cmp12, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  %.lcssa = phi i32 [ %1, %for.body ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %max.0.lcssa = phi i32 [ 0, %entry ], [ %.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %max.0.lcssa

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %max.013 = phi i32 [ 0, %for.body.preheader ], [ %1, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %1 = tail call i32 @llvm.smax.i32(i32 %0, i32 %max.013)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}

declare i32 @llvm.smax.i32(i32, i32)
; RUN: opt < %s  -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-safe-reduction-analysis>" -disable-output 2>&1 | FileCheck %s

; This test case checks that the llvm.umin intrinsic is marked as safe
; reduction. It was created from the following test case:

; unsigned foo(unsigned *a, unsigned n) {
;   unsigned min = 1000;
;   for (unsigned i = 0; i < n; i++) {
;     min = a[i] < min ? a[i] : min;
;   }
;   return min;
; }

; HIR generated

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>  <LEGAL_MAX_TC = 4294967295>
;       |   %min.013 = @llvm.umin.i32((%a)[i1],  %min.013);
;       + END LOOP
; END REGION

; Check for safe reduction

; CHECK: + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>  <LEGAL_MAX_TC = 4294967295>
; CHECK: |   <Safe Reduction> Red Op: select <Has Unsafe Algebra- No> <Conditional- No>
; CHECK: |   %min.013 = @llvm.umin.i32((%a)[i1],  %min.013); <Safe Reduction>
; CHECK: + END LOOP

;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local noundef i32 @_Z3fooPjj(i32* nocapture noundef readonly %a, i32 noundef %n) {
entry:
  %cmp12.not = icmp eq i32 %n, 0
  br i1 %cmp12.not, label %for.cond.cleanup, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  %.lcssa = phi i32 [ %1, %for.body ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %min.0.lcssa = phi i32 [ 1000, %entry ], [ %.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %min.0.lcssa

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %min.013 = phi i32 [ 1000, %for.body.preheader ], [ %1, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %1 = tail call i32 @llvm.umin.i32(i32 %0, i32 %min.013)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}

declare i32 @llvm.umin.i32(i32, i32)
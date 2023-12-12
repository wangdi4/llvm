; RUN: opt -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-reversal,print<hir>' -hir-cost-model-throttling=0 -hir-loop-reversal-assume-profitability -S 2>&1 < %s  | FileCheck %s

; This test checks that loop revesal was applied correctly when there are
; function calls safe to convert. An assertion in the HIR verifier related
; to fake ddrefs shouldn't happen.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   %call = @_Z3barPi(&((%a)[i1]));
;       |   %res.04 = %call  +  %res.04;
;       + END LOOP
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   %call = @_Z3barPi(&((%a)[-1 * i1 + 99]));
; CHECK:       |   %res.04 = %call  +  %res.04; <Safe Reduction>
; CHECK:       + END LOOP
; CHECK: END REGION

;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress uwtable
define dso_local noundef i32 @_Z3fooPPiS_(ptr nocapture noundef readonly %a, ptr nocapture noundef readnone %b) {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %add.lcssa = phi i32 [ %add, %for.body ]
  ret i32 %add.lcssa

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %res.04 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds ptr, ptr %a, i64 %indvars.iv
  %call = tail call noundef i32 @_Z3barPi(ptr noundef %arrayidx)
  %add = add nsw i32 %call, %res.04
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

declare dso_local noundef i32 @_Z3barPi(ptr noalias nocapture readonly %0) #0

attributes #0 = { nofree norecurse nosync nounwind memory(argmem: read) }
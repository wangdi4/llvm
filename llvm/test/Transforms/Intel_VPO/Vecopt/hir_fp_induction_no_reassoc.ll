; Test to verify that ParVecAnalysis does not auto-recognize a FP induction
; without reassoc flag.

; RUN: opt -passes='hir-ssa-deconstruction,hir-vec-dir-insert,print<hir>' -debug-only=dd-utils -disable-output < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; CHECK: is unsafe to vectorize/parallelize (FP induction with reassoc flag off)

; CHECK-LABEL: BEGIN REGION { }
; CHECK-NEXT:        + DO i1 = 0, 1023, 1   <DO_LOOP>
; CHECK-NEXT:        |   %r.014 = %r.014  +  1.000000e+00; <Safe Reduction>
; CHECK-NEXT:        + END LOOP
; CHECK-NEXT:  END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind readonly willreturn uwtable
define dso_local noundef float @_Z3subPfi() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %r.014 = phi float [ -1.000000e+00, %entry ], [ %conv1, %for.body ]
  %conv1 = fadd float %r.014, 1.000000e+00
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %r.014.lcssa = phi float [ %conv1, %for.body ]
  ret float %r.014.lcssa
}


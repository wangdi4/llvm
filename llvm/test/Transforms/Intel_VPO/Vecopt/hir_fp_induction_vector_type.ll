; Test to verify VPlan's support for FP vector inductions that are auto-
; recognized by LoopOpt framework.

; HIR before vectorizer -
;  BEGIN REGION { }
;        %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;
;        + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;        |   %r.014 = %r.014  +  <float 1.800000e+01, float 2.000000e+01, float 2.200000e+01, float 0.000000e+00>; <Safe Reduction>
;        + END LOOP
;
;        @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
;  END REGION

; RUN: opt -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -debug-only=LoopVectorizationPlanner -disable-output < %s 2>&1 | FileCheck %s

; RUN: opt -passes=hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter -disable-output -intel-opt-report=medium < %s 2>&1 | FileCheck %s --check-prefix=OPTRPTMED

; CHECK: A reduction or induction of a vector type is not supported.
; CHECK: DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>

; OPTRPTMED: remark #15573: loop was not vectorized: a reduction or induction of a vector type is not supported.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind readonly willreturn uwtable
define dso_local noundef <4 x float> @_Z3subPfi(<4 x float> %start, i32 noundef %N) local_unnamed_addr #0 {
entry:
  %cmp12 = icmp sgt i32 %N, 0
  br i1 %cmp12, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %N to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  %conv1.lcssa = phi <4 x float> [ %conv1, %for.body ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %ret.lcssa = phi <4 x float> [ %start, %entry ], [ %conv1.lcssa, %for.cond.cleanup.loopexit ]
  ret <4 x float> %ret.lcssa

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %r.014 = phi <4 x float> [ %start, %for.body.preheader ], [ %conv1, %for.body ]
  %conv1 = fadd reassoc <4 x float> %r.014, <float 1.800000e+01, float 2.000000e+01, float 2.200000e+01, float 0.000000e+00>
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}

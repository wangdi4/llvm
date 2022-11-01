; Check that VPlan HIR vectorizer codegen emits correct finalization code
; for FP inductions when remainder loop is also vectorized. Final value
; should be computed using the number of iterations that vectorized loops
; actually execute since original temp will be updated after each vector loop.

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-vec-scenario="n0;v8;m8" -print-after=hir-vplan-vec -hir-details -disable-output < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: Function: test1
; CHECK:  BEGIN REGION { modified }
; CHECK:        %ind.vec.step = 4.000000e+00  *  <float 0.000000e+00, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00>;
; CHECK:        %0 = %r.014  +  %ind.vec.step;
; CHECK:        %ind.step.init = 4.000000e+00  *  8.000000e+00;
; CHECK:        %phi.temp3 = %0;

; CHECK:        + DO i64 i1 = 0, 95, 8   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK:        + END LOOP

; Finalization after vector main loop.
; CHECK:        %cast.crd = sitofp.i64.float(96);
; CHECK:        %1 = 4.000000e+00  *  %cast.crd;
; CHECK:        %r.014 = %r.014  +  %1;
; CHECK:        %phi.temp = 96;

; CHECK:        %ind.vec.step16 = 4.000000e+00  *  <float 0.000000e+00, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00>;
; CHECK:        %3 = %phi.temp1  +  %ind.vec.step16;
; CHECK:        %ind.step.init17 = 4.000000e+00  *  8.000000e+00;
; CHECK:        %phi.temp18 = %3;

; Finalization after vector remainder loop.
; CHECK:        %cast.crd25 = sitofp.i64.float(100);
; CHECK:        %iv.lb.cast.crd = sitofp.i64.float(%phi.temp);
; CHECK:        %iv.lb.sub = %cast.crd25  -  %iv.lb.cast.crd;
; CHECK:        %5 = 4.000000e+00  *  %iv.lb.sub;
; CHECK:        %r.014 = %r.014  +  %5;
; CHECK:  END REGION

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind readonly willreturn uwtable
define dso_local void @test1(float* %A) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %r.014 = phi float [ -1.000000e+00, %entry ], [ %conv1, %for.body ]
  %conv1 = fadd reassoc float %r.014, 4.000000e+00
  store float %conv1, float* %A, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void
}

; CHECK: Function: test2
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i64 i1 = 0, 1023, 1   <DO_LOOP>
; CHECK:        |   %r.014 = -1.000000e+00;
; CHECK:        |   %phi.temp = 0;
; CHECK:        |   %ind.vec.step = 4.000000e+00  *  <float 0.000000e+00, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00>;
; CHECK:        |   %0 = %r.014  +  %ind.vec.step;
; CHECK:        |   %ind.step.init = 4.000000e+00  *  8.000000e+00;
; CHECK:        |   %phi.temp6 = %0;

; CHECK:        |   + DO i64 i2 = 0, %loop.ub, 8   <DO_LOOP> <auto-vectorized> <nounroll> <novectorize>
; CHECK:        |   + END LOOP

; CHECK:        |   %ind.final = 0  +  %vec.tc5;
; CHECK:        |   %cast.crd = sitofp.i64.float(%vec.tc5);
; CHECK:        |   %1 = 4.000000e+00  *  %cast.crd;
; CHECK:        |   %r.014 = %r.014  +  %1;
; CHECK:        |   %.vec12 = %n == %vec.tc5;
; CHECK:        |   %phi.temp = %ind.final;
; CHECK:        |   <LVAL-REG> NON-LINEAR i64 %phi.temp

; CHECK:        |   %ind.vec.step20 = 4.000000e+00  *  <float 0.000000e+00, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00>;
; CHECK:        |   %3 = %phi.temp2  +  %ind.vec.step20;
; CHECK:        |   %ind.step.init21 = 4.000000e+00  *  8.000000e+00;
; CHECK:        |   %phi.temp22 = %3;

; CHECK:        |   %cast.crd31 = sitofp.i64.float(%n);
; CHECK:        |   %iv.lb.cast.crd = sitofp.i64.float(%phi.temp);
; CHECK:        |   <RVAL-REG> NON-LINEAR i64 %phi.temp
; CHECK:        |   %iv.lb.sub = %cast.crd31  -  %iv.lb.cast.crd;
; CHECK:        |   %5 = 4.000000e+00  *  %iv.lb.sub;
; CHECK:        |   %r.014 = %r.014  +  %5;
; CHECK:        + END LOOP
; CHECK:  END REGION

define dso_local void @test2(float* %A, i64 %n) local_unnamed_addr #0 {
entry:
  br label %outer.for.body

outer.for.body:
  %outer.iv = phi i64 [ 0, %entry ], [ %outer.iv.next, %outer.latch ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %outer.for.body ], [ %indvars.iv.next, %for.body ]
  %r.014 = phi float [ -1.000000e+00, %outer.for.body ], [ %conv1, %for.body ]
  %conv1 = fadd reassoc float %r.014, 4.000000e+00
  store float %conv1, float* %A, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond.not, label %outer.latch, label %for.body

outer.latch:                                 ; preds = %for.body
  %outer.iv.next = add nuw nsw i64 %outer.iv, 1
  %outer.exitcond.not = icmp eq i64 %outer.iv.next, 1024
  br i1 %outer.exitcond.not, label %cleanup, label %outer.for.body

cleanup:
  ret void
}

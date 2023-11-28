; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-distribute-memrec,print<hir>" -aa-pipeline="basic-aa" -hir-loop-distribute-skip-vectorization-profitability-check=true -disable-output < %s 2>&1 | FileCheck %s

; Check that instruction order is preserved after the loop distribution.

; BEGIN REGION { }
;       + DO i1 = 0, 31, 1   <DO_LOOP>
;       |   %conv = fpext.float.double(%conv.phi);
;       |   %add = %conv  +  2.000000e+00;
;       |   %conv.phi = fptrunc.double.float(%add);
;       |   (@p)[0][i1] = %conv.phi;
;       |   (@q)[0][i1] = 5.000000e+00;
;       + END LOOP
; END REGION
;
; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1 = 0, 31, 1   <DO_LOOP>
; CHECK: |   %conv = fpext.float.double(%conv.phi);
; CHECK: |   %add = %conv  +  2.000000e+00;
; CHECK: |   %conv.phi = fptrunc.double.float(%add);
; CHECK: |   (@p)[0][i1] = %conv.phi;
; CHECK: + END LOOP

; CHECK: + DO i1 = 0, 31, 1   <DO_LOOP>
; CHECK: |   (@q)[0][i1] = 5.000000e+00;
; CHECK: + END LOOP
; CHECK:  END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@p = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@q = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %conv.phi = phi float [ 0.000000e+00, %entry ], [ %conv1, %for.body ]
  %0 = trunc i64 %indvars.iv to i32
  %conv = fpext float %conv.phi to double
  %add = fadd double %conv, 2.000000e+00
  %conv1 = fptrunc double %add to float
  %arrayidx = getelementptr inbounds [100 x float], ptr @p, i64 0, i64 %indvars.iv
  store float %conv1, ptr %arrayidx, align 4
  %arrayidx3 = getelementptr inbounds [100 x float], ptr @q, i64 0, i64 %indvars.iv
  store float 5.000000e+00, ptr %arrayidx3, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 32
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void
}


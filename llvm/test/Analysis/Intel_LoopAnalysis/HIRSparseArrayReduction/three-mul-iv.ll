; RUN: opt < %s -analyze -hir-ssa-deconstruction -force-hir-sparse-array-reduction-analysis -hir-sparse-array-reduction-analysis | FileCheck %s
; RUN: opt < %s -aa-pipeline=basic-aa -passes="hir-ssa-deconstruction,print<hir-sparse-array-reduction-analysis>" -force-hir-sparse-array-reduction-analysis -disable-output 2>&1 | FileCheck %s

; Check sparse array reduction for (@A)[0][3 * i1];

; BEGIN REGION
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   %1 = (@A)[0][3 * i1];
;       |   %2 = (@C)[0][i1];
;       |   %3 = (@B)[0][%1];
;       |   %add = %2  +  %3;
;       |   (@B)[0][%1] = %add;
;       + END LOOP
; END REGION

; CHECK:  BEGIN REGION { }
; CHECK:     + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:     |   %3 = (@B)[0][%1]; <Sparse Array Reduction>
; CHECK:     |   %add = %2  +  %3; <Sparse Array Reduction>
; CHECK:     |   (@B)[0][%1] = %add; <Sparse Array Reduction>
; CHECK:     + END LOOP
; CHECK:  END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = mul nuw nsw i64 %indvars.iv, 3
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %0
  %1 = load i32, i32* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [100 x float], [100 x float]* @C, i64 0, i64 %indvars.iv
  %2 = load float, float* %arrayidx2, align 4
  %idxprom3 = sext i32 %1 to i64
  %arrayidx4 = getelementptr inbounds [100 x float], [100 x float]* @B, i64 0, i64 %idxprom3
  %3 = load float, float* %arrayidx4, align 4
  %add = fadd float %2, %3
  store float %add, float* %arrayidx4, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}


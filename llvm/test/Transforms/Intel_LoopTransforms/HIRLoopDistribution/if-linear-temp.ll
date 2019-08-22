; RUN: opt -xmain-opt-level=3 -disable-output -S -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-distribute-memrec -print-after=hir-loop-distribute-memrec < %s 2>&1 | FileCheck %s
; RUN: opt -xmain-opt-level=3 -disable-output -S -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, 31, 1   <DO_LOOP>
;       |   if (%n == 8)
;       |   {
;       |      %add = (@B)[0][i1]  +  (@C)[0][i1];
;       |      (@A)[0][i1] = %add;
;       |      %conv10 = (@A)[0][i1 + 1]  +  1.000000e+00;
;       |      (@D)[0][i1] = %conv10;
;       |   }
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1 = 0, 31, 1
; CHECK: |   if (%n == 8)
;        |   {
;        |      %conv10 = (@A)[0][i1 + 1]  +  1.000000e+00;
;        |      (@D)[0][i1] = %conv10;
;        |   }
; CHECK: + END LOOP
;
; CHECK: + DO i1 = 0, 31, 1
; CHECK: |   if (%n == 8)
;        |   {
;        |      %add = (@B)[0][i1]  +  (@C)[0][i1];
;        |      (@A)[0][i1] = %add;
;        |   }
; CHECK: + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@D = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

; Function Attrs: nofree noinline norecurse nounwind uwtable
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp1 = icmp eq i32 %n, 8
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc
  ret void

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @B, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %arrayidx3 = getelementptr inbounds [100 x float], [100 x float]* @C, i64 0, i64 %indvars.iv
  %1 = load float, float* %arrayidx3, align 4
  %add = fadd float %0, %1
  %arrayidx5 = getelementptr inbounds [100 x float], [100 x float]* @A, i64 0, i64 %indvars.iv
  store float %add, float* %arrayidx5, align 4
  %2 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx8 = getelementptr inbounds [100 x float], [100 x float]* @A, i64 0, i64 %2
  %3 = load float, float* %arrayidx8, align 4
  %conv10 = fadd float %3, 1.000000e+00
  %arrayidx12 = getelementptr inbounds [100 x float], [100 x float]* @D, i64 0, i64 %indvars.iv
  store float %conv10, float* %arrayidx12, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 32
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}


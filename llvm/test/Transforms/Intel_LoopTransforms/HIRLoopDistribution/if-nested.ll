; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec,print<hir>" -disable-output -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Check that the loop with two-level nested IFs may be distributed.

;  BEGIN REGION { }
;       + DO i1 = 0, 63, 1   <DO_LOOP>
;       |   if (%n < %k)
;       |   {
;       |      if (%k > 10)
;       |      {
;       |         %add = (@B)[0][i1]  +  (@C)[0][i1];
;       |         (@A)[0][i1] = %add;
;       |         %conv = sitofp.i32.float(i1);
;       |         %add11 = (@A)[0][i1 + 1]  +  %conv;
;       |         (@D)[0][i1] = %add11;
;       |      }
;       |   }
;       + END LOOP
;  END REGION

; CHECK: BEGIN REGION { modified }
; CHECK:      + DO i1 = 0, 63, 1
; CHECK:      |   if (%n < %k)
; CHECK:      |   {
; CHECK:      |      if (%k > 10)
; CHECK:      |      {
; CHECK:      |         %conv = sitofp.i32.float(i1);
; CHECK:      |         %add11 = (@A)[0][i1 + 1]  +  %conv;
; CHECK:      |         (@D)[0][i1] = %add11;
; CHECK:      |      }
; CHECK:      |   }
; CHECK:      + END LOOP
;
; CHECK:      + DO i1 = 0, 63, 1
; CHECK:      |   if (%n < %k)
; CHECK:      |   {
; CHECK:      |      if (%k > 10)
; CHECK:      |      {
; CHECK:      |         %add = (@B)[0][i1]  +  (@C)[0][i1];
; CHECK:      |         (@A)[0][i1] = %add;
; CHECK:      |      }
; CHECK:      |   }
; CHECK:      + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local global [100 x float] zeroinitializer, align 16
@C = common dso_local global [100 x float] zeroinitializer, align 16
@A = common dso_local global [100 x float] zeroinitializer, align 16
@D = common dso_local global [100 x float] zeroinitializer, align 16

define void @foo(i32 %n, i32 %k) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp1 = icmp slt i32 %n, %k
  br i1 %cmp1, label %if.then, label %if.end14

if.then:                                          ; preds = %for.body
  %cmp2 = icmp sgt i32 %k, 10
  br i1 %cmp2, label %if.then3, label %if.end

if.then3:                                         ; preds = %if.then
  %idxprom = sext i32 %i.01 to i64
  %arrayidx = getelementptr inbounds [100 x float], ptr @B, i64 0, i64 %idxprom
  %0 = load float, ptr %arrayidx, align 4
  %idxprom4 = sext i32 %i.01 to i64
  %arrayidx5 = getelementptr inbounds [100 x float], ptr @C, i64 0, i64 %idxprom4
  %1 = load float, ptr %arrayidx5, align 4
  %add = fadd float %0, %1
  %idxprom6 = sext i32 %i.01 to i64
  %arrayidx7 = getelementptr inbounds [100 x float], ptr @A, i64 0, i64 %idxprom6
  store float %add, ptr %arrayidx7, align 4
  %add8 = add nsw i32 %i.01, 1
  %idxprom9 = sext i32 %add8 to i64
  %arrayidx10 = getelementptr inbounds [100 x float], ptr @A, i64 0, i64 %idxprom9
  %2 = load float, ptr %arrayidx10, align 4
  %conv = sitofp i32 %i.01 to float
  %add11 = fadd float %2, %conv
  %idxprom12 = sext i32 %i.01 to i64
  %arrayidx13 = getelementptr inbounds [100 x float], ptr @D, i64 0, i64 %idxprom12
  store float %add11, ptr %arrayidx13, align 4
  br label %if.end

if.end:                                           ; preds = %if.then3, %if.then
  br label %if.end14

if.end14:                                         ; preds = %if.end, %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end14
  %inc = add nsw i32 %i.01, 1
  %cmp = icmp slt i32 %inc, 64
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  ret void
}


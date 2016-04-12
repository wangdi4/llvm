; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the zero extended IV of outer non-generable loop (%idxprom15) is parsed correctly.
; CHECK: DO i1 = 0, zext.i32.i64((-1 + %N))
; CHECK-NEXT: %0 = {al:4}(%B)[%indvars.iv47][i1]
; CHECK-NEXT: %1 = {al:4}(%C)[%indvars.iv47][i1]
; CHECK-NEXT: {al:4}(%A)[%indvars.iv47][i1] = %0 + %1
; CHECK-NEXT: %2 = {al:4}(%A)[trunc.i64.i32((4294967295 + %indvars.iv47))][i1]
; CHECK-NEXT: %conv = sitofp.i32.double(%2)
; CHECK-NEXT: %mul = %conv  *  2.000000e+00
; CHECK-NEXT: %conv18 = fptosi.double.i32(%mul)
; CHECK-NEXT: {al:4}(%D)[%indvars.iv47][i1] = %conv18
; CHECK-NEXT: END LOOP


; ModuleID = 'fault1.cpp'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @_Z3fooPA100_iS0_S0_S0_i([100 x i32]* nocapture %A, [100 x i32]* nocapture readonly %B, [100 x i32]* nocapture readonly %C, [100 x i32]* nocapture %D, i32 %N) {
entry:
  %cmp.44 = icmp eq i32 %N, 0
  br i1 %cmp.44, label %for.end.25, label %for.body.3.lr.ph

for.body.3.lr.ph:                                 ; preds = %entry, %for.inc.23
  %indvars.iv47 = phi i64 [ %indvars.iv.next48, %for.inc.23 ], [ 0, %entry ]
  %sub = add i64 %indvars.iv47, 4294967295
  %idxprom15 = and i64 %sub, 4294967295
  br label %for.body.3

for.body.3:                                       ; preds = %for.body.3, %for.body.3.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.3.lr.ph ], [ %indvars.iv.next, %for.body.3 ]
  %arrayidx5 = getelementptr inbounds [100 x i32], [100 x i32]* %B, i64 %indvars.iv47, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx5, align 4
  %arrayidx9 = getelementptr inbounds [100 x i32], [100 x i32]* %C, i64 %indvars.iv47, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx9, align 4
  %add = add nsw i32 %1, %0
  %arrayidx13 = getelementptr inbounds [100 x i32], [100 x i32]* %A, i64 %indvars.iv47, i64 %indvars.iv
  store i32 %add, i32* %arrayidx13, align 4
  %arrayidx17 = getelementptr inbounds [100 x i32], [100 x i32]* %A, i64 %idxprom15, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx17, align 4
  %conv = sitofp i32 %2 to double
  %mul = fmul double %conv, 2.000000e+00
  %conv18 = fptosi double %mul to i32
  %arrayidx22 = getelementptr inbounds [100 x i32], [100 x i32]* %D, i64 %indvars.iv47, i64 %indvars.iv
  store i32 %conv18, i32* %arrayidx22, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %N
  br i1 %exitcond, label %for.inc.23, label %for.body.3

for.inc.23:                                       ; preds = %for.body.3
  %indvars.iv.next48 = add nuw nsw i64 %indvars.iv47, 1
  %lftr.wideiv49 = trunc i64 %indvars.iv.next48 to i32
  %exitcond50 = icmp eq i32 %lftr.wideiv49, %N
  br i1 %exitcond50, label %for.end.25, label %for.body.3.lr.ph

for.end.25:                                       ; preds = %for.inc.23, %entry
  ret void
}


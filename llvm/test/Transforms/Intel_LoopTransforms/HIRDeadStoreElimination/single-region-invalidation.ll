; RUN: opt -hir-create-function-level-region -hir-ssa-deconstruction -hir-dead-store-elimination -analyze -enable-new-pm=0 -hir-dd-analysis -hir-dd-analysis-verify=Region < %s 2>&1 | FileCheck %s
; RUN: opt -hir-create-function-level-region -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-dead-store-elimination,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region 2>&1 < %s -disable-output | FileCheck %s

; Check after the store (%A)[0][i1] being removed, the DDG will not contain %add edge.

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   %mul = i1  *  i1;
;       |   %conv = sitofp.i32.double(%mul);
;       |   %add = %conv  +  1.000000e+00;
;       |   (%A)[0][i1] = %add;
;       |   (%B)[i1] = i1;
;       + END LOOP
;
;       ret ;
; END REGION

; CHECK-NOT: %add --> %add

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* %B) #0 {
entry:
  %A = alloca [100 x double], align 16
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %mul = mul nsw i32 %i.01, %i.01
  %conv = sitofp i32 %mul to double
  %add = fadd double %conv, 1.000000e+00
  %idxprom = sext i32 %i.01 to i64
  %arrayidx = getelementptr inbounds [100 x double], [100 x double]* %A, i64 0, i64 %idxprom
  store double %add, double* %arrayidx, align 8
  %idxprom1 = sext i32 %i.01 to i64
  %ptridx = getelementptr inbounds i32, i32* %B, i64 %idxprom1
  store i32 %i.01, i32* %ptridx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.01, 1
  %cmp = icmp slt i32 %inc, 100
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  ret void
}



;RUN: opt -hir-ssa-deconstruction -hir-loop-distribute -print-after=hir-loop-distribute -hir-loop-distribute-heuristics=mem-rec  < %s 2>&1 | FileCheck %s
; split off cycle of {11,13,14,16}
;          BEGIN REGION { }
;<21>         + DO i1 = 0, 127, 1   <DO_LOOP>
;<3>          |   %0 = {al:4}(@B)[0][i1];
;<5>          |   %1 = {al:4}(@C)[0][i1];
;<6>          |   %add = %0  +  %1;
;<9>          |   {al:4}(@MC)[0][i1 + 1] = %add;
;<11>         |   %2 = {al:4}(@DC)[0][i1];
;<13>         |   %3 = {al:4}(@MC)[0][i1];
;<14>         |   %add10 = %2  +  %3;
;<16>         |   {al:4}(@DC)[0][i1 + 1] = %add10;
;<22>         + END LOOP
;          END REGION

; CHECK: BEGIN REGION
; CHECK-NEXT: DO i1 = 0, 127, 1
; CHECK-DAG: [[B_LD:%.*]] = {al:4}(@B)[0][i1]
; CHECK-DAG: [[C_LD:%.*]] = {al:4}(@C)[0][i1]
; CHECK-NEXT: [[ADD:%.*]] = [[B_LD]] + [[C_LD]]
; CHECK-NEXT: {al:4}(@MC)[0][i1 + 1] = [[ADD]]
; CHECK: END LOOP

; CHECK-NEXT: DO i1 = 0, 127, 1
; CHECK-DAG: [[DC_LD:%.*]] = {al:4}(@DC)[0][i1]
; CHECK-DAG: [[MC_LD:%.*]] = {al:4}(@MC)[0][i1]
; CHECK-NEXT: [[ADD2:%.*]] = [[DC_LD]] + [[MC_LD]]
; CHECK-NEXT: {al:4}(@DC)[0][i1 + 1] = [[ADD2]]
; CHECK: END LOOP
; CHECK-NEXT: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [128 x float] zeroinitializer, align 16
@C = common global [128 x float] zeroinitializer, align 16
@MC = common global [128 x float] zeroinitializer, align 16
@DC = common global [128 x float] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo() {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [128 x float], [128 x float]* @B, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [128 x float], [128 x float]* @C, i64 0, i64 %indvars.iv
  %1 = load float, float* %arrayidx2, align 4
  %add = fadd float %0, %1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx5 = getelementptr inbounds [128 x float], [128 x float]* @MC, i64 0, i64 %indvars.iv.next
  store float %add, float* %arrayidx5, align 4
  %arrayidx7 = getelementptr inbounds [128 x float], [128 x float]* @DC, i64 0, i64 %indvars.iv
  %2 = load float, float* %arrayidx7, align 4
  %arrayidx9 = getelementptr inbounds [128 x float], [128 x float]* @MC, i64 0, i64 %indvars.iv
  %3 = load float, float* %arrayidx9, align 4
  %add10 = fadd float %2, %3
  %arrayidx13 = getelementptr inbounds [128 x float], [128 x float]* @DC, i64 0, i64 %indvars.iv.next
  store float %add10, float* %arrayidx13, align 4
  %exitcond = icmp ne i64 %indvars.iv.next, 128
  br i1 %exitcond, label %for.body, label %for.cond.cleanup
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) 

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) 



;RUN: opt -hir-ssa-deconstruction -hir-loop-distribute -S -print-after=hir-loop-distribute -hir-loop-distribute-heuristics=mem-rec  < %s 2>&1 | FileCheck %s
; Some stmt reordering can be handled. There are two pi blocks
;here {12,16} and {...}. There is a < edge between the former
; and latter, so we should distribute, but ensure {12,16} 
;executes first
;          BEGIN REGION { }
;<28>         + DO i1 = 0, zext.i32.i64(%UB), 1   <DO_LOOP>
;<3>          |   %1 = {al:4}(@B)[0][i1];
;<5>          |   %2 = {al:4}(@C)[0][i1];
;<6>          |   %add = %1  +  %2;
;<9>          |   {al:4}(@MC)[0][i1 + %blob1] = %add;
;<12>         |   %4 = {al:4}(i32*)(@DC)[0][i1];
;<16>         |   {al:4}(i32*)(@B)[0][i1 + 1] = %4;
;<17>         |   %6 = {al:4}(@C)[0][i1];
;<20>         |   %7 = {al:4}(@MC)[0][i1 + %blob2];
;<21>         |   %add15 = %6  +  %7;
;<22>         |   {al:4}(@C)[0][i1] = %add15;
;<28>         + END LOOP
;          END REGION
;
;Explicitly check contents of first loop
; CHECK: DO i1 = 0, 
; CHECK-NEXT: [[LOAD:%.*]] = {al:4}(i32*)(@DC)[0][i1]; 
; CHECK-NEXT: {al:4}(i32*)(@B)[0][i1 + 1] = [[LOAD]]
; CHECK-NEXT: END LOOP

;Second loop contains rest, but most importantly the other
; @B[][] reference
; CHECK-NEXT: DO i1 = 0, 
; CHECK: {al:4}(@B)[0][i1]
; CHECK: END LOOP

;Only two loops
; CHECK-NOT: DO 
; ModuleID = 'reorder.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [128 x float] zeroinitializer, align 16
@C = common global [128 x float] zeroinitializer, align 16
@MC = common global [128 x float] zeroinitializer, align 16
@DC = common global [128 x float] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define float @foo(i32 %UB, i64 %blob1, i64 %blob2) {
entry:
  %cmp.32 = icmp eq i32 %UB, -1
  br i1 %cmp.32, label %for.end, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %0 = add i32 %UB, 1
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [128 x float], [128 x float]* @B, i64 0, i64 %indvars.iv
  %1 = load float, float* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [128 x float], [128 x float]* @C, i64 0, i64 %indvars.iv
  %2 = load float, float* %arrayidx2, align 4
  %add = fadd float %1, %2
  %add3 = add nsw i64 %indvars.iv, %blob1
  %arrayidx4 = getelementptr inbounds [128 x float], [128 x float]* @MC, i64 0, i64 %add3
  store float %add, float* %arrayidx4, align 4
  %arrayidx6 = getelementptr inbounds [128 x float], [128 x float]* @DC, i64 0, i64 %indvars.iv
  %3 = bitcast float* %arrayidx6 to i32*
  %4 = load i32, i32* %3, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx9 = getelementptr inbounds [128 x float], [128 x float]* @B, i64 0, i64 %indvars.iv.next
  %5 = bitcast float* %arrayidx9 to i32*
  store i32 %4, i32* %5, align 4
  %6 = load float, float* %arrayidx2, align 4
  %add13 = add nsw i64 %indvars.iv, %blob2
  %arrayidx14 = getelementptr inbounds [128 x float], [128 x float]* @MC, i64 0, i64 %add13
  %7 = load float, float* %arrayidx14, align 4
  %add15 = fadd float %6, %7
  store float %add15, float* %arrayidx2, align 4
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %0
  br i1 %exitcond, label %for.cond.for.end_crit_edge, label %for.body

for.cond.for.end_crit_edge:                       ; preds = %for.body
  %phitmp = sext i32 %UB to i64
  br label %for.end

for.end:                                          ; preds = %entry, %for.cond.for.end_crit_edge
  %i.0.lcssa = phi i64 [ %phitmp, %for.cond.for.end_crit_edge ], [ -1, %entry ]
  %arrayidx20 = getelementptr inbounds [128 x float], [128 x float]* @MC, i64 0, i64 %i.0.lcssa
  %8 = load float, float* %arrayidx20, align 4
  ret float %8
}


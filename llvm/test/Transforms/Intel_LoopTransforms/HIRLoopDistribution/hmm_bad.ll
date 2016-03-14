
;RUN: opt -hir-ssa-deconstruction -hir-loop-distribute -print-after=hir-loop-distribute -hir-loop-distribute-heuristics=mem-rec  < %s 2>&1 | FileCheck %s
; We cannot reason about calls without mod-ref/sideeffects analysis

;CHECK-NOT: BEGIN REGION {{.*}} modified
;CHECK: DO i1 = 
;CHECK-NOT: DO i1 =
; ModuleID = 'hmm_bad.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [128 x float] zeroinitializer, align 16
@C = common global [128 x float] zeroinitializer, align 16
@MC = common global [128 x float] zeroinitializer, align 16
@DC = common global [128 x float] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define float @foo(i32 %UB) {
entry:
  %cmp.26 = icmp eq i32 %UB, -1
  br i1 %cmp.26, label %for.end, label %for.body.lr.ph

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
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx5 = getelementptr inbounds [128 x float], [128 x float]* @MC, i64 0, i64 %indvars.iv.next
  store float %add, float* %arrayidx5, align 4
  %arrayidx7 = getelementptr inbounds [128 x float], [128 x float]* @DC, i64 0, i64 %indvars.iv
  %3 = load float, float* %arrayidx7, align 4
  %arrayidx9 = getelementptr inbounds [128 x float], [128 x float]* @MC, i64 0, i64 %indvars.iv
  %4 = load float, float* %arrayidx9, align 4
  %add10 = fadd float %3, %4
  %arrayidx13 = getelementptr inbounds [128 x float], [128 x float]* @DC, i64 0, i64 %indvars.iv.next
  store float %add10, float* %arrayidx13, align 4
  tail call void (...) @bar() #2
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %0
  br i1 %exitcond, label %for.cond.for.end_crit_edge, label %for.body

for.cond.for.end_crit_edge:                       ; preds = %for.body
  %phitmp = sext i32 %UB to i64
  br label %for.end

for.end:                                          ; preds = %entry, %for.cond.for.end_crit_edge
  %i.0.lcssa = phi i64 [ %phitmp, %for.cond.for.end_crit_edge ], [ -1, %entry ]
  %arrayidx16 = getelementptr inbounds [128 x float], [128 x float]* @MC, i64 0, i64 %i.0.lcssa
  %5 = load float, float* %arrayidx16, align 4
  ret float %5
}

declare void @bar(...) 


; Check that non linear substs can not be handled by runtime dd.

; REQUIRES: asserts
; RUN: opt -hir-ssa-deconstruction -hir-runtime-dd -print-after=hir-runtime-dd -debug-only=hir-runtime-dd -S < %s 2>&1 | FileCheck %s

; void foo(int *p, int *q, int N) {
;   int i,j;
;   for (i=0;i<N;i++) {
;     p[i] = q[p[i]];
;   }
; }

; CHECK: Runtime DD for loop [[LOOP:[0-9]+]]:
; CHECK: LOOPOPT_OPTREPORT: [RTDD] Loop [[LOOP]]:{{.*}}non linear

; ModuleID = 'nonlinear_ub.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* %p, i32* %q, i32 %N) #0 {
entry:
  %cmp.1 = icmp slt i32 0, %N
  br i1 %cmp.1, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %i.02 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.inc ]
  %idxprom = sext i32 %i.02 to i64
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  %idxprom1 = sext i32 %0 to i64
  %arrayidx2 = getelementptr inbounds i32, i32* %q, i64 %idxprom1
  %1 = load i32, i32* %arrayidx2, align 4
  %idxprom3 = sext i32 %i.02 to i64
  %arrayidx4 = getelementptr inbounds i32, i32* %p, i64 %idxprom3
  store i32 %1, i32* %arrayidx4, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.02, 1
  %cmp = icmp slt i32 %inc, %N
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  ret void
}


; Check runtime dd multiversioning for a simple case with p[i] and q[i]

; RUN: opt -hir-ssa-deconstruction -hir-runtime-dd -hir-details -print-after=hir-runtime-dd -S < %s 2>&1 | FileCheck %s

; int foo(int *p, int *q, int N) {
;   int i;
;   for (i=0;i<N;i++) {
;     p[i] = q[i];
;   }
;   return p[0];
; }

; CHECK: IR Dump After
; CHECK: if (%N < {{[0-9]+}})
; CHECK: if (&((%q)[%N + -1]) >= &((%p)[0]) && &((%p)[%N + -1]) >= &((%q)[0]))

; CHECK: <RVAL-REG> {{.*}} %q)[{{.*}}]{{.*}} !alias.scope [[SCOPE1:.*]] !noalias [[SCOPE2:.*]] {
; CHECK: <LVAL-REG> {{.*}} %p)[{{.*}}]{{.*}} !alias.scope [[SCOPE2]] !noalias [[SCOPE1]] {

; ModuleID = 'ptrs.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @foo(i32* %p, i32* %q, i32 %N) #0 {
entry:
  %cmp.1 = icmp slt i32 0, %N
  br i1 %cmp.1, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %i.02 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.inc ]
  %idxprom = sext i32 %i.02 to i64
  %arrayidx = getelementptr inbounds i32, i32* %q, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  %idxprom1 = sext i32 %i.02 to i64
  %arrayidx2 = getelementptr inbounds i32, i32* %p, i64 %idxprom1
  store i32 %0, i32* %arrayidx2, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.02, 1
  %cmp = icmp slt i32 %inc, %N
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  %arrayidx3 = getelementptr inbounds i32, i32* %p, i64 0
  %1 = load i32, i32* %arrayidx3, align 4
  ret i32 %1
}


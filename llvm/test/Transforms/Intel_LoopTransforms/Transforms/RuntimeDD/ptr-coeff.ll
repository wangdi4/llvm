; Check runtime dd multiversioning for a case p[const*i + const] and q[const*i + const]

; RUN: opt -hir-ssa-deconstruction -hir-runtime-dd -print-after=hir-runtime-dd -S < %s 2>&1 | FileCheck %s

; int foo(int *p, int *q, int N) {
;   int i;
;   for (i=0;i<N;i++) {
;     p[5*i + 10] = q[-i-5];
;   }
;   return p[0];
; }

; CHECK: IR Dump After
; CHECK: if (%N < {{[0-9]+}})
; CHECK: if (&((%q)[-5]) >= &((%p)[10]) && &((%p)[5 * %N + 5]) >= &((%q)[-1 * %N + -4]))

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
  %sub = sub nsw i32 0, %i.02
  %sub1 = sub nsw i32 %sub, 5
  %idxprom = sext i32 %sub1 to i64
  %arrayidx = getelementptr inbounds i32, i32* %q, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  %mul = mul nsw i32 5, %i.02
  %add = add nsw i32 %mul, 10
  %idxprom2 = sext i32 %add to i64
  %arrayidx3 = getelementptr inbounds i32, i32* %p, i64 %idxprom2
  store i32 %0, i32* %arrayidx3, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.02, 1
  %cmp = icmp slt i32 %inc, %N
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  %arrayidx4 = getelementptr inbounds i32, i32* %p, i64 0
  %1 = load i32, i32* %arrayidx4, align 4
  ret i32 %1
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1607) (llvm/branches/loopopt 1658)"}

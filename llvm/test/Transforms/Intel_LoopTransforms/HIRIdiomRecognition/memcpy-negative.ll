; HIR Idiom Rec: memcpy with negative IV.

; RUN: opt -opaque-pointers=0 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-idiom,print<hir>,hir-cg" -aa-pipeline="basic-aa" -disable-output 2>&1 < %s | FileCheck %s

; HIR:
;       BEGIN REGION { }
;             + DO i1 = 0, %n + -1, 1   <DO_LOOP>
;             |   (%p)[-1 * i1] = (%q)[-1 * i1 + 1];
;             + END LOOP
;       END REGION

; CHECK: BEGIN REGION { modified }
; CHECK: llvm.memcpy.p0i8.p0i8.i32(&((i8*)(%p)[-1 * %n + 1]),  &((i8*)(%q)[-1 * %n + 2]),  4 * %n,  0);

; ModuleID = 'memcpy-negative.bc'
source_filename = "memcpy-negative.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @foo(float* noalias %p, float* %q, i32 %n) #0 {
entry:
  %cmp1 = icmp slt i32 0, %n
  br i1 %cmp1, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %i.02 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.inc ]
  %sub = sub nsw i32 0, %i.02
  %add = add nsw i32 %sub, 1
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds float, float* %q, i64 %idxprom
  %0 = load float, float* %arrayidx, align 4
  %sub1 = sub nsw i32 0, %i.02
  %idxprom2 = sext i32 %sub1 to i64
  %arrayidx3 = getelementptr inbounds float, float* %p, i64 %idxprom2
  store float %0, float* %arrayidx3, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.02, 1
  %cmp = icmp slt i32 %inc, %n
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  ret void
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20662) (llvm/branches/loopopt 20688)"}

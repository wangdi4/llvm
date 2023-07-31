; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-create-function-level-region < %s 2>&1 | FileCheck %s

; Verify that loops with fusion preventive preheader/postexit dependency are not fused (without assertions).
; Note: hir-lmm is used to populate preheader/postexit.

; BEGIN REGION { modified }
;
;   %limm = (%q)[-1];
;   + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;   |   %0 = %limm;
;   |   (%p)[i1] = %0;
;   + END LOOP
;   %x.0.lcssa = %0;  << %x.0.lcssa is used in a second loop body.
;
;
;   + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;   |   %1 = (%p)[i1];
;   |   (%q)[i1] = %1 + 1;
;   |   %limm3 = %x.0.lcssa;  << here
;   + END LOOP
;   (%q)[0] = %limm3;
;
;   ret ;
; END REGION

; CHECK: Function
; CHECK: DO i1
; CHECK: DO i1

; ModuleID = 'one-region-prepost.ll'
source_filename = "1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @foo(ptr noalias %p, ptr %q, i32 %n) #0 {
entry:
  %cmp3 = icmp slt i32 0, %n
  br i1 %cmp3, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %i.04 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.inc ]
  %arrayidx = getelementptr inbounds i32, ptr %q, i64 -1
  %0 = load i32, ptr %arrayidx, align 4
  %idxprom = sext i32 %i.04 to i64
  %arrayidx1 = getelementptr inbounds i32, ptr %p, i64 %idxprom
  store i32 %0, ptr %arrayidx1, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.04, 1
  %cmp = icmp slt i32 %inc, %n
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.inc
  %split = phi i32 [ %0, %for.inc ]
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  %x.0.lcssa = phi i32 [ %split, %for.cond.for.end_crit_edge ], [ undef, %entry ]
  %cmp41 = icmp slt i32 0, %n
  br i1 %cmp41, label %for.body5.lr.ph, label %for.end13

for.body5.lr.ph:                                  ; preds = %for.end
  br label %for.body5

for.body5:                                        ; preds = %for.body5.lr.ph, %for.inc11
  %i2.02 = phi i32 [ 0, %for.body5.lr.ph ], [ %inc12, %for.inc11 ]
  %idxprom6 = sext i32 %i2.02 to i64
  %arrayidx7 = getelementptr inbounds i32, ptr %p, i64 %idxprom6
  %1 = load i32, ptr %arrayidx7, align 4
  %add = add nsw i32 %1, 1
  %idxprom8 = sext i32 %i2.02 to i64
  %arrayidx9 = getelementptr inbounds i32, ptr %q, i64 %idxprom8
  store i32 %add, ptr %arrayidx9, align 4
  %arrayidx10 = getelementptr inbounds i32, ptr %q, i64 0
  store i32 %x.0.lcssa, ptr %arrayidx10, align 4
  br label %for.inc11

for.inc11:                                        ; preds = %for.body5
  %inc12 = add nsw i32 %i2.02, 1
  %cmp4 = icmp slt i32 %inc12, %n
  br i1 %cmp4, label %for.body5, label %for.cond3.for.end13_crit_edge

for.cond3.for.end13_crit_edge:                    ; preds = %for.inc11
  br label %for.end13

for.end13:                                        ; preds = %for.cond3.for.end13_crit_edge, %for.end
  ret void
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}

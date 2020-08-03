; REQUIRES: asserts

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -tbaa -hir-pre-vec-complete-unroll -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll" -aa-pipeline="tbaa" -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that we recognize (%A)[i2] as invariant (hoistable) in the unrolled loop because one of the loads is unconditional.

; + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; |   %t.12 = %t.05;
; |
; |   + DO i2 = 0, 9, 1   <DO_LOOP>
; |   |   %t.2 = %t.12;
; |   |   if (i1 > 5)
; |   |   {
; |   |      %t.2 = (%A)[i2];
; |   |   }
; |   |   %1 = (%A)[i2];
; |   |   %add = %t.2  +  %1;
; |   |   %t.12 = %1 + %t.2;
; |   + END LOOP
; |
; |   %t.05 = %add;
; + END LOOP


; CHECK: GEPSavings: 20

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(i32* %A, i32 %n) {
entry:
  %cmp3 = icmp slt i32 0, %n
  br i1 %cmp3, label %for.body.lr.ph, label %for.end9

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc7
  %t.05 = phi i32 [ 0, %for.body.lr.ph ], [ %t.1.lcssa, %for.inc7 ]
  %i.04 = phi i32 [ 0, %for.body.lr.ph ], [ %inc8, %for.inc7 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body, %if.end
  %t.12 = phi i32 [ %t.05, %for.body ], [ %add, %if.end ]
  %j.01 = phi i32 [ 0, %for.body ], [ %inc, %if.end ]
  %cmp4 = icmp sgt i32 %i.04, 5
  br i1 %cmp4, label %if.then, label %if.end

if.then:                                          ; preds = %for.body3
  %idxprom = sext i32 %j.01 to i64
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body3
  %t.2 = phi i32 [ %0, %if.then ], [ %t.12, %for.body3 ]
  %idxprom5 = sext i32 %j.01 to i64
  %arrayidx6 = getelementptr inbounds i32, i32* %A, i64 %idxprom5
  %1 = load i32, i32* %arrayidx6, align 4
  %add = add nsw i32 %t.2, %1
  %inc = add nsw i32 %j.01, 1
  %cmp2 = icmp slt i32 %inc, 10
  br i1 %cmp2, label %for.body3, label %for.inc7

for.inc7:                                         ; preds = %if.end
  %t.1.lcssa = phi i32 [ %add, %if.end ]
  %inc8 = add nsw i32 %i.04, 1
  %cmp = icmp slt i32 %inc8, %n
  br i1 %cmp, label %for.body, label %for.cond.for.end9_crit_edge

for.cond.for.end9_crit_edge:                      ; preds = %for.inc7
  %split = phi i32 [ %t.1.lcssa, %for.inc7 ]
  br label %for.end9

for.end9:                                         ; preds = %for.cond.for.end9_crit_edge, %entry
  %t.0.lcssa = phi i32 [ %split, %for.cond.for.end9_crit_edge ], [ 0, %entry ]
  ret i32 %t.0.lcssa
}

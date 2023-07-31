; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -hir-cost-model-throttling=0 -disable-output < %s 2>&1 | FileCheck %s

; OUTDATED: Check that node (*) in the DO i2 loop postexit will not be present after opt-predicate and dead node removal.
; This test case now checks that we pass with opt predicate. The instruction in postexit was eliminated with improvement in framework.
; TODO: find a new postexit case.


; + DO i1 = 0, 99, 1   <DO_MULTI_EXIT_LOOP>
; |   + DO i2 = 0, %s + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483647>
; |   |   %t.05 = %t.05  +  i2;
; |   |   if (%n < 5)
; |   |   {
; |   |      goto if.then;
; |   |   }
; |   + END LOOP
; |
; |   (%p)[i1] = %t.05;
; + END LOOP

; CHECK: if (%n < 5)
; CHECK: {
; CHECK:    + DO i1 = 0, 99, 1   <DO_MULTI_EXIT_LOOP>
; CHECK:    |   if (0 < %s)
; CHECK:    |   {
; CHECK:    |      %t.05 = %t.05  +  0;
; CHECK:    |      goto if.then;
; CHECK:    |   }
; CHECK:    |   (%p)[i1] = %t.05;
; CHECK:    + END LOOP
; CHECK: else
; CHECK: {
; CHECK:    + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:    |   + DO i2 = 0, %s + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:    |   |   %t.05 = %t.05  +  i2;
; CHECK:    |   + END LOOP
; CHECK:    |
; CHECK:    |   (%p)[i1] = %t.05;
; CHECK:    + END LOOP
; CHECK: }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr %p, i32 %n, i32 %s) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc5
  %i.06 = phi i32 [ 0, %entry ], [ %inc6, %for.inc5 ]
  %t.05 = phi i32 [ 0, %entry ], [ %t.1.lcssa, %for.inc5 ]
  %cmp22 = icmp slt i32 0, %s
  br i1 %cmp22, label %for.body3.lr.ph, label %for.end

for.body3.lr.ph:                                  ; preds = %for.body
  br label %for.body3

for.body3:                                        ; preds = %for.body3.lr.ph, %for.inc
  %j.04 = phi i32 [ 0, %for.body3.lr.ph ], [ %inc, %for.inc ]
  %t.13 = phi i32 [ %t.05, %for.body3.lr.ph ], [ %add, %for.inc ]
  %add = add nsw i32 %t.13, %j.04
  %cmp4 = icmp slt i32 %n, 5
  br i1 %cmp4, label %if.then, label %if.end

if.then:                                          ; preds = %for.body3
  br label %L1

if.end:                                           ; preds = %for.body3
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %inc = add nsw i32 %j.04, 1
  %cmp2 = icmp slt i32 %inc, %s
  br i1 %cmp2, label %for.body3, label %for.cond1.for.end_crit_edge

for.cond1.for.end_crit_edge:                      ; preds = %for.inc
  %split = phi i32 [ %add, %for.inc ]
  br label %for.end

for.end:                                          ; preds = %for.cond1.for.end_crit_edge, %for.body
  %t.1.lcssa = phi i32 [ %split, %for.cond1.for.end_crit_edge ], [ %t.05, %for.body ]
  %idxprom = sext i32 %i.06 to i64
  %arrayidx = getelementptr inbounds i32, ptr %p, i64 %idxprom
  store i32 %t.1.lcssa, ptr %arrayidx, align 4
  br label %for.inc5

for.inc5:                                         ; preds = %for.end
  %inc6 = add nsw i32 %i.06, 1
  %cmp = icmp slt i32 %inc6, 100
  br i1 %cmp, label %for.body, label %for.end7

for.end7:                                         ; preds = %for.inc5
  br label %L1

L1:                                               ; preds = %for.end7, %if.then
  %arrayidx8 = getelementptr inbounds i32, ptr %p, i64 0
  store i32 0, ptr %arrayidx8, align 4
  ret void
}


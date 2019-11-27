; RUN: opt -hir-ssa-deconstruction -hir-opt-predicate -print-after=hir-opt-predicate -S -hir-details -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -S -hir-details -disable-output < %s 2>&1 | FileCheck %s

; Check that both preheader and postexit are extracted before hoisting "if (i1 > 100)".
; Also check def levels of RVAL %y.12.out in preheader.

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   %y.04.out1 = %y.04;
;       |
;       |      %y.12 = %y.04.out1;
;       |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;       |   |   (%p)[i1] = i2;
;       |   |   if (i1 > 100)
;       |   |   {
;       |   |      %y.12 = %y.12  +  0.000000e+00;
;       |   |   }
;       |   |   %y.12.out = %y.12;
;       |   + END LOOP
;       |      %y.04 = %y.12.out;
;       |
;       |   %y.04.out = %y.04;
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { modified }
; CHECK:       + DO i32 i1 = 0, 99, 1
; CHECK:       |   %y.04.out1 = %y.04;
; CHECK:       |   if (0 < %n)
; CHECK:       |   {
; CHECK:       |      %y.12 = %y.04.out1;
; CHECK:       |      if (i1 > 100)
; CHECK:       |      {
; CHECK:       |         + DO i32 i2 = 0, %n + -1, 1
; CHECK:       |         |   (%p)[i1] = i2;
; CHECK:       |         |   %y.12 = %y.12  +  0.000000e+00;
; CHECK:       |         |   %y.12.out = %y.12;
; CHECK:       |         + END LOOP
; CHECK:       |      }
; CHECK:       |      else
; CHECK:       |      {
; CHECK:       |         + DO i32 i2 = 0, %n + -1, 1
; CHECK:       |         |   (%p)[i1] = i2;
; CHECK:       |         |   %y.12.out = %y.12;
; CHECK:       |         + END LOOP
; CHECK:       |      }
; CHECK:       |      %y.04 = %y.12.out;
; CHECK:       |      <RVAL-REG> NON-LINEAR float %y.12.out
; CHECK:       |   }
; CHECK:       |   %y.04.out = %y.04;
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local float @foo(i32* %p, i32 %n, i32 %m) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc5
  %i.05 = phi i32 [ 0, %entry ], [ %inc6, %for.inc5 ]
  %y.04 = phi float [ 0.000000e+00, %entry ], [ %y.1.lcssa, %for.inc5 ]
  %cmp21 = icmp slt i32 0, %n
  br i1 %cmp21, label %for.body3.lr.ph, label %for.end

for.body3.lr.ph:                                  ; preds = %for.body
  br label %for.body3

for.body3:                                        ; preds = %for.body3.lr.ph, %for.inc
  %j.03 = phi i32 [ 0, %for.body3.lr.ph ], [ %inc, %for.inc ]
  %y.12 = phi float [ %y.04, %for.body3.lr.ph ], [ %y.2, %for.inc ]
  %idxprom = sext i32 %i.05 to i64
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %idxprom
  store i32 %j.03, i32* %arrayidx, align 4
  %cmp4 = icmp sgt i32 %i.05, 100
  br i1 %cmp4, label %if.then, label %if.end

if.then:                                          ; preds = %for.body3
  %add = fadd float %y.12, 0.000000e+00
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body3
  %y.2 = phi float [ %add, %if.then ], [ %y.12, %for.body3 ]
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %inc = add nsw i32 %j.03, 1
  %cmp2 = icmp slt i32 %inc, %n
  br i1 %cmp2, label %for.body3, label %for.cond1.for.end_crit_edge

for.cond1.for.end_crit_edge:                      ; preds = %for.inc
  %split = phi float [ %y.2, %for.inc ]
  br label %for.end

for.end:                                          ; preds = %for.cond1.for.end_crit_edge, %for.body
  %y.1.lcssa = phi float [ %split, %for.cond1.for.end_crit_edge ], [ %y.04, %for.body ]
  br label %for.inc5

for.inc5:                                         ; preds = %for.end
  %inc6 = add nsw i32 %i.05, 1
  %cmp = icmp slt i32 %inc6, 100
  br i1 %cmp, label %for.body, label %for.end7

for.end7:                                         ; preds = %for.inc5
  %y.0.lcssa = phi float [ %y.1.lcssa, %for.inc5 ]
  ret float %y.0.lcssa
}


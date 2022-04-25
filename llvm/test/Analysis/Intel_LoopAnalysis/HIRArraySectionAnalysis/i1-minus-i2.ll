; RUN: opt < %s -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-array-section-analysis | FileCheck %s
; RUN: opt < %s -aa-pipeline=basic-aa -passes="hir-ssa-deconstruction,print<hir-array-section-analysis>" -disable-output 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, 1, 1   <DO_LOOP>
;       |   + DO i2 = 0, 99, 1   <DO_LOOP>
;       |   |   (%p)[i1 + -1 * i2] = 1;
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { }
; CHECK:    + DO i1 = 0, 1, 1   <DO_LOOP>
; CHECK:    %p: (DEF) [i1 + -1 * i2:-99:1]
; CHECK:    |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:        %p: (DEF) [i1 + -1 * i2:i1 + -99:i1]
; CHECK:    |   + END LOOP
; CHECK:    + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* %p) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc4
  %i.02 = phi i32 [ 0, %entry ], [ %inc5, %for.inc4 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body, %for.inc
  %j.01 = phi i32 [ 0, %for.body ], [ %inc, %for.inc ]
  %sub = sub nsw i32 %i.02, %j.01
  %idxprom = sext i32 %sub to i64
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %idxprom
  store i32 1, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3
  %inc = add nsw i32 %j.01, 1
  %cmp2 = icmp slt i32 %inc, 100
  br i1 %cmp2, label %for.body3, label %for.end

for.end:                                          ; preds = %for.inc
  br label %for.inc4

for.inc4:                                         ; preds = %for.end
  %inc5 = add nsw i32 %i.02, 1
  %cmp = icmp slt i32 %inc5, 2
  br i1 %cmp, label %for.body, label %for.end6

for.end6:                                         ; preds = %for.inc4
  ret void
}


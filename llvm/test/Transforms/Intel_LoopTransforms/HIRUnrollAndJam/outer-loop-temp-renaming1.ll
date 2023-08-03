; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,hir-lmm,print<hir>,hir-unroll-and-jam,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that unroll & jam renames %s.020 for different unrolled iterations.
; The problem was that %s.020 was marked as livein to the i1 loop and this
; info was not updated for the 'if' version of the loop after predicate opt.

; Unroll & jam was assuming this info to always be precise, thus incorrectly
; deducing %s.020 to be loopnest reduction temp and skipped its renaming.

; Incoming HIR-
; + DO i1 = 0, 99, 1   <DO_LOOP>
; |   + DO i2 = 0, 99, 1   <DO_LOOP>
; |   |   if (%t > 0)
; |   |   {
; |   |      %s.020 = (%B)[i1];
; |   |   }
; |   |   (%A)[i2] = %s.020;
; |   + END LOOP
; + END LOOP

; CHECK: if (%t > 0)
; CHECK: {
; CHECK:    + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:    |      %s.020 = (%B)[i1];
; CHECK:    |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:    |   |   (%A)[i2] = %s.020;
; CHECK:    |   + END LOOP
; CHECK:    + END LOOP
; CHECK: }
; CHECK: else
; CHECK: {
; CHECK:    + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:    |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:    |   |   (%A)[i2] = %s.020;
; CHECK:    |   + END LOOP
; CHECK:    + END LOOP
; CHECK: }

; CHECK: + DO i1 = 0, 11, 1   <DO_LOOP> <nounroll and jam>
; CHECK: |      %temp = (%B)[8 * i1];
; CHECK: |      %temp2 = (%B)[8 * i1 + 1];
; CHECK: |      %temp3 = (%B)[8 * i1 + 2];
; CHECK: |      %temp4 = (%B)[8 * i1 + 3];
; CHECK: |      %temp5 = (%B)[8 * i1 + 4];
; CHECK: |      %temp6 = (%B)[8 * i1 + 5];
; CHECK: |      %temp7 = (%B)[8 * i1 + 6];
; CHECK: |      %s.020 = (%B)[8 * i1 + 7];
; CHECK: |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   (%A)[i2] = %temp;
; CHECK: |   |   (%A)[i2] = %temp2;
; CHECK: |   |   (%A)[i2] = %temp3;
; CHECK: |   |   (%A)[i2] = %temp4;
; CHECK: |   |   (%A)[i2] = %temp5;
; CHECK: |   |   (%A)[i2] = %temp6;
; CHECK: |   |   (%A)[i2] = %temp7;
; CHECK: |   |   (%A)[i2] = %s.020;
; CHECK: |   + END LOOP
; CHECK: + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo(ptr noalias %A, ptr noalias %B, i32 %t) {
entry:
  %cmp4 = icmp sgt i32 %t, 0
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc7
  %indvars.iv21 = phi i64 [ 0, %entry ], [ %indvars.iv.next22, %for.inc7 ]
  %s.020 = phi i32 [ 0, %entry ], [ %s.2.lcssa, %for.inc7 ]
  %ptridx = getelementptr inbounds i32, ptr %B, i64 %indvars.iv21
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %if.end
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %if.end ]
  %s.118 = phi i32 [ %s.020, %for.cond1.preheader ], [ %s.2, %if.end ]
  br i1 %cmp4, label %if.then, label %if.end

if.then:                                          ; preds = %for.body3
  %0 = load i32, ptr %ptridx, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body3
  %s.2 = phi i32 [ %0, %if.then ], [ %s.118, %for.body3 ]
  %ptridx6 = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  store i32 %s.2, ptr %ptridx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.inc7, label %for.body3

for.inc7:                                         ; preds = %if.end
  %s.2.lcssa = phi i32 [ %s.2, %if.end ]
  %indvars.iv.next22 = add nuw nsw i64 %indvars.iv21, 1
  %exitcond23.not = icmp eq i64 %indvars.iv.next22, 100
  br i1 %exitcond23.not, label %for.end9, label %for.cond1.preheader

for.end9:                                         ; preds = %for.inc7
  ret void
}


; RUN: opt < %s -hir-ssa-deconstruction -hir-unroll-and-jam -print-before=hir-unroll-and-jam -print-after=hir-unroll-and-jam 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-unroll-and-jam,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that we unroll & jam i1 loop by 5, specified via unroll metadata.

; + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; |   |   %0 = (@B)[0][i2];
; |   |   %1 = (@A)[0][i1];
; |   |   (@A)[0][i1] = %0 + %1;
; |   + END LOOP
; + END LOOP


; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100> <unroll and jam = 5>
; CHECK: + END LOOP

; CHECK: BEGIN REGION { modified }
; CHECK: %tgu = (%n)/u5;

; CHECK: + DO i1 = 0, %tgu + -1, 1   <DO_LOOP>  <MAX_TC_EST = 20>
; CHECK: |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   + END LOOP
; CHECK: + END LOOP

; CHECK: + DO i1 = 5 * %tgu, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4>
; CHECK: |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   + END LOOP
; CHECK: + END LOOP
; CHECK: END REGION


; ModuleID = 'simple-unroll-and-jam.ll'
source_filename = "simple-unroll-and-jam.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@A = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define void @foo(i32 %n) local_unnamed_addr {
entry:
  %cmp3 = icmp slt i32 0, %n
  br i1 %cmp3, label %for.cond1.preheader.lr.ph, label %for.end8

for.cond1.preheader.lr.ph:                        ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.inc6
  %i.04 = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %inc7, %for.inc6 ]
  %cmp21 = icmp slt i32 0, %n
  br i1 %cmp21, label %for.body3.lr.ph, label %for.inc6

for.body3.lr.ph:                                  ; preds = %for.cond1.preheader
  br label %for.body3

for.body3:                                        ; preds = %for.body3.lr.ph, %for.body3
  %j.02 = phi i32 [ 0, %for.body3.lr.ph ], [ %inc, %for.body3 ]
  %idxprom = sext i32 %j.02 to i64
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  %idxprom4 = sext i32 %i.04 to i64
  %arrayidx5 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %idxprom4
  %1 = load i32, i32* %arrayidx5, align 4
  %add = add nsw i32 %1, %0
  store i32 %add, i32* %arrayidx5, align 4
  %inc = add nsw i32 %j.02, 1
  %cmp2 = icmp slt i32 %inc, %n
  br i1 %cmp2, label %for.body3, label %for.cond1.for.inc6_crit_edge

for.cond1.for.inc6_crit_edge:                     ; preds = %for.body3
  br label %for.inc6

for.inc6:                                         ; preds = %for.cond1.for.inc6_crit_edge, %for.cond1.preheader
  %inc7 = add nsw i32 %i.04, 1
  %cmp = icmp slt i32 %inc7, %n
  br i1 %cmp, label %for.cond1.preheader, label %for.cond.for.end8_crit_edge, !llvm.loop !0

for.cond.for.end8_crit_edge:                      ; preds = %for.inc6
  br label %for.end8

for.end8:                                         ; preds = %for.cond.for.end8_crit_edge, %entry
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll_and_jam.count", i32 5}


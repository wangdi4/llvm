; RUN: opt -passes='hir-ssa-deconstruction,hir-temp-cleanup,print<hir-safe-reduction-analysis>,hir-general-unroll,print<hir>' -disable-output < %s 2>&1 | FileCheck %s

; Verify that HIR is printed successfully after unrolling.

; Print was failing because utility which replaces single iteration remainder
; loop by its body was not invalidating the analyses so safe reduction became
; stale and asserted from inside the print function which uses it for printing
; safe reduction markings.


; Incoming HIR-
; + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647> <unroll = 2>
; |   |   %t.019 = (%A)[i1 + i2]  +  %t.019;
; |   + END LOOP
; + END LOOP


; CHECK: + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK: |   %tgu = (zext.i32.i64(%n))/u2;
; CHECK: |
; CHECK: |   + DO i2 = 0, %tgu + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1073741823>  <LEGAL_MAX_TC = 1073741823> <nounroll>
; CHECK: |   |   %t.019 = (%A)[i1 + 2 * i2]  +  %t.019;
; CHECK: |   |   %t.019 = (%A)[i1 + 2 * i2 + 1]  +  %t.019;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   if (2 * %tgu <u zext.i32.i64(%n))
; CHECK: |   {
; CHECK: |      %t.019 = (%A)[i1 + 2 * %tgu]  +  %t.019;
; CHECK: |   }
; CHECK: + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo(ptr nocapture noundef readonly %A, i32 noundef %n) {
entry:
  %cmp17 = icmp sgt i32 %n, 0
  br i1 %cmp17, label %for.cond1.preheader.lr.ph, label %for.end7

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %wide.trip.count24 = zext i32 %n to i64
  br label %for.body3.preheader

for.body3.preheader:                              ; preds = %for.inc5, %for.cond1.preheader.lr.ph
  %indvars.iv22 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next23, %for.inc5 ]
  %t.019 = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %add4.lcssa, %for.inc5 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.body3.preheader ], [ %indvars.iv.next, %for.body3 ]
  %t.116 = phi i32 [ %t.019, %for.body3.preheader ], [ %add4, %for.body3 ]
  %0 = add nuw nsw i64 %indvars.iv, %indvars.iv22
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %0
  %1 = load i32, ptr %arrayidx, align 4
  %add4 = add nsw i32 %1, %t.116
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count24
  br i1 %exitcond.not, label %for.inc5, label %for.body3, !llvm.loop !0

for.inc5:                                         ; preds = %for.body3
  %add4.lcssa = phi i32 [ %add4, %for.body3 ]
  %indvars.iv.next23 = add nuw nsw i64 %indvars.iv22, 1
  %exitcond25.not = icmp eq i64 %indvars.iv.next23, %wide.trip.count24
  br i1 %exitcond25.not, label %for.end7.loopexit, label %for.body3.preheader

for.end7.loopexit:                                ; preds = %for.inc5
  %add4.lcssa.lcssa = phi i32 [ %add4.lcssa, %for.inc5 ]
  br label %for.end7

for.end7:                                         ; preds = %for.end7.loopexit, %entry
  %t.0.lcssa = phi i32 [ 0, %entry ], [ %add4.lcssa.lcssa, %for.end7.loopexit ]
  ret i32 %t.0.lcssa
}


!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.count", i32 2}

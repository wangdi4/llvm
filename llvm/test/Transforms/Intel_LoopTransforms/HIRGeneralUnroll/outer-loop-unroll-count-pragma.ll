; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-general-unroll,print<hir>" -S < %s 2>&1 | FileCheck %s

; Verify that we unroll the outer loop with unroll metadata.
; We unroll the inner loop using our own heuristics.

; HIR-
; + DO i1 = 0, 9, 1   <DO_LOOP>
; |      %0 = (%A)[i1];
; |   + DO i2 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; |   |   %0 = %0  +  (%B)[i2];
; |   |   (%A)[i1] = %0;
; |   + END LOOP
; + END LOOP

; Output is too big. Only checking some of it.

; CHECK: BEGIN REGION { modified }

; CHECK: + DO i1 = 0, 2, 1   <DO_LOOP>

; CHECK: |      %0 = (%A)[3 * i1];
; CHECK: |      %tgu = (sext.i32.i64(%n))/u8;
; CHECK: |
; CHECK: |      + DO i2 = 0, %tgu + -1, 1   <DO_LOOP>
; CHECK: |      |   %0 = %0  +  (%B)[8 * i2];
; CHECK: |      |   (%A)[3 * i1] = %0;

; CHECK: + END LOOP


; CHECK: + DO i1 = 9, 9, 1   <DO_LOOP>

; CHECK: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(ptr nocapture %A, ptr nocapture readonly %B, i32 %n) local_unnamed_addr {
entry:
  %cmp215 = icmp sgt i32 %n, 0
  %wide.trip.count = sext i32 %n to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc6, %entry
  %indvars.iv18 = phi i64 [ 0, %entry ], [ %indvars.iv.next19, %for.inc6 ]
  br i1 %cmp215, label %for.body3.lr.ph, label %for.inc6

for.body3.lr.ph:                                  ; preds = %for.cond1.preheader
  %arrayidx5 = getelementptr inbounds i32, ptr %A, i64 %indvars.iv18
  %.pre = load i32, ptr %arrayidx5, align 4
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body3.lr.ph
  %0 = phi i32 [ %.pre, %for.body3.lr.ph ], [ %add, %for.body3 ]
  %indvars.iv = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx, align 4
  %add = add nsw i32 %0, %1
  store i32 %add, ptr %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.inc6.loopexit, label %for.body3

for.inc6.loopexit:                                ; preds = %for.body3
  br label %for.inc6

for.inc6:                                         ; preds = %for.inc6.loopexit, %for.cond1.preheader
  %indvars.iv.next19 = add nuw nsw i64 %indvars.iv18, 1
  %exitcond20 = icmp eq i64 %indvars.iv.next19, 10
  br i1 %exitcond20, label %for.end8, label %for.cond1.preheader, !llvm.loop !0

for.end8:                                         ; preds = %for.inc6
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.count", i32 3}


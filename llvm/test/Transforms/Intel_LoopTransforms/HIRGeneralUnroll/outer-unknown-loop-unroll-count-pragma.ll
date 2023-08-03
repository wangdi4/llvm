; RUN: opt -hir-cost-model-throttling=0 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-general-unroll,print<hir>" -S < %s 2>&1 | FileCheck %s

; Verify that we unroll the outer unknown loop with unroll metadata.
; We unroll the inner loop using our own heuristics.

; HIR-
; + UNKNOWN LOOP i1
; |   <i1 = 0>
; |   for.body3.lr.ph:
; |   %i.018.out = %i.018;
; |   %0 = (%A)[%i.018];
; |
; |   + DO i2 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; |   |   %0 = %0  +  (%B)[i2];
; |   |   (%A)[%i.018] = %0;
; |   + END LOOP
; |
; |   %i.018 = 2 * %i.018;
; |   if (2 * %i.018.out < %n)
; |   {
; |      <i1 = i1 + 1>
; |      goto for.body3.lr.ph;
; |   }
; + END LOOP

; Output is too big. Only checking some part of it.

; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   for.body3.lr.ph:
; CHECK: |   %i.018.out = %i.018;
; CHECK: |   %0 = (%A)[%i.018];
; CHECK: |   %tgu = (sext.i32.i64(%n))/u8;
; CHECK: |
; CHECK: |   + DO i2 = 0, %tgu + -1, 1   <DO_LOOP>
; CHECK: |   |   %0 = %0  +  (%B)[8 * i2];
; CHECK: |   |   (%A)[%i.018] = %0;

; CHECK: |   + END LOOP

; CHECK: |   if (2 * %i.018.out >= %n)
; CHECK: |   {
; CHECK: |      goto loopexit.50;
; CHECK: |   }

; CHECK: |   %i.018.out = %i.018;
; CHECK: |   %0 = (%A)[%i.018];
; CHECK: |   %tgu = (sext.i32.i64(%n))/u8;
; CHECK: |
; CHECK: |   + DO i2 = 0, %tgu + -1, 1   <DO_LOOP>
; CHECK: |   |   %0 = %0  +  (%B)[8 * i2];
; CHECK: |   |   (%A)[%i.018] = %0;

; CHECK: |   + END LOOP
; CHECK: + END LOOP

; CHECK: loopexit.50:

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(ptr nocapture %A, ptr nocapture readonly %B, i32 %n) local_unnamed_addr {
entry:
  %cmp17 = icmp sgt i32 %n, 1
  br i1 %cmp17, label %for.cond1.preheader.lr.ph, label %for.end7

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.inc6, %for.cond1.preheader.lr.ph
  %i.018 = phi i32 [ 1, %for.cond1.preheader.lr.ph ], [ %mul, %for.inc6 ]
  %idxprom4 = sext i32 %i.018 to i64
  %arrayidx5 = getelementptr inbounds i32, ptr %A, i64 %idxprom4
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
  br i1 %exitcond, label %for.inc6, label %for.body3

for.inc6:                                         ; preds = %for.body3
  %mul = shl nsw i32 %i.018, 1
  %cmp = icmp slt i32 %mul, %n
  br i1 %cmp, label %for.body3.lr.ph, label %for.end7.loopexit, !llvm.loop !0

for.end7.loopexit:                                ; preds = %for.inc6
  br label %for.end7

for.end7:                                         ; preds = %for.end7.loopexit, %entry
  ret void
}


!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.count", i32 2}

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -disable-output -hir-pre-vec-complete-unroll -print-after=hir-pre-vec-complete-unroll -hir-details < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,print<hir>" -disable-output -hir-details < %s 2>&1 | FileCheck %s

; Verify that we unroll the i1 and i3 loops with unroll count metadata.

; HIR-
; + DO i1 = 0, 1, 1   <DO_LOOP>
; |   + DO i2 = 0, 3, 1   <DO_LOOP>
; |   |   + DO i3 = 0, 2, 1   <DO_LOOP>
; |   |   |      %1 = (%A)[i1 + i3];
; |   |   |   + DO i4 = 0, i3 + -2, 1   <DO_LOOP>
; |   |   |   |   %1 = %1  +  (%B)[i2 + i4];
; |   |   |   |   (%A)[i1 + i3] = %1;
; |   |   |   + END LOOP
; |   |   + END LOOP
; |   + END LOOP
; + END LOOP

; CHECK: + Ztt: No
; CHECK: + DO i64 i1 = 0, 3, 1   <DO_LOOP>
; CHECK: |      %1 = (%A)[2];
; CHECK: |   + Ztt: No
; CHECK: |   + DO i64 i2 = 0, 0, 1   <DO_LOOP>
; CHECK: |   |   %1 = %1  +  (%B)[i1 + i2];
; CHECK: |   |   (%A)[2] = %1;
; CHECK: |   + END LOOP
; CHECK: + END LOOP

; CHECK: + Ztt: No
; CHECK: + DO i64 i1 = 0, 3, 1   <DO_LOOP>
; CHECK: |      %1 = (%A)[3];
; CHECK: |   + Ztt: No
; CHECK: |   + DO i64 i2 = 0, 0, 1   <DO_LOOP>
; CHECK: |   |   %1 = %1  +  (%B)[i1 + i2];
; CHECK: |   |   (%A)[3] = %1;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i32* nocapture %A, i32* nocapture readonly %B, i32 %n) local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc20, %entry
  %indvars.iv48 = phi i64 [ 0, %entry ], [ %indvars.iv.next49, %for.inc20 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc17, %for.cond1.preheader
  %indvars.iv45 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next46, %for.inc17 ]
  br label %for.cond7.preheader

for.cond7.preheader:                              ; preds = %for.inc14, %for.cond4.preheader
  %indvars.iv41 = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next42, %for.inc14 ]
  %cmp835 = icmp sgt i64 %indvars.iv41, 1
  br i1 %cmp835, label %for.body9.lr.ph, label %for.inc14

for.body9.lr.ph:                                  ; preds = %for.cond7.preheader
  %0 = add nuw nsw i64 %indvars.iv41, %indvars.iv48
  %arrayidx12 = getelementptr inbounds i32, i32* %A, i64 %0
  %.pre = load i32, i32* %arrayidx12, align 4
  br label %for.body9

for.body9:                                        ; preds = %for.body9, %for.body9.lr.ph
  %1 = phi i32 [ %.pre, %for.body9.lr.ph ], [ %add13, %for.body9 ]
  %indvars.iv = phi i64 [ 0, %for.body9.lr.ph ], [ %indvars.iv.next, %for.body9 ]
  %2 = add nuw nsw i64 %indvars.iv, %indvars.iv45
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %2
  %3 = load i32, i32* %arrayidx, align 4
  %add13 = add nsw i32 %1, %3
  store i32 %add13, i32* %arrayidx12, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %x = add nuw nsw i64 %indvars.iv41, -1
  %exitcond = icmp eq i64 %indvars.iv.next, %x
  br i1 %exitcond, label %for.inc14.loopexit, label %for.body9

for.inc14.loopexit:                               ; preds = %for.body9
  br label %for.inc14

for.inc14:                                        ; preds = %for.inc14.loopexit, %for.cond7.preheader
  %indvars.iv.next42 = add nuw nsw i64 %indvars.iv41, 1
  %exitcond44 = icmp eq i64 %indvars.iv.next42, 3
  br i1 %exitcond44, label %for.inc17, label %for.cond7.preheader, !llvm.loop !0

for.inc17:                                        ; preds = %for.inc14
  %indvars.iv.next46 = add nuw nsw i64 %indvars.iv45, 1
  %exitcond47 = icmp eq i64 %indvars.iv.next46, 4
  br i1 %exitcond47, label %for.inc20, label %for.cond4.preheader

for.inc20:                                        ; preds = %for.inc17
  %indvars.iv.next49 = add nuw nsw i64 %indvars.iv48, 1
  %exitcond50 = icmp eq i64 %indvars.iv.next49, 2
  br i1 %exitcond50, label %for.end22, label %for.cond1.preheader, !llvm.loop !2

for.end22:                                        ; preds = %for.inc20
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.count", i32 3}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.unroll.count", i32 2}

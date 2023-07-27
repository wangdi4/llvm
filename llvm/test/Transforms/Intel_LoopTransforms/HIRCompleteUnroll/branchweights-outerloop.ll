; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,hir-cg" -S < %s 2>&1 | FileCheck %s

; Verify that after i1 and i3 are unrolled, branch_weights are adjusted accordingly. Backedge's weights of i2 are divided by 2, and those of i4 ared dividied by 3.
; Input code is adopted from outer-loop-pragma-unroll1.ll.

; TODO: The level 4 loop(i.e. the innermost)  body's TC is dependent on the IV of its parent loop. Currenlty, branch weights of level 4 loop is adjust without considering new TC of after IV replacemet if iv 3. Updating TCs of unrolled version of the innermost loop proportional to the new TCs might be useful in certain cases.

; Before Unroll
; + DO i1 = 0, 1, 1   <DO_LOOP>  // unroll by 2
; |   + DO i2 = 0, 3, 1   <DO_LOOP>  // Branch weights here divided by 2
; |   |   + DO i3 = 0, 2, 1   <DO_LOOP> // unroll by 3
; |   |   |      %1 = (%A)[i1 + i3];
; |   |   |   + DO i4 = 0, i3 + -1, 1   <DO_LOOP> // Branch weights here divided by (2 * 3)
; |   |   |   |   %1 = %1  +  (%B)[i2 + i4];
; |   |   |   |   (%A)[i1 + i3] = %1;
; |   |   |   + END LOOP
; |   |   + END LOOP
; |   + END LOOP
; + END LOOP


; After Unroll - i1 and i3 are completely unrolled
; BEGIN REGION
;  + DO i1 = 0, 3, 1   <DO_LOOP>
;  |      %1 = (%A)[1];
;  |   + DO i2 = 0, 0, 1   <DO_LOOP>  <MAX_TC_EST = 2>
;  |   |   %1 = %1  +  (%B)[i1 + i2];
;  |   |   (%A)[1] = %1;
;  |   + END LOOP
;  |
;  |
;  |      %1 = (%A)[2];
;  |   + DO i2 = 0, 1, 1   <DO_LOOP>  <MAX_TC_EST = 2>
;  |   |   %1 = %1  +  (%B)[i1 + i2];
;  |   |   (%A)[2] = %1;
;  |   + END LOOP
;  + END LOOP
;
;  + DO i1 = 0, 3, 1   <DO_LOOP>
;  |      %1 = (%A)[1];
;  |   + DO i2 = 0, 0, 1   <DO_LOOP>  <MAX_TC_EST = 2>
;  |   |   %1 = %1  +  (%B)[i1 + i2];
;  |   |   (%A)[1] = %1;
;  |   + END LOOP
;  |
;  |
;  |      %1 = (%A)[2];
;  |   + DO i2 = 0, 1, 1   <DO_LOOP>  <MAX_TC_EST = 2>
;  |   |   %1 = %1  +  (%B)[i1 + i2];
;  |   |   (%A)[2] = %1;
;  |   + END LOOP
;  + END LOOP
;END REGION

;CHECK: region.0:

;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_INNER:[0-9]+]]
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_INNER]]
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_OUTER:[0-9]+]]

;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_INNER]]
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_INNER]]
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_OUTER]]

;CHECK-DAG: ![[PROF_INNER]] = !{!"branch_weights", i32 1200, i32 6}
;CHECK-DAG: ![[PROF_OUTER]] = !{!"branch_weights", i32 400, i32 2}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(ptr nocapture %A, ptr nocapture readonly %B, i32 %n) local_unnamed_addr #0 {
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
  %cmp835 = icmp sgt i64 %indvars.iv41, 0
  br i1 %cmp835, label %for.body9.lr.ph, label %for.inc14, !prof !8 ;i4

for.body9.lr.ph:                                  ; preds = %for.cond7.preheader
  %0 = add nuw nsw i64 %indvars.iv41, %indvars.iv48
  %arrayidx12 = getelementptr inbounds i32, ptr %A, i64 %0
  %.pre = load i32, ptr %arrayidx12, align 4
  br label %for.body9

for.body9:                                        ; preds = %for.body9, %for.body9.lr.ph
  %1 = phi i32 [ %.pre, %for.body9.lr.ph ], [ %add13, %for.body9 ]
  %indvars.iv = phi i64 [ 0, %for.body9.lr.ph ], [ %indvars.iv.next, %for.body9 ]
  %2 = add nuw nsw i64 %indvars.iv, %indvars.iv45
  %arrayidx = getelementptr inbounds i32, ptr %B, i64 %2
  %3 = load i32, ptr %arrayidx, align 4
  %add13 = add nsw i32 %1, %3
  store i32 %add13, ptr %arrayidx12, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %indvars.iv41
  br i1 %exitcond, label %for.inc14.loopexit, label %for.body9, !prof !7 ;i4 loop inverse

for.inc14.loopexit:                               ; preds = %for.body9
  br label %for.inc14

for.inc14:                                        ; preds = %for.inc14.loopexit, %for.cond7.preheader
  %indvars.iv.next42 = add nuw nsw i64 %indvars.iv41, 1
  %exitcond44 = icmp eq i64 %indvars.iv.next42, 3
  br i1 %exitcond44, label %for.inc17, label %for.cond7.preheader, !llvm.loop !0, !prof !6;i3 loop inverse

for.inc17:                                        ; preds = %for.inc14
  %indvars.iv.next46 = add nuw nsw i64 %indvars.iv45, 1
  %exitcond47 = icmp eq i64 %indvars.iv.next46, 4
  br i1 %exitcond47, label %for.inc20, label %for.cond4.preheader, !prof !5;i2 loop inverse

for.inc20:                                        ; preds = %for.inc17
  %indvars.iv.next49 = add nuw nsw i64 %indvars.iv48, 1
  %exitcond50 = icmp eq i64 %indvars.iv.next49, 2
  br i1 %exitcond50, label %for.end22, label %for.cond1.preheader, !prof !4, !llvm.loop !2 ;i1 loop inverse

for.end22:                                        ; preds = %for.inc20
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.count", i32 3}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.unroll.count", i32 2}

!4 = !{!"branch_weights", i32 1, i32 200}   ;i1
!5 = !{!"branch_weights", i32 4, i32 800}   ;i2
!6 = !{!"branch_weights", i32 12, i32 2400} ;i3
!7 = !{!"branch_weights", i32 36, i32 7200} ;i4
!8 = !{!"branch_weights", i32 7200, i32 36} ;i4'

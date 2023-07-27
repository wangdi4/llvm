; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll" -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that the only savings in the outer and inner loop is due to substitution of IV by constant.

; The reduction should not be treated as simplified due to the assignment %t.018 = 0 in outer loop.
; Neither should the assignment %t.018 = 0 result in any new savings by unrolling.
; Only savings in the examples below are from the presence of induction variables i1 and i2.

; CHECK: Savings: 110

; CHECK: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   %t.018 = 0;
; CHECK: |
; CHECK: |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   %0 = (%A)[i2];
; CHECK: |   |   %t.018 = %0  +  %t.018;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   (%B)[i1] = %t.018;
; CHECK: + END LOOP


; CHECK: Savings: 10

; One ddref each accounted for %0, %t.018 and (%A)[i2].
; CHECK: Number of ddrefs: 30

; CHECK: + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK: |   %0 = (%A)[i2];
; CHECK: |   %t.018 = %0  +  %t.018;
; CHECK: + END LOOP

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr nocapture readonly %A, ptr nocapture %B) local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.end, %entry
  %indvars.iv20 = phi i64 [ 0, %entry ], [ %indvars.iv.next21, %for.end ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %t.018 = phi i32 [ 0, %for.cond1.preheader ], [ %add, %for.body3 ]
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %add = add nsw i32 %0, %t.018
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.end, label %for.body3

for.end:                                          ; preds = %for.body3
  %add.lcssa = phi i32 [ %add, %for.body3 ]
  %arrayidx5 = getelementptr inbounds i32, ptr %B, i64 %indvars.iv20
  store i32 %add.lcssa, ptr %arrayidx5, align 4
  %indvars.iv.next21 = add nuw nsw i64 %indvars.iv20, 1
  %exitcond22 = icmp eq i64 %indvars.iv.next21, 10
  br i1 %exitcond22, label %for.end8, label %for.cond1.preheader

for.end8:                                         ; preds = %for.end
  ret void
}


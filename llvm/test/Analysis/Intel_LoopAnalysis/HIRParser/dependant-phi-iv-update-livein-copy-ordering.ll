; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir>" -disable-output  2>&1 | FileCheck %s

; Both %t18 and %sub9238.lcssa246250 are loop header phis which are
; deconstructed.

; Verify that this livein copy of %t18:
; %t18 = %t30;
; is not moved before the livein copy of %sub9238.lcssa246250:
; %sub9238.lcssa246250 = 17 * i1 + -17 * %t18 + %sub9238.lcssa246250.out + 17;

; The SCEV form of %sub9238.lcssa246250 looks like an IV and we try to keep
; IV update inst at the end of the loop to prevent live-range issues in cases
; where IV is parsed as a blob.
; In this case though, the SCEV form uses %t8 so reordering them is incorrect.
; Before the fix, the order of the two copies was reversed.


; CHECK: + DO i1 = 0, 42, 1   <DO_LOOP>
; CHECK: |   %sub9238.lcssa246250.out = %sub9238.lcssa246250;
; CHECK: |   %t19 = trunc.i64.i32(i1 + 1);
; CHECK: |
; CHECK: |   + DO i2 = 0, 16, 1   <DO_LOOP>
; CHECK: |   |   %sub9 = i1 + -1 * %t18 + 1  +  ((-1 * %t18) + %t19) * i2 + %sub9238.lcssa246250.out;
; CHECK: |   |   %t23 = (%g)[0][i2 + 1][i2 + 2];
; CHECK: |   |   %t25 = (%g)[0][i2][i1 + 2];
; CHECK: |   |   (%g)[0][i2][i1 + 2] = -1 * %t23 + %t25;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %t30 = (%ji3)[0][i1];
; CHECK: |   %sub9238.lcssa246250 = 17 * i1 + -17 * %t18 + %sub9238.lcssa246250.out + 17;
; CHECK: |   %t18 = %t30;
; CHECK: + END LOOP


define void @foo(i32 %hb.promoted245, i32 %ns.promoted247, i32 %ne.promoted, ptr %g, ptr %l, ptr %ji3, ptr %jp) {
entry:
  br label %for.cond6.preheader

for.cond6.preheader:                              ; preds = %for.end, %entry
  %indvars.iv261 = phi i64 [ 1, %entry ], [ %indvars.iv.next262, %for.end ]
  %sub9238.lcssa246250 = phi i32 [ %hb.promoted245, %entry ], [ %sub9.lcssa, %for.end ]
  %inc239.lcssa248249 = phi i32 [ %ns.promoted247, %entry ], [ %t29, %for.end ]
  %t18 = phi i32 [ %ne.promoted, %entry ], [ %t30, %for.end ]
  %t19 = trunc i64 %indvars.iv261 to i32
  %sub.neg = sub i32 %t19, %t18
  %indvars.iv.next262 = add nuw nsw i64 %indvars.iv261, 1
  %t20 = add nsw i64 %indvars.iv261, -1
  br label %for.body8

for.body8:                                        ; preds = %for.body8, %for.cond6.preheader
  %indvars.iv = phi i64 [ 1, %for.cond6.preheader ], [ %indvars.iv.next, %for.body8 ]
  %sub9238241 = phi i32 [ %sub9238.lcssa246250, %for.cond6.preheader ], [ %sub9, %for.body8 ]
  %inc239240 = phi i32 [ %inc239.lcssa248249, %for.cond6.preheader ], [ %inc, %for.body8 ]
  %inc = add i32 %inc239240, 1
  %sub9 = add i32 %sub.neg, %sub9238241
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx12 = getelementptr inbounds [100 x [100 x i32]], ptr %g, i64 0, i64 %indvars.iv, i64 %indvars.iv.next
  %t23 = load i32, ptr %arrayidx12, align 4
  %t24 = add nsw i64 %indvars.iv, -1
  %arrayidx18 = getelementptr inbounds [100 x [100 x i32]], ptr %g, i64 0, i64 %t24, i64 %indvars.iv.next262
  %t25 = load i32, ptr %arrayidx18, align 4
  %sub19 = sub i32 %t25, %t23
  store i32 %sub19, ptr %arrayidx18, align 4
  %arrayidx21 = getelementptr inbounds [100 x i32], ptr %l, i64 0, i64 %indvars.iv
  %exitcond.not = icmp eq i64 %indvars.iv.next, 18
  br i1 %exitcond.not, label %for.end, label %for.body8

for.end:                                          ; preds = %for.body8
  %sub9.lcssa = phi i32 [ %sub9, %for.body8 ]
  %t29 = add i32 %inc239.lcssa248249, 17
  %arrayidx32 = getelementptr inbounds [100 x i32], ptr %ji3, i64 0, i64 %t20
  %t30 = load i32, ptr %arrayidx32, align 4
  %exitcond265.not = icmp eq i64 %indvars.iv.next262, 44
  br i1 %exitcond265.not, label %for.cond36.preheader, label %for.cond6.preheader

for.cond36.preheader:                             ; preds = %for.end
  %.lcssa = phi i32 [ %t30, %for.end ]
  %sub9.lcssa.lcssa = phi i32 [ %sub9.lcssa, %for.end ]
  ret void
}

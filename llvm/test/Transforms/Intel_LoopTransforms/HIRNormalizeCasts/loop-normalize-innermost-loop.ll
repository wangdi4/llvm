; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-normalize-casts,print<hir>" %s -disable-output 2>&1 | FileCheck %s

; This test case checks that the zext innermost loop was normalized since
; we have information about the legal max trip count, but the outer loop
; wasn't normalized.

; HIR before transformation

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, zext.i32.i64(%rows) + -1, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, zext.i32.i64(%cols) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   |   %3 = (%arr)[sext.i32.i64(%cols) * i1 + i2];
; CHECK:       |   |   %mul.i6926 = %3  *  %3;
; CHECK:       |   |   %result.031.i = %mul.i6926  +  %result.031.i;
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION

; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, zext.i32.i64(%rows) + -1, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, sext.i32.i64(%cols) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   |   %3 = (%arr)[sext.i32.i64(%cols) * i1 + i2];
; CHECK:       |   |   %mul.i6926 = %3  *  %3;
; CHECK:       |   |   %result.031.i = %mul.i6926  +  %result.031.i;
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION


define void @foo(i32 %rows, i32 %cols, ptr %arr) {
entry:
  br label %for.cond2.preheader.lr.ph.i6919

for.cond2.preheader.lr.ph.i6919:                  ; preds = %"??1?$Matrix@N@sfm@@QEAA@XZ.exit6913"
  %cmp327.i = icmp sgt i32 %cols, 0
  %0 = sext i32 %cols to i64
  %wide.trip.count38.i = zext i32 %rows to i64
  %wide.trip.count.i6918 = zext i32 %cols to i64
  br label %for.cond2.preheader.i6920

for.cond2.preheader.i6920:                        ; preds = %for.cond.cleanup4.i6923, %for.cond2.preheader.lr.ph.i6919
  %indvars.iv35.i = phi i64 [ 0, %for.cond2.preheader.lr.ph.i6919 ], [ %indvars.iv.next36.i, %for.cond.cleanup4.i6923 ]
  %result.031.i = phi double [ 0.000000e+00, %for.cond2.preheader.lr.ph.i6919 ], [ %result.1.lcssa.i, %for.cond.cleanup4.i6923 ]
  br i1 %cmp327.i, label %for.body5.lr.ph.i6921, label %for.cond.cleanup4.i6923

for.body5.lr.ph.i6921:                            ; preds = %for.cond2.preheader.i6920
  %1 = mul nsw i64 %indvars.iv35.i, %0
  br label %for.body5.i6930

for.cond.cleanup4.i6923.loopexit:                 ; preds = %for.body5.i6930
  %add.i6927.lcssa = phi double [ %add.i6927, %for.body5.i6930 ]
  br label %for.cond.cleanup4.i6923

for.cond.cleanup4.i6923:                          ; preds = %for.cond.cleanup4.i6923.loopexit, %for.cond2.preheader.i6920
  %result.1.lcssa.i = phi double [ %result.031.i, %for.cond2.preheader.i6920 ], [ %add.i6927.lcssa, %for.cond.cleanup4.i6923.loopexit ]
  %indvars.iv.next36.i = add nuw nsw i64 %indvars.iv35.i, 1
  %exitcond39.not.i = icmp eq i64 %indvars.iv.next36.i, %wide.trip.count38.i
  br i1 %exitcond39.not.i, label %for.cond2.preheader.lr.ph.i6939, label %for.cond2.preheader.i6920

for.body5.i6930:                                  ; preds = %for.body5.i6930, %for.body5.lr.ph.i6921
  %indvars.iv.i6924 = phi i64 [ 0, %for.body5.lr.ph.i6921 ], [ %indvars.iv.next.i6928, %for.body5.i6930 ]
  %result.128.i = phi double [ %result.031.i, %for.body5.lr.ph.i6921 ], [ %add.i6927, %for.body5.i6930 ]
  %2 = add nsw i64 %indvars.iv.i6924, %1
  %arrayidx.i.i6925 = getelementptr inbounds double, ptr %arr, i64 %2
  %3 = load double, ptr %arrayidx.i.i6925, align 8
  %mul.i6926 = fmul fast double %3, %3
  %add.i6927 = fadd fast double %mul.i6926, %result.128.i
  %indvars.iv.next.i6928 = add nuw nsw i64 %indvars.iv.i6924, 1
  %exitcond.not.i6929 = icmp eq i64 %indvars.iv.next.i6928, %wide.trip.count.i6918
  br i1 %exitcond.not.i6929, label %for.cond.cleanup4.i6923.loopexit, label %for.body5.i6930

for.cond2.preheader.lr.ph.i6939:                  ; preds = %for.cond.cleanup4.i6923
  ret void
}

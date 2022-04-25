; RUN: opt < %s -analyze -enable-new-pm=0 -hir-ssa-deconstruction -hir-framework 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

; Verify that the test passes verification successfully.
; It was failing because we updated the def level of deferred ztt candidate
; 'if (%m > 0)' in anticipation of it being successfuly recognized.

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK: |   %m = (%m_ptr)[0];
; CHECK: |
; CHECK: |   + DO i2 = 0, zext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK: |   |   %0 = (%A)[i2];
; CHECK: |   |   (%A)[i2] = i2 + %0;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |
; CHECK: |   + DO i2 = 0, zext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK: |   |   %2 = (%A)[i2];
; CHECK: |   |   (%A)[i2] = i1 + %2;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


define dso_local void @foo(i32 %n, i32* %m_ptr, i32* nocapture %A) {
entry:
  %cmp31 = icmp sgt i32 %n, 0
  br i1 %cmp31, label %for.cond1.preheader.lr.ph, label %for.end15

for.cond1.preheader.lr.ph:                        ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc13, %for.cond1.preheader.lr.ph
  %i.032 = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %inc14, %for.inc13 ]
  %m = load i32, i32* %m_ptr
  %wide.trip.count39 = zext i32 %m to i64
  %wide.trip.count3640 = zext i32 %m to i64
  %cmp227 = icmp sgt i32 %m, 0
  br i1 %cmp227, label %for.body3.preheader, label %unused

for.body3.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body3

for.cond4.preheader:                              ; preds = %for.body3
  br label %for.body6.preheader

for.body6.preheader:                              ; preds = %for.cond4.preheader
  br label %for.body6

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body3 ], [ 0, %for.body3.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %1 = trunc i64 %indvars.iv to i32
  %add = add nsw i32 %0, %1
  store i32 %add, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count39
  br i1 %exitcond, label %for.cond4.preheader, label %for.body3

for.body6:                                        ; preds = %for.body6.preheader, %for.body6
  %indvars.iv34 = phi i64 [ %indvars.iv.next35, %for.body6 ], [ 0, %for.body6.preheader ]
  %arrayidx8 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv34
  %2 = load i32, i32* %arrayidx8, align 4
  %add9 = add nsw i32 %2, %i.032
  store i32 %add9, i32* %arrayidx8, align 4
  %indvars.iv.next35 = add nuw nsw i64 %indvars.iv34, 1
  %exitcond37 = icmp eq i64 %indvars.iv.next35, %wide.trip.count3640
  br i1 %exitcond37, label %for.inc13.loopexit, label %for.body6

for.inc13.loopexit:                               ; preds = %for.body6
  br label %for.inc13

unused:
  %zext = zext i32 %i.032 to i64
  br label %for.inc13

for.inc13:                                        ; preds = %for.inc13.loopexit, %for.cond1.preheader, %for.cond4.preheader
  %inc14 = add nuw nsw i32 %i.032, 1
  %exitcond38 = icmp eq i32 %inc14, %n
  br i1 %exitcond38, label %for.end15.loopexit, label %for.cond1.preheader

for.end15.loopexit:                               ; preds = %for.inc13
  br label %for.end15

for.end15:                                        ; preds = %for.end15.loopexit, %entry
  ret void
}


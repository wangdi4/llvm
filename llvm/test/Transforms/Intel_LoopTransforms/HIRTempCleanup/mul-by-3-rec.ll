; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -print-before=hir-temp-cleanup -print-after=hir-temp-cleanup 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>,hir-temp-cleanup,print<hir-framework>" -disable-output 2>&1 | FileCheck %s

; Verify that we are able to get rid of copy of %t.026 and are using only a single mul recurrence blob in the innermost loop.

; CHECK: Function

; CHECK: |   %t.026.out = %t.026;

; CHECK: Function

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, sext.i32.i64(%n) + -2, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK: |   |   %0 = (%A)[i1 + 1];
; CHECK: |   |   (%A)[i1 + 1] = %0 + %t.026;
; CHECK: |   |   %1 = (%B)[i1 + 1];
; CHECK: |   |   (%B)[i1 + 1] = %1 + 3 * %t.026;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %t.026 = 3 * %t.026;
; CHECK: + END LOOP
; CHECK: END REGION


define void @foo(i32 %n, i32* nocapture %A, i32* nocapture %B) {
entry:
  %cmp24 = icmp sgt i32 %n, 1
  br i1 %cmp24, label %for.body4.lr.ph.preheader, label %for.end10

for.body4.lr.ph.preheader:                        ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body4.lr.ph

for.body4.lr.ph:                                  ; preds = %for.body4.lr.ph.preheader, %for.inc8
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc8 ], [ 1, %for.body4.lr.ph.preheader ]
  %t.026 = phi i32 [ %mul, %for.inc8 ], [ 1, %for.body4.lr.ph.preheader ]
  %mul = mul nsw i32 %t.026, 3
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %arrayidx6 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  br label %for.body4

for.body4:                                        ; preds = %for.body4, %for.body4.lr.ph
  %j.023 = phi i32 [ 0, %for.body4.lr.ph ], [ %inc, %for.body4 ]
  %0 = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %0, %t.026
  store i32 %add, i32* %arrayidx, align 4
  %1 = load i32, i32* %arrayidx6, align 4
  %add7 = add nsw i32 %1, %mul
  store i32 %add7, i32* %arrayidx6, align 4
  %inc = add nuw nsw i32 %j.023, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.inc8, label %for.body4

for.inc8:                                         ; preds = %for.body4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond27 = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond27, label %for.end10.loopexit, label %for.body4.lr.ph

for.end10.loopexit:                               ; preds = %for.inc8
  br label %for.end10

for.end10:                                        ; preds = %for.end10.loopexit, %entry
  ret void
}


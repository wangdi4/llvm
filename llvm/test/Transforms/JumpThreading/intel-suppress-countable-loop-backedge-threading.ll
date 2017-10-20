; RUN: opt < %s -jump-threading -S | FileCheck %s

; Verify that jump threading is suppressed for countable loop backedges prior to loopopt("pre_loopopt").
; CHECK: for.body6:
; CHECK-SAME: preds = %for.inc, %entry
; CHECK-NOT: br label %for.body6
; CHECK: br i1 %exitcond, label %for.inc45.loopexit165, label %for.body6

define void @LBM_initializeSpecialCellsForLDC(double* nocapture %grid, i64 %t, i1 %cmp11, i64 %t10, i1 %t13) "pre_loopopt" {
entry:
  %cmp10 = icmp eq i64 %t, 0
  %or.cond55 = or i1 %cmp10, %cmp11
  br label %for.body6

for.body6:                                        ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %cmp7 = icmp eq i64 %indvars.iv, 0
  %cmp8 = icmp eq i64 %indvars.iv, 99
  %or.cond = or i1 %cmp7, %cmp8
  %or.cond51 = or i1 %cmp10, %or.cond
  br i1 %or.cond51, label %for.inc.sink.split, label %if.else

if.else:                                          ; preds = %for.body6
  %t24 = add i64 %indvars.iv, 4294967294
  %t25 = trunc i64 %t24 to i32
  %t26 = icmp ult i32 %t25, 96
  %t27 = and i1 %or.cond55, %t26
  %t28 = and i1 %t13, %t27
  br i1 %t28, label %for.inc.sink.split, label %for.inc

for.inc.sink.split:                               ; preds = %for.body6, %if.else
  %.sink88 = phi i32 [ 2, %if.else ], [ 1, %for.body6 ]
  %t29 = add nsw i64 %t10, %indvars.iv
  %t30 = mul nsw i64 %t29, 20
  %t31 = add nsw i64 %t30, 19
  %arrayidx42 = getelementptr inbounds double, double* %grid, i64 %t31
  %t32 = bitcast double* %arrayidx42 to i32*
  %t33 = load i32, i32* %t32, align 4
  %or = or i32 %t33, %.sink88
  store i32 %or, i32* %t32, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.inc.sink.split, %if.else
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.inc45.loopexit165, label %for.body6

for.inc45.loopexit165:                            ; preds = %for.inc
  ret void
}

; Verify that jump threading kicks in after loopopt.
; CHECK: for.body6:
; CHECK-SAME: preds = %for.inc.thread, %for.inc, %entry
; CHECK: br label %for.body6
; CHECK: br i1 %exitcond, label %for.inc45.loopexit165, label %for.body6

define void @LBM_initializeSpecialCellsForLDC1(double* nocapture %grid, i64 %t, i1 %cmp11, i64 %t10, i1 %t13) {
entry:
  %cmp10 = icmp eq i64 %t, 0
  %or.cond55 = or i1 %cmp10, %cmp11
  br label %for.body6

for.body6:                                        ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %cmp7 = icmp eq i64 %indvars.iv, 0
  %cmp8 = icmp eq i64 %indvars.iv, 99
  %or.cond = or i1 %cmp7, %cmp8
  %or.cond51 = or i1 %cmp10, %or.cond
  br i1 %or.cond51, label %for.inc.sink.split, label %if.else

if.else:                                          ; preds = %for.body6
  %t24 = add i64 %indvars.iv, 4294967294
  %t25 = trunc i64 %t24 to i32
  %t26 = icmp ult i32 %t25, 96
  %t27 = and i1 %or.cond55, %t26
  %t28 = and i1 %t13, %t27
  br i1 %t28, label %for.inc.sink.split, label %for.inc

for.inc.sink.split:                               ; preds = %for.body6, %if.else
  %.sink88 = phi i32 [ 2, %if.else ], [ 1, %for.body6 ]
  %t29 = add nsw i64 %t10, %indvars.iv
  %t30 = mul nsw i64 %t29, 20
  %t31 = add nsw i64 %t30, 19
  %arrayidx42 = getelementptr inbounds double, double* %grid, i64 %t31
  %t32 = bitcast double* %arrayidx42 to i32*
  %t33 = load i32, i32* %t32, align 4
  %or = or i32 %t33, %.sink88
  store i32 %or, i32* %t32, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.inc.sink.split, %if.else
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.inc45.loopexit165, label %for.body6

for.inc45.loopexit165:                            ; preds = %for.inc
  ret void
}

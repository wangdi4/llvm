; RUN: opt -passes="jump-threading" < %s -S | FileCheck %s

; Verify that jump threading is suppressed for countable loop headers with
; switch terminators prior to loopopt("pre_loopopt").
; CHECK: for.body6:
; CHECK-SAME: preds = %for.inc, %for.cond4.preheader

define void @LBM_initializeSpecialCellsForChannel(double* nocapture %grid) "pre_loopopt" {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc46, %entry
  %indvars.iv115 = phi i64 [ -2, %entry ], [ %indvars.iv.next116, %for.inc46 ]
  %0 = mul nsw i64 %indvars.iv115, 10000
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc43, %for.cond1.preheader
  %indvars.iv110 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next111, %for.inc43 ]
  %y.090 = phi i32 [ 0, %for.cond1.preheader ], [ %inc44, %for.inc43 ]
  %1 = mul nuw nsw i64 %indvars.iv110, 100
  %2 = add nsw i64 %1, %0
  %trunc = trunc i32 %y.090 to i31
  switch i31 %trunc, label %for.body6 [
    i31 99, label %for.body6.us
    i31 0, label %for.body6.us78
  ]

for.body6.us:                                     ; preds = %for.body6.us, %for.cond4.preheader
  %indvars.iv98 = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next99, %for.body6.us ]
  %3 = add nsw i64 %2, %indvars.iv98
  %4 = mul nsw i64 %3, 20
  %5 = add nsw i64 %4, 19
  %arrayidx.us = getelementptr inbounds double, double* %grid, i64 %5
  %6 = bitcast double* %arrayidx.us to i32*
  %7 = load i32, i32* %6, align 4
  %or.us = or i32 %7, 1
  store i32 %or.us, i32* %6, align 4
  %indvars.iv.next99 = add nuw nsw i64 %indvars.iv98, 1
  %exitcond103 = icmp eq i64 %indvars.iv.next99, 100
  br i1 %exitcond103, label %for.inc43, label %for.body6.us

for.body6.us78:                                   ; preds = %for.body6.us78, %for.cond4.preheader
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6.us78 ]
  %8 = add nsw i64 %2, %indvars.iv
  %9 = mul nsw i64 %8, 20
  %10 = add nsw i64 %9, 19
  %arrayidx.us85 = getelementptr inbounds double, double* %grid, i64 %10
  %11 = bitcast double* %arrayidx.us85 to i32*
  %12 = load i32, i32* %11, align 4
  %or.us86 = or i32 %12, 1
  store i32 %or.us86, i32* %11, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.inc43, label %for.body6.us78

for.body6:                                        ; preds = %for.inc, %for.cond4.preheader
  %indvars.iv104 = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next105, %for.inc ]
  %x.077 = phi i32 [ 0, %for.cond4.preheader ], [ %inc, %for.inc ]
  %trunc92 = trunc i32 %x.077 to i31
  switch i31 %trunc92, label %for.inc [
    i31 99, label %if.then
    i31 0, label %if.then
  ]

if.then:                                          ; preds = %for.body6, %for.body6
  %13 = add nsw i64 %2, %indvars.iv104
  %14 = mul nsw i64 %13, 20
  %15 = add nsw i64 %14, 19
  %arrayidx = getelementptr inbounds double, double* %grid, i64 %15
  %16 = bitcast double* %arrayidx to i32*
  %17 = load i32, i32* %16, align 4
  %or = or i32 %17, 1
  store i32 %or, i32* %16, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body6, %if.then
  %indvars.iv.next105 = add nuw nsw i64 %indvars.iv104, 1
  %inc = add nuw nsw i32 %x.077, 1
  %exitcond109 = icmp eq i64 %indvars.iv.next105, 100
  br i1 %exitcond109, label %for.inc43, label %for.body6

for.inc43:                                        ; preds = %for.body6.us78, %for.body6.us, %for.inc
  %indvars.iv.next111 = add nuw nsw i64 %indvars.iv110, 1
  %inc44 = add nuw nsw i32 %y.090, 1
  %exitcond114 = icmp eq i64 %indvars.iv.next111, 100
  br i1 %exitcond114, label %for.inc46, label %for.cond4.preheader

for.inc46:                                        ; preds = %for.inc43
  %indvars.iv.next116 = add nsw i64 %indvars.iv115, 1
  %exitcond118 = icmp eq i64 %indvars.iv.next116, 132
  br i1 %exitcond118, label %for.end48, label %for.cond1.preheader

for.end48:                                        ; preds = %for.inc46
  ret void
}

; Verify that jump threading kicks in after loopopt.
; CHECK: for.body6:
; CHECK-SAME: preds = %for.inc{{[[:space:]]}}

define void @LBM_initializeSpecialCellsForChannel1(double* nocapture %grid) {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc46, %entry
  %indvars.iv115 = phi i64 [ -2, %entry ], [ %indvars.iv.next116, %for.inc46 ]
  %0 = mul nsw i64 %indvars.iv115, 10000
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc43, %for.cond1.preheader
  %indvars.iv110 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next111, %for.inc43 ]
  %y.090 = phi i32 [ 0, %for.cond1.preheader ], [ %inc44, %for.inc43 ]
  %1 = mul nuw nsw i64 %indvars.iv110, 100
  %2 = add nsw i64 %1, %0
  %trunc = trunc i32 %y.090 to i31
  switch i31 %trunc, label %for.body6 [
    i31 99, label %for.body6.us
    i31 0, label %for.body6.us78
  ]

for.body6.us:                                     ; preds = %for.body6.us, %for.cond4.preheader
  %indvars.iv98 = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next99, %for.body6.us ]
  %3 = add nsw i64 %2, %indvars.iv98
  %4 = mul nsw i64 %3, 20
  %5 = add nsw i64 %4, 19
  %arrayidx.us = getelementptr inbounds double, double* %grid, i64 %5
  %6 = bitcast double* %arrayidx.us to i32*
  %7 = load i32, i32* %6, align 4
  %or.us = or i32 %7, 1
  store i32 %or.us, i32* %6, align 4
  %indvars.iv.next99 = add nuw nsw i64 %indvars.iv98, 1
  %exitcond103 = icmp eq i64 %indvars.iv.next99, 100
  br i1 %exitcond103, label %for.inc43, label %for.body6.us

for.body6.us78:                                   ; preds = %for.body6.us78, %for.cond4.preheader
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6.us78 ]
  %8 = add nsw i64 %2, %indvars.iv
  %9 = mul nsw i64 %8, 20
  %10 = add nsw i64 %9, 19
  %arrayidx.us85 = getelementptr inbounds double, double* %grid, i64 %10
  %11 = bitcast double* %arrayidx.us85 to i32*
  %12 = load i32, i32* %11, align 4
  %or.us86 = or i32 %12, 1
  store i32 %or.us86, i32* %11, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.inc43, label %for.body6.us78

for.body6:                                        ; preds = %for.inc, %for.cond4.preheader
  %indvars.iv104 = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next105, %for.inc ]
  %x.077 = phi i32 [ 0, %for.cond4.preheader ], [ %inc, %for.inc ]
  %trunc92 = trunc i32 %x.077 to i31
  switch i31 %trunc92, label %for.inc [
    i31 99, label %if.then
    i31 0, label %if.then
  ]

if.then:                                          ; preds = %for.body6, %for.body6
  %13 = add nsw i64 %2, %indvars.iv104
  %14 = mul nsw i64 %13, 20
  %15 = add nsw i64 %14, 19
  %arrayidx = getelementptr inbounds double, double* %grid, i64 %15
  %16 = bitcast double* %arrayidx to i32*
  %17 = load i32, i32* %16, align 4
  %or = or i32 %17, 1
  store i32 %or, i32* %16, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body6, %if.then
  %indvars.iv.next105 = add nuw nsw i64 %indvars.iv104, 1
  %inc = add nuw nsw i32 %x.077, 1
  %exitcond109 = icmp eq i64 %indvars.iv.next105, 100
  br i1 %exitcond109, label %for.inc43, label %for.body6

for.inc43:                                        ; preds = %for.body6.us78, %for.body6.us, %for.inc
  %indvars.iv.next111 = add nuw nsw i64 %indvars.iv110, 1
  %inc44 = add nuw nsw i32 %y.090, 1
  %exitcond114 = icmp eq i64 %indvars.iv.next111, 100
  br i1 %exitcond114, label %for.inc46, label %for.cond4.preheader

for.inc46:                                        ; preds = %for.inc43
  %indvars.iv.next116 = add nsw i64 %indvars.iv115, 1
  %exitcond118 = icmp eq i64 %indvars.iv.next116, 132
  br i1 %exitcond118, label %for.end48, label %for.cond1.preheader

for.end48:                                        ; preds = %for.inc46
  ret void
}

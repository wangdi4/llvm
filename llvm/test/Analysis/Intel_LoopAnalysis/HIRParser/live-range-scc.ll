; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that live range issue for the scc (%sub13 -> %add -> %sub13152 -> %dec5 -> %sub13153) is handled correctly by creating liveout copies. The issue here was that the live-range of %add and %sub13 in the scc overlaps.

; CHECK: + DO i1 = 0, 56, 1   <DO_LOOP>
; CHECK: |   %sub13153.out1 = %sub13153;
; CHECK: |   %sub13153 = %sub13153  +  -1;
; CHECK: |   if (-1 * i1 + %in <u %sub13153.out1)
; CHECK: |   {
; CHECK: |      %.pre-phi = i1 + 1;
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      %1 = (%sb)[0][i1 + 2];
; CHECK: |      %sub13153 = %1  +  %sub13153;
; CHECK: |      %sub13153.out = %sub13153;
; CHECK: |      %3 = (%sb)[0][i1 + 1];
; CHECK: |      %4 = (%b)[0];
; CHECK: |      (%b)[0] = -1 * %3 + %4;
; CHECK: |      %sub13153 = %sub13153  -  %3;
; CHECK: |      %.pre-phi = i1 + 1;
; CHECK: |      if (84 * %sub13153 != %1)
; CHECK: |      {
; CHECK: |         (%sb)[0][i1 + 1] = %sub13153.out;
; CHECK: |         %.pre-phi = i1 + 1;
; CHECK: |      }
; CHECK: |   }
; CHECK: |   %5 = (%vw)[0][%.pre-phi];
; CHECK: |   %6 = (%j)[0][i1 + 3];
; CHECK: |   (%j)[0][i1 + 3] = (%6 * %5);
; CHECK: |   + DO i2 = 0, i1, 1   <DO_LOOP>
; CHECK: |   |   %7 = (%qu5)[0][i1 + 2][i2 + 1];
; CHECK: |   |   %9 = (%qu5)[0][i2][%.pre-phi];
; CHECK: |   |   (%qu5)[0][i2][%.pre-phi] = (%9 * %7);
; CHECK: |   + END LOOP
; CHECK: + END LOOP


define i32 @foo(i32 %ng.promoted, i32 %in) {
entry:
  %b = alloca i32, align 4
  %j = alloca [100 x i32], align 16
  %sb = alloca [100 x i32], align 16
  %qu5 = alloca [100 x [100 x i32]], align 16
  %vw = alloca [100 x i32], align 16
  br label %for.body

for.body:                                         ; preds = %for.inc44, %entry
  %indvars.iv161 = phi i64 [ 2, %entry ], [ %indvars.iv.next162, %for.inc44 ]
  %indvars.iv158 = phi i32 [ 2, %entry ], [ %indvars.iv.next159, %for.inc44 ]
  %sub13153 = phi i32 [ %ng.promoted, %entry ], [ %sub13152, %for.inc44 ]
  %0 = phi i32 [ %in, %entry ], [ %dec, %for.inc44 ]
  %dec = add i32 %0, -1
  %dec5 = add i32 %sub13153, -1
  %cmp6 = icmp ult i32 %0, %sub13153
  br i1 %cmp6, label %for.body.if.end22_crit_edge, label %if.then

for.body.if.end22_crit_edge:                      ; preds = %for.body
  %.pre = add nsw i64 %indvars.iv161, -1
  br label %for.body32.preheader

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* %sb, i64 0, i64 %indvars.iv161
  %1 = load i32, i32* %arrayidx, align 4 
  %add = add i32 %1, %dec5
  %2 = add nsw i64 %indvars.iv161, -1
  %arrayidx8 = getelementptr inbounds [100 x i32], [100 x i32]* %sb, i64 0, i64 %2
  %3 = load i32, i32* %arrayidx8, align 4 
  %4 = load i32, i32* %b, align 4 
  %sub9 = sub i32 %4, %3
  store i32 %sub9, i32* %b, align 4 
  %sub13 = sub i32 %add, %3
  %mul = mul i32 %sub13, 84
  %cmp16 = icmp eq i32 %mul, %1
  br i1 %cmp16, label %for.body32.preheader, label %if.then17

if.then17:                                        ; preds = %if.then
  store i32 %add, i32* %arrayidx8, align 4 
  br label %for.body32.preheader

for.body32.preheader:                             ; preds = %if.then17, %if.then, %for.body.if.end22_crit_edge
  %.pre-phi = phi i64 [ %.pre, %for.body.if.end22_crit_edge ], [ %2, %if.then ], [ %2, %if.then17 ]
  %sub13152 = phi i32 [ %dec5, %for.body.if.end22_crit_edge ], [ %sub13, %if.then ], [ %sub13, %if.then17 ]
  %arrayidx25 = getelementptr inbounds [100 x i32], [100 x i32]* %vw, i64 0, i64 %.pre-phi
  %5 = load i32, i32* %arrayidx25, align 4 
  %indvars.iv.next162 = add nuw nsw i64 %indvars.iv161, 1
  %arrayidx28 = getelementptr inbounds [100 x i32], [100 x i32]* %j, i64 0, i64 %indvars.iv.next162
  %6 = load i32, i32* %arrayidx28, align 4 
  %mul29 = mul i32 %6, %5
  store i32 %mul29, i32* %arrayidx28, align 4 
  br label %for.body32

for.body32:                                       ; preds = %for.body32, %for.body32.preheader
  %indvars.iv = phi i64 [ 1, %for.body32.preheader ], [ %indvars.iv.next, %for.body32 ]
  %arrayidx36 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %qu5, i64 0, i64 %indvars.iv161, i64 %indvars.iv
  %7 = load i32, i32* %arrayidx36, align 4 
  %8 = add nsw i64 %indvars.iv, -1
  %arrayidx42 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %qu5, i64 0, i64 %8, i64 %.pre-phi
  %9 = load i32, i32* %arrayidx42, align 4 
  %mul43 = mul i32 %9, %7
  store i32 %mul43, i32* %arrayidx42, align 4 
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond160 = icmp eq i32 %lftr.wideiv, %indvars.iv158
  br i1 %exitcond160, label %for.inc44, label %for.body32

for.inc44:                                        ; preds = %for.body32
  %indvars.iv.next159 = add nuw nsw i32 %indvars.iv158, 1
  %exitcond167 = icmp eq i64 %indvars.iv.next162, 59
  br i1 %exitcond167, label %for.end46, label %for.body

for.end46:
  ret i32 0
}

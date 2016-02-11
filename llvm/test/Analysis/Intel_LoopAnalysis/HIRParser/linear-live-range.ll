; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that there are no live range issues with the outer loop IV.

; Source code-
;  for (k = 58; k > 1; k--) {
;    for (i = k; i < 5; ++i) {
;      ti[i][i] = ti[k][k-1];
;      pg5 = &a[i];
;      fs += (n[i]) + i;
;    }
;    for (k7 = k; k7 < 5; k7++) {
;      ti[k-1][k-1] += ep[k7][k7-1];
;      a[k7] = n1--;
;      s7[k7-1] -= x[k7];
;    }
;  }

; CHECK: DO i1 = 0, 56, 1
; CHECK: %fs.promoted = (%fs)[0]
; CHECK-NEXT: %add21136 = %fs.promoted
; CHECK-NEXT: DO i2 = 0, zext.i32.i64((4 + (-1 * trunc.i64.i32(%indvars.iv169))))
; CHECK-NEXT: %5 = (%ti)[0][-1 * i1 + 58][-1 * i1 + 57]
; CHECK-NEXT: (%ti)[0][-1 * i1 + i2 + 58][-1 * i1 + i2 + 58] = %5
; CHECK-NEXT: %6 = (%n)[0][-1 * i1 + i2 + 58]
; CHECK-NEXT: %add21136 = -1 * i1 + i2 + %6 + 58  +  %add21136
; CHECK-NEXT: END LOOP
; CHECK-NEXT: (%fs)[0] = %add21136
; CHECK: %arrayidx35.promoted = (%ti)[0][-1 * i1 + 57][-1 * i1 + 57]
; CHECK-NEXT: %n1.promoted = (%n1)[0]
; CHECK-NEXT: %8 = %arrayidx35.promoted
; CHECK-NEXT: DO i2 = 0, zext.i32.i64((4 + (-1 * trunc.i64.i32(%indvars.iv169))))
; CHECK-NEXT: %10 = (%ep)[0][-1 * i1 + i2 + 58][-1 * i1 + i2 + 57]
; CHECK-NEXT: %8 = %8  +  %10
; CHECK-NEXT: (%a)[0][-1 * i1 + i2 + 58] = -1 * i2 + %n1.promoted
; CHECK-NEXT: %11 = (%x)[0][-1 * i1 + i2 + 58]
; CHECK-NEXT: %12 = (%s7)[0][-1 * i1 + i2 + 57]
; CHECK-NEXT: (%s7)[0][-1 * i1 + i2 + 57] = -1 * %11 + %12
; CHECK-NEXT: END LOOP
; CHECK-NEXT: (%ti)[0][-1 * i1 + 57][-1 * i1 + 57] = %8
; CHECK-NEXT: (%n1)[0] = -1 * i1 + %n1.promoted + 53
; CHECK: %indvars.iv169 = -1 * i1 + 57
; CHECK-NEXT: END LOOP


; ModuleID = 't47.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @main() {
entry:
  %k = alloca i32, align 4
  %i = alloca i32, align 4
  %fs = alloca i32, align 4
  %k7 = alloca i32, align 4
  %n1 = alloca i32, align 4
  %ti = alloca [100 x [100 x i32]], align 16
  %a = alloca [100 x i32], align 16
  %n = alloca [100 x i32], align 16
  %ep = alloca [100 x [100 x i32]], align 16
  %s7 = alloca [100 x i32], align 16
  %x = alloca [100 x i32], align 16
  store i32 69, i32* %k, align 4
  store i32 74, i32* %i, align 4
  store i32 2, i32* %fs, align 4
  store i32 30, i32* %k7, align 4
  store i32 74, i32* %n1, align 4
  br label %for.cond.7.preheader

for.cond.7.preheader:                             ; preds = %for.inc.48, %entry
  %indvars.iv169 = phi i64 [ 58, %entry ], [ %4, %for.inc.48 ]
  %indvars.iv165 = phi i32 [ 53, %entry ], [ %indvars.iv.next166, %for.inc.48 ]
  %indvars.iv163 = phi i32 [ -54, %entry ], [ %indvars.iv.next164, %for.inc.48 ]
  %0 = phi i32 [ 58, %entry ], [ %dec49, %for.inc.48 ]
  %pg5.0134 = phi i32* [ %n1, %entry ], [ %pg5.1.lcssa, %for.inc.48 ]
  %1 = zext i32 %indvars.iv163 to i64
  %2 = add i64 %indvars.iv169, %1
  %cmp8.128 = icmp ult i32 %0, 5
  br i1 %cmp8.128, label %for.body.9.lr.ph, label %for.cond.22.preheader

for.body.9.lr.ph:                                 ; preds = %for.cond.7.preheader
  %fs.promoted = load i32, i32* %fs, align 4
  %3 = add nsw i64 %indvars.iv169, -1
  %arrayidx12 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %ti, i64 0, i64 %indvars.iv169, i64 %3
  br label %for.body.9

for.cond.7.for.cond.22.preheader_crit_edge:       ; preds = %for.body.9
  %add21.lcssa = phi i32 [ %add21, %for.body.9 ]
  store i32 %add21.lcssa, i32* %fs, align 4
  %arrayidx18.le = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %2
  br label %for.cond.22.preheader

for.cond.22.preheader:                            ; preds = %for.cond.7.for.cond.22.preheader_crit_edge, %for.cond.7.preheader
  %pg5.1.lcssa = phi i32* [ %arrayidx18.le, %for.cond.7.for.cond.22.preheader_crit_edge ], [ %pg5.0134, %for.cond.7.preheader ]
  %cmp23.131 = icmp ult i64 %indvars.iv169, 5
  %4 = add nsw i64 %indvars.iv169, -1
  br i1 %cmp23.131, label %for.body.24.lr.ph, label %for.inc.48

for.body.24.lr.ph:                                ; preds = %for.cond.22.preheader
  %arrayidx35 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %ti, i64 0, i64 %4, i64 %4
  %arrayidx35.promoted = load i32, i32* %arrayidx35, align 4
  %n1.promoted = load i32, i32* %n1, align 4
  br label %for.body.24

for.body.9:                                       ; preds = %for.body.9, %for.body.9.lr.ph
  %indvars.iv148 = phi i64 [ %indvars.iv169, %for.body.9.lr.ph ], [ %indvars.iv.next149, %for.body.9 ]
  %add21136 = phi i32 [ %fs.promoted, %for.body.9.lr.ph ], [ %add21, %for.body.9 ]
  %5 = load i32, i32* %arrayidx12, align 4
  %arrayidx16 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %ti, i64 0, i64 %indvars.iv148, i64 %indvars.iv148
  store i32 %5, i32* %arrayidx16, align 4
  %arrayidx20 = getelementptr inbounds [100 x i32], [100 x i32]* %n, i64 0, i64 %indvars.iv148
  %6 = load i32, i32* %arrayidx20, align 4
  %7 = trunc i64 %indvars.iv148 to i32
  %add = add i32 %6, %7
  %add21 = add i32 %add, %add21136
  %indvars.iv.next149 = add nuw nsw i64 %indvars.iv148, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next149 to i32
  %exitcond154 = icmp eq i32 %lftr.wideiv, 5
  br i1 %exitcond154, label %for.cond.7.for.cond.22.preheader_crit_edge, label %for.body.9

for.body.24:                                      ; preds = %for.body.24, %for.body.24.lr.ph
  %indvars.iv157 = phi i64 [ %indvars.iv169, %for.body.24.lr.ph ], [ %indvars.iv.next158, %for.body.24 ]
  %dec137 = phi i32 [ %n1.promoted, %for.body.24.lr.ph ], [ %dec, %for.body.24 ]
  %8 = phi i32 [ %arrayidx35.promoted, %for.body.24.lr.ph ], [ %add36, %for.body.24 ]
  %9 = add nsw i64 %indvars.iv157, -1
  %arrayidx29 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %ep, i64 0, i64 %indvars.iv157, i64 %9
  %10 = load i32, i32* %arrayidx29, align 4
  %add36 = add i32 %8, %10
  %dec = add i32 %dec137, -1
  %arrayidx38 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %indvars.iv157
  store i32 %dec137, i32* %arrayidx38, align 4
  %arrayidx40 = getelementptr inbounds [100 x i32], [100 x i32]* %x, i64 0, i64 %indvars.iv157
  %11 = load i32, i32* %arrayidx40, align 4
  %arrayidx43 = getelementptr inbounds [100 x i32], [100 x i32]* %s7, i64 0, i64 %9
  %12 = load i32, i32* %arrayidx43, align 4
  %sub44 = sub i32 %12, %11
  store i32 %sub44, i32* %arrayidx43, align 4
  %indvars.iv.next158 = add nuw nsw i64 %indvars.iv157, 1
  %lftr.wideiv167 = trunc i64 %indvars.iv.next158 to i32
  %exitcond168 = icmp eq i32 %lftr.wideiv167, 5
  br i1 %exitcond168, label %for.cond.22.for.inc.48_crit_edge, label %for.body.24

for.cond.22.for.inc.48_crit_edge:                 ; preds = %for.body.24
  %add36.lcssa = phi i32 [ %add36, %for.body.24 ]
  %13 = add i32 %n1.promoted, %indvars.iv165
  store i32 %add36.lcssa, i32* %arrayidx35, align 4
  store i32 %13, i32* %n1, align 4
  br label %for.inc.48

for.inc.48:                                       ; preds = %for.cond.22.preheader, %for.cond.22.for.inc.48_crit_edge
  %dec49 = add nsw i32 %0, -1
  %indvars.iv.next164 = add nsw i32 %indvars.iv163, 1
  %indvars.iv.next166 = add nsw i32 %indvars.iv165, -1
  %exitcond173 = icmp eq i32 %indvars.iv.next164, 3
  br i1 %exitcond173, label %for.end.50, label %for.cond.7.preheader

for.end.50:
  ret i32 0
}


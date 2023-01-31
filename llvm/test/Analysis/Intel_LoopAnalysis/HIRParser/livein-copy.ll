; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-details -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Verify that i3 loop has a ztt.
; CHECK: Ztt: if (i2 < i1)
; CHECK-NEXT: NumExits: 1
; CHECK-NEXT: Innermost: Yes
; CHECK-NEXT: HasSignedIV: Yes
; CHECK: DO i64 i3

; Check parsing output for the loop verifying that there is no inconsistency in parsing livein copies, i.e. lval and rval have identical canon expr.
; CHECK: %l.061 = %M + 5
; CHECK-NEXT: <LVAL-REG> LINEAR i64 %M + 5
; CHECK: <RVAL-REG> LINEAR i64 %M + 5

; CHECK: %l2.059 = %indvars.iv * i2
; CHECK-NEXT: <LVAL-REG> LINEAR i64 %indvars.iv * i2{def@1}
; CHECK: <RVAL-REG> LINEAR i64 %indvars.iv * i2{def@1}


; ModuleID = 'livein-copy1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(i32** nocapture readonly %A, i32** nocapture readonly %B, i64 %M) {
entry:
  %cmp.58 = icmp sgt i64 %M, 0
  br i1 %cmp.58, label %for.cond.1.preheader.lr.ph, label %for.cond.cleanup

for.cond.1.preheader.lr.ph:                       ; preds = %entry
  %add16.le = add nsw i64 %M, 5
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.cond.cleanup.3, %for.cond.1.preheader.lr.ph
  %indvars.iv = phi i64 [ 0, %for.cond.1.preheader.lr.ph ], [ %indvars.iv.next, %for.cond.cleanup.3 ]
  %l.061 = phi i64 [ undef, %for.cond.1.preheader.lr.ph ], [ %l.2.lcssa, %for.cond.cleanup.3 ]
  %l2.059 = phi i64 [ undef, %for.cond.1.preheader.lr.ph ], [ %l2.2.lcssa, %for.cond.cleanup.3 ]
  br label %for.cond.5.preheader

for.cond.5.preheader:                             ; preds = %for.cond.cleanup.7, %for.cond.1.preheader
  %i.056 = phi i64 [ 0, %for.cond.1.preheader ], [ %inc18, %for.cond.cleanup.7 ]
  %l.155 = phi i64 [ %l.061, %for.cond.1.preheader ], [ %l.2.lcssa, %for.cond.cleanup.7 ]
  %l2.154 = phi i64 [ %l2.059, %for.cond.1.preheader ], [ %l2.2.lcssa, %for.cond.cleanup.7 ]
  %cmp6.51 = icmp slt i64 %i.056, %indvars.iv
  br i1 %cmp6.51, label %for.body.8.lr.ph, label %for.cond.cleanup.7

for.body.8.lr.ph:                                 ; preds = %for.cond.5.preheader
  %arrayidx11 = getelementptr inbounds i32*, i32** %A, i64 %i.056
  %0 = load i32*, i32** %arrayidx11, align 8
  %1 = load i32*, i32** %B, align 8
  br label %for.body.8

for.body.8:                                       ; preds = %for.body.8, %for.body.8.lr.ph
  %j.052 = phi i64 [ %i.056, %for.body.8.lr.ph ], [ %inc, %for.body.8 ]
  %arrayidx = getelementptr inbounds i32*, i32** %B, i64 %j.052
  %2 = load i32*, i32** %arrayidx, align 8
  %arrayidx9 = getelementptr inbounds i32, i32* %2, i64 %i.056
  %3 = load i32, i32* %arrayidx9, align 4
  %add10 = add nsw i64 %j.052, %M
  %arrayidx12 = getelementptr inbounds i32, i32* %0, i64 %add10
  store i32 %3, i32* %arrayidx12, align 4
  %add13 = add nuw nsw i64 %j.052, %i.056
  %arrayidx15 = getelementptr inbounds i32, i32* %1, i64 %add13
  store i32 5, i32* %arrayidx15, align 4
  %inc = add nuw nsw i64 %j.052, 1
  %exitcond = icmp eq i64 %inc, %indvars.iv
  br i1 %exitcond, label %for.cond.5.for.cond.cleanup.7_crit_edge, label %for.body.8

for.cond.5.for.cond.cleanup.7_crit_edge:          ; preds = %for.body.8
  %mul.le = mul nsw i64 %i.056, %indvars.iv
  br label %for.cond.cleanup.7

for.cond.cleanup.7:                               ; preds = %for.cond.5.for.cond.cleanup.7_crit_edge, %for.cond.5.preheader
  %l.2.lcssa = phi i64 [ %add16.le, %for.cond.5.for.cond.cleanup.7_crit_edge ], [ %l.155, %for.cond.5.preheader ]
  %l2.2.lcssa = phi i64 [ %mul.le, %for.cond.5.for.cond.cleanup.7_crit_edge ], [ %l2.154, %for.cond.5.preheader ]
  %inc18 = add nuw nsw i64 %i.056, 1
  %exitcond64 = icmp eq i64 %inc18, 5
  br i1 %exitcond64, label %for.cond.cleanup.3, label %for.cond.5.preheader

for.cond.cleanup.3:                               ; preds = %for.cond.cleanup.7
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond65 = icmp eq i64 %indvars.iv.next, %M
  br i1 %exitcond65, label %for.cond.cleanup.loopexit, label %for.cond.1.preheader

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup.3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %l.0.lcssa = phi i64 [ undef, %entry ], [ %l.2.lcssa, %for.cond.cleanup.loopexit ]
  %l2.0.lcssa = phi i64 [ undef, %entry ], [ %l2.2.lcssa, %for.cond.cleanup.loopexit ]
  %add23 = add nsw i64 %l2.0.lcssa, 3
  %add24 = add nsw i64 %l.0.lcssa, 2
  %arrayidx25 = getelementptr inbounds i32*, i32** %A, i64 %add24
  %4 = load i32*, i32** %arrayidx25, align 8
  %arrayidx26 = getelementptr inbounds i32, i32* %4, i64 %add23
  %5 = load i32, i32* %arrayidx26, align 4
  ret i32 %5
}

; RUN: opt < %s -hir-ssa-deconstruction -hir-cg -force-hir-cg -S | FileCheck %s

; DO i1 = 0, 4, 1   <DO_LOOP>
;
;      %0 = (%A)[i1];
;      %1 = (%B)[0];
;   + DO i2 = 0, -1 * i1 + %indvars.iv + -1, 1   <DO_LOOP>
;   |   %2 = (%B)[i1 + i2];
;   |   %3 = (%2)[i1];
;   |   (%0)[i1 + i2 + %M] = %3;
;   |   (%1)[2 * i1 + i2] = 5;
;   + END LOOP
;      %l.155 = %M + 5;
;      %l2.154 = %indvars.iv * i1;
;
;   %l2.154.out = %l2.154;
;   %l.155.out = %l.155;
; END LOOP


; Check that the i2 loop with ztt, preheader and postexit is CG'd correctly.
; CHECK: region.0:
; Check outer loop begin
; CHECK: store i64 0, i64* %i1.i64
; CHECK: loop.{{[0-9]+}}:

; Check ztt
; CHECK: [[I1LOAD:%.*]] = load i64, i64* %i1.i64
; CHECK: [[ZTTCMP:%.*]] = icmp slt i64 [[I1LOAD]], [[INDVARS:%.*]]
; CHECK: br i1 [[ZTTCMP]], label %[[TRUEZTT:then.[0-9]]]

; Check preheader stmts
; CHECK: [[TRUEZTT]]:
; CHECK: [[I1LOAD1:%.*]] = load i64, i64* %i1.i64
; CHECK-NEXT: [[AGEP:%.*]] = getelementptr inbounds i32*, i32** %A, i64 [[I1LOAD1]]
; CHECK-NEXT: [[ALOAD:%.*]] = load i32*, i32** [[AGEP]]
; CHECK-NEXT: store i32* [[ALOAD]], i32** [[STORE0:%.*]]
; CHECK-NEXT: [[BLOAD:%.*]] = load i32*, i32** 
; CHECK-NEXT: store i32* [[BLOAD]], i32** [[STORE1:%.*]]
; CHECK: br label %[[I2LOOP:.*]]

; Check some stmts in i2 loop body
; CHECK: [[I2LOOP]]:
; CHECK: [[BGEP:%.*]] = getelementptr inbounds i32*, i32** %B
; CHECK-NEXT: load i32*, i32** [[BGEP]]
; CHECK: getelementptr inbounds i32, i32* [[STORE0]]
; CHECK: getelementptr inbounds i32, i32* [[STORE1]]
; CHECK: br i1 {{%.*}}, label %[[I2LOOP]], label %[[POSTEXIT:.*]]

; Check postexit stmts
; CHECK: [[POSTEXIT]]:
; CHECK: add i64 %M, 5
; CHECK: [[I1LOAD2:%.*]] = load i64, i64* %i1.i64
; CHECK: mul i64 [[INDVARS]]{{[0-9]+}}, [[I1LOAD2]]


; ModuleID = 'livein-copy1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
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
  br i1 %exitcond65, label %for.cond.cleanup, label %for.cond.1.preheader

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.3, %entry
  %l.0.lcssa = phi i64 [ undef, %entry ], [ %l.2.lcssa, %for.cond.cleanup.3 ]
  %l2.0.lcssa = phi i64 [ undef, %entry ], [ %l2.2.lcssa, %for.cond.cleanup.3 ]
  %add23 = add nsw i64 %l2.0.lcssa, 3
  %add24 = add nsw i64 %l.0.lcssa, 2
  %arrayidx25 = getelementptr inbounds i32*, i32** %A, i64 %add24
  %4 = load i32*, i32** %arrayidx25, align 8
  %arrayidx26 = getelementptr inbounds i32, i32* %4, i64 %add23
  %5 = load i32, i32* %arrayidx26, align 4
  ret i32 %5
}


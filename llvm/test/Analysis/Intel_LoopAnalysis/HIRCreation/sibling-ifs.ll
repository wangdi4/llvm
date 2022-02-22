; RUN: opt < %s -enable-new-pm=0 -analyze -hir-framework -hir-framework-debug=creation | FileCheck %s
; RUN: opt %s -passes="print<hir-framework>" -hir-framework-debug=creation -disable-output 2>&1 | FileCheck %s

; Verify that we do not throttle this loopnest due to presence of multiple sibling ifs.
; CHECK: BEGIN REGION
; CHECK: for.body:
; CHECK: if (0x0 true 0x0)
; CHECK: {
; CHECK: }
; CHECK: else
; CHECK: {
; CHECK: }
; CHECK: if (0x0 true 0x0)
; CHECK: {
; CHECK: }
; CHECK: else
; CHECK: {
; CHECK: }
; CHECK: if (0x0 true 0x0)
; CHECK: {
; CHECK: }
; CHECK: else
; CHECK: {
; CHECK: }
; CHECK: if (0x0 true 0x0)
; CHECK: {
; CHECK: goto for.end.loopexit;
; CHECK: }
; CHECK: else
; CHECK: {
; CHECK: goto for.body;
; CHECK: }
; CHECK: END REGION

; Check that we throttle loop when number of ifs is greater than threshold.
; RUN: opt < %s -enable-new-pm=0 -analyze -hir-framework -hir-loop-if-threshold=1 -hir-region-print-cost-model-stats -debug-only=hir-region-identification 2>&1 | FileCheck -check-prefix=COST-MODEL %s
; RUN: opt %s -passes="print<hir-framework>" -hir-loop-if-threshold=1 -hir-region-print-cost-model-stats -debug-only=hir-region-identification -disable-output 2>&1 | FileCheck -check-prefix=COST-MODEL %s
; COST-MODEL: Loop throttled due to presence of too many ifs.

; Verify that the second conditional branch condition %cmp2 which is invariant
; w.r.t to the loop is not counted in the stats.

; COST-MODEL: Loop if count: 2


; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %A, i32 %n, i32 %a) local_unnamed_addr #0 {
entry:
  %cmp21 = icmp sgt i32 %n, 0
  br i1 %cmp21, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %cmp2 = icmp eq i32 %a, 4
  br label %for.body

for.body:                                         ; preds = %for.inc, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.inc ]
  %arrayidxld = getelementptr inbounds i32, i32* %A, i64 0
  %ld = load i32, i32* %arrayidxld, align 4
  %cmp1 = icmp sgt i32 %ld, 0
  %cmp7 = icmp eq i32 %ld, 2
  br label %uncond.jump

uncond.jump:
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32* %arrayidx, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  br i1 %cmp2, label %if.end6, label %if.then3

if.then3:                                         ; preds = %if.end
  %arrayidx5 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx5, align 4
  %add = add nsw i32 %1, 2
  store i32 %add, i32* %arrayidx5, align 4
  br label %if.end6

if.end6:                                          ; preds = %if.end, %if.then3
  br i1 %cmp7, label %if.then8, label %for.inc

if.then8:                                         ; preds = %if.end6
  %arrayidx10 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx10, align 4
  %add11 = add nsw i32 %2, 3
  store i32 %add11, i32* %arrayidx10, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.end6, %if.then8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}


; REQUIRES: asserts

; RUN: opt < %s -analyze -hir-creation -hir-cost-model-throttling=0 | FileCheck %s

; Check that we create 3 nested ifs without throttling.
; CHECK: BEGIN REGION
; CHECK: if (0x0 true 0x0)
; CHECK: {
; CHECK: if (0x0 true 0x0)
; CHECK: {
; CHECK: if (0x0 true 0x0)
; CHECK: {
; CHECK: }
; CHECK: else
; CHECK: {
; CHECK: }
; CHECK: else
; CHECK: {
; CHECK: }
; CHECK: }
; CHECK: else
; CHECK: {
; CHECK: }
; CHECK: }
; CHECK: END REGION

; Check that we throttle nested ifs.
; RUN: opt < %s -analyze -hir-creation -debug-only=hir-region-identification 2>&1 | FileCheck -check-prefix=COST-MODEL %s
; COST-MODEL: Loop throttled due to presence of too many nested ifs


; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %A, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp33 = icmp sgt i32 %n, 0
  br i1 %cmp33, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp sgt i32 %0, 0
  br i1 %cmp1, label %if.then, label %if.else18

if.then:                                          ; preds = %for.body
  %cmp4 = icmp slt i32 %0, 6
  br i1 %cmp4, label %if.then5, label %if.else14

if.then5:                                         ; preds = %if.then
  %cmp8 = icmp eq i32 %0, 3
  br i1 %cmp8, label %if.else, label %if.then9

if.then9:                                         ; preds = %if.then5
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32* %arrayidx, align 4
  br label %for.inc

if.else:                                          ; preds = %if.then5
  store i32 2, i32* %arrayidx, align 4
  br label %for.inc

if.else14:                                        ; preds = %if.then
  %add = add nsw i32 %0, 2
  store i32 %add, i32* %arrayidx, align 4
  br label %for.inc

if.else18:                                        ; preds = %for.body
  %add21 = add nsw i32 %0, 3
  store i32 %add21, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.else18, %if.then9, %if.else, %if.else14
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}


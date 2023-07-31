; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-create-function-level-region < %s 2>&1 | FileCheck %s

; This case should not be fused: value in %a[i2] depends on the iteration of i1 loop.

; INPUT:
; BEGIN REGION { }
;
; + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; |   + DO i2 = 0, 99, 1   <DO_LOOP>
; |   |   (%a)[i2] = i1;
; |   + END LOOP
; + END LOOP
;
;
; + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; |   + DO i2 = 0, 99, 1   <DO_LOOP>
; |   |   %0 = (%a)[i2];
; |   |   (%b)[i2] = %0 + 1;
; |   + END LOOP
; + END LOOP
;
; ret ;
; END REGION

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   (%a)[i2] = i1;
; CHECK: |   + END LOOP
;
; CHECK: |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   %0 = (%a)[i2];
; CHECK: |   |   (%b)[i2] = %0 + 1;
; CHECK: |   + END LOOP
; CHECK: + END LOOP
; CHECK: END REGION

;Module Before HIR; ModuleID = 'enclosed-simple.c'
source_filename = "enclosed-simple.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr noalias nocapture %a, ptr noalias nocapture %b, ptr nocapture readnone %c, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp46 = icmp sgt i32 %n, 0
  br i1 %cmp46, label %for.body.lr.ph, label %for.cond.cleanup11

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  br i1 %cmp46, label %for.body12.lr.ph, label %for.cond.cleanup11

for.body12.lr.ph:                                 ; preds = %for.cond.cleanup
  br label %for.body12

for.body:                                         ; preds = %for.cond.cleanup3, %for.body.lr.ph
  %i.047 = phi i32 [ 0, %for.body.lr.ph ], [ %inc6, %for.cond.cleanup3 ]
  br label %for.body4

for.cond.cleanup3:                                ; preds = %for.body4
  %inc6 = add nuw nsw i32 %i.047, 1
  %exitcond52 = icmp eq i32 %inc6, %n
  br i1 %exitcond52, label %for.cond.cleanup, label %for.body

for.body4:                                        ; preds = %for.body4, %for.body
  %indvars.iv49 = phi i64 [ 0, %for.body ], [ %indvars.iv.next50, %for.body4 ]
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv49
  store i32 %i.047, ptr %arrayidx, align 4
  %indvars.iv.next50 = add nuw nsw i64 %indvars.iv49, 1
  %exitcond51 = icmp eq i64 %indvars.iv.next50, 100
  br i1 %exitcond51, label %for.cond.cleanup3, label %for.body4

for.cond.cleanup11.loopexit:                      ; preds = %for.cond.cleanup16
  br label %for.cond.cleanup11

for.cond.cleanup11:                               ; preds = %for.cond.cleanup11.loopexit, %entry, %for.cond.cleanup
  ret void

for.body12:                                       ; preds = %for.cond.cleanup16, %for.body12.lr.ph
  %i8.044 = phi i32 [ 0, %for.body12.lr.ph ], [ %inc26, %for.cond.cleanup16 ]
  br label %for.body17

for.cond.cleanup16:                               ; preds = %for.body17
  %inc26 = add nuw nsw i32 %i8.044, 1
  %exitcond48 = icmp eq i32 %inc26, %n
  br i1 %exitcond48, label %for.cond.cleanup11.loopexit, label %for.body12

for.body17:                                       ; preds = %for.body17, %for.body12
  %indvars.iv = phi i64 [ 0, %for.body12 ], [ %indvars.iv.next, %for.body17 ]
  %arrayidx19 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx19, align 4
  %add = add nsw i32 %0, 1
  %arrayidx21 = getelementptr inbounds i32, ptr %b, i64 %indvars.iv
  store i32 %add, ptr %arrayidx21, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup16, label %for.body17
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }



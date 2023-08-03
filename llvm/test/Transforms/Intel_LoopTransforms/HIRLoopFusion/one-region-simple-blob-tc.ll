; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-create-function-level-region < %s 2>&1 | FileCheck %s

; INPUT:
; BEGIN REGION { }
;    + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
;    |   (%a)[i1] = i1;
;    + END LOOP
;
;
;    + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
;    |   %1 = (%a)[i1];
;    |   (%b)[i1] = %1 + 1;
;    + END LOOP
;
;    ret ;
; END REGION

; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1 = 0, sext.i32.i64(%n) + -1, 1
; CHECK:    (%a)[i1] = i1;
; CHECK-NOT: + END LOOP
; CHECK:    %1 = (%a)[i1];
; CHECK:    (%b)[i1] = %1 + 1;
; CHECK: + END LOOP

; CHECK-NOT: + DO i1
; CHECK: END REGION


;Module Before HIR; ModuleID = 'enclosed-simple.c'
source_filename = "enclosed-simple.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr noalias nocapture %a, ptr noalias nocapture %b, ptr nocapture readnone %c, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp25 = icmp sgt i32 %n, 0
  br i1 %cmp25, label %for.body.lr.ph, label %for.cond.cleanup4

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count29 = sext i32 %n to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  br i1 %cmp25, label %for.body5.lr.ph, label %for.cond.cleanup4

for.body5.lr.ph:                                  ; preds = %for.cond.cleanup
  %wide.trip.count = sext i32 %n to i64
  br label %for.body5

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv27 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next28, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv27
  %0 = trunc i64 %indvars.iv27 to i32
  store i32 %0, ptr %arrayidx, align 4
  %indvars.iv.next28 = add nuw nsw i64 %indvars.iv27, 1
  %exitcond30 = icmp eq i64 %indvars.iv.next28, %wide.trip.count29
  br i1 %exitcond30, label %for.cond.cleanup, label %for.body

for.cond.cleanup4.loopexit:                       ; preds = %for.body5
  br label %for.cond.cleanup4

for.cond.cleanup4:                                ; preds = %for.cond.cleanup4.loopexit, %entry, %for.cond.cleanup
  ret void

for.body5:                                        ; preds = %for.body5, %for.body5.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body5.lr.ph ], [ %indvars.iv.next, %for.body5 ]
  %arrayidx7 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx7, align 4
  %add = add nsw i32 %1, 1
  %arrayidx9 = getelementptr inbounds i32, ptr %b, i64 %indvars.iv
  store i32 %add, ptr %arrayidx9, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup4.loopexit, label %for.body5
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }



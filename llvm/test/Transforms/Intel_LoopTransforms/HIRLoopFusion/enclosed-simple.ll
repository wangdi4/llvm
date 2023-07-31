; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; INPUT:
; BEGIN REGION { }
;      + DO i1 = 0, 99, 1   <DO_LOOP>
;      |   + DO i2 = 0, 99, 1   <DO_LOOP>
;      |   |   (%a)[i2] = i2;
;      |   + END LOOP
;      |
;      |
;      |   + DO i2 = 0, 99, 1   <DO_LOOP>
;      |   |   %1 = (%a)[i2];
;      |   |   (%b)[i2] = %1 + 1;
;      |   + END LOOP
;      + END LOOP
; END REGION

; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1 = 0, 99, 1

; CHECK: + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:    (%a)[i2] = i2;
; CHECK-NOT: + END LOOP
; CHECK:    %1 = (%a)[i2];
; CHECK:    (%b)[i2] = %1 + 1;
; CHECK: + END LOOP

; CHECK-NOT: + DO i2
; CHECK: + END LOOP

;Module Before HIR; ModuleID = 'enclosed-simple.c'
source_filename = "enclosed-simple.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr noalias nocapture %a, ptr nocapture %b, i32 %n) local_unnamed_addr #0 {
entry:
  br label %while.body

while.body:                                       ; preds = %entry, %for.cond.cleanup4
  %dec25 = phi i32 [ 99, %entry ], [ %dec, %for.cond.cleanup4 ]
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  br label %for.body5

for.body:                                         ; preds = %for.body, %while.body
  %indvars.iv = phi i64 [ 0, %while.body ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

for.cond.cleanup4:                                ; preds = %for.body5
  %dec = add nsw i32 %dec25, -1
  %tobool = icmp eq i32 %dec25, 0
  br i1 %tobool, label %while.end, label %while.body

for.body5:                                        ; preds = %for.body5, %for.cond.cleanup
  %indvars.iv26 = phi i64 [ 0, %for.cond.cleanup ], [ %indvars.iv.next27, %for.body5 ]
  %arrayidx7 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv26
  %1 = load i32, ptr %arrayidx7, align 4
  %add = add nsw i32 %1, 1
  %arrayidx9 = getelementptr inbounds i32, ptr %b, i64 %indvars.iv26
  store i32 %add, ptr %arrayidx9, align 4
  %indvars.iv.next27 = add nuw nsw i64 %indvars.iv26, 1
  %exitcond28 = icmp eq i64 %indvars.iv.next27, 100
  br i1 %exitcond28, label %for.cond.cleanup4, label %for.body5

while.end:                                        ; preds = %for.cond.cleanup4
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }



; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-create-function-level-region < %s 2>&1 | FileCheck %s

; INPUT:
; BEGIN REGION { }
;    + DO i1 = 0, 99, 1   <DO_LOOP>
;    |   (%a)[i1] = i1;
;    + END LOOP
;
;
;    + DO i1 = 0, 99, 1   <DO_LOOP>
;    |   %1 = (%a)[i1+1];
;    |   (%b)[i1] = %1 + 1;
;    + END LOOP
;
;    ret ;
; END REGION

; CHECK: Function
; CHECK: BEGIN REGION { }
; CHECK: DO i1
; CHECK: DO i1
; CHECK: END REGION

;Module Before HIR; ModuleID = 'enclosed-simple.c'
source_filename = "enclosed-simple.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr noalias nocapture %a, ptr noalias nocapture readnone %b, ptr nocapture %c, i32 %n) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  br label %for.body5

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv25 = phi i64 [ 0, %entry ], [ %indvars.iv.next26, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv25
  %0 = trunc i64 %indvars.iv25 to i32
  store i32 %0, ptr %arrayidx, align 4
  %indvars.iv.next26 = add nuw nsw i64 %indvars.iv25, 1
  %exitcond27 = icmp eq i64 %indvars.iv.next26, 100
  br i1 %exitcond27, label %for.cond.cleanup, label %for.body

for.cond.cleanup4:                                ; preds = %for.body5
  ret void

for.body5:                                        ; preds = %for.body5, %for.cond.cleanup
  %indvars.iv = phi i64 [ 0, %for.cond.cleanup ], [ %indvars.iv.next, %for.body5 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx7 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv.next
  %1 = load i32, ptr %arrayidx7, align 4
  %add8 = add nsw i32 %1, 1
  %arrayidx10 = getelementptr inbounds i32, ptr %c, i64 %indvars.iv
  store i32 %add8, ptr %arrayidx10, align 4
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup4, label %for.body5
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }



; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-create-function-level-region < %s 2>&1 | FileCheck %s

; INPUT:
; BEGIN REGION { }
; + DO i1 = 0, 99, 1   <DO_LOOP>
; |   (%a)[i1] = i1;
; + END LOOP
;
;
; + DO i1 = 0, 99, 1   <DO_LOOP>
; |   %1 = (%a)[i1];
; |   (%b)[i1] = %1 + 1;
; + END LOOP
;
;
; + DO i1 = 0, 99, 1   <DO_LOOP>
; |   %2 = (%b)[i1];
; |   (%c)[i1] = i1 + %2;
; + END LOOP
;
; ret ;
; END REGION

; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1 = 0, 99, 1
; CHECK:    (%a)[i1] = i1;
; CHECK-NOT: + END LOOP
; CHECK:    %1 = (%a)[i1];
; CHECK:    (%b)[i1] = %1 + 1;
; CHECK-NOT: + END LOOP
; CHECK:    %2 = (%b)[i1];
; CHECK:     (%c)[i1] = i1 + %2;
; CHECK: + END LOOP

; CHECK-NOT: + DO
; CHECK: END REGION

;Module Before HIR; ModuleID = 'enclosed-simple.c'
source_filename = "enclosed-simple.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr noalias nocapture %a, ptr noalias nocapture %b, ptr nocapture %c, i32 %n) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  br label %for.body5

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv47 = phi i64 [ 0, %entry ], [ %indvars.iv.next48, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv47
  %0 = trunc i64 %indvars.iv47 to i32
  store i32 %0, ptr %arrayidx, align 4
  %indvars.iv.next48 = add nuw nsw i64 %indvars.iv47, 1
  %exitcond49 = icmp eq i64 %indvars.iv.next48, 100
  br i1 %exitcond49, label %for.cond.cleanup, label %for.body

for.cond.cleanup4:                                ; preds = %for.body5
  br label %for.body17

for.body5:                                        ; preds = %for.body5, %for.cond.cleanup
  %indvars.iv44 = phi i64 [ 0, %for.cond.cleanup ], [ %indvars.iv.next45, %for.body5 ]
  %arrayidx7 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv44
  %1 = load i32, ptr %arrayidx7, align 4
  %add = add nsw i32 %1, 1
  %arrayidx9 = getelementptr inbounds i32, ptr %b, i64 %indvars.iv44
  store i32 %add, ptr %arrayidx9, align 4
  %indvars.iv.next45 = add nuw nsw i64 %indvars.iv44, 1
  %exitcond46 = icmp eq i64 %indvars.iv.next45, 100
  br i1 %exitcond46, label %for.cond.cleanup4, label %for.body5

for.cond.cleanup16:                               ; preds = %for.body17
  ret void

for.body17:                                       ; preds = %for.body17, %for.cond.cleanup4
  %indvars.iv = phi i64 [ 0, %for.cond.cleanup4 ], [ %indvars.iv.next, %for.body17 ]
  %arrayidx19 = getelementptr inbounds i32, ptr %b, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx19, align 4
  %3 = trunc i64 %indvars.iv to i32
  %add20 = add nsw i32 %2, %3
  %arrayidx22 = getelementptr inbounds i32, ptr %c, i64 %indvars.iv
  store i32 %add20, ptr %arrayidx22, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup16, label %for.body17
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }



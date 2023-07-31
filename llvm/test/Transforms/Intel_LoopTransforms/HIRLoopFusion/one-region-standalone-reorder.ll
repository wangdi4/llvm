; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-create-function-level-region < %s 2>&1 | FileCheck %s

; Verify that standalone node is reordered after the fusion.

; BEGIN REGION { }
; + DO i1 = 0, 99, 1   <DO_LOOP>
; |   (%a)[i1] = i1;
; + END LOOP
;
; (%b)[0] = 0;
;
; + DO i1 = 0, 99, 1   <DO_LOOP>
; |   %0 = (%a)[i1];
; |   %1 = (%b)[i1];
; |   (%b)[i1] = %0 + %1 + 1;
; + END LOOP
;
; ret ;
; END REGION

; CHECK: BEGIN REGION { modified }
; CHECK-NOT: DO
; CHECK: (%b)[0] = 0;
; CHECK-NOT: DO
;
; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   (%a)[i1] = i1;
; CHECK-NOT: END LOOP
; CHECK-NOT: DO
; CHECK: |   %0 = (%a)[i1];
; CHECK: |   %1 = (%b)[i1];
; CHECK: |   (%b)[i1] = %0 + %1 + 1;
; CHECK: + END LOOP
;
; CHECK: END REGION

;Module Before HIR; ModuleID = 'enclosed-simple.c'
source_filename = "enclosed-simple.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: norecurse nounwind
define void @foo(ptr noalias nocapture %a, ptr noalias nocapture %b, ptr nocapture readnone %c, i32 %n) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  store i32 0, ptr %b, align 4
  br label %for.body6

for.body:                                         ; preds = %for.body, %entry
  %j.027 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %a, i32 %j.027
  store i32 %j.027, ptr %arrayidx, align 4
  %inc = add nuw nsw i32 %j.027, 1
  %exitcond28 = icmp eq i32 %inc, 100
  br i1 %exitcond28, label %for.cond.cleanup, label %for.body

for.cond.cleanup5:                                ; preds = %for.body6
  ret void

for.body6:                                        ; preds = %for.body6, %for.cond.cleanup
  %j2.026 = phi i32 [ 0, %for.cond.cleanup ], [ %inc12, %for.body6 ]
  %arrayidx7 = getelementptr inbounds i32, ptr %a, i32 %j2.026
  %0 = load i32, ptr %arrayidx7, align 4
  %arrayidx8 = getelementptr inbounds i32, ptr %b, i32 %j2.026
  %1 = load i32, ptr %arrayidx8, align 4
  %add = add i32 %0, 1
  %add9 = add i32 %add, %1
  store i32 %add9, ptr %arrayidx8, align 4
  %inc12 = add nuw nsw i32 %j2.026, 1
  %exitcond = icmp eq i32 %inc12, 100
  br i1 %exitcond, label %for.cond.cleanup5, label %for.body6
}

attributes #0 = { norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }



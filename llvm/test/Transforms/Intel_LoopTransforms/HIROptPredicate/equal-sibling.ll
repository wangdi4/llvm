; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; BEGIN REGION { }
; + DO i1 = 0, 99, 1   <DO_LOOP>
; |   if (%n != 0)
; |   {
; |      (%p)[i1] = i1;
; |   }
; |   (%q)[i1] = i1;
; |   if (%m != 0)
; |   {
; |      (%p)[i1] = i1;
; |      if (%n != 0)
; |      {
; |         (%q)[i1] = 0;
; |      }
; |   }
; + END LOOP
; END REGION

; CHECK: BEGIN REGION { modified }
; CHECK: if (%n != 0)
; CHECK: {
; CHECK-NOT: if
; CHECK:   if (%m != 0)
; CHECK:   {
; CHECK-NOT: if
; CHECK:     + DO i1
; CHECK:     + END LOOP
; CHECK:   }
; CHECK:   else
; CHECK:   {
; CHECK-NOT: if
; CHECK:     + DO i1
; CHECK:     + END LOOP
; CHECK:   }
; CHECK: }
; CHECK: else
; CHECK: {
; CHECK:   if (%m != 0)
; CHECK:   {
; CHECK-NOT: if
; CHECK:     + DO i1
; CHECK:     + END LOOP
; CHECK:   }
; CHECK:   else
; CHECK:   {
; CHECK-NOT: if
; CHECK:     + DO i1
; CHECK:     + END LOOP
; CHECK:   }
; CHECK: }
; CHECK: END REGION

;Module Before HIR; ModuleID = '7.c'
source_filename = "7.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr nocapture %p, ptr nocapture %q, i64 %n, i64 %m) local_unnamed_addr #0 {
entry:
  %tobool = icmp ne i64 %n, 0
  %tobool2 = icmp eq i64 %m, 0
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc
  ret void

for.body:                                         ; preds = %for.inc, %entry
  %i.024 = phi i64 [ 0, %entry ], [ %inc, %for.inc ]
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i64, ptr %p, i64 %i.024
  store i64 %i.024, ptr %arrayidx, align 8
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %arrayidx1 = getelementptr inbounds i64, ptr %q, i64 %i.024
  store i64 %i.024, ptr %arrayidx1, align 8
  br i1 %tobool2, label %for.inc, label %if.then3

if.then3:                                         ; preds = %if.end
  %arrayidx5 = getelementptr inbounds i64, ptr %p, i64 %i.024
  store i64 %i.024, ptr %arrayidx5, align 8
  br i1 %tobool, label %if.then7, label %for.inc

if.then7:                                         ; preds = %if.then3
  store i64 0, ptr %arrayidx1, align 8
  br label %for.inc

for.inc:                                          ; preds = %if.end, %if.then7, %if.then3
  %inc = add nuw nsw i64 %i.024, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }



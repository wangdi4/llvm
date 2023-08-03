; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Verify that opt predicate may unswitch two identical nested Ifs.

; BEGIN REGION { }
; + DO i1 = 0, 99, 1   <DO_LOOP>
; |   if (undef true undef)
; |   {
; |      if (undef true undef)
; |      {
; |         (%a)[i1] = 0;
; |      }
; |   }
; + END LOOP
; END REGION

; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   (%a)[i1] = 0;
; CHECK: + END LOOP
; CHECK: END REGION

;Module Before HIR; ModuleID = 'equal-child.c'
source_filename = "equal-child.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32 %x, i32 %n, ptr nocapture %a) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc
  ret void

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %1, %for.inc ]
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %1 = add nuw nsw i64 %indvars.iv, 1
  br i1 true, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, ptr %a, i64 %1
  %2 = load i32, ptr %arrayidx3, align 4
  br i1 true, label %if.then5, label %for.inc

if.then5:                                         ; preds = %if.then
  store i32 0, ptr %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then5, %if.then
  %exitcond = icmp eq i64 %1, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }



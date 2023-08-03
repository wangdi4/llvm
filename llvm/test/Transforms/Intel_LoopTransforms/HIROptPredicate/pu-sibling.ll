; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; BEGIN REGION { }
; + DO i1 = 0, 99, 1   <DO_LOOP>
; |   %0 = (%a)[0];
; |   if (%0 == 0)
; |   {
; |      (%a)[i1] = i1;
; |   }
; |   %3 = (%b)[1];
; |   if (%3 == 0)
; |   {
; |      (%b)[i1] = i1;
; |   }
; + END LOOP
; END REGION

; CHECK: Function
; CHECK: BEGIN REGION { modified }
; CHECK: %0 = (%a)[0];
; CHECK: if (%0 == 0)
; CHECK: {
; CHECK:   %3 = (%b)[1];
; CHECK:   if (%3 == 0)
; CHECK:   {
; CHECK:     + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:     |   %0 = (%a)[0];
; CHECK:     |   if (%0 == 0)
; CHECK:     |   {
; CHECK:     |      (%a)[i1] = i1;
; CHECK:     |   }
; CHECK:     |   %3 = (%b)[1];
; CHECK:     |   if (%3 == 0)
; CHECK:     |   {
; CHECK:     |      (%b)[i1] = i1;
; CHECK:     |   }
; CHECK:     + END LOOP
; CHECK:   }
; CHECK:   else
; CHECK:   {
; CHECK:     + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:     |   %0 = (%a)[0];
; CHECK:     |   if (%0 == 0)
; CHECK:     |   {
; CHECK:     |      (%a)[i1] = i1;
; CHECK:     |   }
; CHECK:     + END LOOP
; CHECK:   }
; CHECK: }
; CHECK: else
; CHECK: {
; CHECK:   %3 = (%b)[1];
; CHECK:   if (%3 == 0)
; CHECK:   {
; CHECK:     + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:     |   %3 = (%b)[1];
; CHECK:     |   if (%3 == 0)
; CHECK:     |   {
; CHECK:     |      (%b)[i1] = i1;
; CHECK:     |   }
; CHECK:     + END LOOP
; CHECK:   }
; CHECK: }
; CHECK: END REGION

;Module Before HIR; ModuleID = '/export/iusers/pgprokof/loopopt-6/pu3.c'
source_filename = "/export/iusers/pgprokof/loopopt-6/pu3.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr noalias nocapture %a, ptr nocapture %b, ptr nocapture readnone %c) local_unnamed_addr #0 {
entry:
  %arrayidx5 = getelementptr inbounds i32, ptr %b, i64 1
  br label %for.body

for.cond.cleanup:                                 ; preds = %if.end13
  ret void

for.body:                                         ; preds = %if.end13, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %if.end13 ]
  %0 = load i32, ptr %a, align 4
  %1 = trunc i32 %0 to i8
  %cmp2 = icmp eq i8 %1, 0
  br i1 %cmp2, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %arrayidx4 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %2 = trunc i64 %indvars.iv to i32
  store i32 %2, ptr %arrayidx4, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %3 = load i32, ptr %arrayidx5, align 4
  %4 = trunc i32 %3 to i8
  %cmp8 = icmp eq i8 %4, 0
  br i1 %cmp8, label %if.then10, label %if.end13

if.then10:                                        ; preds = %if.end
  %arrayidx12 = getelementptr inbounds i32, ptr %b, i64 %indvars.iv
  %5 = trunc i64 %indvars.iv to i32
  store i32 %5, ptr %arrayidx12, align 4
  br label %if.end13

if.end13:                                         ; preds = %if.then10, %if.end
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }



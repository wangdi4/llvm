; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -xmain-opt-level=3 -disable-output < %s 2>&1 | FileCheck --check-prefix CHECK-TEMP-CLEANUP %s

; BEGIN REGION { }
; + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; |   %0 = (%p)[0]
; |   if (%0 > 100) // %0 may be substituted by hir-temp-cleanup, test check both cases
; |   {
; |      (%p)[i1] = i1;
; |      %.pre-phi = i1;
; |   }
; |   else
; |   {
; |      %.pre-phi = i1;
; |   }
; |   (%q)[i1] = %.pre-phi;
; + END LOOP
; END REGION

; CHECK-TEMP-CLEANUP: BEGIN REGION { modified }
; CHECK-TEMP-CLEANUP: if ((%p)[0] > 100)
; CHECK-TEMP-CLEANUP: {
; CHECK-TEMP-CLEANUP:   + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK-TEMP-CLEANUP:   |   if ((%p)[0] > 100)
; CHECK-TEMP-CLEANUP-SAME: <no_unswitch>
; CHECK-TEMP-CLEANUP:   |   {
; CHECK-TEMP-CLEANUP:   |      (%p)[i1] = i1;
; CHECK-TEMP-CLEANUP:   |      %.pre-phi = i1;
; CHECK-TEMP-CLEANUP:   |   }
; CHECK-TEMP-CLEANUP:   |   else
; CHECK-TEMP-CLEANUP:   |   {
; CHECK-TEMP-CLEANUP:   |      %.pre-phi = i1 + 1;
; CHECK-TEMP-CLEANUP:   |   }
; CHECK-TEMP-CLEANUP:   |   (%q)[i1] = %.pre-phi;
; CHECK-TEMP-CLEANUP:   + END LOOP
; CHECK-TEMP-CLEANUP: }
; CHECK-TEMP-CLEANUP: else
; CHECK-TEMP-CLEANUP: {
; CHECK-TEMP-CLEANUP:   + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK-TEMP-CLEANUP:   |   %.pre-phi = i1 + 1;
; CHECK-TEMP-CLEANUP:   |   (%q)[i1] = %.pre-phi;
; CHECK-TEMP-CLEANUP:   + END LOOP
; CHECK-TEMP-CLEANUP: }
; CHECK-TEMP-CLEANUP: END REGION

; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -xmain-opt-level=3 -disable-output < %s 2>&1 | FileCheck --check-prefix CHECK-NO-CLEANUP %s

; This is the same test as before, but without hir-temp-cleanup. The temp
; won't be removed from the condition.

; CHECK-NO-CLEANUP:     %0 = (%p)[0];
; CHECK-NO-CLEANUP:     if (%0 > 100)
; CHECK-NO-CLEANUP:        + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK-NO-CLEANUP:        |   %0 = (%p)[0];
; CHECK-NO-CLEANUP:        |   if (%0 > 100) <no_unswitch>

; CHECK-NO-CLEANUP:     else
; CHECK-NO-CLEANUP:     {
; CHECK-NO-CLEANUP:        + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK-NO-CLEANUP:        |   %.pre-phi = i1 + 1;
; CHECK-NO-CLEANUP:        |   (%q)[i1] = %.pre-phi;
; CHECK-NO-CLEANUP:        + END LOOP
; CHECK-NO-CLEANUP:     }


;Module Before HIR; ModuleID = '/export/iusers/pgprokof/loopopt-6/pu.c'
source_filename = "/export/iusers/pgprokof/loopopt-6/pu.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr nocapture %p, ptr noalias nocapture %q, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp13 = icmp sgt i32 %n, 0
  br i1 %cmp13, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %if.end
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %if.end, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %if.end ]
  %0 = load i32, ptr %p, align 4
  %cmp1 = icmp sgt i32 %0, 100
  br i1 %cmp1, label %if.then, label %for.body.if.end_crit_edge

for.body.if.end_crit_edge:                        ; preds = %for.body
  %.pre = trunc i64 %indvars.iv to i32
  %add = add i32 %.pre, 1
  br label %if.end

if.then:                                          ; preds = %for.body
  %arrayidx2 = getelementptr inbounds i32, ptr %p, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %arrayidx2, align 4
  br label %if.end

if.end:                                           ; preds = %for.body.if.end_crit_edge, %if.then
  %.pre-phi = phi i32 [ %add, %for.body.if.end_crit_edge ], [ %1, %if.then ]
  %arrayidx4 = getelementptr inbounds i32, ptr %q, i64 %indvars.iv
  store i32 %.pre-phi, ptr %arrayidx4, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }



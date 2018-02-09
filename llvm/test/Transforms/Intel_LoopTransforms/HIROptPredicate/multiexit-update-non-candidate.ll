; RUN: opt -hir-ssa-deconstruction -disable-output -hir-opt-predicate -print-after=hir-opt-predicate< %s 2>&1 | FileCheck %s

; HIR:
; BEGIN REGION { }
;     + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
;     |   %0 = (%a)[i1];
;     |   if (%0 != 0)
;     |   {
;     |      + DO i2 = 0, 99, 1   <DO_MULTI_EXIT_LOOP>
;     |      |   %j.023.out = i2;
;     |      |   %j.023.lcssa = %j.023.out;
;     |      |   if (%n == 5)
;     |      |   {
;     |      |      goto for.inc10.loopexit;
;     |      |   }
;     |      |   %j.023.lcssa = %j.023.out;
;     |      + END LOOP
;     |
;     |      for.inc10.loopexit:
;     |      (%b)[i1] = i1 + %j.023.lcssa;
;     |   }
;     + END LOOP
;  END REGION

; CHECK:   BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK:        |   %0 = (%a)[i1];
; CHECK:        |   if (%0 != 0)
; CHECK:        |   {
; CHECK:        |      if (%n == 5)
; CHECK:        |      {
; CHECK:        |         %j.023.out = 0;
; CHECK:        |         %j.023.lcssa = %j.023.out;
; CHECK:        |      }
; CHECK:        |      else
; CHECK:        |      {
; CHECK:        |         + DO i2 = 0, 99, 1   <DO_LOOP>
;               |         |                    ^ Should be regular do loop
; CHECK:        |         |   %j.023.out = i2;
; CHECK:        |         |   %j.023.lcssa = %j.023.out;
; CHECK:        |         |   %j.023.lcssa = %j.023.out;
; CHECK:        |         + END LOOP
; CHECK:        |      }
; CHECK:        |      (%b)[i1] = i1 + %j.023.lcssa;
; CHECK:        |   }
; CHECK:        + END LOOP
; CHECK:   END REGION

;Module Before HIR; ModuleID = 'multiexit-update-non-candidate.c'
source_filename = "multiexit-update-non-candidate.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture readonly %a, i32* nocapture %b, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp24 = icmp sgt i32 %n, 0
  br i1 %cmp24, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %cmp7 = icmp eq i32 %n, 5
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc10
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.inc10, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.inc10 ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %tobool = icmp eq i32 %0, 0
  br i1 %tobool, label %for.inc10, label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.body
  %arrayidx6 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  br label %for.body4

for.cond1:                                        ; preds = %for.body4
  %inc = add nuw nsw i32 %j.023, 1
  %cmp2 = icmp slt i32 %j.023, 99
  br i1 %cmp2, label %for.body4, label %for.inc10.loopexit

for.body4:                                        ; preds = %for.cond1.preheader, %for.cond1
  %j.023 = phi i32 [ 0, %for.cond1.preheader ], [ %inc, %for.cond1 ]
  br i1 %cmp7, label %for.inc10.loopexit, label %for.cond1

for.inc10.loopexit:                               ; preds = %for.body4, %for.cond1
  %j.023.lcssa = phi i32 [ %j.023, %for.body4 ], [ %j.023, %for.cond1 ]
  %1 = trunc i64 %indvars.iv to i32
  %add.le = add nuw nsw i32 %j.023.lcssa, %1
  store i32 %add.le, i32* %arrayidx6, align 4
  br label %for.inc10

for.inc10:                                        ; preds = %for.inc10.loopexit, %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }



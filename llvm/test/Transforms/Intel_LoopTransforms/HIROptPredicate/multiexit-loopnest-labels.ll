; RUN: opt -hir-ssa-deconstruction -disable-output -hir-opt-predicate -print-after=hir-opt-predicate < %s 2>&1 | FileCheck %s

; HIR:
; BEGIN REGION { }
;      + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
;      |   + DO i2 = 0, 9, 1   <DO_MULTI_EXIT_LOOP>
;      |   |   (%a)[i1] = i2;
;      |   |   %indvars.iv.next = i2  +  1;
;      |   |   if (%n + %m > 10)
;      |   |   {
;      |   |      goto if.then;
;      |   |   }
;      |   |   %2 = (%a)[i2];
;      |   |   (%a)[i2] = %2 + 1;
;      |   + END LOOP
;      |
;      |   goto cleanup;
;      |   if.then:
;      |   (%a)[i1] = %indvars.iv.next;
;      |   cleanup:
;      + END LOOP
; END REGION

; CHECK:       After
; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:        if (%n + %m > 10)
; CHECK-NEXT:        {
; CHECK-NEXT:          + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK-NEXT:          |   (%a)[i1] = 0;
; CHECK-NEXT:          |   %indvars.iv.next = 0  +  1;
; CHECK-NEXT:          |   (%a)[i1] = %indvars.iv.next;
; CHECK-NEXT:          + END LOOP
; CHECK-NEXT:        }
; CHECK-NEXT:        else
; CHECK-NEXT:        {
; CHECK-NEXT:          + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK-NEXT:          |   + DO i2 = 0, 9, 1   <DO_LOOP>
;                      |   |                    ^ should be regular do exit loop
; CHECK-NEXT:          |   |   (%a)[i1] = i2;
; CHECK-NEXT:          |   |   %indvars.iv.next = i2  +  1;
; CHECK-NEXT:          |   |   %2 = (%a)[i2];
; CHECK-NEXT:          |   |   (%a)[i2] = %2 + 1;
; CHECK-NEXT:          |   + END LOOP
; CHECK-NEXT:          + END LOOP
; CHECK-NEXT:        }
; CHECK-NEXT:  END REGION

;Module Before HIR; ModuleID = 'multiexit-perfert-loopnest.c'
source_filename = "multiexit-perfert-loopnest.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %a, i32 %n, i32 %m) local_unnamed_addr #0 {
entry:
  %cmp27 = icmp sgt i32 %n, 0
  br i1 %cmp27, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %add = add nsw i32 %m, %n
  %cmp5 = icmp sgt i32 %add, 10
  %wide.trip.count = sext i32 %n to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %cleanup, %for.cond1.preheader.lr.ph
  %indvars.iv30 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next31, %cleanup ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv30
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %cleanup
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body4:                                        ; preds = %for.cond1.preheader, %if.end
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %if.end ]
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  br i1 %cmp5, label %if.then, label %if.end

if.then:                                          ; preds = %for.body4
  %indvars.iv.next.lcssa = phi i64 [ %indvars.iv.next, %for.body4 ]
  %1 = trunc i64 %indvars.iv.next.lcssa to i32
  store i32 %1, i32* %arrayidx, align 4
  br label %cleanup

if.end:                                           ; preds = %for.body4
  %arrayidx9 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx9, align 4
  %inc10 = add nsw i32 %2, 1
  store i32 %inc10, i32* %arrayidx9, align 4
  %cmp2 = icmp slt i64 %indvars.iv, 9
  br i1 %cmp2, label %for.body4, label %cleanup.loopexit

cleanup.loopexit:                                 ; preds = %if.end
  br label %cleanup

cleanup:                                          ; preds = %cleanup.loopexit, %if.then
  %indvars.iv.next31 = add nuw nsw i64 %indvars.iv30, 1
  %exitcond = icmp eq i64 %indvars.iv.next31, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.cond1.preheader
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }



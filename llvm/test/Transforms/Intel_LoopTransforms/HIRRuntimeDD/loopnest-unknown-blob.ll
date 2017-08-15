; RUN: opt -hir-ssa-deconstruction -disable-output -hir-runtime-dd -print-after=hir-runtime-dd < %s 2>&1 | FileCheck %s

; Verify that the inner loop is multiversioned, while outer loops can not be transformed because of unknown %k.

; HIR:
; BEGIN REGION { }
;      + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>
;      |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>
;      |   |   %2 = (%b)[i1];
;      |   |   (%a)[sext.i32.i64(%k) * i1 + i2] = %2;
;      |   + END LOOP
;      + END LOOP
; END REGION

; CHECK: After
; CHECK: DO i1
; CHECK: if
; CHECK: DO i2

;Module Before HIR; ModuleID = 'loopnest-unknown-blob.c'
source_filename = "loopnest-unknown-blob.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %a, i32* nocapture readonly %b, i32 %n, i32 %k) local_unnamed_addr #0 {
entry:
  %cmp20 = icmp sgt i32 %n, 0
  br i1 %cmp20, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %0 = sext i32 %k to i64
  %wide.trip.count = zext i32 %n to i64
  br label %for.body4.lr.ph

for.body4.lr.ph:                                  ; preds = %for.cond1.preheader.lr.ph, %for.cond.cleanup3
  %indvars.iv23 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next24, %for.cond.cleanup3 ]
  %arrayidx = getelementptr inbounds i32, i32* %b, i64 %indvars.iv23
  %1 = mul nsw i64 %indvars.iv23, %0
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.cond.cleanup3:                                ; preds = %for.body4
  %indvars.iv.next24 = add nuw nsw i64 %indvars.iv23, 1
  %exitcond27 = icmp eq i64 %indvars.iv.next24, %wide.trip.count
  br i1 %exitcond27, label %for.cond.cleanup.loopexit, label %for.body4.lr.ph

for.body4:                                        ; preds = %for.body4, %for.body4.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body4.lr.ph ], [ %indvars.iv.next, %for.body4 ]
  %2 = load i32, i32* %arrayidx, align 4
  %3 = add nsw i64 %indvars.iv, %1
  %arrayidx6 = getelementptr inbounds i32, i32* %a, i64 %3
  store i32 %2, i32* %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }



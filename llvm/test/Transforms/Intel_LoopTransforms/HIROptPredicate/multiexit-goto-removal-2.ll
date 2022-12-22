; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Verify that the redundant goto and label are removed.

;   BEGIN REGION { }
;        + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
;        |   if (%k > 1024)
;        |   {
;        |      + DO i2 = 0, 9, 1   <DO_MULTI_EXIT_LOOP>
;        |      |   %0 = (%a)[i2];
;        |      |   (%a)[i2] = %0 + 1;
;        |      |   if (%n < 256)
;        |      |   {
;        |      |      goto L.loopexit;
;        |      |   }
;        |      |   (%a)[i2] = %0 + 2;
;        |      |   if (%m > 1024)
;        |      |   {
;        |      |      goto L.loopexit;
;        |      |   }
;        |      + END LOOP
;        |
;        |      L.loopexit:
;        |   }
;        |   %1 = (%a)[i1];
;        |   (%a)[i1] = %1 + 1;
;        + END LOOP
;   END REGION

; CHECK:   BEGIN REGION { modified }
; CHECK:         if (%k > 1024)
; CHECK:         {
; CHECK:            if (%n < 256)
; CHECK:            {
; CHECK:               if (%m > 1024)
; CHECK:               {
; CHECK:                  + DO i1 = 0, sext.i32.i64(%n) + -1, 1
; CHECK:                  |   %0 = (%a)[0];
; CHECK:                  |   (%a)[0] = %0 + 1;
; CHECK:                  |   %1 = (%a)[i1];
; CHECK:                  |   (%a)[i1] = %1 + 1;
; CHECK:                  + END LOOP
; CHECK:               }
; CHECK:               else
; CHECK:               {
; CHECK:                  + DO i1 = 0, sext.i32.i64(%n) + -1, 1
; CHECK:                  |   %0 = (%a)[0];
; CHECK:                  |   (%a)[0] = %0 + 1;
; CHECK:                  |   %1 = (%a)[i1];
; CHECK:                  |   (%a)[i1] = %1 + 1;
; CHECK-NOT:                 goto L.loopexit;
; CHECK:                  + END LOOP
; CHECK:               }
; CHECK:            }
; CHECK:            else
; CHECK:            {
; CHECK:               if (%m > 1024)
; CHECK:               {
; CHECK:                  + DO i1 = 0, sext.i32.i64(%n) + -1, 1
; CHECK:                  |   %0 = (%a)[0];
; CHECK:                  |   (%a)[0] = %0 + 1;
; CHECK:                  |   (%a)[0] = %0 + 2;
; CHECK:                  |   %1 = (%a)[i1];
; CHECK:                  |   (%a)[i1] = %1 + 1;
; CHECK:                  + END LOOP
; CHECK:               }
; CHECK:               else
; CHECK:               {
; CHECK:                  + DO i1 = 0, sext.i32.i64(%n) + -1, 1
; CHECK:                  |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:                  |   |   %0 = (%a)[i2];
; CHECK:                  |   |   (%a)[i2] = %0 + 1;
; CHECK:                  |   |   (%a)[i2] = %0 + 2;
; CHECK-NOT:                    goto L.loopexit;
; CHECK:                  |   + END LOOP
; CHECK:                  |
; CHECK:                  |   %1 = (%a)[i1];
; CHECK:                  |   (%a)[i1] = %1 + 1;
; CHECK:                  + END LOOP
; CHECK:               }
; CHECK:            }
; CHECK:         }
; CHECK:         else
; CHECK:         {
; CHECK:            + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:            |   %1 = (%a)[i1];
; CHECK:            |   (%a)[i1] = %1 + 1;
; CHECK:            + END LOOP
; CHECK:         }
; CHECK:   END REGION


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %a, i32 %n, i32 %m, i32 %k) local_unnamed_addr #0 {
entry:
  %cmp31 = icmp sgt i32 %n, 0
  br i1 %cmp31, label %for.body.lr.ph, label %cleanup18

for.body.lr.ph:                                   ; preds = %entry
  %cmp1 = icmp sgt i32 %m, 1024
  %cmp11 = icmp sgt i32 %k, 1024
  %cmp6 = icmp slt i32 %n, 256
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %L, %for.body.lr.ph
  %indvars.iv33 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next34, %L ]
  br i1 %cmp11, label %for.body5.preheader, label %L

for.body5.preheader:                              ; preds = %for.body
  br label %for.body5

for.body5:                                        ; preds = %for.body5.preheader, %if.end
  %indvars.iv = phi i64 [ %indvars.iv.next, %b ], [ 0, %for.body5.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32* %arrayidx, align 4
  br i1 %cmp6, label %L.loopexit, label %if.end

if.end:                                           ; preds = %for.body5
  %inc10 = add nsw i32 %0, 2
  store i32 %inc10, i32* %arrayidx, align 4
  br i1 %cmp1, label %L.loopexit, label %b

b:
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp3 = icmp slt i64 %indvars.iv, 9
  br i1 %cmp3, label %for.body5, label %L.loopexit

L.loopexit:                                       ; preds = %for.body5, %if.end
  br label %L

L:                                                ; preds = %L.loopexit, %for.body
  %arrayidx14 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv33
  %1 = load i32, i32* %arrayidx14, align 4
  %inc15 = add nsw i32 %1, 1
  store i32 %inc15, i32* %arrayidx14, align 4
  %indvars.iv.next34 = add nuw nsw i64 %indvars.iv33, 1
  %exitcond = icmp eq i64 %indvars.iv.next34, %wide.trip.count
  br i1 %exitcond, label %cleanup18.loopexit, label %for.body

cleanup18.loopexit:                               ; preds = %L
  br label %cleanup18

cleanup18:                                        ; preds = %cleanup18.loopexit, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }



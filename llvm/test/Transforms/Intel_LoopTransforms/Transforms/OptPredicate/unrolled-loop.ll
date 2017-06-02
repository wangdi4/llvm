; RUN: opt -hir-ssa-deconstruction -hir-pre-vec-complete-unroll -hir-opt-predicate -print-after=hir-opt-predicate -disable-output < %s 2>&1 | FileCheck %s

; HIR:
; <0>       BEGIN REGION { }
; <31>            + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; <32>            |   + DO i2 = 0, 9, 1   <DO_LOOP>
; <4>             |   |   if (%c < 10)
; <4>             |   |   {
; <10>            |   |      (%a)[i2] = i2;
; <4>             |   |   }
; <13>            |   |   if (%n < 100)
; <13>            |   |   {
; <17>            |   |      (%b)[%n] = 0;
; <13>            |   |   }
; <32>            |   + END LOOP
; <31>            + END LOOP
; <0>       END REGION

; After unroll:
; <0>       BEGIN REGION { modified }
; <34>            + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; <36>            |   if (%c < 10)
; <36>            |   {
; <37>            |      (%a)[0] = 0;
; <36>            |   }
; <38>            |   if (%n < 100)
; <38>            |   {
; <39>            |      (%b)[%n] = 0;
; <38>            |   }

; ...

; <72>            |   if (%c < 10)
; <72>            |   {
; <73>            |      (%a)[9] = 9;
; <72>            |   }
; <74>            |   if (%n < 100)
; <74>            |   {
; <75>            |      (%b)[%n] = 0;
; <74>            |   }
; <34>            + END LOOP
; <0>       END REGION

; CHECK: BEGIN REGION { modified }
; CHECK:       if (%c < 10)
; CHECK:       {
; CHECK:         if (%n < 100)
; CHECK:         {
; CHECK:            + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK:            |   (%a)[0] = 0;
; CHECK:            |   (%b)[%n] = 0;
; CHECK:            |   (%a)[1] = 1;
; CHECK:            |   (%b)[%n] = 0;
; CHECK:            |   (%a)[2] = 2;
; CHECK:            |   (%b)[%n] = 0;
; CHECK:            |   (%a)[3] = 3;
; CHECK:            |   (%b)[%n] = 0;
; CHECK:            |   (%a)[4] = 4;
; CHECK:            |   (%b)[%n] = 0;
; CHECK:            |   (%a)[5] = 5;
; CHECK:            |   (%b)[%n] = 0;
; CHECK:            |   (%a)[6] = 6;
; CHECK:            |   (%b)[%n] = 0;
; CHECK:            |   (%a)[7] = 7;
; CHECK:            |   (%b)[%n] = 0;
; CHECK:            |   (%a)[8] = 8;
; CHECK:            |   (%b)[%n] = 0;
; CHECK:            |   (%a)[9] = 9;
; CHECK:            |   (%b)[%n] = 0;
; CHECK:            + END LOOP
; CHECK:         }
; CHECK:         else
; CHECK:         {
; CHECK:            + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK:            |   (%a)[0] = 0;
; CHECK:            |   (%a)[1] = 1;
; CHECK:            |   (%a)[2] = 2;
; CHECK:            |   (%a)[3] = 3;
; CHECK:            |   (%a)[4] = 4;
; CHECK:            |   (%a)[5] = 5;
; CHECK:            |   (%a)[6] = 6;
; CHECK:            |   (%a)[7] = 7;
; CHECK:            |   (%a)[8] = 8;
; CHECK:            |   (%a)[9] = 9;
; CHECK:            + END LOOP
; CHECK:         }
; CHECK:       }
; CHECK:       else
; CHECK:       {
; CHECK:         if (%n < 100)
; CHECK:         {
; CHECK:            + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK:            |   (%b)[%n] = 0;
; CHECK:            |   (%b)[%n] = 0;
; CHECK:            |   (%b)[%n] = 0;
; CHECK:            |   (%b)[%n] = 0;
; CHECK:            |   (%b)[%n] = 0;
; CHECK:            |   (%b)[%n] = 0;
; CHECK:            |   (%b)[%n] = 0;
; CHECK:            |   (%b)[%n] = 0;
; CHECK:            |   (%b)[%n] = 0;
; CHECK:            |   (%b)[%n] = 0;
; CHECK:            + END LOOP
; CHECK:         }
; CHECK:       }
; CHECK: END REGION

;Module Before HIR; ModuleID = '2.c'
source_filename = "2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %a, i32* nocapture %b, i32 %c, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp21 = icmp sgt i32 %n, 0
  br i1 %cmp21, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp5 = icmp slt i32 %c, 10
  %cmp6 = icmp slt i32 %n, 100
  %idx.ext = sext i32 %n to i64
  %add.ptr = getelementptr inbounds i32, i32* %b, i64 %idx.ext
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %for.cond1.preheader.lr.ph
  %j.022 = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %inc10, %for.cond.cleanup3 ]
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.cond.cleanup3:                                ; preds = %for.inc
  %inc10 = add nuw nsw i32 %j.022, 1
  %exitcond23 = icmp eq i32 %inc10, %n
  br i1 %exitcond23, label %for.cond.cleanup.loopexit, label %for.cond1.preheader

for.body4:                                        ; preds = %for.inc, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.inc ]
  br i1 %cmp5, label %if.then, label %if.end

if.then:                                          ; preds = %for.body4
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body4
  br i1 %cmp6, label %if.then7, label %for.inc

if.then7:                                         ; preds = %if.end
  store i32 0, i32* %add.ptr, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.end, %if.then7
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }



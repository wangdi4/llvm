; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Verify that after inner loop complete unrolling the empty switches will be removed.

; BEGIN REGION { }
;      + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;      |   + DO i2 = 0, 2, 1   <DO_LOOP>
;      |   |   switch((%a)[i2])
;      |   |   {
;      |   |   case 0:
;      |   |      if (i2 == 1)
;      |   |      {
;      |   |         (%a)[i2] = 3;
;      |   |      }
;      |   |      break;
;      |   |   case 1:
;      |   |      if (i2 == 1)
;      |   |      {
;      |   |         (%a)[i2] = 7;
;      |   |      }
;      |   |      break;
;      |   |   default:
;      |   |      break;
;      |   |   }
;      |   + END LOOP
;      |   %1 = (%a)[i1];
;      |   (%a)[i1] = %1 + 1;
;      + END LOOP
; END REGION

; CHECK:  BEGIN REGION
; CHECK:  + DO i1 = 0, %n + -1, 1
; CHECK-NOT:  DO i2
; CHECK:  |   switch((%a)[1])
; CHECK:  |   {
; CHECK:  |   case 0:
; CHECK:  |      (%a)[1] = 3;
; CHECK:  |      break;
; CHECK:  |   case 1:
; CHECK:  |      (%a)[1] = 7;
; CHECK:  |      break;
; CHECK:  |   default:
; CHECK:  |      break;
; CHECK:  |   }
; CHECK-NOT:  switch
; CHECK:  |   %1 = (%a)[i1];
; CHECK:  |   (%a)[i1] = %1 + 1;
; CHECK:  + END LOOP
; CHECK:  END REGION

; ModuleID = 'switch-empty.ll'
source_filename = "switch-empty.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @foo(ptr %a, i32 %n) #0 {
entry:
  %cmp3 = icmp slt i32 0, %n
  br i1 %cmp3, label %for.body.lr.ph, label %for.end18

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc16
  %i.0 = phi i32 [ 0, %for.body.lr.ph ], [ %inc17, %for.inc16 ]
  %cmp21 = icmp slt i32 0, 3
  br i1 %cmp21, label %for.body3.lr.ph, label %for.end

for.body3.lr.ph:                                  ; preds = %for.body
  br label %for.body3

for.body3:                                        ; preds = %for.body3.lr.ph, %for.inc
  %j.0 = phi i32 [ 0, %for.body3.lr.ph ], [ %inc, %for.inc ]
  %idxprom = sext i32 %j.0 to i64
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %idxprom
  %0 = load i32, ptr %arrayidx, align 4
  switch i32 %0, label %sw.default [
    i32 0, label %sw.bb
    i32 1, label %sw.bb7
  ]

sw.bb:                                            ; preds = %for.body3
  %cmp4 = icmp eq i32 %j.0, 1
  br i1 %cmp4, label %if.then, label %if.end

if.then:                                          ; preds = %sw.bb
  %idxprom5 = sext i32 %j.0 to i64
  %arrayidx6 = getelementptr inbounds i32, ptr %a, i64 %idxprom5
  store i32 3, ptr %arrayidx6, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %sw.bb
  br label %sw.epilog

sw.bb7:                                           ; preds = %for.body3
  %cmp8 = icmp eq i32 %j.0, 1
  br i1 %cmp8, label %if.then9, label %if.end12

if.then9:                                         ; preds = %sw.bb7
  %idxprom10 = sext i32 %j.0 to i64
  %arrayidx11 = getelementptr inbounds i32, ptr %a, i64 %idxprom10
  store i32 7, ptr %arrayidx11, align 4
  br label %if.end12

if.end12:                                         ; preds = %if.then9, %sw.bb7
  br label %sw.epilog

sw.default:                                       ; preds = %for.body3
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %if.end12, %if.end
  br label %for.inc

for.inc:                                          ; preds = %sw.epilog
  %inc = add nsw i32 %j.0, 1
  %cmp2 = icmp slt i32 %inc, 3
  br i1 %cmp2, label %for.body3, label %for.cond1.for.end_crit_edge

for.cond1.for.end_crit_edge:                      ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond1.for.end_crit_edge, %for.body
  %idxprom13 = sext i32 %i.0 to i64
  %arrayidx14 = getelementptr inbounds i32, ptr %a, i64 %idxprom13
  %1 = load i32, ptr %arrayidx14, align 4
  %inc15 = add nsw i32 %1, 1
  store i32 %inc15, ptr %arrayidx14, align 4
  br label %for.inc16

for.inc16:                                        ; preds = %for.end
  %inc17 = add nsw i32 %i.0, 1
  %cmp = icmp slt i32 %inc17, %n
  br i1 %cmp, label %for.body, label %for.cond.for.end18_crit_edge

for.cond.for.end18_crit_edge:                     ; preds = %for.inc16
  br label %for.end18

for.end18:                                        ; preds = %for.cond.for.end18_crit_edge, %entry
  ret void
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }


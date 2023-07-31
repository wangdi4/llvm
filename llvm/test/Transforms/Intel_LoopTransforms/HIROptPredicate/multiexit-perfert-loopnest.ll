; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" < %s -hir-cost-model-throttling=0 -disable-output < %s 2>&1 | FileCheck %s

; Verify that loop unswitching completely removes the loopnest in the true case of the if (%n > 10).

; + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483647>
; |   + DO i2 = 0, 49, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483646>
; |   |   %1 = i2  +  i1;
; |   |   if (%n > 10)
; |   |   {
; |   |      goto if.then;
; |   |   }
; |   |   %4 = (%a)[i1 + i2];
; |   |   %5 = (%4)[0];
; |   |   (%b)[i1] = %5;
; |   + END LOOP
; + END LOOP

; CHECK: Function
; CHECK: BEGIN REGION { modified }
; CHECK:       if (%n > 10)
; CHECK:       {
; CHECK-NOT:      DO
; CHECK:          %1 = 0  +  0;
; CHECK:          goto if.then;
; CHECK:       }
; CHECK:       else
; CHECK:       {
; CHECK:         + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:         |   + DO i2 = 0, 49, 1   <DO_LOOP>
; CHECK:         |   |   %1 = i2  +  i1;
; CHECK:         |   |   %4 = (%a)[i1 + i2];
; CHECK:         |   |   %5 = (%4)[0];
; CHECK:         |   |   (%b)[i1] = %5;
; CHECK:         |   + END LOOP
; CHECK:         + END LOOP
; CHECK:       }
; CHECK: END REGION

;Module Before HIR; ModuleID = 'multiexit-perfect-loopnest.c'
source_filename = "multiexit-perfert-loopnest.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr nocapture %a, ptr nocapture %b, i32 %n, i32 %m) {
entry:
  %cmp39 = icmp sgt i32 %n, 0
  br i1 %cmp39, label %for.cond1.preheader.lr.ph, label %cleanup17

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp5 = icmp sgt i32 %n, 10
  %0 = sext i32 %n to i64
  br label %for.body4.lr.ph

for.body4.lr.ph:                                  ; preds = %for.cond1.preheader
  %indvars.iv45 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next46, %for.inc15 ]
  %arrayidx13 = getelementptr inbounds i32, ptr %b, i64 %indvars.iv45
  br label %for.body4

for.body4:                                        ; preds = %for.body4.lr.ph, %if.end
  %indvars.iv = phi i64 [ 0, %for.body4.lr.ph ], [ %indvars.iv.next, %if.end ]
  %1 = add nuw nsw i64 %indvars.iv, %indvars.iv45
  br i1 %cmp5, label %if.then, label %if.end

if.then:                                          ; preds = %for.body4
  %.lcssa = phi i64 [ %1, %for.body4 ]
  %2 = load ptr, ptr %a, align 8
  %arrayidx7 = getelementptr inbounds i32, ptr %2, i64 %.lcssa
  %3 = load i32, ptr %arrayidx7, align 4
  %inc = add nsw i32 %3, 1
  store i32 %inc, ptr %arrayidx7, align 4
  br label %cleanup17

if.end:                                           ; preds = %for.body4
  %arrayidx10 = getelementptr inbounds ptr, ptr %a, i64 %1
  %4 = load ptr, ptr %arrayidx10, align 8
  %5 = load i32, ptr %4, align 4
  store i32 %5, ptr %arrayidx13, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp2 = icmp slt i64 %indvars.iv.next, 50
  br i1 %cmp2, label %for.body4, label %for.inc15.loopexit

for.inc15.loopexit:                               ; preds = %if.end
  br label %for.inc15

for.inc15:                                        ; preds = %for.inc15.loopexit, %for.cond1.preheader
  %indvars.iv.next46 = add nuw nsw i64 %indvars.iv45, 1
  %cmp = icmp slt i64 %indvars.iv.next46, %0
  br i1 %cmp, label %for.body4.lr.ph, label %cleanup17.loopexit

cleanup17.loopexit:                               ; preds = %for.inc15
  br label %cleanup17

cleanup17:                                        ; preds = %cleanup17.loopexit, %entry, %if.then
  %6 = load ptr, ptr %a, align 8
  %incdec.ptr = getelementptr inbounds i32, ptr %6, i64 1
  store ptr %incdec.ptr, ptr %a, align 8
  ret void
}


; RUN: opt < %s -hir-ssa-deconstruction -hir-cost-model-throttling=0 | opt -analyze -enable-new-pm=0 -hir-framework -hir-framework-debug=parser -hir-cost-model-throttling=0 | FileCheck %s
; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -hir-cost-model-throttling=0 -disable-output  2>&1 | FileCheck %s

; Verify that we are able to parse multi-exit loopnest correctly.

; CHECK: + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK: |   + DO i2 = 0, i1 + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483646>
; CHECK: |   |   %1 = i2  +  i1;
; CHECK: |   |   if (%n > 10)
; CHECK: |   |   {
; CHECK: |   |      goto if.then;
; CHECK: |   |   }
; CHECK: |   |   %4 = (%a)[i1 + i2];
; CHECK: |   |   %5 = (%4)[0];
; CHECK: |   |   (%b)[i1] = %5;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


;Module Before HIR; ModuleID = 'multiexit-perfect-loopnest.c'
source_filename = "multiexit-perfert-loopnest.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32** nocapture %a, i32* nocapture %b, i32 %n, i32 %m) {
entry:
  %cmp39 = icmp sgt i32 %n, 0
  br i1 %cmp39, label %for.cond1.preheader.lr.ph, label %cleanup17

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp5 = icmp sgt i32 %n, 10
  %0 = sext i32 %n to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.inc15
  %indvars.iv45 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next46, %for.inc15 ]
  %cmp237 = icmp sgt i64 %indvars.iv45, 0
  br i1 %cmp237, label %for.body4.lr.ph, label %for.inc15

for.body4.lr.ph:                                  ; preds = %for.cond1.preheader
  %arrayidx13 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv45
  br label %for.body4

for.body4:                                        ; preds = %for.body4.lr.ph, %if.end
  %indvars.iv = phi i64 [ 0, %for.body4.lr.ph ], [ %indvars.iv.next, %if.end ]
  %1 = add nuw nsw i64 %indvars.iv, %indvars.iv45
  br i1 %cmp5, label %if.then, label %if.end

if.then:                                          ; preds = %for.body4
  %.lcssa = phi i64 [ %1, %for.body4 ]
  %2 = load i32*, i32** %a, align 8
  %arrayidx7 = getelementptr inbounds i32, i32* %2, i64 %.lcssa
  %3 = load i32, i32* %arrayidx7, align 4
  %inc = add nsw i32 %3, 1
  store i32 %inc, i32* %arrayidx7, align 4
  br label %cleanup17

if.end:                                           ; preds = %for.body4
  %arrayidx10 = getelementptr inbounds i32*, i32** %a, i64 %1
  %4 = load i32*, i32** %arrayidx10, align 8
  %5 = load i32, i32* %4, align 4
  store i32 %5, i32* %arrayidx13, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp2 = icmp slt i64 %indvars.iv.next, %indvars.iv45
  br i1 %cmp2, label %for.body4, label %for.inc15.loopexit

for.inc15.loopexit:                               ; preds = %if.end
  br label %for.inc15

for.inc15:                                        ; preds = %for.inc15.loopexit, %for.cond1.preheader
  %indvars.iv.next46 = add nuw nsw i64 %indvars.iv45, 1
  %cmp = icmp slt i64 %indvars.iv.next46, %0
  br i1 %cmp, label %for.cond1.preheader, label %cleanup17.loopexit

cleanup17.loopexit:                               ; preds = %for.inc15
  br label %cleanup17

cleanup17:                                        ; preds = %cleanup17.loopexit, %entry, %if.then
  %6 = load i32*, i32** %a, align 8
  %incdec.ptr = getelementptr inbounds i32, i32* %6, i64 1
  store i32* %incdec.ptr, i32** %a, align 8
  ret void
}


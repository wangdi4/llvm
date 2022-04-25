; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region 2>&1 | FileCheck %s

; Verify that both flow edges for %0 are constructed with (=) DV as the definition dominates both uses.

; + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; |   %0 = (%A)[i1];
; |   %1 = (%B)[i1];
; |   if (%1 > 5)
; |   {
; |      + DO i2 = 0, 3, 1   <DO_MULTI_EXIT_LOOP>
; |      |   %3 = (%B)[i1];
; |      |   if (%3 < 10)
; |      |   {
; |      |      (%B)[i1] = %0;
; |      |   }
; |      |   %4 = (%A)[i1 + 1];
; |      |   if (%4 > 2)
; |      |   {
; |      |      goto for.inc22.loopexit;
; |      |   }
; |      |   (%B)[i1 + 1] = %0;
; |      + END LOOP
; |
; |      for.inc22.loopexit:
; |   }
; + END LOOP

; CHECK: %0 --> %0 FLOW (=)
; CHECK: %0 --> %0 FLOW (=)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture readonly %A, i32* nocapture %B, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp42 = icmp sgt i32 %n, 0
  br i1 %cmp42, label %for.body.lr.ph, label %for.end24

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc22
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.inc22 ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx2, align 4
  %cmp3 = icmp sgt i32 %1, 5
  br i1 %cmp3, label %if.then, label %for.inc22

if.then:                                          ; preds = %for.body
  %2 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx14 = getelementptr inbounds i32, i32* %A, i64 %2
  %arrayidx20 = getelementptr inbounds i32, i32* %B, i64 %2
  br label %for.body6

for.body6:                                        ; preds = %if.then, %if.end17
  %j.041 = phi i32 [ 0, %if.then ], [ %inc, %if.end17 ]
  %3 = load i32, i32* %arrayidx2, align 4
  %cmp9 = icmp slt i32 %3, 10
  br i1 %cmp9, label %if.then10, label %if.end

if.then10:                                        ; preds = %for.body6
  store i32 %0, i32* %arrayidx2, align 4
  br label %if.end

if.end:                                           ; preds = %if.then10, %for.body6
  %4 = load i32, i32* %arrayidx14, align 4
  %cmp15 = icmp sgt i32 %4, 2
  br i1 %cmp15, label %for.inc22.loopexit, label %if.end17

if.end17:                                         ; preds = %if.end
  store i32 %0, i32* %arrayidx20, align 4
  %inc = add nuw nsw i32 %j.041, 1
  %cmp5 = icmp ult i32 %inc, 4
  br i1 %cmp5, label %for.body6, label %for.inc22.loopexit

for.inc22.loopexit:                               ; preds = %if.end, %if.end17
  br label %for.inc22

for.inc22:                                        ; preds = %for.inc22.loopexit, %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %wide.trip.count = sext i32 %n to i64
  %exitcond = icmp ne i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.body, label %for.end24.loopexit

for.end24.loopexit:                               ; preds = %for.inc22
  br label %for.end24

for.end24:                                        ; preds = %for.end24.loopexit, %entry
  ret void
}

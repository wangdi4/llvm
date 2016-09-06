; RUN: opt < %s -hir-ssa-deconstruction -hir-cg -force-hir-cg -S | FileCheck %s
; liveout value %cmp3 is in defined in entry block of region, for.body
; that def shouldn't be replaced or modified

; CHECK: %cmp3 = icmp slt i32 %0, %1
; ModuleID = 'cmp.cpp'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define zeroext i1 @_Z3fooPiS_i(i32* nocapture %A, i32* nocapture %B, i32 %n) {
entry:
  %cmp.26 = icmp sgt i32 %n, 0
  br i1 %cmp.26, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx2, align 4
  %cmp3 = icmp slt i32 %0, %1
  br i1 %cmp3, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  store i32 %1, i32* %arrayidx, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body
  store i32 %0, i32* %arrayidx2, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.else, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %small.0.lcssa = phi i1 [ undef, %entry ], [ %cmp3, %for.end.loopexit ]
  ret i1 %small.0.lcssa
}

; RUN: opt -xmain-opt-level=3 -hir-ssa-deconstruction -hir-framework -analyze -enable-new-pm=0 < %s 2>&1 | FileCheck %s
; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Check that loopnest with 7 *ifs* will be recognized.

; CHECK: if
; CHECK: if
; CHECK: if
; CHECK: if
; CHECK: if
; CHECK: if
; CHECK: if

source_filename = "dbg-inst.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i32* nocapture %a, i32 %n) local_unnamed_addr #0 {
entry:
  %arrayidx1 = getelementptr inbounds i32, i32* %a, i64 1
  %arrayidx4 = getelementptr inbounds i32, i32* %a, i64 2
  %arrayidx7 = getelementptr inbounds i32, i32* %a, i64 3
  %arrayidx10 = getelementptr inbounds i32, i32* %a, i64 4
  %arrayidx13 = getelementptr inbounds i32, i32* %a, i64 5
  %arrayidx16 = getelementptr inbounds i32, i32* %a, i64 6
  %0 = sext i32 %n to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc
  ret void

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %1 = load i32, i32* %a, align 4
  %tobool = icmp eq i32 %1, 0
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %2 = load i32, i32* %arrayidx1, align 4
  %tobool2 = icmp eq i32 %2, 0
  br i1 %tobool2, label %for.inc, label %if.then3

if.then3:                                         ; preds = %if.then
  %3 = load i32, i32* %arrayidx4, align 4
  %tobool5 = icmp eq i32 %3, 0
  br i1 %tobool5, label %for.inc, label %if.then6

if.then6:                                         ; preds = %if.then3
  %4 = load i32, i32* %arrayidx7, align 4
  %tobool8 = icmp eq i32 %4, 0
  br i1 %tobool8, label %for.inc, label %if.then9

if.then9:                                         ; preds = %if.then6
  %5 = load i32, i32* %arrayidx10, align 4
  %tobool11 = icmp eq i32 %5, 0
  br i1 %tobool11, label %for.inc, label %if.then12

if.then12:                                        ; preds = %if.then9
  %6 = load i32, i32* %arrayidx13, align 4
  %tobool14 = icmp eq i32 %6, 0
  br i1 %tobool14, label %for.inc, label %if.then15

if.then15:                                        ; preds = %if.then12
  %7 = load i32, i32* %arrayidx16, align 4
  %tobool17 = icmp eq i32 %7, 0
  br i1 %tobool17, label %for.inc, label %if.then18

if.then18:                                        ; preds = %if.then15
  %8 = add nsw i64 %indvars.iv, %0
  %arrayidx19 = getelementptr inbounds i32, i32* %a, i64 %8
  %9 = trunc i64 %indvars.iv to i32
  store i32 %9, i32* %arrayidx19, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then15, %if.then12, %if.then9, %if.then6, %if.then3, %if.then, %for.body, %if.then18
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}


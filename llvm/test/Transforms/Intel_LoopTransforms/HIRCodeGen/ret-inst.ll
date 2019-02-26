; RUN: opt < %s -hir-create-function-level-region -hir-ssa-deconstruction -hir-cg -force-hir-cg -S | FileCheck %s
; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,hir-cg" < %s -force-hir-cg -S | FileCheck %s

; Verify that return instruction is correctly handled by CG.

; CHECK: region.0:
; CHECK: loop.[[LOOPNUM:[0-9]+]]:
; CHECK: afterloop.[[LOOPNUM]]:
; CHECK: ret void

; ModuleID = 't2.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* nocapture %arr, i32* nocapture %barr) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %index.022 = phi i64 [ 0, %entry ], [ %inc9, %for.inc ]
  %rem = srem i64 %index.022, 3
  %tobool = icmp eq i64 %rem, 0
  br i1 %tobool, label %if.else, label %if.then

if.then:                                          ; preds = %for.body
  %add = add nuw nsw i64 %index.022, 2
  %conv = trunc i64 %add to i32
  %arrayidx = getelementptr inbounds i32, i32* %arr, i64 %index.022
  store i32 %conv, i32* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds i32, i32* %barr, i64 %index.022
  %0 = load i32, i32* %arrayidx1, align 4
  %tobool2 = icmp eq i32 %0, 0
  br i1 %tobool2, label %for.inc, label %if.then.3

if.then.3:                                        ; preds = %if.then
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32* %arrayidx1, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body
  %conv6 = trunc i64 %index.022 to i32
  %arrayidx7 = getelementptr inbounds i32, i32* %arr, i64 %index.022
  store i32 %conv6, i32* %arrayidx7, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else, %if.then.3
  %inc9 = add nuw nsw i64 %index.022, 1
  %exitcond = icmp eq i64 %inc9, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}


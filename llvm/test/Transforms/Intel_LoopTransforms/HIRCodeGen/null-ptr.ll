; RUN: opt < %s -loop-simplify -hir-ssa-deconstruction -hir-cg -force-hir-cg -S | FileCheck %s

; Check that CG correctly handles the pointer comparison with null pointer.
; CHECK: region:
; CHECK: hir.cmp.{{[0-9]+}}
; CHECK-SAME: icmp eq i32* 
; CHECK-SAME: null


; ModuleID = 'null-ptr.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32** nocapture readonly %A, i32* nocapture %B, i32 %n) {
entry:
  %cmp.14 = icmp sgt i32 %n, 0
  br i1 %cmp.14, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds i32*, i32** %A, i64 %indvars.iv
  %0 = load i32*, i32** %arrayidx, align 8
  %tobool = icmp eq i32* %0, null
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx4 = getelementptr inbounds i32, i32* %0, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx4, align 4
  %arrayidx6 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  store i32 %1, i32* %arrayidx6, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc, %entry
  ret void
}


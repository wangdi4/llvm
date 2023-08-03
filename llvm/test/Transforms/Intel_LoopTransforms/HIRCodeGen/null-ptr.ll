; RUN: opt -passes="hir-ssa-deconstruction,hir-cg" < %s -force-hir-cg -S | FileCheck %s

; Check that CG correctly handles the pointer comparison with null pointer.
; CHECK: region.0:
; CHECK: hir.cmp.{{[0-9]+}}
; CHECK-SAME: icmp ne ptr
; CHECK-SAME: null


; ModuleID = 'null-ptr.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr nocapture readonly %A, ptr nocapture %B, i32 %n) {
entry:
  %cmp.14 = icmp sgt i32 %n, 0
  br i1 %cmp.14, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds ptr, ptr %A, i64 %indvars.iv
  %0 = load ptr, ptr %arrayidx, align 8
  %tobool = icmp eq ptr %0, null
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx4 = getelementptr inbounds i32, ptr %0, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx4, align 4
  %arrayidx6 = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  store i32 %1, ptr %arrayidx6, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; ; Verify that we do not unswitch condition inside inner loop because the
; outer target loop has convergent call.

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, 0, 1   <DO_LOOP>
; CHECK: |   @llvm.nvvm.barrier0();
; CHECK: |
; CHECK: |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   if (%n == 8)
; CHECK: |   |   {
; CHECK: |   |      (%p)[0] = 1;
; CHECK: |   |   }
; CHECK: |   + END LOOP
; CHECK: + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32 %n, ptr %p) {
entry:
  br label %for.body.outer

for.body.outer:
  %i.outer = phi i32 [ 0, %entry ], [ %i.outer.inc, %for.latch.outer ]
  tail call void @llvm.nvvm.barrier0()
  br label %for.body

for.body:
  %i = phi i32 [ 0, %for.body.outer ], [ %ip, %for.inc ]
  %cmp1 = icmp eq i32 %n, 8
  br i1 %cmp1, label %if.then, label %for.inc

if.then:
  store i32 1, ptr %p
  br label %for.inc

for.inc:
  %ip = add nsw i32 %i, 1
  %cmp = icmp slt i32 %i, 99
  br i1 %cmp, label %for.body, label %for.latch.outer

for.latch.outer:
  %i.outer.inc = add nsw i32 %i.outer, 1
  %cmp.outer = icmp slt i32 %i, 99
  br i1 %cmp.outer, label %for.body.outer, label %for.end

for.end:
  ret void
}

declare void @llvm.nvvm.barrier0() #0

attributes #0 = { convergent nounwind }


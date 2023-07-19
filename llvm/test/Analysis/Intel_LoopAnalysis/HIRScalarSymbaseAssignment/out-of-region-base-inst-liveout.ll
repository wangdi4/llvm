; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=scalar-symbase-assignment -debug-only=hir-framework -disable-output < %s 2>&1 | FileCheck %s

; This test was failing because the base inst (%t0) of the liveout single operand phi %t1 is (unexpectedly) coming from outside the region represented by the loop %loop.
; Note that we use base inst %t0 instead of %t1 in HIR.

; In this 'reduced' test case the loop is empty so it is optimized away.

; CHECK: BEGIN REGION


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = external local_unnamed_addr global i32, align 4

; Function Attrs: nounwind uwtable
define void @foo() local_unnamed_addr {
for.body3.thread:
  br label %if.end7.us.us

if.end7.us.us:                                    ; preds = %for.body3.thread
  %t0 = load i32, ptr @x, align 4
  br label %loop

loop:                                   ; preds = %for.cond1.us.us, %if.end7.us.us
  %t3 = phi i32 [ undef, %if.end7.us.us ], [ %t2, %for.cond1.us.us ]
  %inc11.us.us = add nsw i32 %t3, 1
  br label %for.cond1.us.us

for.cond1.us.us:                                  ; preds = %loop
  %t1 = phi i32 [ %t0, %loop ]
  %t2 = phi i32 [ %inc11.us.us, %loop ]
  %cmp2.us.us = icmp sgt i32 undef, %t2
  br i1 %cmp2.us.us, label %loop, label %for.cond1.us.us.for.body3.us.us_crit_edge

for.cond1.us.us.for.body3.us.us_crit_edge:        ; preds = %for.cond1.us.us
  %split = phi i32 [ %t1, %for.cond1.us.us ]
  unreachable

}


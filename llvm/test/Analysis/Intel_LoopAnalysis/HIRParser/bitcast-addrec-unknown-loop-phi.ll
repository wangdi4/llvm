; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" -hir-framework-debug=parser 2>&1 | FileCheck %s

; Verify that unconventional access of %phi.ptr (which has AddRec form)
; is considered non-linear by SSA deconstruction and a liveout copy
; %phi.ptr.out is added which is used to parse AddressOf ref in bottom test.
; Prevously bottom test was incorrectly parsed in terms of %phi.ptr causing
; live-range violation.

; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   do.loop:
; CHECK: |   %phi.ptr.out = &((%phi.ptr)[0]);
; CHECK: |   %val = (%phi.ptr.out)[0].0;
; CHECK: |   %end.ptr = (%end.ptr.ptr)[0];
; CHECK: |   %phi.ptr = &((%phi.ptr.out)[1].1);
; CHECK: |   if (&((%phi.ptr.out)[1].1) != &((%end.ptr)[0]))
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto do.loop;
; CHECK: |   }
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.Segment = type { i32, ptr }

define void @foo(ptr %init.ptr, ptr %end.ptr.ptr) {
entry:
  br label %do.loop

do.loop:                                          ; preds = %do.loop, %entry
  %phi.ptr = phi ptr [ %init.ptr, %entry ], [ %incdec.ptr, %do.loop ]
  %gep = getelementptr inbounds %struct.Segment, ptr %phi.ptr, i64 0, i32 0
  %val = load i32, ptr %gep, align 8
  %incdec.ptr = getelementptr inbounds %struct.Segment, ptr %phi.ptr, i64 1, i32 1
  %end.ptr = load ptr, ptr %end.ptr.ptr, align 8
  %cmp44 = icmp eq ptr %incdec.ptr, %end.ptr
  br i1 %cmp44, label %exit, label %do.loop

exit:
  %lcssa = phi i32 [ %val, %do.loop ]
  ret void
}

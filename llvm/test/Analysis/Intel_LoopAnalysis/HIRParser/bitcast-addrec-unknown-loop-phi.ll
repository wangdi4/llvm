; RUN: opt -opaque-pointers=0 < %s -passes="hir-ssa-deconstruction,print<hir>" -hir-framework-debug=parser 2>&1 | FileCheck %s

; Verify that unconventional bitcast access of %phi.ptr (which has AddRec form)
; is considered non-linear by SSA deconstruction and a liveout copy
; %phi.ptr.out is added which is used to parse AddressOf ref in bottom test.
; Prevously bottom test was incorrectly parsed in terms of %phi.ptr causing
; live-range violation.

; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   do.loop:
; CHECK: |   %phi.ptr.out = &((%phi.ptr)[0]);
; CHECK: |   %val = (%phi.ptr.out)[0].0;
; CHECK: |   %phi.ptr = &((%struct.Segment*)(%phi.ptr.out)[1].1);
; CHECK: |   if (&((%struct.Segment*)(%phi.ptr.out)[1].1) != &((%end.ptr)[0]))
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto do.loop;
; CHECK: |   }
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.Segment = type { i32, %struct.Segment* }

define void @foo(%struct.Segment* %init.ptr, %struct.Segment* %end.ptr) {
entry:
  br label %do.loop

do.loop:                                          ; preds = %do.loop, %entry
  %phi.ptr = phi %struct.Segment* [ %init.ptr, %entry ], [ %bc, %do.loop ]
  %gep = getelementptr inbounds %struct.Segment, %struct.Segment* %phi.ptr, i64 0, i32 0
  %val = load i32, i32* %gep, align 8
  %incdec.ptr = getelementptr inbounds %struct.Segment, %struct.Segment* %phi.ptr, i64 1, i32 1
  %bc = bitcast %struct.Segment** %incdec.ptr to %struct.Segment*
  %cmp44 = icmp eq %struct.Segment* %bc, %end.ptr
  br i1 %cmp44, label %exit, label %do.loop

exit:
  %lcssa = phi i32 [ %val, %do.loop ]
  ret void
}

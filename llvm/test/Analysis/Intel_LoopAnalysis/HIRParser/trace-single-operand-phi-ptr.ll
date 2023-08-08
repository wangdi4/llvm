; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" -xmain-opt-level=3 -hir-framework-debug=parser 2>&1 | FileCheck %s

; Verify that %incdec.ptr is parsed as linear in the loop bottom test by
; tracing through single operand phi %J.ptr.1.

; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   do.loop:
; CHECK: |   %J.ptr158.out = &((%J.ptr158)[0]);
; CHECK: |   %incdec.ptr159.out = &((%init.ptr)[i1]);
; CHECK: |   %ptr.val = (%ptr)[0];
; CHECK: |   %t67 = (%J.ptr158.out)[1].1.0.0;
; CHECK: |   %t68 = inttoptr.i64.ptr(8 * (%t67 /u 8));
; CHECK: |   %t69 = (%t68)[0];
; CHECK: |   %or.i.i = trunc.i64.i2((%t67 /u 2))  |  %t69;
; CHECK: |   %t72 = inttoptr.i64.ptr(8 * (%ptr.val /u 8));
; CHECK: |   %t73 = (%t72)[0];
; CHECK: |   %or.i8.i = %t73  |  trunc.i64.i2((%ptr.val /u 2));
; CHECK: |   if (%or.i.i >=u %or.i8.i)
; CHECK: |   {
; CHECK: |      goto while.cond.loopexit;
; CHECK: |   }
; CHECK: |   %J.ptr158 = &((%incdec.ptr159)[0]);
; CHECK: |   %incdec.ptr159 = &((%incdec.ptr159)[1]);
; CHECK: |   if (&((%init.ptr)[i1 + 1]) != &((%end.ptr)[0]))
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto do.loop;
; CHECK: |   }
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.Segment = type { %struct.SlotIndex, %struct.SlotIndex }
%struct.SlotIndex = type { %struct.PointerIntPair }
%struct.PointerIntPair = type { i64 }

define void @foo(ptr %init.ptr, ptr %end.ptr, ptr %J.ptr, i64* %ptr) {
entry:
  br label %do.loop

do.loop:                                          ; preds = %latch, %entry
  %incdec.ptr159 = phi ptr [ %init.ptr, %entry ], [ %incdec.ptr, %latch ]
  %J.ptr158 = phi ptr [ %J.ptr, %entry ], [ %J.ptr.1, %latch ]
  %ptr.val = load i64, i64* %ptr, align 8
  %Value..i = getelementptr inbounds %struct.Segment, ptr %J.ptr158, i64 1, i32 1, i32 0, i32 0
  %t67 = load i64, i64* %Value..i, align 8
  %and..i.i = and i64 %t67, -8
  %t68 = inttoptr i64 %and..i.i to ptr
  %t69 = load i32, ptr %t68, align 8
  %t70 = lshr i64 %t67, 1
  %t71 = trunc i64 %t70 to i32
  %conv..i = and i32 %t71, 3
  %or.i.i = or i32 %conv..i, %t69
  %and..i5.i = and i64 %ptr.val, -8
  %t72 = inttoptr i64 %and..i5.i to ptr
  %t73 = load i32, ptr %t72, align 8
  %t74 = lshr i64 %ptr.val, 1
  %t75 = trunc i64 %t74 to i32
  %conv.7.i = and i32 %t75, 3
  %or.i8.i = or i32 %t73, %conv.7.i
  %cmp.i = icmp ult i32 %or.i.i, %or.i8.i
  br i1 %cmp.i, label %latch, label %while.cond.loopexit

latch:                                          ; preds = %do.loop
  %J.ptr.1 = phi ptr [ %incdec.ptr159, %do.loop ]
  %incdec.ptr = getelementptr inbounds %struct.Segment, ptr %J.ptr.1, i64 1
  %cmp44 = icmp eq ptr %incdec.ptr, %end.ptr
  br i1 %cmp44, label %exit, label %do.loop

while.cond.loopexit:                              ; preds = %do.loop
  %incdec.ptr.lcssa154 = phi ptr [ %incdec.ptr159, %do.loop ]
  ret void

exit:
  ret void
}

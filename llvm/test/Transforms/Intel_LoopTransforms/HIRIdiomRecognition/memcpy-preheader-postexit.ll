; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-idiom" -print-before=hir-idiom -print-after=hir-idiom -disable-output < %s 2>&1 | FileCheck %s

; Verify that we can generate memcpy in the preheader for the first copy inst
; and in the postexit for the second copy inst. The result is an empty loop
; which is optimized away.

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   (%r)[i1] = (%p)[i1];
; CHECK: |   (%p)[i1] = (%q)[i1];
; CHECK: + END LOOP


; CHECK: BEGIN REGION { modified }

; CHECK: if (%n >u 12)
; CHECK: {
; CHECK:    @llvm.memcpy.p0.p0.i32(&((i8*)(%r)[0]),  &((i8*)(%p)[0]),  4 * %n,  0);
; CHECK:    @llvm.memcpy.p0.p0.i32(&((i8*)(%p)[0]),  &((i8*)(%q)[0]),  4 * %n,  0);
; CHECK: }
; CHECK: else
; CHECK: {
; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 12>  <LEGAL_MAX_TC = 12> <max_trip_count = 12>
; CHECK: |   (%r)[i1] = (%p)[i1];
; CHECK: |   (%p)[i1] = (%q)[i1];
; CHECK: + END LOOP
; CHECK: }



;Module Before HIR; ModuleID = 'memcpy.c'
source_filename = "memcpy.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr nocapture %p, ptr noalias nocapture readonly %q, ptr noalias %r, i32 %n)  {
entry:
  %tobool2 = icmp eq i32 %n, 0
  br i1 %tobool2, label %while.end, label %while.body.preheader

while.body.preheader:                             ; preds = %entry
  br label %while.body

while.body:                                       ; preds = %while.body.preheader, %while.body
  %n.addr.05 = phi i32 [ %dec, %while.body ], [ %n, %while.body.preheader ]
  %q.addr.04 = phi ptr [ %incdec.ptr, %while.body ], [ %q, %while.body.preheader ]
  %p.addr.03 = phi ptr [ %incdec.ptr1, %while.body ], [ %p, %while.body.preheader ]
  %r.addr.03 = phi ptr [ %incdec.ptr2, %while.body ], [ %r, %while.body.preheader ]
  %incdec.ptr1 = getelementptr inbounds i32, ptr %p.addr.03, i64 1
  %pld = load i32, ptr %p.addr.03, align 4
  %incdec.ptr2 = getelementptr inbounds i32, ptr %r.addr.03, i64 1
  store i32 %pld, ptr %r.addr.03, align 4
  %dec = add nsw i32 %n.addr.05, -1
  %incdec.ptr = getelementptr inbounds i32, ptr %q.addr.04, i64 1
  %0 = load i32, ptr %q.addr.04, align 4
  store i32 %0, ptr %p.addr.03, align 4
  %tobool = icmp eq i32 %dec, 0
  br i1 %tobool, label %while.end.loopexit, label %while.body

while.end.loopexit:                               ; preds = %while.body
  br label %while.end

while.end:                                        ; preds = %while.end.loopexit, %entry
  ret void
}


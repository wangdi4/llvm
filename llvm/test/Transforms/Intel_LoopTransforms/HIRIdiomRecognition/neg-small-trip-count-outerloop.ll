; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-idiom" -print-before=hir-idiom -print-after=hir-idiom -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that multiversioning is not done since the candidate
; is not the innermost loop.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>  <LEGAL_MAX_TC = 4294967295>
;       |   (%r)[i1] = (%p)[i1];
;       |   (%p)[i1] = (%q)[i1];
;       |
;       |   + DO i2 = 0, %n, 1   <DO_LOOP>
;       |   |   (%ret)[0] = i2;
;       |   + END LOOP
;       + END LOOP
; END REGION

; HIR after transformation:

; CHECK: BEGIN REGION { modified }
; CHECK-NEXT:          @llvm.memcpy.p0.p0.i32(&((i8*)(%r)[0]),  &((i8*)(%p)[0]),  4 * %n,  0);
; CHECK-NEXT:          @llvm.memcpy.p0.p0.i32(&((i8*)(%p)[0]),  &((i8*)(%q)[0]),  4 * %n,  0);
; CHECK-NEXT:       + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>  <LEGAL_MAX_TC = 4294967295>
; CHECK-NEXT:       |   + DO i2 = 0, %n, 1   <DO_LOOP>
; CHECK-NEXT:       |   |   (%ret)[0] = i2;
; CHECK-NEXT:       |   + END LOOP
; CHECK-NEXT:       + END LOOP
; CHECK-NEXT: END REGION

;Module Before HIR; ModuleID = 'memcpy.c'
source_filename = "memcpy.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr nocapture %p, ptr noalias nocapture readonly %q, ptr noalias %r, i32 %n)  {
entry:
  %tobool2 = icmp eq i32 %n, 0
  %ret = alloca i32
  br i1 %tobool2, label %outer.end, label %outer.body.preheader

outer.body.preheader:                             ; preds = %entry
  br label %outer.body

outer.body:                                       ; preds = %outer.body.preheader, %outer.body
  %n.addr.05 = phi i32 [ %dec, %outer.inc ], [ %n, %outer.body.preheader ]
  %q.addr.04 = phi ptr [ %incdec.ptr, %outer.inc], [ %q, %outer.body.preheader ]
  %p.addr.03 = phi ptr [ %incdec.ptr1, %outer.inc ], [ %p, %outer.body.preheader ]
  %r.addr.03 = phi ptr [ %incdec.ptr2, %outer.inc ], [ %r, %outer.body.preheader ]
  %pld = load i32, ptr %p.addr.03, align 4
  store i32 %pld, ptr %r.addr.03, align 4
  %0 = load i32, ptr %q.addr.04, align 4
  store i32 %0, ptr %p.addr.03, align 4
  br label %inner.body.preheader

inner.body.preheader:                             ; preds = %outer.body
  br label %inner.body

inner.body:                                       ; preds = %outer.body.preheader, %outer.body
  %iv.index = phi i32 [ %iv.next, %inner.body ], [ 0, %inner.body.preheader ]
  %iv.next = add i32 %iv.index, 1
  store i32 %iv.index, ptr %ret, align 4
  %exit.inner = icmp eq i32 %iv.index, %n
  br i1 %exit.inner, label %outer.inc, label %inner.body

outer.inc:
  %incdec.ptr1 = getelementptr inbounds i32, ptr %p.addr.03, i64 1
  %incdec.ptr2 = getelementptr inbounds i32, ptr %r.addr.03, i64 1
  %dec = add nsw i32 %n.addr.05, -1
  %incdec.ptr = getelementptr inbounds i32, ptr %q.addr.04, i64 1
  %tobool = icmp eq i32 %dec, 0
  br i1 %tobool, label %outer.end.loopexit, label %outer.body

outer.end.loopexit:                               ; preds = %outer.body
  br label %outer.end

outer.end:                                        ; preds = %outer.end.loopexit, %entry
  ret void
}

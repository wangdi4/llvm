; RUN: opt -passes="hir-ssa-deconstruction,hir-cg" < %s -force-hir-cg -S | FileCheck %s

;          BEGIN REGION { }
;<11>         + DO i1 = 0, (-1 * %p + %q + -4)/u4, 1   <DO_LOOP>
;<4>          |   (%p)[2 * i1] = i1;
;<11>         + END LOOP
;          END REGION
;The interesting part is the UB, it contains CE with blobs which are pointers
;but with integers added/multiplied. This means blobs for ptrs do not match
;src CE type

;In order to do the arithmetic, ptrs must become ints
; CHECK: [[Q_TO_INT:%.*]] = ptrtoint {{.*}} %q to i64
; CHECK: [[P_TO_INT:%.*]] = ptrtoint {{.*}} %p to i64

; -1 * (int)p
; CHECK: [[P_MUL:%.*]] = sub i64 0, [[P_TO_INT]]

; add (int)q
; CHECK: [[P_PLUS_Q:%.*]] = add i64 [[P_MUL]], [[Q_TO_INT]]

;decrement by 4 and divide by 4 to get (-1 * %p + %q + -4)/u4 for UB
; CHECK: [[PTR_MINUS_4:%.*]] = add i64 [[P_PLUS_Q]], -4
; CHECK: [[UB:%.*]] = udiv i64 [[PTR_MINUS_4]], 4

; Check wrap flags on IV
; CHECK: [[IV_UPDATE:%nextiv.*]] = add nuw nsw i64 {{%.*}}, 1


; ModuleID = 'ptr-iv.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr %p, ptr readnone %q) {
entry:
  %cmp.6 = icmp eq ptr %p, %q
  br i1 %cmp.6, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %p.addr.07 = phi ptr [ %incdec.ptr, %for.body ], [ %p, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, ptr %p.addr.07, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, ptr %arrayidx, align 4
  %incdec.ptr = getelementptr inbounds i32, ptr %p.addr.07, i64 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp eq ptr %incdec.ptr, %q
  br i1 %cmp, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

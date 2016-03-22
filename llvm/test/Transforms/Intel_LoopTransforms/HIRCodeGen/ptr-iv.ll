; RUN: opt < %s -loop-simplify -hir-ssa-deconstruction -hir-cg -force-hir-cg -S | FileCheck %s

;          BEGIN REGION { }
;<11>         + DO i1 = 0, (-1 * %p + %q + -4)/u4, 1   <DO_LOOP>
;<4>          |   (%p)[2 * i1] = i1;
;<11>         + END LOOP
;          END REGION
;The interesting part is the UB, it contains CE with blobs which are pointers
;but with integers added/multiplied. This means blobs for ptrs do not match 
;src CE type

;In order to do the arithmetic, ptrs must become ints
; CHECK: [[P_TO_INT:%.*]] = ptrtoint i32* %p to i64

; -1 * (int)p
; CHECK: [[P_MUL:%.*]] = mul i64 -1, [[P_TO_INT]]

; add (int)q
; CHECK: [[Q_TO_INT:%.*]] = ptrtoint i32* %q to i64
; CHECK: [[P_PLUS_Q:%.*]] = add i64 [[P_MUL]], [[Q_TO_INT]]

;decrement by 4 and divide by 4 to get (-1 * %p + %q + -4)/u4 for UB
; CHECK: [[PTR_MINUS_4:%.*]] = add i64 [[P_PLUS_Q]], -4
; CHECK: [[UB:%.*]] = udiv i64 [[PTR_MINUS_4]], 4

; ensure UB is used in loop cmp
; CHECK: icmp sle i64 %nextivloop{{.*}}, [[UB]]
; ModuleID = 'ptr-iv.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* %p, i32* readnone %q) {
entry:
  %cmp.6 = icmp eq i32* %p, %q
  br i1 %cmp.6, label %for.end, label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
  %p.addr.07 = phi i32* [ %incdec.ptr, %for.body ], [ %p, %entry ]
  %arrayidx = getelementptr inbounds i32, i32* %p.addr.07, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4
  %incdec.ptr = getelementptr inbounds i32, i32* %p.addr.07, i64 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp eq i32* %incdec.ptr, %q
  br i1 %cmp, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}


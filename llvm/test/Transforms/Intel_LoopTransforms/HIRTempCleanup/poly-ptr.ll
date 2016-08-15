; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -print-before=hir-temp-cleanup -print-after=hir-temp-cleanup 2>&1 | FileCheck %s

; Verify that we can eliminate the liveout copy.

; CHECK: Dump Before HIR Temp Cleanup

; CHECK: + DO i1 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; CHECK: |   %p.addr.07.out = &((%p.addr.07)[0]);
; CHECK: |   (%p.addr.07.out)[0] = i1;
; CHECK: |   %p.addr.07 = &((%p.addr.07)[i1]);
; CHECK: + END LOOP

; CHECK: Dump After HIR Temp Cleanup 

; CHECK: + DO i1 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; CHECK: |   (%p.addr.07)[0] = i1;
; CHECK: |   %p.addr.07 = &((%p.addr.07)[i1]);
; CHECK: + END LOOP


; ModuleID = 'poly-ptr.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture %p, i32 %n) {
entry:
  %cmp.6 = icmp sgt i32 %n, 0
  br i1 %cmp.6, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %p.addr.07 = phi i32* [ %add.ptr, %for.body ], [ %p, %for.body.preheader ]
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %p.addr.07, align 4
  %add.ptr = getelementptr inbounds i32, i32* %p.addr.07, i64 %indvars.iv
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

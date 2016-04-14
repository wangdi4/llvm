; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; This command checks that -hir-ssa-deconstruction invalidates SCEV so that the parser doesn't pick up the cached version. HIR output should be the same as for the above command.
; RUN: opt < %s -hir-ssa-deconstruction -hir-complete-unroll -print-before=hir-complete-unroll 2>&1 | FileCheck %s

; Check parsing output for the loop
; CHECK: DO i1 = 0, zext.i32.i64((-1 + %n))
; CHECK-SAME: DO_LOOP
; CHECK-NEXT: %c.addr.08.out = %c.addr.08
; CHECK-NEXT: %a.addr.010.out = %a.addr.010
; CHECK-NEXT: (%A)[i1] = %a.addr.010
; CHECK-NEXT: %a.addr.010 = %b.addr.07
; CHECK-NEXT: %c.addr.08 = %a.addr.010.out
; CHECK-NEXT: %b.addr.07 = %c.addr.08.out
; CHECK-NEXT: END LOOP


; ModuleID = 'circularwrap.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture %A, i32 %n, i32 %a, i32 %b, i32 %c) {
entry:
  %cmp.6 = icmp sgt i32 %n, 0
  br i1 %cmp.6, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %a.addr.010 = phi i32 [ %b.addr.07, %for.body ], [ %a, %for.body.preheader ]
  %c.addr.08 = phi i32 [ %a.addr.010, %for.body ], [ %c, %for.body.preheader ]
  %b.addr.07 = phi i32 [ %c.addr.08, %for.body ], [ %b, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  store i32 %a.addr.010, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

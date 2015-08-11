; RUN: opt < %s -loop-simplify -hir-de-ssa | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop
; CHECK: DO i1 = 0, %n + -1
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

; Function Attrs: nounwind uwtable
define void @foo(i32* nocapture %A, i32 %n, i32 %a, i32 %b, i32 %c) {
entry:
  %cmp.6 = icmp sgt i32 %n, 0
  br i1 %cmp.6, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
  %a.addr.010 = phi i32 [ %b.addr.07, %for.body ], [ %a, %entry ]
  %c.addr.08 = phi i32 [ %a.addr.010, %for.body ], [ %c, %entry ]
  %b.addr.07 = phi i32 [ %c.addr.08, %for.body ], [ %b, %entry ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  store i32 %a.addr.010, i32* %arrayidx, align 4 
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}


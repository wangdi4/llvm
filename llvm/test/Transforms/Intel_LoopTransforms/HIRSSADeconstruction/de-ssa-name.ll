; RUN: opt %s -passes="hir-ssa-deconstruction" -S | FileCheck %s

; Check that an unnamed value's (%0 here) copy is provided an appropriate name.
; CHECK: %0
; CHECK-NEXT: hir.de.ssa.copy0.out
; CHECK-SAME: out.de.ssa
; CHECK: output.1.in1
; CHECK-SAME: in.de.ssa
; CHECK: live.range.de.ssa
; CHECK: output.1.in
; CHECK-SAME: in.de.ssa
; CHECK: output.1
; CHECK-SAME: in.de.ssa


; ModuleID = 'de_ssa1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(ptr nocapture %A, ptr nocapture %B, i32 %a, i32 %b, i32 %n) {
entry:
  %cmp13 = icmp sgt i32 %n, 0
  br i1 %cmp13, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %if.end
  %indvars.iv = phi i64 [ %indvars.iv.next, %if.end ], [ 0, %for.body.preheader ]
  %0 = phi i32 [ %a.addr.1, %if.end ], [ %a, %for.body.preheader ]
  %cmp1 = icmp sgt i64 %indvars.iv, 77
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %inc = add nsw i32 %0, 1
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  store i32 %inc, ptr %arrayidx, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %a.addr.1 = phi i32 [ %inc, %if.then ], [ %0, %for.body ]
  %output.1 = phi i32 [ %0, %if.then ], [ %b, %for.body ]
  %arrayidx3 = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  store i32 %output.1, ptr %arrayidx3, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %if.end
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %output.0.lcssa = phi i32 [ -1, %entry ], [ %output.1, %for.end.loopexit ]
  ret i32 %output.0.lcssa
}

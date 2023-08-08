; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-details -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Check parsing output for the loop verifying that phi region liveout value(p.addr.013) is handled correctly by insertion of liveout copy.

; CHECK: LiveOuts
; CHECK-DAG: p.addr.013.out
; CHECK-DAG: incdec.ptr

; CHECK: HasSignedIV: Yes

; CHECK: DO i32 i1 = 0, %n + -1
; CHECK: %p.addr.013.out = &((%p)[i1])
; CHECK: (%p)[i1] = i1 + 5
; CHECK: %incdec.ptr = &((%p)[i1 + 1])

; Verify that the pointer lval is parsed as self-blob.
; CHECK-NEXT: NON-LINEAR ptr %incdec.ptr
; CHECK: (%p)[i1 + 1] = i1 + 10
; CHECK: END LOOP


; ModuleID = 'multiple-liveouts.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @foo(ptr nocapture %p, i32 %n) {
entry:
  %cmp.12 = icmp sgt i32 %n, 0
  br i1 %cmp.12, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %i.014 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %p.addr.013 = phi ptr [ %p, %for.body.lr.ph ], [ %incdec.ptr, %for.body ]
  %add = add nsw i32 %i.014, 5
  store i32 %add, ptr %p.addr.013, align 4
  %incdec.ptr = getelementptr inbounds i32, ptr %p.addr.013, i64 1
  %add1 = add nsw i32 %i.014, 10
  store i32 %add1, ptr %incdec.ptr, align 4
  %inc = add nsw i32 %i.014, 1
  %cmp = icmp slt i32 %inc, %n
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.body
  %incdec.ptr.lcssa = phi ptr [ %incdec.ptr, %for.body ]
  %p.addr.013.lcssa = phi ptr [ %p.addr.013, %for.body ]
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  %q.0.lcssa = phi ptr [ %p.addr.013.lcssa, %for.cond.for.end_crit_edge ], [ %p, %entry ]
  %p.addr.0.lcssa = phi ptr [ %incdec.ptr.lcssa, %for.cond.for.end_crit_edge ], [ %p, %entry ]
  %0 = load i32, ptr %p.addr.0.lcssa, align 4
  %1 = load i32, ptr %q.0.lcssa, align 4
  %add2 = add nsw i32 %1, %0
  ret i32 %add2
}


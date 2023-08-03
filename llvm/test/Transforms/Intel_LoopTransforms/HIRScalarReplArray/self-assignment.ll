; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-independent-scalar-repl,print<hir>" -disable-output 2>&1 | FileCheck %s

; Verify that we are able to successfully compile this self-assignment test
; case and do not modifiy the HIR as it is not profitable.

; Previously, this test case was compfailing due to a bug in the code which
; fails to find max index load for the group.


; CHECK-NOT modified

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   (%ptr)[i1] = (%ptr)[i1];
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


define void @foo(ptr %ptr) {
entry:
  br label %for.body

for.body:                                         
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ptr.iv = phi ptr [ %ptr, %entry ], [ %incdec.ptr, %for.body ]
  %ld = load i32, ptr %ptr.iv, align 4
  %arrayidx = getelementptr inbounds i32, ptr %ptr, i64 %indvars.iv
  store i32 %ld, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %incdec.ptr = getelementptr inbounds i32, ptr %ptr.iv, i64 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %exit, label %for.body

exit:                                         
  ret void
}


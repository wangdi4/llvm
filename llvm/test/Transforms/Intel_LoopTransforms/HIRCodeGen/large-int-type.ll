; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-cg" < %s -force-hir-cg -S 2>&1 | FileCheck %s

; Verify that HIR is formed for loop containing integer types than 64 bits and
; we succesfully generate code for it.

; CHECK: + DO i1 = 0, 63, 1   <DO_LOOP>
; CHECK: |   %sext = sext.i92.i128(%t2);
; CHECK: |   %add = %t1  +  %sext;
; CHECK: |   (%A)[i1] = %add;
; CHECK: + END LOOP


; CHECK: region.0:
; CHECK: store i64 0, i64* %i1.i64


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i128* %A, i128 %t1, i92 %t2) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %sext = sext i92 %t2 to i128
  %add = add nsw i128 %t1, %sext
  %arrayidx2 = getelementptr inbounds i128, i128* %A, i64 %indvars.iv
  store i128 %add, i128* %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 64
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void
}


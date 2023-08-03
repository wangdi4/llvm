; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-independent-scalar-repl" -print-before=hir-loop-independent-scalar-repl -print-after=hir-loop-independent-scalar-repl -disable-output 2>&1 | FileCheck %s
; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-independent-scalar-repl" -print-changed -disable-output 2>&1 | FileCheck %s --check-prefix=CHECK-CHANGED

; Check that we are able to trigger loop independant scalar replacement on the
; two groups (%A)[i1] refs and (%A)[i1 + 1] refs separately.

; The (%A)[i1] group starts with a store so the load is eliminated.
; The (%A)[i1 + 1] starts with a load so everything except the first load and
; final store is eliminated.

; CHECK: Dump Before 

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   (%A)[i1] = 1;
; CHECK: |   %0 = (%A)[i1];
; CHECK: |   %1 = (%A)[i1 + 1];
; CHECK: |   (%A)[i1 + 1] = 5;
; CHECK: |   %2 = (%A)[i1 + 1];
; CHECK: |   (%A)[i1 + 1] = 10;
; CHECK: |   %add2 = %0 + %1  +  %2;
; CHECK: + END LOOP

; CHECK: Dump After

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   (%A)[i1] = 1;
; CHECK: |   %scalarepl2 = (%A)[i1 + 1];
; CHECK: |   %1 = %scalarepl2;
; CHECK: |   (%A)[i1 + 1] = 10;
; CHECK: |   %add2 = %1 + 1  +  5;
; CHECK: + END LOOP

; Verify that pass is dumped with print-changed when it triggers.

; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED: Dump After HIRLoopIndependentScalarRepl


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr nocapture noundef readonly %A) {
entry:
  br label %for.body3

for.body3:                                        ; preds = %entry, %for.body3
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  store i32 1, ptr %arrayidx, align 4
  %0 = load i32, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx5 = getelementptr inbounds i32, ptr %A, i64 %indvars.iv.next
  %1 = load i32, ptr %arrayidx5, align 4
  store i32 5, ptr %arrayidx5, align 4
  %2 = load i32, ptr %arrayidx5, align 4
  store i32 10, ptr %arrayidx5, align 4
  %add1 = add nsw i32 %0, %1
  %add2 = add nsw i32 %add1, %2
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %end, label %for.body3

end:                                        ; preds = %end
  %add2.lcssa = phi i32 [ %add2, %for.body3 ]
  ret void
}


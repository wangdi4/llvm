; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-safe-reduction-analysis>,hir-loop-reversal" -aa-pipeline="basic-aa" -print-before=hir-loop-reversal -print-after=hir-loop-reversal %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-reversal" -print-changed -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK-CHANGED
;
; Verify that loop is not reversed because the safe reduction has unsafe algebra.
;
; Check output of safe reduction analysis-
;
; CHECK: Has Unsafe Algebra- Yes


; CHECK: Dump Before
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 10, 1   <DO_LOOP>
; CHECK:        |   %add = %s2.019  +  (%A)[-1 * i1 + sext.i32.i64((2 * %n))];
; CHECK:        |   %s2.019 = %add  +  (%A)[-1 * i1 + sext.i32.i64(%n)];
; CHECK:        + END LOOP
; CHECK:  END REGION


; CHECK: Dump After

; CHECK-NOT: modified


; Verify that pass is not dumped with print-changed if it bails out.


; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED-NOT: Dump After HIRLoopReversal


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define float @foo(ptr noalias nocapture readonly %A, i32 %n) {
entry:
  %mul = shl nsw i32 %n, 1
  %0 = sext i32 %n to i64
  %1 = sext i32 %mul to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %2 = load float, ptr %A, align 4
  %add5 = fadd float %add4, %2
  ret float %add5

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %s2.019 = phi float [ 0.000000e+00, %entry ], [ %add4, %for.body ]
  %3 = sub nsw i64 %1, %indvars.iv
  %arrayidx = getelementptr inbounds float, ptr %A, i64 %3
  %4 = load float, ptr %arrayidx, align 4
  %add = fadd float %s2.019, %4
  %5 = sub nsw i64 %0, %indvars.iv
  %arrayidx3 = getelementptr inbounds float, ptr %A, i64 %5
  %6 = load float, ptr %arrayidx3, align 4
  %add4 = fadd float %add, %6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 11
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}


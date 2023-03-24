; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-general-unroll" -debug-only=hir-general-unroll < %s 2>&1 | FileCheck %s

; Verify that even though the loop contains expensive fdiv inst, the cost of the
; loop is 1 because we only count number of operations.

; CHECK: + DO i1 = 0, 49, 1   <DO_LOOP>
; CHECK: |   %phi = %phi  /  2.000000e+00;
; CHECK: + END LOOP

; CHECK: Computed loop body cost of unroll candidate to be: 1

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


define float @_Z3fool(float %init) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %phi = phi float [ %init, %entry ], [ %fd, %for.body ]
  %i.09 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %fd = fdiv float %phi, 2.00
  %inc = add nuw nsw i64 %i.09, 1
  %exitcond = icmp eq i64 %inc, 50
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  %fd.lcssa = phi float [ %fd, %for.body ]
  ret float %fd.lcssa
}


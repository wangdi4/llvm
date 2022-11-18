; RUN: opt %s -S -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec --debug-only=LoopVectorizationPlanner_vec_lengths 2>&1 | FileCheck %s
; REQUIRES: asserts
;
; Checks if the code with #pragma vector vectorlength(1) (!{!"llvm.loop.vector.vectorlength", i64 1} metadata)
; was not vectorized with HIR.
;
; void foo(int *arr)
; {
;   int i;
;
; #pragma vector vectorlength(1)
;   for (i = 0; i < 1024; i++)
;     arr[i] = i;
; }
;
; CHECK: LVP: Specified vectorlengths: 1{{[[:space:]]}}
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* %arr) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %arr, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !6

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { nofree uwtable mustprogress }

!6 = distinct !{!6, !7, !8}
!7 = !{!"llvm.loop.mustprogress"}
!8 = !{!"llvm.loop.intel.vector.vectorlength", i64 1}

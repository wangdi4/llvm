; RUN: opt %s -S -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec' --debug-only=LoopVectorizationPlanner_vec_lengths 2>&1 | FileCheck %s
; RUN: opt %s -S -passes=hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter -disable-output -intel-opt-report=medium 2>&1 | FileCheck %s --check-prefix=OPTRPTMED
; REQUIRES: asserts
;
; Checks if the code with #pragma vector vectorlength(0) (!{!"llvm.loop.vector.vectorlength", i64 0} metadata)
; was not vectorized with HIR.
;
; void foo(int *arr)
; {
;   int i;
;
; #pragma vector vectorlength(0)
;   for (i = 0; i < 1024; i++)
;     arr[i] = i;
; }
;
; CHECK: LVP: Specified vectorlengths: 0{{[[:space:]]}}
;
; OPTRPTMED: remark #15436: loop was not vectorized: User specified #pragma vector vectorlength(0).
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr %arr) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %arr, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !6

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { nofree uwtable mustprogress }

!6 = distinct !{!6, !7, !8}
!7 = !{!"llvm.loop.mustprogress"}
!8 = !{!"llvm.loop.intel.vector.vectorlength", i64 0}

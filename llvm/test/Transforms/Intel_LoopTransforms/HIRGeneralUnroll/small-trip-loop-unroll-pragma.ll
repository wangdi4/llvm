; RUN: opt -passes="hir-ssa-deconstruction,hir-general-unroll,print<hir>" -S < %s 2>&1 | FileCheck %s

; Verify that we are successfully able to handle small trip count loops with unroll pragma.

; HIR-
; + DO i1 = 0, 6, 1   <DO_LOOP> <unroll>
; |   (%A)[i1] = i1;
; + END LOOP

; CHECK: DO i1 = 0, 2

; Check that remainder loop with trip count of 1 was completely unrolled. 
; CHECK-NOT: DO i1

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @foo(ptr nocapture %A) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 7
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !6

for.end:                                          ; preds = %for.body
  ret void
}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.unroll.enable"}

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-scalarrepl-array,print<hir>" -hir-details-dims -disable-output < %s 2>&1 | FileCheck %s

; Verify that the group with two (%A)[i1] refs which have mismatched
; base ptr element type (i32 vs float) is skipped by scalar replacement.

; CHECK: + DO i1 = 0, 100, 1   <DO_LOOP>
; CHECK: |   (%B)[0:i1:4(i32:0)] = (%A)[0:i1:4(i32:0)];
; CHECK: |   %add = (%A)[0:i1:4(float:0)]  +  5.000000e+00;
; CHECK: |   (%C)[0:i1:4(float:0)] = %add;
; CHECK: + END LOOP

define void @foo(ptr noalias %A, ptr noalias %B, ptr noalias %C) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %ld1 = load i32, ptr %arrayidx, align 4
  %arrayidxB = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  store i32 %ld1, ptr %arrayidxB, align 4
  %arrayidx2 = getelementptr inbounds float, ptr %A, i64 %indvars.iv
  %ld2 = load float, ptr %arrayidx2, align 4
  %add = fadd float %ld2, 5.0
  %arrayidxC = getelementptr inbounds float, ptr %C, i64 %indvars.iv
  store float %add, ptr %arrayidxC, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 101
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void
}



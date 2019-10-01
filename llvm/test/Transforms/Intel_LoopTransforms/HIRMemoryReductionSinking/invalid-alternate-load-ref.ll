; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-memory-reduction-sinking -print-after=hir-memory-reduction-sinking < %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-memory-reduction-sinking,print<hir-framework>" 2>&1 < %s | FileCheck %s

; Verify that we do not process reduction as it is not invariant.

; CHECK-NOT: modified

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   %mul59 = %mul52  *  (%t13)[i1];
; CHECK: |   (%t13)[i1] = %mul59;
; CHECK: + END LOOP


define void @foo(float *%t13, float %mul52, i64 %n) {
entry:
  br label %for.body56

for.body56:                                       ; preds = %for.body56, %entry
  %indvars.iv158 = phi i64 [ 0, %entry ], [ %indvars.iv.next159, %for.body56 ]
  %arrayidx58 = getelementptr inbounds float, float* %t13, i64 %indvars.iv158
  %t14 = load float, float* %arrayidx58, align 4
  %mul59 = fmul fast float %mul52, %t14
  store float %mul59, float* %arrayidx58, align 4
  %indvars.iv.next159 = add nuw nsw i64 %indvars.iv158, 1
  %exitcond161 = icmp eq i64 %indvars.iv.next159, %n
  br i1 %exitcond161, label %for.end62.loopexit, label %for.body56

for.end62.loopexit:
  ret void
}

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -mtriple=x86_64-unknown-unknown -mattr=+avx2 -disable-output -vplan-cost-model-print-analysis-for-vf=4 -vplan-cost-model-use-gettype -vplan-force-vf=4 < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -mtriple=x86_64-unknown-unknown -mattr=+avx2 -disable-output -vplan-cost-model-print-analysis-for-vf=4 -vplan-cost-model-use-gettype -vplan-force-vf=4 < %s 2>&1 | FileCheck %s

;
; LIT test to check cost modeling of abs VPInstruction.
;
define dso_local void @foo(i64* noalias nocapture %larr, i64* noalias nocapture %larr2) local_unnamed_addr #0 {
;
; CHECK-LABEL:  Cost Model for VPlan foo:HIR.#{{[0-9]+}} with VF = 4:
; CHECK:        Cost 4 for i1 [[VP4:%.*]] = icmp slt i64 [[VP2:%.*]] i64 10
; CHECK-NEXT:   Cost 2 for i64 [[VP5:%.*]] = select i1 [[VP4]] i64 [[VP3:%.*]] i64 222
; CHECK-NEXT:   Cost 6 for i64 [[VP6:%.*]] = abs i64 [[VP2]]
;
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %l1.08 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds i64, i64* %larr, i64 %l1.08
  %arrayidx2 = getelementptr inbounds i64, i64* %larr2, i64 %l1.08
  %0 = load i64, i64* %arrayidx, align 8
  %val2 = load i64, i64* %arrayidx2, align 8
  %cmp = icmp slt i64 %0, 10
  %select = select i1 %cmp, i64 %val2, i64 222
  %1 = icmp slt i64 %0, 0
  %neg = sub nsw i64 0, %0
  %2 = select i1 %1, i64 %neg, i64 %0
  %add = add i64 %select, %2
  store i64 %add, i64* %arrayidx, align 8
  %inc = add nuw nsw i64 %l1.08, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

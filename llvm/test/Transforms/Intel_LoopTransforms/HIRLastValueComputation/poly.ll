; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation -print-after=hir-last-value-computation -hir-cost-model-throttling=0 < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,print<hir>" -aa-pipeline="basic-aa" -hir-cost-model-throttling=0 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Last Value Computation ***
;
;<0>       BEGIN REGION { }
;<9>             + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;<2>             |   %mul = i1  *  i1;
;<9>             + END LOOP
;<0>       END REGION
;
;*** IR Dump After HIR Last Value Computation ***
;
; CHECK:   BEGIN REGION { modified }
; CHECK:       %mul = %N + -1  *  %N + -1;
; CHECK:   END REGION
;
; ModuleID = 'new.ll'
source_filename = "new.ll"

define dso_local i32 @foo(i32 %N) local_unnamed_addr {
entry:
  %cmp1 = icmp slt i32 0, %N
  br i1 %cmp1, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %i.02 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %mul = mul nsw i32 %i.02, %i.02
  %inc = add nuw nsw i32 %i.02, 1
  %cmp = icmp slt i32 %inc, %N
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.body
  %split = phi i32 [ %mul, %for.body ]
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  %j.0.lcssa = phi i32 [ %split, %for.cond.for.end_crit_edge ], [ 0, %entry ]
  ret i32 %j.0.lcssa
}

;
; RUN: opt -passes="hir-ssa-deconstruction,hir-last-value-computation,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s -disable-output | FileCheck %s
;
; *** IR Dump Before HIR Last Value Computation ***
;
;<0>       BEGIN REGION { }
;<10>            + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;<3>             |   %add = %b * i1  +  1;
;<10>            + END LOOP
;<0>       END REGION
;
; *** IR Dump After HIR Last Value Computation ***
;
; CHECK:   BEGIN REGION { modified }
; CHECK:         %add = -1 * %b + (%N * %b)  +  1;
; CHECK:   END REGION
;
; ModuleID = 't.ll'
source_filename = "t.ll"

define dso_local i32 @foo(i32 %N, i32 %b) local_unnamed_addr {
entry:
  %cmp1 = icmp slt i32 0, %N
  br i1 %cmp1, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %j.02 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %mul = mul nsw i32 %j.02, %b
  %add = add nsw i32 %mul, 1
  %inc = add nuw nsw i32 %j.02, 1
  %cmp = icmp slt i32 %inc, %N
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.body
  %split = phi i32 [ %add, %for.body ]
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  %t.0.lcssa = phi i32 [ %split, %for.cond.for.end_crit_edge ], [ undef, %entry ]
  ret i32 %t.0.lcssa
}

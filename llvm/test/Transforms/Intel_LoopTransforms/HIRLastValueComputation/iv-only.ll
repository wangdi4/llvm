; RUN: opt -passes="hir-ssa-deconstruction,hir-last-value-computation,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
; *** IR Dump Before HIR Last Value Computation ***
;
;<0>       BEGIN REGION { }
;<9>             + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;<2>             |   %j.07.out = i1;
;<9>             + END LOOP
;<0>       END REGION
;
; *** IR Dump After HIR Last Value Computation ***
;
; CHECK:   BEGIN REGION { modified }
; CHECK:         %j.07.out = %N + -1;
; CHECK:   END REGION
;
; Function Attrs: norecurse nounwind readnone uwtable
define dso_local i32 @foo(i32 %N, i32 %b) local_unnamed_addr #0 {
entry:
  %cmp6 = icmp sgt i32 %N, 0
  br i1 %cmp6, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %j.07 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %inc = add nuw nsw i32 %j.07, 1
  %cmp = icmp slt i32 %inc, %N
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.body
  %j.07.lcssa = phi i32 [ %j.07, %for.body ]
  %mul.le = mul nsw i32 %j.07.lcssa, %b
  %add.le = add nsw i32 %mul.le, 1
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  %t.0.lcssa = phi i32 [ %add.le, %for.cond.for.end_crit_edge ], [ undef, %entry ]
  ret i32 %t.0.lcssa
}


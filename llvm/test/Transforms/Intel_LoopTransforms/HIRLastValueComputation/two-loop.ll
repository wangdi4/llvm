; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Last Value Computation ***
;
;<0>       BEGIN REGION { }
;<29>            + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;<30>            |   + DO i2 = 0, %M + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;<11>            |   |   %div = 2 * i2  /  i1;
;<30>            |   + END LOOP
;<19>            |      %t.05 = %add;
;<29>            + END LOOP
;<0>       END REGION
;
;*** IR Dump After HIR Last Value Computation ***
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:        |   if (0 < %M)
; CHECK:        |   {
; CHECK:        |      %div = 2 * %M + -2  /  i1;
; CHECK:        |      %t.05 = %div;
; CHECK:        |   }
; CHECK:        + END LOOP
; CHECK:  END REGION
;
; ModuleID = 'new2.ll'
source_filename = "new2.ll"

define dso_local i32 @foo(ptr nocapture readnone %A, i32 %b, i32 %N, i32 %M) local_unnamed_addr {
entry:
  %cmp3 = icmp slt i32 0, %N
  br i1 %cmp3, label %for.cond1.preheader.lr.ph, label %for.end6

for.cond1.preheader.lr.ph:                        ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.inc4
  %t.05 = phi i32 [ undef, %for.cond1.preheader.lr.ph ], [ %t.1.lcssa, %for.inc4 ]
  %i.04 = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %inc5, %for.inc4 ]
  %cmp21 = icmp slt i32 0, %M
  br i1 %cmp21, label %for.body3.lr.ph, label %for.inc4

for.body3.lr.ph:                                  ; preds = %for.cond1.preheader
  br label %for.body3

for.body3:                                        ; preds = %for.body3.lr.ph, %for.body3
  %j.02 = phi i32 [ 0, %for.body3.lr.ph ], [ %inc, %for.body3 ]
  %mul = mul nuw nsw i32 %j.02, 2
  %div = sdiv i32 %mul, %i.04
  %inc = add nuw nsw i32 %j.02, 1
  %cmp2 = icmp slt i32 %inc, %M
  br i1 %cmp2, label %for.body3, label %for.cond1.for.inc4_crit_edge

for.cond1.for.inc4_crit_edge:                     ; preds = %for.body3
  %split = phi i32 [ %div, %for.body3 ]
  br label %for.inc4

for.inc4:                                         ; preds = %for.cond1.for.inc4_crit_edge, %for.cond1.preheader
  %t.1.lcssa = phi i32 [ %split, %for.cond1.for.inc4_crit_edge ], [ %t.05, %for.cond1.preheader ]
  %inc5 = add nuw nsw i32 %i.04, 1
  %cmp = icmp slt i32 %inc5, %N
  br i1 %cmp, label %for.cond1.preheader, label %for.cond.for.end6_crit_edge

for.cond.for.end6_crit_edge:                      ; preds = %for.inc4
  %split6 = phi i32 [ %t.1.lcssa, %for.inc4 ]
  br label %for.end6

for.end6:                                         ; preds = %for.cond.for.end6_crit_edge, %entry
  %t.0.lcssa = phi i32 [ %split6, %for.cond.for.end6_crit_edge ], [ undef, %entry ]
  ret i32 %t.0.lcssa
}

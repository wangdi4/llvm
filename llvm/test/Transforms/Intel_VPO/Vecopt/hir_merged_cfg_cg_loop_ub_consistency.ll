; Test for basic functionality of HIR vectorizer CG for merged CFG where loop UB is
; defined in parent loop.

; Input HIR
; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   %0 = (%lpp)[i1];
;       |   %1 = (%lp)[i1];
;       |   %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;       |
;       |   + DO i2 = 0, %1 + -1, 1   <DO_LOOP>
;       |   |   (%0)[i2] = i2;
;       |   + END LOOP
;       |
;       |   @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
;       + END LOOP
; END REGION

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-enable-new-cfg-merge-hir -vplan-vec-scenario="n0;v4;s1" -print-after=hir-vplan-vec -hir-details -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-enable-new-cfg-merge-hir -vplan-vec-scenario="n0;v4;s1" -hir-details -disable-output < %s 2>&1 | FileCheck %s

; CHECK-LABEL: Function: _Z3fooPlPS_
; CHECK:    BEGIN REGION { modified }
; CHECK:          + DO i64 i1 = 0, 99, 1   <DO_LOOP>
; CHECK:          |   %0 = (%lpp)[i1];
; CHECK:          |   %1 = (%lp)[i1];
; CHECK:          |   <LVAL-REG> NON-LINEAR i64 %1 {sb:9}
; CHECK:          |   <RVAL-REG> {al:8}(LINEAR i64* %lp)[LINEAR i64 i1] inbounds  {sb:17}
; CHECK:          |      <BLOB> LINEAR i64* %lp {sb:8}
; CHECK:          |   if (%1 > 0)
; CHECK:          |   {
; CHECK:          |      %tgu = %1  /u  4;
; CHECK:          |      <LVAL-REG> NON-LINEAR i64 %tgu {sb:20}
; CHECK:          |      <RVAL-REG> NON-LINEAR i64 %1 {sb:9}
; CHECK:          |      %vec.tc = %tgu  *  4;
; CHECK:          |      %.vec = 0 == %vec.tc;
; CHECK:          |      %phi.temp = 0;
; CHECK:          |      %extract.0. = extractelement %.vec,  0;
; CHECK:          |      if (%extract.0. == 1)
; CHECK:          |      {
; CHECK:          |         goto [[MERGE_AFTER_MAIN:.*]];
; CHECK:          |      }
; CHECK:          |      %tgu2 = %1  /u  4;
; CHECK:          |      <LVAL-REG> NON-LINEAR i64 %tgu2 {sb:25}
; CHECK:          |      <RVAL-REG> NON-LINEAR i64 %1 {sb:9}
; CHECK:          |      %vec.tc3 = %tgu2  *  4;
; CHECK:          |
; CHECK:          |      + DO i64 i2 = 0, %vec.tc3 + -1, 4   <DO_LOOP> <auto-vectorized> <nounroll> <novectorize>
; CHECK:          |      |   (<4 x i64>*)(%0)[i2] = i2 + <i64 0, i64 1, i64 2, i64 3>;
; CHECK:          |      + END LOOP
; CHECK:          |
; CHECK:          |      %.vec4 = %1 == %vec.tc3;
; CHECK:          |      <LVAL-REG> NON-LINEAR <4 x i1> %.vec4 {sb:29}
; CHECK:          |      <RVAL-REG> NON-LINEAR <4 x i64> %1 {sb:9}
; CHECK:          |      <RVAL-REG> NON-LINEAR <4 x i64> %vec.tc3 {sb:26}
; CHECK:          |      %phi.temp = %vec.tc3;
; CHECK:          |      %phi.temp6 = %vec.tc3;
; CHECK:          |      %extract.0.8 = extractelement %.vec4,  0;
; CHECK:          |      if (%extract.0.8 == 1)
; CHECK:          |      {
; CHECK:          |         goto [[FINAL_MERGE:.*]];
; CHECK:          |      }
; CHECK:          |      [[MERGE_AFTER_MAIN]]:
; CHECK:          |      %lb.tmp = %phi.temp;
; CHECK:          |
; CHECK:          |      + DO i64 i2 = %lb.tmp, %1 + -1, 1   <DO_LOOP>
; CHECK:          |      |   (%0)[i2] = i2;
; CHECK:          |      + END LOOP
; CHECK:          |
; CHECK:          |      %phi.temp6 = %1 + -1;
; CHECK:          |      <LVAL-REG> NON-LINEAR i64 %phi.temp6 {sb:30}
; CHECK:          |      <RVAL-REG> NON-LINEAR i64 %1 + -1 {sb:2}
; CHECK:          |         <BLOB> NON-LINEAR i64 %1 {sb:9}
; CHECK:          |      [[FINAL_MERGE]]:
; CHECK:          |   }
; CHECK:          + END LOOP
; CHECK:    END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
define dso_local void @_Z3fooPlPS_(i64* nocapture noundef readonly %lp, i64** nocapture noundef readonly %lpp) local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.end
  %l1.021 = phi i64 [ 0, %entry ], [ %inc7, %for.end ]
  %arrayidx = getelementptr inbounds i64*, i64** %lpp, i64 %l1.021
  %0 = load i64*, i64** %arrayidx, align 8
  %arrayidx1 = getelementptr inbounds i64, i64* %lp, i64 %l1.021
  %1 = load i64, i64* %arrayidx1, align 8
  %cmp319 = icmp sgt i64 %1, 0
  br i1 %cmp319, label %for.body4.preheader, label %for.end

for.body4.preheader:                              ; preds = %for.body
  br label %for.body4

for.body4:                                        ; preds = %for.body4.preheader, %for.body4
  %l2.020 = phi i64 [ %inc, %for.body4 ], [ 0, %for.body4.preheader ]
  %arrayidx5 = getelementptr inbounds i64, i64* %0, i64 %l2.020
  store i64 %l2.020, i64* %arrayidx5, align 8
  %inc = add nuw nsw i64 %l2.020, 1
  %exitcond.not = icmp eq i64 %inc, %1
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body4

for.end.loopexit:                                 ; preds = %for.body4
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %for.body
  %inc7 = add nuw nsw i64 %l1.021, 1
  %exitcond22.not = icmp eq i64 %inc7, 100
  br i1 %exitcond22.not, label %for.end8, label %for.body

for.end8:                                         ; preds = %for.end
  ret void
}

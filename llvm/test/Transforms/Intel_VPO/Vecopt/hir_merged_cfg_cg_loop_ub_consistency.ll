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
; CHECK:        + DO i64 i1 = 0, 99, 1   <DO_LOOP>
; CHECK:        |   [[TMP0:%.*]] = ([[LPP0:%.*]])[i1]
; CHECK:        |   [[TMP1:%.*]] = ([[LP0:%.*]])[i1]
; CHECK:        |   <LVAL-REG> NON-LINEAR i64 [[TMP1]] {sb:9}
; CHECK:        |   <RVAL-REG> {al:8}(LINEAR i64* [[LP0]])[LINEAR i64 i1] inbounds  {sb:17}
; CHECK:        |      <BLOB> LINEAR i64* [[LP0]] {sb:8}
; CHECK:        |   if ([[TMP1]] > 0)
; CHECK:        |   {
; CHECK:        |      [[TGU0:%.*]] = [[TMP1]]  /u  4
; CHECK:        |      <LVAL-REG> NON-LINEAR i64 [[TGU0]] {sb:20}
; CHECK:        |      <RVAL-REG> NON-LINEAR i64 [[TMP1]] {sb:9}
; CHECK:        |
; CHECK:        |      [[VEC_TC0:%.*]] = [[TGU0]]  *  4
; CHECK:        |      [[DOTVEC0:%.*]] = 0 == [[VEC_TC0]]
; CHECK:        |      [[PHI_TEMP0:%.*]] = 0
; CHECK:        |      [[EXTRACT_0_0:%.*]] = extractelement [[DOTVEC0]],  0
; CHECK:        |      if ([[EXTRACT_0_0]] == 1)
; CHECK:        |      {
; CHECK:        |         goto [[MERGE_AFTER_MAIN:.*]];
; CHECK:        |      }
; CHECK:        |      [[TGU20:%.*]] = [[TMP1]]  /u  4
; CHECK:        |      <LVAL-REG> NON-LINEAR i64 [[TGU20]] {sb:25}
; CHECK:        |      <RVAL-REG> NON-LINEAR i64 [[TMP1]] {sb:9}
; CHECK:        |      [[VEC_TC30:%.*]] = [[TGU20]]  *  4
; CHECK:        |
; CHECK:        |      [[LOOP_UB0:%.*]] = [[VEC_TC30]]  -  1
; CHECK:        |      <LVAL-REG> NON-LINEAR i64 [[LOOP_UB0]] {sb:27}
; CHECK:        |      <RVAL-REG> NON-LINEAR i64 [[VEC_TC30]] {sb:26}
; CHECK:        |      + DO i64 i2 = 0, [[LOOP_UB0]], 4   <DO_LOOP> <auto-vectorized> <nounroll> <novectorize>
; CHECK:        |      |   (<4 x i64>*)([[TMP0]])[i2] = i2 + <i64 0, i64 1, i64 2, i64 3>
; CHECK:        |      + END LOOP
; CHECK:        |
; CHECK:        |      [[IND_FINAL0:%.*]] = 0 + [[VEC_TC30]]
; CHECK:        |      [[DOTVEC40:%.*]] = [[TMP1]] == [[VEC_TC30]]
; CHECK:        |      <LVAL-REG> NON-LINEAR <4 x i1> [[DOTVEC40]] {sb:31}
; CHECK:        |      <RVAL-REG> NON-LINEAR <4 x i64> [[TMP1]] {sb:9}
; CHECK:        |      <RVAL-REG> NON-LINEAR <4 x i64> [[VEC_TC30]] {sb:26}
; CHECK:        |      [[PHI_TEMP0]] = [[IND_FINAL0]]
; CHECK:        |      [[PHI_TEMP60:%.*]] = [[IND_FINAL0]]
; CHECK:        |      [[EXTRACT_0_80:%.*]] = extractelement [[DOTVEC40]],  0
; CHECK:        |      if ([[EXTRACT_0_80]] == 1)
; CHECK:        |      {
; CHECK:        |         goto [[FINAL_MERGE:.*]];
; CHECK:        |      }
; CHECK:        |      [[MERGE_AFTER_MAIN]]:
; CHECK:        |      [[LB_TMP0:%.*]] = [[PHI_TEMP0]]
; CHECK:        |      + DO i64 i2 = [[LB_TMP0]], [[TMP1]] + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>  <LEGAL_MAX_TC = 3> <nounroll> <novectorize> <max_trip_count = 3>
; CHECK:        |      |   ([[TMP0]])[i2] = i2
; CHECK:        |      + END LOOP
; CHECK:        |
; CHECK:        |      [[PHI_TEMP60]] = [[TMP1]] + -1
; CHECK:        |      <LVAL-REG> NON-LINEAR i64 [[PHI_TEMP60]] {sb:32}
; CHECK:        |      <RVAL-REG> NON-LINEAR i64 [[TMP1]] + -1 {sb:2}
; CHECK:        |         <BLOB> NON-LINEAR i64 [[TMP1]] {sb:9}
; CHECK:        |      [[FINAL_MERGE]]:
; CHECK:        |   }
; CHECK:        + END LOOP
; CHECK:  END REGION

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

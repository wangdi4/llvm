; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=4 -hir-details-no-verbose-indent -vplan-enable-masked-vectorized-remainder=0 -vplan-enable-non-masked-vectorized-remainder=0 < %s 2>&1 | FileCheck %s
;
; LIT test to check dynamic peeling in VPlan HIR path for the case a[i] = a[i] + 1.
;
; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:        [[DOTVEC0:%.*]] = ptrtoint.<4 x i64*>.<4 x i64>(&((<4 x i64*>)([[LP0:%.*]])[0]))
; CHECK-NEXT:        [[DOTVEC10:%.*]] = [[DOTVEC0]]  /u  8
; CHECK-NEXT:        [[DOTVEC20:%.*]] = [[DOTVEC10]]  *  3
; CHECK-NEXT:        [[DOTVEC30:%.*]] = [[DOTVEC20]]  [[U0:%.*]]  4
; CHECK-NEXT:        [[DOTVEC40:%.*]] = 0 == [[DOTVEC30]]
; CHECK-NEXT:        [[PHI_TEMP0:%.*]] = 0
; CHECK-NEXT:        [[EXTRACT_0_0:%.*]] = extractelement [[DOTVEC40]],  0
; CHECK-NEXT:        if ([[EXTRACT_0_0]] == 1)
; CHECK-NEXT:        {
; CHECK-NEXT:           goto [[MERGE_AFTER_PEEL:.*]];
; CHECK-NEXT:        }
; CHECK-NEXT:        [[DOTVEC50:%.*]] = [[DOTVEC30]] + 4 >u 400
; CHECK-NEXT:        [[PHI_TEMP60:%.*]] = 0
; CHECK-NEXT:        [[EXTRACT_0_80:%.*]] = extractelement [[DOTVEC50]],  0
; CHECK-NEXT:        if ([[EXTRACT_0_80]] == 1)
; CHECK-NEXT:        {
; CHECK-NEXT:           goto [[MERGE_AFTER_MAIN:.*]];
; CHECK-NEXT:        }
; CHECK-NEXT:        [[EXTRACT_0_90:%.*]] = extractelement [[DOTVEC30]],  0
; CHECK-NEXT:        [[UB_TMP0:%.*]] = [[EXTRACT_0_90]]
; CHECK-NEXT:        [[PEEL_UB0:%.*]] = [[UB_TMP0]]  -  1

; CHECK:             + DO i1 = 0, [[PEEL_UB0]], 1   <DO_LOOP>  <MAX_TC_EST = 3>  <LEGAL_MAX_TC = 3> <vector-peel> <nounroll> <novectorize> <max_trip_count = 3>
; CHECK-NEXT:        |   [[VAL0:%.*]] = ([[LP0]])[i1]
; CHECK-NEXT:        |   ([[LP0]])[i1] = [[VAL0]] + 1
; CHECK-NEXT:        + END LOOP

; CHECK:             [[PHI_TEMP0]] = [[UB_TMP0]]
; CHECK-NEXT:        [[MERGE_AFTER_PEEL]]:
; CHECK-NEXT:        [[DOTVEC110:%.*]] = [[DOTVEC30]] + 4 >u 400
; CHECK-NEXT:        [[PHI_TEMP60]] = [[PHI_TEMP0]]
; CHECK-NEXT:        [[EXTRACT_0_130:%.*]] = extractelement [[DOTVEC110]],  0
; CHECK-NEXT:        if ([[EXTRACT_0_130]] == 1)
; CHECK-NEXT:        {
; CHECK-NEXT:           goto [[MERGE_AFTER_MAIN]]
; CHECK-NEXT:        }
; CHECK-NEXT:        [[EXTRACT_0_140:%.*]] = extractelement [[DOTVEC30]],  0
; CHECK-NEXT:        [[ADJ_TC0:%.*]] = 400  -  [[EXTRACT_0_140]]
; CHECK-NEXT:        [[TGU0:%.*]] = [[ADJ_TC0]]  /u  4
; CHECK-NEXT:        [[VEC_TC0:%.*]] = [[TGU0]]  *  4
; CHECK-NEXT:        [[EXTRACT_0_150:%.*]] = extractelement [[DOTVEC30]],  0
; CHECK-NEXT:        [[ADJ_TC160:%.*]] = [[VEC_TC0]]  +  [[EXTRACT_0_150]]
; CHECK-NEXT:        [[TMP0:%.*]] = [[PHI_TEMP0]]  +  <i64 0, i64 1, i64 2, i64 3>
; CHECK-NEXT:        [[LOOP_UB0:%.*]] = [[ADJ_TC160]]  -  1

; CHECK:             + DO i1 = [[PHI_TEMP0]], [[LOOP_UB0]], 4   <DO_LOOP> <auto-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:        |   [[DOTVEC170:%.*]] = (<4 x i64>*)([[LP0]])[i1]
; CHECK-NEXT:        |   (<4 x i64>*)([[LP0]])[i1] = [[DOTVEC170]] + 1
; CHECK-NEXT:        + END LOOP

; CHECK:             [[IND_FINAL0:%.*]] = 0 + [[ADJ_TC160]]
; CHECK-NEXT:        [[DOTVEC180:%.*]] = 400 == [[ADJ_TC160]]
; CHECK-NEXT:        [[PHI_TEMP60]] = [[IND_FINAL0]]
; CHECK-NEXT:        [[PHI_TEMP200:%.*]] = [[IND_FINAL0]]
; CHECK-NEXT:        [[EXTRACT_0_220:%.*]] = extractelement [[DOTVEC180]],  0
; CHECK-NEXT:        if ([[EXTRACT_0_220]] == 1)
; CHECK-NEXT:        {
; CHECK-NEXT:           goto [[FINAL_MERGE:.*]];
; CHECK-NEXT:        }
; CHECK-NEXT:        [[MERGE_AFTER_MAIN]]:
; CHECK-NEXT:        [[LB_TMP0:%.*]] = [[PHI_TEMP60]]

; CHECK:             + DO i1 = [[LB_TMP0]], 399, 1   <DO_LOOP>  <MAX_TC_EST = 3>  <LEGAL_MAX_TC = 3> <vector-remainder> <nounroll> <novectorize> <max_trip_count = 3>
; CHECK-NEXT:        |   [[VAL0]] = ([[LP0]])[i1]
; CHECK-NEXT:        |   ([[LP0]])[i1] = [[VAL0]] + 1
; CHECK-NEXT:        + END LOOP
; CHECK:             [[PHI_TEMP200]] = 399
; CHECK-NEXT:        [[FINAL_MERGE]]:
; CHECK-NEXT:  END REGION
define void @foo(i64* %lp) {
entry:
  br label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %l1.06 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i64, i64* %lp, i64 %l1.06
  %val = load i64, i64* %arrayidx, align 8
  %add = add i64 %val, 1
  store i64 %add, i64* %arrayidx, align 8
  %inc = add nuw nsw i64 %l1.06, 1
  %exitcond.not = icmp eq i64 %inc, 400
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !0

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.intel.vector.dynamic_align", !"true"}

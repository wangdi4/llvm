;
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=8 -vplan-force-uf=4 < %s 2>&1 | FileCheck %s --check-prefix=VF8UF4-CHECK
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=8 -vplan-force-uf=5 < %s 2>&1 | FileCheck %s --check-prefix=VF8UF5-CHECK
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=2 -vplan-force-uf=1 -vplan-enable-masked-main-loop=0 < %s 2>&1 | FileCheck %s --check-prefix=VF2UF1-CHECK
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=8 -vplan-force-uf=1 -vplan-enable-masked-main-loop=0 < %s 2>&1 | FileCheck %s --check-prefix=VF8UF1-CHECK
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=8 -vplan-force-uf=1 -vplan-enable-masked-main-loop=1 -vplan-masked-main-cost-threshold=0 < %s 2>&1 | FileCheck %s --check-prefix=VF8UF1e-CHECK
;
; LIT test to check that VPlan vectorizer honors MAX_TC_ESTIMATE from HIR when
; vectorizing with or without unroll. Loopopt passes rely on array indices being
; in range and accesses that appear out-of-range can cause compile time issues even
; if the loop is guarded with a proper zero trip count check.
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr6 = dso_local global [6 x i64] zeroinitializer, align 4
@arr32 = dso_local global [32 x i64] zeroinitializer, align 4

; Incoming HIR:
;             + DO i1 = 0, %n1 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 32>
;             |   (@arr32)[0][i1] = i1;
;             + END LOOP
;
; VF8UF4-CHECK:        + DO i1 = 0, %loop.ub, 32   <DO_LOOP>  <MAX_TC_EST = 1> <auto-vectorized> <nounroll> <novectorize>
; VF8UF4-CHECK-NEXT:   |   (<8 x i64>*)(@arr32)[0][i1] = i1 + <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>;
; VF8UF4-CHECK-NEXT:   |   (<8 x i64>*)(@arr32)[0][i1 + 8] = i1 + <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7> + 8;
; VF8UF4-CHECK-NEXT:   |   (<8 x i64>*)(@arr32)[0][i1 + 16] = i1 + <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7> + 16;
; VF8UF4-CHECK-NEXT:   |   (<8 x i64>*)(@arr32)[0][i1 + 24] = i1 + <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7> + 24;
; VF8UF4-CHECK-NEXT:   + END LOOP
;
; VF8UF5-CHECK:        + DO i1 = 0, %n1 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 32>
; VF8UF5-CHECK-NEXT:   |   (@arr32)[0][i1] = i1;
; VF8UF5-CHECK-NEXT:   + END LOOP
;
define void @foo(i64 %n1) {
entry:
  %cmp = icmp sgt i64 %n1, 0
  br i1 %cmp, label %for.ph, label %exit

for.ph:
  br label %for.body

for.body:                                         ;
  %iv = phi i64 [ 0, %for.ph ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds [32 x i64], ptr @arr32, i64 0, i64 %iv
  store i64 %iv, ptr %arrayidx, align 8
  %inc = add nuw nsw i64 %iv, 1
  %exitcond.not = icmp eq i64 %inc, %n1
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:
  br label %exit

exit:                                          ;
  ret void
}

; Incoming HIR:
;             + DO i1 = 0, %n1 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 6>
;             |   (@arr6)[0][i1] = i1;
;             + END LOOP
;
; VF2UF1-CHECK:        + DO i1 = 0, %loop.ub, 2   <DO_LOOP>  <MAX_TC_EST = 3> <auto-vectorized> <nounroll> <novectorize>
; VF2UF1-CHECK-NEXT:   |   (<2 x i64>*)(@arr6)[0][i1] = i1 + <i64 0, i64 1>;
; VF2UF1-CHECK-NEXT:   + END LOOP
;
; VF8UF1-CHECK:        + DO i1 = 0, %n1 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 6>
; VF8UF1-CHECK-NEXT:   |   (@arr6)[0][i1] = i1;
; VF8UF1-CHECK-NEXT:   + END LOOP
;
; VF8UF1e-CHECK-LABEL:  Function: baz
; VF8UF1e-CHECK:       BEGIN REGION { modified }
; VF8UF1e-CHECK-NEXT:        [[DOTVEC0:%.*]] = [[N10:%.*]]  -  0
; VF8UF1e-CHECK-NEXT:        [[EXTRACT_0_0:%.*]] = extractelement [[DOTVEC0]],  0
; VF8UF1e-CHECK-NEXT:        [[LOOP_UB0:%.*]] = [[EXTRACT_0_0]]  -  1
; VF8UF1e-CHECK:             + DO i1 = 0, [[LOOP_UB0]], 8   <DO_LOOP> <auto-vectorized> <nounroll> <novectorize>
; VF8UF1e-CHECK-NEXT:        |   [[DOTVEC10:%.*]] = i1 + <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7> <u [[DOTVEC0]]
; VF8UF1e-CHECK-NEXT:        |   (<8 x i64>*)(@arr6)[0][i1] = i1 + <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>, Mask = @{[[DOTVEC10]]}
; VF8UF1e-CHECK-NEXT:        |   [[DOTVEC20:%.*]] = i1 + <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7> + 8 <u [[DOTVEC0]]
; VF8UF1e-CHECK-NEXT:        |   [[TMP0:%.*]] = bitcast.<8 x i1>.i8([[DOTVEC20]])
; VF8UF1e-CHECK-NEXT:        |   [[CMP30:%.*]] = [[TMP0]] == 0
; VF8UF1e-CHECK-NEXT:        |   [[ALL_ZERO_CHECK0:%.*]] = [[CMP30]]
; VF8UF1e-CHECK-NEXT:        + END LOOP
; VF8UF1e-CHECK:             [[IND_FINAL0:%.*]] = 0  +  [[N10]]
; VF8UF1e-CHECK-NEXT:  END REGION
;
define void @baz(i64 %n1) {
entry:
  %cmp = icmp sgt i64 %n1, 0
  br i1 %cmp, label %for.ph, label %exit

for.ph:
  br label %for.body

for.body:                                         ;
  %iv = phi i64 [ 0, %for.ph ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds [6 x i64], ptr @arr6, i64 0, i64 %iv
  store i64 %iv, ptr %arrayidx, align 8
  %inc = add nuw nsw i64 %iv, 1
  %exitcond.not = icmp eq i64 %inc, %n1
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:
  br label %exit

exit:                                          ;
  ret void
}

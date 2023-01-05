; Test for basic functionality of HIR vectorizer CG for merged CFG.
; Vec Scenario:
;    - No peel
;    - Main vector loop with VF=4
;    - Scalar remainder

; Input HIR
; BEGIN REGION { }
;       %tok = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]
;       %sum.07 = %init;
;
;       + DO i1 = 0, %N + -1, 1   <DO_LOOP> <simd>
;       |   %A.i = (%A)[i1];
;       |   %sum.07 = %A.i  +  %sum.07;
;       + END LOOP
;
;       @llvm.directive.region.exit(%tok); [ DIR.OMP.END.SIMD() ]
; END REGION

; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' -vplan-vec-scenario="n0;v4;s1" -disable-output < %s 2>&1 | FileCheck %s

; CHECK-LABEL: Function: foo
; CHECK:          BEGIN REGION { modified }
; CHECK-NEXT:        [[SUM_070:%.*]] = [[INIT0:%.*]]
; CHECK-NEXT:        [[TGU0:%.*]] = [[N0:%.*]]  /u  4
; CHECK-NEXT:        [[VEC_TC0:%.*]] = [[TGU0]]  *  4
; CHECK-NEXT:        [[DOTVEC0:%.*]] = 0 == [[VEC_TC0]]
; CHECK-NEXT:        [[PHI_TEMP0:%.*]] = [[SUM_070]]
; CHECK-NEXT:        [[PHI_TEMP20:%.*]] = 0
; CHECK-NEXT:        [[EXTRACT_0_0:%.*]] = extractelement [[DOTVEC0]],  0
; CHECK-NEXT:        if ([[EXTRACT_0_0]] == 1)
; CHECK-NEXT:        {
; CHECK-NEXT:           goto [[MERGE_AFTER_MAIN:.*]];
; CHECK-NEXT:        }
; CHECK-NEXT:        [[TGU40:%.*]] = [[N0]]  /u  4
; CHECK-NEXT:        [[VEC_TC50:%.*]] = [[TGU40]]  *  4
; CHECK-NEXT:        [[RED_INIT0:%.*]] = 0
; CHECK-NEXT:        [[RED_INIT_INSERT0:%.*]] = insertelement [[RED_INIT0]],  [[SUM_070]],  0
; CHECK-NEXT:        [[PHI_TEMP60:%.*]] = [[RED_INIT_INSERT0]]
; CHECK-NEXT:        [[LOOP_UB0:%.*]] = [[VEC_TC50]]  -  1

; CHECK:             + DO i1 = 0, [[LOOP_UB0]], 4   <DO_LOOP> <simd-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:        |   [[DOTVEC80:%.*]] = (<4 x i32>*)([[A0:%.*]])[i1]
; CHECK-NEXT:        |   [[DOTVEC90:%.*]] = [[DOTVEC80]]  +  [[PHI_TEMP60]]
; CHECK-NEXT:        |   [[PHI_TEMP60]] = [[DOTVEC90]]
; CHECK-NEXT:        + END LOOP

; CHECK:             [[SUM_070]] = @llvm.vector.reduce.add.v4i32([[DOTVEC90]])
; CHECK:             [[IND_FINAL0:%.*]] = 0 + [[VEC_TC50]]
; CHECK-NEXT:        [[DOTVEC110:%.*]] = [[N0]] == [[VEC_TC50]]
; CHECK-NEXT:        [[PHI_TEMP0]] = [[SUM_070]]
; CHECK-NEXT:        [[PHI_TEMP20]] = [[IND_FINAL0]]
; CHECK-NEXT:        [[PHI_TEMP140:%.*]] = [[SUM_070]]
; CHECK-NEXT:        [[PHI_TEMP160:%.*]] = [[IND_FINAL0]]
; CHECK-NEXT:        [[EXTRACT_0_180:%.*]] = extractelement [[DOTVEC110]],  0
; CHECK-NEXT:        if ([[EXTRACT_0_180]] == 1)
; CHECK-NEXT:        {
; CHECK-NEXT:           goto [[FINAL_MERGE:.*]];
; CHECK-NEXT:        }
; CHECK-NEXT:        [[MERGE_AFTER_MAIN]]:
; CHECK-NEXT:        [[LB_TMP0:%.*]] = [[PHI_TEMP20]]
; CHECK-NEXT:        [[SUM_070]] = [[PHI_TEMP0]]

; CHECK:             + DO i1 = [[LB_TMP0]], [[N0]] + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>  <LEGAL_MAX_TC = 3> <vector-remainder> <nounroll> <novectorize> <max_trip_count = 3>
; CHECK-NEXT:        |   [[A_I0:%.*]] = ([[A0]])[i1]
; CHECK-NEXT:        |   [[SUM_070]] = [[A_I0]]  +  [[SUM_070]]
; CHECK-NEXT:        + END LOOP

; CHECK:             [[PHI_TEMP140]] = [[SUM_070]]
; CHECK-NEXT:        [[PHI_TEMP160]] = [[N0]] + -1
; CHECK-NEXT:        [[FINAL_MERGE]]:
; CHECK-NEXT:  END REGION


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(i32* nocapture readonly %A, i64 %N, i32 %init) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                           ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
  %sum.07 = phi i32 [ %add, %for.body ], [ %init, %entry ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %A.i = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %A.i, %sum.07
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %N
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body

for.cond.cleanup.loopexit:                             ; preds = %for.body
  %add.lcssa = phi i32 [ %add, %for.body ]
  br label %end.simd

end.simd:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  ret i32 %add.lcssa

}
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

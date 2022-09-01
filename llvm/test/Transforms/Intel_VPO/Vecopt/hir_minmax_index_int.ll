; RUN: opt %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -disable-output 2>&1 | FileCheck %s

define void @foo() {
; CHECK:       BEGIN REGION { modified }

; CHECK:                [[RED_INIT0:%red.init*]] = [[TMP1:%.*]];
; CHECK-NEXT:           [[RED_INIT10:%.*]] = [[RETVAL_SROA_2_0_COPYLOAD_SROA_SPECULATE_LOAD_FALSE0:%.*]];
; CHECK-NEXT:           [[PHI_TEMP0:%.*]] = [[RED_INIT10]]
; CHECK-NEXT:           [[PHI_TEMP20:%.*]] = [[RED_INIT0]]

; CHECK:                + DO i1 = 0, {{.*}}, 4   <DO_LOOP> <auto-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:           |   [[DOTVEC0:%.*]] = (<4 x i32>*)([[TMP0:%.*]])[i1]
; CHECK-NEXT:           |   [[DOTVEC40:%.*]] = ([[PHI_TEMP20]] > [[DOTVEC0]]) ? i1 + <i64 0, i64 1, i64 2, i64 3> : [[PHI_TEMP0]]
; CHECK-NEXT:           |   [[DOTVEC50:%.*]] = ([[PHI_TEMP20]] > [[DOTVEC0]]) ? [[DOTVEC0]] : [[PHI_TEMP20]]
; CHECK-NEXT:           |   [[PHI_TEMP0]] = [[DOTVEC40]]
; CHECK-NEXT:           |   [[PHI_TEMP20]] = [[DOTVEC50]]
; CHECK-NEXT:           + END LOOP

; CHECK:                [[TMP1]] = @llvm.vector.reduce.smin.v4i32([[DOTVEC50]])
; CHECK-NEXT:           [[IDX_BLEND0:%.*]] = ([[TMP1]] == [[DOTVEC50]]) ? [[DOTVEC40]] : <i64 9223372036854775807, i64 9223372036854775807, i64 9223372036854775807, i64 9223372036854775807>
; CHECK-NEXT:           [[RETVAL_SROA_2_0_COPYLOAD_SROA_SPECULATE_LOAD_FALSE0]] = @llvm.vector.reduce.smin.v4i64([[IDX_BLEND0]])
; CHECK:             + DO i1 = {{.*}}, [[SPEC_SELECT3090:%.*]] + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>  <LEGAL_MAX_TC = 3> <vector-remainder> <nounroll> <novectorize> <max_trip_count = 3>
; CHECK-NEXT:        |   [[TMP2:%.*]] = ([[TMP0]])[i1]
; CHECK-NEXT:        |   [[RETVAL_SROA_2_0_COPYLOAD_SROA_SPECULATE_LOAD_FALSE0]] = ([[TMP1]] > [[TMP2]]) ? i1 : [[RETVAL_SROA_2_0_COPYLOAD_SROA_SPECULATE_LOAD_FALSE0]]
; CHECK-NEXT:        |   [[TMP1]] = ([[TMP1]] > [[TMP2]]) ? [[TMP2]] : [[TMP1]]
; CHECK-NEXT:        + END LOOP
; CHECK:       END REGION
;
entry:
  %0 = load i32*, i32** undef
  br label %for.body.preheader

for.body.preheader:
  %spec.select309 = select i1 undef, i64 undef, i64 0
  br label %for.body

for.body:
  %retval.sroa.2.0.copyload.sroa.speculate.load.false = phi i64 [ %retval.sroa.2.0.copyload.sroa.speculated, %for.body ], [ -1, %for.body.preheader ]
  %1 = phi i32 [ %retval.sroa.0.0.copyload.sroa.speculated, %for.body ], [ 7, %for.body.preheader ]
  %storemerge7 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %0, i64 %storemerge7
  %2 = load i32, i32* %arrayidx
  %cmp = icmp sgt i32 %1, %2
  %retval.sroa.0.0.copyload.sroa.speculated = select i1 %cmp, i32 %2, i32 %1
  %retval.sroa.2.0.copyload.sroa.speculated = select i1 %cmp, i64 %storemerge7, i64 %retval.sroa.2.0.copyload.sroa.speculate.load.false
  %inc = add nuw nsw i64 %storemerge7, 1
  %exitcond.not = icmp eq i64 %inc, %spec.select309
  br i1 %exitcond.not, label %invoke.cont5.loopexit, label %for.body

invoke.cont5.loopexit:
  %retval.sroa.2.0.copyload.sroa.speculated.lcssa = phi i64 [ %retval.sroa.2.0.copyload.sroa.speculated, %for.body ]
  ret void

sw.default:
  ret void
}

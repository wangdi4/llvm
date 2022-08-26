; RUN: opt %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -disable-output 2>&1 | FileCheck %s

define void @foo() {
; CHECK:      BEGIN REGION { modified }
; CHECK:               + END LOOP
; CHECK:               [[TMP1:%.*]] = @llvm.vector.reduce.fmin.v4f64(%.[[vec5:.*]]);
; CHECK-NEXT:          %[[idxblend:.*]] = ([[TMP1]] == %.[[vec5]]) ? %.[[vec4:.*]] : <i64 -1, i64 -1, i64 -1, i64 -1>;
; CHECK-NEXT:          %retval.sroa.2.0.copyload.sroa.speculate.load.false = @llvm.vector.reduce.umin.v4i64(%[[idxblend]]);
; CHECK:            }
;
; CHECK:            + DO i1 = {{.*}}, %spec.select309 + -1, 1   <DO_LOOP>
; CHECK-NEXT:       |   [[TMP2:%.*]] = (%0)[i1];
; CHECK-NEXT:       |   %retval.sroa.2.0.copyload.sroa.speculate.load.false = ([[TMP1]] > [[TMP2]]) ? i1 : %retval.sroa.2.0.copyload.sroa.speculate.load.false;
; CHECK-NEXT:       |   [[TMP1]] = ([[TMP1]] > [[TMP2]]) ? [[TMP2]] : [[TMP1]];
; CHECK-NEXT:       + END LOOP
; CHECK:      END REGION
;
entry:
  %0 = load double*, double** undef
  br label %for.body.preheader

for.body.preheader:
  %spec.select309 = select i1 undef, i64 undef, i64 0
  br label %for.body

for.body:
  %retval.sroa.2.0.copyload.sroa.speculate.load.false = phi i64 [ %retval.sroa.2.0.copyload.sroa.speculated, %for.body ], [ -1, %for.body.preheader ]
  %1 = phi double [ %retval.sroa.0.0.copyload.sroa.speculated, %for.body ], [ 0x7FEFFFFFFFFFFFFF, %for.body.preheader ]
  %storemerge7 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds double, double* %0, i64 %storemerge7
  %2 = load double, double* %arrayidx
  %cmp = fcmp fast ogt double %1, %2
  %retval.sroa.0.0.copyload.sroa.speculated = select i1 %cmp, double %2, double %1
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

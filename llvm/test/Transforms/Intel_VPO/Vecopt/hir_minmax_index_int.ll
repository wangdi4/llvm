; RUN: opt -vplan-enable-new-cfg-merge-hir=false %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -disable-output 2>&1 | FileCheck %s
; RUN: opt %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -disable-output 2>&1 | FileCheck %s

define void @foo() {
; CHECK:      BEGIN REGION { modified }
; CHECK-NEXT:       %tgu = (%spec.select309)/u4;
; CHECK-NEXT:       if (0 <u 4 * %tgu)
; CHECK-NEXT:       {
; CHECK-NEXT:          %red.init = %1;
; CHECK-NEXT:          %red.init1 = %retval.sroa.2.0.copyload.sroa.speculate.load.false;
; CHECK-NEXT:          %phi.temp = %red.init1;
; CHECK-NEXT:          %phi.temp2 = %red.init;
; CHECK:               + DO i1 = 0, 4 * %tgu + -1, 4   <DO_LOOP> <auto-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:          |   %.vec = (<4 x i32>*)(%0)[i1];
; CHECK-NEXT:          |   %.vec4 = (%phi.temp2 > %.vec) ? i1 + <i64 0, i64 1, i64 2, i64 3> : %phi.temp;
; CHECK-NEXT:          |   %.vec5 = (%phi.temp2 > %.vec) ? %.vec : %phi.temp2;
; CHECK-NEXT:          |   %phi.temp = %.vec4;
; CHECK-NEXT:          |   %phi.temp2 = %.vec5;
; CHECK-NEXT:          + END LOOP
; CHECK:               %1 = @llvm.vector.reduce.smin.v4i32(%.vec5);
; CHECK-NEXT:          %idx.blend = (%1 == %.vec5) ? %.vec4 : <i64 9223372036854775807, i64 9223372036854775807, i64 9223372036854775807, i64 9223372036854775807>;
; CHECK-NEXT:          %retval.sroa.2.0.copyload.sroa.speculate.load.false = @llvm.vector.reduce.smin.v4i64(%idx.blend);
; CHECK-NEXT:       }
;
; CHECK:            + DO i1 = 4 * %tgu, %spec.select309 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>   <LEGAL_MAX_TC = 3> <nounroll> <novectorize> <max_trip_count = 3>
; CHECK-NEXT:       |   %2 = (%0)[i1];
; CHECK-NEXT:       |   %retval.sroa.2.0.copyload.sroa.speculate.load.false = (%1 > %2) ? i1 : %retval.sroa.2.0.copyload.sroa.speculate.load.false;
; CHECK-NEXT:       |   %1 = (%1 > %2) ? %2 : %1;
; CHECK-NEXT:       + END LOOP
; CHECK-NEXT: END REGION
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

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation -print-after=hir-last-value-computation < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation -print-after=hir-last-value-computation -hir-cg -force-hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter < %s 2>&1 | FileCheck %s -check-prefix=OPTREPORT
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-cg,simplifycfg,intel-ir-optreport-emitter" -aa-pipeline="basic-aa" -intel-opt-report=low -force-hir-cg 2>&1 < %s | FileCheck %s -check-prefix=OPTREPORT

;*** IR Dump Before HIR Last Value Computation ***
;Function: foo
;
;<0>          BEGIN REGION { }
;<16>               + DO i1 = 0, %n + -1, 1   <DO_LOOP> <simd>
;<6>                |   %and = %uni1  &  127;
;<16>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Last Value Computation ***
;Function: foo
;
; CHECK:     BEGIN REGION { modified }
; CHECK:           %and = %uni1  &  127;
; CHECK:     END REGION
;
; OPTREPORT: LOOP BEGIN
; OPTREPORT:    remark #25530: Stmt at line 0 sinked after loop using last value computation
; OPTREPORT: LOOP END
define i32 @foo(i32 %uni1, i64 %n) local_unnamed_addr #0 {
omp.inner.for.body.lr.ph:
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %if.end, %omp.inner.for.body.lr.ph
  %.omp.iv.local.010 = phi i64 [ 0, %omp.inner.for.body.lr.ph ], [ %add4, %omp.inner.for.body ]
  %and = and i32 %uni1, 127
  %add4 = add nuw nsw i64 %.omp.iv.local.010, 1
  %exitcond = icmp eq i64 %add4, %n
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %if.end
  %live.out = phi i32 [ %and, %omp.inner.for.body ]
  br label %return

return:
  ret i32 %live.out
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)


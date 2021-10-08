; RUN: opt -S -vplan-vec -vplan-force-vf=2 -vplan-print-after-create-masked-vplan -vplan-enable-masked-variant  -vplan-print-after-vpentity-instrs < %s -disable-output | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: VPlan after insertion of VPEntities instructions
; CHECK-NOT:  VPlan after emitting masked variant:

define void @foo() {
entry:
  %b3.i.lpriv = alloca [12 x i16], align 1
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE"([12 x i16]* %b3.i.lpriv) ]
  br label %omp.inner.for.body.i

omp.inner.for.body.i:                             ; preds = %omp.body.continue.i, %DIR.OMP.SIMD.112
  %.omp.iv.i.local.03 = phi i32 [ %add5.i, %omp.body.continue.i ], [ 0, %DIR.OMP.SIMD.1 ]
  %arrayidx.i = getelementptr inbounds [12 x i16], [12 x i16]* %b3.i.lpriv, i64 0, i64 1
  store i16 1, i16* %arrayidx.i, align 2
  br label %omp.body.continue.i

omp.body.continue.i:                              ; preds = %if.then.i, %omp.inner.for.body.i
  %add5.i = add nuw nsw i32 %.omp.iv.i.local.03, 1
  %exitcond.not = icmp eq i32 %.omp.iv.i.local.03, undef
  br i1 %exitcond.not, label %omp.inner.for.cond.i.DIR.OMP.END.SIMD.5.i.loopexit_crit_edge, label %omp.inner.for.body.i

omp.inner.for.cond.i.DIR.OMP.END.SIMD.5.i.loopexit_crit_edge: ; preds = %omp.body.continue.i
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

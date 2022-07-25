; RUN: opt -S -vplan-vec -vplan-force-vf=2 -vplan-print-after-create-masked-vplan -vplan-enable-masked-variant  -vplan-print-after-vpentity-instrs < %s -disable-output | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: VPlan after insertion of VPEntities instructions
; CHECK-NOT:  VPlan after emitting masked variant:

define i16 @foo(i16 %a, i32 %n) {
omp.inner.for.body.i.lr.ph:
  %b3.i.lpriv = alloca [12 x i16], align 1
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE"([12 x i16]* %b3.i.lpriv) ]
  br label %omp.inner.for.body.i

omp.inner.for.body.i:
  %.omp.iv.i.local.03 = phi i32 [ %add5.i, %omp.body.continue.i ], [ 0, %DIR.OMP.SIMD.1 ]

  %arrayidx.i = getelementptr inbounds [12 x i16], [12 x i16]* %b3.i.lpriv, i32 %.omp.iv.i.local.03, i32 3
  store i16 %a, i16* %arrayidx.i

  br label %omp.body.continue.i

omp.body.continue.i:
  %add5.i = add nuw nsw i32 %.omp.iv.i.local.03, 1
  %exitcond.not = icmp eq i32 %.omp.iv.i.local.03, %n
  br i1 %exitcond.not, label %omp.inner.for.cond.i.DIR.OMP.END.SIMD.5.i.loopexit_crit_edge, label %omp.inner.for.body.i

omp.inner.for.cond.i.DIR.OMP.END.SIMD.5.i.loopexit_crit_edge:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]

  %idx = getelementptr inbounds [12 x i16], [12 x i16]* %b3.i.lpriv, i64 0, i64 1
  %res = load i16, i16* %idx

  ret i16 %res
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

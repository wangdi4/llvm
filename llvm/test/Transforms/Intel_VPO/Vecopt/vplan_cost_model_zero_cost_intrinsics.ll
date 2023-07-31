; Test to check VPlan cost-modelling of zero-cost intrinsics.

; RUN: opt -vplan-enable-soa=false < %s -S -passes=vplan-vec -mtriple=x86_64-unknown-unknown -mattr=+avx2 \
; RUN:     -vplan-cost-model-print-analysis-for-vf=4 -disable-output \
; RUN:     -vector-library=SVML -vplan-force-vf=4 | FileCheck %s

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)

define void @foo(ptr %a, ptr %c, ptr %func, i32 %n) {
DIR.OMP.SIMD.116:                                 ; preds = %entry
  %b.priv = alloca i32, align 4
  %i.lpriv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.116
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %i.lpriv, i32 0, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %b.priv, i32 0, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %wide.trip.count = sext i32 %n to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.2
; CHECK-LABEL: Cost Model for VPlan foo:omp.inner.for.body.#{{[0-9]+}} with VF = 4:
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %i.lpriv, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %b.priv)
; CHECK:    Cost 0 for call i64 4 ptr [[VP0:%vp.*]] ptr @llvm.lifetime.start.p0 [Serial]
  store i32 0, ptr %b.priv, align 4
  %2 = load i32, ptr %b.priv, align 4
  %add11 = add nsw i32 %2, %1
  store i32 %add11, ptr %a, align 4
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %b.priv)
; CHECK:    Cost 0 for call i64 4 ptr [[VP0]] ptr @llvm.lifetime.end.p0 [Serial]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

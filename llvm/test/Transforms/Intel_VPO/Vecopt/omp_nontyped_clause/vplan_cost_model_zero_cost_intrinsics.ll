; Test to check VPlan cost-modelling of zero-cost intrinsics.

; RUN: opt -vplan-enable-soa=false < %s -S -vplan-vec -mtriple=x86_64-unknown-unknown -mattr=+avx2 \
; RUN:     -vplan-cost-model-print-analysis-for-vf=4 -disable-output \
; RUN:     -vector-library=SVML -vplan-force-vf=4 | FileCheck %s

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)

define void @foo(i32* %a, i32* %c, i32 (i32)** %func, i32 %n) {
DIR.OMP.SIMD.116:                                 ; preds = %entry
  %b.priv = alloca i32, align 4
  %i.lpriv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.116
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LASTPRIVATE"(i32* %i.lpriv), "QUAL.OMP.PRIVATE"(i32* %b.priv) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %1 = bitcast i32* %b.priv to i8*
  %wide.trip.count = sext i32 %n to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.2
; CHECK-LABEL: Cost Model for VPlan foo:omp.inner.for.body.#{{[0-9]+}} with VF = 4:
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %2 = trunc i64 %indvars.iv to i32
  store i32 %2, i32* %i.lpriv, align 4
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1)
; CHECK:    Cost 0 for call i64 4 i8* [[VP0:%vp.*]] void (i64, i8*)* @llvm.lifetime.start.p0i8 [Serial]
  store i32 0, i32* %b.priv, align 4
  %3 = load i32, i32* %b.priv, align 4
  %add11 = add nsw i32 %3, %2
  store i32 %add11, i32* %a, align 4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1)
; CHECK:    Cost 0 for call i64 4 i8* [[VP0]] void (i64, i8*)* @llvm.lifetime.end.p0i8 [Serial]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

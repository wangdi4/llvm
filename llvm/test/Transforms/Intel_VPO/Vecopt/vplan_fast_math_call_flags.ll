;; Check that the fast math flags of the call is preserved.
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -vplan-vec -S %s | FileCheck %s
; RUN: opt -passes="vplan-vec" -S %s | FileCheck %s

define void @_Z3fooPfS_S_i(float* %A, float* %B, float* %C, i32 %N) {
;
; CHECK-LABEL:  vector.body:
; CHECK:        {{%.*}} = call fast noundef float @_Z8ext_funcff(float noundef {{%.*}}, float noundef {{%.*}})
; CHECK:        {{%.*}} = call fast noundef float @_Z8ext_funcff(float noundef {{%.*}}, float noundef {{%.*}})
; CHECK:        {{%.*}} = call fast noundef float @_Z8ext_funcff(float noundef {{%.*}}, float noundef {{%.*}})
; CHECK:        {{%.*}} = call fast noundef float @_Z8ext_funcff(float noundef {{%.*}}, float noundef {{%.*}})
;
entry:
  %i.linear.iv = alloca i32, align 4
  %cmp3.not18 = icmp slt i32 %N, 1
  br i1 %cmp3.not18, label %omp.precond.end, label %omp.inner.for.body.lr.ph

omp.inner.for.body.lr.ph:                         ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.LINEAR:IV.TYPED"(i32* %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %wide.trip.count = sext i32 %N to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.1, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, i32* %i.linear.iv, align 4
  %arrayidx = getelementptr inbounds float, float* %A, i64 %indvars.iv
  %2 = load float, float* %arrayidx, align 4
  %arrayidx6 = getelementptr inbounds float, float* %B, i64 %indvars.iv
  %3 = load float, float* %arrayidx6, align 4
  %call = call fast noundef float @_Z8ext_funcff(float noundef %2, float noundef %3)
  %arrayidx8 = getelementptr inbounds float, float* %C, i64 %indvars.iv
  store float %call, float* %arrayidx8, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge, label %omp.inner.for.body, !llvm.loop !0

omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge: ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge, %entry
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare noundef float @_Z8ext_funcff(float noundef, float noundef)

!0 = distinct !{!0, !1, !2}
!1 = !{!"llvm.loop.vectorize.enable", i1 true}
!2 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}

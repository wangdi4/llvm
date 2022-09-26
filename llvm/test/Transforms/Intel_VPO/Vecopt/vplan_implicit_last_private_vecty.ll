;
; RUN: opt -enable-new-pm=0 -vplan-vec -disable-output -disable-output -vplan-print-after-vpentity-instrs -vplan-print-after-create-masked-vplan -vplan-enable-masked-variant < %s 2>&1 | FileCheck %s
; RUN: opt -passes='vplan-vec' -disable-output -disable-output -vplan-print-after-vpentity-instrs -vplan-print-after-create-masked-vplan -vplan-enable-masked-variant < %s 2>&1 | FileCheck %s

; TODO: Enable test for HIR when vectors are supported by loopopt

target triple = "x86_64-unknown-linux-gnu"

; CHECK-NOT: VPlan after emitting masked variant
;
define dso_local <2 x i32> @_Z3fooPi(<2 x i32>* nocapture readonly %A) {
DIR.OMP.SIMD.116:
  %i.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.116
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV.TYPED"(i32* %i.linear.iv, i32 0, i32 1, i32 1), "QUAL.OMP.SIMDLEN"(i64 2) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %ptridx = getelementptr inbounds <2 x i32>, <2 x i32>* %A, i64 %indvars.iv
  %1 = load <2 x i32>, <2 x i32>* %ptridx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.217, label %omp.inner.for.body

DIR.OMP.END.SIMD.217:                             ; preds = %omp.inner.for.body
  %.lcssa = phi <2 x i32> [ %1, %omp.inner.for.body ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.217
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %DIR.OMP.END.SIMD.3
  ret <2 x i32> %.lcssa
}

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)

; RUN: opt -vpo-paropt -scoped-noalias-aa -licm -S %s 2>&1 | FileCheck %s

; After paropt, the load of c0 should be hoisted by LICM as invariant.
; VPO outlines the for loop and generates noalias sets for the load and store.
; VPO must recognize that c0 is not captured by the OMP directive.
; This nocapture is tested directly by test "vpo-shared-nocapture.ll".

; void const_load(float c0, float **A) {
;   int i;
;   #pragma omp parallel for
;   for(i=1;i<100;i++) {
;     (*A)[i] = c0+i; // c0 is a parameter with a non-captured address
;   }
; }

;  outlined function definition
; CHECK: define{{.*}}split

;  c0 should be loaded in the pre-header of the loop
; CHECK: .ph:
; CHECK: load{{.*}}%c0.addr

;  c0 should not be loaded in the body
; CHECK: for.body{{.*}}:
; CHECK-NOT: load{{.*}}%c0.addr

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @const_load(float %c0, float** %A) local_unnamed_addr {
entry:
  %c0.addr = alloca float, align 4
  %A.addr = alloca float**, align 8
  %i = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store float %c0, float* %c0.addr, align 4, !tbaa !2
  store float** %A, float*** %A.addr, align 8, !tbaa !6
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0)
  %1 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1)
  %2 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2)
  store i32 0, i32* %.omp.lb, align 4, !tbaa !8
  %3 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %3)
  store volatile i32 98, i32* %.omp.ub, align 4, !tbaa !8
  br label %DIR.OMP.PARALLEL.LOOP.1

DIR.OMP.PARALLEL.LOOP.1:                          ; preds = %entry
  br label %DIR.OMP.PARALLEL.LOOP.2

DIR.OMP.PARALLEL.LOOP.2:                          ; preds = %DIR.OMP.PARALLEL.LOOP.1
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"(float* %c0.addr), "QUAL.OMP.SHARED"(float*** %A.addr) ]
  br label %DIR.OMP.PARALLEL.LOOP.39

DIR.OMP.PARALLEL.LOOP.39:                         ; preds = %DIR.OMP.PARALLEL.LOOP.2
  br label %DIR.OMP.PARALLEL.LOOP.3

DIR.OMP.PARALLEL.LOOP.3:                          ; preds = %DIR.OMP.PARALLEL.LOOP.39
  %5 = load i32, i32* %.omp.lb, align 4, !tbaa !8
  store volatile i32 %5, i32* %.omp.iv, align 4, !tbaa !8
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.PARALLEL.LOOP.3
  %6 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !8
  %7 = load volatile i32, i32* %.omp.ub, align 4, !tbaa !8
  %cmp = icmp sgt i32 %6, %7
  br i1 %cmp, label %omp.loop.exit.split, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !8
  %add = add nsw i32 %8, 1
  store i32 %add, i32* %i, align 4, !tbaa !8
  %9 = load float, float* %c0.addr, align 4, !tbaa !2
  %conv = sitofp i32 %add to float
  %add1 = fadd float %9, %conv
  %10 = load float**, float*** %A.addr, align 8, !tbaa !6
  %11 = load float*, float** %10, align 8, !tbaa !10
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds float, float* %11, i64 %idxprom
  store float %add1, float* %arrayidx, align 4, !tbaa !2
  %12 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !8
  %add2 = add nsw i32 %12, 1
  store volatile i32 %add2, i32* %.omp.iv, align 4, !tbaa !8
  br label %omp.inner.for.cond

omp.loop.exit.split:                              ; preds = %omp.inner.for.cond
  br label %DIR.OMP.END.PARALLEL.LOOP.4

DIR.OMP.END.PARALLEL.LOOP.4:                      ; preds = %omp.loop.exit.split
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.5

DIR.OMP.END.PARALLEL.LOOP.5:                      ; preds = %DIR.OMP.END.PARALLEL.LOOP.4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %3)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %2)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0)
  ret void
}

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"pointer@_ZTSPPf", !4, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !4, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"pointer@_ZTSPf", !4, i64 0}

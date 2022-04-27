; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -simplifycfg -instcombine -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation -hir-vplan-vec -disable-output -print-before=hir-vplan-vec %s 2>&1 | FileCheck %s

; VPlanDriverHIR cannot be run via new pass manager.

; Original code:
; void foo(int *x, int *y) {
; #pragma omp parallel for simd
;   for (int i = 0; i < 10000; ++i)
;     x[i] = y[i] + i;
; }

; Check that the HIR loop region does not contain any code
; generated during parallel loop outlining.

; CHECK: [[RID:%.+]] = @llvm.directive.region.entry(){{.*}}DIR.OMP.SIMD()
; CHECK-NOT: __kmpc_for_static_init
; CHECK-NOT: __kmpc_for_static_fini
; CHECK: @llvm.directive.region.exit([[RID]]){{.*}}DIR.OMP.END.SIMD()

; ModuleID = 'test.cpp'
source_filename = "test.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @_Z3fooPiS_(i32* %x, i32* %y) #0 {
entry:
  %x.addr = alloca i32*, align 8
  %y.addr = alloca i32*, align 8
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32* %x, i32** %x.addr, align 8, !tbaa !2
  store i32* %y, i32** %y.addr, align 8, !tbaa !2
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !6
  %2 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  store i32 9999, i32* %.omp.ub, align 4, !tbaa !6
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"(i32** %y.addr), "QUAL.OMP.SHARED"(i32** %x.addr) ]
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  %5 = load i32, i32* %.omp.lb, align 4, !tbaa !6
  store i32 %5, i32* %.omp.iv, align 4, !tbaa !6
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %6 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %7 = load i32, i32* %.omp.ub, align 4, !tbaa !6
  %cmp = icmp sle i32 %6, %7
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %8) #2
  %9 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %mul = mul nsw i32 %9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !6
  %10 = load i32*, i32** %y.addr, align 8, !tbaa !2
  %11 = load i32, i32* %i, align 4, !tbaa !6
  %idxprom = sext i32 %11 to i64
  %arrayidx = getelementptr inbounds i32, i32* %10, i64 %idxprom
  %12 = load i32, i32* %arrayidx, align 4, !tbaa !6
  %13 = load i32, i32* %i, align 4, !tbaa !6
  %add1 = add nsw i32 %12, %13
  %14 = load i32*, i32** %x.addr, align 8, !tbaa !2
  %15 = load i32, i32* %i, align 4, !tbaa !6
  %idxprom2 = sext i32 %15 to i64
  %arrayidx3 = getelementptr inbounds i32, i32* %14, i64 %idxprom2
  store i32 %add1, i32* %arrayidx3, align 4, !tbaa !6
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %16 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %16) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %17 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %add4 = add nsw i32 %17, 1
  store i32 %add4, i32* %.omp.iv, align 4, !tbaa !6
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.SIMD"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %18 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %18) #2
  %19 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %19) #2
  %20 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %20) #2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "may-have-openmp-directive"="true" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPi", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}

; end INTEL_FEATURE_SW_ADVANCED


; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
;struct s1 {
;  double d[10];
;};
;void foo() {
;  struct s1 a;
;  double s = 0;
;#pragma omp target firstprivate(a) map(s)
;  {
;#pragma omp parallel for
;    for (int i = 0; i < 10; ++i)
;      a.d[i] = i;
;    s += a.d[0] + a.d[9];
;  }
;}

; Verify that pass-as-kernel-argument optimization is not applied
; for 'a'. Making it a kernel argument effectively moves it to addrspace(0),
; which makes it thread-local, so the store to a.d[9] may not be visible
; to the master thread that reads from a.d[9], if the store is executed
; not by the master thread.
; CHECK: define{{.*}}spir_kernel void @__omp_offloading_805_be2278__Z3foov_l7
; CHECK-NOT: byval

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%struct.s1 = type { [10 x double] }

; Function Attrs: convergent mustprogress noinline nounwind
define hidden spir_func void @_Z3foov() #0 {
entry:
  %a = alloca %struct.s1, align 8
  %a.ascast = addrspacecast %struct.s1* %a to %struct.s1 addrspace(4)*
  %s = alloca double, align 8
  %s.ascast = addrspacecast double* %s to double addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  store double 0.000000e+00, double addrspace(4)* %s.ascast, align 8, !tbaa !9
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(%struct.s1 addrspace(4)* %a.ascast), "QUAL.OMP.MAP.TO"(%struct.s1 addrspace(4)* %a.ascast, %struct.s1 addrspace(4)* %a.ascast, i64 80, i64 161, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(double addrspace(4)* %s.ascast, double addrspace(4)* %s.ascast, i64 8, i64 35, i8* null, i8* null), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4, !tbaa !13
  store i32 9, i32 addrspace(4)* %.omp.ub.ascast, align 4, !tbaa !13
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SHARED"(%struct.s1 addrspace(4)* %a.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast) ]
  %2 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4, !tbaa !13
  store i32 %2, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !13
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !13
  %4 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4, !tbaa !13
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !13
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4, !tbaa !13
  %6 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !13
  %conv = sitofp i32 %6 to double
  %d = getelementptr inbounds %struct.s1, %struct.s1 addrspace(4)* %a.ascast, i32 0, i32 0, !intel-tbaa !15
  %7 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !13
  %idxprom = sext i32 %7 to i64
  %arrayidx = getelementptr inbounds [10 x double], [10 x double] addrspace(4)* %d, i64 0, i64 %idxprom, !intel-tbaa !18
  store double %conv, double addrspace(4)* %arrayidx, align 8, !tbaa !19
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !13
  %add1 = add nsw i32 %8, 1
  store i32 %add1, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !13
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %d2 = getelementptr inbounds %struct.s1, %struct.s1 addrspace(4)* %a.ascast, i32 0, i32 0, !intel-tbaa !15
  %arrayidx3 = getelementptr inbounds [10 x double], [10 x double] addrspace(4)* %d2, i64 0, i64 0, !intel-tbaa !18
  %9 = load double, double addrspace(4)* %arrayidx3, align 8, !tbaa !19
  %d4 = getelementptr inbounds %struct.s1, %struct.s1 addrspace(4)* %a.ascast, i32 0, i32 0, !intel-tbaa !15
  %arrayidx5 = getelementptr inbounds [10 x double], [10 x double] addrspace(4)* %d4, i64 0, i64 9, !intel-tbaa !18
  %10 = load double, double addrspace(4)* %arrayidx5, align 8, !tbaa !19
  %add6 = fadd fast double %9, %10
  %11 = load double, double addrspace(4)* %s.ascast, align 8, !tbaa !9
  %add7 = fadd fast double %11, %add6
  store double %add7, double addrspace(4)* %s.ascast, align 8, !tbaa !9
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent mustprogress noinline nounwind "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!7}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!8}

!0 = !{i32 0, i32 2053, i32 12460664, !"_Z3foov", i32 7, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!7 = !{!"cl_doubles"}
!8 = !{!"clang version 13.0.0"}
!9 = !{!10, !10, i64 0}
!10 = !{!"double", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C++ TBAA"}
!13 = !{!14, !14, i64 0}
!14 = !{!"int", !11, i64 0}
!15 = !{!16, !17, i64 0}
!16 = !{!"struct@_ZTS2s1", !17, i64 0}
!17 = !{!"array@_ZTSA10_d", !10, i64 0}
!18 = !{!17, !10, i64 0}
!19 = !{!16, !10, i64 0}

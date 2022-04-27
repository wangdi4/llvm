; INTEL_CUSTOMIZATION
; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; void foo() {
; #pragma omp target teams
;   {
; #pragma omp distribute parallel for collapse(2)
;     for (int i = 0; i < 77; ++i)
;       for (int j = 0; j < 99; ++j);
;   }
; }

; Check that the collapsed LB and UB variables are properly privatized
; by target and teams as WILOCAL, so that there is no barrier required.
; CHECK-NOT: call{{.*}}spirv_ControlBarrier

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define hidden spir_func void @foo() #0 {
entry:
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %tmp1 = alloca i32, align 4
  %tmp1.ascast = addrspacecast i32* %tmp1 to i32 addrspace(4)*
  %.omp.uncollapsed.iv = alloca i32, align 4
  %.omp.uncollapsed.iv.ascast = addrspacecast i32* %.omp.uncollapsed.iv to i32 addrspace(4)*
  %.omp.uncollapsed.iv2 = alloca i32, align 4
  %.omp.uncollapsed.iv2.ascast = addrspacecast i32* %.omp.uncollapsed.iv2 to i32 addrspace(4)*
  %.omp.uncollapsed.lb = alloca i32, align 4
  %.omp.uncollapsed.lb.ascast = addrspacecast i32* %.omp.uncollapsed.lb to i32 addrspace(4)*
  %.omp.uncollapsed.ub = alloca i32, align 4
  %.omp.uncollapsed.ub.ascast = addrspacecast i32* %.omp.uncollapsed.ub to i32 addrspace(4)*
  %.omp.uncollapsed.lb3 = alloca i32, align 4
  %.omp.uncollapsed.lb3.ascast = addrspacecast i32* %.omp.uncollapsed.lb3 to i32 addrspace(4)*
  %.omp.uncollapsed.ub4 = alloca i32, align 4
  %.omp.uncollapsed.ub4.ascast = addrspacecast i32* %.omp.uncollapsed.ub4 to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %j = alloca i32, align 4
  %j.ascast = addrspacecast i32* %j to i32 addrspace(4)*
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb3.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.ub4.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.iv2.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp1.ascast) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.iv2.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb3.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.ub4.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp1.ascast) ]
  store i32 0, i32 addrspace(4)* %.omp.uncollapsed.lb.ascast, align 4
  store i32 76, i32 addrspace(4)* %.omp.uncollapsed.ub.ascast, align 4
  store i32 0, i32 addrspace(4)* %.omp.uncollapsed.lb3.ascast, align 4
  store i32 98, i32 addrspace(4)* %.omp.uncollapsed.ub4.ascast, align 4
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.COLLAPSE"(i32 2), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, i32 addrspace(4)* %.omp.uncollapsed.iv2.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.uncollapsed.ub.ascast, i32 addrspace(4)* %.omp.uncollapsed.ub4.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb3.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast) ]
  %3 = load i32, i32 addrspace(4)* %.omp.uncollapsed.lb.ascast, align 4
  store i32 %3, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc11, %entry
  %4 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  %5 = load i32, i32 addrspace(4)* %.omp.uncollapsed.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end13

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %6 = load i32, i32 addrspace(4)* %.omp.uncollapsed.lb3.ascast, align 4
  store i32 %6, i32 addrspace(4)* %.omp.uncollapsed.iv2.ascast, align 4
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.cond5:                       ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body
  %7 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv2.ascast, align 4
  %8 = load i32, i32 addrspace(4)* %.omp.uncollapsed.ub4.ascast, align 4
  %cmp6 = icmp sle i32 %7, %8
  br i1 %cmp6, label %omp.uncollapsed.loop.body7, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body7:                       ; preds = %omp.uncollapsed.loop.cond5
  %9 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  %mul = mul nsw i32 %9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4
  %10 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv2.ascast, align 4
  %mul8 = mul nsw i32 %10, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, i32 addrspace(4)* %j.ascast, align 4
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.uncollapsed.loop.body7
  %11 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv2.ascast, align 4
  %add10 = add nsw i32 %11, 1
  store i32 %add10, i32 addrspace(4)* %.omp.uncollapsed.iv2.ascast, align 4
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond5
  br label %omp.uncollapsed.loop.inc11

omp.uncollapsed.loop.inc11:                       ; preds = %omp.uncollapsed.loop.end
  %12 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  %add12 = add nsw i32 %12, 1
  store i32 %add12, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end13:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2052, i32 85994564, !"foo", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 9.0.0"}

; end INTEL_CUSTOMIZATION

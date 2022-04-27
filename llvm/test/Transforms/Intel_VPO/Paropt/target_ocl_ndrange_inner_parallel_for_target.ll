; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; void foo() {
; #pragma omp target teams distribute collapse(2)
;   for (int i = 0; i < 32; ++i)
;     for (int j = 0; j < 32; ++j) {
; #pragma omp parallel for
;       for (int k = 0; k < 32; ++k);
;     }
; }

; Check that SPMD mode is not applied to the inner parallel for:
; CHECK-NOT: call{{.*}}@_Z13get_global_idj

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define hidden spir_func void @foo() #0 {
entry:
  %.omp.uncollapsed.lb = alloca i32, align 4
  %.omp.uncollapsed.lb.ascast = addrspacecast i32* %.omp.uncollapsed.lb to i32 addrspace(4)*
  %.omp.uncollapsed.ub = alloca i32, align 4
  %.omp.uncollapsed.ub.ascast = addrspacecast i32* %.omp.uncollapsed.ub to i32 addrspace(4)*
  %.omp.uncollapsed.lb1 = alloca i32, align 4
  %.omp.uncollapsed.lb1.ascast = addrspacecast i32* %.omp.uncollapsed.lb1 to i32 addrspace(4)*
  %.omp.uncollapsed.ub2 = alloca i32, align 4
  %.omp.uncollapsed.ub2.ascast = addrspacecast i32* %.omp.uncollapsed.ub2 to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %tmp3 = alloca i32, align 4
  %tmp3.ascast = addrspacecast i32* %tmp3 to i32 addrspace(4)*
  %.omp.uncollapsed.iv = alloca i32, align 4
  %.omp.uncollapsed.iv.ascast = addrspacecast i32* %.omp.uncollapsed.iv to i32 addrspace(4)*
  %.omp.uncollapsed.iv4 = alloca i32, align 4
  %.omp.uncollapsed.iv4.ascast = addrspacecast i32* %.omp.uncollapsed.iv4 to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %j = alloca i32, align 4
  %j.ascast = addrspacecast i32* %j to i32 addrspace(4)*
  %tmp10 = alloca i32, align 4
  %tmp10.ascast = addrspacecast i32* %tmp10 to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %k = alloca i32, align 4
  %k.ascast = addrspacecast i32* %k to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %.omp.uncollapsed.lb.ascast, align 4
  store i32 31, i32 addrspace(4)* %.omp.uncollapsed.ub.ascast, align 4
  store i32 0, i32 addrspace(4)* %.omp.uncollapsed.lb1.ascast, align 4
  store i32 31, i32 addrspace(4)* %.omp.uncollapsed.ub2.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb1.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.ub2.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %k.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp3.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp10.ascast) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.iv.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.uncollapsed.lb.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.uncollapsed.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.uncollapsed.lb1.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.uncollapsed.ub2.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %k.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp3.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp10.ascast) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(), "QUAL.OMP.COLLAPSE"(i32 2), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.uncollapsed.ub.ascast, i32 addrspace(4)* %.omp.uncollapsed.ub2.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb1.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %k.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp10.ascast) ]
  %3 = load i32, i32 addrspace(4)* %.omp.uncollapsed.lb.ascast, align 4
  store i32 %3, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc17, %entry
  %4 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  %5 = load i32, i32 addrspace(4)* %.omp.uncollapsed.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end19

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %6 = load i32, i32 addrspace(4)* %.omp.uncollapsed.lb1.ascast, align 4
  store i32 %6, i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast, align 4
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.cond5:                       ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body
  %7 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast, align 4
  %8 = load i32, i32 addrspace(4)* %.omp.uncollapsed.ub2.ascast, align 4
  %cmp6 = icmp sle i32 %7, %8
  br i1 %cmp6, label %omp.uncollapsed.loop.body7, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body7:                       ; preds = %omp.uncollapsed.loop.cond5
  %9 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  %mul = mul nsw i32 %9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4
  %10 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast, align 4
  %mul8 = mul nsw i32 %10, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, i32 addrspace(4)* %j.ascast, align 4
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 31, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %k.ascast) ]
  %12 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %12, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.uncollapsed.loop.body7
  %13 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %14 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp11 = icmp sle i32 %13, %14
  br i1 %cmp11, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %15 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul12 = mul nsw i32 %15, 1
  %add13 = add nsw i32 0, %mul12
  store i32 %add13, i32 addrspace(4)* %k.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %16 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add14 = add nsw i32 %16, 1
  store i32 %add14, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.body.continue15

omp.body.continue15:                              ; preds = %omp.loop.exit
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.body.continue15
  %17 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast, align 4
  %add16 = add nsw i32 %17, 1
  store i32 %add16, i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast, align 4
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond5
  br label %omp.uncollapsed.loop.inc17

omp.uncollapsed.loop.inc17:                       ; preds = %omp.uncollapsed.loop.end
  %18 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  %add18 = add nsw i32 %18, 1
  store i32 %add18, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end19:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE"() ]
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

!0 = !{i32 0, i32 2054, i32 116260867, !"_Z3foo", i32 2, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 9.0.0"}

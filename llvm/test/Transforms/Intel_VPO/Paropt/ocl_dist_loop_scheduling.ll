; RUN: opt < %s -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg  -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -switch-to-offload -vpo-paropt-gpu-execution-scheme=0 -S | FileCheck %s
; RUN: opt < %s -passes='function(loop(rotate),vpo-cfg-restructuring,vpo-paropt-prepare,simplify-cfg,loop(simplify-cfg),sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -vpo-paropt-gpu-execution-scheme=0 -S | FileCheck %s

; Original code:
; void foo() {
;   int i;
; #pragma omp target teams distribute parallel for
;   for (int i = 0; i < 100; ++i);
; }

; Check that workitem loop distribution computation initializes
; loop bounds with the workgroup (team) bounds:
; CHECK: call{{.*}}get_local_size
; CHECK: %[[TLB:.*]] = load i32, i32* %team.lb
; CHECK: store i32 %[[TLB]], i32* %lower.bnd
; CHECK: %[[TUB:.*]] = load i32, i32* %team.ub
; CHECK: store i32 %[[TUB]], i32* %upper.bnd

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @foo() #0 {
entry:
  %i = alloca i32, align 4
  %0 = addrspacecast i32* %i to i32 addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %1 = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %2 = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %3 = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %4 = addrspacecast i32* %tmp to i32 addrspace(4)*
  %i1 = alloca i32, align 4
  %5 = addrspacecast i32* %i1 to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %1, align 4
  store i32 99, i32 addrspace(4)* %2, align 4
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %5), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %1), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %2), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %3), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %4) ]
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %5), "QUAL.OMP.SHARED"(i32 addrspace(4)* %1), "QUAL.OMP.SHARED"(i32 addrspace(4)* %2), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %3), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %4) ]
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %1), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %3), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %2), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %5) ]
  %9 = load i32, i32 addrspace(4)* %1, align 4
  store i32 %9, i32 addrspace(4)* %3, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %10 = load i32, i32 addrspace(4)* %3, align 4
  %11 = load i32, i32 addrspace(4)* %2, align 4
  %cmp = icmp sle i32 %10, %11
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %12 = load i32, i32 addrspace(4)* %3, align 4
  %mul = mul nsw i32 %12, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %5, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %13 = load i32, i32 addrspace(4)* %3, align 4
  %add2 = add nsw i32 %13, 1
  store i32 %add2, i32 addrspace(4)* %3, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2052, i32 85985690, !"foo", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 9.0.0"}

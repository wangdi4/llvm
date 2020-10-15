; RUN: opt < %s -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg  -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -switch-to-offload -vpo-paropt-enable-64bit-opencl-atomics -S | FileCheck %s
; RUN: opt < %s -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,simplify-cfg,loop-simplifycfg,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -vpo-paropt-enable-64bit-opencl-atomics -S | FileCheck %s

; Original code:
; void foo() {
;   double add = 0.0, sub = 0.0, mul = 1.0;
; #pragma omp target parallel for reduction(+: add) reduction(-: sub) reduction(*: mul)
;   for (int i = 0; i < 100; ++i);
; }

; Check that Paropt generates atomic updates under -vpo-paropt-enable-64bit-opencl-atomics:
; CHECK: call spir_func void @__kmpc_atomic_float8_add
; CHECK: call spir_func void @__kmpc_atomic_float8_add
; CHECK: call spir_func void @__kmpc_atomic_float8_mul

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define hidden spir_func void @foo() #0 {
entry:
  %add = alloca double, align 8
  %add.ascast = addrspacecast double* %add to double addrspace(4)*
  %sub = alloca double, align 8
  %sub.ascast = addrspacecast double* %sub to double addrspace(4)*
  %mul = alloca double, align 8
  %mul.ascast = addrspacecast double* %mul to double addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  store double 0.000000e+00, double addrspace(4)* %add.ascast, align 8
  store double 0.000000e+00, double addrspace(4)* %sub.ascast, align 8
  store double 1.000000e+00, double addrspace(4)* %mul.ascast, align 8
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 99, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(double addrspace(4)* %add.ascast, double addrspace(4)* %add.ascast, i64 8, i64 547), "QUAL.OMP.MAP.TOFROM"(double addrspace(4)* %sub.ascast, double addrspace(4)* %sub.ascast, i64 8, i64 547), "QUAL.OMP.MAP.TOFROM"(double addrspace(4)* %mul.ascast, double addrspace(4)* %mul.ascast, i64 8, i64 547), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD"(double addrspace(4)* %add.ascast), "QUAL.OMP.REDUCTION.SUB"(double addrspace(4)* %sub.ascast), "QUAL.OMP.REDUCTION.MUL"(double addrspace(4)* %mul.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast) ]
  %2 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %2, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %4 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul1 = mul nsw i32 %5, 1
  %add2 = add nsw i32 0, %mul1
  store i32 %add2, i32 addrspace(4)* %i.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add3 = add nsw i32 %6, 1
  store i32 %add3, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!4}

!0 = !{i32 0, i32 2054, i32 1856822, !"_Z3foo", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"cl_doubles"}
!4 = !{!"clang version 10.0.0"}

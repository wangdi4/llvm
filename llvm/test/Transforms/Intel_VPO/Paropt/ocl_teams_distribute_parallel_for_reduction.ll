; RUN: opt < %s -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg  -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -switch-to-offload -S | FileCheck %s
; RUN: opt < %s -passes='function(loop(rotate),vpo-cfg-restructuring,vpo-paropt-prepare,simplify-cfg,loop(simplify-cfg),sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload  -S | FileCheck %s

; Original code:
; double foo(double *x) {
;   int i;
;   double d = 0.0;
; #pragma omp target teams distribute parallel for map(to: x[0:1000]) reduction(+: d)
;   for (i = 0; i < 1000; ++i)
;     d += x[i];
;
;   return d;
; }

; Make sure that we generate two critical sections for the reduction update,
; one for "distribute parallel for" and another for "teams":
; CHECK-DAG: [[ID0:%[a-zA-Z._0-9]+]] = call spir_func i64 @_Z12get_local_idj(i32 0)
; CHECK-DAG: [[CMP0:%[a-zA-Z._0-9]+]] = icmp eq i64 [[ID0]], 0
; CHECK-DAG: [[ID1:%[a-zA-Z._0-9]+]] = call spir_func i64 @_Z12get_local_idj(i32 1)
; CHECK-DAG: [[CMP1:%[a-zA-Z._0-9]+]] = icmp eq i64 [[ID1]], 0
; CHECK-DAG: [[ID2:%[a-zA-Z._0-9]+]] = call spir_func i64 @_Z12get_local_idj(i32 2)
; CHECK-DAG: [[CMP2:%[a-zA-Z._0-9]+]] = icmp eq i64 [[ID2]], 0
; CHECK-DAG: [[AND:%[a-zA-Z._0-9]+]] = and i1 [[CMP0]], [[CMP1]]
; CHECK-DAG: [[PRED:%[a-zA-Z._0-9]+]] = and i1 [[AND]], [[CMP2]]
; CHECK: call spir_func void @__kmpc_critical
; CHECK: br i1 [[PRED]], label %[[MASTERCODE1:[a-zA-Z._0-9]+]], label %[[FALLTHRU1:[a-zA-Z._0-9]+]]
; CHECK: [[MASTERCODE1]]:
; CHECK: call spir_func void @__kmpc_critical
; CHECK: br label %[[FALLTHRU1]]
; CHECK: [[FALLTHRU1]]:
; CHECK: br i1 [[PRED]], label
; CHECK: br i1 [[PRED]], label %[[MASTERCODE2:[a-zA-Z._0-9]+]], label %[[FALLTHRU2:[a-zA-Z._0-9]+]]
; CHECK: [[MASTERCODE2]]:
; CHECK: call spir_func void @__kmpc_end_critical
; CHECK: br label %[[FALLTHRU2]]
; CHECK: [[FALLTHRU2]]:
; CHECK-NOT: call spir_func void @__kmpc_critical

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func double @foo(double addrspace(4)* %x) #0 {
entry:
  %retval = alloca double, align 8
  %0 = addrspacecast double* %retval to double addrspace(4)*
  %x.addr = alloca double addrspace(4)*, align 8
  %1 = addrspacecast double addrspace(4)** %x.addr to double addrspace(4)* addrspace(4)*
  %i = alloca i32, align 4
  %2 = addrspacecast i32* %i to i32 addrspace(4)*
  %d = alloca double, align 8
  %3 = addrspacecast double* %d to double addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %4 = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %5 = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %6 = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %7 = addrspacecast i32* %tmp to i32 addrspace(4)*
  store double addrspace(4)* %x, double addrspace(4)* addrspace(4)* %1, align 8
  store double 0.000000e+00, double addrspace(4)* %3, align 8
  store i32 0, i32 addrspace(4)* %4, align 4
  store i32 999, i32 addrspace(4)* %5, align 4
  %8 = load double addrspace(4)*, double addrspace(4)* addrspace(4)* %1, align 8
  %arrayidx = getelementptr inbounds double, double addrspace(4)* %8, i64 0
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TO:AGGRHEAD"(double addrspace(4)* addrspace(4)* %1, double addrspace(4)* addrspace(4)* %1, i64 8), "QUAL.OMP.MAP.TO:AGGR"(double addrspace(4)* addrspace(4)* %1, double addrspace(4)* %arrayidx, i64 8000), "QUAL.OMP.MAP.TOFROM"(double addrspace(4)* %3), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %4), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %5), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %2), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %6), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %7) ]
  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.REDUCTION.ADD"(double addrspace(4)* %3), "QUAL.OMP.SHARED"(i32 addrspace(4)* %4), "QUAL.OMP.SHARED"(i32 addrspace(4)* %5), "QUAL.OMP.SHARED"(i32 addrspace(4)* %2), "QUAL.OMP.SHARED"(double addrspace(4)* addrspace(4)* %1), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %6), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %7) ]
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.REDUCTION.ADD"(double addrspace(4)* %3), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %4), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %6), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %5), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %2), "QUAL.OMP.SHARED"(double addrspace(4)* addrspace(4)* %1) ]
  %12 = load i32, i32 addrspace(4)* %4, align 4
  store i32 %12, i32 addrspace(4)* %6, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %13 = load i32, i32 addrspace(4)* %6, align 4
  %14 = load i32, i32 addrspace(4)* %5, align 4
  %cmp = icmp sle i32 %13, %14
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %15 = load i32, i32 addrspace(4)* %6, align 4
  %mul = mul nsw i32 %15, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %2, align 4
  %16 = load double addrspace(4)*, double addrspace(4)* addrspace(4)* %1, align 8
  %17 = load i32, i32 addrspace(4)* %2, align 4
  %idxprom = sext i32 %17 to i64
  %arrayidx1 = getelementptr inbounds double, double addrspace(4)* %16, i64 %idxprom
  %18 = load double, double addrspace(4)* %arrayidx1, align 8
  %19 = load double, double addrspace(4)* %3, align 8
  %add2 = fadd double %19, %18
  store double %add2, double addrspace(4)* %3, align 8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %20 = load i32, i32 addrspace(4)* %6, align 4
  %add3 = add nsw i32 %20, 1
  store i32 %add3, i32 addrspace(4)* %6, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.TARGET"() ]
  %21 = load double, double addrspace(4)* %3, align 8
  ret double %21
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
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!4}

!0 = !{i32 0, i32 2052, i32 85985690, !"foo", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"cl_doubles"}
!4 = !{!"clang version 9.0.0"}

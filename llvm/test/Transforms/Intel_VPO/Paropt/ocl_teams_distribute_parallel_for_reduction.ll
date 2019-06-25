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

; Make sure that we generate only one critical section for the reduction update:
; CHECK: call void @__kmpc_critical
; CHECK-NOT: call void @__kmpc_critical

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func double @foo(double* %x) #0 {
entry:
  %x.addr = alloca double*, align 8
  %i = alloca i32, align 4
  %d = alloca double, align 8
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  store double* %x, double** %x.addr, align 8
  store double 0.000000e+00, double* %d, align 8
  store i32 0, i32* %.omp.lb, align 4
  store i32 999, i32* %.omp.ub, align 4
  %0 = load double*, double** %x.addr, align 8
  %arrayidx = getelementptr inbounds double, double* %0, i64 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TO:AGGRHEAD"(double** %x.addr, double** %x.addr, i64 8), "QUAL.OMP.MAP.TO:AGGR"(double** %x.addr, double* %arrayidx, i64 8000), "QUAL.OMP.MAP.TOFROM"(double* %d), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.ub), "QUAL.OMP.FIRSTPRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %tmp) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.REDUCTION.ADD"(double* %d), "QUAL.OMP.SHARED"(i32* %.omp.lb), "QUAL.OMP.SHARED"(i32* %.omp.ub), "QUAL.OMP.SHARED"(i32* %i), "QUAL.OMP.SHARED"(double** %x.addr), "QUAL.OMP.PRIVATE"(i32* %tmp) ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.REDUCTION.ADD"(double* %d), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"(double** %x.addr) ]
  %4 = load i32, i32* %.omp.lb, align 4
  store i32 %4, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %5 = load i32, i32* %.omp.iv, align 4
  %6 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %5, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %8 = load double*, double** %x.addr, align 8
  %9 = load i32, i32* %i, align 4
  %idxprom = sext i32 %9 to i64
  %arrayidx1 = getelementptr inbounds double, double* %8, i64 %idxprom
  %10 = load double, double* %arrayidx1, align 8
  %11 = load double, double* %d, align 8
  %add2 = fadd double %11, %10
  store double %add2, double* %d, align 8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, i32* %.omp.iv, align 4
  %add3 = add nsw i32 %12, 1
  store i32 %add3, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  %13 = load double, double* %d, align 8
  ret double %13
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!4}

!0 = !{i32 0, i32 2052, i32 85987291, !"foo", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"cl_doubles"}
!4 = !{!"clang version 9.0.0"}

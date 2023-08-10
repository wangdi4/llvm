; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -switch-to-offload -vpo-paropt-spirv-target-has-eu-fusion=false -vpo-paropt-enable-64bit-opencl-atomics=false -vpo-paropt-opt-data-sharing-for-reduction=true -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck --check-prefix=OPT_AFTER_ODS %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing)' -switch-to-offload -vpo-paropt-spirv-target-has-eu-fusion=false -vpo-paropt-enable-64bit-opencl-atomics=false -vpo-paropt-opt-data-sharing-for-reduction=true -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck --check-prefix=OPT_AFTER_ODS %s

; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -vpo-paropt -switch-to-offload -vpo-paropt-spirv-target-has-eu-fusion=false -vpo-paropt-enable-64bit-opencl-atomics=false -vpo-paropt-opt-data-sharing-for-reduction=true -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck --check-prefix=OPT %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -switch-to-offload -vpo-paropt-spirv-target-has-eu-fusion=false -vpo-paropt-enable-64bit-opencl-atomics=false -vpo-paropt-opt-data-sharing-for-reduction=true -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck --check-prefix=OPT %s

; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -vpo-paropt -switch-to-offload -vpo-paropt-spirv-target-has-eu-fusion=false -vpo-paropt-enable-64bit-opencl-atomics=false -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -switch-to-offload -vpo-paropt-spirv-target-has-eu-fusion=false -vpo-paropt-enable-64bit-opencl-atomics=false -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck %s

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
; one for "distribute parallel for" and another for "teams".
; We generate a call "__kmpc_critical_simd" for the teams region now.
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
; CHECK: call spir_func void @__kmpc_critical_simd
; CHECK: call spir_func void @__kmpc_end_critical_simd
; CHECK: br label %[[FALLTHRU1]]
; CHECK: [[FALLTHRU1]]:
; CHECK-NOT: call spir_func void @__kmpc_critical

; Check that after vpo-paropt-optimize-data-sharing, reduction clause is
; converted to shared, when the optimization is enabled.
; OPT_AFTER_ODS:      "DIR.OMP.TEAMS"
; OPT_AFTER_ODS-SAME: "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %d.ascast, double 0.000000e+00, i32 1)
; OPT_AFTER_ODS:      "DIR.OMP.DISTRIBUTE.PARLOOP"
; OPT_AFTER_ODS-SAME: "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %d.ascast, double 0.000000e+00, i32 1)

; Check for IR after Paropt when reduction to shared conversion is enabled.
; OPT: call spir_func void @__kmpc_critical
; OPT: call spir_func void @__kmpc_end_critical
; OPT-NOT: call spir_func void @__kmpc_critical
; OPT-NOT: call spir_func void @__kmpc_end_critical

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define hidden spir_func double @foo(ptr addrspace(4) %x) {
entry:
  %retval = alloca double, align 8
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %x.addr = alloca ptr addrspace(4), align 8
  %x.addr.ascast = addrspacecast ptr %x.addr to ptr addrspace(4)
  %i = alloca i32, align 4
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %d = alloca double, align 8
  %d.ascast = addrspacecast ptr %d to ptr addrspace(4)
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %x.map.ptr.tmp = alloca ptr addrspace(4), align 8
  %x.map.ptr.tmp.ascast = addrspacecast ptr %x.map.ptr.tmp to ptr addrspace(4)
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  store ptr addrspace(4) %x, ptr addrspace(4) %x.addr.ascast, align 8
  store double 0.000000e+00, ptr addrspace(4) %d.ascast, align 8
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 999, ptr addrspace(4) %.omp.ub.ascast, align 4
  %0 = load ptr addrspace(4), ptr addrspace(4) %x.addr.ascast, align 8
  %1 = load ptr addrspace(4), ptr addrspace(4) %x.addr.ascast, align 8
  %2 = load ptr addrspace(4), ptr addrspace(4) %x.addr.ascast, align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %d.ascast, ptr addrspace(4) %d.ascast, i64 8, i64 547, ptr null, ptr null),
    "QUAL.OMP.MAP.TO"(ptr addrspace(4) %1, ptr addrspace(4) %2, i64 8000, i64 33, ptr null, ptr null),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %x.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  store ptr addrspace(4) %1, ptr addrspace(4) %x.map.ptr.tmp.ascast, align 8
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %d.ascast, double 0.000000e+00, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %x.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %d.ascast, double 0.000000e+00, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %x.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0) ]

  %6 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %6, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %7 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %8 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %7, %8
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  %10 = load ptr addrspace(4), ptr addrspace(4) %x.map.ptr.tmp.ascast, align 8
  %11 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom = sext i32 %11 to i64
  %arrayidx1 = getelementptr inbounds double, ptr addrspace(4) %10, i64 %idxprom
  %12 = load double, ptr addrspace(4) %arrayidx1, align 8
  %13 = load double, ptr addrspace(4) %d.ascast, align 8
  %add2 = fadd fast double %13, %12
  store double %add2, ptr addrspace(4) %d.ascast, align 8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %14 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add3 = add nsw i32 %14, 1
  store i32 %add3, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]
  %15 = load double, ptr addrspace(4) %d.ascast, align 8
  ret double %15

}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2053, i32 12460681, !"_Z3foo", i32 4, i32 0, i32 0}

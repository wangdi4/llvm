; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -switch-to-offload -vpo-paropt-spirv-target-has-eu-fusion=false -vpo-paropt-enable-64bit-opencl-atomics=false -vpo-paropt-opt-data-sharing-for-reduction=true -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck --check-prefix=OPT_AFTER_ODS %s
; RUN: opt -opaque-pointers=0 -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing)' -switch-to-offload -vpo-paropt-spirv-target-has-eu-fusion=false -vpo-paropt-enable-64bit-opencl-atomics=false -vpo-paropt-opt-data-sharing-for-reduction=true -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck --check-prefix=OPT_AFTER_ODS %s

; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -vpo-paropt -switch-to-offload -vpo-paropt-spirv-target-has-eu-fusion=false -vpo-paropt-enable-64bit-opencl-atomics=false -vpo-paropt-opt-data-sharing-for-reduction=true -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck --check-prefix=OPT %s
; RUN: opt -opaque-pointers=0 -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -switch-to-offload -vpo-paropt-spirv-target-has-eu-fusion=false -vpo-paropt-enable-64bit-opencl-atomics=false -vpo-paropt-opt-data-sharing-for-reduction=true -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck --check-prefix=OPT %s

; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -vpo-paropt -switch-to-offload -vpo-paropt-spirv-target-has-eu-fusion=false -vpo-paropt-enable-64bit-opencl-atomics=false -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -switch-to-offload -vpo-paropt-spirv-target-has-eu-fusion=false -vpo-paropt-enable-64bit-opencl-atomics=false -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck %s

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
; CHECK: call spir_func void @__kmpc_end_critical
; CHECK: br label %[[FALLTHRU1]]
; CHECK: [[FALLTHRU1]]:
; CHECK-NOT: call spir_func void @__kmpc_critical

; Check that after vpo-paropt-optimize-data-sharing, reduction clause is
; converted to shared, when the optimization is enabled.
; OPT_AFTER_ODS:      "DIR.OMP.TEAMS"
; OPT_AFTER_ODS-SAME: "QUAL.OMP.SHARED:TYPED"(double addrspace(4)* %d.ascast, double 0.000000e+00, i32 1)
; OPT_AFTER_ODS:      "DIR.OMP.DISTRIBUTE.PARLOOP"
; OPT_AFTER_ODS-SAME: "QUAL.OMP.REDUCTION.ADD"(double addrspace(4)* %d.ascast)

; Check for IR after Paropt when reduction to shared conversion is enabled.
; OPT: call spir_func void @__kmpc_critical
; OPT: call spir_func void @__kmpc_end_critical
; OPT-NOT: call spir_func void @__kmpc_critical
; OPT-NOT: call spir_func void @__kmpc_end_critical

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define hidden spir_func double @foo(double addrspace(4)* %x) {
entry:
  %retval = alloca double, align 8
  %retval.ascast = addrspacecast double* %retval to double addrspace(4)*
  %x.addr = alloca double addrspace(4)*, align 8
  %x.addr.ascast = addrspacecast double addrspace(4)** %x.addr to double addrspace(4)* addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %d = alloca double, align 8
  %d.ascast = addrspacecast double* %d to double addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %x.map.ptr.tmp = alloca double addrspace(4)*, align 8
  %x.map.ptr.tmp.ascast = addrspacecast double addrspace(4)** %x.map.ptr.tmp to double addrspace(4)* addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  store double addrspace(4)* %x, double addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  store double 0.000000e+00, double addrspace(4)* %d.ascast, align 8
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 999, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %0 = load double addrspace(4)*, double addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %1 = load double addrspace(4)*, double addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %2 = load double addrspace(4)*, double addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %arrayidx = getelementptr inbounds double, double addrspace(4)* %2, i64 0
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(double addrspace(4)* %d.ascast, double addrspace(4)* %d.ascast, i64 8, i64 547, i8* null, i8* null),
    "QUAL.OMP.MAP.TO"(double addrspace(4)* %1, double addrspace(4)* %arrayidx, i64 8000, i64 33, i8* null, i8* null),
    "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %i.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast),
    "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast),
    "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast),
    "QUAL.OMP.PRIVATE"(double addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]

  store double addrspace(4)* %1, double addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD"(double addrspace(4)* %d.ascast),
    "QUAL.OMP.SHARED"(double addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast),
    "QUAL.OMP.SHARED"(i32 addrspace(4)* %i.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast),
    "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb.ascast),
    "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]

  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.REDUCTION.ADD"(double addrspace(4)* %d.ascast),
    "QUAL.OMP.SHARED"(double addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast),
    "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast),
    "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast),
    "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast) ]

  %6 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %6, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %7 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %8 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %7, %8
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4
  %10 = load double addrspace(4)*, double addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, align 8
  %11 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %idxprom = sext i32 %11 to i64
  %arrayidx1 = getelementptr inbounds double, double addrspace(4)* %10, i64 %idxprom
  %12 = load double, double addrspace(4)* %arrayidx1, align 8
  %13 = load double, double addrspace(4)* %d.ascast, align 8
  %add2 = fadd fast double %13, %12
  store double %add2, double addrspace(4)* %d.ascast, align 8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %14 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add3 = add nsw i32 %14, 1
  store i32 %add3, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]
  %15 = load double, double addrspace(4)* %d.ascast, align 8
  ret double %15

; uselistorder directives
  uselistorder i32 addrspace(4)* %i.ascast, { 3, 4, 2, 1, 0 }
  uselistorder double addrspace(4)* %d.ascast, { 0, 5, 6, 4, 3, 1, 2, 7 }
  uselistorder i32 addrspace(4)* %.omp.lb.ascast, { 3, 2, 1, 0, 4 }
  uselistorder i32 addrspace(4)* %.omp.ub.ascast, { 3, 2, 1, 0, 4 }
  uselistorder double addrspace(4)* addrspace(4)* %x.map.ptr.tmp.ascast, { 3, 2, 1, 4, 0 }
  uselistorder i32 addrspace(4)* %tmp.ascast, { 1, 0 }
  uselistorder i32 addrspace(4)* %.omp.iv.ascast, { 3, 4, 5, 6, 7, 2, 1, 0 }
  uselistorder double addrspace(4)* %1, { 1, 0 }
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

; uselistorder directives
uselistorder i32 0, { 1, 0, 2 }
uselistorder void (token)* @llvm.directive.region.exit, { 2, 1, 0 }

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2053, i32 12460681, !"_Z3foo", i32 4, i32 0, i32 0}

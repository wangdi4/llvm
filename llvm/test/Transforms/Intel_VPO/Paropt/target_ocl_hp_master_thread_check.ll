; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; void foo() {
; #pragma omp target
;   {
;     int x = 10;
; #pragma omp parallel for shared(x)
;     for (int i = 0; i < 77; ++i)
; #pragma omp atomic update
;       x += i;
;   }
; }

; Check that the master thread predicate is computed and used only once.
; The three side-effect stores inside the target region are consecuitive,
; so they may be guarded all at once:
; CHECK-DAG: [[ID0:%[a-zA-Z._0-9]+]] = call spir_func i64 @_Z12get_local_idj(i32 0)
; CHECK-DAG: [[CMP0:%[a-zA-Z._0-9]+]] = icmp eq i64 [[ID0]], 0
; CHECK-DAG: [[ID1:%[a-zA-Z._0-9]+]] = call spir_func i64 @_Z12get_local_idj(i32 1)
; CHECK-DAG: [[CMP1:%[a-zA-Z._0-9]+]] = icmp eq i64 [[ID1]], 0
; CHECK-DAG: [[ID2:%[a-zA-Z._0-9]+]] = call spir_func i64 @_Z12get_local_idj(i32 2)
; CHECK-DAG: [[CMP2:%[a-zA-Z._0-9]+]] = icmp eq i64 [[ID2]], 0
; CHECK-DAG: [[AND:%[a-zA-Z._0-9]+]] = and i1 [[CMP0]], [[CMP1]]
; CHECK-DAG: [[PRED:%[a-zA-Z._0-9]+]] = and i1 [[AND]], [[CMP2]]
; CHECK: br i1 [[PRED]], label %[[MASTERCODE:[a-zA-Z._0-9]+]], label %[[FALLTHRU:[a-zA-Z._0-9]+]]
; CHECK: [[MASTERCODE]]:
; CHECK: br label %[[FALLTHRU]]
; CHECK: [[FALLTHRU]]:
; CHECK-NOT: br i1 [[PRED]]
; An extra call to get_local_id(0) is required for parallel for:
; CHECK: call spir_func i64 @_Z12get_local_idj(i32 0)
; CHECK-NOT: call{{.*}}@_Z12get_local_idj(i32 0)
; CHECK-NOT: call{{.*}}@_Z12get_local_idj(i32 1)
; CHECK-NOT: call{{.*}}@_Z12get_local_idj(i32 2)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

define hidden spir_func void @foo() {
entry:
  %x = alloca i32, align 4
  %x.ascast = addrspacecast ptr %x to ptr addrspace(4)
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %i = alloca i32, align 4
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %atomic-temp = alloca i32, align 4
  %atomic-temp.ascast = addrspacecast ptr %atomic-temp to ptr addrspace(4)
  %atomic-temp1 = alloca i32, align 4
  %atomic-temp1.ascast = addrspacecast ptr %atomic-temp1 to ptr addrspace(4)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %x.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %atomic-temp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %atomic-temp1.ascast, i32 0, i32 1) ]

  store i32 10, ptr addrspace(4) %x.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 76, ptr addrspace(4) %.omp.ub.ascast, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %x.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %atomic-temp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %atomic-temp1.ascast, i32 0, i32 1) ]

  %2 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %2, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %4 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  %6 = load i32, ptr addrspace(4) %i.ascast, align 4
  call void @__atomic_load(i64 4, ptr addrspace(4) %x.ascast, ptr addrspace(4) %atomic-temp.ascast, i32 0)
  br label %atomic_cont

atomic_cont:                                      ; preds = %atomic_cont, %omp.inner.for.body
  %7 = load i32, ptr addrspace(4) %atomic-temp.ascast, align 4
  %add2 = add nsw i32 %7, %6
  store i32 %add2, ptr addrspace(4) %atomic-temp1.ascast, align 4
  %call = call zeroext i1 @__atomic_compare_exchange(i64 4, ptr addrspace(4) %x.ascast, ptr addrspace(4) %atomic-temp.ascast, ptr addrspace(4) %atomic-temp1.ascast, i32 0, i32 0)
  br i1 %call, label %atomic_exit, label %atomic_cont

atomic_exit:                                      ; preds = %atomic_cont
  br label %omp.body.continue

omp.body.continue:                                ; preds = %atomic_exit
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add3 = add nsw i32 %8, 1
  store i32 %add3, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local void @__atomic_load(i64, ptr addrspace(4), ptr addrspace(4), i32)
declare dso_local i1 @__atomic_compare_exchange(i64, ptr addrspace(4), ptr addrspace(4), ptr addrspace(4), i32, i32)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2052, i32 51517360, !"foo", i32 2, i32 0, i32 0}

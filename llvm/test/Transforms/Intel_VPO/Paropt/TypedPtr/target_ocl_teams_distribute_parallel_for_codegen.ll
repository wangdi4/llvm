; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; The test case checks whether the omp teams distribute parlalel loop
; is partitioned as expected.
; on top of OCL.
;
; void bar() {
;   #pragma omp target
;   #pragma omp teams
;   #pragma omp distribute parallel for
;   for (int j=0; j<100; j++) foo();
; }

; CHECK: %{{.*}} = call spir_func i64 @_Z14get_num_groupsj(i32 0)
; CHECK: %{{.*}} = call spir_func i64 @_Z12get_group_idj(i32 0)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

define dso_local spir_func void @bar() {
entry:
  %.omp.iv = alloca i32, align 4
  %0 = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %1 = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %2 = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %3 = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %j = alloca i32, align 4
  %4 = addrspacecast i32* %j to i32 addrspace(4)*
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %4),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %2),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %3),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %0),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %1) ]

  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %2),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %3),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %4),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %0),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %1) ]

  store i32 0, i32 addrspace(4)* %2, align 4
  store i32 99, i32 addrspace(4)* %3, align 4
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %2),
    "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %0),
    "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %3),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %4) ]

  %8 = load i32, i32 addrspace(4)* %2, align 4
  store i32 %8, i32 addrspace(4)* %0, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %9 = load i32, i32 addrspace(4)* %0, align 4
  %10 = load i32, i32 addrspace(4)* %3, align 4
  %cmp = icmp sle i32 %9, %10
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %11 = load i32, i32 addrspace(4)* %0, align 4
  %mul = mul nsw i32 %11, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %4, align 4
  %call = call spir_func i32 bitcast (i32 (...)* @foo to i32 ()*)()
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, i32 addrspace(4)* %0, align 4
  %add1 = add nsw i32 %12, 1
  store i32 %add1, i32 addrspace(4)* %0, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local spir_func i32 @foo(...)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2052, i32 85985690, !"bar", i32 2, i32 0, i32 0}

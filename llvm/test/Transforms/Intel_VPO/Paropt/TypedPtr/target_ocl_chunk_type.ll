; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-gpu-execution-scheme=0 -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -vpo-paropt-gpu-execution-scheme=0 -S %s | FileCheck %s

; void star1() {
;     #pragma omp target parallel for schedule(static,7)
;     for (long i=1; i<100; ++i) { }
; }
; Compiling it with -fiopenmp -fopenmp-targets=spir64 will require building
; MUL expressions involving the chunk size (i32 7) and other operands cast
; to IV's type (i64). Verify that this situation is properly handled and
; that no type-mismatch assertions occur.

; CHECK: [[NUMTHREADS:%[0-9]+]] = call spir_func i64 @_Z14get_local_sizej(i32 0)
; CHECK: %{{.*}} = mul i64 [[NUMTHREADS]], 7
;
; CHECK: [[LOCALID:%[0-9]+]] = call spir_func i64 @_Z12get_local_idj(i32 0)
; CHECK: %{{.*}} = mul i64 [[LOCALID]], 7

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

define dso_local spir_func void @star1() {
entry:
  %.omp.lb = alloca i64, align 8
  %0 = addrspacecast i64* %.omp.lb to i64 addrspace(4)*
  %.omp.ub = alloca i64, align 8
  %1 = addrspacecast i64* %.omp.ub to i64 addrspace(4)*
  %.omp.iv = alloca i64, align 8
  %2 = addrspacecast i64* %.omp.iv to i64 addrspace(4)*
  %tmp = alloca i64, align 8
  %3 = addrspacecast i64* %tmp to i64 addrspace(4)*
  %i = alloca i64, align 8
  %4 = addrspacecast i64* %i to i64 addrspace(4)*
  store i64 0, i64 addrspace(4)* %0, align 8
  store i64 98, i64 addrspace(4)* %1, align 8

  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE"(i64 addrspace(4)* %4),
    "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %0),
    "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %1),
    "QUAL.OMP.PRIVATE"(i64 addrspace(4)* %2),
    "QUAL.OMP.PRIVATE"(i64 addrspace(4)* %3) ]

  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SCHEDULE.STATIC"(i32 7),
    "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %0),
    "QUAL.OMP.NORMALIZED.IV"(i64 addrspace(4)* %2),
    "QUAL.OMP.NORMALIZED.UB"(i64 addrspace(4)* %1),
    "QUAL.OMP.PRIVATE"(i64 addrspace(4)* %4) ]

  %7 = load i64, i64 addrspace(4)* %0, align 8
  store i64 %7, i64 addrspace(4)* %2, align 8
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %8 = load i64, i64 addrspace(4)* %2, align 8
  %9 = load i64, i64 addrspace(4)* %1, align 8
  %cmp = icmp sle i64 %8, %9
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = load i64, i64 addrspace(4)* %2, align 8
  %mul = mul nsw i64 %10, 1
  %add = add nsw i64 1, %mul
  store i64 %add, i64 addrspace(4)* %4, align 8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %11 = load i64, i64 addrspace(4)* %2, align 8
  %add1 = add nsw i64 %11, 1
  store i64 %add1, i64 addrspace(4)* %2, align 8
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2052, i32 85985690, !"star1", i32 2, i32 0, i32 0}

; REQUIRES: asserts

; RUN: opt -opaque-pointers=0 -enable-new-pm=0 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-reduction-slm=true -S %s | FileCheck -check-prefixes=CHECK %s
; RUN: opt -opaque-pointers=0 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-reduction-slm=true -S %s | FileCheck -check-prefixes=CHECK %s

; Test src:
;
; int main(void) {
;   int i, sum = 0;
;
; #pragma omp target parallel map(tofrom:sum) shared(sum)
; {
;   #pragma omp for reduction (+:sum)
;   for (i = 0; i < 10; i++) {
;     sum += i;
;   }
; }
;   return 0;
; }

; This test checks if atomic-free reduction kicks in for parallel loop
; when only loop construct has reduction clause

; CHECK-LABEL: atomic.free.red.local.update.update.header:

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind optnone
define protected noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %sum = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %sum.ascast = addrspacecast i32* %sum to i32 addrspace(4)*
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %retval.ascast, align 4
  store i32 0, i32 addrspace(4)* %sum.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %sum.ascast, i32 addrspace(4)* %sum.ascast, i64 4, i64 35, i8* null, i8* null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %i.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED"(i32 addrspace(4)* %sum.ascast),
    "QUAL.OMP.SHARED"(i32 addrspace(4)* %i.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]

  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 9, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.REDUCTION.ADD"(i32 addrspace(4)* %sum.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast),
    "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast),
    "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast),
    "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast) ]

  %3 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %3, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %5 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4
  %7 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %8 = load i32, i32 addrspace(4)* %sum.ascast, align 4
  %add1 = add nsw i32 %8, %7
  store i32 %add1, i32 addrspace(4)* %sum.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add2 = add nsw i32 %9, 1
  store i32 %add2, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond
  
omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.LOOP"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 64773, i32 8803962, !"_Z4main", i32 5, i32 0, i32 0, i32 0}

; RUN: opt -opaque-pointers=0 -enable-new-pm=0 -switch-to-offload -vpo-paropt-assume-nonegative-iv -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -switch-to-offload -vpo-paropt-assume-nonegative-iv -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Original code:
; void foo(float *P, int I) {
; #pragma omp target teams distribute parallel for
;   for (int i = 0; i < I; ++i)
;     P[i] = i;
; }
;
; Check that paropt transform pass adds assumption that IV is nonnegative.

; CHECK: omp.inner.for.body:
; CHECK:   [[IV:%.+]] = phi i32
; CHECK:   [[CMP:%.+]] = icmp sge i32 [[IV]], 0
; CHECK:   call void @llvm.assume(i1 [[CMP]])

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define hidden spir_func void @foo(float addrspace(4)* %P, i32 %I) {
entry:
  %P.addr = alloca float addrspace(4)*, align 8
  %P.addr.ascast = addrspacecast float addrspace(4)** %P.addr to float addrspace(4)* addrspace(4)*
  %I.addr = alloca i32, align 4
  %I.addr.ascast = addrspacecast i32* %I.addr to i32 addrspace(4)*
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.0.ascast = addrspacecast i32* %.capture_expr.0 to i32 addrspace(4)*
  %.capture_expr.1 = alloca i32, align 4
  %.capture_expr.1.ascast = addrspacecast i32* %.capture_expr.1 to i32 addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %P.map.ptr.tmp = alloca float addrspace(4)*, align 8
  %P.map.ptr.tmp.ascast = addrspacecast float addrspace(4)** %P.map.ptr.tmp to float addrspace(4)* addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  store float addrspace(4)* %P, float addrspace(4)* addrspace(4)* %P.addr.ascast, align 8
  store i32 %I, i32 addrspace(4)* %I.addr.ascast, align 4
  %0 = load i32, i32 addrspace(4)* %I.addr.ascast, align 4
  store i32 %0, i32 addrspace(4)* %.capture_expr.0.ascast, align 4
  %1 = load i32, i32 addrspace(4)* %.capture_expr.0.ascast, align 4
  %sub = sub nsw i32 %1, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, i32 addrspace(4)* %.capture_expr.1.ascast, align 4
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  %2 = load i32, i32 addrspace(4)* %.capture_expr.1.ascast, align 4
  store i32 %2, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %3 = load float addrspace(4)*, float addrspace(4)* addrspace(4)* %P.addr.ascast, align 8
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(float addrspace(4)* %3, float addrspace(4)* %3, i64 0, i64 544, i8* null, i8* null), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.capture_expr.0.ascast), "QUAL.OMP.PRIVATE"(float addrspace(4)* addrspace(4)* %P.map.ptr.tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  store float addrspace(4)* %3, float addrspace(4)* addrspace(4)* %P.map.ptr.tmp.ascast, align 8
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.SHARED"(float addrspace(4)* addrspace(4)* %P.map.ptr.tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.capture_expr.0.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %6 = load i32, i32 addrspace(4)* %.capture_expr.0.ascast, align 4
  %cmp = icmp slt i32 0, %6
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.SHARED"(float addrspace(4)* addrspace(4)* %P.map.ptr.tmp.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast) ]
  %8 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %8, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %omp.precond.then
  %9 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %10 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp3 = icmp sle i32 %9, %10
  br i1 %cmp3, label %omp.inner.for.body, label %omp.loop.exit

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %11 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %11, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, i32 addrspace(4)* %i.ascast, align 4
  %12 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %conv = sitofp i32 %12 to float
  %13 = load float addrspace(4)*, float addrspace(4)* addrspace(4)* %P.map.ptr.tmp.ascast, align 8
  %14 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %idxprom = sext i32 %14 to i64
  %arrayidx = getelementptr inbounds float, float addrspace(4)* %13, i64 %idxprom
  store float %conv, float addrspace(4)* %arrayidx, align 4
  %15 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add5 = add nsw i32 %15, 1
  store i32 %add5, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 53, i32 -685103488, !"_Z3foo", i32 2, i32 0, i32 0}

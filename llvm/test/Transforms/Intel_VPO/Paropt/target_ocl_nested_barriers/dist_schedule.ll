; RUN: opt -bugpoint-enable-legacy-pm -xmain-opt-level=0 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -vpo-paropt -S %s | FileCheck %s --check-prefixes=CHECK,CHECK-O0
; RUN: opt -bugpoint-enable-legacy-pm -xmain-opt-level=2 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -vpo-paropt -S %s | FileCheck %s --check-prefixes=CHECK,CHECK-O2
; RUN: opt -xmain-opt-level=0 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -S %s | FileCheck %s --check-prefixes=CHECK,CHECK-O0
; RUN: opt -xmain-opt-level=2 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -S %s | FileCheck %s --check-prefixes=CHECK,CHECK-O2

; Original code:
;
; void test_dist_schedule(unsigned E) {
; #pragma omp target teams distribute dist_schedule(static)
;   for (int e = 0; e < E; e++) {
; #pragma omp parallel for
;     for (int i = 0; i < (6 * 9 * 9); i++) {}
; #pragma omp parallel for
;     for (int i = 0; i < (6 * 9 * 9); i++) {}
;   }
; }
;
; Check that paropt inserts barriers where necessary after 'omp parallel for' constructs.
;
; CHECK-LABEL: define weak dso_local spir_kernel void @__omp_offloading_35_d725aaff__Z18test_dist_schedule_l12(
; CHECK-O0-COUNT-2: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
; CHECK-O2-COUNT-2: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define hidden spir_func void @test_dist_schedule(i32 %E) {
entry:
  %E.addr = alloca i32, align 4
  %E.addr.ascast = addrspacecast ptr %E.addr to ptr addrspace(4)
  %.capture_expr.2 = alloca i32, align 4
  %.capture_expr.2.ascast = addrspacecast ptr %.capture_expr.2 to ptr addrspace(4)
  %.capture_expr.3 = alloca i32, align 4
  %.capture_expr.3.ascast = addrspacecast ptr %.capture_expr.3 to ptr addrspace(4)
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %e = alloca i32, align 4
  %e.ascast = addrspacecast ptr %e to ptr addrspace(4)
  %tmp6 = alloca i32, align 4
  %tmp6.ascast = addrspacecast ptr %tmp6 to ptr addrspace(4)
  %.omp.iv7 = alloca i32, align 4
  %.omp.iv7.ascast = addrspacecast ptr %.omp.iv7 to ptr addrspace(4)
  %.omp.lb8 = alloca i32, align 4
  %.omp.lb8.ascast = addrspacecast ptr %.omp.lb8 to ptr addrspace(4)
  %.omp.ub9 = alloca i32, align 4
  %.omp.ub9.ascast = addrspacecast ptr %.omp.ub9 to ptr addrspace(4)
  %i = alloca i32, align 4
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %tmp16 = alloca i32, align 4
  %tmp16.ascast = addrspacecast ptr %tmp16 to ptr addrspace(4)
  %.omp.iv17 = alloca i32, align 4
  %.omp.iv17.ascast = addrspacecast ptr %.omp.iv17 to ptr addrspace(4)
  %.omp.lb18 = alloca i32, align 4
  %.omp.lb18.ascast = addrspacecast ptr %.omp.lb18 to ptr addrspace(4)
  %.omp.ub19 = alloca i32, align 4
  %.omp.ub19.ascast = addrspacecast ptr %.omp.ub19 to ptr addrspace(4)
  %i23 = alloca i32, align 4
  %i23.ascast = addrspacecast ptr %i23 to ptr addrspace(4)
  store i32 %E, ptr addrspace(4) %E.addr.ascast, align 4
  %0 = load i32, ptr addrspace(4) %E.addr.ascast, align 4
  store i32 %0, ptr addrspace(4) %.capture_expr.2.ascast, align 4
  %1 = load i32, ptr addrspace(4) %.capture_expr.2.ascast, align 4
  %sub = sub i32 %1, 0
  %sub1 = sub i32 %sub, 1
  %add = add i32 %sub1, 1
  %div = udiv i32 %add, 1
  %sub2 = sub i32 %div, 1
  store i32 %sub2, ptr addrspace(4) %.capture_expr.3.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  %2 = load i32, ptr addrspace(4) %.capture_expr.3.ascast, align 4
  store i32 %2, ptr addrspace(4) %.omp.ub.ascast, align 4
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %e.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv7.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb8.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub9.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv17.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb18.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub19.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i23.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp6.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp16.ascast, i32 0, i32 1) ]

  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %e.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.capture_expr.2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv7.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb8.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub9.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv17.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb18.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub19.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i23.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp6.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp16.ascast, i32 0, i32 1) ]

  %5 = load i32, ptr addrspace(4) %.capture_expr.2.ascast, align 4
  %cmp = icmp ult i32 0, %5
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.DIST_SCHEDULE.STATIC"(i32 0),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %e.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv7.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb8.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub9.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv17.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb18.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub19.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i23.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp6.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp16.ascast, i32 0, i32 1) ]

  %7 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %7, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.loop.exit30, %omp.precond.then
  %8 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %9 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %add3 = add i32 %9, 1
  %cmp4 = icmp ult i32 %8, %add3
  br i1 %cmp4, label %omp.inner.for.body, label %omp.loop.exit35

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul i32 %10, 1
  %add5 = add i32 0, %mul
  store i32 %add5, ptr addrspace(4) %e.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb8.ascast, align 4
  store i32 485, ptr addrspace(4) %.omp.ub9.ascast, align 4
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv7.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb8.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub9.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1) ]

  %12 = load i32, ptr addrspace(4) %.omp.lb8.ascast, align 4
  store i32 %12, ptr addrspace(4) %.omp.iv7.ascast, align 4
  br label %omp.inner.for.cond10

omp.inner.for.cond10:                             ; preds = %omp.inner.for.body12, %omp.inner.for.body
  %13 = load i32, ptr addrspace(4) %.omp.iv7.ascast, align 4
  %14 = load i32, ptr addrspace(4) %.omp.ub9.ascast, align 4
  %cmp11 = icmp sle i32 %13, %14
  br i1 %cmp11, label %omp.inner.for.body12, label %omp.loop.exit

omp.inner.for.body12:                             ; preds = %omp.inner.for.cond10
  %15 = load i32, ptr addrspace(4) %.omp.iv7.ascast, align 4
  %mul13 = mul nsw i32 %15, 1
  %add14 = add nsw i32 0, %mul13
  store i32 %add14, ptr addrspace(4) %i.ascast, align 4
  %16 = load i32, ptr addrspace(4) %.omp.iv7.ascast, align 4
  %add15 = add nsw i32 %16, 1
  store i32 %add15, ptr addrspace(4) %.omp.iv7.ascast, align 4
  br label %omp.inner.for.cond10

omp.loop.exit:                                    ; preds = %omp.inner.for.cond10
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  store i32 0, ptr addrspace(4) %.omp.lb18.ascast, align 4
  store i32 485, ptr addrspace(4) %.omp.ub19.ascast, align 4
  %17 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv17.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb18.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub19.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i23.ascast, i32 0, i32 1) ]

  %18 = load i32, ptr addrspace(4) %.omp.lb18.ascast, align 4
  store i32 %18, ptr addrspace(4) %.omp.iv17.ascast, align 4
  br label %omp.inner.for.cond20

omp.inner.for.cond20:                             ; preds = %omp.inner.for.body22, %omp.loop.exit
  %19 = load i32, ptr addrspace(4) %.omp.iv17.ascast, align 4
  %20 = load i32, ptr addrspace(4) %.omp.ub19.ascast, align 4
  %cmp21 = icmp sle i32 %19, %20
  br i1 %cmp21, label %omp.inner.for.body22, label %omp.loop.exit30

omp.inner.for.body22:                             ; preds = %omp.inner.for.cond20
  %21 = load i32, ptr addrspace(4) %.omp.iv17.ascast, align 4
  %mul24 = mul nsw i32 %21, 1
  %add25 = add nsw i32 0, %mul24
  store i32 %add25, ptr addrspace(4) %i23.ascast, align 4
  %22 = load i32, ptr addrspace(4) %.omp.iv17.ascast, align 4
  %add28 = add nsw i32 %22, 1
  store i32 %add28, ptr addrspace(4) %.omp.iv17.ascast, align 4
  br label %omp.inner.for.cond20

omp.loop.exit30:                                  ; preds = %omp.inner.for.cond20
  call void @llvm.directive.region.exit(token %17) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  %23 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add33 = add nuw i32 %23, 1
  store i32 %add33, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.loop.exit35:                                  ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.DISTRIBUTE"() ]

  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit35, %entry
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 53, i32 -685397249, !"_Z18test_dist_schedule", i32 12, i32 0, i32 0}

; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=vpo-paropt-transform -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload --debug-only=vpo-paropt-transform -S %s 2>&1 | FileCheck %s

; Original code:
; #pragma omp declare target
; void test(int *a, const int n, int *res) {
;         #pragma omp target teams distribute parallel for
;         for (int i = 0; i < n; ++i)
;                 res[i] += a[i];
; }
; #pragma omp end declare target

; This test checks that no assertion fired while propagating loop's KNOWN.NDRANGE up to the enclosing target region.
; As for target-declare functions TARGET construct is removed at the beginning of transform pass, that case
; should be ignored by the propagation routine and not cause a crash.

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(){{.*}}QUAL.OMP.OFFLOAD.KNOWN.NDRANGE
; CHECK-NOT: Removing OFFLOAD.KNOWN.NDRANGE qual

; Function Attrs: convergent mustprogress noinline nounwind optnone
define hidden spir_func void @_Z4testPiiS_(ptr addrspace(4) noundef %a, i32 noundef %n, ptr addrspace(4) noundef %res) #0 {
entry:
  %a.addr = alloca ptr addrspace(4), align 8
  %n.addr = alloca i32, align 4
  %res.addr = alloca ptr addrspace(4), align 8
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %res.map.ptr.tmp = alloca ptr addrspace(4), align 8
  %a.map.ptr.tmp = alloca ptr addrspace(4), align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  %a.addr.ascast = addrspacecast ptr %a.addr to ptr addrspace(4)
  %n.addr.ascast = addrspacecast ptr %n.addr to ptr addrspace(4)
  %res.addr.ascast = addrspacecast ptr %res.addr to ptr addrspace(4)
  %.capture_expr.0.ascast = addrspacecast ptr %.capture_expr.0 to ptr addrspace(4)
  %.capture_expr.1.ascast = addrspacecast ptr %.capture_expr.1 to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %res.map.ptr.tmp.ascast = addrspacecast ptr %res.map.ptr.tmp to ptr addrspace(4)
  %a.map.ptr.tmp.ascast = addrspacecast ptr %a.map.ptr.tmp to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  store ptr addrspace(4) %a, ptr addrspace(4) %a.addr.ascast, align 8
  store i32 %n, ptr addrspace(4) %n.addr.ascast, align 4
  store ptr addrspace(4) %res, ptr addrspace(4) %res.addr.ascast, align 8
  %0 = load i32, ptr addrspace(4) %n.addr.ascast, align 4
  store i32 %0, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %1 = load i32, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %sub = sub nsw i32 %1, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr addrspace(4) %.capture_expr.1.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  %2 = load i32, ptr addrspace(4) %.capture_expr.1.ascast, align 4
  store i32 %2, ptr addrspace(4) %.omp.ub.ascast, align 4
  %3 = load ptr addrspace(4), ptr addrspace(4) %res.addr.ascast, align 8
  %4 = load ptr addrspace(4), ptr addrspace(4) %a.addr.ascast, align 8
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %3, ptr addrspace(4) %3, i64 0, i64 544, ptr null, ptr null), ; MAP type: 544 = 0x220 = IMPLICIT (0x200) | TARGET_PARAM (0x20)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %4, ptr addrspace(4) %4, i64 0, i64 544, ptr null, ptr null), ; MAP type: 544 = 0x220 = IMPLICIT (0x200) | TARGET_PARAM (0x20)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %res.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %a.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  store ptr addrspace(4) %3, ptr addrspace(4) %res.map.ptr.tmp.ascast, align 8
  store ptr addrspace(4) %4, ptr addrspace(4) %a.map.ptr.tmp.ascast, align 8
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %res.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.capture_expr.0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  %7 = load i32, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %cmp = icmp slt i32 0, %7
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %res.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1) ]

  %9 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %9, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %10 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %11 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp3 = icmp sle i32 %10, %11
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %12 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %12, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, ptr addrspace(4) %i.ascast, align 4
  %13 = load ptr addrspace(4), ptr addrspace(4) %a.map.ptr.tmp.ascast, align 8
  %14 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom = sext i32 %14 to i64
  %arrayidx = getelementptr inbounds i32, ptr addrspace(4) %13, i64 %idxprom
  %15 = load i32, ptr addrspace(4) %arrayidx, align 4
  %16 = load ptr addrspace(4), ptr addrspace(4) %res.map.ptr.tmp.ascast, align 8
  %17 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom5 = sext i32 %17 to i64
  %arrayidx6 = getelementptr inbounds i32, ptr addrspace(4) %16, i64 %idxprom5
  %18 = load i32, ptr addrspace(4) %arrayidx6, align 4
  %add7 = add nsw i32 %18, %15
  store i32 %add7, ptr addrspace(4) %arrayidx6, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %19 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add8 = add nsw i32 %19, 1
  store i32 %add8, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

attributes #0 = { "contains-openmp-target"="true" "openmp-target-declare"="true" }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2050, i32 52691103, !"_Z4testPiiS_", i32 3, i32 0, i32 0, i32 0}

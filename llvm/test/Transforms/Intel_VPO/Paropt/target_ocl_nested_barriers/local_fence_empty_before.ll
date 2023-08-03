; RUN: opt -xmain-opt-level=2 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -S %s | FileCheck %s --check-prefixes=CHECK,CHECK-NOOPT
; RUN: opt -xmain-opt-level=3 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -S %s | FileCheck %s --check-prefixes=CHECK,CHECK-OPT
; RUN: opt -xmain-opt-level=3 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -vpo-paropt-simplify-workgroup-barrier-fences=false -S %s | FileCheck %s --check-prefixes=CHECK,CHECK-NOOPT
;
; Original test src:
;
; void test4(int *X) {
; #pragma omp target teams firstprivate(X)
;   {
; #pragma omp distribute parallel for
;     for (int I = 0; I < 5; ++I) {}
;     // barrier with local fence
; #pragma omp distribute parallel for
;     for (int I = 0; I < 5; ++I) { X[I] = I; }
;     // no barrier
;   }
; }
;
; Check that paropt avoids generating global fences where it is legal to do so.

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define hidden spir_func void @test4(ptr addrspace(4) %X) {
; CHECK-LABEL: @__omp_offloading_87_d6575405__Z5test4_l40(
; CHECK-OPT-COUNT-1:   call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 272)
; CHECK-NOOPT-COUNT-1: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
entry:
  %X.addr = alloca ptr addrspace(4), align 8
  %X.map.ptr.tmp = alloca ptr addrspace(4), align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  %tmp2 = alloca i32, align 4
  %.omp.iv3 = alloca i32, align 4
  %.omp.lb4 = alloca i32, align 4
  %.omp.ub5 = alloca i32, align 4
  %I9 = alloca i32, align 4
  %X.addr.ascast = addrspacecast ptr %X.addr to ptr addrspace(4)
  %X.map.ptr.tmp.ascast = addrspacecast ptr %X.map.ptr.tmp to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %I.ascast = addrspacecast ptr %I to ptr addrspace(4)
  %tmp2.ascast = addrspacecast ptr %tmp2 to ptr addrspace(4)
  %.omp.iv3.ascast = addrspacecast ptr %.omp.iv3 to ptr addrspace(4)
  %.omp.lb4.ascast = addrspacecast ptr %.omp.lb4 to ptr addrspace(4)
  %.omp.ub5.ascast = addrspacecast ptr %.omp.ub5 to ptr addrspace(4)
  %I9.ascast = addrspacecast ptr %I9 to ptr addrspace(4)
  store ptr addrspace(4) %X, ptr addrspace(4) %X.addr.ascast, align 8
  %0 = load ptr addrspace(4), ptr addrspace(4) %X.addr.ascast, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %X.addr.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %0, ptr addrspace(4) %0, i64 0, i64 32, ptr null, ptr null), ; MAP type: 32 = 0x20 = TARGET_PARAM (0x20)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %I.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %I9.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %X.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp2.ascast, i32 0, i32 1) ]

  store ptr addrspace(4) %0, ptr addrspace(4) %X.map.ptr.tmp.ascast, align 8
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %X.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %I.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %I9.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp2.ascast, i32 0, i32 1) ]

  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 4, ptr addrspace(4) %.omp.ub.ascast, align 4
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %I.ascast, i32 0, i32 1) ]

  %4 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %4, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %5 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %6 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %5, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %I.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add1 = add nsw i32 %8, 1
  store i32 %add1, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  store i32 0, ptr addrspace(4) %.omp.lb4.ascast, align 4
  store i32 4, ptr addrspace(4) %.omp.ub5.ascast, align 4
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %X.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv3.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb4.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub5.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %I9.ascast, i32 0, i32 1) ]

  %10 = load i32, ptr addrspace(4) %.omp.lb4.ascast, align 4
  store i32 %10, ptr addrspace(4) %.omp.iv3.ascast, align 4
  br label %omp.inner.for.cond6

omp.inner.for.cond6:                              ; preds = %omp.inner.for.inc13, %omp.loop.exit
  %11 = load i32, ptr addrspace(4) %.omp.iv3.ascast, align 4
  %12 = load i32, ptr addrspace(4) %.omp.ub5.ascast, align 4
  %cmp7 = icmp sle i32 %11, %12
  br i1 %cmp7, label %omp.inner.for.body8, label %omp.inner.for.end15

omp.inner.for.body8:                              ; preds = %omp.inner.for.cond6
  %13 = load i32, ptr addrspace(4) %.omp.iv3.ascast, align 4
  %mul10 = mul nsw i32 %13, 1
  %add11 = add nsw i32 0, %mul10
  store i32 %add11, ptr addrspace(4) %I9.ascast, align 4
  %14 = load i32, ptr addrspace(4) %I9.ascast, align 4
  %15 = load ptr addrspace(4), ptr addrspace(4) %X.map.ptr.tmp.ascast, align 8
  %16 = load i32, ptr addrspace(4) %I9.ascast, align 4
  %idxprom = sext i32 %16 to i64
  %arrayidx = getelementptr inbounds i32, ptr addrspace(4) %15, i64 %idxprom
  store i32 %14, ptr addrspace(4) %arrayidx, align 4
  br label %omp.body.continue12

omp.body.continue12:                              ; preds = %omp.inner.for.body8
  br label %omp.inner.for.inc13

omp.inner.for.inc13:                              ; preds = %omp.body.continue12
  %17 = load i32, ptr addrspace(4) %.omp.iv3.ascast, align 4
  %add14 = add nsw i32 %17, 1
  store i32 %add14, ptr addrspace(4) %.omp.iv3.ascast, align 4
  br label %omp.inner.for.cond6

omp.inner.for.end15:                              ; preds = %omp.inner.for.cond6
  br label %omp.loop.exit16

omp.loop.exit16:                                  ; preds = %omp.inner.for.end15
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 135, i32 -698919931, !"_Z5test4", i32 40, i32 0, i32 0}

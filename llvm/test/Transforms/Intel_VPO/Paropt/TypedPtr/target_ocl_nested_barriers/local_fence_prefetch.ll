; RUN: opt -opaque-pointers=0 -xmain-opt-level=2 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -S %s | FileCheck %s --check-prefixes=CHECK,CHECK-NOOPT
; RUN: opt -opaque-pointers=0 -xmain-opt-level=3 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -S %s | FileCheck %s --check-prefixes=CHECK,CHECK-OPT
; RUN: opt -opaque-pointers=0 -xmain-opt-level=3 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -vpo-paropt-simplify-workgroup-barrier-fences=false -S %s | FileCheck %s --check-prefixes=CHECK,CHECK-NOOPT
;
; Original test src:
;
; void test5(int *X) {
; #pragma omp target teams firstprivate(X)
;   {
; #pragma omp distribute parallel for
;     for (int I = 0; I < 5; ++I) {
;       __builtin_prefetch(&X[I], 1, 3);
;       __builtin_spirv_OpenCL_prefetch_p1i32_i32(&X[I], 1);
;       __builtin_IB_lsc_prefetch_global_uint(&X[I], 0, 0);
;     }
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

define hidden spir_func void @test5(i32 addrspace(4)* %X) {
; CHECK-LABEL: @__omp_offloading_87_d6575405__Z5test5_l52(
; CHECK-OPT-COUNT-1:   call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 272)
; CHECK-NOOPT-COUNT-1: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
entry:
  %X.addr = alloca i32 addrspace(4)*, align 8
  %X.map.ptr.tmp = alloca i32 addrspace(4)*, align 8
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
  %X.addr.ascast = addrspacecast i32 addrspace(4)** %X.addr to i32 addrspace(4)* addrspace(4)*
  %X.map.ptr.tmp.ascast = addrspacecast i32 addrspace(4)** %X.map.ptr.tmp to i32 addrspace(4)* addrspace(4)*
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %I.ascast = addrspacecast i32* %I to i32 addrspace(4)*
  %tmp2.ascast = addrspacecast i32* %tmp2 to i32 addrspace(4)*
  %.omp.iv3.ascast = addrspacecast i32* %.omp.iv3 to i32 addrspace(4)*
  %.omp.lb4.ascast = addrspacecast i32* %.omp.lb4 to i32 addrspace(4)*
  %.omp.ub5.ascast = addrspacecast i32* %.omp.ub5 to i32 addrspace(4)*
  %I9.ascast = addrspacecast i32* %I9 to i32 addrspace(4)*
  store i32 addrspace(4)* %X, i32 addrspace(4)* addrspace(4)* %X.addr.ascast, align 8
  %0 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %X.addr.ascast, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* addrspace(4)* %X.addr.ascast),
    "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %0, i32 addrspace(4)* %0, i64 0, i64 32, i8* null, i8* null), ; MAP type: 32 = 0x20 = TARGET_PARAM (0x20)
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv3.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb4.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub5.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I9.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspace(4)* %X.map.ptr.tmp.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp2.ascast) ]

  store i32 addrspace(4)* %0, i32 addrspace(4)* addrspace(4)* %X.map.ptr.tmp.ascast, align 8
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* addrspace(4)* %X.map.ptr.tmp.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv3.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb4.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub5.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I9.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp2.ascast) ]

  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 4, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast),
    "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast),
    "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I.ascast) ]

  %4 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %4, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %5 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %6 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %5, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %7, 1
  %X_addr = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %X.map.ptr.tmp.ascast, align 8
  %pref_addr = getelementptr inbounds i32, i32 addrspace(4)* %X_addr, i32 %7
  call void @llvm.prefetch(i32 addrspace(4)* %pref_addr, i32 1, i32 0, i32 1)
  call void @__builtin_spirv_OpenCL_prefetch_p1i32_i32(i32 addrspace(4)* %pref_addr, i32 1)
  call void @__builtin_IB_lsc_prefetch_global_uint(i32 addrspace(4)* %pref_addr, i32 0, i32 0)
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %I.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add1 = add nsw i32 %8, 1
  store i32 %add1, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  store i32 0, i32 addrspace(4)* %.omp.lb4.ascast, align 4
  store i32 4, i32 addrspace(4)* %.omp.ub5.ascast, align 4
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspace(4)* %X.map.ptr.tmp.ascast),
    "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv3.ascast),
    "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb4.ascast),
    "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub5.ascast),
    "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I9.ascast) ]

  %10 = load i32, i32 addrspace(4)* %.omp.lb4.ascast, align 4
  store i32 %10, i32 addrspace(4)* %.omp.iv3.ascast, align 4
  br label %omp.inner.for.cond6

omp.inner.for.cond6:                              ; preds = %omp.inner.for.inc13, %omp.loop.exit
  %11 = load i32, i32 addrspace(4)* %.omp.iv3.ascast, align 4
  %12 = load i32, i32 addrspace(4)* %.omp.ub5.ascast, align 4
  %cmp7 = icmp sle i32 %11, %12
  br i1 %cmp7, label %omp.inner.for.body8, label %omp.inner.for.end15

omp.inner.for.body8:                              ; preds = %omp.inner.for.cond6
  %13 = load i32, i32 addrspace(4)* %.omp.iv3.ascast, align 4
  %mul10 = mul nsw i32 %13, 1
  %add11 = add nsw i32 0, %mul10
  store i32 %add11, i32 addrspace(4)* %I9.ascast, align 4
  %14 = load i32, i32 addrspace(4)* %I9.ascast, align 4
  %15 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %X.map.ptr.tmp.ascast, align 8
  %16 = load i32, i32 addrspace(4)* %I9.ascast, align 4
  %idxprom = sext i32 %16 to i64
  %arrayidx = getelementptr inbounds i32, i32 addrspace(4)* %15, i64 %idxprom
  store i32 %14, i32 addrspace(4)* %arrayidx, align 4
  br label %omp.body.continue12

omp.body.continue12:                              ; preds = %omp.inner.for.body8
  br label %omp.inner.for.inc13

omp.inner.for.inc13:                              ; preds = %omp.body.continue12
  %17 = load i32, i32 addrspace(4)* %.omp.iv3.ascast, align 4
  %add14 = add nsw i32 %17, 1
  store i32 %add14, i32 addrspace(4)* %.omp.iv3.ascast, align 4
  br label %omp.inner.for.cond6

omp.inner.for.end15:                              ; preds = %omp.inner.for.cond6
  br label %omp.loop.exit16

omp.loop.exit16:                                  ; preds = %omp.inner.for.end15
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare void @llvm.prefetch(i32 addrspace(4)*, i32, i32, i32)
declare void @__builtin_spirv_OpenCL_prefetch_p1i32_i32(i32 addrspace(4)*, i32)
declare void @__builtin_IB_lsc_prefetch_global_uint(i32 addrspace(4)*, i32, i32)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 135, i32 -698919931, !"_Z5test5", i32 52, i32 0, i32 0}

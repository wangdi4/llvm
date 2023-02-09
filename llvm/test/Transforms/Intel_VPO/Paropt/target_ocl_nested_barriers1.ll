; RUN: opt -opaque-pointers=0 -enable-new-pm=0 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -S %s | FileCheck %s
;
; Original test src:
;
; void test1() {
; #pragma omp target teams
;   {
; #pragma omp distribute parallel for
;     for (int I = 0; I < 5; ++I) {}
;     // barrier
; #pragma omp distribute parallel for
;     for (int I = 0; I < 5; ++I) {}
;     // no barrier
;   }
; }
;
; void test2(int X) {
; #pragma omp target teams
;   {
;     if (X) {
; #pragma omp distribute parallel for
;       for (int I = 0; I < 5; ++I) {}
;       // barrier
;     }
; #pragma omp distribute parallel for
;     for (int I = 0; I < 5; ++I) {}
;     // no barrier
;   }
; }
;
; void test3(int X) {
; #pragma omp target teams
;   {
;     if (X) {
; #pragma omp distribute parallel for
;       for (int I = 0; I < 5; ++I) {}
;       // no barrier
;     } else {
; #pragma omp distribute parallel for
;       for (int I = 0; I < 5; ++I) {}
;       // no barrier
;     }
;   }
; }
;
; void test4() {
; #pragma omp target teams
;   {
; #pragma omp distribute parallel for
;     for (int I = 0; I < 5; ++I) {
; #pragma omp simd
;       for (int J = 0; J < 5; ++J) {}
; #pragma omp simd
;       for (int J = 0; J < 5; ++J) {}
;     }
;     // no barrier
;   }
; }

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define hidden spir_func void @test1() {
; CHECK-LABEL: @__omp_offloading_35_d77efd39__Z5test1_l2(
; CHECK-COUNT-1: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
entry:
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %I = alloca i32, align 4
  %I.ascast = addrspacecast i32* %I to i32 addrspace(4)*
  %tmp2 = alloca i32, align 4
  %tmp2.ascast = addrspacecast i32* %tmp2 to i32 addrspace(4)*
  %.omp.iv3 = alloca i32, align 4
  %.omp.iv3.ascast = addrspacecast i32* %.omp.iv3 to i32 addrspace(4)*
  %.omp.lb4 = alloca i32, align 4
  %.omp.lb4.ascast = addrspacecast i32* %.omp.lb4 to i32 addrspace(4)*
  %.omp.ub5 = alloca i32, align 4
  %.omp.ub5.ascast = addrspacecast i32* %.omp.ub5 to i32 addrspace(4)*
  %I9 = alloca i32, align 4
  %I9.ascast = addrspacecast i32* %I9 to i32 addrspace(4)*
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv3.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb4.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub5.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I9.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp2.ascast) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv3.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb4.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub5.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I9.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp2.ascast) ]
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 4, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I.ascast) ]
  %3 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %3, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %entry
  %4 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %5 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.loop.exit

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %I.ascast, align 4
  %7 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add1 = add nsw i32 %7, 1
  store i32 %add1, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  store i32 0, i32 addrspace(4)* %.omp.lb4.ascast, align 4
  store i32 4, i32 addrspace(4)* %.omp.ub5.ascast, align 4
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv3.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb4.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub5.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I9.ascast) ]
  %9 = load i32, i32 addrspace(4)* %.omp.lb4.ascast, align 4
  store i32 %9, i32 addrspace(4)* %.omp.iv3.ascast, align 4
  br label %omp.inner.for.cond6

omp.inner.for.cond6:                              ; preds = %omp.inner.for.body8, %omp.loop.exit
  %10 = load i32, i32 addrspace(4)* %.omp.iv3.ascast, align 4
  %11 = load i32, i32 addrspace(4)* %.omp.ub5.ascast, align 4
  %cmp7 = icmp sle i32 %10, %11
  br i1 %cmp7, label %omp.inner.for.body8, label %omp.loop.exit16

omp.inner.for.body8:                              ; preds = %omp.inner.for.cond6
  %12 = load i32, i32 addrspace(4)* %.omp.iv3.ascast, align 4
  %mul10 = mul nsw i32 %12, 1
  %add11 = add nsw i32 0, %mul10
  store i32 %add11, i32 addrspace(4)* %I9.ascast, align 4
  %13 = load i32, i32 addrspace(4)* %.omp.iv3.ascast, align 4
  %add14 = add nsw i32 %13, 1
  store i32 %add14, i32 addrspace(4)* %.omp.iv3.ascast, align 4
  br label %omp.inner.for.cond6

omp.loop.exit16:                                  ; preds = %omp.inner.for.cond6
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

define hidden spir_func void @test2(i32 %X) {
; CHECK-LABEL: @__omp_offloading_35_d77efd39__Z5test2_l14(
; CHECK-COUNT-1: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
entry:
  %X.addr = alloca i32, align 4
  %X.addr.ascast = addrspacecast i32* %X.addr to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %I = alloca i32, align 4
  %I.ascast = addrspacecast i32* %I to i32 addrspace(4)*
  %tmp2 = alloca i32, align 4
  %tmp2.ascast = addrspacecast i32* %tmp2 to i32 addrspace(4)*
  %.omp.iv3 = alloca i32, align 4
  %.omp.iv3.ascast = addrspacecast i32* %.omp.iv3 to i32 addrspace(4)*
  %.omp.lb4 = alloca i32, align 4
  %.omp.lb4.ascast = addrspacecast i32* %.omp.lb4 to i32 addrspace(4)*
  %.omp.ub5 = alloca i32, align 4
  %.omp.ub5.ascast = addrspacecast i32* %.omp.ub5 to i32 addrspace(4)*
  %I9 = alloca i32, align 4
  %I9.ascast = addrspacecast i32* %I9 to i32 addrspace(4)*
  store i32 %X, i32 addrspace(4)* %X.addr.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %X.addr.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv3.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb4.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub5.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I9.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp2.ascast) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.SHARED"(i32 addrspace(4)* %X.addr.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv3.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb4.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub5.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I9.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp2.ascast) ]
  %2 = load i32, i32 addrspace(4)* %X.addr.ascast, align 4
  %tobool = icmp ne i32 %2, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 4, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I.ascast) ]
  %4 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %4, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %if.then
  %5 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %6 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %5, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.loop.exit

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %I.ascast, align 4
  %8 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add1 = add nsw i32 %8, 1
  store i32 %add1, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  br label %if.end

if.end:                                           ; preds = %omp.loop.exit, %entry
  store i32 0, i32 addrspace(4)* %.omp.lb4.ascast, align 4
  store i32 4, i32 addrspace(4)* %.omp.ub5.ascast, align 4
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv3.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb4.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub5.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I9.ascast) ]
  %10 = load i32, i32 addrspace(4)* %.omp.lb4.ascast, align 4
  store i32 %10, i32 addrspace(4)* %.omp.iv3.ascast, align 4
  br label %omp.inner.for.cond6

omp.inner.for.cond6:                              ; preds = %omp.inner.for.body8, %if.end
  %11 = load i32, i32 addrspace(4)* %.omp.iv3.ascast, align 4
  %12 = load i32, i32 addrspace(4)* %.omp.ub5.ascast, align 4
  %cmp7 = icmp sle i32 %11, %12
  br i1 %cmp7, label %omp.inner.for.body8, label %omp.loop.exit16

omp.inner.for.body8:                              ; preds = %omp.inner.for.cond6
  %13 = load i32, i32 addrspace(4)* %.omp.iv3.ascast, align 4
  %mul10 = mul nsw i32 %13, 1
  %add11 = add nsw i32 0, %mul10
  store i32 %add11, i32 addrspace(4)* %I9.ascast, align 4
  %14 = load i32, i32 addrspace(4)* %.omp.iv3.ascast, align 4
  %add14 = add nsw i32 %14, 1
  store i32 %add14, i32 addrspace(4)* %.omp.iv3.ascast, align 4
  br label %omp.inner.for.cond6

omp.loop.exit16:                                  ; preds = %omp.inner.for.cond6
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

define hidden spir_func void @test3(i32 %X) {
; CHECK-LABEL: @__omp_offloading_35_d77efd39__Z5test3_l28(
; CHECK-NO: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
entry:
  %X.addr = alloca i32, align 4
  %X.addr.ascast = addrspacecast i32* %X.addr to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %I = alloca i32, align 4
  %I.ascast = addrspacecast i32* %I to i32 addrspace(4)*
  %tmp2 = alloca i32, align 4
  %tmp2.ascast = addrspacecast i32* %tmp2 to i32 addrspace(4)*
  %.omp.iv3 = alloca i32, align 4
  %.omp.iv3.ascast = addrspacecast i32* %.omp.iv3 to i32 addrspace(4)*
  %.omp.lb4 = alloca i32, align 4
  %.omp.lb4.ascast = addrspacecast i32* %.omp.lb4 to i32 addrspace(4)*
  %.omp.ub5 = alloca i32, align 4
  %.omp.ub5.ascast = addrspacecast i32* %.omp.ub5 to i32 addrspace(4)*
  %I9 = alloca i32, align 4
  %I9.ascast = addrspacecast i32* %I9 to i32 addrspace(4)*
  store i32 %X, i32 addrspace(4)* %X.addr.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %X.addr.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv3.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb4.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub5.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I9.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp2.ascast) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.SHARED"(i32 addrspace(4)* %X.addr.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv3.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb4.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub5.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I9.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp2.ascast) ]
  %2 = load i32, i32 addrspace(4)* %X.addr.ascast, align 4
  %tobool = icmp ne i32 %2, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 4, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I.ascast) ]
  %4 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %4, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %if.then
  %5 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %6 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %5, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.loop.exit

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %I.ascast, align 4
  %8 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add1 = add nsw i32 %8, 1
  store i32 %add1, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  br label %if.end

if.else:                                          ; preds = %entry
  store i32 0, i32 addrspace(4)* %.omp.lb4.ascast, align 4
  store i32 4, i32 addrspace(4)* %.omp.ub5.ascast, align 4
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv3.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb4.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub5.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I9.ascast) ]
  %10 = load i32, i32 addrspace(4)* %.omp.lb4.ascast, align 4
  store i32 %10, i32 addrspace(4)* %.omp.iv3.ascast, align 4
  br label %omp.inner.for.cond6

omp.inner.for.cond6:                              ; preds = %omp.inner.for.body8, %if.else
  %11 = load i32, i32 addrspace(4)* %.omp.iv3.ascast, align 4
  %12 = load i32, i32 addrspace(4)* %.omp.ub5.ascast, align 4
  %cmp7 = icmp sle i32 %11, %12
  br i1 %cmp7, label %omp.inner.for.body8, label %omp.loop.exit16

omp.inner.for.body8:                              ; preds = %omp.inner.for.cond6
  %13 = load i32, i32 addrspace(4)* %.omp.iv3.ascast, align 4
  %mul10 = mul nsw i32 %13, 1
  %add11 = add nsw i32 0, %mul10
  store i32 %add11, i32 addrspace(4)* %I9.ascast, align 4
  %14 = load i32, i32 addrspace(4)* %.omp.iv3.ascast, align 4
  %add14 = add nsw i32 %14, 1
  store i32 %add14, i32 addrspace(4)* %.omp.iv3.ascast, align 4
  br label %omp.inner.for.cond6

omp.loop.exit16:                                  ; preds = %omp.inner.for.cond6
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  br label %if.end

if.end:                                           ; preds = %omp.loop.exit16, %omp.loop.exit
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

define hidden spir_func void @test4() {
; CHECK-LABEL: @__omp_offloading_35_d77efd39__Z5test4_l43(
; CHECK-NO: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
entry:
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %I = alloca i32, align 4
  %I.ascast = addrspacecast i32* %I to i32 addrspace(4)*
  %tmp1 = alloca i32, align 4
  %tmp1.ascast = addrspacecast i32* %tmp1 to i32 addrspace(4)*
  %.omp.iv2 = alloca i32, align 4
  %.omp.iv2.ascast = addrspacecast i32* %.omp.iv2 to i32 addrspace(4)*
  %.omp.ub3 = alloca i32, align 4
  %.omp.ub3.ascast = addrspacecast i32* %.omp.ub3 to i32 addrspace(4)*
  %J = alloca i32, align 4
  %J.ascast = addrspacecast i32* %J to i32 addrspace(4)*
  %tmp10 = alloca i32, align 4
  %tmp10.ascast = addrspacecast i32* %tmp10 to i32 addrspace(4)*
  %.omp.iv11 = alloca i32, align 4
  %.omp.iv11.ascast = addrspacecast i32* %.omp.iv11 to i32 addrspace(4)*
  %.omp.ub12 = alloca i32, align 4
  %.omp.ub12.ascast = addrspacecast i32* %.omp.ub12 to i32 addrspace(4)*
  %J16 = alloca i32, align 4
  %J16.ascast = addrspacecast i32* %J16 to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 4, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 3), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv2.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub3.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %J.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv11.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub12.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %J16.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp1.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp10.ascast) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv2.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub3.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %J.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv11.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub12.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %J16.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp1.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp10.ascast) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %I.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv2.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub3.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %J.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv11.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub12.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %J16.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp1.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp10.ascast) ]
  %3 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %3, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.loop.exit23, %entry
  %4 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %5 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.loop.exit28

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %I.ascast, align 4
  store i32 4, i32 addrspace(4)* %.omp.ub3.ascast, align 4
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv2.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub3.ascast), "QUAL.OMP.LINEAR:IV"(i32 addrspace(4)* %J.ascast, i32 1) ]
  store i32 0, i32 addrspace(4)* %.omp.iv2.ascast, align 4
  br label %omp.inner.for.cond4

omp.inner.for.cond4:                              ; preds = %omp.inner.for.body6, %omp.inner.for.body
  %8 = load i32, i32 addrspace(4)* %.omp.iv2.ascast, align 4
  %9 = load i32, i32 addrspace(4)* %.omp.ub3.ascast, align 4
  %cmp5 = icmp sle i32 %8, %9
  br i1 %cmp5, label %omp.inner.for.body6, label %omp.loop.exit

omp.inner.for.body6:                              ; preds = %omp.inner.for.cond4
  %10 = load i32, i32 addrspace(4)* %.omp.iv2.ascast, align 4
  %mul7 = mul nsw i32 %10, 1
  %add8 = add nsw i32 0, %mul7
  store i32 %add8, i32 addrspace(4)* %J.ascast, align 4
  %11 = load i32, i32 addrspace(4)* %.omp.iv2.ascast, align 4
  %add9 = add nsw i32 %11, 1
  store i32 %add9, i32 addrspace(4)* %.omp.iv2.ascast, align 4
  br label %omp.inner.for.cond4

omp.loop.exit:                                    ; preds = %omp.inner.for.cond4
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.SIMD"() ]
  store i32 4, i32 addrspace(4)* %.omp.ub12.ascast, align 4
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv11.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub12.ascast), "QUAL.OMP.LINEAR:IV"(i32 addrspace(4)* %J16.ascast, i32 1) ]
  store i32 0, i32 addrspace(4)* %.omp.iv11.ascast, align 4
  br label %omp.inner.for.cond13

omp.inner.for.cond13:                             ; preds = %omp.inner.for.body15, %omp.loop.exit
  %13 = load i32, i32 addrspace(4)* %.omp.iv11.ascast, align 4
  %14 = load i32, i32 addrspace(4)* %.omp.ub12.ascast, align 4
  %cmp14 = icmp sle i32 %13, %14
  br i1 %cmp14, label %omp.inner.for.body15, label %omp.loop.exit23

omp.inner.for.body15:                             ; preds = %omp.inner.for.cond13
  %15 = load i32, i32 addrspace(4)* %.omp.iv11.ascast, align 4
  %mul17 = mul nsw i32 %15, 1
  %add18 = add nsw i32 0, %mul17
  store i32 %add18, i32 addrspace(4)* %J16.ascast, align 4
  %16 = load i32, i32 addrspace(4)* %.omp.iv11.ascast, align 4
  %add21 = add nsw i32 %16, 1
  store i32 %add21, i32 addrspace(4)* %.omp.iv11.ascast, align 4
  br label %omp.inner.for.cond13

omp.loop.exit23:                                  ; preds = %omp.inner.for.cond13
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.SIMD"() ]
  %17 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add26 = add nsw i32 %17, 1
  store i32 %add26, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.loop.exit28:                                  ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0, !1, !2, !3}

!0 = !{i32 0, i32 53, i32 -679543495, !"_Z5test1", i32 2, i32 0, i32 0}
!1 = !{i32 0, i32 53, i32 -679543495, !"_Z5test2", i32 14, i32 1, i32 0}
!2 = !{i32 0, i32 53, i32 -679543495, !"_Z5test3", i32 28, i32 2, i32 0}
!3 = !{i32 0, i32 53, i32 -679543495, !"_Z5test4", i32 43, i32 3, i32 0}

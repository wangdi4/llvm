; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; void foo(int n) {
; #pragma omp target parallel for collapse(3)
;   for (int i = 0; i < n; ++i)
;     for (int j = 0; j < n; ++j)
;       for (int k = 0; k < n; ++k)
;         for (int l = 0; l < n; ++l);
; }

; Check that all 4 loops were collapsed and 1D-range is used:
; CHECK: call spir_func i64 @_Z13get_global_idj(i32 0)
; CHECK-NOT: call{{.*}}@_Z13get_global_idj

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @foo(i32 %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %n.addr.ascast = addrspacecast i32* %n.addr to i32 addrspace(4)*
  %.capture_expr. = alloca i32, align 4
  %.capture_expr..ascast = addrspacecast i32* %.capture_expr. to i32 addrspace(4)*
  %.capture_expr.1 = alloca i32, align 4
  %.capture_expr.1.ascast = addrspacecast i32* %.capture_expr.1 to i32 addrspace(4)*
  %.capture_expr.2 = alloca i32, align 4
  %.capture_expr.2.ascast = addrspacecast i32* %.capture_expr.2 to i32 addrspace(4)*
  %.capture_expr.3 = alloca i32, align 4
  %.capture_expr.3.ascast = addrspacecast i32* %.capture_expr.3 to i32 addrspace(4)*
  %.capture_expr.4 = alloca i64, align 8
  %.capture_expr.4.ascast = addrspacecast i64* %.capture_expr.4 to i64 addrspace(4)*
  %.omp.uncollapsed.lb = alloca i64, align 8
  %.omp.uncollapsed.lb.ascast = addrspacecast i64* %.omp.uncollapsed.lb to i64 addrspace(4)*
  %.omp.uncollapsed.ub = alloca i64, align 8
  %.omp.uncollapsed.ub.ascast = addrspacecast i64* %.omp.uncollapsed.ub to i64 addrspace(4)*
  %.omp.uncollapsed.lb30 = alloca i64, align 8
  %.omp.uncollapsed.lb30.ascast = addrspacecast i64* %.omp.uncollapsed.lb30 to i64 addrspace(4)*
  %.omp.uncollapsed.ub31 = alloca i64, align 8
  %.omp.uncollapsed.ub31.ascast = addrspacecast i64* %.omp.uncollapsed.ub31 to i64 addrspace(4)*
  %.omp.uncollapsed.lb38 = alloca i64, align 8
  %.omp.uncollapsed.lb38.ascast = addrspacecast i64* %.omp.uncollapsed.lb38 to i64 addrspace(4)*
  %.omp.uncollapsed.ub39 = alloca i64, align 8
  %.omp.uncollapsed.ub39.ascast = addrspacecast i64* %.omp.uncollapsed.ub39 to i64 addrspace(4)*
  %.omp.uncollapsed.lb46 = alloca i64, align 8
  %.omp.uncollapsed.lb46.ascast = addrspacecast i64* %.omp.uncollapsed.lb46 to i64 addrspace(4)*
  %.omp.uncollapsed.ub47 = alloca i64, align 8
  %.omp.uncollapsed.ub47.ascast = addrspacecast i64* %.omp.uncollapsed.ub47 to i64 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %tmp54 = alloca i32, align 4
  %tmp54.ascast = addrspacecast i32* %tmp54 to i32 addrspace(4)*
  %tmp55 = alloca i32, align 4
  %tmp55.ascast = addrspacecast i32* %tmp55 to i32 addrspace(4)*
  %tmp56 = alloca i32, align 4
  %tmp56.ascast = addrspacecast i32* %tmp56 to i32 addrspace(4)*
  %.omp.uncollapsed.iv = alloca i64, align 8
  %.omp.uncollapsed.iv.ascast = addrspacecast i64* %.omp.uncollapsed.iv to i64 addrspace(4)*
  %.omp.uncollapsed.iv70 = alloca i64, align 8
  %.omp.uncollapsed.iv70.ascast = addrspacecast i64* %.omp.uncollapsed.iv70 to i64 addrspace(4)*
  %.omp.uncollapsed.iv71 = alloca i64, align 8
  %.omp.uncollapsed.iv71.ascast = addrspacecast i64* %.omp.uncollapsed.iv71 to i64 addrspace(4)*
  %.omp.uncollapsed.iv72 = alloca i64, align 8
  %.omp.uncollapsed.iv72.ascast = addrspacecast i64* %.omp.uncollapsed.iv72 to i64 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %j = alloca i32, align 4
  %j.ascast = addrspacecast i32* %j to i32 addrspace(4)*
  %k = alloca i32, align 4
  %k.ascast = addrspacecast i32* %k to i32 addrspace(4)*
  %l = alloca i32, align 4
  %l.ascast = addrspacecast i32* %l to i32 addrspace(4)*
  store i32 %n, i32 addrspace(4)* %n.addr.ascast, align 4
  %0 = load i32, i32 addrspace(4)* %n.addr.ascast, align 4
  store i32 %0, i32 addrspace(4)* %.capture_expr..ascast, align 4
  %1 = load i32, i32 addrspace(4)* %n.addr.ascast, align 4
  store i32 %1, i32 addrspace(4)* %.capture_expr.1.ascast, align 4
  %2 = load i32, i32 addrspace(4)* %n.addr.ascast, align 4
  store i32 %2, i32 addrspace(4)* %.capture_expr.2.ascast, align 4
  %3 = load i32, i32 addrspace(4)* %n.addr.ascast, align 4
  store i32 %3, i32 addrspace(4)* %.capture_expr.3.ascast, align 4
  %4 = load i32, i32 addrspace(4)* %.capture_expr..ascast, align 4
  %sub = sub nsw i32 %4, 0
  %sub5 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub5, 1
  %div = sdiv i32 %add, 1
  %conv = sext i32 %div to i64
  %5 = load i32, i32 addrspace(4)* %.capture_expr.1.ascast, align 4
  %sub6 = sub nsw i32 %5, 0
  %sub7 = sub nsw i32 %sub6, 1
  %add8 = add nsw i32 %sub7, 1
  %div9 = sdiv i32 %add8, 1
  %conv10 = sext i32 %div9 to i64
  %mul = mul nsw i64 %conv, %conv10
  %6 = load i32, i32 addrspace(4)* %.capture_expr.2.ascast, align 4
  %sub11 = sub nsw i32 %6, 0
  %sub12 = sub nsw i32 %sub11, 1
  %add13 = add nsw i32 %sub12, 1
  %div14 = sdiv i32 %add13, 1
  %conv15 = sext i32 %div14 to i64
  %mul16 = mul nsw i64 %mul, %conv15
  %7 = load i32, i32 addrspace(4)* %.capture_expr.3.ascast, align 4
  %sub17 = sub nsw i32 %7, 0
  %sub18 = sub nsw i32 %sub17, 1
  %add19 = add nsw i32 %sub18, 1
  %div20 = sdiv i32 %add19, 1
  %conv21 = sext i32 %div20 to i64
  %mul22 = mul nsw i64 %mul16, %conv21
  %sub23 = sub nsw i64 %mul22, 1
  store i64 %sub23, i64 addrspace(4)* %.capture_expr.4.ascast, align 8
  store i64 0, i64 addrspace(4)* %.omp.uncollapsed.lb.ascast, align 8
  %8 = load i32, i32 addrspace(4)* %.capture_expr..ascast, align 4
  %sub24 = sub nsw i32 %8, 0
  %sub25 = sub nsw i32 %sub24, 1
  %add26 = add nsw i32 %sub25, 1
  %div27 = sdiv i32 %add26, 1
  %conv28 = sext i32 %div27 to i64
  %sub29 = sub nsw i64 %conv28, 1
  store i64 %sub29, i64 addrspace(4)* %.omp.uncollapsed.ub.ascast, align 8
  store i64 0, i64 addrspace(4)* %.omp.uncollapsed.lb30.ascast, align 8
  %9 = load i32, i32 addrspace(4)* %.capture_expr.1.ascast, align 4
  %sub32 = sub nsw i32 %9, 0
  %sub33 = sub nsw i32 %sub32, 1
  %add34 = add nsw i32 %sub33, 1
  %div35 = sdiv i32 %add34, 1
  %conv36 = sext i32 %div35 to i64
  %sub37 = sub nsw i64 %conv36, 1
  store i64 %sub37, i64 addrspace(4)* %.omp.uncollapsed.ub31.ascast, align 8
  store i64 0, i64 addrspace(4)* %.omp.uncollapsed.lb38.ascast, align 8
  %10 = load i32, i32 addrspace(4)* %.capture_expr.2.ascast, align 4
  %sub40 = sub nsw i32 %10, 0
  %sub41 = sub nsw i32 %sub40, 1
  %add42 = add nsw i32 %sub41, 1
  %div43 = sdiv i32 %add42, 1
  %conv44 = sext i32 %div43 to i64
  %sub45 = sub nsw i64 %conv44, 1
  store i64 %sub45, i64 addrspace(4)* %.omp.uncollapsed.ub39.ascast, align 8
  store i64 0, i64 addrspace(4)* %.omp.uncollapsed.lb46.ascast, align 8
  %11 = load i32, i32 addrspace(4)* %.capture_expr.3.ascast, align 4
  %sub48 = sub nsw i32 %11, 0
  %sub49 = sub nsw i32 %sub48, 1
  %add50 = add nsw i32 %sub49, 1
  %div51 = sdiv i32 %add50, 1
  %conv52 = sext i32 %div51 to i64
  %sub53 = sub nsw i64 %conv52, 1
  store i64 %sub53, i64 addrspace(4)* %.omp.uncollapsed.ub47.ascast, align 8
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.capture_expr..ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.capture_expr.1.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.capture_expr.2.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.capture_expr.3.ascast), "QUAL.OMP.PRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.iv.ascast), "QUAL.OMP.PRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.iv70.ascast), "QUAL.OMP.PRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.iv71.ascast), "QUAL.OMP.PRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.iv72.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %l.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %k.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.ub.ascast), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.lb30.ascast), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.ub31.ascast), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.lb38.ascast), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.ub39.ascast), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.lb46.ascast), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.ub47.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp54.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp55.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp56.ascast) ]
  %13 = load i32, i32 addrspace(4)* %.capture_expr..ascast, align 4
  %cmp = icmp slt i32 0, %13
  br i1 %cmp, label %land.lhs.true, label %omp.precond.end

land.lhs.true:                                    ; preds = %entry
  %14 = load i32, i32 addrspace(4)* %.capture_expr.1.ascast, align 4
  %cmp59 = icmp slt i32 0, %14
  br i1 %cmp59, label %land.lhs.true62, label %omp.precond.end

land.lhs.true62:                                  ; preds = %land.lhs.true
  %15 = load i32, i32 addrspace(4)* %.capture_expr.2.ascast, align 4
  %cmp63 = icmp slt i32 0, %15
  br i1 %cmp63, label %land.lhs.true66, label %omp.precond.end

land.lhs.true66:                                  ; preds = %land.lhs.true62
  %16 = load i32, i32 addrspace(4)* %.capture_expr.3.ascast, align 4
  %cmp67 = icmp slt i32 0, %16
  br i1 %cmp67, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %land.lhs.true66
  %17 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.COLLAPSE"(i32 4), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.lb.ascast), "QUAL.OMP.NORMALIZED.IV"(i64 addrspace(4)* %.omp.uncollapsed.iv.ascast, i64 addrspace(4)* %.omp.uncollapsed.iv70.ascast, i64 addrspace(4)* %.omp.uncollapsed.iv71.ascast, i64 addrspace(4)* %.omp.uncollapsed.iv72.ascast), "QUAL.OMP.NORMALIZED.UB"(i64 addrspace(4)* %.omp.uncollapsed.ub.ascast, i64 addrspace(4)* %.omp.uncollapsed.ub31.ascast, i64 addrspace(4)* %.omp.uncollapsed.ub39.ascast, i64 addrspace(4)* %.omp.uncollapsed.ub47.ascast), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.lb30.ascast), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.lb38.ascast), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.lb46.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %k.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %l.ascast) ]
  %18 = load i64, i64 addrspace(4)* %.omp.uncollapsed.lb.ascast, align 8
  store i64 %18, i64 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 8
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc106, %omp.precond.then
  %19 = load i64, i64 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 8
  %20 = load i64, i64 addrspace(4)* %.omp.uncollapsed.ub.ascast, align 8
  %cmp73 = icmp sle i64 %19, %20
  br i1 %cmp73, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end108

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %21 = load i64, i64 addrspace(4)* %.omp.uncollapsed.lb30.ascast, align 8
  store i64 %21, i64 addrspace(4)* %.omp.uncollapsed.iv70.ascast, align 8
  br label %omp.uncollapsed.loop.cond75

omp.uncollapsed.loop.cond75:                      ; preds = %omp.uncollapsed.loop.inc103, %omp.uncollapsed.loop.body
  %22 = load i64, i64 addrspace(4)* %.omp.uncollapsed.iv70.ascast, align 8
  %23 = load i64, i64 addrspace(4)* %.omp.uncollapsed.ub31.ascast, align 8
  %cmp76 = icmp sle i64 %22, %23
  br i1 %cmp76, label %omp.uncollapsed.loop.body78, label %omp.uncollapsed.loop.end105

omp.uncollapsed.loop.body78:                      ; preds = %omp.uncollapsed.loop.cond75
  %24 = load i64, i64 addrspace(4)* %.omp.uncollapsed.lb38.ascast, align 8
  store i64 %24, i64 addrspace(4)* %.omp.uncollapsed.iv71.ascast, align 8
  br label %omp.uncollapsed.loop.cond79

omp.uncollapsed.loop.cond79:                      ; preds = %omp.uncollapsed.loop.inc100, %omp.uncollapsed.loop.body78
  %25 = load i64, i64 addrspace(4)* %.omp.uncollapsed.iv71.ascast, align 8
  %26 = load i64, i64 addrspace(4)* %.omp.uncollapsed.ub39.ascast, align 8
  %cmp80 = icmp sle i64 %25, %26
  br i1 %cmp80, label %omp.uncollapsed.loop.body82, label %omp.uncollapsed.loop.end102

omp.uncollapsed.loop.body82:                      ; preds = %omp.uncollapsed.loop.cond79
  %27 = load i64, i64 addrspace(4)* %.omp.uncollapsed.lb46.ascast, align 8
  store i64 %27, i64 addrspace(4)* %.omp.uncollapsed.iv72.ascast, align 8
  br label %omp.uncollapsed.loop.cond83

omp.uncollapsed.loop.cond83:                      ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body82
  %28 = load i64, i64 addrspace(4)* %.omp.uncollapsed.iv72.ascast, align 8
  %29 = load i64, i64 addrspace(4)* %.omp.uncollapsed.ub47.ascast, align 8
  %cmp84 = icmp sle i64 %28, %29
  br i1 %cmp84, label %omp.uncollapsed.loop.body86, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body86:                      ; preds = %omp.uncollapsed.loop.cond83
  %30 = load i64, i64 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 8
  %mul87 = mul nsw i64 %30, 1
  %add88 = add nsw i64 0, %mul87
  %conv89 = trunc i64 %add88 to i32
  store i32 %conv89, i32 addrspace(4)* %i.ascast, align 4
  %31 = load i64, i64 addrspace(4)* %.omp.uncollapsed.iv70.ascast, align 8
  %mul90 = mul nsw i64 %31, 1
  %add91 = add nsw i64 0, %mul90
  %conv92 = trunc i64 %add91 to i32
  store i32 %conv92, i32 addrspace(4)* %j.ascast, align 4
  %32 = load i64, i64 addrspace(4)* %.omp.uncollapsed.iv71.ascast, align 8
  %mul93 = mul nsw i64 %32, 1
  %add94 = add nsw i64 0, %mul93
  %conv95 = trunc i64 %add94 to i32
  store i32 %conv95, i32 addrspace(4)* %k.ascast, align 4
  %33 = load i64, i64 addrspace(4)* %.omp.uncollapsed.iv72.ascast, align 8
  %mul96 = mul nsw i64 %33, 1
  %add97 = add nsw i64 0, %mul96
  %conv98 = trunc i64 %add97 to i32
  store i32 %conv98, i32 addrspace(4)* %l.ascast, align 4
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.uncollapsed.loop.body86
  %34 = load i64, i64 addrspace(4)* %.omp.uncollapsed.iv72.ascast, align 8
  %add99 = add nsw i64 %34, 1
  store i64 %add99, i64 addrspace(4)* %.omp.uncollapsed.iv72.ascast, align 8
  br label %omp.uncollapsed.loop.cond83

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond83
  br label %omp.uncollapsed.loop.inc100

omp.uncollapsed.loop.inc100:                      ; preds = %omp.uncollapsed.loop.end
  %35 = load i64, i64 addrspace(4)* %.omp.uncollapsed.iv71.ascast, align 8
  %add101 = add nsw i64 %35, 1
  store i64 %add101, i64 addrspace(4)* %.omp.uncollapsed.iv71.ascast, align 8
  br label %omp.uncollapsed.loop.cond79

omp.uncollapsed.loop.end102:                      ; preds = %omp.uncollapsed.loop.cond79
  br label %omp.uncollapsed.loop.inc103

omp.uncollapsed.loop.inc103:                      ; preds = %omp.uncollapsed.loop.end102
  %36 = load i64, i64 addrspace(4)* %.omp.uncollapsed.iv70.ascast, align 8
  %add104 = add nsw i64 %36, 1
  store i64 %add104, i64 addrspace(4)* %.omp.uncollapsed.iv70.ascast, align 8
  br label %omp.uncollapsed.loop.cond75

omp.uncollapsed.loop.end105:                      ; preds = %omp.uncollapsed.loop.cond75
  br label %omp.uncollapsed.loop.inc106

omp.uncollapsed.loop.inc106:                      ; preds = %omp.uncollapsed.loop.end105
  %37 = load i64, i64 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 8
  %add107 = add nsw i64 %37, 1
  store i64 %add107, i64 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 8
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end108:                      ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %17) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.uncollapsed.loop.end108, %land.lhs.true66, %land.lhs.true62, %land.lhs.true, %entry
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2052, i32 55981756, !"foo", i32 2, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 9.0.0"}

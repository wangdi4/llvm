; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-collapse-always=true -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -vpo-paropt-collapse-always=true -S %s | FileCheck %s

; Original code:
; void foo(int n) {
; #pragma omp target parallel for collapse(3)
;   for (int i = 0; i < n; ++i)
;     for (int j = 0; j < n; ++j)
;       for (int k = 0; k < n; ++k);
; }

; Check that the loop nest was collapsed due to -vpo-paropt-collapse-always,
; and that ND-range parallelization is used:
; CHECK: call spir_func i64 @_Z13get_global_idj(i32 0)
; CHECK-NOT: call spir_func i64 @_Z13get_global_idj(i32 1)
; CHECK-NOT: call spir_func i64 @_Z13get_global_idj(i32 2)

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
  %.capture_expr.3 = alloca i64, align 8
  %.capture_expr.3.ascast = addrspacecast i64* %.capture_expr.3 to i64 addrspace(4)*
  %.omp.uncollapsed.lb = alloca i64, align 8
  %.omp.uncollapsed.lb.ascast = addrspacecast i64* %.omp.uncollapsed.lb to i64 addrspace(4)*
  %.omp.uncollapsed.ub = alloca i64, align 8
  %.omp.uncollapsed.ub.ascast = addrspacecast i64* %.omp.uncollapsed.ub to i64 addrspace(4)*
  %.omp.uncollapsed.lb23 = alloca i64, align 8
  %.omp.uncollapsed.lb23.ascast = addrspacecast i64* %.omp.uncollapsed.lb23 to i64 addrspace(4)*
  %.omp.uncollapsed.ub24 = alloca i64, align 8
  %.omp.uncollapsed.ub24.ascast = addrspacecast i64* %.omp.uncollapsed.ub24 to i64 addrspace(4)*
  %.omp.uncollapsed.lb31 = alloca i64, align 8
  %.omp.uncollapsed.lb31.ascast = addrspacecast i64* %.omp.uncollapsed.lb31 to i64 addrspace(4)*
  %.omp.uncollapsed.ub32 = alloca i64, align 8
  %.omp.uncollapsed.ub32.ascast = addrspacecast i64* %.omp.uncollapsed.ub32 to i64 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %tmp39 = alloca i32, align 4
  %tmp39.ascast = addrspacecast i32* %tmp39 to i32 addrspace(4)*
  %tmp40 = alloca i32, align 4
  %tmp40.ascast = addrspacecast i32* %tmp40 to i32 addrspace(4)*
  %.omp.uncollapsed.iv = alloca i64, align 8
  %.omp.uncollapsed.iv.ascast = addrspacecast i64* %.omp.uncollapsed.iv to i64 addrspace(4)*
  %.omp.uncollapsed.iv50 = alloca i64, align 8
  %.omp.uncollapsed.iv50.ascast = addrspacecast i64* %.omp.uncollapsed.iv50 to i64 addrspace(4)*
  %.omp.uncollapsed.iv51 = alloca i64, align 8
  %.omp.uncollapsed.iv51.ascast = addrspacecast i64* %.omp.uncollapsed.iv51 to i64 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %j = alloca i32, align 4
  %j.ascast = addrspacecast i32* %j to i32 addrspace(4)*
  %k = alloca i32, align 4
  %k.ascast = addrspacecast i32* %k to i32 addrspace(4)*
  store i32 %n, i32 addrspace(4)* %n.addr.ascast, align 4
  %0 = load i32, i32 addrspace(4)* %n.addr.ascast, align 4
  store i32 %0, i32 addrspace(4)* %.capture_expr..ascast, align 4
  %1 = load i32, i32 addrspace(4)* %n.addr.ascast, align 4
  store i32 %1, i32 addrspace(4)* %.capture_expr.1.ascast, align 4
  %2 = load i32, i32 addrspace(4)* %n.addr.ascast, align 4
  store i32 %2, i32 addrspace(4)* %.capture_expr.2.ascast, align 4
  %3 = load i32, i32 addrspace(4)* %.capture_expr..ascast, align 4
  %sub = sub nsw i32 %3, 0
  %sub4 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub4, 1
  %div = sdiv i32 %add, 1
  %conv = sext i32 %div to i64
  %4 = load i32, i32 addrspace(4)* %.capture_expr.1.ascast, align 4
  %sub5 = sub nsw i32 %4, 0
  %sub6 = sub nsw i32 %sub5, 1
  %add7 = add nsw i32 %sub6, 1
  %div8 = sdiv i32 %add7, 1
  %conv9 = sext i32 %div8 to i64
  %mul = mul nsw i64 %conv, %conv9
  %5 = load i32, i32 addrspace(4)* %.capture_expr.2.ascast, align 4
  %sub10 = sub nsw i32 %5, 0
  %sub11 = sub nsw i32 %sub10, 1
  %add12 = add nsw i32 %sub11, 1
  %div13 = sdiv i32 %add12, 1
  %conv14 = sext i32 %div13 to i64
  %mul15 = mul nsw i64 %mul, %conv14
  %sub16 = sub nsw i64 %mul15, 1
  store i64 %sub16, i64 addrspace(4)* %.capture_expr.3.ascast, align 8
  store i64 0, i64 addrspace(4)* %.omp.uncollapsed.lb.ascast, align 8
  %6 = load i32, i32 addrspace(4)* %.capture_expr..ascast, align 4
  %sub17 = sub nsw i32 %6, 0
  %sub18 = sub nsw i32 %sub17, 1
  %add19 = add nsw i32 %sub18, 1
  %div20 = sdiv i32 %add19, 1
  %conv21 = sext i32 %div20 to i64
  %sub22 = sub nsw i64 %conv21, 1
  store i64 %sub22, i64 addrspace(4)* %.omp.uncollapsed.ub.ascast, align 8
  store i64 0, i64 addrspace(4)* %.omp.uncollapsed.lb23.ascast, align 8
  %7 = load i32, i32 addrspace(4)* %.capture_expr.1.ascast, align 4
  %sub25 = sub nsw i32 %7, 0
  %sub26 = sub nsw i32 %sub25, 1
  %add27 = add nsw i32 %sub26, 1
  %div28 = sdiv i32 %add27, 1
  %conv29 = sext i32 %div28 to i64
  %sub30 = sub nsw i64 %conv29, 1
  store i64 %sub30, i64 addrspace(4)* %.omp.uncollapsed.ub24.ascast, align 8
  store i64 0, i64 addrspace(4)* %.omp.uncollapsed.lb31.ascast, align 8
  %8 = load i32, i32 addrspace(4)* %.capture_expr.2.ascast, align 4
  %sub33 = sub nsw i32 %8, 0
  %sub34 = sub nsw i32 %sub33, 1
  %add35 = add nsw i32 %sub34, 1
  %div36 = sdiv i32 %add35, 1
  %conv37 = sext i32 %div36 to i64
  %sub38 = sub nsw i64 %conv37, 1
  store i64 %sub38, i64 addrspace(4)* %.omp.uncollapsed.ub32.ascast, align 8
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.capture_expr..ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.capture_expr.1.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.capture_expr.2.ascast), "QUAL.OMP.PRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.iv.ascast), "QUAL.OMP.PRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.iv50.ascast), "QUAL.OMP.PRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.iv51.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %k.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.ub.ascast), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.lb23.ascast), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.ub24.ascast), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.lb31.ascast), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.ub32.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp39.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp40.ascast) ]
  %10 = load i32, i32 addrspace(4)* %.capture_expr..ascast, align 4
  %cmp = icmp slt i32 0, %10
  br i1 %cmp, label %land.lhs.true, label %omp.precond.end

land.lhs.true:                                    ; preds = %entry
  %11 = load i32, i32 addrspace(4)* %.capture_expr.1.ascast, align 4
  %cmp43 = icmp slt i32 0, %11
  br i1 %cmp43, label %land.lhs.true46, label %omp.precond.end

land.lhs.true46:                                  ; preds = %land.lhs.true
  %12 = load i32, i32 addrspace(4)* %.capture_expr.2.ascast, align 4
  %cmp47 = icmp slt i32 0, %12
  br i1 %cmp47, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %land.lhs.true46
  %13 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.COLLAPSE"(i32 3), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.lb.ascast), "QUAL.OMP.NORMALIZED.IV"(i64 addrspace(4)* %.omp.uncollapsed.iv.ascast, i64 addrspace(4)* %.omp.uncollapsed.iv50.ascast, i64 addrspace(4)* %.omp.uncollapsed.iv51.ascast), "QUAL.OMP.NORMALIZED.UB"(i64 addrspace(4)* %.omp.uncollapsed.ub.ascast, i64 addrspace(4)* %.omp.uncollapsed.ub24.ascast, i64 addrspace(4)* %.omp.uncollapsed.ub32.ascast), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.lb23.ascast), "QUAL.OMP.FIRSTPRIVATE"(i64 addrspace(4)* %.omp.uncollapsed.lb31.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %k.ascast) ]
  %14 = load i64, i64 addrspace(4)* %.omp.uncollapsed.lb.ascast, align 8
  store i64 %14, i64 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 8
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc75, %omp.precond.then
  %15 = load i64, i64 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 8
  %16 = load i64, i64 addrspace(4)* %.omp.uncollapsed.ub.ascast, align 8
  %cmp52 = icmp sle i64 %15, %16
  br i1 %cmp52, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end77

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %17 = load i64, i64 addrspace(4)* %.omp.uncollapsed.lb23.ascast, align 8
  store i64 %17, i64 addrspace(4)* %.omp.uncollapsed.iv50.ascast, align 8
  br label %omp.uncollapsed.loop.cond54

omp.uncollapsed.loop.cond54:                      ; preds = %omp.uncollapsed.loop.inc72, %omp.uncollapsed.loop.body
  %18 = load i64, i64 addrspace(4)* %.omp.uncollapsed.iv50.ascast, align 8
  %19 = load i64, i64 addrspace(4)* %.omp.uncollapsed.ub24.ascast, align 8
  %cmp55 = icmp sle i64 %18, %19
  br i1 %cmp55, label %omp.uncollapsed.loop.body57, label %omp.uncollapsed.loop.end74

omp.uncollapsed.loop.body57:                      ; preds = %omp.uncollapsed.loop.cond54
  %20 = load i64, i64 addrspace(4)* %.omp.uncollapsed.lb31.ascast, align 8
  store i64 %20, i64 addrspace(4)* %.omp.uncollapsed.iv51.ascast, align 8
  br label %omp.uncollapsed.loop.cond58

omp.uncollapsed.loop.cond58:                      ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body57
  %21 = load i64, i64 addrspace(4)* %.omp.uncollapsed.iv51.ascast, align 8
  %22 = load i64, i64 addrspace(4)* %.omp.uncollapsed.ub32.ascast, align 8
  %cmp59 = icmp sle i64 %21, %22
  br i1 %cmp59, label %omp.uncollapsed.loop.body61, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body61:                      ; preds = %omp.uncollapsed.loop.cond58
  %23 = load i64, i64 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 8
  %mul62 = mul nsw i64 %23, 1
  %add63 = add nsw i64 0, %mul62
  %conv64 = trunc i64 %add63 to i32
  store i32 %conv64, i32 addrspace(4)* %i.ascast, align 4
  %24 = load i64, i64 addrspace(4)* %.omp.uncollapsed.iv50.ascast, align 8
  %mul65 = mul nsw i64 %24, 1
  %add66 = add nsw i64 0, %mul65
  %conv67 = trunc i64 %add66 to i32
  store i32 %conv67, i32 addrspace(4)* %j.ascast, align 4
  %25 = load i64, i64 addrspace(4)* %.omp.uncollapsed.iv51.ascast, align 8
  %mul68 = mul nsw i64 %25, 1
  %add69 = add nsw i64 0, %mul68
  %conv70 = trunc i64 %add69 to i32
  store i32 %conv70, i32 addrspace(4)* %k.ascast, align 4
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.uncollapsed.loop.body61
  %26 = load i64, i64 addrspace(4)* %.omp.uncollapsed.iv51.ascast, align 8
  %add71 = add nsw i64 %26, 1
  store i64 %add71, i64 addrspace(4)* %.omp.uncollapsed.iv51.ascast, align 8
  br label %omp.uncollapsed.loop.cond58

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond58
  br label %omp.uncollapsed.loop.inc72

omp.uncollapsed.loop.inc72:                       ; preds = %omp.uncollapsed.loop.end
  %27 = load i64, i64 addrspace(4)* %.omp.uncollapsed.iv50.ascast, align 8
  %add73 = add nsw i64 %27, 1
  store i64 %add73, i64 addrspace(4)* %.omp.uncollapsed.iv50.ascast, align 8
  br label %omp.uncollapsed.loop.cond54

omp.uncollapsed.loop.end74:                       ; preds = %omp.uncollapsed.loop.cond54
  br label %omp.uncollapsed.loop.inc75

omp.uncollapsed.loop.inc75:                       ; preds = %omp.uncollapsed.loop.end74
  %28 = load i64, i64 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 8
  %add76 = add nsw i64 %28, 1
  store i64 %add76, i64 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 8
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end77:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %13) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.uncollapsed.loop.end77, %land.lhs.true46, %land.lhs.true, %entry
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.TARGET"() ]
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

; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-collapse-always=true -S %s | FileCheck %s
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

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define protected spir_func void @foo(i32 noundef %n) {
entry:
  %n.addr = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.capture_expr.2 = alloca i32, align 4
  %.capture_expr.3 = alloca i64, align 8
  %.omp.uncollapsed.lb = alloca i64, align 8
  %.omp.uncollapsed.ub = alloca i64, align 8
  %.omp.uncollapsed.lb20 = alloca i64, align 8
  %.omp.uncollapsed.ub21 = alloca i64, align 8
  %.omp.uncollapsed.lb28 = alloca i64, align 8
  %.omp.uncollapsed.ub29 = alloca i64, align 8
  %tmp = alloca i32, align 4
  %tmp36 = alloca i32, align 4
  %tmp37 = alloca i32, align 4
  %.omp.uncollapsed.iv = alloca i64, align 8
  %.omp.uncollapsed.iv47 = alloca i64, align 8
  %.omp.uncollapsed.iv48 = alloca i64, align 8
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %k = alloca i32, align 4
  %n.addr.ascast = addrspacecast ptr %n.addr to ptr addrspace(4)
  %.capture_expr.0.ascast = addrspacecast ptr %.capture_expr.0 to ptr addrspace(4)
  %.capture_expr.1.ascast = addrspacecast ptr %.capture_expr.1 to ptr addrspace(4)
  %.capture_expr.2.ascast = addrspacecast ptr %.capture_expr.2 to ptr addrspace(4)
  %.capture_expr.3.ascast = addrspacecast ptr %.capture_expr.3 to ptr addrspace(4)
  %.omp.uncollapsed.lb.ascast = addrspacecast ptr %.omp.uncollapsed.lb to ptr addrspace(4)
  %.omp.uncollapsed.ub.ascast = addrspacecast ptr %.omp.uncollapsed.ub to ptr addrspace(4)
  %.omp.uncollapsed.lb20.ascast = addrspacecast ptr %.omp.uncollapsed.lb20 to ptr addrspace(4)
  %.omp.uncollapsed.ub21.ascast = addrspacecast ptr %.omp.uncollapsed.ub21 to ptr addrspace(4)
  %.omp.uncollapsed.lb28.ascast = addrspacecast ptr %.omp.uncollapsed.lb28 to ptr addrspace(4)
  %.omp.uncollapsed.ub29.ascast = addrspacecast ptr %.omp.uncollapsed.ub29 to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %tmp36.ascast = addrspacecast ptr %tmp36 to ptr addrspace(4)
  %tmp37.ascast = addrspacecast ptr %tmp37 to ptr addrspace(4)
  %.omp.uncollapsed.iv.ascast = addrspacecast ptr %.omp.uncollapsed.iv to ptr addrspace(4)
  %.omp.uncollapsed.iv47.ascast = addrspacecast ptr %.omp.uncollapsed.iv47 to ptr addrspace(4)
  %.omp.uncollapsed.iv48.ascast = addrspacecast ptr %.omp.uncollapsed.iv48 to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %j.ascast = addrspacecast ptr %j to ptr addrspace(4)
  %k.ascast = addrspacecast ptr %k to ptr addrspace(4)
  store i32 %n, ptr addrspace(4) %n.addr.ascast, align 4
  %0 = load i32, ptr addrspace(4) %n.addr.ascast, align 4
  store i32 %0, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %1 = load i32, ptr addrspace(4) %n.addr.ascast, align 4
  store i32 %1, ptr addrspace(4) %.capture_expr.1.ascast, align 4
  %2 = load i32, ptr addrspace(4) %n.addr.ascast, align 4
  store i32 %2, ptr addrspace(4) %.capture_expr.2.ascast, align 4
  %3 = load i32, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %sub = sub nsw i32 %3, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %conv = sext i32 %div to i64
  %4 = load i32, ptr addrspace(4) %.capture_expr.1.ascast, align 4
  %sub2 = sub nsw i32 %4, 0
  %sub3 = sub nsw i32 %sub2, 1
  %add4 = add nsw i32 %sub3, 1
  %div5 = sdiv i32 %add4, 1
  %conv6 = sext i32 %div5 to i64
  %mul = mul nsw i64 %conv, %conv6
  %5 = load i32, ptr addrspace(4) %.capture_expr.2.ascast, align 4
  %sub7 = sub nsw i32 %5, 0
  %sub8 = sub nsw i32 %sub7, 1
  %add9 = add nsw i32 %sub8, 1
  %div10 = sdiv i32 %add9, 1
  %conv11 = sext i32 %div10 to i64
  %mul12 = mul nsw i64 %mul, %conv11
  %sub13 = sub nsw i64 %mul12, 1
  store i64 %sub13, ptr addrspace(4) %.capture_expr.3.ascast, align 8
  store i64 0, ptr addrspace(4) %.omp.uncollapsed.lb.ascast, align 8
  %6 = load i32, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %sub14 = sub nsw i32 %6, 0
  %sub15 = sub nsw i32 %sub14, 1
  %add16 = add nsw i32 %sub15, 1
  %div17 = sdiv i32 %add16, 1
  %conv18 = sext i32 %div17 to i64
  %sub19 = sub nsw i64 %conv18, 1
  store i64 %sub19, ptr addrspace(4) %.omp.uncollapsed.ub.ascast, align 8
  store i64 0, ptr addrspace(4) %.omp.uncollapsed.lb20.ascast, align 8
  %7 = load i32, ptr addrspace(4) %.capture_expr.1.ascast, align 4
  %sub22 = sub nsw i32 %7, 0
  %sub23 = sub nsw i32 %sub22, 1
  %add24 = add nsw i32 %sub23, 1
  %div25 = sdiv i32 %add24, 1
  %conv26 = sext i32 %div25 to i64
  %sub27 = sub nsw i64 %conv26, 1
  store i64 %sub27, ptr addrspace(4) %.omp.uncollapsed.ub21.ascast, align 8
  store i64 0, ptr addrspace(4) %.omp.uncollapsed.lb28.ascast, align 8
  %8 = load i32, ptr addrspace(4) %.capture_expr.2.ascast, align 4
  %sub30 = sub nsw i32 %8, 0
  %sub31 = sub nsw i32 %sub30, 1
  %add32 = add nsw i32 %sub31, 1
  %div33 = sdiv i32 %add32, 1
  %conv34 = sext i32 %div33 to i64
  %sub35 = sub nsw i64 %conv34, 1
  store i64 %sub35, ptr addrspace(4) %.omp.uncollapsed.ub29.ascast, align 8
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i64 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i64 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv47.ascast, i64 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb20.ascast, i64 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub21.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.1.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv48.ascast, i64 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb28.ascast, i64 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub29.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp36.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp37.ascast, i32 0, i32 1) ]

  %10 = load i32, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %cmp = icmp slt i32 0, %10
  br i1 %cmp, label %land.lhs.true, label %omp.precond.end

land.lhs.true:                                    ; preds = %entry
  %11 = load i32, ptr addrspace(4) %.capture_expr.1.ascast, align 4
  %cmp40 = icmp slt i32 0, %11
  br i1 %cmp40, label %land.lhs.true43, label %omp.precond.end

land.lhs.true43:                                  ; preds = %land.lhs.true
  %12 = load i32, ptr addrspace(4) %.capture_expr.2.ascast, align 4
  %cmp44 = icmp slt i32 0, %12
  br i1 %cmp44, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %land.lhs.true43
  %13 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.COLLAPSE"(i32 3),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i64 0, ptr addrspace(4) %.omp.uncollapsed.iv47.ascast, i64 0, ptr addrspace(4) %.omp.uncollapsed.iv48.ascast, i64 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i64 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i64 0, ptr addrspace(4) %.omp.uncollapsed.ub21.ascast, i64 0, ptr addrspace(4) %.omp.uncollapsed.ub29.ascast, i64 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb20.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb28.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1) ]

  %14 = load i64, ptr addrspace(4) %.omp.uncollapsed.lb.ascast, align 8
  store i64 %14, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 8
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc72, %omp.precond.then
  %15 = load i64, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 8
  %16 = load i64, ptr addrspace(4) %.omp.uncollapsed.ub.ascast, align 8
  %cmp49 = icmp sle i64 %15, %16
  br i1 %cmp49, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end74

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %17 = load i64, ptr addrspace(4) %.omp.uncollapsed.lb20.ascast, align 8
  store i64 %17, ptr addrspace(4) %.omp.uncollapsed.iv47.ascast, align 8
  br label %omp.uncollapsed.loop.cond51

omp.uncollapsed.loop.cond51:                      ; preds = %omp.uncollapsed.loop.inc69, %omp.uncollapsed.loop.body
  %18 = load i64, ptr addrspace(4) %.omp.uncollapsed.iv47.ascast, align 8
  %19 = load i64, ptr addrspace(4) %.omp.uncollapsed.ub21.ascast, align 8
  %cmp52 = icmp sle i64 %18, %19
  br i1 %cmp52, label %omp.uncollapsed.loop.body54, label %omp.uncollapsed.loop.end71

omp.uncollapsed.loop.body54:                      ; preds = %omp.uncollapsed.loop.cond51
  %20 = load i64, ptr addrspace(4) %.omp.uncollapsed.lb28.ascast, align 8
  store i64 %20, ptr addrspace(4) %.omp.uncollapsed.iv48.ascast, align 8
  br label %omp.uncollapsed.loop.cond55

omp.uncollapsed.loop.cond55:                      ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body54
  %21 = load i64, ptr addrspace(4) %.omp.uncollapsed.iv48.ascast, align 8
  %22 = load i64, ptr addrspace(4) %.omp.uncollapsed.ub29.ascast, align 8
  %cmp56 = icmp sle i64 %21, %22
  br i1 %cmp56, label %omp.uncollapsed.loop.body58, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body58:                      ; preds = %omp.uncollapsed.loop.cond55
  %23 = load i64, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 8
  %mul59 = mul nsw i64 %23, 1
  %add60 = add nsw i64 0, %mul59
  %conv61 = trunc i64 %add60 to i32
  store i32 %conv61, ptr addrspace(4) %i.ascast, align 4
  %24 = load i64, ptr addrspace(4) %.omp.uncollapsed.iv47.ascast, align 8
  %mul62 = mul nsw i64 %24, 1
  %add63 = add nsw i64 0, %mul62
  %conv64 = trunc i64 %add63 to i32
  store i32 %conv64, ptr addrspace(4) %j.ascast, align 4
  %25 = load i64, ptr addrspace(4) %.omp.uncollapsed.iv48.ascast, align 8
  %mul65 = mul nsw i64 %25, 1
  %add66 = add nsw i64 0, %mul65
  %conv67 = trunc i64 %add66 to i32
  store i32 %conv67, ptr addrspace(4) %k.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.uncollapsed.loop.body58
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.body.continue
  %26 = load i64, ptr addrspace(4) %.omp.uncollapsed.iv48.ascast, align 8
  %add68 = add nsw i64 %26, 1
  store i64 %add68, ptr addrspace(4) %.omp.uncollapsed.iv48.ascast, align 8
  br label %omp.uncollapsed.loop.cond55

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond55
  br label %omp.uncollapsed.loop.inc69

omp.uncollapsed.loop.inc69:                       ; preds = %omp.uncollapsed.loop.end
  %27 = load i64, ptr addrspace(4) %.omp.uncollapsed.iv47.ascast, align 8
  %add70 = add nsw i64 %27, 1
  store i64 %add70, ptr addrspace(4) %.omp.uncollapsed.iv47.ascast, align 8
  br label %omp.uncollapsed.loop.cond51

omp.uncollapsed.loop.end71:                       ; preds = %omp.uncollapsed.loop.cond51
  br label %omp.uncollapsed.loop.inc72

omp.uncollapsed.loop.inc72:                       ; preds = %omp.uncollapsed.loop.end71
  %28 = load i64, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 8
  %add73 = add nsw i64 %28, 1
  store i64 %add73, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 8
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end74:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %13) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.uncollapsed.loop.end74, %land.lhs.true43, %land.lhs.true, %entry
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 52, i32 -700641365, !"_Z3foo", i32 2, i32 0, i32 0, i32 0}

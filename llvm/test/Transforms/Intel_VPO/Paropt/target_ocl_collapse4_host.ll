; RUN: opt -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s

; Original code:
; void foo(int n) {
; #pragma omp target parallel for collapse(3)
;   for (int i = 0; i < n; ++i)
;     for (int j = 0; j < n; ++j)
;       for (int k = 0; k < n; ++k)
;         for (int l = 0; l < n; ++l);
; }

; Check that 1D-range is created for the runtime:
; CHECK-LABEL: @foo
; CHECK: [[UB0:%[a-zA-Z._0-9]+]] = load i64, i64* %.omp.uncollapsed.ub,
; CHECK: [[UB0ADD:%[a-zA-Z._0-9]+]] = add nuw nsw i64 [[UB0]]
; CHECK: [[UB0ZEROPRED:%[a-zA-Z._0-9]+]] = icmp slt i64 [[UB0]], 0

; CHECK: [[UB1:%[a-zA-Z._0-9]+]] = load i64, i64* %.omp.uncollapsed.ub31,
; CHECK: [[UB1ADD:%[a-zA-Z._0-9]+]] = add nuw nsw i64 [[UB1]]
; CHECK: [[UB1ZEROPRED:%[a-zA-Z._0-9]+]] = icmp slt i64 [[UB1]], 0
; CHECK: [[OR0:%[a-zA-Z._0-9]+]] = or i1 [[UB0ZEROPRED]], [[UB1ZEROPRED]]

; CHECK: [[UB2:%[a-zA-Z._0-9]+]] = load i64, i64* %.omp.uncollapsed.ub39,
; CHECK: [[UB2ADD:%[a-zA-Z._0-9]+]] = add nuw nsw i64 [[UB2]]
; CHECK: [[UB2ZEROPRED:%[a-zA-Z._0-9]+]] = icmp slt i64 [[UB2]], 0
; CHECK: [[OR1:%[a-zA-Z._0-9]+]] = or i1 [[OR0]], [[UB2ZEROPRED]]

; CHECK: [[UB3:%[a-zA-Z._0-9]+]] = load i64, i64* %.omp.uncollapsed.ub47,
; CHECK: [[UB3ADD:%[a-zA-Z._0-9]+]] = add nuw nsw i64 [[UB3]]
; CHECK: [[UB3ZEROPRED:%[a-zA-Z._0-9]+]] = icmp slt i64 [[UB3]], 0
; CHECK: [[OR2:%[a-zA-Z._0-9]+]] = or i1 [[OR1]], [[UB3ZEROPRED]]

; CHECK: [[MUL0:%[a-zA-Z._0-9]+]] = mul nuw nsw i64 [[UB0ADD]], [[UB1ADD]]
; CHECK: [[MUL1:%[a-zA-Z._0-9]+]] = mul nuw nsw i64 [[MUL0]], [[UB2ADD]]
; CHECK: [[MUL2:%[a-zA-Z._0-9]+]] = mul nuw nsw i64 [[MUL1]], [[UB3ADD]]

; Verify that we set the collapsed UB to 0, when any of the loops
; has zero iterations:
; CHECK: [[SEL:%[a-zA-Z._0-9]+]] = select i1 [[OR2]], i64 0, i64 [[MUL2]]
; CHECK: [[COLLAPSEDUBVAL:%[a-zA-Z._0-9]+]] = sub nuw nsw i64 [[SEL]], 1
; CHECK: store i64 [[COLLAPSEDUBVAL]], i64* [[COLLAPSEDUBPTR:%[a-zA-Z._0-9]+]]

; CHECK: [[NDDESC:%[a-zA-Z._0-9]+]] = alloca { i32, i32, i64, i64, i64 }
; CHECK: [[M0:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 0
; CHECK: store i32 1, i32* [[M0]]
; CHECK: [[M1:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 1
; CHECK: store i32 0, i32* [[M1]]
; CHECK: [[M2:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 2
; CHECK: store i64 0, i64* [[M2]]
; CHECK: [[M3:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 3
; CHECK: [[COLLAPSEDUB:%[a-zA-Z._0-9]+]] = load i64, i64* [[COLLAPSEDUBPTR]]
; CHECK: store i64 [[COLLAPSEDUB]], i64* [[M3]]
; CHECK: [[M4:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 4
; CHECK: store i64 1, i64* [[M4]]
; CHECK: call void @__omp_offloading_804_35636bc_foo_l2({{.*}}i64* [[COLLAPSEDUBPTR]]{{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @.omp_offloading.requires_reg, i8* null }]

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32 %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %.capture_expr. = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.capture_expr.2 = alloca i32, align 4
  %.capture_expr.3 = alloca i32, align 4
  %.capture_expr.4 = alloca i64, align 8
  %.omp.uncollapsed.lb = alloca i64, align 8
  %.omp.uncollapsed.ub = alloca i64, align 8
  %.omp.uncollapsed.lb30 = alloca i64, align 8
  %.omp.uncollapsed.ub31 = alloca i64, align 8
  %.omp.uncollapsed.lb38 = alloca i64, align 8
  %.omp.uncollapsed.ub39 = alloca i64, align 8
  %.omp.uncollapsed.lb46 = alloca i64, align 8
  %.omp.uncollapsed.ub47 = alloca i64, align 8
  %tmp = alloca i32, align 4
  %tmp54 = alloca i32, align 4
  %tmp55 = alloca i32, align 4
  %tmp56 = alloca i32, align 4
  %.omp.uncollapsed.iv = alloca i64, align 8
  %.omp.uncollapsed.iv70 = alloca i64, align 8
  %.omp.uncollapsed.iv71 = alloca i64, align 8
  %.omp.uncollapsed.iv72 = alloca i64, align 8
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %k = alloca i32, align 4
  %l = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4, !tbaa !3
  %0 = bitcast i32* %.capture_expr. to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #1
  %1 = load i32, i32* %n.addr, align 4, !tbaa !3
  store i32 %1, i32* %.capture_expr., align 4, !tbaa !3
  %2 = bitcast i32* %.capture_expr.1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #1
  %3 = load i32, i32* %n.addr, align 4, !tbaa !3
  store i32 %3, i32* %.capture_expr.1, align 4, !tbaa !3
  %4 = bitcast i32* %.capture_expr.2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #1
  %5 = load i32, i32* %n.addr, align 4, !tbaa !3
  store i32 %5, i32* %.capture_expr.2, align 4, !tbaa !3
  %6 = bitcast i32* %.capture_expr.3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %6) #1
  %7 = load i32, i32* %n.addr, align 4, !tbaa !3
  store i32 %7, i32* %.capture_expr.3, align 4, !tbaa !3
  %8 = bitcast i64* %.capture_expr.4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %8) #1
  %9 = load i32, i32* %.capture_expr., align 4, !tbaa !3
  %sub = sub nsw i32 %9, 0
  %sub5 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub5, 1
  %div = sdiv i32 %add, 1
  %conv = sext i32 %div to i64
  %10 = load i32, i32* %.capture_expr.1, align 4, !tbaa !3
  %sub6 = sub nsw i32 %10, 0
  %sub7 = sub nsw i32 %sub6, 1
  %add8 = add nsw i32 %sub7, 1
  %div9 = sdiv i32 %add8, 1
  %conv10 = sext i32 %div9 to i64
  %mul = mul nsw i64 %conv, %conv10
  %11 = load i32, i32* %.capture_expr.2, align 4, !tbaa !3
  %sub11 = sub nsw i32 %11, 0
  %sub12 = sub nsw i32 %sub11, 1
  %add13 = add nsw i32 %sub12, 1
  %div14 = sdiv i32 %add13, 1
  %conv15 = sext i32 %div14 to i64
  %mul16 = mul nsw i64 %mul, %conv15
  %12 = load i32, i32* %.capture_expr.3, align 4, !tbaa !3
  %sub17 = sub nsw i32 %12, 0
  %sub18 = sub nsw i32 %sub17, 1
  %add19 = add nsw i32 %sub18, 1
  %div20 = sdiv i32 %add19, 1
  %conv21 = sext i32 %div20 to i64
  %mul22 = mul nsw i64 %mul16, %conv21
  %sub23 = sub nsw i64 %mul22, 1
  store i64 %sub23, i64* %.capture_expr.4, align 8, !tbaa !7
  %13 = bitcast i64* %.omp.uncollapsed.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %13) #1
  store i64 0, i64* %.omp.uncollapsed.lb, align 8, !tbaa !7
  %14 = bitcast i64* %.omp.uncollapsed.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %14) #1
  %15 = load i32, i32* %.capture_expr., align 4, !tbaa !3
  %sub24 = sub nsw i32 %15, 0
  %sub25 = sub nsw i32 %sub24, 1
  %add26 = add nsw i32 %sub25, 1
  %div27 = sdiv i32 %add26, 1
  %conv28 = sext i32 %div27 to i64
  %sub29 = sub nsw i64 %conv28, 1
  store i64 %sub29, i64* %.omp.uncollapsed.ub, align 8, !tbaa !7
  %16 = bitcast i64* %.omp.uncollapsed.lb30 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %16) #1
  store i64 0, i64* %.omp.uncollapsed.lb30, align 8, !tbaa !7
  %17 = bitcast i64* %.omp.uncollapsed.ub31 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %17) #1
  %18 = load i32, i32* %.capture_expr.1, align 4, !tbaa !3
  %sub32 = sub nsw i32 %18, 0
  %sub33 = sub nsw i32 %sub32, 1
  %add34 = add nsw i32 %sub33, 1
  %div35 = sdiv i32 %add34, 1
  %conv36 = sext i32 %div35 to i64
  %sub37 = sub nsw i64 %conv36, 1
  store i64 %sub37, i64* %.omp.uncollapsed.ub31, align 8, !tbaa !7
  %19 = bitcast i64* %.omp.uncollapsed.lb38 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %19) #1
  store i64 0, i64* %.omp.uncollapsed.lb38, align 8, !tbaa !7
  %20 = bitcast i64* %.omp.uncollapsed.ub39 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %20) #1
  %21 = load i32, i32* %.capture_expr.2, align 4, !tbaa !3
  %sub40 = sub nsw i32 %21, 0
  %sub41 = sub nsw i32 %sub40, 1
  %add42 = add nsw i32 %sub41, 1
  %div43 = sdiv i32 %add42, 1
  %conv44 = sext i32 %div43 to i64
  %sub45 = sub nsw i64 %conv44, 1
  store i64 %sub45, i64* %.omp.uncollapsed.ub39, align 8, !tbaa !7
  %22 = bitcast i64* %.omp.uncollapsed.lb46 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %22) #1
  store i64 0, i64* %.omp.uncollapsed.lb46, align 8, !tbaa !7
  %23 = bitcast i64* %.omp.uncollapsed.ub47 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %23) #1
  %24 = load i32, i32* %.capture_expr.3, align 4, !tbaa !3
  %sub48 = sub nsw i32 %24, 0
  %sub49 = sub nsw i32 %sub48, 1
  %add50 = add nsw i32 %sub49, 1
  %div51 = sdiv i32 %add50, 1
  %conv52 = sext i32 %div51 to i64
  %sub53 = sub nsw i64 %conv52, 1
  store i64 %sub53, i64* %.omp.uncollapsed.ub47, align 8, !tbaa !7
  %25 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(i32* %.capture_expr.), "QUAL.OMP.FIRSTPRIVATE"(i32* %.capture_expr.1), "QUAL.OMP.FIRSTPRIVATE"(i32* %.capture_expr.2), "QUAL.OMP.FIRSTPRIVATE"(i32* %.capture_expr.3), "QUAL.OMP.PRIVATE"(i64* %.omp.uncollapsed.iv), "QUAL.OMP.PRIVATE"(i64* %.omp.uncollapsed.iv70), "QUAL.OMP.PRIVATE"(i64* %.omp.uncollapsed.iv71), "QUAL.OMP.PRIVATE"(i64* %.omp.uncollapsed.iv72), "QUAL.OMP.PRIVATE"(i32* %k), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %l), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.lb), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.ub), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.lb30), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.ub31), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.lb38), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.ub39), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.lb46), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.ub47), "QUAL.OMP.PRIVATE"(i32* %tmp), "QUAL.OMP.PRIVATE"(i32* %tmp54), "QUAL.OMP.PRIVATE"(i32* %tmp55), "QUAL.OMP.PRIVATE"(i32* %tmp56) ]
  %26 = load i32, i32* %.capture_expr., align 4, !tbaa !3
  %cmp = icmp slt i32 0, %26
  br i1 %cmp, label %land.lhs.true, label %omp.precond.end

land.lhs.true:                                    ; preds = %entry
  %27 = load i32, i32* %.capture_expr.1, align 4, !tbaa !3
  %cmp59 = icmp slt i32 0, %27
  br i1 %cmp59, label %land.lhs.true62, label %omp.precond.end

land.lhs.true62:                                  ; preds = %land.lhs.true
  %28 = load i32, i32* %.capture_expr.2, align 4, !tbaa !3
  %cmp63 = icmp slt i32 0, %28
  br i1 %cmp63, label %land.lhs.true66, label %omp.precond.end

land.lhs.true66:                                  ; preds = %land.lhs.true62
  %29 = load i32, i32* %.capture_expr.3, align 4, !tbaa !3
  %cmp67 = icmp slt i32 0, %29
  br i1 %cmp67, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %land.lhs.true66
  %30 = bitcast i64* %.omp.uncollapsed.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %30) #1
  %31 = bitcast i64* %.omp.uncollapsed.iv70 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %31) #1
  %32 = bitcast i64* %.omp.uncollapsed.iv71 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %32) #1
  %33 = bitcast i64* %.omp.uncollapsed.iv72 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %33) #1
  %34 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.COLLAPSE"(i32 4), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.lb), "QUAL.OMP.NORMALIZED.IV"(i64* %.omp.uncollapsed.iv, i64* %.omp.uncollapsed.iv70, i64* %.omp.uncollapsed.iv71, i64* %.omp.uncollapsed.iv72), "QUAL.OMP.NORMALIZED.UB"(i64* %.omp.uncollapsed.ub, i64* %.omp.uncollapsed.ub31, i64* %.omp.uncollapsed.ub39, i64* %.omp.uncollapsed.ub47), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.lb30), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.lb38), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.lb46), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %k), "QUAL.OMP.PRIVATE"(i32* %l) ]
  %35 = load i64, i64* %.omp.uncollapsed.lb, align 8, !tbaa !7
  store i64 %35, i64* %.omp.uncollapsed.iv, align 8, !tbaa !7
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc106, %omp.precond.then
  %36 = load i64, i64* %.omp.uncollapsed.iv, align 8, !tbaa !7
  %37 = load i64, i64* %.omp.uncollapsed.ub, align 8, !tbaa !7
  %cmp73 = icmp sle i64 %36, %37
  br i1 %cmp73, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end108

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %38 = load i64, i64* %.omp.uncollapsed.lb30, align 8, !tbaa !7
  store i64 %38, i64* %.omp.uncollapsed.iv70, align 8, !tbaa !7
  br label %omp.uncollapsed.loop.cond75

omp.uncollapsed.loop.cond75:                      ; preds = %omp.uncollapsed.loop.inc103, %omp.uncollapsed.loop.body
  %39 = load i64, i64* %.omp.uncollapsed.iv70, align 8, !tbaa !7
  %40 = load i64, i64* %.omp.uncollapsed.ub31, align 8, !tbaa !7
  %cmp76 = icmp sle i64 %39, %40
  br i1 %cmp76, label %omp.uncollapsed.loop.body78, label %omp.uncollapsed.loop.end105

omp.uncollapsed.loop.body78:                      ; preds = %omp.uncollapsed.loop.cond75
  %41 = load i64, i64* %.omp.uncollapsed.lb38, align 8, !tbaa !7
  store i64 %41, i64* %.omp.uncollapsed.iv71, align 8, !tbaa !7
  br label %omp.uncollapsed.loop.cond79

omp.uncollapsed.loop.cond79:                      ; preds = %omp.uncollapsed.loop.inc100, %omp.uncollapsed.loop.body78
  %42 = load i64, i64* %.omp.uncollapsed.iv71, align 8, !tbaa !7
  %43 = load i64, i64* %.omp.uncollapsed.ub39, align 8, !tbaa !7
  %cmp80 = icmp sle i64 %42, %43
  br i1 %cmp80, label %omp.uncollapsed.loop.body82, label %omp.uncollapsed.loop.end102

omp.uncollapsed.loop.body82:                      ; preds = %omp.uncollapsed.loop.cond79
  %44 = load i64, i64* %.omp.uncollapsed.lb46, align 8, !tbaa !7
  store i64 %44, i64* %.omp.uncollapsed.iv72, align 8, !tbaa !7
  br label %omp.uncollapsed.loop.cond83

omp.uncollapsed.loop.cond83:                      ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body82
  %45 = load i64, i64* %.omp.uncollapsed.iv72, align 8, !tbaa !7
  %46 = load i64, i64* %.omp.uncollapsed.ub47, align 8, !tbaa !7
  %cmp84 = icmp sle i64 %45, %46
  br i1 %cmp84, label %omp.uncollapsed.loop.body86, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body86:                      ; preds = %omp.uncollapsed.loop.cond83
  %47 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %47) #1
  %48 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %48) #1
  %49 = bitcast i32* %k to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %49) #1
  %50 = bitcast i32* %l to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %50) #1
  %51 = load i64, i64* %.omp.uncollapsed.iv, align 8, !tbaa !7
  %mul87 = mul nsw i64 %51, 1
  %add88 = add nsw i64 0, %mul87
  %conv89 = trunc i64 %add88 to i32
  store i32 %conv89, i32* %i, align 4, !tbaa !3
  %52 = load i64, i64* %.omp.uncollapsed.iv70, align 8, !tbaa !7
  %mul90 = mul nsw i64 %52, 1
  %add91 = add nsw i64 0, %mul90
  %conv92 = trunc i64 %add91 to i32
  store i32 %conv92, i32* %j, align 4, !tbaa !3
  %53 = load i64, i64* %.omp.uncollapsed.iv71, align 8, !tbaa !7
  %mul93 = mul nsw i64 %53, 1
  %add94 = add nsw i64 0, %mul93
  %conv95 = trunc i64 %add94 to i32
  store i32 %conv95, i32* %k, align 4, !tbaa !3
  %54 = load i64, i64* %.omp.uncollapsed.iv72, align 8, !tbaa !7
  %mul96 = mul nsw i64 %54, 1
  %add97 = add nsw i64 0, %mul96
  %conv98 = trunc i64 %add97 to i32
  store i32 %conv98, i32* %l, align 4, !tbaa !3
  %55 = bitcast i32* %l to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %55) #1
  %56 = bitcast i32* %k to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %56) #1
  %57 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %57) #1
  %58 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %58) #1
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.uncollapsed.loop.body86
  %59 = load i64, i64* %.omp.uncollapsed.iv72, align 8, !tbaa !7
  %add99 = add nsw i64 %59, 1
  store i64 %add99, i64* %.omp.uncollapsed.iv72, align 8, !tbaa !7
  br label %omp.uncollapsed.loop.cond83

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond83
  br label %omp.uncollapsed.loop.inc100

omp.uncollapsed.loop.inc100:                      ; preds = %omp.uncollapsed.loop.end
  %60 = load i64, i64* %.omp.uncollapsed.iv71, align 8, !tbaa !7
  %add101 = add nsw i64 %60, 1
  store i64 %add101, i64* %.omp.uncollapsed.iv71, align 8, !tbaa !7
  br label %omp.uncollapsed.loop.cond79

omp.uncollapsed.loop.end102:                      ; preds = %omp.uncollapsed.loop.cond79
  br label %omp.uncollapsed.loop.inc103

omp.uncollapsed.loop.inc103:                      ; preds = %omp.uncollapsed.loop.end102
  %61 = load i64, i64* %.omp.uncollapsed.iv70, align 8, !tbaa !7
  %add104 = add nsw i64 %61, 1
  store i64 %add104, i64* %.omp.uncollapsed.iv70, align 8, !tbaa !7
  br label %omp.uncollapsed.loop.cond75

omp.uncollapsed.loop.end105:                      ; preds = %omp.uncollapsed.loop.cond75
  br label %omp.uncollapsed.loop.inc106

omp.uncollapsed.loop.inc106:                      ; preds = %omp.uncollapsed.loop.end105
  %62 = load i64, i64* %.omp.uncollapsed.iv, align 8, !tbaa !7
  %add107 = add nsw i64 %62, 1
  store i64 %add107, i64* %.omp.uncollapsed.iv, align 8, !tbaa !7
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end108:                      ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %34) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.uncollapsed.loop.end108, %land.lhs.true66, %land.lhs.true62, %land.lhs.true, %entry
  %63 = bitcast i64* %.omp.uncollapsed.iv72 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %63) #1
  %64 = bitcast i64* %.omp.uncollapsed.iv71 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %64) #1
  %65 = bitcast i64* %.omp.uncollapsed.iv70 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %65) #1
  %66 = bitcast i64* %.omp.uncollapsed.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %66) #1
  call void @llvm.directive.region.exit(token %25) [ "DIR.OMP.END.TARGET"() ]
  %67 = bitcast i64* %.omp.uncollapsed.ub47 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %67) #1
  %68 = bitcast i64* %.omp.uncollapsed.lb46 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %68) #1
  %69 = bitcast i64* %.omp.uncollapsed.ub39 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %69) #1
  %70 = bitcast i64* %.omp.uncollapsed.lb38 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %70) #1
  %71 = bitcast i64* %.omp.uncollapsed.ub31 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %71) #1
  %72 = bitcast i64* %.omp.uncollapsed.lb30 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %72) #1
  %73 = bitcast i64* %.omp.uncollapsed.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %73) #1
  %74 = bitcast i64* %.omp.uncollapsed.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %74) #1
  %75 = bitcast i64* %.capture_expr.4 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %75) #1
  %76 = bitcast i32* %.capture_expr.3 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %76) #1
  %77 = bitcast i32* %.capture_expr.2 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %77) #1
  %78 = bitcast i32* %.capture_expr.1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %78) #1
  %79 = bitcast i32* %.capture_expr. to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %79) #1
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: nounwind uwtable
define internal void @.omp_offloading.requires_reg() #3 section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

declare dso_local void @__tgt_register_requires(i64)

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nounwind willreturn }
attributes #3 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 2052, i32 55981756, !"foo", i32 2, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 9.0.0"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"long", !5, i64 0}

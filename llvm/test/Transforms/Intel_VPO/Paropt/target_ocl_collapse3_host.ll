; RUN: opt -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s

; Original code:
; void foo(int n) {
; #pragma omp target parallel for collapse(3)
;   for (int i = 0; i < n; ++i)
;     for (int j = 0; j < n; ++j)
;       for (int k = 0; k < n; ++k);
; }

; Check that ND-range is created for the runtime:
; CHECK: [[NDDESC:%[a-zA-Z._0-9]+]] = alloca { i32, i32, i64, i64, i64, i64, i64, i64, i64, i64, i64 }
; CHECK: [[M0:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 0
; CHECK: store i32 3, i32* [[M0]]
; CHECK: [[M1:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 1
; CHECK: store i32 0, i32* [[M1]]
; CHECK: [[M2:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 2
; CHECK: store i64 0, i64* [[M2]]
; CHECK: [[M3:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 3
; CHECK: [[UB1:%[a-zA-Z._0-9]+]] = load i64, i64* [[UBPTR1:%[a-zA-Z._0-9]+]]
; CHECK: store i64 [[UB1]], i64* [[M3]]
; CHECK: [[M4:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 4
; CHECK: store i64 1, i64* [[M4]]
; CHECK: [[M5:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 5
; CHECK: store i64 0, i64* [[M5]]
; CHECK: [[M6:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 6
; CHECK: [[UB2:%[a-zA-Z._0-9]+]] = load i64, i64* [[UBPTR2:%[a-zA-Z._0-9]+]]
; CHECK: store i64 [[UB2]], i64* [[M6]]
; CHECK: [[M7:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 7
; CHECK: store i64 1, i64* [[M7]]
; CHECK: [[M8:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 8
; CHECK: store i64 0, i64* [[M8]]
; CHECK: [[M9:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 9
; CHECK: [[UB3:%[a-zA-Z._0-9]+]] = load i64, i64* [[UBPTR3:%[a-zA-Z._0-9]+]]
; CHECK: store i64 [[UB3]], i64* [[M9]]
; CHECK: [[M10:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 10
; CHECK: store i64 1, i64* [[M10]]
; CHECK: call void @__omp_offloading_804_35636bc_foo_l2({{.*}}i64* [[UBPTR3]],{{.*}}i64* [[UBPTR2]],{{.*}}i64* [[UBPTR1]]{{.*}})

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
  %.capture_expr.3 = alloca i64, align 8
  %.omp.uncollapsed.lb = alloca i64, align 8
  %.omp.uncollapsed.ub = alloca i64, align 8
  %.omp.uncollapsed.lb23 = alloca i64, align 8
  %.omp.uncollapsed.ub24 = alloca i64, align 8
  %.omp.uncollapsed.lb31 = alloca i64, align 8
  %.omp.uncollapsed.ub32 = alloca i64, align 8
  %tmp = alloca i32, align 4
  %tmp39 = alloca i32, align 4
  %tmp40 = alloca i32, align 4
  %.omp.uncollapsed.iv = alloca i64, align 8
  %.omp.uncollapsed.iv50 = alloca i64, align 8
  %.omp.uncollapsed.iv51 = alloca i64, align 8
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %k = alloca i32, align 4
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
  %6 = bitcast i64* %.capture_expr.3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %6) #1
  %7 = load i32, i32* %.capture_expr., align 4, !tbaa !3
  %sub = sub nsw i32 %7, 0
  %sub4 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub4, 1
  %div = sdiv i32 %add, 1
  %conv = sext i32 %div to i64
  %8 = load i32, i32* %.capture_expr.1, align 4, !tbaa !3
  %sub5 = sub nsw i32 %8, 0
  %sub6 = sub nsw i32 %sub5, 1
  %add7 = add nsw i32 %sub6, 1
  %div8 = sdiv i32 %add7, 1
  %conv9 = sext i32 %div8 to i64
  %mul = mul nsw i64 %conv, %conv9
  %9 = load i32, i32* %.capture_expr.2, align 4, !tbaa !3
  %sub10 = sub nsw i32 %9, 0
  %sub11 = sub nsw i32 %sub10, 1
  %add12 = add nsw i32 %sub11, 1
  %div13 = sdiv i32 %add12, 1
  %conv14 = sext i32 %div13 to i64
  %mul15 = mul nsw i64 %mul, %conv14
  %sub16 = sub nsw i64 %mul15, 1
  store i64 %sub16, i64* %.capture_expr.3, align 8, !tbaa !7
  %10 = bitcast i64* %.omp.uncollapsed.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %10) #1
  store i64 0, i64* %.omp.uncollapsed.lb, align 8, !tbaa !7
  %11 = bitcast i64* %.omp.uncollapsed.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %11) #1
  %12 = load i32, i32* %.capture_expr., align 4, !tbaa !3
  %sub17 = sub nsw i32 %12, 0
  %sub18 = sub nsw i32 %sub17, 1
  %add19 = add nsw i32 %sub18, 1
  %div20 = sdiv i32 %add19, 1
  %conv21 = sext i32 %div20 to i64
  %sub22 = sub nsw i64 %conv21, 1
  store i64 %sub22, i64* %.omp.uncollapsed.ub, align 8, !tbaa !7
  %13 = bitcast i64* %.omp.uncollapsed.lb23 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %13) #1
  store i64 0, i64* %.omp.uncollapsed.lb23, align 8, !tbaa !7
  %14 = bitcast i64* %.omp.uncollapsed.ub24 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %14) #1
  %15 = load i32, i32* %.capture_expr.1, align 4, !tbaa !3
  %sub25 = sub nsw i32 %15, 0
  %sub26 = sub nsw i32 %sub25, 1
  %add27 = add nsw i32 %sub26, 1
  %div28 = sdiv i32 %add27, 1
  %conv29 = sext i32 %div28 to i64
  %sub30 = sub nsw i64 %conv29, 1
  store i64 %sub30, i64* %.omp.uncollapsed.ub24, align 8, !tbaa !7
  %16 = bitcast i64* %.omp.uncollapsed.lb31 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %16) #1
  store i64 0, i64* %.omp.uncollapsed.lb31, align 8, !tbaa !7
  %17 = bitcast i64* %.omp.uncollapsed.ub32 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %17) #1
  %18 = load i32, i32* %.capture_expr.2, align 4, !tbaa !3
  %sub33 = sub nsw i32 %18, 0
  %sub34 = sub nsw i32 %sub33, 1
  %add35 = add nsw i32 %sub34, 1
  %div36 = sdiv i32 %add35, 1
  %conv37 = sext i32 %div36 to i64
  %sub38 = sub nsw i64 %conv37, 1
  store i64 %sub38, i64* %.omp.uncollapsed.ub32, align 8, !tbaa !7
  %19 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(i32* %.capture_expr.), "QUAL.OMP.FIRSTPRIVATE"(i32* %.capture_expr.1), "QUAL.OMP.FIRSTPRIVATE"(i32* %.capture_expr.2), "QUAL.OMP.PRIVATE"(i64* %.omp.uncollapsed.iv), "QUAL.OMP.PRIVATE"(i64* %.omp.uncollapsed.iv50), "QUAL.OMP.PRIVATE"(i64* %.omp.uncollapsed.iv51), "QUAL.OMP.PRIVATE"(i32* %k), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.lb), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.ub), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.lb23), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.ub24), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.lb31), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.ub32), "QUAL.OMP.PRIVATE"(i32* %tmp), "QUAL.OMP.PRIVATE"(i32* %tmp39), "QUAL.OMP.PRIVATE"(i32* %tmp40) ]
  %20 = load i32, i32* %.capture_expr., align 4, !tbaa !3
  %cmp = icmp slt i32 0, %20
  br i1 %cmp, label %land.lhs.true, label %omp.precond.end

land.lhs.true:                                    ; preds = %entry
  %21 = load i32, i32* %.capture_expr.1, align 4, !tbaa !3
  %cmp43 = icmp slt i32 0, %21
  br i1 %cmp43, label %land.lhs.true46, label %omp.precond.end

land.lhs.true46:                                  ; preds = %land.lhs.true
  %22 = load i32, i32* %.capture_expr.2, align 4, !tbaa !3
  %cmp47 = icmp slt i32 0, %22
  br i1 %cmp47, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %land.lhs.true46
  %23 = bitcast i64* %.omp.uncollapsed.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %23) #1
  %24 = bitcast i64* %.omp.uncollapsed.iv50 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %24) #1
  %25 = bitcast i64* %.omp.uncollapsed.iv51 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %25) #1
  %26 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.COLLAPSE"(i32 3), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.lb), "QUAL.OMP.NORMALIZED.IV"(i64* %.omp.uncollapsed.iv, i64* %.omp.uncollapsed.iv50, i64* %.omp.uncollapsed.iv51), "QUAL.OMP.NORMALIZED.UB"(i64* %.omp.uncollapsed.ub, i64* %.omp.uncollapsed.ub24, i64* %.omp.uncollapsed.ub32), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.lb23), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.uncollapsed.lb31), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %k) ]
  %27 = load i64, i64* %.omp.uncollapsed.lb, align 8, !tbaa !7
  store i64 %27, i64* %.omp.uncollapsed.iv, align 8, !tbaa !7
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc75, %omp.precond.then
  %28 = load i64, i64* %.omp.uncollapsed.iv, align 8, !tbaa !7
  %29 = load i64, i64* %.omp.uncollapsed.ub, align 8, !tbaa !7
  %cmp52 = icmp sle i64 %28, %29
  br i1 %cmp52, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end77

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %30 = load i64, i64* %.omp.uncollapsed.lb23, align 8, !tbaa !7
  store i64 %30, i64* %.omp.uncollapsed.iv50, align 8, !tbaa !7
  br label %omp.uncollapsed.loop.cond54

omp.uncollapsed.loop.cond54:                      ; preds = %omp.uncollapsed.loop.inc72, %omp.uncollapsed.loop.body
  %31 = load i64, i64* %.omp.uncollapsed.iv50, align 8, !tbaa !7
  %32 = load i64, i64* %.omp.uncollapsed.ub24, align 8, !tbaa !7
  %cmp55 = icmp sle i64 %31, %32
  br i1 %cmp55, label %omp.uncollapsed.loop.body57, label %omp.uncollapsed.loop.end74

omp.uncollapsed.loop.body57:                      ; preds = %omp.uncollapsed.loop.cond54
  %33 = load i64, i64* %.omp.uncollapsed.lb31, align 8, !tbaa !7
  store i64 %33, i64* %.omp.uncollapsed.iv51, align 8, !tbaa !7
  br label %omp.uncollapsed.loop.cond58

omp.uncollapsed.loop.cond58:                      ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body57
  %34 = load i64, i64* %.omp.uncollapsed.iv51, align 8, !tbaa !7
  %35 = load i64, i64* %.omp.uncollapsed.ub32, align 8, !tbaa !7
  %cmp59 = icmp sle i64 %34, %35
  br i1 %cmp59, label %omp.uncollapsed.loop.body61, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body61:                      ; preds = %omp.uncollapsed.loop.cond58
  %36 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %36) #1
  %37 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %37) #1
  %38 = bitcast i32* %k to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %38) #1
  %39 = load i64, i64* %.omp.uncollapsed.iv, align 8, !tbaa !7
  %mul62 = mul nsw i64 %39, 1
  %add63 = add nsw i64 0, %mul62
  %conv64 = trunc i64 %add63 to i32
  store i32 %conv64, i32* %i, align 4, !tbaa !3
  %40 = load i64, i64* %.omp.uncollapsed.iv50, align 8, !tbaa !7
  %mul65 = mul nsw i64 %40, 1
  %add66 = add nsw i64 0, %mul65
  %conv67 = trunc i64 %add66 to i32
  store i32 %conv67, i32* %j, align 4, !tbaa !3
  %41 = load i64, i64* %.omp.uncollapsed.iv51, align 8, !tbaa !7
  %mul68 = mul nsw i64 %41, 1
  %add69 = add nsw i64 0, %mul68
  %conv70 = trunc i64 %add69 to i32
  store i32 %conv70, i32* %k, align 4, !tbaa !3
  %42 = bitcast i32* %k to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %42) #1
  %43 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %43) #1
  %44 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %44) #1
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.uncollapsed.loop.body61
  %45 = load i64, i64* %.omp.uncollapsed.iv51, align 8, !tbaa !7
  %add71 = add nsw i64 %45, 1
  store i64 %add71, i64* %.omp.uncollapsed.iv51, align 8, !tbaa !7
  br label %omp.uncollapsed.loop.cond58

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond58
  br label %omp.uncollapsed.loop.inc72

omp.uncollapsed.loop.inc72:                       ; preds = %omp.uncollapsed.loop.end
  %46 = load i64, i64* %.omp.uncollapsed.iv50, align 8, !tbaa !7
  %add73 = add nsw i64 %46, 1
  store i64 %add73, i64* %.omp.uncollapsed.iv50, align 8, !tbaa !7
  br label %omp.uncollapsed.loop.cond54

omp.uncollapsed.loop.end74:                       ; preds = %omp.uncollapsed.loop.cond54
  br label %omp.uncollapsed.loop.inc75

omp.uncollapsed.loop.inc75:                       ; preds = %omp.uncollapsed.loop.end74
  %47 = load i64, i64* %.omp.uncollapsed.iv, align 8, !tbaa !7
  %add76 = add nsw i64 %47, 1
  store i64 %add76, i64* %.omp.uncollapsed.iv, align 8, !tbaa !7
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end77:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %26) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.uncollapsed.loop.end77, %land.lhs.true46, %land.lhs.true, %entry
  %48 = bitcast i64* %.omp.uncollapsed.iv51 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %48) #1
  %49 = bitcast i64* %.omp.uncollapsed.iv50 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %49) #1
  %50 = bitcast i64* %.omp.uncollapsed.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %50) #1
  call void @llvm.directive.region.exit(token %19) [ "DIR.OMP.END.TARGET"() ]
  %51 = bitcast i64* %.omp.uncollapsed.ub32 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %51) #1
  %52 = bitcast i64* %.omp.uncollapsed.lb31 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %52) #1
  %53 = bitcast i64* %.omp.uncollapsed.ub24 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %53) #1
  %54 = bitcast i64* %.omp.uncollapsed.lb23 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %54) #1
  %55 = bitcast i64* %.omp.uncollapsed.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %55) #1
  %56 = bitcast i64* %.omp.uncollapsed.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %56) #1
  %57 = bitcast i64* %.capture_expr.3 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %57) #1
  %58 = bitcast i32* %.capture_expr.2 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %58) #1
  %59 = bitcast i32* %.capture_expr.1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %59) #1
  %60 = bitcast i32* %.capture_expr. to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %60) #1
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

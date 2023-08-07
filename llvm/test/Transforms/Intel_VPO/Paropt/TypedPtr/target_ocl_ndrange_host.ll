; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Original code:
; void foo() {
; #pragma omp target parallel for collapse(2)
;   for (int i = 0; i <= 19; ++i)
;     for (int j = 0; j <= 23; ++j);
; }

; CHECK: store i32 19, i32* [[UBPTR1:%[a-zA-Z._0-9]+]]
; CHECK: store i32 23, i32* [[UBPTR2:%[a-zA-Z._0-9]+]]
; CHECK: [[NDDESC:%[a-zA-Z._0-9]+]] = alloca { i32, i32, i64, i64, i64, i64, i64, i64 }
; CHECK: [[M0:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 0
; CHECK: store i32 2, i32* [[M0]]
; CHECK: [[M1:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 1
; CHECK: store i32 0, i32* [[M1]]
; CHECK: [[M2:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 2
; CHECK: store i64 0, i64* [[M2]]
; CHECK: [[M3:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 3
; CHECK: [[UB2:%[a-zA-Z._0-9]+]] = load i32, i32* [[UBPTR2]]
; CHECK: [[UB2_:%[a-zA-Z._0-9]+]] = sext i32 [[UB2]] to i64
; CHECK: store i64 [[UB2_]], i64* [[M3]]
; CHECK: [[M4:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 4
; CHECK: store i64 1, i64* [[M4]]
; CHECK: [[M5:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 5
; CHECK: store i64 0, i64* [[M5]]
; CHECK: [[M6:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 6
; CHECK: [[UB1:%[a-zA-Z._0-9]+]] = load i32, i32* [[UBPTR1]]
; CHECK: [[UB1_:%[a-zA-Z._0-9]+]] = sext i32 [[UB1]] to i64
; CHECK: store i64 [[UB1_]], i64* [[M6]]
; CHECK: [[M7:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 7
; CHECK: store i64 1, i64* [[M7]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local void @foo() #0 {
entry:
  %.omp.uncollapsed.lb = alloca i32, align 4
  %.omp.uncollapsed.ub = alloca i32, align 4
  %.omp.uncollapsed.lb1 = alloca i32, align 4
  %.omp.uncollapsed.ub2 = alloca i32, align 4
  %tmp = alloca i32, align 4
  %tmp3 = alloca i32, align 4
  %.omp.uncollapsed.iv = alloca i32, align 4
  %.omp.uncollapsed.iv4 = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %0 = bitcast i32* %.omp.uncollapsed.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0)
  store i32 0, i32* %.omp.uncollapsed.lb, align 4
  %1 = bitcast i32* %.omp.uncollapsed.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1)
  store i32 19, i32* %.omp.uncollapsed.ub, align 4
  %2 = bitcast i32* %.omp.uncollapsed.lb1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2)
  store i32 0, i32* %.omp.uncollapsed.lb1, align 4
  %3 = bitcast i32* %.omp.uncollapsed.ub2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3)
  store i32 23, i32* %.omp.uncollapsed.ub2, align 4
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i32* %.omp.uncollapsed.iv), "QUAL.OMP.PRIVATE"(i32* %.omp.uncollapsed.iv4), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.uncollapsed.lb), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.uncollapsed.ub), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.uncollapsed.lb1), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.uncollapsed.ub2), "QUAL.OMP.PRIVATE"(i32* %tmp), "QUAL.OMP.PRIVATE"(i32* %tmp3) ]
  %5 = bitcast i32* %.omp.uncollapsed.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5)
  %6 = bitcast i32* %.omp.uncollapsed.iv4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %6)
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.COLLAPSE"(i32 2), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.uncollapsed.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.uncollapsed.iv, i32* %.omp.uncollapsed.iv4), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.uncollapsed.ub, i32* %.omp.uncollapsed.ub2), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.uncollapsed.lb1), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %j) ]
  %8 = load i32, i32* %.omp.uncollapsed.lb, align 4
  store i32 %8, i32* %.omp.uncollapsed.iv, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc11, %entry
  %9 = load i32, i32* %.omp.uncollapsed.iv, align 4
  %10 = load i32, i32* %.omp.uncollapsed.ub, align 4
  %cmp = icmp sle i32 %9, %10
  br i1 %cmp, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end13

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %11 = load i32, i32* %.omp.uncollapsed.lb1, align 4
  store i32 %11, i32* %.omp.uncollapsed.iv4, align 4
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.cond5:                       ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body
  %12 = load i32, i32* %.omp.uncollapsed.iv4, align 4
  %13 = load i32, i32* %.omp.uncollapsed.ub2, align 4
  %cmp6 = icmp sle i32 %12, %13
  br i1 %cmp6, label %omp.uncollapsed.loop.body7, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body7:                       ; preds = %omp.uncollapsed.loop.cond5
  %14 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %14)
  %15 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %15)
  %16 = load i32, i32* %.omp.uncollapsed.iv, align 4
  %mul = mul nsw i32 %16, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %17 = load i32, i32* %.omp.uncollapsed.iv4, align 4
  %mul8 = mul nsw i32 %17, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, i32* %j, align 4
  %18 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %18)
  %19 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %19)
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.uncollapsed.loop.body7
  %20 = load i32, i32* %.omp.uncollapsed.iv4, align 4
  %add10 = add nsw i32 %20, 1
  store i32 %add10, i32* %.omp.uncollapsed.iv4, align 4
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond5
  br label %omp.uncollapsed.loop.inc11

omp.uncollapsed.loop.inc11:                       ; preds = %omp.uncollapsed.loop.end
  %21 = load i32, i32* %.omp.uncollapsed.iv, align 4
  %add12 = add nsw i32 %21, 1
  store i32 %add12, i32* %.omp.uncollapsed.iv, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end13:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %22 = bitcast i32* %.omp.uncollapsed.iv4 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %22)
  %23 = bitcast i32* %.omp.uncollapsed.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %23)
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TARGET"() ]
  %24 = bitcast i32* %.omp.uncollapsed.ub2 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %24)
  %25 = bitcast i32* %.omp.uncollapsed.lb1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %25)
  %26 = bitcast i32* %.omp.uncollapsed.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %26)
  %27 = bitcast i32* %.omp.uncollapsed.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %27)
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)

attributes #0 = { "contains-openmp-target"="true" }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2052, i32 85987529, !"foo", i32 2, i32 0, i32 0}

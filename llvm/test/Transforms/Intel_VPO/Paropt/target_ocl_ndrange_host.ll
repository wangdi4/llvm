; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Original code:
; void foo() {
; #pragma omp target parallel for collapse(2)
;   for (int i = 0; i <= 19; ++i)
;     for (int j = 0; j <= 23; ++j);
; }

; CHECK: store i32 19, ptr [[UBPTR1:%[a-zA-Z._0-9]+]]
; CHECK: store i32 23, ptr [[UBPTR2:%[a-zA-Z._0-9]+]]
; CHECK: [[NDDESC:%[a-zA-Z._0-9]+]] = alloca { i32, i32, i64, i64, i64, i64, i64, i64 }
; CHECK: [[M0:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 0
; CHECK: store i32 2, ptr [[M0]]
; CHECK: [[M1:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 1
; CHECK: store i32 0, ptr [[M1]]
; CHECK: [[M2:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 2
; CHECK: store i64 0, ptr [[M2]]
; CHECK: [[M3:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 3
; CHECK: [[UB2:%[a-zA-Z._0-9]+]] = load i32, ptr [[UBPTR2]]
; CHECK: [[UB2_:%[a-zA-Z._0-9]+]] = sext i32 [[UB2]] to i64
; CHECK: store i64 [[UB2_]], ptr [[M3]]
; CHECK: [[M4:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 4
; CHECK: store i64 1, ptr [[M4]]
; CHECK: [[M5:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 5
; CHECK: store i64 0, ptr [[M5]]
; CHECK: [[M6:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 6
; CHECK: [[UB1:%[a-zA-Z._0-9]+]] = load i32, ptr [[UBPTR1]]
; CHECK: [[UB1_:%[a-zA-Z._0-9]+]] = sext i32 [[UB1]] to i64
; CHECK: store i64 [[UB1_]], ptr [[M6]]
; CHECK: [[M7:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 7
; CHECK: store i64 1, ptr [[M7]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local void @_Z3foov() #0 {
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
  store i32 0, ptr %.omp.uncollapsed.lb, align 4
  store i32 19, ptr %.omp.uncollapsed.ub, align 4
  store i32 0, ptr %.omp.uncollapsed.lb1, align 4
  store i32 23, ptr %.omp.uncollapsed.ub2, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.uncollapsed.iv, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.uncollapsed.lb, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.uncollapsed.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.uncollapsed.iv4, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.uncollapsed.lb1, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.uncollapsed.ub2, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp3, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.COLLAPSE"(i32 2),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.uncollapsed.iv, i32 0, ptr %.omp.uncollapsed.iv4, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.uncollapsed.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.uncollapsed.ub, i32 0, ptr %.omp.uncollapsed.ub2, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.uncollapsed.lb1, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1) ]

  %2 = load i32, ptr %.omp.uncollapsed.lb, align 4
  store i32 %2, ptr %.omp.uncollapsed.iv, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc11, %entry
  %3 = load i32, ptr %.omp.uncollapsed.iv, align 4
  %4 = load i32, ptr %.omp.uncollapsed.ub, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end13

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %5 = load i32, ptr %.omp.uncollapsed.lb1, align 4
  store i32 %5, ptr %.omp.uncollapsed.iv4, align 4
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.cond5:                       ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body
  %6 = load i32, ptr %.omp.uncollapsed.iv4, align 4
  %7 = load i32, ptr %.omp.uncollapsed.ub2, align 4
  %cmp6 = icmp sle i32 %6, %7
  br i1 %cmp6, label %omp.uncollapsed.loop.body7, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body7:                       ; preds = %omp.uncollapsed.loop.cond5
  %8 = load i32, ptr %.omp.uncollapsed.iv, align 4
  %mul = mul nsw i32 %8, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %9 = load i32, ptr %.omp.uncollapsed.iv4, align 4
  %mul8 = mul nsw i32 %9, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, ptr %j, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.uncollapsed.loop.body7
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.body.continue
  %10 = load i32, ptr %.omp.uncollapsed.iv4, align 4
  %add10 = add nsw i32 %10, 1
  store i32 %add10, ptr %.omp.uncollapsed.iv4, align 4
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond5
  br label %omp.uncollapsed.loop.inc11

omp.uncollapsed.loop.inc11:                       ; preds = %omp.uncollapsed.loop.end
  %11 = load i32, ptr %.omp.uncollapsed.iv, align 4
  %add12 = add nsw i32 %11, 1
  store i32 %add12, ptr %.omp.uncollapsed.iv, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end13:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}


declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { "contains-openmp-target"="true" }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2052, i32 85987529, !"foo", i32 2, i32 0, i32 0}

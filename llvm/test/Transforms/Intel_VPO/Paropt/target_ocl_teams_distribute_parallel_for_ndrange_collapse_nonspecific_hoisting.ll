; RUN: opt -passes='vpo-cfg-restructuring,vpo-paropt-loop-collapse' -vpo-paropt-gpu-execution-scheme=0 -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-gpu-execution-scheme=0 -S %s | FileCheck -check-prefixes=CHECK_FUNC_PROTO %s

; Original code:
; int main() {
;         static constexpr int N = 32;
;         int a[N][N];
;
; #pragma omp target teams distribute parallel for map(tofrom:a[:N][:N]) collapse(2)
;         for (int i = 0; i < N; ++i) {
;                 const int M = N*2;
;                 for (int j = 0; j < M; ++j)
;                         a[i][j] = i+j;
;         }
;
;         return 0;
; }

; The test checks that no _unnecessary_ UB hoisting happens when passing loop TC info with explicitly disabled specific ND-range

; CHECK-NOT: load i32, ptr %.omp.uncollapsed.ub
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"()
; CHECK: load i32, ptr %.omp.uncollapsed.ub

; CHECK_FUNC_PROTO: define internal void @__omp_offloading{{.*}}_Z4main_l7(ptr noalias %{{[.a-z0-9]+}}, i64 %{{[.a-z0-9]+}}, i64 %{{[.a-z0-9]+}}, i64 %{{[.a-z0-9]+}}, i64 %{{[.a-z0-9]+}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %a = alloca [32 x [32 x i32]], align 16
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
  store i32 0, ptr %retval, align 4
  store i32 0, ptr %.omp.uncollapsed.lb, align 4
  store i32 31, ptr %.omp.uncollapsed.ub, align 4
  store i32 0, ptr %.omp.uncollapsed.lb1, align 4
  store i32 31, ptr %.omp.uncollapsed.ub2, align 4
  %arrayidx = getelementptr inbounds [32 x [32 x i32]], ptr %a, i64 0, i64 0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %a, ptr %arrayidx, i64 4096, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
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

  %array.begin21 = getelementptr inbounds [32 x [32 x i32]], ptr %a, i32 0, i32 0, i32 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %a, i32 0, i64 1024),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.uncollapsed.iv, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.omp.uncollapsed.lb, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.omp.uncollapsed.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.uncollapsed.iv4, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.omp.uncollapsed.lb1, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.omp.uncollapsed.ub2, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp3, i32 0, i32 1) ]

  %array.begin = getelementptr inbounds [32 x [32 x i32]], ptr %a, i32 0, i32 0, i32 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.COLLAPSE"(i32 2),
    "QUAL.OMP.SHARED:TYPED"(ptr %a, i32 0, i64 1024),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.uncollapsed.iv, i32 0, ptr %.omp.uncollapsed.iv4, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.uncollapsed.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.uncollapsed.ub, i32 0, ptr %.omp.uncollapsed.ub2, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.uncollapsed.lb1, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1) ]

  %3 = load i32, ptr %.omp.uncollapsed.lb, align 4
  store i32 %3, ptr %.omp.uncollapsed.iv, align 4
  %4 = load i32, ptr %.omp.uncollapsed.iv, align 4
  %5 = load i32, ptr %.omp.uncollapsed.ub, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.uncollapsed.loop.lh, label %omp.uncollapsed.loop.end20

omp.uncollapsed.loop.lh:                          ; preds = %entry
  br label %omp.uncollapsed.loop.body

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.inc16, %omp.uncollapsed.loop.lh
  %6 = load i32, ptr %.omp.uncollapsed.lb1, align 4
  store i32 %6, ptr %.omp.uncollapsed.iv4, align 4
  %7 = load i32, ptr %.omp.uncollapsed.iv4, align 4
  %8 = load i32, ptr %.omp.uncollapsed.ub2, align 4
  %cmp5 = icmp sle i32 %7, %8
  br i1 %cmp5, label %omp.uncollapsed.loop.lh6, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.lh6:                         ; preds = %omp.uncollapsed.loop.body
  br label %omp.uncollapsed.loop.body7

omp.uncollapsed.loop.body7:                       ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.lh6
  %9 = load i32, ptr %.omp.uncollapsed.iv, align 4
  %mul = mul nsw i32 %9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %10 = load i32, ptr %.omp.uncollapsed.iv4, align 4
  %mul8 = mul nsw i32 %10, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, ptr %j, align 4
  %11 = load i32, ptr %i, align 4
  %12 = load i32, ptr %j, align 4
  %add10 = add nsw i32 %11, %12
  %13 = load i32, ptr %i, align 4
  %idxprom = sext i32 %13 to i64
  %arrayidx11 = getelementptr inbounds [32 x [32 x i32]], ptr %a, i64 0, i64 %idxprom
  %14 = load i32, ptr %j, align 4
  %idxprom12 = sext i32 %14 to i64
  %arrayidx13 = getelementptr inbounds [32 x i32], ptr %arrayidx11, i64 0, i64 %idxprom12
  store i32 %add10, ptr %arrayidx13, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.uncollapsed.loop.body7
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.body.continue
  %15 = load i32, ptr %.omp.uncollapsed.iv4, align 4
  %add14 = add nsw i32 %15, 1
  store i32 %add14, ptr %.omp.uncollapsed.iv4, align 4
  %16 = load i32, ptr %.omp.uncollapsed.iv4, align 4
  %17 = load i32, ptr %.omp.uncollapsed.ub2, align 4
  %cmp15 = icmp sle i32 %16, %17
  br i1 %cmp15, label %omp.uncollapsed.loop.body7, label %omp.uncollapsed.loop.end_crit_edge

omp.uncollapsed.loop.end_crit_edge:               ; preds = %omp.uncollapsed.loop.inc
  br label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.end_crit_edge, %omp.uncollapsed.loop.body
  br label %omp.uncollapsed.loop.inc16

omp.uncollapsed.loop.inc16:                       ; preds = %omp.uncollapsed.loop.end
  %18 = load i32, ptr %.omp.uncollapsed.iv, align 4
  %add17 = add nsw i32 %18, 1
  store i32 %add17, ptr %.omp.uncollapsed.iv, align 4
  %19 = load i32, ptr %.omp.uncollapsed.iv, align 4
  %20 = load i32, ptr %.omp.uncollapsed.ub, align 4
  %cmp18 = icmp sle i32 %19, %20
  br i1 %cmp18, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end_crit_edge19

omp.uncollapsed.loop.end_crit_edge19:             ; preds = %omp.uncollapsed.loop.inc16
  br label %omp.uncollapsed.loop.end20

omp.uncollapsed.loop.end20:                       ; preds = %omp.uncollapsed.loop.end_crit_edge19, %entry
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { "may-have-openmp-directive"="true" }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2050, i32 49130736, !"_Z4main", i32 7, i32 0, i32 0, i32 0}


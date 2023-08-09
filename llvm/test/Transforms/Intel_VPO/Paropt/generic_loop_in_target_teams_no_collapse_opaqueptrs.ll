; RUN: opt -bugpoint-enable-legacy-pm  -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S <%s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -S <%s | FileCheck %s

; // C++ test
; void bar(int,int);
; void foo() {
;   #pragma omp target teams loop collapse(2)
;   for(int i=0; i<100; i++)
;     for(int j=0; j<100; j++)
;       bar(i,j);
; }

; Check that BE loop collapsing does not collapse the loop so the parent TARGET
; construct gets a QUAL.OMP.OFFLOAD.NDRANGE with two uncollapsed loop upperbounds.
;
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), {{.*}}, "QUAL.OMP.OFFLOAD.NDRANGE"(ptr %.omp.uncollapsed.ub{{.*}}, i32 0, ptr %.omp.uncollapsed.ub{{.*}}, i32 0)
;
; In the nonperformant case (to be avoided) the loop was collapsed, resulting in
; the TARGET getting "QUAL.OMP.OFFLOAD.NDRANGE"(ptr %omp.collapsed.ub, i64 0)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z3foov() {
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
  store i32 99, ptr %.omp.uncollapsed.ub, align 4
  store i32 0, ptr %.omp.uncollapsed.lb1, align 4
  store i32 99, ptr %.omp.uncollapsed.ub2, align 4

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

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
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

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.COLLAPSE"(i32 2),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.uncollapsed.iv, i32 0, ptr %.omp.uncollapsed.iv4, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.uncollapsed.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.uncollapsed.ub, i32 0, ptr %.omp.uncollapsed.ub2, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.uncollapsed.lb1, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1) ]

  %3 = load i32, ptr %.omp.uncollapsed.lb, align 4
  store i32 %3, ptr %.omp.uncollapsed.iv, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc11, %entry
  %4 = load i32, ptr %.omp.uncollapsed.iv, align 4
  %5 = load i32, ptr %.omp.uncollapsed.ub, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end13

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %6 = load i32, ptr %.omp.uncollapsed.lb1, align 4
  store i32 %6, ptr %.omp.uncollapsed.iv4, align 4
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.cond5:                       ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body
  %7 = load i32, ptr %.omp.uncollapsed.iv4, align 4
  %8 = load i32, ptr %.omp.uncollapsed.ub2, align 4
  %cmp6 = icmp sle i32 %7, %8
  br i1 %cmp6, label %omp.uncollapsed.loop.body7, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body7:                       ; preds = %omp.uncollapsed.loop.cond5
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
  call void @_Z3barii(i32 noundef %11, i32 noundef %12)
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.uncollapsed.loop.body7
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.body.continue
  %13 = load i32, ptr %.omp.uncollapsed.iv4, align 4
  %add10 = add nsw i32 %13, 1
  store i32 %add10, ptr %.omp.uncollapsed.iv4, align 4
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond5
  br label %omp.uncollapsed.loop.inc11

omp.uncollapsed.loop.inc11:                       ; preds = %omp.uncollapsed.loop.end
  %14 = load i32, ptr %.omp.uncollapsed.iv, align 4
  %add12 = add nsw i32 %14, 1
  store i32 %add12, ptr %.omp.uncollapsed.iv, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end13:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.GENERICLOOP"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

declare dso_local void @_Z3barii(i32 noundef, i32 noundef)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2050, i32 49102707, !"_Z3foo", i32 3, i32 0, i32 0, i32 0}

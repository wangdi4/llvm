; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S %s 2>&1 | FileCheck %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -S %s 2>&1 | FileCheck %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -vpo-paropt-map-loop-bind-teams-to-distribute=false -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s 2>&1 | FileCheck %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -vpo-paropt-map-loop-bind-teams-to-distribute=false -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s 2>&1 | FileCheck %s

; Test src:
;
; int test[1000] = {0};
; int sum = 0;
;
; void foo() {
;  #pragma omp target teams map(tofrom:test[0:1000], sum) reduction(+:sum)
;  for (int i=0; i<1000; ++i) {
;    #pragma omp loop bind(teams) reduction(+:sum)
;    for (int j=0; j<100; j++) {
;      test[i] += i + j;
;      sum += i * j;
;    }
;  }
; }

; This test checks that prepare pass will map to DISTRIBUTE_PARLOOP if reduction clause
; is found on loop construct with teams binding.

; Verify that DIR.OMP.GENERICLOOP is mapped to DIR.OMP.DISTRIBUTE.PARLOOP
; CHECK-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), {{.*}}
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
; CHECK-NOT: "QUAL.OMP.BIND.TEAMS"

; CHECK-NOT: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.GENERICLOOP"() {{.*}}
; CHECK: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() {{.*}}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@test = dso_local global [1000 x i32] zeroinitializer, align 16
@sum = dso_local global i32 0, align 4

define dso_local void @_Z3foov() {
entry:
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %j = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr @sum, ptr @sum, i64 4, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr @test, ptr @test, i64 4000, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr @sum, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr @test, i32 0, i64 1000),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]

  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %2 = load i32, ptr %i, align 4
  %cmp = icmp slt i32 %2, 1000
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.BIND.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr @sum, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr @test, i32 0, i64 1000),
    "QUAL.OMP.SHARED:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1) ]

  %4 = load i32, ptr %.omp.lb, align 4
  store i32 %4, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %for.body
  %5 = load i32, ptr %.omp.iv, align 4
  %6 = load i32, ptr %.omp.ub, align 4
  %cmp1 = icmp sle i32 %5, %6
  br i1 %cmp1, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %j, align 4
  %8 = load i32, ptr %i, align 4
  %9 = load i32, ptr %j, align 4
  %add2 = add nsw i32 %8, %9
  %10 = load i32, ptr %i, align 4
  %idxprom = sext i32 %10 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @test, i64 0, i64 %idxprom
  %11 = load i32, ptr %arrayidx, align 4
  %add3 = add nsw i32 %11, %add2
  store i32 %add3, ptr %arrayidx, align 4
  %12 = load i32, ptr %i, align 4
  %13 = load i32, ptr %j, align 4
  %mul4 = mul nsw i32 %12, %13
  %14 = load i32, ptr @sum, align 4
  %add5 = add nsw i32 %14, %mul4
  store i32 %add5, ptr @sum, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %15 = load i32, ptr %.omp.iv, align 4
  %add6 = add nsw i32 %15, 1
  store i32 %add6, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %for.inc

for.inc:                                          ; preds = %omp.loop.exit
  %16 = load i32, ptr %i, align 4
  %inc = add nsw i32 %16, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond 

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry() 

declare void @llvm.directive.region.exit(token) 

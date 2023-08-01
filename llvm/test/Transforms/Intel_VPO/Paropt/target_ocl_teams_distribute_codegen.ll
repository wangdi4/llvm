; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s
;
; The compiler checks the loop is only partitioned by teams.
;#define MAX 100
;
;int A[MAX][MAX], B[MAX][MAX], C[MAX][MAX];
;
;void __attribute__ ((noinline)) Compute()
;{
;  #pragma omp target map(to: A, B) map(tofrom: C)
;  {
;    #pragma omp teams num_teams(2)
;    #pragma omp distribute
;    for (int i = 0; i < MAX; i++)
;    for (int j = 0; j < MAX; j++)
;    for (int k = 0; k < MAX; k++)
;         C[i][j] += A[i][k] * B[k][j];
;  }
;}

; CHECK: %{{.*}} = call spir_func i64 @_Z14get_num_groupsj(i32 0)
; CHECK: %{{.*}} = call spir_func i64 @_Z12get_group_idj(i32 0)
; CHECK-NOT: %{{.*}} = call{{.*}}@_Z14get_local_sizej(i32 0)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

@A = external dso_local addrspace(1) global [100 x [100 x i32]], align 4
@B = external dso_local addrspace(1) global [100 x [100 x i32]], align 4
@C = external dso_local addrspace(1) global [100 x [100 x i32]], align 4

define dso_local spir_func void @Compute() {
entry:
  %.omp.iv = alloca i32, align 4
  %0 = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %tmp = alloca i32, align 4
  %1 = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.lb = alloca i32, align 4
  %2 = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub = alloca i32, align 4
  %3 = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %i = alloca i32, align 4
  %4 = addrspacecast ptr %i to ptr addrspace(4)
  %j = alloca i32, align 4
  %5 = addrspacecast ptr %j to ptr addrspace(4)
  %k = alloca i32, align 4
  %6 = addrspacecast ptr %k to ptr addrspace(4)
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TO"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @A to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @A to ptr addrspace(4)), i64 40000, i64 33, ptr null, ptr null),
    "QUAL.OMP.MAP.TO"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @B to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @B to ptr addrspace(4)), i64 40000, i64 33, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @C to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @C to ptr addrspace(4)), i64 40000, i64 35, ptr null, ptr null),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %6, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %5, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %3, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %2, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %4, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %0, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %1, i32 0, i32 1) ]

  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.NUM_TEAMS"(i32 2),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %2, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %3, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %6, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %5, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %4, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @A to ptr addrspace(4)), i32 0, i32 10000),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @B to ptr addrspace(4)), i32 0, i32 10000),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @C to ptr addrspace(4)), i32 0, i32 10000),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %0, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %1, i32 0, i32 1) ]

  store i32 0, ptr addrspace(4) %2, align 4
  store i32 99, ptr addrspace(4) %3, align 4
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %2, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %0, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %3, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %4, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %5, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %6, i32 0, i32 1) ]

  %10 = load i32, ptr addrspace(4) %2, align 4
  store i32 %10, ptr addrspace(4) %0, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %11 = load i32, ptr addrspace(4) %0, align 4
  %12 = load i32, ptr addrspace(4) %3, align 4
  %cmp = icmp sle i32 %11, %12
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %13 = load i32, ptr addrspace(4) %0, align 4
  %mul = mul nsw i32 %13, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %4, align 4
  store i32 0, ptr addrspace(4) %5, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc17, %omp.inner.for.body
  %14 = load i32, ptr addrspace(4) %5, align 4
  %cmp1 = icmp slt i32 %14, 100
  br i1 %cmp1, label %for.body, label %for.end19

for.body:                                         ; preds = %for.cond
  store i32 0, ptr addrspace(4) %6, align 4
  br label %for.cond2

for.cond2:                                        ; preds = %for.inc, %for.body
  %15 = load i32, ptr addrspace(4) %6, align 4
  %cmp3 = icmp slt i32 %15, 100
  br i1 %cmp3, label %for.body4, label %for.end

for.body4:                                        ; preds = %for.cond2
  %16 = load i32, ptr addrspace(4) %4, align 4
  %idxprom = sext i32 %16 to i64
  %arrayidx = getelementptr inbounds [100 x [100 x i32]], ptr addrspace(4) addrspacecast (ptr addrspace(1) @A to ptr addrspace(4)), i64 0, i64 %idxprom
  %17 = load i32, ptr addrspace(4) %6, align 4
  %idxprom5 = sext i32 %17 to i64
  %arrayidx6 = getelementptr inbounds [100 x i32], ptr addrspace(4) %arrayidx, i64 0, i64 %idxprom5
  %18 = load i32, ptr addrspace(4) %arrayidx6, align 4
  %19 = load i32, ptr addrspace(4) %6, align 4
  %idxprom7 = sext i32 %19 to i64
  %arrayidx8 = getelementptr inbounds [100 x [100 x i32]], ptr addrspace(4) addrspacecast (ptr addrspace(1) @B to ptr addrspace(4)), i64 0, i64 %idxprom7
  %20 = load i32, ptr addrspace(4) %5, align 4
  %idxprom9 = sext i32 %20 to i64
  %arrayidx10 = getelementptr inbounds [100 x i32], ptr addrspace(4) %arrayidx8, i64 0, i64 %idxprom9
  %21 = load i32, ptr addrspace(4) %arrayidx10, align 4
  %mul11 = mul nsw i32 %18, %21
  %22 = load i32, ptr addrspace(4) %4, align 4
  %idxprom12 = sext i32 %22 to i64
  %arrayidx13 = getelementptr inbounds [100 x [100 x i32]], ptr addrspace(4) addrspacecast (ptr addrspace(1) @C to ptr addrspace(4)), i64 0, i64 %idxprom12
  %23 = load i32, ptr addrspace(4) %5, align 4
  %idxprom14 = sext i32 %23 to i64
  %arrayidx15 = getelementptr inbounds [100 x i32], ptr addrspace(4) %arrayidx13, i64 0, i64 %idxprom14
  %24 = load i32, ptr addrspace(4) %arrayidx15, align 4
  %add16 = add nsw i32 %24, %mul11
  store i32 %add16, ptr addrspace(4) %arrayidx15, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body4
  %25 = load i32, ptr addrspace(4) %6, align 4
  %inc = add nsw i32 %25, 1
  store i32 %inc, ptr addrspace(4) %6, align 4
  br label %for.cond2

for.end:                                          ; preds = %for.cond2
  br label %for.inc17

for.inc17:                                        ; preds = %for.end
  %26 = load i32, ptr addrspace(4) %5, align 4
  %inc18 = add nsw i32 %26, 1
  store i32 %inc18, ptr addrspace(4) %5, align 4
  br label %for.cond

for.end19:                                        ; preds = %for.cond
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end19
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %27 = load i32, ptr addrspace(4) %0, align 4
  %add20 = add nsw i32 %27, 1
  store i32 %add20, ptr addrspace(4) %0, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.DISTRIBUTE"() ]
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2052, i32 85985690, !"Compute", i32 7, i32 0, i32 0}

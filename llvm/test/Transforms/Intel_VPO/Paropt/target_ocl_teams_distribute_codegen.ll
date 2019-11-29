; RUN: opt < %s -prepare-switch-to-offload=true -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt  -S | FileCheck %s
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -prepare-switch-to-offload=true -switch-to-offload  -S | FileCheck %s
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

; CHECK: %{{.*}} = call i64 @_Z14get_num_groupsj(i32 0)
; CHECK: %{{.*}} = call i64 @_Z12get_group_idj(i32 0)
; CHECK-NOT: %{{.*}} = call i64 @_Z14get_local_sizej(i32 0)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

@A = external dso_local addrspace(1) global [100 x [100 x i32]], align 4
@B = external dso_local addrspace(1) global [100 x [100 x i32]], align 4
@C = external dso_local addrspace(1) global [100 x [100 x i32]], align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @Compute() #0 {
entry:
  %.omp.iv = alloca i32, align 4
  %0 = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %1 = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %2 = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %3 = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %i = alloca i32, align 4
  %4 = addrspacecast i32* %i to i32 addrspace(4)*
  %j = alloca i32, align 4
  %5 = addrspacecast i32* %j to i32 addrspace(4)*
  %k = alloca i32, align 4
  %6 = addrspacecast i32* %k to i32 addrspace(4)*
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TO"([100 x [100 x i32]] addrspace(4)* addrspacecast ([100 x [100 x i32]] addrspace(1)* @A to [100 x [100 x i32]] addrspace(4)*)), "QUAL.OMP.MAP.TO"([100 x [100 x i32]] addrspace(4)* addrspacecast ([100 x [100 x i32]] addrspace(1)* @B to [100 x [100 x i32]] addrspace(4)*)), "QUAL.OMP.MAP.TOFROM"([100 x [100 x i32]] addrspace(4)* addrspacecast ([100 x [100 x i32]] addrspace(1)* @C to [100 x [100 x i32]] addrspace(4)*)), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %6), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %5), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %3), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %2), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %4), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %0), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %1) ]
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS"(i32 2), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %2), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %3), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %6), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %5), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %4), "QUAL.OMP.SHARED"([100 x [100 x i32]] addrspace(4)* addrspacecast ([100 x [100 x i32]] addrspace(1)* @A to [100 x [100 x i32]] addrspace(4)*)), "QUAL.OMP.SHARED"([100 x [100 x i32]] addrspace(4)* addrspacecast ([100 x [100 x i32]] addrspace(1)* @B to [100 x [100 x i32]] addrspace(4)*)), "QUAL.OMP.SHARED"([100 x [100 x i32]] addrspace(4)* addrspacecast ([100 x [100 x i32]] addrspace(1)* @C to [100 x [100 x i32]] addrspace(4)*)), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %0), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %1) ]
  store i32 0, i32 addrspace(4)* %2, align 4
  store i32 99, i32 addrspace(4)* %3, align 4
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %2), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %0), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %3), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %4), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %5), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %6) ]
  %10 = load i32, i32 addrspace(4)* %2, align 4
  store i32 %10, i32 addrspace(4)* %0, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %11 = load i32, i32 addrspace(4)* %0, align 4
  %12 = load i32, i32 addrspace(4)* %3, align 4
  %cmp = icmp sle i32 %11, %12
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %13 = load i32, i32 addrspace(4)* %0, align 4
  %mul = mul nsw i32 %13, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %4, align 4
  store i32 0, i32 addrspace(4)* %5, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc17, %omp.inner.for.body
  %14 = load i32, i32 addrspace(4)* %5, align 4
  %cmp1 = icmp slt i32 %14, 100
  br i1 %cmp1, label %for.body, label %for.end19

for.body:                                         ; preds = %for.cond
  store i32 0, i32 addrspace(4)* %6, align 4
  br label %for.cond2

for.cond2:                                        ; preds = %for.inc, %for.body
  %15 = load i32, i32 addrspace(4)* %6, align 4
  %cmp3 = icmp slt i32 %15, 100
  br i1 %cmp3, label %for.body4, label %for.end

for.body4:                                        ; preds = %for.cond2
  %16 = load i32, i32 addrspace(4)* %4, align 4
  %idxprom = sext i32 %16 to i64
  %arrayidx = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]] addrspace(4)* addrspacecast ([100 x [100 x i32]] addrspace(1)* @A to [100 x [100 x i32]] addrspace(4)*), i64 0, i64 %idxprom
  %17 = load i32, i32 addrspace(4)* %6, align 4
  %idxprom5 = sext i32 %17 to i64
  %arrayidx6 = getelementptr inbounds [100 x i32], [100 x i32] addrspace(4)* %arrayidx, i64 0, i64 %idxprom5
  %18 = load i32, i32 addrspace(4)* %arrayidx6, align 4
  %19 = load i32, i32 addrspace(4)* %6, align 4
  %idxprom7 = sext i32 %19 to i64
  %arrayidx8 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]] addrspace(4)* addrspacecast ([100 x [100 x i32]] addrspace(1)* @B to [100 x [100 x i32]] addrspace(4)*), i64 0, i64 %idxprom7
  %20 = load i32, i32 addrspace(4)* %5, align 4
  %idxprom9 = sext i32 %20 to i64
  %arrayidx10 = getelementptr inbounds [100 x i32], [100 x i32] addrspace(4)* %arrayidx8, i64 0, i64 %idxprom9
  %21 = load i32, i32 addrspace(4)* %arrayidx10, align 4
  %mul11 = mul nsw i32 %18, %21
  %22 = load i32, i32 addrspace(4)* %4, align 4
  %idxprom12 = sext i32 %22 to i64
  %arrayidx13 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]] addrspace(4)* addrspacecast ([100 x [100 x i32]] addrspace(1)* @C to [100 x [100 x i32]] addrspace(4)*), i64 0, i64 %idxprom12
  %23 = load i32, i32 addrspace(4)* %5, align 4
  %idxprom14 = sext i32 %23 to i64
  %arrayidx15 = getelementptr inbounds [100 x i32], [100 x i32] addrspace(4)* %arrayidx13, i64 0, i64 %idxprom14
  %24 = load i32, i32 addrspace(4)* %arrayidx15, align 4
  %add16 = add nsw i32 %24, %mul11
  store i32 %add16, i32 addrspace(4)* %arrayidx15, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body4
  %25 = load i32, i32 addrspace(4)* %6, align 4
  %inc = add nsw i32 %25, 1
  store i32 %inc, i32 addrspace(4)* %6, align 4
  br label %for.cond2

for.end:                                          ; preds = %for.cond2
  br label %for.inc17

for.inc17:                                        ; preds = %for.end
  %26 = load i32, i32 addrspace(4)* %5, align 4
  %inc18 = add nsw i32 %26, 1
  store i32 %inc18, i32 addrspace(4)* %5, align 4
  br label %for.cond

for.end19:                                        ; preds = %for.cond
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end19
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %27 = load i32, i32 addrspace(4)* %0, align 4
  %add20 = add nsw i32 %27, 1
  store i32 %add20, i32 addrspace(4)* %0, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.DISTRIBUTE"() ]
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2052, i32 85985690, !"Compute", i32 7, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 9.0.0"}

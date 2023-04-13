; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; int main(void)
; {
;   int i;
;   int sum[2] = {0, 0};
;
; #pragma omp target teams distribute parallel for reduction(+:sum[0])
;   for (i=0; i<10; i++) {
;     sum[0]+=i;
;   }
;
;   return 0;
; }

; Check no crash happens because of treating zero-offset typed array-section reduction clause
; as a scalar one in presence of multiple clauses.
; The crash would happen due to pointer types mismatch when generation reduction variable
; load, so this test only makes sense for typed pointers. A bitcast (RED_VAR_PTR_CAST0|1 below) ensures proper types matching.

; CHECK: define weak dso_local spir_kernel void @__omp_offloading{{.*}}main{{.*}}([2 x i32] addrspace(1)* noalias %[[RESULT_PTR0:sum.*]], [2 x i32] addrspace(1)* noalias %[[RESULT_PTR1:sum1.*]], i32 addrspace(1)* %red_local_buf, i32 addrspace(1)* %red_local_buf{{.*}}, i32 addrspace(1)* %[[RED_GLOBAL_BUF0:red_buf.*]], i32 addrspace(1)* %[[RED_GLOBAL_BUF1:red_buf.*]], i32 addrspace(1)* %[[TEAMS_COUNTER_PTR:teams_counter.*]], i64 %.omp.lb
; CHECK: %[[GROUP_ID1:[^,]+]] = call spir_func i64 @_Z12get_group_idj(i32 0)
; CHECK: %[[LOCAL_SUM_GEP1:[^,]+]] = getelementptr inbounds i32, i32 addrspace(1)* %[[RED_GLOBAL_BUF1]], i64 %[[GROUP_ID1]]
; CHECK: %[[LOCAL_SUM_PTR_CAST1:[^,]+]] = bitcast i32 addrspace(1)* %[[LOCAL_SUM_GEP1]] to [2 x i32] addrspace(1)*
; CHECK: %[[GROUP_ID0:[^,]+]] = call spir_func i64 @_Z12get_group_idj(i32 0)
; CHECK: %[[LOCAL_SUM_GEP0:[^,]+]] = getelementptr inbounds i32, i32 addrspace(1)* %[[RED_GLOBAL_BUF0]], i64 %[[GROUP_ID0]]
; CHECK: %[[LOCAL_SUM_PTR_CAST0:[^,]+]] = bitcast i32 addrspace(1)* %[[LOCAL_SUM_GEP0]] to [2 x i32] addrspace(1)*
; CHECK: %[[RED_VAR_PTR_CAST1:[^,]+]] = bitcast [2 x i32] addrspace(1)* %[[LOCAL_SUM_PTR_CAST1]] to i32 addrspace(1)*
; CHECK: %[[RED_VAR_PTR_CAST0:[^,]+]] = bitcast [2 x i32] addrspace(1)* %[[LOCAL_SUM_PTR_CAST0]] to i32 addrspace(1)*
; CHECK-LABEL: atomic.free.red.local.update.update.exit:
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 272)
; CHECK: %[[LOCAL_SUM_LD0:[^,]+]] = load i32, i32 addrspace(1)* %[[RED_VAR_PTR_CAST0]]
; CHECK: %[[LOCAL_RED_ADD0:[^,]+]] = add i32 %[[LOCAL_SUM_LD0]]
; CHECK: store i32 %[[LOCAL_RED_ADD0]], i32 addrspace(1)* %[[RED_VAR_PTR_CAST0]]
; CHECK: %[[LOCAL_SUM_LD1:[^,]+]] = load i32, i32 addrspace(1)* %[[RED_VAR_PTR_CAST1]]
; CHECK: %[[LOCAL_RED_ADD1:[^,]+]] = add i32 %[[LOCAL_SUM_LD1]]
; CHECK: store i32 %[[LOCAL_RED_ADD1]], i32 addrspace(1)* %[[RED_VAR_PTR_CAST1]]

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind
define hidden i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %sum = alloca [2 x i32], align 4
  %sum1 = alloca [2 x i32], align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %sum.ascast = addrspacecast [2 x i32]* %sum to [2 x i32] addrspace(4)*
  %sum1.ascast = addrspacecast [2 x i32]* %sum1 to [2 x i32] addrspace(4)*
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %retval.ascast, align 4
  %0 = bitcast [2 x i32] addrspace(4)* %sum.ascast to i8 addrspace(4)*
  call void @llvm.memset.p4i8.i64(i8 addrspace(4)* align 4 %0, i8 0, i64 8, i1 false)
  %1 = bitcast [2 x i32] addrspace(4)* %sum1.ascast to i8 addrspace(4)*
  call void @llvm.memset.p4i8.i64(i8 addrspace(4)* align 4 %1, i8 0, i64 8, i1 false)
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 9, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %arrayidx = getelementptr inbounds [2 x i32], [2 x i32] addrspace(4)* %sum.ascast, i64 0, i64 0
  %arrayidx1 = getelementptr inbounds [2 x i32], [2 x i32] addrspace(4)* %sum1.ascast, i64 0, i64 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"([2 x i32] addrspace(4)* %sum.ascast, i32 addrspace(4)* %arrayidx, i64 4, i64 547, i8* null, i8* null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"([2 x i32] addrspace(4)* %sum1.ascast, i32 addrspace(4)* %arrayidx1, i64 4, i64 547, i8* null, i8* null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %tmp.ascast, i32 0, i32 1) ]

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"([2 x i32] addrspace(4)* %sum.ascast, i32 0, i64 1, i64 0),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"([2 x i32] addrspace(4)* %sum1.ascast, i32 0, i64 1, i64 0),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(i32 addrspace(4)* %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(i32 addrspace(4)* %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %tmp.ascast, i32 0, i32 1) ]

  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"([2 x i32] addrspace(4)* %sum.ascast, i32 0, i64 1, i64 0),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"([2 x i32] addrspace(4)* %sum1.ascast, i32 0, i64 1, i64 0),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %i.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(i32 addrspace(4)* %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(i32 addrspace(4)* %.omp.ub.ascast, i32 0) ]

  %5 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %5, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %7 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %6, %7
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %8, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4
  %9 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %arrayidx2 = getelementptr inbounds [2 x i32], [2 x i32] addrspace(4)* %sum.ascast, i64 0, i64 0
  %10 = load i32, i32 addrspace(4)* %arrayidx2, align 4
  %add3 = add nsw i32 %10, %9
  store i32 %add3, i32 addrspace(4)* %arrayidx2, align 4
  %11 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %arrayidx4 = getelementptr inbounds [2 x i32], [2 x i32] addrspace(4)* %sum1.ascast, i64 0, i64 0
  %12 = load i32, i32 addrspace(4)* %arrayidx4, align 4
  %add5 = add nsw i32 %12, %11
  store i32 %add5, i32 addrspace(4)* %arrayidx4, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %13 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add6 = add nsw i32 %13, 1
  store i32 %add6, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p4i8.i64(i8 addrspace(4)* nocapture writeonly, i8, i64, i1 immarg) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { convergent mustprogress noinline norecurse nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #2 = { nounwind }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 66306, i32 204411266, !"_Z4main", i32 6, i32 0, i32 0, i32 0}


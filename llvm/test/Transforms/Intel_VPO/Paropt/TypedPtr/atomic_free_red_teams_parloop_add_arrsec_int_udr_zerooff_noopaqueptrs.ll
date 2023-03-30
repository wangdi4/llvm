; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; int main(void)
; {
;   int i;
;   int sum[2] = {0, 0};
;
; #pragma omp declare reduction(myadd : int : omp_out += omp_in) initializer (omp_priv = 0)
; #pragma omp target teams distribute parallel for reduction(+:sum[0])
;   for (i=0; i<10; i++) {
;     sum[0]+=i;
;   }
;
;   return 0;
; }

; Check no crash happens because of treating zero-offset typed array-section reduction clause
; as a scalar one. The crash would happen due to pointer types mismatch when generation reduction variable
; load, so this test only makes sense for typed pointers. A bitcast (RED_VAR_PTR_CAST below) ensures proper types matching.
; This test checks UDR codegen satisifies the statement above.

; CHECK: define weak dso_local spir_kernel void @__omp_offloading{{.*}}main{{.*}}([2 x i32] addrspace(1)* noalias %[[RESULT_PTR:sum.*]], i32 addrspace(1)* %[[RED_GLOBAL_BUF:red_buf.*]], i32 addrspace(1)* %[[TEAMS_COUNTER_PTR:teams_counter.*]], i64 %.omp.lb
; CHECK: %[[GROUP_ID:[^,]+]] = call spir_func i64 @_Z12get_group_idj(i32 0)
; CHECK: %[[LOCAL_SUM_GEP:[^,]+]] = getelementptr inbounds i32, i32 addrspace(1)* %[[RED_GLOBAL_BUF]], i64 %[[GROUP_ID]]
; CHECK: %[[LOCAL_SUM_PTR_CAST:[^,]+]] = bitcast i32 addrspace(1)* %[[LOCAL_SUM_GEP]] to [2 x i32] addrspace(1)*
; CHECK: %[[RED_VAR_PTR_CAST:[^,]+]] = bitcast [2 x i32] addrspace(1)* %[[LOCAL_SUM_PTR_CAST]] to i32 addrspace(1)*
; CHECK: %[[RED_VAR_PTR_CAST_ASCAST:[^,]+]] = addrspacecast i32 addrspace(1)* %[[RED_VAR_PTR_CAST]] to i32 addrspace(4)*
; CHECK-LABEL: atomic.free.red.local.update.update.body:
; CHECK: call spir_func void @.omp_combiner.(i32 addrspace(4)* %[[RED_VAR_PTR_CAST_ASCAST]]

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind
define protected noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %sum = alloca [2 x i32], align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %sum.ascast = addrspacecast [2 x i32]* %sum to [2 x i32] addrspace(4)*
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %retval.ascast, align 4
  %0 = bitcast [2 x i32] addrspace(4)* %sum.ascast to i8 addrspace(4)*
  call void @llvm.memset.p4i8.i64(i8 addrspace(4)* align 4 %0, i8 0, i64 8, i1 false)
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 9, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %arrayidx = getelementptr inbounds [2 x i32], [2 x i32] addrspace(4)* %sum.ascast, i64 0, i64 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"([2 x i32] addrspace(4)* %sum.ascast, i32 addrspace(4)* %arrayidx, i64 4, i64 547, i8* null, i8* null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %tmp.ascast, i32 0, i32 1) ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.UDR:ARRSECT.TYPED"([2 x i32] addrspace(4)* %sum.ascast, i32 0, i64 1, i64 0, i8* null, i8* null, void (i32 addrspace(4)*, i32 addrspace(4)*)* @.omp_combiner., void (i32 addrspace(4)*, i32 addrspace(4)*)* @.omp_initializer.),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(i32 addrspace(4)* %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(i32 addrspace(4)* %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %tmp.ascast, i32 0, i32 1) ]

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.REDUCTION.UDR:ARRSECT.TYPED"([2 x i32] addrspace(4)* %sum.ascast, i32 0, i64 1, i64 0, i8* null, i8* null, void (i32 addrspace(4)*, i32 addrspace(4)*)* @.omp_combiner., void (i32 addrspace(4)*, i32 addrspace(4)*)* @.omp_initializer.),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %i.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(i32 addrspace(4)* %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(i32 addrspace(4)* %.omp.ub.ascast, i32 0) ]

  %4 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %4, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %5 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %6 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %5, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4
  %8 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %arrayidx1 = getelementptr inbounds [2 x i32], [2 x i32] addrspace(4)* %sum.ascast, i64 0, i64 0
  %9 = load i32, i32 addrspace(4)* %arrayidx1, align 4
  %add2 = add nsw i32 %9, %8
  store i32 %add2, i32 addrspace(4)* %arrayidx1, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add3 = add nsw i32 %10, 1
  store i32 %add3, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p4i8.i64(i8 addrspace(4)* nocapture writeonly, i8, i64, i1 immarg) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: convergent noinline nounwind
define internal void @.omp_combiner.(i32 addrspace(4)* noalias noundef %0, i32 addrspace(4)* noalias noundef %1) #3 {
entry:
  %.addr = alloca i32 addrspace(4)*, align 8
  %.addr1 = alloca i32 addrspace(4)*, align 8
  %.addr.ascast = addrspacecast i32 addrspace(4)** %.addr to i32 addrspace(4)* addrspace(4)*
  %.addr1.ascast = addrspacecast i32 addrspace(4)** %.addr1 to i32 addrspace(4)* addrspace(4)*
  store i32 addrspace(4)* %0, i32 addrspace(4)* addrspace(4)* %.addr.ascast, align 8
  store i32 addrspace(4)* %1, i32 addrspace(4)* addrspace(4)* %.addr1.ascast, align 8
  %2 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %.addr1.ascast, align 8
  %3 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %.addr.ascast, align 8
  %4 = load i32, i32 addrspace(4)* %2, align 4
  %5 = load i32, i32 addrspace(4)* %3, align 4
  %add = add nsw i32 %5, %4
  store i32 %add, i32 addrspace(4)* %3, align 4
  ret void
}

; Function Attrs: convergent noinline nounwind
define internal void @.omp_initializer.(i32 addrspace(4)* noalias noundef %0, i32 addrspace(4)* noalias noundef %1) #3 {
entry:
  %.addr = alloca i32 addrspace(4)*, align 8
  %.addr1 = alloca i32 addrspace(4)*, align 8
  %.addr.ascast = addrspacecast i32 addrspace(4)** %.addr to i32 addrspace(4)* addrspace(4)*
  %.addr1.ascast = addrspacecast i32 addrspace(4)** %.addr1 to i32 addrspace(4)* addrspace(4)*
  store i32 addrspace(4)* %0, i32 addrspace(4)* addrspace(4)* %.addr.ascast, align 8
  store i32 addrspace(4)* %1, i32 addrspace(4)* addrspace(4)* %.addr1.ascast, align 8
  %2 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %.addr1.ascast, align 8
  %3 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %.addr.ascast, align 8
  store i32 0, i32 addrspace(4)* %3, align 4
  ret void
}

attributes #0 = { convergent mustprogress noinline norecurse nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #2 = { nounwind }
attributes #3 = { convergent noinline nounwind "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 2050, i32 60967280, !"_Z4main", i32 7, i32 0, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 8, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}


; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-paropt-spirv-target-has-eu-fusion=true -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -switch-to-offload -vpo-paropt-spirv-target-has-eu-fusion=true -S %s | FileCheck %s

; Original code:
; void test() {
; #pragma omp target parallel for
;   for (int i = 0; i < 1000; ++i) {
; #pragma omp critical
;     ;
;   }
; }

; Check that critical loop was generated:
; CHECK: [[SGID:%[a-zA-Z._0-9]+]] = call spir_func i32 @_Z16get_sub_group_idv()
; CHECK: [[SGIDPARITY:%[a-zA-Z._0-9]+]] = trunc i32 [[SGID]] to i1
; CHECK: br i1 [[SGIDPARITY]], label %[[ODDBRANCH:[a-zA-Z._0-9]+]], label %[[EVENBRANCH:[a-zA-Z._0-9]+]]

; CHECK: [[ODDBRANCH]]:
; CHECK: call{{.*}}__kmpc_critical
; CHECK: [[SGS1:%[a-zA-Z._0-9]+]] = call{{.*}}get_sub_group_size
; CHECK: br label %[[HEADERBB1:[a-zA-Z._0-9]+]]
; CHECK: [[HEADERBB1]]:
; CHECK: [[PHI1:%[a-zA-Z._0-9]+]] = phi i32 [ 0, %{{[a-zA-Z._0-9]+}} ], [ [[INC1:%[a-zA-Z._0-9]+]], %[[INCBB1:[a-zA-Z._0-9]+]] ]
; CHECK: [[EXITPRED1:%[a-zA-Z._0-9]+]] = icmp uge i32 [[PHI1]], [[SGS1]]
; CHECK: br i1 [[EXITPRED1]], label %[[JUMPTOEXITBB1:[a-zA-Z._0-9]+]], label %[[CHECKBB1:[a-zA-Z._0-9]+]]
; CHECK: [[JUMPTOEXITBB1]]:
; CHECK: br label %[[EXITBB1:[a-zA-Z._0-9]+]]
; CHECK: [[CHECKBB1]]:
; CHECK: [[SGLI1:%[a-zA-Z._0-9]+]] = call{{.*}}get_sub_group_local_id
; CHECK: [[SKIPPRED1:%[a-zA-Z._0-9]+]] = icmp ne i32 [[PHI1]], [[SGLI1]]
; CHECK: br i1 [[SKIPPRED1]], label %[[JUMPTOINCBB1:[a-zA-Z._0-9]+]],
; CHECK: [[JUMPTOINCBB1]]:
; CHECK: br label %[[INCBB1]]
; CHECK: [[INCBB1]]:
; CHECK: [[INC1]] = add{{.*}}[[PHI1]], 1
; CHECK: br label %[[HEADERBB1]]
; CHECK: [[EXITBB1]]:
; CHECK: call{{.*}}__kmpc_end_critical
; CHECK: br label %[[MERGEBB:[a-zA-Z._0-9]+]]

; CHECK: [[EVENBRANCH]]:
; CHECK: call{{.*}}__kmpc_critical
; CHECK: [[SGS2:%[a-zA-Z._0-9]+]] = call{{.*}}get_sub_group_size
; CHECK: br label %[[HEADERBB2:[a-zA-Z._0-9]+]]
; CHECK: [[HEADERBB2]]:
; CHECK: [[PHI2:%[a-zA-Z._0-9]+]] = phi i32 [ 0, %{{[a-zA-Z._0-9]+}} ], [ [[INC2:%[a-zA-Z._0-9]+]], %[[INCBB2:[a-zA-Z._0-9]+]] ]
; CHECK: [[EXITPRED2:%[a-zA-Z._0-9]+]] = icmp uge i32 [[PHI2]], [[SGS2]]
; CHECK: br i1 [[EXITPRED2]], label %[[JUMPTOEXITBB2:[a-zA-Z._0-9]+]], label %[[CHECKBB2:[a-zA-Z._0-9]+]]
; CHECK: [[JUMPTOEXITBB2]]:
; CHECK: br label %[[EXITBB2:[a-zA-Z._0-9]+]]
; CHECK: [[CHECKBB2]]:
; CHECK: [[SGLI2:%[a-zA-Z._0-9]+]] = call{{.*}}get_sub_group_local_id
; CHECK: [[SKIPPRED2:%[a-zA-Z._0-9]+]] = icmp ne i32 [[PHI2]], [[SGLI2]]
; CHECK: br i1 [[SKIPPRED2]], label %[[JUMPTOINCBB2:[a-zA-Z._0-9]+]],
; CHECK: [[JUMPTOINCBB2]]:
; CHECK: br label %[[INCBB2]]
; CHECK: [[INCBB2]]:
; CHECK: [[INC2]] = add{{.*}}[[PHI2]], 1
; CHECK: br label %[[HEADERBB2]]
; CHECK: [[EXITBB2]]:
; CHECK: call{{.*}}__kmpc_end_critical
; CHECK: br label %[[MERGEBB]]

; CHECK: [[MERGEBB]]:

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define hidden spir_func void @test() #0 {
entry:
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 999, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast) ]
  %2 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %2, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %4 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.CRITICAL"() ]
  fence acquire
  fence release
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.CRITICAL"() ]
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add1 = add nsw i32 %7, 1
  store i32 %add1, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2052, i32 55999408, !"test", i32 2, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 10.0.0"}

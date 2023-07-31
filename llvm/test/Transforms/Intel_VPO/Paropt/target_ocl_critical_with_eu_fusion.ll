; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-paropt-spirv-target-has-eu-fusion=true -S %s | FileCheck %s
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

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind optnone
define protected spir_func void @test() #0 {
entry:
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 999, ptr addrspace(4) %.omp.ub.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1) ]

  %2 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %2, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %4 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.CRITICAL"() ]

  fence acquire
  fence release
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.CRITICAL"() ]

  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add1 = add nsw i32 %7, 1
  store i32 %add1, ptr addrspace(4) %.omp.iv.ascast, align 4
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

attributes #0 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 66312, i32 199832001, !"_Z4test", i32 2, i32 0, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 8, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}

; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-ocl-loop-processing=non-strided-lws -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-ocl-loop-processing=non-strided-lws -switch-to-offload -S %s | FileCheck %s

; Original code:
; void foo() {
; #pragma omp target
;   {
;     int i;
; #pragma omp parallel for
;     for (i = 0; i < 100; ++i);
;   }
; }

; This test checks parallel loop scheduling code for the case of non-strided processing with LWS increment

; CHECK-LABEL: DIR.OMP.PARALLEL.LOOP{{.*}}:
; CHECK-DAG: [[LSize:%[^,]+]] = call spir_func i64 @_Z14get_local_sizej(i32 0)
; CHECK-DAG: [[LId:%[^,]+]] = call spir_func i64 @_Z12get_local_idj(i32 0)
; CHECK: [[LIdTrunc:%[^,]+]] = trunc i64 [[LId]] to i32
; CHECK: [[NewLB:%[^,]+]] = add i32 %{{[^,]+}}, [[LIdTrunc]]
; CHECK: store i32 [[NewLB]], ptr [[LBVar:%[^,]+]]
; CHECK: [[LB:%[^,]+]] = load i32, ptr [[LBVar]]
; CHECK: [[ZTT:%[^,]+]] = icmp sle i32 [[LB]],
; CHECK: br i1 [[ZTT]], label %[[LoopBody:[^,]+]], label %
; CHECK: [[LoopBody]]:
; CHECK: [[IVPhi:%[^,]+]] = phi i32 [ [[Inc:%[^,]+]], %[[IncBlock:[^,]+]] ], [ [[LB]],
; CHECK: [[LSizeTrunc:%[^,]+]] = trunc i64 [[LSize]] to i32
; CHECK: [[Inc]] = add{{.*}}i32 [[IVPhi]], [[LSizeTrunc]]

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent mustprogress noinline nounwind optnone
define protected spir_func void @_Z3foov() #0 {
entry:
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 99, ptr addrspace(4) %.omp.ub.ascast, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0) ]

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
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %6 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add1 = add nsw i32 %6, 1
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
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

attributes #0 = { "contains-openmp-target"="true" }

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2050, i32 49102686, !"_Z3foov", i32 2, i32 0, i32 0, i32 0}

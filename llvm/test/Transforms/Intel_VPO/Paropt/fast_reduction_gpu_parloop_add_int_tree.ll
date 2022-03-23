; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt --vpo-paropt-atomic-free-reduction-ctrl=1 -vpo-paropt-atomic-free-red-local-buf-size=1024 -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' --vpo-paropt-atomic-free-reduction-ctrl=1 -vpo-paropt-atomic-free-red-local-buf-size=1024 -S %s | FileCheck %s


;
; int main(void)
; {
;   int i, sum = 0;
;
; #pragma omp target parallel for reduction(+:sum) map(tofrom:sum)
;   for (i=0; i<10; i++) {
;     sum+=i;
;   }
;
;   return 0;
; }


; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; CHECK-LABEL: DIR.OMP.END.PARALLEL.LOOP.{{[0-9]+}}
; CHECK: %[[LOCAL_SIZE:[^,]+]] = call spir_func i64 @_Z14get_local_sizej(i32 0)
; CHECK: %[[LOCAL_ID:[^,]+]] = call spir_func i64 @_Z12get_local_idj(i32 0)
; CHECK: lshr i64 %[[LOCAL_SIZE]], 1
; CHECK: sub i64 %{{[0-9]+}}, 1
; CHECK: lshr i64 %{{[0-9]+}}, 1
; CHECK: or i64
; CHECK: lshr i64 %{{[0-9]+}}, 2
; CHECK: or i64
; CHECK: lshr i64 %{{[0-9]+}}, 4
; CHECK: or i64
; CHECK: lshr i64 %{{[0-9]+}}, 8
; CHECK: or i64
; CHECK: lshr i64 %{{[0-9]+}}, 16
; CHECK: or i64
; CHECK: lshr i64 %{{[0-9]+}}, 32
; CHECK: or i64
; CHECK-NEXT: %[[TREE_LOOP_UB:[^,]+]] = add i64 %{{[0-9]+}}, 1
; CHECK: %[[LOCAL_BUF_PTR:[^,]+]] = getelementptr inbounds [1024 x i32], [1024 x i32] addrspace(3)* @red_local_buf, i32 0, i64 %[[LOCAL_ID]]
; CHECK: %[[PRIV_VAL:[^,]+]] = load i32, i32* %sum.ascast.red, align 4
; CHECK: store i32 %[[PRIV_VAL]], i32 addrspace(3)* %[[LOCAL_BUF_PTR]], align 4
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 272)
; CHECK-LABEL: atomic.free.red.local.update.update.header:
; CHECK: %[[IDX_PHI:[^,]+]] = phi i64 [ %{{[0-9]+}}, %{{[a-z.]+}} ], [ %[[TREE_LOOP_UB]]
; CHECK: %[[CMP0:[^,]+]] = icmp eq i64 %[[IDX_PHI]], 0
; CHECK: br i1 %[[CMP0]], label %atomic.free.red.local.update.update.exit, label %atomic.free.red.local.update.update.idcheck
; CHECK-LABEL: atomic.free.red.local.update.update.idcheck:
; CHECK: %[[CMP1:[^,]+]] = icmp ult i64 %[[LOCAL_ID]], %[[IDX_PHI]]
; CHECK: %[[RHS_IDX:[^,]+]] = add i64 %[[LOCAL_ID]], %[[IDX_PHI]]
; CHECK: %[[CMP2:[^,]+]] = icmp ult i64 %[[RHS_IDX]], %[[LOCAL_SIZE]]
; CHECK: %[[UPD_COND:[^,]+]] = select i1 %[[CMP1]], i1 %[[CMP2]], i1 false
; CHECK: br i1 %[[UPD_COND]], label %atomic.free.red.local.update.update.body, label %atomic.free.red.local.update.update.latch
; CHECK-LABEL: atomic.free.red.local.update.update.body:
; CHECK: %[[LOCAL_BUF_PTR_PLUS:[^,]+]] = getelementptr inbounds i32, i32 addrspace(3)* %[[LOCAL_BUF_PTR]], i64 %[[IDX_PHI]]
; CHECK: %[[LOCAL_SUM_OLD:[^,]+]] = load i32, i32 addrspace(3)* %[[LOCAL_BUF_PTR_PLUS]]
; CHECK: %[[LOCAL_SUM_NEW:[^,]+]] = load volatile i32, i32 addrspace(3)* %[[LOCAL_BUF_PTR]]
; CHECK: %[[RED_VALUE:[^,]+]] = add i32 %[[LOCAL_SUM_NEW]], %[[LOCAL_SUM_OLD]]
; CHECK: store i32 %[[RED_VALUE]], i32 addrspace(3)* %[[LOCAL_BUF_PTR]]
; CHECK: br label %atomic.free.red.local.update.update.latch
; CHECK-LABEL: atomic.free.red.local.update.update.latch:
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
; CHECK: lshr i64 %[[IDX_PHI]], 1
; CHECK: br label %atomic.free.red.local.update.update.header
; CHECK-LABEL: atomic.free.red.local.update.update.exit:
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
; CHECK: %[[MTT:[^,]+]] = icmp ne i64 %[[LOCAL_ID]], 0
; CHECK: br i1 %[[MTT]]
; CHECK: %[[LOCAL_VAL:[^,]+]] = load i32, i32 addrspace(3)* %[[LOCAL_BUF_PTR]]
; CHECK: %[[CUR_GLOBAL_VAL:[^,]+]] = load i32, i32 addrspace(1)* %[[GLOBAL_PTR:[^,]+]], align 4
; CHECK: %[[NEW_GLOBAL_VAL:[^,]+]] = add i32 %[[CUR_GLOBAL_VAL]], %[[LOCAL_VAL]]
; CHECK: store i32 %[[NEW_GLOBAL_VAL]], i32 addrspace(1)* %[[GLOBAL_PTR]]
; CHECK-NOT: call spir_func void @__kmpc_atomic_{{.*}}_add(

; Function Attrs: convergent noinline nounwind
define hidden i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %sum = alloca i32, align 4
  %sum.ascast = addrspacecast i32* %sum to i32 addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %retval.ascast, align 4
  store i32 0, i32 addrspace(4)* %sum.ascast, align 4, !tbaa !8
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4, !tbaa !8
  store i32 9, i32 addrspace(4)* %.omp.ub.ascast, align 4, !tbaa !8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %sum.ascast, i32 addrspace(4)* %sum.ascast, i64 4, i64 35, i8* null, i8* null), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD"(i32 addrspace(4)* %sum.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast) ]
  %2 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4, !tbaa !8
  store i32 %2, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !8
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !8
  %4 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4, !tbaa !8
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !8
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4, !tbaa !8
  %6 = load i32, i32 addrspace(4)* %i.ascast, align 4, !tbaa !8
  %7 = load i32, i32 addrspace(4)* %sum.ascast, align 4, !tbaa !8
  %add1 = add nsw i32 %7, %6
  store i32 %add1, i32 addrspace(4)* %sum.ascast, align 4, !tbaa !8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !8
  %add2 = add nsw i32 %8, 1
  store i32 %add2, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !8
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #1

attributes #0 = { convergent noinline nounwind "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!6}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}

!0 = !{i32 0, i32 66311, i32 42218157, !"_Z4main", i32 5, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!7 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}


; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-reduction-ctrl=3 -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-reduction-ctrl=3 -S %s | FileCheck %s


;
; int main(void)
; {
;   int i;
;   int sum[1] = {0};
;
; #pragma omp target teams distribute parallel for reduction(+:sum[0])
;   for (i=0; i<10; i++) {
;     sum[0]+=i;
;   }
;
;   return 0;
; }


; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; CHECK-LABEL: omp.loop.exit:
; CHECK: %[[LOCAL_OFFSET:[^,]+]] = mul i64 %[[LOCAL_ID:[^,]+]], 1
; CHECK: %[[LOCAL_BUF_BASE:[^,]+]] = getelementptr [1024 x i32], [1024 x i32] addrspace(3)* @[[LOCAL_BUF:[^,]+]], i32 0, i64 %[[LOCAL_OFFSET]]
; CHECK-LABEL: red.update.body.to.tree:
; CHECK: %[[DST_PTR_TO:[^,]+]] = phi i32 addrspace(3)* [ %[[LOCAL_BUF_BASE]]
; CHECK: %[[SRC_PTR_TO:[^,]+]] = phi i32*
; CHECK: %[[PRIV_VAL:[^,]+]] = load i32, i32* %[[SRC_PTR_TO]]
; CHECK: store i32 %[[PRIV_VAL]], i32 addrspace(3)* %[[DST_PTR_TO]]
; CHECK-LABEL: red.update.done.to.tree:
; CHECK-COUNT-7: lshr
; CHECK: add
; CHECK-LABEL: atomic.free.red.local.update.update.header:
; CHECK: %[[IDX_PHI:[^,]+]] = phi i64
; CHECK-LABEL: atomic.free.red.local.update.update.idcheck:
; CHECK-LABEL: atomic.free.red.local.update.update.body:
; CHECK: %[[DST_PTR:[^,]+]] = phi i32 addrspace(3)* [ %[[LOCAL_BUF_BASE]]
; CHECK: %[[SRC_PTR:[^,]+]] = phi i32 addrspace(3)* [ %[[LOCAL_BUF_BASE]]
; CHECK: %[[SEC_SZ_OFF:[^,]+]] = mul i64 1, %[[IDX_PHI]]
; CHECK: %[[SRC_PTR_PLUS:[^,]+]] = getelementptr inbounds i32, i32 addrspace(3)* %[[SRC_PTR]], i64 %[[SEC_SZ_OFF]]
; CHECK: %[[RHS1:[^,]+]] = load i32, i32 addrspace(3)* %[[SRC_PTR_PLUS]]
; CHECK: %[[RHS2:[^,]+]] = load volatile i32, i32 addrspace(3)* %[[DST_PTR]]
; CHECK: %[[RED_OP:[^,]+]] = add i32 %[[RHS2]], %[[RHS1]]
; CHECK: store i32 %[[RED_OP]], i32 addrspace(3)* %[[DST_PTR]]
; CHECK: br i1 %{{[0-9a-z.]+}}, label %atomic.free.red.local.update.update.latch, label %atomic.free.red.local.update.update.body
; CHECK-LABEL: atomic.free.red.local.update.update.latch:
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK: lshr
; CHECK: br label %atomic.free.red.local.update.update.header
; CHECK-LABEL: red.update.body.from.tree:
; CHECK: %[[DST_PTR_FROM:[^,]+]] = phi i32 addrspace(1)*
; CHECK: %[[SRC_PTR_FROM:[^,]+]] = phi i32 addrspace(3)* [ %[[LOCAL_BUF_BASE]]
; CHECK: %[[DST_VAL:[^,]+]] = load i32, i32 addrspace(1)*  %[[DST_PTR_FROM]]
; CHECK: %[[SRC_VAL:[^,]+]] = load i32, i32 addrspace(3)*  %[[SRC_PTR_FROM]]
; CHECK: %[[NEW_VAL:[^,]+]] = add i32 %[[DST_VAL]], %[[SRC_VAL]]
; CHECK: br i1
; CHECK: store i32 %[[NEW_VAL]], i32 addrspace(1)* %[[DST_PTR_FROM]]
; CHECK-NEXT: br label
; CHECK-LABEL: red.update.done.from.tree:
; CHECK-LABEL: counter_check:
; CHECK-LABEL: atomic.free.red.global.update.header:
; CHECK: %[[IDX_PHI_GLOBAL:[^,]+]] = phi i64
; CHECK: %[[GLOBAL_OFFSET:[^,]+]] = mul i64 %[[IDX_PHI_GLOBAL]], 1
; CHECK: %[[GLOBAL_BUF_BASE:[^,]+]] = getelementptr [1 x i32], [1 x i32] addrspace(1)* %[[GLOBAL_BUF:[^,]+]], i64 %[[GLOBAL_OFFSET]]
; CHECK: %[[GLOBAL_BUF_BC:[^,]+]] = bitcast [1 x i32] addrspace(1)* %[[GLOBAL_BUF_BASE]] to i32 addrspace(1)*
; CHECK-LABEL: atomic.free.red.global.update.body:
; CHECK-LABEL: red.update.body:
; CHECK: %[[DST_PTR_GLOBAL:[^,]+]] = phi i32 addrspace(1)*
; CHECK: %[[SRC_PTR_GLOBAL:[^,]+]] = phi i32 addrspace(1)* [ %[[GLOBAL_BUF_BC]]
; CHECK: %[[SRC_VAL_GLOBAL:[^,]+]] = load i32, i32 addrspace(1)*  %[[SRC_PTR_GLOBAL]]
; CHECK: %[[DST_VAL_GLOBAL:[^,]+]] = load i32, i32 addrspace(1)*  %[[DST_PTR_GLOBAL]]
; CHECK: %[[NEW_VAL_GLOBAL:[^,]+]] = add i32 %[[DST_VAL_GLOBAL]], %[[SRC_VAL_GLOBAL]]
; CHECK: store i32 %[[NEW_VAL_GLOBAL]], i32 addrspace(1)* %[[DST_PTR_GLOBAL]]
; CHECK: br i1 %red.cpy.done73, label %item.exit, label %red.update.body
; CHECK-NOT: store

; Function Attrs: convergent noinline nounwind optnone
define hidden i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %sum = alloca [1 x i32], align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %sum.ascast = addrspacecast [1 x i32]* %sum to [1 x i32] addrspace(4)*
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %retval.ascast, align 4
  %0 = bitcast [1 x i32] addrspace(4)* %sum.ascast to i8 addrspace(4)*
  call void @llvm.memset.p4i8.i64(i8 addrspace(4)* align 4 %0, i8 0, i64 4, i1 false)
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 9, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %arrayidx = getelementptr inbounds [1 x i32], [1 x i32] addrspace(4)* %sum.ascast, i64 0, i64 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"([1 x i32] addrspace(4)* %sum.ascast, i32 addrspace(4)* %arrayidx, i64 4, i64 547, i8* null, i8* null), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.REDUCTION.ADD:ARRSECT"([1 x i32] addrspace(4)* %sum.ascast, i64 1, i64 0, i64 1, i64 1), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.REDUCTION.ADD:ARRSECT"([1 x i32] addrspace(4)* %sum.ascast, i64 1, i64 0, i64 1, i64 1), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast) ]
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
  %arrayidx1 = getelementptr inbounds [1 x i32], [1 x i32] addrspace(4)* %sum.ascast, i64 0, i64 0
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

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p4i8.i64(i8 addrspace(4)* nocapture writeonly, i8, i64, i1 immarg) #1

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

!0 = !{i32 0, i32 66311, i32 42218157, !"_Z4main", i32 5, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}


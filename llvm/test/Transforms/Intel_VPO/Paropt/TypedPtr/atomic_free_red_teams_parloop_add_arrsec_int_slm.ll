; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-reduction-ctrl=3 -vpo-paropt-atomic-free-reduction-slm=true -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-reduction-ctrl=3 -vpo-paropt-atomic-free-reduction-slm=true -S %s | FileCheck %s

; Test src:
;
; int main(void)
; {
;   int i;
;   int sum[2] = {0, 0};
;
; #pragma omp target teams distribute parallel for reduction(+:sum[1])
;   for (i=0; i<10; i++) {
;     sum[1]+=i;
;   }
;
;   return 0;
; }

; This test checks atomic-free reduction codegen for non-zero offset array section item
; with SLM intra-team reduction buffer

; CHECK-LABEL: omp.loop.exit:
; CHECK: %[[LOCAL_ID:[^,]+]] = call spir_func i64 @_Z12get_local_idj(i32 0)
; CHECK: %[[LOCAL_BUF_BASE:[^,]+]] = getelementptr inbounds [1024 x [1 x i32]], [1024 x [1 x i32]] addrspace(3)* @[[LOCAL_BUF:[^,]+]], i32 0, i64 %[[LOCAL_ID]], i32 0
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
; CHECK: %[[SRC_VAL:[^,]+]] = load i32, i32 addrspace(3)*  %[[SRC_PTR_FROM]]
; CHECK: %[[DST_VAL:[^,]+]] = load i32, i32 addrspace(1)*  %[[DST_PTR_FROM]]
; CHECK: %[[NEW_VAL:[^,]+]] = add i32 %[[DST_VAL]], %[[SRC_VAL]]
; CHECK: br i1
; CHECK: store i32 %[[NEW_VAL]], i32 addrspace(1)* %[[DST_PTR_FROM]]
; CHECK-NEXT: br label
; CHECK-LABEL: red.update.done.from.tree:
; CHECK-LABEL: counter_check:
; CHECK-LABEL: atomic.free.red.global.update.header:
; CHECK: %[[IDX_PHI_GLOBAL:[^,]+]] = phi i64
; CHECK: %[[GLOBAL_BUF_BASE:[^,]+]] = getelementptr [1 x i32], [1 x i32] addrspace(1)* %[[GLOBAL_BUF:[^,]+]], i64 %[[IDX_PHI_GLOBAL]]
; CHECK: %[[GLOBAL_BUF_BC:[^,]+]] = bitcast [1 x i32] addrspace(1)* %[[GLOBAL_BUF_BASE]] to i32 addrspace(1)*
; CHECK-LABEL: atomic.free.red.global.update.body:
; CHECK-LABEL: red.update.body:
; CHECK: %[[DST_PTR_GLOBAL:[^,]+]] = phi i32 addrspace(1)*
; CHECK: %[[SRC_PTR_GLOBAL:[^,]+]] = phi i32 addrspace(1)* [ %[[GLOBAL_BUF_BC]]
; CHECK: %[[SRC_VAL_GLOBAL:[^,]+]] = load i32, i32 addrspace(1)*  %[[SRC_PTR_GLOBAL]]
; CHECK: %[[DST_VAL_GLOBAL:[^,]+]] = load i32, i32 addrspace(1)*  %[[DST_PTR_GLOBAL]]
; CHECK: %[[NEW_VAL_GLOBAL:[^,]+]] = add i32 %[[DST_VAL_GLOBAL]], %[[SRC_VAL_GLOBAL]]
; CHECK: store i32 %[[NEW_VAL_GLOBAL]], i32 addrspace(1)* %[[DST_PTR_GLOBAL]]
; CHECK: br i1 %red.cpy.done{{.*}}, label %item.exit, label %red.update.body
; CHECK-NOT: store

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind optnone
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
  %arrayidx = getelementptr inbounds [2 x i32], [2 x i32] addrspace(4)* %sum.ascast, i64 0, i64 1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"([2 x i32] addrspace(4)* %sum.ascast, i32 addrspace(4)* %arrayidx, i64 4, i64 33315, i8* null, i8* null), ; MAP type: 33315 = 0x8223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1) | UNKNOWN (0x8000)
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %tmp.ascast, i32 0, i32 1) ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"([2 x i32] addrspace(4)* %sum.ascast, i32 0, i64 1, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(i32 addrspace(4)* %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(i32 addrspace(4)* %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %tmp.ascast, i32 0, i32 1) ]

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"([2 x i32] addrspace(4)* %sum.ascast, i32 0, i64 1, i64 1),
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
  %arrayidx1 = getelementptr inbounds [2 x i32], [2 x i32] addrspace(4)* %sum.ascast, i64 0, i64 1
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
declare void @llvm.memset.p4i8.i64(i8 addrspace(4)* nocapture writeonly, i8, i64, i1 immarg)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0)

attributes #0 = { "contains-openmp-target"="true" }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 66311, i32 42218157, !"_Z4main", i32 5, i32 0, i32 0}


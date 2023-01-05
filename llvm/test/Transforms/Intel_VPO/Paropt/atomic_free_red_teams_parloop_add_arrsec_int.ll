; RUN: opt -enable-new-pm=0 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-reduction-ctrl=3 -vpo-paropt-atomic-free-reduction-slm=true -vpo-paropt-atomic-free-red-use-fp-team-counter=false  -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-reduction-ctrl=3 -vpo-paropt-atomic-free-reduction-slm=true -vpo-paropt-atomic-free-red-use-fp-team-counter=false  -S %s | FileCheck %s

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

; The test IR was hand-modified to use a constant section offset for
; reduction. CFE currently generates IR instructions to compute it.

; CHECK-LABEL: omp.loop.exit:
; CHECK: %[[LOCAL_OFFSET:[^,]+]] = mul i64 %[[LOCAL_ID:[^,]+]], 1
; CHECK: %[[LOCAL_BUF_BASE:[^,]+]] = getelementptr [1024 x i32], ptr addrspace(3) @[[LOCAL_BUF:[^,]+]], i32 0, i64 %[[LOCAL_OFFSET]]
; CHECK-LABEL: red.update.body.to.tree:
; CHECK: %[[DST_PTR_TO:[^,]+]] = phi ptr addrspace(3) [ %[[LOCAL_BUF_BASE]]
; CHECK: %[[SRC_PTR_TO:[^,]+]] = phi ptr
; CHECK: %[[PRIV_VAL:[^,]+]] = load i32, ptr %[[SRC_PTR_TO]]
; CHECK: store i32 %[[PRIV_VAL]], ptr addrspace(3) %[[DST_PTR_TO]]
; CHECK-LABEL: red.update.done.to.tree:
; CHECK-COUNT-7: lshr
; CHECK: add
; CHECK-LABEL: atomic.free.red.local.update.update.header:
; CHECK: %[[IDX_PHI:[^,]+]] = phi i64
; CHECK-LABEL: atomic.free.red.local.update.update.idcheck:
; CHECK-LABEL: atomic.free.red.local.update.update.body:
; CHECK: %[[DST_PTR:[^,]+]] = phi ptr addrspace(3) [ %[[LOCAL_BUF_BASE]]
; CHECK: %[[SRC_PTR:[^,]+]] = phi ptr addrspace(3) [ %[[LOCAL_BUF_BASE]]
; CHECK: %[[SEC_SZ_OFF:[^,]+]] = mul i64 1, %[[IDX_PHI]]
; CHECK: %[[SRC_PTR_PLUS:[^,]+]] = getelementptr inbounds i32, ptr addrspace(3) %[[SRC_PTR]], i64 %[[SEC_SZ_OFF]]
; CHECK: %[[RHS1:[^,]+]] = load i32, ptr addrspace(3) %[[SRC_PTR_PLUS]]
; CHECK: %[[RHS2:[^,]+]] = load volatile i32, ptr addrspace(3) %[[DST_PTR]]
; CHECK: %[[RED_OP:[^,]+]] = add i32 %[[RHS2]], %[[RHS1]]
; CHECK: store i32 %[[RED_OP]], ptr addrspace(3) %[[DST_PTR]]
; CHECK: br i1 %{{[0-9a-z.]+}}, label %atomic.free.red.local.update.update.latch, label %atomic.free.red.local.update.update.body
; CHECK-LABEL: atomic.free.red.local.update.update.latch:
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK: lshr
; CHECK: br label %atomic.free.red.local.update.update.header
; CHECK-LABEL: red.update.body.from.tree:
; CHECK: %[[DST_PTR_FROM:[^,]+]] = phi ptr addrspace(1)
; CHECK: %[[SRC_PTR_FROM:[^,]+]] = phi ptr addrspace(3) [ %[[LOCAL_BUF_BASE]]
; CHECK: %[[DST_VAL:[^,]+]] = load i32, ptr addrspace(1)  %[[DST_PTR_FROM]]
; CHECK: %[[SRC_VAL:[^,]+]] = load i32, ptr addrspace(3)  %[[SRC_PTR_FROM]]
; CHECK: %[[NEW_VAL:[^,]+]] = add i32 %[[DST_VAL]], %[[SRC_VAL]]
; CHECK: br i1
; CHECK: store i32 %[[NEW_VAL]], ptr addrspace(1) %[[DST_PTR_FROM]]
; CHECK-NEXT: br label
; CHECK-LABEL: red.update.done.from.tree:
; CHECK-LABEL: counter_check:
; CHECK-LABEL: atomic.free.red.global.update.header:
; CHECK: %[[IDX_PHI_GLOBAL:[^,]+]] = phi i64
; CHECK: %[[NUM_TEAMS_0:.*]] = call spir_func i64 @_Z14get_num_groupsj(i32 0)
; CHECK: %[[GLOBAL_UPDATE_DONE:.*]] = icmp uge i64 %{{.*}}, %[[NUM_TEAMS_0]]
; CHECK: %[[GLOBAL_OFFSET:[^,]+]] = mul i64 %[[IDX_PHI_GLOBAL]], 1
; CHECK: %[[GLOBAL_BUF_BASE:[^,]+]] = getelementptr [1 x i32], ptr addrspace(1) %[[GLOBAL_BUF:[^,]+]], i64 %[[GLOBAL_OFFSET]]
; CHECK: br i1 %[[GLOBAL_UPDATE_DONE]], label %counter.reset, label %atomic.free.red.global.update.body
; CHECK-LABEL: counter.reset:
; CHECK: store i32 0, ptr addrspace(1) %teams_counter, align 4
; CHECK: br label %red.update.done
; CHECK-LABEL: atomic.free.red.global.update.body:
; CHECK-LABEL: red.update.body:
; CHECK: %[[DST_PTR_GLOBAL:[^,]+]] = phi ptr addrspace(1)
; CHECK: %[[SRC_PTR_GLOBAL:[^,]+]] = phi ptr addrspace(1) [ %[[GLOBAL_BUF_BASE]]
; CHECK: %[[SRC_VAL_GLOBAL:[^,]+]] = load i32, ptr addrspace(1)  %[[SRC_PTR_GLOBAL]]
; CHECK: %[[DST_VAL_GLOBAL:[^,]+]] = load i32, ptr addrspace(1)  %[[DST_PTR_GLOBAL]]
; CHECK: %[[NEW_VAL_GLOBAL:[^,]+]] = add i32 %[[DST_VAL_GLOBAL]], %[[SRC_VAL_GLOBAL]]
; CHECK: store i32 %[[NEW_VAL_GLOBAL]], ptr addrspace(1) %[[DST_PTR_GLOBAL]]
; CHECK: br i1 %red.cpy.done{{.*}}, label %item.exit, label %red.update.body
; CHECK-NOT: store

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind
define hidden i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %sum = alloca [2 x i32], align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %sum.ascast = addrspacecast ptr %sum to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  call void @llvm.memset.p4.i64(ptr addrspace(4) align 4 %sum.ascast, i8 0, i64 8, i1 false)
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 9, ptr addrspace(4) %.omp.ub.ascast, align 4
  %arrayidx = getelementptr inbounds [2 x i32], ptr addrspace(4) %sum.ascast, i64 0, i64 1
  %i2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum.ascast, ptr addrspace(4) %arrayidx, i64 4, i64 547, ptr null, ptr null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  %i3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"(ptr addrspace(4) %sum.ascast, i32 0, i64 1, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  %i4 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"(ptr addrspace(4) %sum.ascast, i32 0, i64 1, i64 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0) ]

  %i5 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %i5, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %i6 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %i7 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %i6, %i7
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %i8 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %i8, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  %i9 = load i32, ptr addrspace(4) %i.ascast, align 4
  %arrayidx1 = getelementptr inbounds [2 x i32], ptr addrspace(4) %sum.ascast, i64 0, i64 1
  %i10 = load i32, ptr addrspace(4) %arrayidx1, align 4
  %add2 = add nsw i32 %i10, %i9
  store i32 %add2, ptr addrspace(4) %arrayidx1, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %i11 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add3 = add nsw i32 %i11, 1
  store i32 %add3, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %i4) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  call void @llvm.directive.region.exit(token %i3) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %i2) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

; Function Attrs: argmemonly nocallback nofree nounwind willreturn writeonly
declare void @llvm.memset.p4.i64(ptr addrspace(4) nocapture writeonly, i8, i64, i1 immarg) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { convergent mustprogress noinline norecurse nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nocallback nofree nounwind willreturn writeonly }
attributes #2 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 66313, i32 186593318, !"_Z4main", i32 6, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}

; RUN: opt -enable-new-pm=0 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-reduction-par-global=true -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-reduction-par-global=true -S %s | FileCheck %s

; Test src:
;
; int main(void) {
;   int i, sum = 0;
;
; #pragma omp target teams distribute parallel for reduction(+:sum) map(tofrom:sum)
;   for (i = 0; i < 10; i++) {
;     sum += i;
;   }
;
;   return 0;
; }

; CHECK: define weak dso_local spir_kernel void @__omp_offloading{{.*}}main{{.*}}(i32 addrspace(1)* noalias %[[RESULT_PTR:sum.*]], i32 addrspace(1)* %red_local_buf, i32 addrspace(1)* %[[RED_GLOBAL_BUF:red_buf.*]], i32 addrspace(1)* %[[TEAMS_COUNTER_PTR:teams_counter.*]], i64 %.omp.lb

; CHECK-LABEL: counter_check:
; CHECK: %[[NUM_GROUPS:[^,]+]] = call spir_func i64 @_Z14get_num_groupsj(i32 0)
; CHECK: %[[TEAMS_COUNTER:[^,]+]] = addrspacecast i32 addrspace(1)* %[[TEAMS_COUNTER_PTR]] to i32 addrspace(4)*
; CHECK-LABEL: master.thread.code{{[0-9]+}}:
; CHECK: %[[UPD_CNTR:[^,]+]] = call spir_func i32 @__kmpc_atomic_fixed4_add_cpt(i32 addrspace(4)* %[[TEAMS_COUNTER]], i32 1, i32 1)
; CHECK: store i32 %[[UPD_CNTR]], i32 addrspace(3)* @.broadcast.ptr.__local, align 4
; CHECK-LABEL: master.thread.fallthru{{[0-9]+}}:
; CHECK: %.new = load i32, i32 addrspace(3)* @.broadcast.ptr.__local, align 4
; CHECK: %[[NUM_GROUPS_TRUNC:[^,]+]] = trunc i64 %[[NUM_GROUPS]] to i32
; CHECK: %[[CNTR_CHECK:[^,]+]] = icmp ne i32 %.new, %[[NUM_GROUPS_TRUNC]]
; CHECK: %[[LOCAL_ID:[^,]+]] = call spir_func i64 @_Z12get_local_idj(i32 0)
; CHECK: br i1 %[[CNTR_CHECK]], label [[EXIT_BB:[^,]+]], label %atomic.free.red.global.update.header
; CHECK-LABEL: atomic.free.red.global.update.header:
; CHECK: %[[IDX_PHI:[^,]+]] = phi i64
; CHECK: %[[NUM_GROUPS1:[^,]+]] = call spir_func i64 @_Z14get_num_groupsj(i32 0)
; CHECK: %[[EXIT_COND:[^,]+]] = icmp uge i64 %[[IDX_PHI]], %[[NUM_GROUPS1]]
; CHECK: %[[LOCAL_ID0:[^,]+]] = call spir_func i64 @_Z12get_local_idj(i32 0)
; CHECK: %[[GLOBAL_GEP:[^,]+]] = getelementptr i32, i32 addrspace(1)* %[[RED_GLOBAL_BUF]], i64 %[[LOCAL_ID0]]
; CHECK: %[[GLOBAL_GEP_OFF:[^,]+]] = getelementptr i32, i32 addrspace(1)* %[[GLOBAL_GEP]], i64 %[[IDX_PHI]]
; CHECK: br i1 %[[EXIT_COND]], label %atomic.free.red.global.update.store, label %atomic.free.red.global.update.tree.header

; Computing ((thread_id & (tree_offset*2-1)) || thread_id + tree_offset >= num_teams)
; CHECK-LABEL: atomic.free.red.global.update.tree.header:
; CHECK: %[[OFF_2:[^,]+]] = shl i64 %[[IDX_PHI]], 1
; CHECK: %[[OFF_2_1:[^,]+]] = sub i64 %[[OFF_2]], 1
; CHECK: %[[ID_AND_OFF:[^,]+]] = and i64 %[[LOCAL_ID]], %[[OFF_2_1]]
; CHECK: %[[ID_TST:[^,]+]] = icmp ne i64 %[[ID_AND_OFF]], 0
; CHECK: %[[ID_OFF:[^,]+]] = add i64 %[[LOCAL_ID]], %[[IDX_PHI]]
; CHECK: %[[ID_OFF_TEST:[^,]+]] = icmp uge i64 %[[ID_OFF]], %[[NUM_GROUPS1]]
; CHECK: %[[TREE_UPDATE_TST:[^,]+]] = select i1 %[[ID_TST]], i1 true, i1 %[[ID_OFF_TEST]]
; CHECK: br i1 %[[TREE_UPDATE_TST]], label %atomic.free.red.global.update.latch, label %atomic.free.red.global.update.body

; CHECK-LABEL: atomic.free.red.global.update.body:
; CHECK: %[[CUR_VAL:[^,]+]] = load i32, i32 addrspace(1)* %[[GLOBAL_GEP]]
; CHECK: %[[NEIGHB_VAL:[^,]+]] = load i32, i32 addrspace(1)* %[[GLOBAL_GEP_OFF]]
; CHECK: %[[NEW_VAL:[^,]+]] = add i32 %[[NEIGHB_VAL]], %[[CUR_VAL]]
; CHECK: store i32 %[[NEW_VAL]], i32 addrspace(1)* %[[GLOBAL_GEP]]
; CHECK: shl i64 %[[IDX_PHI]], 1
; CHECK-NEXT: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 272)
; CHECK: br label %atomic.free.red.global.update.header
; CHECK-LABEL: atomic.free.red.global.update.store:
; CHECK: %[[INIT:[^,]+]] = load i32, i32 addrspace(1)* %[[RESULT_PTR:[^,]+]]
; CHECK: %[[TREE_RESULT:[^,]+]] = load i32, i32 addrspace(1)* %[[RED_GLOBAL_BUF]]
; CHECK: %[[RESULT:[^,]+]] = add i32 %[[TREE_RESULT]], %[[INIT]]
; CHECK: store i32 %[[RESULT]], i32 addrspace(1)* %[[RESULT_PTR]]

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent mustprogress noinline norecurse nounwind optnone
define protected noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %sum = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %sum.ascast = addrspacecast i32* %sum to i32 addrspace(4)*
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %retval.ascast, align 4
  store i32 0, i32 addrspace(4)* %sum.ascast, align 4
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 9, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %sum.ascast, i32 addrspace(4)* %sum.ascast, i64 4, i64 35, i8* null, i8* null), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.REDUCTION.ADD"(i32 addrspace(4)* %sum.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.REDUCTION.ADD"(i32 addrspace(4)* %sum.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast) ]
  %3 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %3, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %5 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4
  %7 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %8 = load i32, i32 addrspace(4)* %sum.ascast, align 4
  %add1 = add nsw i32 %8, %7
  store i32 %add1, i32 addrspace(4)* %sum.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add2 = add nsw i32 %9, 1
  store i32 %add2, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret i32 0
}


; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}

!0 = !{i32 0, i32 53, i32 -1915655848, !"_Z4main", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}

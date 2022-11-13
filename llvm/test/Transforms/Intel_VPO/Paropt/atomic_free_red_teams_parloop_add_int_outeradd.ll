; RUN: opt -enable-new-pm=0 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-red-local-buf-size=0 -vpo-paropt-atomic-free-reduction-slm=true  -vpo-paropt-atomic-free-reduction-par-global=false -vpo-paropt-atomic-free-red-use-fp-team-counter=false  -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-red-local-buf-size=0 -vpo-paropt-atomic-free-reduction-slm=true  -vpo-paropt-atomic-free-reduction-par-global=false -vpo-paropt-atomic-free-red-use-fp-team-counter=false  -S %s | FileCheck %s

; Test src:
;
; #include <stdio.h>
; int main() {
;   int s = 0;
; #pragma omp target teams reduction(+ : s) num_teams(1)
;   {
;     s += 10;
; #pragma omp parallel for reduction(+ : s)
;     for (int i = 0; i < 100; ++i) {
;       s += 1;
;     }
;   }
;
;   printf("%d\n", s);
;   return 0;
; }

; CHECK: %[[GROUP_ID:[^,]+]] = call spir_func i64 @_Z12get_group_idj(i32 0)
; CHECK: %[[LOCAL_SUM_GEP:[^,]+]] = getelementptr inbounds i32, ptr addrspace(1) %red_buf, i64 %[[GROUP_ID]]
; CHECK: %[[OLD_VAL:[^,]+]] = load i32, ptr addrspace(3) @[[LOCAL_PTR:[^,]+]]
; CHECK: %[[NEW_VAL:[^,]+]] = add nsw i32 %[[OLD_VAL]], 10
; CHECK: store i32 %[[NEW_VAL]], ptr addrspace(3) @[[LOCAL_PTR]]
; CHECK-LABEL: atomic.free.red.local.update.update.header:
; CHECK: %[[IDX_PHI:[^,]+]] = phi
; CHECK: %[[LOCAL_ID:[^,]+]] = call spir_func i64 @_Z12get_local_idj(i32 0)
; CHECK: %[[LOCAL_SIZE:[^,]+]] = call spir_func i64 @_Z14get_local_sizej(i32 0)
; CHECK: %[[CMP0:[^,]+]] = icmp uge i64 %[[IDX_PHI]], %[[LOCAL_SIZE]]
; CHECK: br i1 %[[CMP0]], label %atomic.free.red.local.update.update.exit, label %atomic.free.red.local.update.update.idcheck
; CHECK-LABEL: atomic.free.red.local.update.update.idcheck:
; CHECK: %[[CMP1:[^,]+]] = icmp eq i64 %[[LOCAL_ID]], %[[IDX_PHI]]
; CHECK: br i1 %[[CMP1]], label %atomic.free.red.local.update.update.body, label %atomic.free.red.local.update.update.latch
; CHECK-LABEL: atomic.free.red.local.update.update.body:
; CHECK: %[[PRIV_SUM_VAL:[^,]+]] = load
; CHECK: %[[LOCAL_SUM_VAL:[^,]+]] = load volatile i32, ptr addrspace(4) addrspacecast (ptr addrspace(3) @[[LOCAL_PTR]] to ptr addrspace(4))
; CHECK: %[[RED_VALUE:[^,]+]] = add i32 %[[LOCAL_SUM_VAL]], %[[PRIV_SUM_VAL]]
; CHECK: store i32 %[[RED_VALUE]], ptr addrspace(3) @[[LOCAL_PTR]]
; CHECK: br label %atomic.free.red.local.update.update.latch
; CHECK-LABEL: atomic.free.red.local.update.update.latch:
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 272)
; CHECK: add i64 %[[IDX_PHI]], 1
; CHECK: br label %atomic.free.red.local.update.update.header
; CHECK-LABEL: atomic.free.red.local.update.update.exit:
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 272)
; CHECK: %[[LOCAL_LD:[^,]+]] = load i32, ptr addrspace(3) @[[LOCAL_PTR]]
; CHECK: store i32 %[[LOCAL_LD]], ptr addrspace(1) %[[LOCAL_SUM_GEP]]
; CHECK-LABEL: counter_check:
; CHECK: %[[NUM_GROUPS:[^,]+]] = call spir_func i64 @_Z14get_num_groupsj(i32 0)
; CHECK: %[[TEAMS_COUNTER:[^,]+]] = addrspacecast ptr addrspace(1) %teams_counter to ptr addrspace(4)
; CHECK-LABEL: master.thread.code{{[0-9]+}}:
; CHECK: %[[UPD_CNTR:[^,]+]] = call spir_func i32 @__kmpc_atomic_fixed4_add_cpt(ptr addrspace(4) %[[TEAMS_COUNTER]], i32 1, i32 1)
; CHECK: store i32 %[[UPD_CNTR]], ptr addrspace(3) @.broadcast.ptr.__local, align 4
; CHECK-LABEL: master.thread.fallthru{{[0-9]+}}:
; CHECK: %.new = load i32, ptr addrspace(3) @.broadcast.ptr.__local, align 4
; CHECK: %[[NUM_GROUPS_TRUNC:[^,]+]] = trunc i64 %[[NUM_GROUPS]] to i32
; CHECK: %[[CNTR_CHECK:[^,]+]] = icmp ne i32 %.new, %[[NUM_GROUPS_TRUNC]]
; CHECK: br i1 %[[CNTR_CHECK]], label [[EXIT_BB:[^,]+]], label
; CHECK: call spir_func i64 @_Z12get_local_idj(i32 0)
; CHECK: call spir_func i64 @_Z12get_local_idj(i32 1)
; CHECK: call spir_func i64 @_Z12get_local_idj(i32 2)
; CHECK: %[[MT_CHECK:[^,]+]] = xor i1 %is.master.thread, true
; CHECK: br i1 %[[MT_CHECK]], label [[EXIT_BB]], label %atomic.free.red.global.update.header
; CHECK-LABEL: atomic.free.red.global.update.header:
; CHECK: %[[IDX_PHI:[^,]+]] = phi i64
; CHECK: %[[SUM_PHI:[^,]+]] = phi i32
; CHECK: %[[NUM_GROUPS1:[^,]+]] = call spir_func i64 @_Z14get_num_groupsj(i32 0)
; CHECK: %[[EXIT_COND:[^,]+]] = icmp uge i64 %[[IDX_PHI]], %[[NUM_GROUPS1]]
; CHECK: %[[GLOBAL_GEP:[^,]+]] = getelementptr i32, ptr addrspace(1) %red_buf, i64 %[[IDX_PHI]]
; CHECK: br i1 %[[EXIT_COND]], label %atomic.free.red.global.update.store, label %atomic.free.red.global.update.body
; CHECK-LABEL: atomic.free.red.global.update.body:
; CHECK: %[[CUR_VAL:[^,]+]] = load i32, ptr addrspace(1) %[[GLOBAL_GEP]], align 4
; CHECK: add i32 %[[SUM_PHI]], %[[CUR_VAL]]
; CHECK: add i64 %[[IDX_PHI]], 1
; CHECK: br label %atomic.free.red.global.update.header
; CHECK-LABEL: atomic.free.red.global.update.store:
; CHECK: store i32 %[[SUM_PHI]], ptr addrspace(1) %
; CHECK-NEXT: store i32 0, ptr addrspace(1) %teams_counter, align 4

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr addrspace(1) constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: convergent noinline nounwind optnone
define protected i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %s = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %s.ascast = addrspacecast ptr %s to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  store i32 0, ptr addrspace(4) %s.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %s.ascast, ptr addrspace(4) %s.ascast, i64 4, i64 547, ptr null, ptr null),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %s.ascast, i32 0, i32 1),
    "QUAL.OMP.NUM_TEAMS"(i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]
  %2 = load i32, ptr addrspace(4) %s.ascast, align 4
  %add = add nsw i32 %2, 10
  store i32 %add, ptr addrspace(4) %s.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 99, ptr addrspace(4) %.omp.ub.ascast, align 4
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %s.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1) ]
  %4 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %4, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %5 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %6 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %5, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %7, 1
  %add1 = add nsw i32 0, %mul
  store i32 %add1, ptr addrspace(4) %i.ascast, align 4
  %8 = load i32, ptr addrspace(4) %s.ascast, align 4
  %add2 = add nsw i32 %8, 1
  store i32 %add2, ptr addrspace(4) %s.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add3 = add nsw i32 %9, 1
  store i32 %add3, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  %10 = load i32, ptr addrspace(4) %s.ascast, align 4
  %call = call spir_func i32 (ptr addrspace(4), ...) @printf(ptr addrspace(4) noundef addrspacecast (ptr addrspace(1) @.str to ptr addrspace(4)), i32 noundef %10) #3
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: convergent
declare spir_func i32 @printf(ptr addrspace(4) noundef, ...) #2

attributes #0 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { convergent "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #3 = { convergent }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}

!0 = !{i32 0, i32 53, i32 -1915655849, !"_Z4main", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}

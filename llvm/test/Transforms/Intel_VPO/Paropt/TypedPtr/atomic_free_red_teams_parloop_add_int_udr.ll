; REQUIRES: asserts

; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-red-local-buf-size=0 -vpo-paropt-atomic-free-reduction-slm=true  -vpo-paropt-atomic-free-reduction-par-global=false -vpo-paropt-atomic-free-red-use-fp-team-counter=false -S %s | FileCheck -check-prefixes=CHECK,TC_ZEROINIT %s
; RUN: opt -opaque-pointers=0 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-red-local-buf-size=0 -vpo-paropt-atomic-free-reduction-slm=true  -vpo-paropt-atomic-free-reduction-par-global=false -vpo-paropt-atomic-free-red-use-fp-team-counter=false  -S %s | FileCheck -check-prefixes=CHECK,TC_ZEROINIT %s
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-red-local-buf-size=0 -vpo-paropt-atomic-free-reduction-slm=true  -vpo-paropt-atomic-free-reduction-par-global=false -S -vpo-paropt-atomic-free-red-use-fp-team-counter=true %s | FileCheck -check-prefixes=CHECK,TC_FP %s
; RUN: opt -opaque-pointers=0 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-red-local-buf-size=0 -vpo-paropt-atomic-free-reduction-slm=true  -vpo-paropt-atomic-free-reduction-par-global=false -vpo-paropt-atomic-free-red-use-fp-team-counter=true -S %s | FileCheck -check-prefixes=CHECK,TC_FP %s

; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-red-local-buf-size=0 -vpo-paropt-atomic-free-reduction-slm=true  -vpo-paropt-atomic-free-reduction-par-global=false -vpo-paropt-atomic-free-red-use-fp-team-counter=false -debug-only=vpo-paropt-target -S 2>&1 %s | FileCheck -check-prefixes=MAP,MAP_TC_ZEROINIT %s
; RUN: opt -opaque-pointers=0 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-red-local-buf-size=0 -vpo-paropt-atomic-free-reduction-slm=true  -vpo-paropt-atomic-free-reduction-par-global=false -vpo-paropt-atomic-free-red-use-fp-team-counter=false -debug-only=vpo-paropt-target -S 2>&1 %s | FileCheck -check-prefixes=MAP,MAP_TC_ZEROINIT %s
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-red-local-buf-size=0 -vpo-paropt-atomic-free-reduction-slm=true  -vpo-paropt-atomic-free-reduction-par-global=false -vpo-paropt-atomic-free-red-use-fp-team-counter=true -debug-only=vpo-paropt-target -S 2>&1 %s | FileCheck -check-prefixes=MAP,MAP_TC_FP %s
; RUN: opt -opaque-pointers=0 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-red-local-buf-size=0 -vpo-paropt-atomic-free-reduction-slm=true  -vpo-paropt-atomic-free-reduction-par-global=false -vpo-paropt-atomic-free-red-use-fp-team-counter=true -debug-only=vpo-paropt-target -S 2>&1 %s | FileCheck -check-prefixes=MAP,MAP_TC_FP %s

; Test src:
;
; int main(void) {
;   int i, sum = 0;
; #pragma omp declare reduction(myadd : int : omp_out += omp_in) initializer (omp_priv = 0)
; #pragma omp target teams distribute parallel for reduction(myadd:sum)
;   for (i = 0; i < 10; i++) {
;     sum += i;
;   }
;
;   return 0;
; }

; CHECK: define weak dso_local spir_kernel void @__omp_offloading{{.*}}main{{.*}}(i32 addrspace(1)* noalias %[[RESULT_PTR:sum.*]], i32 addrspace(1)* %[[RED_GLOBAL_BUF:red_buf.*]], i32 addrspace(1)* %[[TEAMS_COUNTER_PTR:teams_counter.*]], i64 %.omp.lb

; CHECK: %[[RESULT_PTR_ASCAST:[^,]+]] = addrspacecast i32 addrspace(1)* %sum.ascast to i32 addrspace(4)*
; CHECK: %[[GROUP_ID:[^,]+]] = call spir_func i64 @_Z12get_group_idj(i32 0)
; CHECK: %[[LOCAL_SUM_GEP:[^,]+]] = getelementptr inbounds i32, i32 addrspace(1)* %[[RED_GLOBAL_BUF]], i64 %[[GROUP_ID]]

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
; CHECK: call spir_func void @.omp_combiner.(i32 addrspace(4)* addrspacecast (i32 addrspace(3)* @[[LOCAL_PTR:[^,]+]] to i32 addrspace(4)*), i32 addrspace(4)* %
; CHECK: br label %atomic.free.red.local.update.update.latch
; CHECK-LABEL: atomic.free.red.local.update.update.latch:
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 272)
; CHECK: add i64 %[[IDX_PHI]], 1
; CHECK: br label %atomic.free.red.local.update.update.header
; CHECK-LABEL: atomic.free.red.local.update.update.exit:
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 272)

; CHECK-LABEL: master.thread.code{{[0-9]+}}:
; CHECK: %[[LOCAL_LD:[^,]+]] = load i32, i32 addrspace(3)* @[[LOCAL_PTR]]
; CHECK: store i32 %[[LOCAL_LD]], i32 addrspace(1)* %[[LOCAL_SUM_GEP]]

; CHECK-LABEL: counter_check:
; CHECK: %[[NUM_GROUPS:[^,]+]] = call spir_func i64 @_Z14get_num_groupsj(i32 0)
; CHECK: %[[NUM_GROUPS_TRUNC:[^,]+]] = trunc i64 %[[NUM_GROUPS]] to i32
; CHECK: %[[TEAMS_COUNTER:[^,]+]] = addrspacecast i32 addrspace(1)* %teams_counter to i32 addrspace(4)*
; CHECK: %[[UPD_CNTR:[^,]+]] = call spir_func i1 @__kmpc_team_reduction_ready(i32 addrspace(4)* %[[TEAMS_COUNTER]], i32 %[[NUM_GROUPS_TRUNC]])
; CHECK: %[[CNTR_CHECK:[^,]+]] = icmp ne i1 %[[UPD_CNTR]], true
; CHECK: br i1 %[[CNTR_CHECK]], label [[EXIT_BB:[^,]+]], label

; CHECK: call spir_func i64 @_Z12get_local_idj(i32 0)
; CHECK: call spir_func i64 @_Z12get_local_idj(i32 1)
; CHECK: call spir_func i64 @_Z12get_local_idj(i32 2)
; CHECK: %[[MT_CHECK:[^,]+]] = xor i1 %is.master.thread, true
; CHECK: br i1 %[[MT_CHECK]], label [[EXIT_BB]], label %atomic.free.red.global.update.header

; CHECK-LABEL: atomic.free.red.global.update.header:
; CHECK: %[[IDX_PHI:[^,]+]] = phi i64
; CHECK: %[[NUM_GROUPS1:[^,]+]] = call spir_func i64 @_Z14get_num_groupsj(i32 0)
; CHECK: %[[EXIT_COND:[^,]+]] = icmp uge i64 %[[IDX_PHI]], %[[NUM_GROUPS1]]
; CHECK: %[[GLOBAL_GEP:[^,]+]] = getelementptr i32, i32 addrspace(1)* %[[RED_GLOBAL_BUF]], i64 %[[IDX_PHI]]
; CHECK: %[[GLOBAL_GEP_ASCAST:[^,]+]] =  addrspacecast i32 addrspace(1)* %[[GLOBAL_GEP]] to i32 addrspace(4)*
; CHECK: br i1 %[[EXIT_COND]], label %atomic.free.red.global.update.store, label %atomic.free.red.global.update.body

; CHECK-LABEL: atomic.free.red.global.update.body:
; CHECK: call spir_func void @.omp_combiner.(i32 addrspace(4)* %[[RESULT_PTR_ASCAST]], i32 addrspace(4)* %[[GLOBAL_GEP_ASCAST]]), !paropt_guarded_by_thread_check
; CHECK: add i64 %[[IDX_PHI]], 1
; CHECK: br label %atomic.free.red.global.update.header

; CHECK-LABEL: atomic.free.red.global.update.store:
; TC_ZEROINIT-NEXT: store i32 0, i32 addrspace(1)* %[[TEAMS_COUNTER_PTR]]
; TC_FP-NOT: store i32 0, i32 addrspace(1)* %[[TEAMS_COUNTER_PTR]]

; CHECK-LABEL: master.thread.fallthru{{[0-9]+}}:

; MAP:              Adding map-type (@red_buf = extern_weak addrspace(1) global i32 #0, @red_buf = extern_weak addrspace(1) global i32 #0, i64 4, i64 66688)
; MAP_TC_ZEROINIT:  Adding map-type (@teams_counter = private addrspace(1) global i32 0 #1, @teams_counter = private addrspace(1) global i32 0 #1, i64 4, i64 16544)
; MAP_TC_FP:        Adding map-type (@teams_counter = private addrspace(1) global i32 0 #1, @teams_counter = private addrspace(1) global i32 0 #1, i64 4, i64 161)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind
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
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %sum.ascast, i32 addrspace(4)* %sum.ascast, i64 4, i64 547, i8* null, i8* null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %tmp.ascast, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.UDR:TYPED"(i32 addrspace(4)* %sum.ascast, i32 0, i32 1, i8* null, i8* null, void (i32 addrspace(4)*, i32 addrspace(4)*)* @.omp_combiner., void (i32 addrspace(4)*, i32 addrspace(4)*)* @.omp_initializer.),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(i32 addrspace(4)* %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(i32 addrspace(4)* %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %tmp.ascast, i32 0, i32 1) ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.REDUCTION.UDR:TYPED"(i32 addrspace(4)* %sum.ascast, i32 0, i32 1, i8* null, i8* null, void (i32 addrspace(4)*, i32 addrspace(4)*)* @.omp_combiner., void (i32 addrspace(4)*, i32 addrspace(4)*)* @.omp_initializer.),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %i.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(i32 addrspace(4)* %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(i32 addrspace(4)* %.omp.ub.ascast, i32 0) ]

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

; Function Attrs: convergent noinline nounwind
define internal void @.omp_combiner.(i32 addrspace(4)* noalias noundef %0, i32 addrspace(4)* noalias noundef %1) #2 {
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
define internal void @.omp_initializer.(i32 addrspace(4)* noalias noundef %0, i32 addrspace(4)* noalias noundef %1) #2 {
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
attributes #1 = { nounwind }
attributes #2 = { convergent noinline nounwind "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 2050, i32 60961173, !"_Z4main", i32 4, i32 0, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 8, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}

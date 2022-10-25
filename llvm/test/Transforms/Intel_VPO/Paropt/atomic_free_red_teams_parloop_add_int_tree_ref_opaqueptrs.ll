; RUN: opt -enable-new-pm=0 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-reduction-slm=true  -vpo-paropt-atomic-free-reduction-par-global=false -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-reduction-slm=true  -vpo-paropt-atomic-free-reduction-par-global=false -S %s | FileCheck %s
; RUN: opt -enable-new-pm=0 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-paropt-atomic-free-reduction-slm=true  -vpo-paropt-atomic-free-reduction-par-global=false -S %s | FileCheck -check-prefix=MAP %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -vpo-paropt-atomic-free-reduction-slm=true  -vpo-paropt-atomic-free-reduction-par-global=false -S %s | FileCheck -check-prefix=MAP %s

; Test src:
;
; int main(void) {
;   int i, sum = 0;
;   int &sumref = sum;
;
; #pragma omp target teams distribute parallel for reduction(+:sumref) map(tofrom:sumref)
;   for (i = 0; i < 10; i++) {
;     sumref += i;
;   }
;
;   return 0;
; }

; CHECK: %[[REF_PTR:[^,]+]] = alloca ptr addrspace(4)
; CHECK: %[[SUM:sumref.map.ptr.tmp.ascast.red]] = alloca i32
; CHECK: %[[SUM_REF:sumref.map.ptr.tmp.ascast.red.ascast.ref]] = alloca ptr addrspace(4)
; CHECK: %[[GROUP_ID:[^,]+]] = call spir_func i64 @_Z12get_group_idj(i32 0)
; CHECK: %[[LOCAL_SUM_GEP:[^,]+]] = getelementptr inbounds i32, ptr addrspace(1) %red_buf
; CHECK: store ptr addrspace(4) addrspacecast (ptr addrspace(3) @[[LOCAL_PTR:[^,]+]] to ptr addrspace(4)), ptr %[[REF_PTR]]
; CHECK: %[[SUM_CAST:sumref.map.ptr.tmp.ascast.red.ascast]] = addrspacecast ptr %[[SUM]] to ptr addrspace(4)
; CHECK: store ptr addrspace(4) %[[SUM_CAST]], ptr %[[SUM_REF]]
; CHECK: %[[REF_VAL:[^,]+]] = load ptr addrspace(4), ptr %[[REF_PTR]]
; CHECK: %[[LOCAL_ID:[^,]+]] = call spir_func i64 @_Z12get_local_idj(i32 0)
; CHECK-COUNT-7: lshr
; CHECK: add
; CHECK: %[[LOCAL_BUF:[^,]+]] = getelementptr inbounds [1024 x i32], ptr addrspace(3) @
; CHECK: load
; CHECK: store
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 272)
; CHECK-LABEL: atomic.free.red.local.update.update.header:
; CHECK: %[[IDX_PHI:[^,]+]] = phi
; CHECK-LABEL: atomic.free.red.local.update.update.idcheck:
; CHECK-LABEL: atomic.free.red.local.update.update.body:
; CHECK: %[[LOCAL_BUF_ITER:[^,]+]] = getelementptr inbounds i32, ptr addrspace(3) %[[LOCAL_BUF]], i64 %[[IDX_PHI]]
; CHECK: %[[V0_RHS:[^,]+]] = load i32, ptr addrspace(3) %[[LOCAL_BUF_ITER]]
; CHECK: %[[V0_LHS:[^,]+]] = load volatile i32, ptr addrspace(3) %[[LOCAL_BUF]]
; CHECK: %[[V0:[^,]+]] = add i32 %[[V0_LHS]], %[[V0_RHS]]
; CHECK: store i32 %[[V0]], ptr addrspace(3) %[[LOCAL_BUF]]
; CHECK: br label %atomic.free.red.local.update.update.latch
; CHECK-LABEL: atomic.free.red.local.update.update.latch:
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 272)
; CHECK: lshr i64 %[[IDX_PHI]], 1
; CHECK: br label %atomic.free.red.local.update.update.header
; CHECK-LABEL: atomic.free.red.local.update.update.exit:
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 272)
; CHECK: %[[MTT:[^,]+]] = icmp ne i64 %[[LOCAL_ID]], 0
; CHECK: br i1 %[[MTT]]
; CHECK: %[[V0_RHS_FINAL:[^,]+]] = load i32, ptr addrspace(3) %[[LOCAL_BUF]]
; CHECK: %[[V0_LHS_FINAL:[^,]+]] = load i32, ptr addrspace(4) %[[REF_VAL]]
; CHECK: %[[V0_FINAL:[^,]+]] = add i32 %[[V0_LHS_FINAL]], %[[V0_RHS_FINAL]]
; CHECK: store i32 %[[V0_FINAL]], ptr addrspace(4) %[[REF_VAL]]
; CHECK: %[[LOCAL_LD:[^,]+]] = load i32, ptr addrspace(3) @[[LOCAL_PTR]]
; CHECK: store i32 %[[LOCAL_LD]], ptr addrspace(1) %[[LOCAL_SUM_GEP]]
; CHECK-LABEL: atomic.free.red.global.update.header:
; CHECK: %[[IDX_PHI:[^,]+]] = phi i64
; CHECK: %[[SUM_PHI:[^,]+]] = phi i32
; CHECK: %[[GLOBAL_GEP:[^,]+]] = getelementptr i32, ptr addrspace(1) %red_buf, i64 %[[IDX_PHI]]
; CHECK-LABEL: atomic.free.red.global.update.body:
; CHECK: %[[CUR_VAL:[^,]+]] = load i32, ptr addrspace(1) %[[GLOBAL_GEP]]
; CHECK: add i32 %[[SUM_PHI]], %[[CUR_VAL]]
; CHECK: add i64 %[[IDX_PHI]], 1
; CHECK: br label %atomic.free.red.global.update.header
; CHECK-LABEL: atomic.free.red.global.update.store:
; CHECK: store i32 %[[SUM_PHI]], ptr addrspace(4) %

; MAP: call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(){{.*}}"QUAL.OMP.MAP.TO"(ptr addrspace(1) @red_buf, ptr addrspace(1) @red_buf, i64 4096, i64 1152), "QUAL.OMP.MAP.TO"(ptr addrspace(1) @teams_counter, ptr addrspace(1) @teams_counter, i64 4, i64 129)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent mustprogress noinline norecurse nounwind optnone
define protected noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %sum = alloca i32, align 4
  %sumref = alloca ptr addrspace(4), align 8
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %sumref.map.ptr.tmp = alloca ptr addrspace(4), align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %sum.ascast = addrspacecast ptr %sum to ptr addrspace(4)
  %sumref.ascast = addrspacecast ptr %sumref to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %sumref.map.ptr.tmp.ascast = addrspacecast ptr %sumref.map.ptr.tmp to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  store i32 0, ptr addrspace(4) %sum.ascast, align 4
  store ptr addrspace(4) %sum.ascast, ptr addrspace(4) %sumref.ascast, align 8
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 9, ptr addrspace(4) %.omp.ub.ascast, align 4
  %0 = load ptr addrspace(4), ptr addrspace(4) %sumref.ascast, align 8
  %1 = load ptr addrspace(4), ptr addrspace(4) %sumref.ascast, align 8
  %2 = load ptr addrspace(4), ptr addrspace(4) %sumref.ascast, align 8
  %3 = load ptr addrspace(4), ptr addrspace(4) %sumref.ascast, align 8
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %2, ptr addrspace(4) %3, i64 4, i64 35, ptr null, ptr null),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %sumref.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]
  store ptr addrspace(4) %2, ptr addrspace(4) %sumref.map.ptr.tmp.ascast, align 8
  %5 = load ptr addrspace(4), ptr addrspace(4) %sumref.map.ptr.tmp.ascast, align 8
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:BYREF.TYPED"(ptr addrspace(4) %sumref.map.ptr.tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]
  %7 = load ptr addrspace(4), ptr addrspace(4) %sumref.map.ptr.tmp.ascast, align 8
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.REDUCTION.ADD:BYREF.TYPED"(ptr addrspace(4) %sumref.map.ptr.tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0) ]
  %9 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %9, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %10 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %11 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %10, %11
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %12 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %12, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  %13 = load i32, ptr addrspace(4) %i.ascast, align 4
  %14 = load ptr addrspace(4), ptr addrspace(4) %sumref.map.ptr.tmp.ascast, align 8
  %15 = load i32, ptr addrspace(4) %14, align 4
  %add1 = add nsw i32 %15, %13
  store i32 %add1, ptr addrspace(4) %14, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %16 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add2 = add nsw i32 %16, 1
  store i32 %add2, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TARGET"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent mustprogress noinline norecurse nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 53, i32 -1916233603, !"_Z4main", i32 5, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}

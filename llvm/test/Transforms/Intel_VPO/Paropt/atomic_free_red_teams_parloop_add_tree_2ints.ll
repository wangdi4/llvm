; RUN: opt -enable-new-pm=0 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; int main(void) {
;   int i, sum1 = 0, sum2 = 0;
;
; #pragma omp target teams distribute parallel for reduction(+ : sum1, sum2)
;   for (i = 0; i < 10; i++) {
;     sum1 += i;
;     sum2 += i * 2;
;   }
;
;   return 0;
; }

; CHECK: %[[GROUP_ID:[^,]+]] = call spir_func i64 @_Z12get_group_idj(i32 0)
; CHECK: %[[LOCAL_SUM_GEP1:[^,]+]] = getelementptr i32, ptr addrspace(1) %red_buf.1
; CHECK: %[[LOCAL_SUM_GEP:[^,]+]] = getelementptr i32, ptr addrspace(1) %red_buf
; CHECK-LABEL: omp.loop.exit:
; CHECK: DIR.OMP.END.DISTRIBUTE.PARLOOP{{[0-9.]*}}:
; CHECK: %[[LOCAL_ID:[^,]+]] = call spir_func i64 @_Z12get_local_idj(i32 0)
; CHECK-COUNT-7: lshr
; CHECK: add
; CHECK: %[[LOCAL_BUF:[^,]+]] = getelementptr inbounds [1024 x i32], ptr addrspace(3) @
; CHECK: load
; CHECK: store
; CHECK: %[[LOCAL_BUF1:[^,]+]] = getelementptr inbounds [1024 x i32], ptr addrspace(3) @
; CHECK: load
; CHECK: store
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 272)
; CHECK-LABEL: atomic.free.red.local.update.update.header:
; CHECK: %[[IDX_PHI:[^,]+]] = phi
; CHECK-LABEL: atomic.free.red.local.update.update.idcheck:
; CHECK-LABEL: atomic.free.red.local.update.update.body:
; CHECK: %[[LOCAL_BUF1_ITER:[^,]+]] = getelementptr inbounds i32, ptr addrspace(3) %[[LOCAL_BUF1]], i64 %[[IDX_PHI]]
; CHECK: %[[LOCAL_BUF_ITER:[^,]+]] = getelementptr inbounds i32, ptr addrspace(3) %[[LOCAL_BUF]], i64 %[[IDX_PHI]]
; CHECK: %[[V0_RHS:[^,]+]] = load i32, ptr addrspace(3) %[[LOCAL_BUF_ITER]]
; CHECK: %[[V0_LHS:[^,]+]] = load volatile i32, ptr addrspace(3) %[[LOCAL_BUF]]
; CHECK: %[[V0:[^,]+]] = add i32 %[[V0_LHS]], %[[V0_RHS]]
; CHECK: store i32 %[[V0]], ptr addrspace(3) %[[LOCAL_BUF]]
; CHECK: %[[V1_RHS:[^,]+]] = load i32, ptr addrspace(3) %[[LOCAL_BUF1_ITER]]
; CHECK: %[[V1_LHS:[^,]+]] = load volatile i32, ptr addrspace(3) %[[LOCAL_BUF1]]
; CHECK: %[[V1:[^,]+]] = add i32 %[[V1_LHS]], %[[V1_RHS]]
; CHECK: store i32 %[[V1]], ptr addrspace(3) %[[LOCAL_BUF1]]
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
; CHECK: %[[V0_LHS_FINAL:[^,]+]] = load i32, ptr addrspace(3) @[[LOCAL_VAR:[^,]]]
; CHECK: %[[V0_FINAL:[^,]+]] = add i32 %[[V0_LHS_FINAL]], %[[V0_RHS_FINAL]]
; CHECK: store i32 %[[V0_FINAL]], ptr addrspace(3) @[[LOCAL_VAR]]
; CHECK: %[[V1_RHS_FINAL:[^,]+]] = load i32, ptr addrspace(3) %[[LOCAL_BUF1]],
; CHECK: %[[V1_LHS_FINAL:[^,]+]] = load i32, ptr addrspace(3) @[[LOCAL_VAR1:[^,]]]
; CHECK: %[[V1_FINAL:[^,]+]] = add i32 %[[V1_LHS_FINAL]], %[[V1_RHS_FINAL]]
; CHECK: store i32 %[[V1_FINAL]], ptr addrspace(3) @[[LOCAL_VAR1]]
; CHECK: %[[LOCAL_LD:[^,]+]] = load i32, ptr addrspace(3) @[[LOCAL_VAR]]
; CHECK: store i32 %[[LOCAL_LD]], ptr addrspace(1) %[[LOCAL_SUM_GEP]]
; CHECK: %[[LOCAL_LD1:[^,]+]] = load i32, ptr addrspace(3) @[[LOCAL_VAR1]]
; CHECK: store i32 %[[LOCAL_LD1]], ptr addrspace(1) %[[LOCAL_SUM_GEP1]]
; CHECK-LABEL: atomic.free.red.global.update.header:
; CHECK: %[[IDX_PHI:[^,]+]] = phi i64
; CHECK: %[[SUM_PHI1:[^,]+]] = phi i32
; CHECK: %[[SUM_PHI:[^,]+]] = phi i32
; CHECK: %[[GLOBAL_GEP:[^,]+]] = getelementptr i32, ptr addrspace(1) %red_buf, i64 %[[IDX_PHI]]
; CHECK: %[[GLOBAL_GEP1:[^,]+]] = getelementptr i32, ptr addrspace(1) %red_buf.1, i64 %[[IDX_PHI]]
; CHECK-LABEL: atomic.free.red.global.update.body:
; CHECK: %[[CUR_VAL:[^,]+]] = load i32, ptr addrspace(1) %[[GLOBAL_GEP]]
; CHECK: add i32 %[[SUM_PHI]], %[[CUR_VAL]]
; CHECK: %[[CUR_VAL1:[^,]+]] = load i32, ptr addrspace(1) %[[GLOBAL_GEP1]]
; CHECK: add i32 %[[SUM_PHI1]], %[[CUR_VAL1]]
; CHECK: add i64 %[[IDX_PHI]], 1
; CHECK: br label %atomic.free.red.global.update.header
; CHECK-LABEL: atomic.free.red.global.update.store:
; CHECK: store i32 %[[SUM_PHI]], ptr addrspace(1) %
; CHECK: store i32 %[[SUM_PHI1]], ptr addrspace(1) %

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind optnone
define protected i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %sum1 = alloca i32, align 4
  %sum2 = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %sum1.ascast = addrspacecast ptr %sum1 to ptr addrspace(4)
  %sum2.ascast = addrspacecast ptr %sum2 to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  store i32 0, ptr addrspace(4) %sum1.ascast, align 4
  store i32 0, ptr addrspace(4) %sum2.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 9, ptr addrspace(4) %.omp.ub.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum1.ascast, ptr addrspace(4) %sum1.ascast, i64 4, i64 547, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum2.ascast, ptr addrspace(4) %sum2.ascast, i64 4, i64 547, ptr null, ptr null),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum1.ascast, i32 0, i32 1),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum1.ascast, i32 0, i32 1),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0) ]
  %3 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %3, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %5 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  %7 = load i32, ptr addrspace(4) %i.ascast, align 4
  %8 = load i32, ptr addrspace(4) %sum1.ascast, align 4
  %add1 = add nsw i32 %8, %7
  store i32 %add1, ptr addrspace(4) %sum1.ascast, align 4
  %9 = load i32, ptr addrspace(4) %i.ascast, align 4
  %mul2 = mul nsw i32 %9, 2
  %10 = load i32, ptr addrspace(4) %sum2.ascast, align 4
  %add3 = add nsw i32 %10, %mul2
  store i32 %add3, ptr addrspace(4) %sum2.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %11 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add4 = add nsw i32 %11, 1
  store i32 %add4, ptr addrspace(4) %.omp.iv.ascast, align 4
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

!0 = !{i32 0, i32 53, i32 -1926676584, !"_Z4main", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}

; REQUIRES: asserts

; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; int main(void) {
;   float sum = 0;
;   int sum2 = 0;
;
; #pragma omp target teams distribute parallel for reduction(+:sum, sum2)
;   for (int i = 0; i < 10; i++) {
;     sum += i;
;     sum2 += 2*i;
;   }
;
;   return 0;
; }

; CHECK: %[[BUF1_BASE:[^,]+]] = getelementptr inbounds float, ptr addrspace(1) %red_local_buf, i64
; CHECK: %[[BUF2_BASE:[^,]+]] = getelementptr inbounds i32, ptr addrspace(1) %red_local_buf{{.+}}, i64
; CHECK-LABEL: atomic.free.red.local.update.update.header:
; CHECK: %[[IDX_PHI:[^,]+]] = phi i64
; CHECK-LABEL: atomic.free.red.local.update.update.body:
; CHECK: %[[BUF2:[^,]+]] = getelementptr inbounds i32, ptr addrspace(1) %[[BUF2_BASE]], i64 %[[IDX_PHI]]
; CHECK: %[[BUF1:[^,]+]] = getelementptr inbounds float, ptr addrspace(1) %[[BUF1_BASE]], i64 %[[IDX_PHI]]
; CHECK: %[[VAL1_PRIV:[^,]+]] = load float, ptr addrspace(1) %[[BUF1]]
; CHECK: %[[VAL1_BASE:[^,]+]] = load volatile float, ptr addrspace(1) %[[BUF1_BASE]]
; CHECK: %[[VAL1_RES:[^,]+]] = fadd float %[[VAL1_BASE]], %[[VAL1_PRIV]]
; CHECK: store float %[[VAL1_RES]], ptr addrspace(1) %[[BUF1_BASE]]
; CHECK: %[[VAL2_PRIV:[^,]+]] = load i32, ptr addrspace(1) %[[BUF2]]
; CHECK: %[[VAL2_BASE:[^,]+]] = load volatile i32, ptr addrspace(1) %[[BUF2_BASE]]
; CHECK: %[[VAL2_RES:[^,]+]] = add i32 %[[VAL2_BASE]], %[[VAL2_PRIV]]
; CHECK: store i32 %[[VAL2_RES]], ptr addrspace(1) %[[BUF2_BASE]]
; CHECK: br label %atomic.free.red.local.update.update.latch
; CHECK-LABEL: atomic.free.red.local.update.update.latch:

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent mustprogress noinline norecurse nounwind optnone
define protected noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %sum = alloca float, align 4
  %sum2 = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %sum.ascast = addrspacecast ptr %sum to ptr addrspace(4)
  %sum2.ascast = addrspacecast ptr %sum2 to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  store float 0.000000e+00, ptr addrspace(4) %sum.ascast, align 4
  store i32 0, ptr addrspace(4) %sum2.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 9, ptr addrspace(4) %.omp.ub.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum.ascast, ptr addrspace(4) %sum.ascast, i64 4, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1) | UNKNOWN (0x8000)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum2.ascast, ptr addrspace(4) %sum2.ascast, i64 4, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1) | UNKNOWN (0x8000)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum.ascast, float 0.000000e+00, i32 1),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum.ascast, float 0.000000e+00, i32 1),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum2.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1) ]

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
  %conv = sitofp i32 %7 to float
  %8 = load float, ptr addrspace(4) %sum.ascast, align 4
  %add1 = fadd fast float %8, %conv
  store float %add1, ptr addrspace(4) %sum.ascast, align 4
  %9 = load i32, ptr addrspace(4) %i.ascast, align 4
  %mul2 = mul nsw i32 2, %9
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
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

attributes #0 = { "contains-openmp-target"="true" }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 66312, i32 204411369, !"_Z4main", i32 5, i32 0, i32 0, i32 0}

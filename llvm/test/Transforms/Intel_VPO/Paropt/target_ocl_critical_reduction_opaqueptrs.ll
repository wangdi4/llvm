; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck %s

; Original code:
;
; using TargetTy = int64_t;
; using AtomicTy = int32_t;
; 
; int main() {
;         constexpr int N = 1024;
;         AtomicTy a[N];
;
;         for (int i = 0; i < N; i++)
;                a[i] = i+1;
;
;         TargetTy sum0 = 0, sum1 = 0;
;         AtomicTy sum2 = 0;
; #pragma omp target parallel for reduction(*:sum0) reduction(+:sum1) reduction(+:sum2) map(to:a[:N])
;         for (int i = 0; i < N; i++) {
;                 sum0 *= a[i];
;                 sum1 += a[i];
;                 sum2 += static_cast<AtomicTy>(a[i]);
;         }
;         return 0;
; }
;
; This test checks that reduction items of different types concerning critical section need
; are guarded correctly:
;   sum0 should be wrapped in a critical section with a subgroup loop;
;   sum1 should be wrapped in a critical section w/o any loop as it has a mathcing OCL sub_group_* builtin;
;   sum2 should NOT be wrapped in a critical section as it can be done with an atomic.
;
; Atomic-free reduction is disabled just for simplicity of check for a single __kmpc_atomic_ builtin
;

; CHECK-LABEL: non.crit.insert.block
; CHECK: br label %[[NONCRITBODYBB:[^,]+]]
; CHECK-LABEL: crit.serial.insert.block
; CHECK: call spir_func void @__kmpc_critical
; CHECK: br label %[[CRITSERIALBODYBB:[^,]+]]
; CHECK-LABEL: crit.loop.insert.block
; CHECK: call spir_func i32 @_Z16get_sub_group_idv()
; CHECK: br label %[[CRITLOOPBODYBB:[^,]+]]

; CHECK: [[CRITLOOPBODYBB]]:
; CHECK: call spir_func void @__kmpc_critical(
; CHECK: call spir_func i32 @_Z18get_sub_group_sizev()
; CHECK: load
; CHECK: load
; CHECK: mul
; CHECK: store

; CHECK: [[CRITSERIALBODYBB]]:
; CHECK: call spir_func i64 @_Z20sub_group_reduce_addl
; CHECK: br label %crit.loop.insert.block

; CHECK: [[NONCRITBODYBB]]:
; CHECK-NOT:: call spir_func void @__kmpc_critical
; CHECK: call spir_func void @__kmpc_atomic_fixed4_add
; CHECK: br label %crit.serial.insert.block

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent mustprogress noinline norecurse nounwind optnone
define protected noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %N = alloca i32, align 4
  %a = alloca [1024 x i32], align 4
  %i = alloca i32, align 4
  %sum0 = alloca i64, align 8
  %sum1 = alloca i64, align 8
  %sum2 = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i3 = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %N.ascast = addrspacecast ptr %N to ptr addrspace(4)
  %a.ascast = addrspacecast ptr %a to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %sum0.ascast = addrspacecast ptr %sum0 to ptr addrspace(4)
  %sum1.ascast = addrspacecast ptr %sum1 to ptr addrspace(4)
  %sum2.ascast = addrspacecast ptr %sum2 to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %i3.ascast = addrspacecast ptr %i3 to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  store i32 1024, ptr addrspace(4) %N.ascast, align 4
  store i32 0, ptr addrspace(4) %i.ascast, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr addrspace(4) %i.ascast, align 4
  %cmp = icmp slt i32 %0, 1024
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32, ptr addrspace(4) %i.ascast, align 4
  %add = add nsw i32 %1, 1
  %2 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds [1024 x i32], ptr addrspace(4) %a.ascast, i64 0, i64 %idxprom
  store i32 %add, ptr addrspace(4) %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %3 = load i32, ptr addrspace(4) %i.ascast, align 4
  %inc = add nsw i32 %3, 1
  store i32 %inc, ptr addrspace(4) %i.ascast, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  store i64 1, ptr addrspace(4) %sum0.ascast, align 8
  store i64 0, ptr addrspace(4) %sum1.ascast, align 8
  store i32 0, ptr addrspace(4) %sum2.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 1023, ptr addrspace(4) %.omp.ub.ascast, align 4
  %arrayidx1 = getelementptr inbounds [1024 x i32], ptr addrspace(4) %a.ascast, i64 0, i64 0
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum0.ascast, ptr addrspace(4) %sum0.ascast, i64 8, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1) | UNKNOWN (0x8000)
    "QUAL.OMP.MAP.TO"(ptr addrspace(4) %a.ascast, ptr addrspace(4) %arrayidx1, i64 4096, i64 33, ptr null, ptr null), ; MAP type: 33 = 0x21 = TARGET_PARAM (0x20) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum1.ascast, ptr addrspace(4) %sum1.ascast, i64 8, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1) | UNKNOWN (0x8000)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum2.ascast, ptr addrspace(4) %sum2.ascast, i64 4, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1) | UNKNOWN (0x8000)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  %array.begin = getelementptr inbounds [1024 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.MUL:TYPED"(ptr addrspace(4) %sum0.ascast, i64 0, i32 1),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum1.ascast, i64 0, i32 1),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum2.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 1024),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i3.ascast, i32 0, i32 1) ]

  %6 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %6, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %for.end
  %7 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %8 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp2 = icmp sle i32 %7, %8
  br i1 %cmp2, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %9, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, ptr addrspace(4) %i3.ascast, align 4
  %10 = load i32, ptr addrspace(4) %i3.ascast, align 4
  %idxprom5 = sext i32 %10 to i64
  %arrayidx6 = getelementptr inbounds [1024 x i32], ptr addrspace(4) %a.ascast, i64 0, i64 %idxprom5
  %11 = load i32, ptr addrspace(4) %arrayidx6, align 4
  %conv = sext i32 %11 to i64
  %12 = load i64, ptr addrspace(4) %sum0.ascast, align 8
  %mul7 = mul nsw i64 %12, %conv
  store i64 %mul7, ptr addrspace(4) %sum0.ascast, align 8
  %13 = load i32, ptr addrspace(4) %i3.ascast, align 4
  %idxprom8 = sext i32 %13 to i64
  %arrayidx9 = getelementptr inbounds [1024 x i32], ptr addrspace(4) %a.ascast, i64 0, i64 %idxprom8
  %14 = load i32, ptr addrspace(4) %arrayidx9, align 4
  %conv9 = sext i32 %14 to i64
  %15 = load i64, ptr addrspace(4) %sum1.ascast, align 8
  %add10 = sub nsw i64 %15, %conv9
  store i64 %add10, ptr addrspace(4) %sum1.ascast, align 8
  %16 = load i32, ptr addrspace(4) %i3.ascast, align 4
  %idxprom12 = sext i32 %16 to i64
  %arrayidx13 = getelementptr inbounds [1024 x i32], ptr addrspace(4) %a.ascast, i64 0, i64 %idxprom12
  %17 = load i32, ptr addrspace(4) %arrayidx13, align 4
  %18 = load i32, ptr addrspace(4) %sum2.ascast, align 4
  %add14 = add nsw i32 %18, %17
  store i32 %add14, ptr addrspace(4) %sum2.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %19 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add15 = add nsw i32 %19, 1
  store i32 %add15, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

attributes #0 = { "contains-openmp-target"="true" "may-have-openmp-directive"="true" }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2050, i32 52095079, !"_Z4main", i32 15, i32 0, i32 0, i32 0}

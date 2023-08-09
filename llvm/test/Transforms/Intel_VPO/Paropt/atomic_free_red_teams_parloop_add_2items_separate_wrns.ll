; REQUIRES: asserts

; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-reduction-slm=false -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-reduction-slm=false -S %s | FileCheck %s

; Test src:
;
; int main(void) {
;   float sum = 0;
;   int sum2 = 0;
;
; #pragma omp target teams distribute reduction(+:sum, sum2)
;   for (int i = 0; i < 10; i++) {
;     #pragma omp parallel for reduction(+:sum)
;     for (int j = 0; j < 10; j++)
;       sum += i;
;     #pragma omp parallel for reduction(+:sum2)
;     for (int j = 0; j < 10; j++)
;       sum2 += 2*i;
;   }
;
;   return 0;
; }

; CHECK:  getelementptr inbounds float, ptr addrspace(1) %red_local_buf, i64
; CHECK:  getelementptr inbounds i32, ptr addrspace(1) %red_local_buf{{.+}}, i64

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
  %tmp1 = alloca i32, align 4
  %.omp.iv2 = alloca i32, align 4
  %.omp.lb3 = alloca i32, align 4
  %.omp.ub4 = alloca i32, align 4
  %j = alloca i32, align 4
  %tmp12 = alloca i32, align 4
  %.omp.iv13 = alloca i32, align 4
  %.omp.lb14 = alloca i32, align 4
  %.omp.ub15 = alloca i32, align 4
  %j19 = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %sum.ascast = addrspacecast ptr %sum to ptr addrspace(4)
  %sum2.ascast = addrspacecast ptr %sum2 to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %tmp1.ascast = addrspacecast ptr %tmp1 to ptr addrspace(4)
  %.omp.iv2.ascast = addrspacecast ptr %.omp.iv2 to ptr addrspace(4)
  %.omp.lb3.ascast = addrspacecast ptr %.omp.lb3 to ptr addrspace(4)
  %.omp.ub4.ascast = addrspacecast ptr %.omp.ub4 to ptr addrspace(4)
  %j.ascast = addrspacecast ptr %j to ptr addrspace(4)
  %tmp12.ascast = addrspacecast ptr %tmp12 to ptr addrspace(4)
  %.omp.iv13.ascast = addrspacecast ptr %.omp.iv13 to ptr addrspace(4)
  %.omp.lb14.ascast = addrspacecast ptr %.omp.lb14 to ptr addrspace(4)
  %.omp.ub15.ascast = addrspacecast ptr %.omp.ub15 to ptr addrspace(4)
  %j19.ascast = addrspacecast ptr %j19 to ptr addrspace(4)
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
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv13.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb14.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub15.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j19.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp1.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp12.ascast, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum.ascast, float 0.000000e+00, i32 1),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv13.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb14.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub15.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j19.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp1.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp12.ascast, i32 0, i32 1) ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv13.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb14.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub15.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j19.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp1.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp12.ascast, i32 0, i32 1) ]

  %3 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %3, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc30, %entry
  %4 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %5 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end32

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb3.ascast, align 4
  store i32 9, ptr addrspace(4) %.omp.ub4.ascast, align 4
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum.ascast, float 0.000000e+00, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv2.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb3.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub4.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1) ]

  %8 = load i32, ptr addrspace(4) %.omp.lb3.ascast, align 4
  store i32 %8, ptr addrspace(4) %.omp.iv2.ascast, align 4
  br label %omp.inner.for.cond5

omp.inner.for.cond5:                              ; preds = %omp.inner.for.inc, %omp.inner.for.body
  %9 = load i32, ptr addrspace(4) %.omp.iv2.ascast, align 4
  %10 = load i32, ptr addrspace(4) %.omp.ub4.ascast, align 4
  %cmp6 = icmp sle i32 %9, %10
  br i1 %cmp6, label %omp.inner.for.body7, label %omp.inner.for.end

omp.inner.for.body7:                              ; preds = %omp.inner.for.cond5
  %11 = load i32, ptr addrspace(4) %.omp.iv2.ascast, align 4
  %mul8 = mul nsw i32 %11, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, ptr addrspace(4) %j.ascast, align 4
  %12 = load i32, ptr addrspace(4) %i.ascast, align 4
  %conv = sitofp i32 %12 to float
  %13 = load float, ptr addrspace(4) %sum.ascast, align 4
  %add10 = fadd fast float %13, %conv
  store float %add10, ptr addrspace(4) %sum.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body7
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %14 = load i32, ptr addrspace(4) %.omp.iv2.ascast, align 4
  %add11 = add nsw i32 %14, 1
  store i32 %add11, ptr addrspace(4) %.omp.iv2.ascast, align 4
  br label %omp.inner.for.cond5

omp.inner.for.end:                                ; preds = %omp.inner.for.cond5
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  store i32 0, ptr addrspace(4) %.omp.lb14.ascast, align 4
  store i32 9, ptr addrspace(4) %.omp.ub15.ascast, align 4
  %15 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum2.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv13.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb14.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub15.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j19.ascast, i32 0, i32 1) ]

  %16 = load i32, ptr addrspace(4) %.omp.lb14.ascast, align 4
  store i32 %16, ptr addrspace(4) %.omp.iv13.ascast, align 4
  br label %omp.inner.for.cond16

omp.inner.for.cond16:                             ; preds = %omp.inner.for.inc25, %omp.loop.exit
  %17 = load i32, ptr addrspace(4) %.omp.iv13.ascast, align 4
  %18 = load i32, ptr addrspace(4) %.omp.ub15.ascast, align 4
  %cmp17 = icmp sle i32 %17, %18
  br i1 %cmp17, label %omp.inner.for.body18, label %omp.inner.for.end27

omp.inner.for.body18:                             ; preds = %omp.inner.for.cond16
  %19 = load i32, ptr addrspace(4) %.omp.iv13.ascast, align 4
  %mul20 = mul nsw i32 %19, 1
  %add21 = add nsw i32 0, %mul20
  store i32 %add21, ptr addrspace(4) %j19.ascast, align 4
  %20 = load i32, ptr addrspace(4) %i.ascast, align 4
  %mul22 = mul nsw i32 2, %20
  %21 = load i32, ptr addrspace(4) %sum2.ascast, align 4
  %add23 = add nsw i32 %21, %mul22
  store i32 %add23, ptr addrspace(4) %sum2.ascast, align 4
  br label %omp.body.continue24

omp.body.continue24:                              ; preds = %omp.inner.for.body18
  br label %omp.inner.for.inc25

omp.inner.for.inc25:                              ; preds = %omp.body.continue24
  %22 = load i32, ptr addrspace(4) %.omp.iv13.ascast, align 4
  %add26 = add nsw i32 %22, 1
  store i32 %add26, ptr addrspace(4) %.omp.iv13.ascast, align 4
  br label %omp.inner.for.cond16

omp.inner.for.end27:                              ; preds = %omp.inner.for.cond16
  br label %omp.loop.exit28

omp.loop.exit28:                                  ; preds = %omp.inner.for.end27
  call void @llvm.directive.region.exit(token %15) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  br label %omp.body.continue29

omp.body.continue29:                              ; preds = %omp.loop.exit28
  br label %omp.inner.for.inc30

omp.inner.for.inc30:                              ; preds = %omp.body.continue29
  %23 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add31 = add nsw i32 %23, 1
  store i32 %add31, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end32:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit33


omp.loop.exit33:                                  ; preds = %omp.inner.for.end32
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE"() ]

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

!0 = !{i32 0, i32 66312, i32 204411083, !"_Z4main", i32 5, i32 0, i32 0, i32 0}
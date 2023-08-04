; REQUIRES: asserts
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-reduction-slm=false -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-reduction-slm=false -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-reduction=false -vpo-paropt-atomic-free-reduction-slm=false -S %s | FileCheck -check-prefixes=CHECK_NOAF %s
; RUN: opt -opaque-pointers=0 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-reduction=false -vpo-paropt-atomic-free-reduction-slm=false -S %s | FileCheck -check-prefixes=CHECK_NOAF %s

; Test src:
;
; int main(void) {
;   static constexpr int N = 10;
;   int res[N];
;   int sum = 0;
;
; #pragma omp target teams distribute map(tofrom:res[0:N])
;   for (int i = 0; i < N; i++) {
;     #pragma omp parallel for reduction(+:sum)
;     for (int j = 0; j < N; j++) {
;       sum += j;
;     }
;     res[i] = sum;
;   }
;   return 0;
; }

; This test checks the kernel_info format with enabled and disable atomic-free reduction. In the 'enabled' case there's an extra argument (reduction buffer) passed,
; as well as the WI/WG limits equal to 1024

; CHECK: %[[KINFO_TYPE:[^,]+]] = type { i32, i32, [5 x %1], i64, i64, i64 }
; CHECK: %[[KINFO_ARGMAP_TYPE:[^,]+]] = type { i32, i32 }
; CHECK: @__omp_offloading{{.*}}main{{.*}}_kernel_info = weak target_declare addrspace(1) constant %[[KINFO_TYPE]] { i32 5, i32 5, [5 x %[[KINFO_ARGMAP_TYPE]]] [%[[KINFO_ARGMAP_TYPE]] { i32 0, i32 8 }, %[[KINFO_ARGMAP_TYPE]] { i32 0, i32 8 }, %[[KINFO_ARGMAP_TYPE]] { i32 0, i32 8 }, %[[KINFO_ARGMAP_TYPE]] { i32 0, i32 8 }, %[[KINFO_ARGMAP_TYPE]] { i32 0, i32 8 }], i64 0, i64 1024, i64 1024 }

; CHECK_NOAF: %[[KINFO_TYPE:[^,]+]] = type { i32, i32, [4 x %1], i64, i64, i64 }
; CHECK_NOAF: %[[KINFO_ARGMAP_TYPE:[^,]+]] = type { i32, i32 }
; CHECK_NOAF: @__omp_offloading{{.*}}main{{.*}}_kernel_info = weak target_declare addrspace(1) constant %[[KINFO_TYPE]] { i32 5, i32 4, [4 x %[[KINFO_ARGMAP_TYPE]]] [%[[KINFO_ARGMAP_TYPE]] { i32 0, i32 8 }, %[[KINFO_ARGMAP_TYPE]] { i32 0, i32 8 }, %[[KINFO_ARGMAP_TYPE]] { i32 0, i32 8 }, %[[KINFO_ARGMAP_TYPE]] { i32 0, i32 8 }], i64 0, i64 0, i64 0 }


target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@_ZZ4mainE1N = internal addrspace(1) constant i32 10, align 4

; Function Attrs: convergent mustprogress noinline norecurse nounwind optnone
define protected noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %res = alloca [10 x i32], align 4
  %sum = alloca i32, align 4
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
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %res.ascast = addrspacecast [10 x i32]* %res to [10 x i32] addrspace(4)*
  %sum.ascast = addrspacecast i32* %sum to i32 addrspace(4)*
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %tmp1.ascast = addrspacecast i32* %tmp1 to i32 addrspace(4)*
  %.omp.iv2.ascast = addrspacecast i32* %.omp.iv2 to i32 addrspace(4)*
  %.omp.lb3.ascast = addrspacecast i32* %.omp.lb3 to i32 addrspace(4)*
  %.omp.ub4.ascast = addrspacecast i32* %.omp.ub4 to i32 addrspace(4)*
  %j.ascast = addrspacecast i32* %j to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %retval.ascast, align 4
  store i32 0, i32 addrspace(4)* %sum.ascast, align 4
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 9, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32] addrspace(4)* %res.ascast, i64 0, i64 0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"([10 x i32] addrspace(4)* %res.ascast, i32 addrspace(4)* %arrayidx, i64 40, i64 35, i8* null, i8* null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %sum.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %.omp.iv2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %.omp.lb3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %.omp.ub4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %tmp1.ascast, i32 0, i32 1) ]

  %array.begin = getelementptr inbounds [10 x i32], [10 x i32] addrspace(4)* %res.ascast, i32 0, i32 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"([10 x i32] addrspace(4)* %res.ascast, i32 0, i64 10),
    "QUAL.OMP.SHARED:TYPED"(i32 addrspace(4)* %sum.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(i32 addrspace(4)* %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(i32 addrspace(4)* %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %.omp.iv2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %.omp.lb3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %.omp.ub4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %tmp1.ascast, i32 0, i32 1) ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(i32 addrspace(4)* %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(i32 addrspace(4)* %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %.omp.iv2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %.omp.lb3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %.omp.ub4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %tmp1.ascast, i32 0, i32 1) ]

  %3 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %3, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc14, %entry
  %4 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %5 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end16

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4
  store i32 0, i32 addrspace(4)* %.omp.lb3.ascast, align 4
  store i32 9, i32 addrspace(4)* %.omp.ub4.ascast, align 4
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(i32 addrspace(4)* %sum.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(i32 addrspace(4)* %.omp.iv2.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %.omp.lb3.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(i32 addrspace(4)* %.omp.ub4.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %j.ascast, i32 0, i32 1) ]

  %8 = load i32, i32 addrspace(4)* %.omp.lb3.ascast, align 4
  store i32 %8, i32 addrspace(4)* %.omp.iv2.ascast, align 4
  br label %omp.inner.for.cond5

omp.inner.for.cond5:                              ; preds = %omp.inner.for.inc, %omp.inner.for.body
  %9 = load i32, i32 addrspace(4)* %.omp.iv2.ascast, align 4
  %10 = load i32, i32 addrspace(4)* %.omp.ub4.ascast, align 4
  %cmp6 = icmp sle i32 %9, %10
  br i1 %cmp6, label %omp.inner.for.body7, label %omp.inner.for.end

omp.inner.for.body7:                              ; preds = %omp.inner.for.cond5
  %11 = load i32, i32 addrspace(4)* %.omp.iv2.ascast, align 4
  %mul8 = mul nsw i32 %11, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, i32 addrspace(4)* %j.ascast, align 4
  %12 = load i32, i32 addrspace(4)* %j.ascast, align 4
  %13 = load i32, i32 addrspace(4)* %sum.ascast, align 4
  %add10 = add nsw i32 %13, %12
  store i32 %add10, i32 addrspace(4)* %sum.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body7
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %14 = load i32, i32 addrspace(4)* %.omp.iv2.ascast, align 4
  %add11 = add nsw i32 %14, 1
  store i32 %add11, i32 addrspace(4)* %.omp.iv2.ascast, align 4
  br label %omp.inner.for.cond5

omp.inner.for.end:                                ; preds = %omp.inner.for.cond5
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  %15 = load i32, i32 addrspace(4)* %sum.ascast, align 4
  %16 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %idxprom = sext i32 %16 to i64
  %arrayidx12 = getelementptr inbounds [10 x i32], [10 x i32] addrspace(4)* %res.ascast, i64 0, i64 %idxprom
  store i32 %15, i32 addrspace(4)* %arrayidx12, align 4
  br label %omp.body.continue13

omp.body.continue13:                              ; preds = %omp.loop.exit
  br label %omp.inner.for.inc14

omp.inner.for.inc14:                              ; preds = %omp.body.continue13
  %17 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add15 = add nsw i32 %17, 1
  store i32 %add15, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end16:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit17

omp.loop.exit17:                                  ; preds = %omp.inner.for.end16
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

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

!0 = !{i32 0, i32 64773, i32 8149272, !"_Z4main", i32 8, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 8, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}

; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-collapse-always=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -vpo-paropt-collapse-always=false -S %s | FileCheck %s

; Original code:
;int main() {
;  int s = 0;
;#pragma omp target teams distribute parallel for collapse(2) reduction(+:s)
;  for (int i = 0; i < 100; ++i)
;    for (int j = 0; j < 100; ++j);
;
;  return 0;
;}

; For regions with reduction we do not use specific ND-range partitioning,
; which implies that we use 1D-range with some number of WGs and WIs.
; In this case we have to make sure that we collapse the loop nest,
; otherwise, the outer loop will run serially using GWS 1 and LWS 1
; of dimension 1.

; Check that the loop nest is collapsed and parallelized with default ND-range:
; CHECK: call spir_func i64 @_Z14get_num_groupsj(i32 0)
; CHECK-NOT: call spir_func i64 @_Z13get_global_idj(
; CHECK-NOT: call spir_func i64 @_Z14get_num_groupsj(i32 1)
; CHECK-NOT: call spir_func i64 @_Z14get_num_groupsj(i32 2)


target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define protected i32 @main() {
entry:
  %retval = alloca i32, align 4
  %s = alloca i32, align 4
  %.omp.uncollapsed.lb = alloca i32, align 4
  %.omp.uncollapsed.ub = alloca i32, align 4
  %.omp.uncollapsed.lb1 = alloca i32, align 4
  %.omp.uncollapsed.ub2 = alloca i32, align 4
  %tmp = alloca i32, align 4
  %tmp3 = alloca i32, align 4
  %.omp.uncollapsed.iv = alloca i32, align 4
  %.omp.uncollapsed.iv4 = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %s.ascast = addrspacecast ptr %s to ptr addrspace(4)
  %.omp.uncollapsed.lb.ascast = addrspacecast ptr %.omp.uncollapsed.lb to ptr addrspace(4)
  %.omp.uncollapsed.ub.ascast = addrspacecast ptr %.omp.uncollapsed.ub to ptr addrspace(4)
  %.omp.uncollapsed.lb1.ascast = addrspacecast ptr %.omp.uncollapsed.lb1 to ptr addrspace(4)
  %.omp.uncollapsed.ub2.ascast = addrspacecast ptr %.omp.uncollapsed.ub2 to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %tmp3.ascast = addrspacecast ptr %tmp3 to ptr addrspace(4)
  %.omp.uncollapsed.iv.ascast = addrspacecast ptr %.omp.uncollapsed.iv to ptr addrspace(4)
  %.omp.uncollapsed.iv4.ascast = addrspacecast ptr %.omp.uncollapsed.iv4 to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %j.ascast = addrspacecast ptr %j to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  store i32 0, ptr addrspace(4) %s.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.uncollapsed.lb.ascast, align 4
  store i32 99, ptr addrspace(4) %.omp.uncollapsed.ub.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.uncollapsed.lb1.ascast, align 4
  store i32 99, ptr addrspace(4) %.omp.uncollapsed.ub2.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %s.ascast, ptr addrspace(4) %s.ascast, i64 4, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1) | UNKNOWN (0x8000)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv4.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb1.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp3.ascast, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %s.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv4.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb1.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp3.ascast, i32 0, i32 1) ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.COLLAPSE"(i32 2),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %s.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i32 0, ptr addrspace(4) %.omp.uncollapsed.iv4.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i32 0, ptr addrspace(4) %.omp.uncollapsed.ub2.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb1.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1) ]

  %3 = load i32, ptr addrspace(4) %.omp.uncollapsed.lb.ascast, align 4
  store i32 %3, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc11, %entry
  %4 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  %5 = load i32, ptr addrspace(4) %.omp.uncollapsed.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end13

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %6 = load i32, ptr addrspace(4) %.omp.uncollapsed.lb1.ascast, align 4
  store i32 %6, ptr addrspace(4) %.omp.uncollapsed.iv4.ascast, align 4
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.cond5:                       ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body
  %7 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv4.ascast, align 4
  %8 = load i32, ptr addrspace(4) %.omp.uncollapsed.ub2.ascast, align 4
  %cmp6 = icmp sle i32 %7, %8
  br i1 %cmp6, label %omp.uncollapsed.loop.body7, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body7:                       ; preds = %omp.uncollapsed.loop.cond5
  %9 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  %mul = mul nsw i32 %9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  %10 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv4.ascast, align 4
  %mul8 = mul nsw i32 %10, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, ptr addrspace(4) %j.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.uncollapsed.loop.body7
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.body.continue
  %11 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv4.ascast, align 4
  %add10 = add nsw i32 %11, 1
  store i32 %add10, ptr addrspace(4) %.omp.uncollapsed.iv4.ascast, align 4
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond5
  br label %omp.uncollapsed.loop.inc11

omp.uncollapsed.loop.inc11:                       ; preds = %omp.uncollapsed.loop.end
  %12 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  %add12 = add nsw i32 %12, 1
  store i32 %add12, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end13:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 52, i32 -700641407, !"_Z4main", i32 3, i32 0, i32 0, i32 0}

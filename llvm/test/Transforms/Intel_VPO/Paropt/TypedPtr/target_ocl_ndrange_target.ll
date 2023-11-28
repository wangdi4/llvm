; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; void foo() {
; #pragma omp target parallel for collapse(2)
;   for (int i = 0; i <= 19; ++i)
;     for (int j = 0; j <= 23; ++j);
; }

; CHECK: [[GID0:%[a-zA-Z._0-9]+]] = call spir_func i64 @_Z13get_global_idj(i32 0)

; CHECK: [[GID1:%[a-zA-Z._0-9]+]] = call spir_func i64 @_Z13get_global_idj(i32 1)
; CHECK: [[BND1:%[a-zA-Z._0-9]+]] = trunc i64 [[GID1]] to i32
; CHECK: store i32 [[BND1]], i32* [[LBNDPTR1:%[a-zA-Z._0-9]+]]
; CHECK: [[MINUBPRED1:%.+]] = icmp slt i32 [[BND1]], [[UBV1:%[a-zA-Z._0-9]+]]
; CHECK: [[MINUBV1:%.+]] = select i1 [[MINUBPRED1]], i32 [[BND1]], i32 [[UBV1]]
; CHECK: store i32 [[MINUBV1]], i32* [[UBNDPTR1:%[a-zA-Z._0-9]+]]
; CHECK: [[LBND1:%[a-zA-Z._0-9]+]] = load i32, i32* [[LBNDPTR1]]
; CHECK: [[UBND1:%[a-zA-Z._0-9]+]] = load i32, i32* [[UBNDPTR1]]
; CHECK: icmp sle i32 [[LBND1]], [[UBND1]]

; CHECK: [[BND0:%[a-zA-Z._0-9]+]] = trunc i64 [[GID0]] to i32
; CHECK: store i32 [[BND0]], i32* [[LBNDPTR0:%[a-zA-Z._0-9]+]]
; CHECK: [[MINUBPRED0:%.+]] = icmp slt i32 [[BND0]], [[UBV0:%[a-zA-Z._0-9]+]]
; CHECK: [[MINUBV0:%.+]] = select i1 [[MINUBPRED0]], i32 [[BND0]], i32 [[UBV0]]
; CHECK: store i32 [[MINUBV0]], i32* [[UBNDPTR0:%[a-zA-Z._0-9]+]]
; CHECK: [[LBND0:%[a-zA-Z._0-9]+]] = load i32, i32* [[LBNDPTR0]]
; CHECK: [[UBND0:%[a-zA-Z._0-9]+]] = load i32, i32* [[UBNDPTR0]]
; CHECK: icmp sle i32 [[LBND0]], [[UBND0]]

; CHECK: icmp sle i32 %{{.*}}, [[UBND0]]
; CHECK: icmp sle i32 %{{.*}}, [[UBND1]]


target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

define dso_local spir_func void @foo() #0 {
entry:
  %.omp.uncollapsed.lb = alloca i32, align 4
  %.omp.uncollapsed.lb.ascast = addrspacecast i32* %.omp.uncollapsed.lb to i32 addrspace(4)*
  %.omp.uncollapsed.ub = alloca i32, align 4
  %.omp.uncollapsed.ub.ascast = addrspacecast i32* %.omp.uncollapsed.ub to i32 addrspace(4)*
  %.omp.uncollapsed.lb1 = alloca i32, align 4
  %.omp.uncollapsed.lb1.ascast = addrspacecast i32* %.omp.uncollapsed.lb1 to i32 addrspace(4)*
  %.omp.uncollapsed.ub2 = alloca i32, align 4
  %.omp.uncollapsed.ub2.ascast = addrspacecast i32* %.omp.uncollapsed.ub2 to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %tmp3 = alloca i32, align 4
  %tmp3.ascast = addrspacecast i32* %tmp3 to i32 addrspace(4)*
  %.omp.uncollapsed.iv = alloca i32, align 4
  %.omp.uncollapsed.iv.ascast = addrspacecast i32* %.omp.uncollapsed.iv to i32 addrspace(4)*
  %.omp.uncollapsed.iv4 = alloca i32, align 4
  %.omp.uncollapsed.iv4.ascast = addrspacecast i32* %.omp.uncollapsed.iv4 to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %j = alloca i32, align 4
  %j.ascast = addrspacecast i32* %j to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %.omp.uncollapsed.lb.ascast, align 4
  store i32 19, i32 addrspace(4)* %.omp.uncollapsed.ub.ascast, align 4
  store i32 0, i32 addrspace(4)* %.omp.uncollapsed.lb1.ascast, align 4
  store i32 23, i32 addrspace(4)* %.omp.uncollapsed.ub2.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.ub.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb1.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.ub2.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp3.ascast) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.COLLAPSE"(i32 2), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.uncollapsed.ub.ascast, i32 addrspace(4)* %.omp.uncollapsed.ub2.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb1.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast) ]
  %2 = load i32, i32 addrspace(4)* %.omp.uncollapsed.lb.ascast, align 4
  store i32 %2, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc11, %entry
  %3 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  %4 = load i32, i32 addrspace(4)* %.omp.uncollapsed.ub.ascast, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end13

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %5 = load i32, i32 addrspace(4)* %.omp.uncollapsed.lb1.ascast, align 4
  store i32 %5, i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast, align 4
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.cond5:                       ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body
  %6 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast, align 4
  %7 = load i32, i32 addrspace(4)* %.omp.uncollapsed.ub2.ascast, align 4
  %cmp6 = icmp sle i32 %6, %7
  br i1 %cmp6, label %omp.uncollapsed.loop.body7, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body7:                       ; preds = %omp.uncollapsed.loop.cond5
  %8 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  %mul = mul nsw i32 %8, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4
  %9 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast, align 4
  %mul8 = mul nsw i32 %9, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, i32 addrspace(4)* %j.ascast, align 4
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.uncollapsed.loop.body7
  %10 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast, align 4
  %add10 = add nsw i32 %10, 1
  store i32 %add10, i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast, align 4
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond5
  br label %omp.uncollapsed.loop.inc11

omp.uncollapsed.loop.inc11:                       ; preds = %omp.uncollapsed.loop.end
  %11 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  %add12 = add nsw i32 %11, 1
  store i32 %add12, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end13:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { "contains-openmp-target"="true" }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2052, i32 85987529, !"foo", i32 2, i32 0, i32 0}

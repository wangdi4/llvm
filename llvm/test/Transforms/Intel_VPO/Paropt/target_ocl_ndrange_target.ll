; RUN: opt < %s -prepare-switch-to-offload=true -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt  -S | FileCheck %s
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -prepare-switch-to-offload=true -switch-to-offload  -S | FileCheck %s

; Original code:
; void foo() {
; #pragma omp target parallel for collapse(2)
;   for (int i = 0; i <= 19; ++i)
;     for (int j = 0; j <= 23; ++j);
; }

; CHECK: [[GID0:%[a-zA-Z._0-9]+]] = call i64 @_Z13get_global_idj(i32 0)

; CHECK: [[BND0:%[a-zA-Z._0-9]+]] = trunc i64 [[GID0]] to i32
; CHECK: store i32 [[BND0]], i32* [[LBNDPTR0:%[a-zA-Z._0-9]+]]
; CHECK: store i32 [[BND0]], i32* [[UBNDPTR0:%[a-zA-Z._0-9]+]]
; CHECK: [[LBND0:%[a-zA-Z._0-9]+]] = load i32, i32* [[LBNDPTR0]]
; CHECK: [[UBND0:%[a-zA-Z._0-9]+]] = load i32, i32* [[UBNDPTR0]]
; CHECK: icmp sle i32 [[LBND0]], [[UBND0]]

; CHECK: [[GID1:%[a-zA-Z._0-9]+]] = call i64 @_Z13get_global_idj(i32 1)
; CHECK: [[BND1:%[a-zA-Z._0-9]+]] = trunc i64 [[GID1]] to i32
; CHECK: store i32 [[BND1]], i32* [[LBNDPTR1:%[a-zA-Z._0-9]+]]
; CHECK: store i32 [[BND1]], i32* [[UBNDPTR1:%[a-zA-Z._0-9]+]]
; CHECK: [[LBND1:%[a-zA-Z._0-9]+]] = load i32, i32* [[LBNDPTR1]]
; CHECK: [[UBND1:%[a-zA-Z._0-9]+]] = load i32, i32* [[UBNDPTR1]]
; CHECK: icmp sle i32 [[LBND1]], [[UBND1]]

; CHECK: icmp sle i32 %{{.*}}, [[UBND0]]
; CHECK: icmp sle i32 %{{.*}}, [[UBND1]]


target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
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

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2052, i32 85987529, !"foo", i32 2, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 9.0.0"}

; RUN: opt -enable-new-pm=0 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-implicit-multiple-teams=true -S %s | FileCheck -check-prefix=TRUE %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -vpo-implicit-multiple-teams=true -S %s | FileCheck -check-prefix=TRUE %s
; RUN: opt -enable-new-pm=0 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-implicit-multiple-teams=false -S %s | FileCheck -check-prefix=FALSE %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -vpo-implicit-multiple-teams=false -S %s | FileCheck -check-prefix=FALSE %s

; Original code:
; void foo(int *a) {
; #pragma omp target parallel for map(a[:1000])
;   for (int i = 0; i < 1000; ++i)
;     a[i] = 1;
; }

; Check that ND-range parallelization is used with -vpo-implicit-multiple-teams=true,
; and not used with -vpo-implicit-multiple-teams=false:

; TRUE: call{{.*}}_Z13get_global_idj
; FALSE-NOT: call{{.*}}_Z13get_global_idj

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"
; Function Attrs: noinline nounwind optnone uwtable
define hidden spir_func void @foo(i32 addrspace(4)* %a) #0 {
entry:
  %a.addr = alloca i32 addrspace(4)*, align 8
  %a.addr.ascast = addrspacecast i32 addrspace(4)** %a.addr to i32 addrspace(4)* addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %a.map.ptr.tmp = alloca i32 addrspace(4)*, align 8
  %a.map.ptr.tmp.ascast = addrspacecast i32 addrspace(4)** %a.map.ptr.tmp to i32 addrspace(4)* addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  store i32 addrspace(4)* %a, i32 addrspace(4)* addrspace(4)* %a.addr.ascast, align 8
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 999, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %0 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %a.addr.ascast, align 8
  %1 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %a.addr.ascast, align 8
  %arrayidx = getelementptr inbounds i32, i32 addrspace(4)* %1, i64 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(i32 addrspace(4)* %0, i32 addrspace(4)* %arrayidx, i64 4000), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* addrspace(4)* %a.map.ptr.tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  store i32 addrspace(4)* %0, i32 addrspace(4)* addrspace(4)* %a.map.ptr.tmp.ascast, align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspace(4)* %a.map.ptr.tmp.ascast) ]
  %4 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %4, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %5 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %6 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %5, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4
  %8 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %a.map.ptr.tmp.ascast, align 8
  %9 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %idxprom = sext i32 %9 to i64
  %arrayidx1 = getelementptr inbounds i32, i32 addrspace(4)* %8, i64 %idxprom
  store i32 1, i32 addrspace(4)* %arrayidx1, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add2 = add nsw i32 %10, 1
  store i32 %add2, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  ret void
}
; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1
; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2052, i32 55989659, !"foo", i32 2, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 9.0.0"}

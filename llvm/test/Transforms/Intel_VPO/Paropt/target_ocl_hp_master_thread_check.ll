; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; void foo() {
; #pragma omp target
;   {
;     int x = 10;
; #pragma omp parallel for shared(x)
;     for (int i = 0; i < 77; ++i)
; #pragma omp atomic update
;       x += i;
;   }
; }

; Check that the master thread predicate is computed and used only once.
; The three side-effect stores inside the target region are consecuitive,
; so they may be guarded all at once:
; CHECK-DAG: [[ID0:%[a-zA-Z._0-9]+]] = call spir_func i64 @_Z12get_local_idj(i32 0)
; CHECK-DAG: [[CMP0:%[a-zA-Z._0-9]+]] = icmp eq i64 [[ID0]], 0
; CHECK-DAG: [[ID1:%[a-zA-Z._0-9]+]] = call spir_func i64 @_Z12get_local_idj(i32 1)
; CHECK-DAG: [[CMP1:%[a-zA-Z._0-9]+]] = icmp eq i64 [[ID1]], 0
; CHECK-DAG: [[ID2:%[a-zA-Z._0-9]+]] = call spir_func i64 @_Z12get_local_idj(i32 2)
; CHECK-DAG: [[CMP2:%[a-zA-Z._0-9]+]] = icmp eq i64 [[ID2]], 0
; CHECK-DAG: [[AND:%[a-zA-Z._0-9]+]] = and i1 [[CMP0]], [[CMP1]]
; CHECK-DAG: [[PRED:%[a-zA-Z._0-9]+]] = and i1 [[AND]], [[CMP2]]
; CHECK: br i1 [[PRED]], label %[[MASTERCODE:[a-zA-Z._0-9]+]], label %[[FALLTHRU:[a-zA-Z._0-9]+]]
; CHECK: [[MASTERCODE]]:
; CHECK: br label %[[FALLTHRU]]
; CHECK: [[FALLTHRU]]:
; CHECK-NOT: br i1 [[PRED]]
; An extra call to get_local_id(0) is required for parallel for:
; CHECK: call spir_func i64 @_Z12get_local_idj(i32 0)
; CHECK-NOT: call{{.*}}@_Z12get_local_idj(i32 0)
; CHECK-NOT: call{{.*}}@_Z12get_local_idj(i32 1)
; CHECK-NOT: call{{.*}}@_Z12get_local_idj(i32 2)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"
; Function Attrs: noinline nounwind optnone uwtable
define hidden spir_func void @foo() #0 {
entry:
  %x = alloca i32, align 4
  %x.ascast = addrspacecast i32* %x to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %atomic-temp = alloca i32, align 4
  %atomic-temp.ascast = addrspacecast i32* %atomic-temp to i32 addrspace(4)*
  %atomic-temp1 = alloca i32, align 4
  %atomic-temp1.ascast = addrspacecast i32* %atomic-temp1 to i32 addrspace(4)*
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %x.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %atomic-temp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %atomic-temp1.ascast) ]
  store i32 10, i32 addrspace(4)* %x.ascast, align 4
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 76, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SHARED"(i32 addrspace(4)* %x.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %atomic-temp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %atomic-temp1.ascast) ]
  %2 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %2, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %4 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4
  %6 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %7 = bitcast i32 addrspace(4)* %x.ascast to i8 addrspace(4)*
  %8 = bitcast i32 addrspace(4)* %atomic-temp.ascast to i8 addrspace(4)*
  call void @__atomic_load(i64 4, i8 addrspace(4)* %7, i8 addrspace(4)* %8, i32 0)
  br label %atomic_cont

atomic_cont:                                      ; preds = %atomic_cont, %omp.inner.for.body
  %9 = load i32, i32 addrspace(4)* %atomic-temp.ascast, align 4
  %add2 = add nsw i32 %9, %6
  store i32 %add2, i32 addrspace(4)* %atomic-temp1.ascast, align 4
  %10 = bitcast i32 addrspace(4)* %x.ascast to i8 addrspace(4)*
  %11 = bitcast i32 addrspace(4)* %atomic-temp.ascast to i8 addrspace(4)*
  %12 = bitcast i32 addrspace(4)* %atomic-temp1.ascast to i8 addrspace(4)*
  %call = call zeroext i1 @__atomic_compare_exchange(i64 4, i8 addrspace(4)* %10, i8 addrspace(4)* %11, i8 addrspace(4)* %12, i32 0, i32 0)
  br i1 %call, label %atomic_exit, label %atomic_cont

atomic_exit:                                      ; preds = %atomic_cont
  br label %omp.body.continue

omp.body.continue:                                ; preds = %atomic_exit
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %13 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add3 = add nsw i32 %13, 1
  store i32 %add3, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}
; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1
; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1
declare dso_local void @__atomic_load(i64, i8 addrspace(4)*, i8 addrspace(4)*, i32)
declare dso_local i1 @__atomic_compare_exchange(i64, i8 addrspace(4)*, i8 addrspace(4)*, i8 addrspace(4)*, i32, i32)

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2052, i32 51517360, !"foo", i32 2, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 9.0.0"}

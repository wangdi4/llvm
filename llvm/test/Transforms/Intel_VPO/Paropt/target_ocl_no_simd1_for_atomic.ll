; RUN: opt < %s -prepare-switch-to-offload=true -switch-to-offload=true -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt  -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -prepare-switch-to-offload=true -switch-to-offload=true  -S  2>&1 | FileCheck %s

; Original code:
; void foo(int *x, int *v, int *expr) {
; #pragma omp target parallel for map(tofrom: x[0], v[0], expr[0])
;   for (int i = 0; i < 100; i++) {
; #pragma omp atomic capture
;     {*v = *x; *x += *expr; }
;   }
; }

; Check that there is no SIMD1 emulation caused by calls to atomic functions:
; CHECK-DAG: @__kmpc_atomic_load
; CHECK-DAG: @__kmpc_atomic_compare_exchange
; CHECK-NOT: @_Z18get_num_sub_groupsv
; CHECK-NOT: @_Z16get_sub_group_idv

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @foo(i32 addrspace(4)* %x, i32 addrspace(4)* %v, i32 addrspace(4)* %expr) #0 {
entry:
  %x.addr = alloca i32 addrspace(4)*, align 8
  %x.addr.ascast = addrspacecast i32 addrspace(4)** %x.addr to i32 addrspace(4)* addrspace(4)*
  %v.addr = alloca i32 addrspace(4)*, align 8
  %v.addr.ascast = addrspacecast i32 addrspace(4)** %v.addr to i32 addrspace(4)* addrspace(4)*
  %expr.addr = alloca i32 addrspace(4)*, align 8
  %expr.addr.ascast = addrspacecast i32 addrspace(4)** %expr.addr to i32 addrspace(4)* addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %atomic-temp = alloca i32, align 4
  %atomic-temp.ascast = addrspacecast i32* %atomic-temp to i32 addrspace(4)*
  %atomic-temp3 = alloca i32, align 4
  %atomic-temp3.ascast = addrspacecast i32* %atomic-temp3 to i32 addrspace(4)*
  store i32 addrspace(4)* %x, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  store i32 addrspace(4)* %v, i32 addrspace(4)* addrspace(4)* %v.addr.ascast, align 8
  store i32 addrspace(4)* %expr, i32 addrspace(4)* addrspace(4)* %expr.addr.ascast, align 8
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 99, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %0 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %arrayidx = getelementptr inbounds i32, i32 addrspace(4)* %0, i64 0
  %1 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %v.addr.ascast, align 8
  %arrayidx1 = getelementptr inbounds i32, i32 addrspace(4)* %1, i64 0
  %2 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %expr.addr.ascast, align 8
  %arrayidx2 = getelementptr inbounds i32, i32 addrspace(4)* %2, i64 0
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(i32 addrspace(4)* addrspace(4)* %x.addr.ascast, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, i64 8), "QUAL.OMP.MAP.TOFROM:AGGR"(i32 addrspace(4)* addrspace(4)* %x.addr.ascast, i32 addrspace(4)* %arrayidx, i64 4), "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(i32 addrspace(4)* addrspace(4)* %v.addr.ascast, i32 addrspace(4)* addrspace(4)* %v.addr.ascast, i64 8), "QUAL.OMP.MAP.TOFROM:AGGR"(i32 addrspace(4)* addrspace(4)* %v.addr.ascast, i32 addrspace(4)* %arrayidx1, i64 4), "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(i32 addrspace(4)* addrspace(4)* %expr.addr.ascast, i32 addrspace(4)* addrspace(4)* %expr.addr.ascast, i64 8), "QUAL.OMP.MAP.TOFROM:AGGR"(i32 addrspace(4)* addrspace(4)* %expr.addr.ascast, i32 addrspace(4)* %arrayidx2, i64 4), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %atomic-temp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %atomic-temp3.ascast) ]
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspace(4)* %v.addr.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspace(4)* %x.addr.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspace(4)* %expr.addr.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %atomic-temp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %atomic-temp3.ascast) ]
  %5 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %5, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %7 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %6, %7
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %8, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4
  %9 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %v.addr.ascast, align 8
  %10 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %11 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %expr.addr.ascast, align 8
  %12 = load i32, i32 addrspace(4)* %11, align 4
  %13 = bitcast i32 addrspace(4)* %10 to i8 addrspace(4)*
  %14 = bitcast i32 addrspace(4)* %atomic-temp.ascast to i8 addrspace(4)*
  call void @__atomic_load(i64 4, i8 addrspace(4)* %13, i8 addrspace(4)* %14, i32 0)
  br label %atomic_cont

atomic_cont:                                      ; preds = %atomic_cont, %omp.inner.for.body
  %15 = load i32, i32 addrspace(4)* %atomic-temp.ascast, align 4
  %add4 = add nsw i32 %15, %12
  store i32 %add4, i32 addrspace(4)* %atomic-temp3.ascast, align 4
  %16 = bitcast i32 addrspace(4)* %10 to i8 addrspace(4)*
  %17 = bitcast i32 addrspace(4)* %atomic-temp.ascast to i8 addrspace(4)*
  %18 = bitcast i32 addrspace(4)* %atomic-temp3.ascast to i8 addrspace(4)*
  %call = call zeroext i1 @__atomic_compare_exchange(i64 4, i8 addrspace(4)* %16, i8 addrspace(4)* %17, i8 addrspace(4)* %18, i32 0, i32 0)
  br i1 %call, label %atomic_exit, label %atomic_cont

atomic_exit:                                      ; preds = %atomic_cont
  store i32 %15, i32 addrspace(4)* %9, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %atomic_exit
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %19 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add5 = add nsw i32 %19, 1
  store i32 %add5, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]
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

!0 = !{i32 0, i32 2052, i32 55982831, !"foo", i32 2, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 8.0.0"}

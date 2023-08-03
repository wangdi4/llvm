; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; int test1() {
;   int res;
; #pragma omp target map(from: res)
;   {
;     int xvar = 10;
; #pragma omp parallel for shared(xvar)
;     for (int i = 0; i < 77; ++i)
; #pragma omp atomic update
;       xvar += i;
;     res = xvar;
;   }
;
;   return res;
; }
;
; void test2(int *output) {
;   int res;
; #pragma omp target map(from: res, output)
;   {
;     int xvar = 10;
; #pragma omp parallel for shared(xvar)
;     for (int i = 0; i < 77; ++i)
;       output[i] = xvar;
;   }
; }

; 'xvar' must be globalized, because it is modified by multiple
; threads:
; CHECK-LABEL: @__omp_offloading_804_3121174_test1_l3(
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
; CHECK: store i32 10, ptr addrspace(1) @xvar{{.*}}
; CHECK: br label %[[FALLTHRU]]
; CHECK: [[FALLTHRU]]:
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)

; 'xvar' must be WI local, because it is only modified inside "target"
; and only read by WI threads:
; CHECK-LABEL: @__omp_offloading_804_3121174_test2_l18(
; CHECK-NOT: call{{.*}}@_Z12get_local_idj(i32 1)
; CHECK-NOT: call{{.*}}@_Z12get_local_idj(i32 2)
; CHECK: store i32 10, ptr %xvar{{.*}}

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"
; Function Attrs: noinline nounwind optnone uwtable
define hidden spir_func i32 @test1() #0 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %res = alloca i32, align 4
  %res.ascast = addrspacecast ptr %res to ptr addrspace(4)
  %xvar = alloca i32, align 4
  %xvar.ascast = addrspacecast ptr %xvar to ptr addrspace(4)
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %i = alloca i32, align 4
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %atomic-temp = alloca i32, align 4
  %atomic-temp.ascast = addrspacecast ptr %atomic-temp to ptr addrspace(4)
  %atomic-temp1 = alloca i32, align 4
  %atomic-temp1.ascast = addrspacecast ptr %atomic-temp1 to ptr addrspace(4)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.FROM"(ptr addrspace(4) %res.ascast, ptr addrspace(4) %res.ascast, i64 4, i64 34, ptr null, ptr null), ; MAP type: 34 = 0x22 = TARGET_PARAM (0x20) | FROM (0x2)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %xvar.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %atomic-temp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %atomic-temp1.ascast, i32 0, i32 1) ]

  store i32 10, ptr addrspace(4) %xvar.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 76, ptr addrspace(4) %.omp.ub.ascast, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %xvar.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %atomic-temp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %atomic-temp1.ascast, i32 0, i32 1) ]

  %2 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %2, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %4 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  %6 = load i32, ptr addrspace(4) %i.ascast, align 4
  %7 = bitcast ptr addrspace(4) %xvar.ascast to ptr addrspace(4)
  %8 = bitcast ptr addrspace(4) %atomic-temp.ascast to ptr addrspace(4)
  call void @__atomic_load(i64 4, ptr addrspace(4) %7, ptr addrspace(4) %8, i32 0)
  br label %atomic_cont

atomic_cont:                                      ; preds = %atomic_cont, %omp.inner.for.body
  %9 = load i32, ptr addrspace(4) %atomic-temp.ascast, align 4
  %add2 = add nsw i32 %9, %6
  store i32 %add2, ptr addrspace(4) %atomic-temp1.ascast, align 4
  %10 = bitcast ptr addrspace(4) %xvar.ascast to ptr addrspace(4)
  %11 = bitcast ptr addrspace(4) %atomic-temp.ascast to ptr addrspace(4)
  %12 = bitcast ptr addrspace(4) %atomic-temp1.ascast to ptr addrspace(4)
  %call = call zeroext i1 @__atomic_compare_exchange(i64 4, ptr addrspace(4) %10, ptr addrspace(4) %11, ptr addrspace(4) %12, i32 0, i32 0)
  br i1 %call, label %atomic_exit, label %atomic_cont

atomic_exit:                                      ; preds = %atomic_cont
  br label %omp.body.continue

omp.body.continue:                                ; preds = %atomic_exit
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %13 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add3 = add nsw i32 %13, 1
  store i32 %add3, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %14 = load i32, ptr addrspace(4) %xvar.ascast, align 4
  store i32 %14, ptr addrspace(4) %res.ascast, align 4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  %15 = load i32, ptr addrspace(4) %res.ascast, align 4
  ret i32 %15
}
; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1
; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1
declare dso_local void @__atomic_load(i64, ptr addrspace(4), ptr addrspace(4), i32)
declare dso_local i1 @__atomic_compare_exchange(i64, ptr addrspace(4), ptr addrspace(4), ptr addrspace(4), i32, i32)
; Function Attrs: noinline nounwind optnone uwtable
define hidden spir_func void @test2(ptr addrspace(4) %output) #0 {
entry:
  %output.addr = alloca ptr addrspace(4), align 8
  %output.addr.ascast = addrspacecast ptr %output.addr to ptr addrspace(4)
  %res = alloca i32, align 4
  %res.ascast = addrspacecast ptr %res to ptr addrspace(4)
  %xvar = alloca i32, align 4
  %xvar.ascast = addrspacecast ptr %xvar to ptr addrspace(4)
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %i = alloca i32, align 4
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  store ptr addrspace(4) %output, ptr addrspace(4) %output.addr.ascast, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1),
    "QUAL.OMP.MAP.FROM"(ptr addrspace(4) %output.addr.ascast, ptr addrspace(4) %output.addr.ascast, i64 8, i64 34, ptr null, ptr null), ; MAP type: 34 = 0x22 = TARGET_PARAM (0x20) | FROM (0x2)
    "QUAL.OMP.MAP.FROM"(ptr addrspace(4) %res.ascast, ptr addrspace(4) %res.ascast, i64 4, i64 2, ptr null, ptr null), ; MAP type: 2 = 0x2 = FROM (0x2)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %xvar.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  store i32 10, ptr addrspace(4) %xvar.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 76, ptr addrspace(4) %.omp.ub.ascast, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %xvar.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %output.addr.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1) ]

  %2 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %2, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %4 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  %6 = load i32, ptr addrspace(4) %xvar.ascast, align 4
  %7 = load ptr addrspace(4), ptr addrspace(4) %output.addr.ascast, align 8
  %8 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom = sext i32 %8 to i64
  %arrayidx = getelementptr inbounds i32, ptr addrspace(4) %7, i64 %idxprom
  store i32 %6, ptr addrspace(4) %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add1 = add nsw i32 %9, 1
  store i32 %add1, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0, !1}
!llvm.module.flags = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 0, i32 2052, i32 51515764, !"test1", i32 3, i32 0, i32 0}
!1 = !{i32 0, i32 2052, i32 51515764, !"test2", i32 18, i32 1, i32 0}
!2 = !{i32 1, !"wchar_size", i32 4}
!3 = !{}
!4 = !{!"clang version 9.0.0"}

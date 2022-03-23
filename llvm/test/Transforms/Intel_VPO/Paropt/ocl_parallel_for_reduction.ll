; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -sroa -vpo-cfg-restructuring -vpo-paropt -simplifycfg -switch-to-offload -S -vpo-paropt-atomic-free-reduction=false -pass-remarks=vpo-paropt-transform -pass-remarks-missed=vpo-paropt-transform %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,loop-simplify,sroa,vpo-cfg-restructuring),vpo-paropt,function(simplifycfg)' -switch-to-offload -S -vpo-paropt-atomic-free-reduction=false -pass-remarks=vpo-paropt-transform -pass-remarks-missed=vpo-paropt-transform %s 2>&1 | FileCheck %s

; Original code:
; int test()
; {
;   int r1 = 0, r2 = 0, r3 = 1, r4 = -1, r5 = 0, r6 = 0, r7 = 1, r8 = 0, r9 = 0, r10 = 0;
; #pragma omp target map(r1,r2,r3,r4,r5,r6,r7,r8,r9,r10)
; #pragma omp parallel for reduction(+: r1) reduction(-: r2) reduction(*: r3) reduction(&: r4) reduction(|: r5) reduction(^: r6) reduction(&&: r7) reduction(||: r8) reduction(min: r9) reduction(max: r10)
;   for (int i = 0; i < 128; i++);
;
;   return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
; }

; CHECK-DAG: ADD reduction update of type i32 made atomic
; CHECK-DAG: SUB reduction update of type i32 made atomic
; CHECK-DAG: MUL reduction update of type i32 made atomic
; CHECK-DAG: BAND reduction update of type i32 made atomic
; CHECK-DAG: BOR reduction update of type i32 made atomic
; CHECK-DAG: BXOR reduction update of type i32 made atomic
; CHECK-DAG: MIN reduction update of type i32 made atomic
; CHECK-DAG: MAX reduction update of type i32 made atomic
; FIXME: we need runtime support for these reduction operations OR
;        we have to encode compare-exchange loop in IR.
; CHECK-DAG: AND reduction update of type i32 cannot be done using atomic API
; CHECK-DAG: OR reduction update of type i32 cannot be done using atomic API
; CHECK: Critical section was generated for reduction update(s)

; CHECK: define weak dso_local spir_kernel void @__omp_offloading_
; CHECK-DAG: call spir_func void @__kmpc_atomic_fixed4_add
; CHECK-DAG: call spir_func void @__kmpc_atomic_fixed4_add
; CHECK-DAG: call spir_func void @__kmpc_atomic_fixed4_mul
; CHECK-DAG: call spir_func void @__kmpc_atomic_fixed4_andb
; CHECK-DAG: call spir_func void @__kmpc_atomic_fixed4_orb
; CHECK-DAG: call spir_func void @__kmpc_atomic_fixed4_xor
; CHECK-DAG: call spir_func void @__kmpc_atomic_fixed4_min
; CHECK-DAG: call spir_func void @__kmpc_atomic_fixed4_max

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind uwtable
define dso_local spir_func i32 @test() #0 {
entry:
  %retval = alloca i32, align 4
  %0 = addrspacecast i32* %retval to i32 addrspace(4)*
  %r1 = alloca i32, align 4
  %1 = addrspacecast i32* %r1 to i32 addrspace(4)*
  %r2 = alloca i32, align 4
  %2 = addrspacecast i32* %r2 to i32 addrspace(4)*
  %r3 = alloca i32, align 4
  %3 = addrspacecast i32* %r3 to i32 addrspace(4)*
  %r4 = alloca i32, align 4
  %4 = addrspacecast i32* %r4 to i32 addrspace(4)*
  %r5 = alloca i32, align 4
  %5 = addrspacecast i32* %r5 to i32 addrspace(4)*
  %r6 = alloca i32, align 4
  %6 = addrspacecast i32* %r6 to i32 addrspace(4)*
  %r7 = alloca i32, align 4
  %7 = addrspacecast i32* %r7 to i32 addrspace(4)*
  %r8 = alloca i32, align 4
  %8 = addrspacecast i32* %r8 to i32 addrspace(4)*
  %r9 = alloca i32, align 4
  %9 = addrspacecast i32* %r9 to i32 addrspace(4)*
  %r10 = alloca i32, align 4
  %10 = addrspacecast i32* %r10 to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %11 = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %12 = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %13 = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %14 = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %i = alloca i32, align 4
  %15 = addrspacecast i32* %i to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %1, align 4
  store i32 0, i32 addrspace(4)* %2, align 4
  store i32 1, i32 addrspace(4)* %3, align 4
  store i32 -1, i32 addrspace(4)* %4, align 4
  store i32 0, i32 addrspace(4)* %5, align 4
  store i32 0, i32 addrspace(4)* %6, align 4
  store i32 1, i32 addrspace(4)* %7, align 4
  store i32 0, i32 addrspace(4)* %8, align 4
  store i32 0, i32 addrspace(4)* %9, align 4
  store i32 0, i32 addrspace(4)* %10, align 4
  %16 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %1, i32 addrspace(4)* %1, i64 4, i64 35), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %2, i32 addrspace(4)* %2, i64 4, i64 35), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %3, i32 addrspace(4)* %3, i64 4, i64 35), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %4, i32 addrspace(4)* %4, i64 4, i64 35), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %5, i32 addrspace(4)* %5, i64 4, i64 35), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %6, i32 addrspace(4)* %6, i64 4, i64 35), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %7, i32 addrspace(4)* %7, i64 4, i64 35), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %8, i32 addrspace(4)* %8, i64 4, i64 35), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %9, i32 addrspace(4)* %9, i64 4, i64 35), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %10, i32 addrspace(4)* %10, i64 4, i64 35), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %13), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %14), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %15), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %11), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %12) ]
  store i32 0, i32 addrspace(4)* %13, align 4
  store i32 127, i32 addrspace(4)* %14, align 4
  %17 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD"(i32 addrspace(4)* %1), "QUAL.OMP.REDUCTION.SUB"(i32 addrspace(4)* %2), "QUAL.OMP.REDUCTION.MUL"(i32 addrspace(4)* %3), "QUAL.OMP.REDUCTION.BAND"(i32 addrspace(4)* %4), "QUAL.OMP.REDUCTION.BOR"(i32 addrspace(4)* %5), "QUAL.OMP.REDUCTION.BXOR"(i32 addrspace(4)* %6), "QUAL.OMP.REDUCTION.AND"(i32 addrspace(4)* %7), "QUAL.OMP.REDUCTION.OR"(i32 addrspace(4)* %8), "QUAL.OMP.REDUCTION.MIN"(i32 addrspace(4)* %9), "QUAL.OMP.REDUCTION.MAX"(i32 addrspace(4)* %10), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %13), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %11), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %14), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %15) ]
  %18 = load i32, i32 addrspace(4)* %13, align 4
  store i32 %18, i32 addrspace(4)* %11, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %19 = load i32, i32 addrspace(4)* %11, align 4
  %20 = load i32, i32 addrspace(4)* %14, align 4
  %cmp = icmp sle i32 %19, %20
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %21 = load i32, i32 addrspace(4)* %11, align 4
  %mul = mul nsw i32 %21, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %15, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %22 = load i32, i32 addrspace(4)* %11, align 4
  %add1 = add nsw i32 %22, 1
  store i32 %add1, i32 addrspace(4)* %11, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %17) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %16) [ "DIR.OMP.END.TARGET"() ]
  %23 = load i32, i32 addrspace(4)* %1, align 4
  %24 = load i32, i32 addrspace(4)* %2, align 4
  %add2 = add nsw i32 %23, %24
  %25 = load i32, i32 addrspace(4)* %3, align 4
  %add3 = add nsw i32 %add2, %25
  %26 = load i32, i32 addrspace(4)* %4, align 4
  %add4 = add nsw i32 %add3, %26
  %27 = load i32, i32 addrspace(4)* %5, align 4
  %add5 = add nsw i32 %add4, %27
  %28 = load i32, i32 addrspace(4)* %6, align 4
  %add6 = add nsw i32 %add5, %28
  %29 = load i32, i32 addrspace(4)* %7, align 4
  %add7 = add nsw i32 %add6, %29
  %30 = load i32, i32 addrspace(4)* %8, align 4
  %add8 = add nsw i32 %add7, %30
  %31 = load i32, i32 addrspace(4)* %9, align 4
  %add9 = add nsw i32 %add8, %31
  %32 = load i32, i32 addrspace(4)* %10, align 4
  %add10 = add nsw i32 %add9, %32
  ret i32 %add10
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2052, i32 85985690, !"test", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 9.0.0"}

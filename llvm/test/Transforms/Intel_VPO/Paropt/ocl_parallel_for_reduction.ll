; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -sroa -vpo-cfg-restructuring -vpo-paropt -simplifycfg -switch-to-offload -S -vpo-paropt-atomic-free-reduction=false -pass-remarks=vpo-paropt-transform -pass-remarks-missed=vpo-paropt-transform %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,loop-simplify,sroa,vpo-cfg-restructuring),vpo-paropt,function(simplifycfg)' -switch-to-offload -S -vpo-paropt-atomic-free-reduction=false -pass-remarks=vpo-paropt-transform -pass-remarks-missed=vpo-paropt-transform %s 2>&1 | FileCheck %s

; Test src:
;
; int test() {
;   int r1 = 0, r2 = 0, r3 = 1, r4 = -1, r5 = 0, r6 = 0, r7 = 1, r8 = 0, r9 = 0,
;       r10 = 0;
; #pragma omp target map(r1, r2, r3, r4, r5, r6, r7, r8, r9, r10)
; #pragma omp parallel for reduction(+: r1) reduction(-: r2) reduction(*: r3) reduction(&: r4) reduction(|: r5) reduction(^: r6) reduction(&&: r7) reduction(||: r8) reduction(min: r9) reduction(max: r10)
;   for (int i = 0; i < 128; i++)
;     ;
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

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind optnone
define protected spir_func i32 @test() #0 {
entry:
  %retval = alloca i32, align 4
  %r1 = alloca i32, align 4
  %r2 = alloca i32, align 4
  %r3 = alloca i32, align 4
  %r4 = alloca i32, align 4
  %r5 = alloca i32, align 4
  %r6 = alloca i32, align 4
  %r7 = alloca i32, align 4
  %r8 = alloca i32, align 4
  %r9 = alloca i32, align 4
  %r10 = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %r1.ascast = addrspacecast ptr %r1 to ptr addrspace(4)
  %r2.ascast = addrspacecast ptr %r2 to ptr addrspace(4)
  %r3.ascast = addrspacecast ptr %r3 to ptr addrspace(4)
  %r4.ascast = addrspacecast ptr %r4 to ptr addrspace(4)
  %r5.ascast = addrspacecast ptr %r5 to ptr addrspace(4)
  %r6.ascast = addrspacecast ptr %r6 to ptr addrspace(4)
  %r7.ascast = addrspacecast ptr %r7 to ptr addrspace(4)
  %r8.ascast = addrspacecast ptr %r8 to ptr addrspace(4)
  %r9.ascast = addrspacecast ptr %r9 to ptr addrspace(4)
  %r10.ascast = addrspacecast ptr %r10 to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %r1.ascast, align 4
  store i32 0, ptr addrspace(4) %r2.ascast, align 4
  store i32 1, ptr addrspace(4) %r3.ascast, align 4
  store i32 -1, ptr addrspace(4) %r4.ascast, align 4
  store i32 0, ptr addrspace(4) %r5.ascast, align 4
  store i32 0, ptr addrspace(4) %r6.ascast, align 4
  store i32 1, ptr addrspace(4) %r7.ascast, align 4
  store i32 0, ptr addrspace(4) %r8.ascast, align 4
  store i32 0, ptr addrspace(4) %r9.ascast, align 4
  store i32 0, ptr addrspace(4) %r10.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 127, ptr addrspace(4) %.omp.ub.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %r1.ascast, ptr addrspace(4) %r1.ascast, i64 4, i64 35, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %r2.ascast, ptr addrspace(4) %r2.ascast, i64 4, i64 35, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %r3.ascast, ptr addrspace(4) %r3.ascast, i64 4, i64 35, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %r4.ascast, ptr addrspace(4) %r4.ascast, i64 4, i64 35, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %r5.ascast, ptr addrspace(4) %r5.ascast, i64 4, i64 35, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %r6.ascast, ptr addrspace(4) %r6.ascast, i64 4, i64 35, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %r7.ascast, ptr addrspace(4) %r7.ascast, i64 4, i64 35, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %r8.ascast, ptr addrspace(4) %r8.ascast, i64 4, i64 35, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %r9.ascast, ptr addrspace(4) %r9.ascast, i64 4, i64 35, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %r10.ascast, ptr addrspace(4) %r10.ascast, i64 4, i64 35, ptr null, ptr null),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %r1.ascast, i32 0, i32 1),
    "QUAL.OMP.REDUCTION.SUB:TYPED"(ptr addrspace(4) %r2.ascast, i32 0, i32 1),
    "QUAL.OMP.REDUCTION.MUL:TYPED"(ptr addrspace(4) %r3.ascast, i32 0, i32 1),
    "QUAL.OMP.REDUCTION.BAND:TYPED"(ptr addrspace(4) %r4.ascast, i32 0, i32 1),
    "QUAL.OMP.REDUCTION.BOR:TYPED"(ptr addrspace(4) %r5.ascast, i32 0, i32 1),
    "QUAL.OMP.REDUCTION.BXOR:TYPED"(ptr addrspace(4) %r6.ascast, i32 0, i32 1),
    "QUAL.OMP.REDUCTION.AND:TYPED"(ptr addrspace(4) %r7.ascast, i32 0, i32 1),
    "QUAL.OMP.REDUCTION.OR:TYPED"(ptr addrspace(4) %r8.ascast, i32 0, i32 1),
    "QUAL.OMP.REDUCTION.MIN:TYPED"(ptr addrspace(4) %r9.ascast, i32 0, i32 1),
    "QUAL.OMP.REDUCTION.MAX:TYPED"(ptr addrspace(4) %r10.ascast, i32 0, i32 1),
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
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %6 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add1 = add nsw i32 %6, 1
  store i32 %add1, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  %7 = load i32, ptr addrspace(4) %r1.ascast, align 4
  %8 = load i32, ptr addrspace(4) %r2.ascast, align 4
  %add2 = add nsw i32 %7, %8
  %9 = load i32, ptr addrspace(4) %r3.ascast, align 4
  %add3 = add nsw i32 %add2, %9
  %10 = load i32, ptr addrspace(4) %r4.ascast, align 4
  %add4 = add nsw i32 %add3, %10
  %11 = load i32, ptr addrspace(4) %r5.ascast, align 4
  %add5 = add nsw i32 %add4, %11
  %12 = load i32, ptr addrspace(4) %r6.ascast, align 4
  %add6 = add nsw i32 %add5, %12
  %13 = load i32, ptr addrspace(4) %r7.ascast, align 4
  %add7 = add nsw i32 %add6, %13
  %14 = load i32, ptr addrspace(4) %r8.ascast, align 4
  %add8 = add nsw i32 %add7, %14
  %15 = load i32, ptr addrspace(4) %r9.ascast, align 4
  %add9 = add nsw i32 %add8, %15
  %16 = load i32, ptr addrspace(4) %r10.ascast, align 4
  %add10 = add nsw i32 %add9, %16
  ret i32 %add10
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 53, i32 -1915577681, !"_Z4test", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}

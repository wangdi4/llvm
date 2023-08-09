; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -switch-to-offload -vpo-paropt-enable-64bit-opencl-atomics -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -vpo-paropt-enable-64bit-opencl-atomics -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck %s

; Test src:
;
; void foo() {
;   double add = 0.0, sub = 0.0, mul = 1.0;
; #pragma omp target parallel for reduction(+ : add) reduction(- : sub) reduction(* : mul)
;   for (int i = 0; i < 100; ++i)
;     ;
; }

; Check that Paropt generates atomic updates under -vpo-paropt-enable-64bit-opencl-atomics:
; CHECK: call spir_func void @__kmpc_atomic_float8_add
; CHECK: call spir_func void @__kmpc_atomic_float8_add
; CHECK: call spir_func void @__kmpc_atomic_float8_mul

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind
define protected spir_func void @foo() #0 {
entry:
  %add = alloca double, align 8
  %sub = alloca double, align 8
  %mul = alloca double, align 8
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  %add.ascast = addrspacecast ptr %add to ptr addrspace(4)
  %sub.ascast = addrspacecast ptr %sub to ptr addrspace(4)
  %mul.ascast = addrspacecast ptr %mul to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  store double 0.000000e+00, ptr addrspace(4) %add.ascast, align 8, !tbaa !8
  store double 0.000000e+00, ptr addrspace(4) %sub.ascast, align 8, !tbaa !8
  store double 1.000000e+00, ptr addrspace(4) %mul.ascast, align 8, !tbaa !8
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4, !tbaa !12
  store i32 99, ptr addrspace(4) %.omp.ub.ascast, align 4, !tbaa !12
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %add.ascast, ptr addrspace(4) %add.ascast, i64 8, i64 547, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sub.ascast, ptr addrspace(4) %sub.ascast, i64 8, i64 547, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %mul.ascast, ptr addrspace(4) %mul.ascast, i64 8, i64 547, ptr null, ptr null),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %add.ascast, double 0.000000e+00, i32 1),
    "QUAL.OMP.REDUCTION.SUB:TYPED"(ptr addrspace(4) %sub.ascast, double 0.000000e+00, i32 1),
    "QUAL.OMP.REDUCTION.MUL:TYPED"(ptr addrspace(4) %mul.ascast, double 0.000000e+00, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1) ]
  %2 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4, !tbaa !12
  store i32 %2, ptr addrspace(4) %.omp.iv.ascast, align 4, !tbaa !12
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4, !tbaa !12
  %4 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4, !tbaa !12
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4, !tbaa !12
  %mul1 = mul nsw i32 %5, 1
  %add2 = add nsw i32 0, %mul1
  store i32 %add2, ptr addrspace(4) %i.ascast, align 4, !tbaa !12
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %6 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4, !tbaa !12
  %add3 = add nsw i32 %6, 1
  store i32 %add3, ptr addrspace(4) %.omp.iv.ascast, align 4, !tbaa !12
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

attributes #0 = { convergent noinline nounwind "approx-func-fp-math"="true" "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 53, i32 -1924411035, !"_Z3foo", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!8 = !{!9, !9, i64 0}
!9 = !{!"double", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}
!12 = !{!13, !13, i64 0}
!13 = !{!"int", !10, i64 0}

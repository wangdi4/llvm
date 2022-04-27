; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S -switch-to-offload %s 2>&1 | FileCheck %s

; Test src:
;
; #pragma omp declare target
; void foo(int *x, int n);
; #pragma omp end declare target
;
; void foo(int *x, int n) {
; #pragma omp parallel
; #pragma omp for
;   for (int i = 0; i < n; i++)
;     {
; //      #pragma omp atomic
;       x[i] = x[i] + 1;
;     }
; }

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Check that warnings are emitted for the ignored constructs.
; CHECK: warning:{{.*}}'loop' construct, in a declare target function, was ignored for calls from target regions.
; CHECK: warning:{{.*}}'parallel' construct, in a declare target function, was ignored for calls from target regions.

; Check that scheduling code for parallel-for is not generated.
; CHECK-NOT: call spir_func i64 @_Z14get_local_sizej(i32 0)
; CHECK-NOT: call spir_func i64 @_Z12get_local_idj(i32 0)

; Function Attrs: noinline nounwind optnone
define hidden spir_func void @foo(i32 addrspace(4)* %x, i32 %n) #0 {
entry:
  %x.addr = alloca i32 addrspace(4)*, align 8
  %x.addr.ascast = addrspacecast i32 addrspace(4)** %x.addr to i32 addrspace(4)* addrspace(4)*
  %n.addr = alloca i32, align 4
  %n.addr.ascast = addrspacecast i32* %n.addr to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.0.ascast = addrspacecast i32* %.capture_expr.0 to i32 addrspace(4)*
  %.capture_expr.1 = alloca i32, align 4
  %.capture_expr.1.ascast = addrspacecast i32* %.capture_expr.1 to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  store i32 addrspace(4)* %x, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  store i32 %n, i32 addrspace(4)* %n.addr.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(i32 addrspace(4)* addrspace(4)* %x.addr.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %n.addr.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.capture_expr.1.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.capture_expr.0.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %1 = load i32, i32 addrspace(4)* %n.addr.ascast, align 4
  store i32 %1, i32 addrspace(4)* %.capture_expr.0.ascast, align 4
  %2 = load i32, i32 addrspace(4)* %.capture_expr.0.ascast, align 4
  %sub = sub nsw i32 %2, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, i32 addrspace(4)* %.capture_expr.1.ascast, align 4
  %3 = load i32, i32 addrspace(4)* %.capture_expr.0.ascast, align 4
  %cmp = icmp slt i32 0, %3
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  %4 = load i32, i32 addrspace(4)* %.capture_expr.1.ascast, align 4
  store i32 %4, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast) ]
  %6 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %6, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %7 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %8 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp3 = icmp sle i32 %7, %8
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %9, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, i32 addrspace(4)* %i.ascast, align 4
  %10 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %11 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %idxprom = sext i32 %11 to i64
  %ptridx = getelementptr inbounds i32, i32 addrspace(4)* %10, i64 %idxprom
  %12 = load i32, i32 addrspace(4)* %ptridx, align 4
  %add5 = add nsw i32 %12, 1
  %13 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %x.addr.ascast, align 8
  %14 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %idxprom6 = sext i32 %14 to i64
  %ptridx7 = getelementptr inbounds i32, i32 addrspace(4)* %13, i64 %idxprom6
  store i32 %add5, i32 addrspace(4)* %ptridx7, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %15 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add8 = add nsw i32 %15, 1
  store i32 %add8, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{}
!3 = !{!"clang version 10.0.0"}

; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S -switch-to-offload %s 2>&1 | FileCheck %s

; Test src:
;
; #pragma omp declare target
; void foo(int *x, int n);
; #pragma omp end declare target
;
; void foo(int *x, int n) {
; #pragma omp parallel for
;   for (int i = 0; i < n; i++)
;     {
; //      #pragma omp atomic
;       x[i] = x[i] + 1;
;     }
; }

; Check that warning is emitted for the ignored construct.
; CHECK: warning:{{.*}} do/for/loop construct, in a declare target function, was ignored for calls from target regions.

; Check that scheduling code for parallel-for is not generated.
; CHECK-NOT: call spir_func i64 @_Z14get_local_sizej(i32 0)
; CHECK-NOT: call spir_func i64 @_Z12get_local_idj(i32 0)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind optnone
define protected spir_func void @foo(ptr addrspace(4) noundef %x, i32 noundef %n) #0 {
entry:
  %x.addr = alloca ptr addrspace(4), align 8
  %n.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %x.addr.ascast = addrspacecast ptr %x.addr to ptr addrspace(4)
  %n.addr.ascast = addrspacecast ptr %n.addr to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.capture_expr.0.ascast = addrspacecast ptr %.capture_expr.0 to ptr addrspace(4)
  %.capture_expr.1.ascast = addrspacecast ptr %.capture_expr.1 to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  store ptr addrspace(4) %x, ptr addrspace(4) %x.addr.ascast, align 8
  store i32 %n, ptr addrspace(4) %n.addr.ascast, align 4
  %0 = load i32, ptr addrspace(4) %n.addr.ascast, align 4
  store i32 %0, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %1 = load i32, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %sub = sub nsw i32 %1, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr addrspace(4) %.capture_expr.1.ascast, align 4
  %2 = load i32, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %cmp = icmp slt i32 0, %2
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  %3 = load i32, ptr addrspace(4) %.capture_expr.1.ascast, align 4
  store i32 %3, ptr addrspace(4) %.omp.ub.ascast, align 4
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %x.addr.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1) ]
  %5 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %5, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %6 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %7 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp3 = icmp sle i32 %6, %7
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %8, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, ptr addrspace(4) %i.ascast, align 4
  %9 = load ptr addrspace(4), ptr addrspace(4) %x.addr.ascast, align 8
  %10 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom = sext i32 %10 to i64
  %arrayidx = getelementptr inbounds i32, ptr addrspace(4) %9, i64 %idxprom
  %11 = load i32, ptr addrspace(4) %arrayidx, align 4
  %add5 = add nsw i32 %11, 1
  %12 = load ptr addrspace(4), ptr addrspace(4) %x.addr.ascast, align 8
  %13 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom6 = sext i32 %13 to i64
  %arrayidx7 = getelementptr inbounds i32, ptr addrspace(4) %12, i64 %idxprom6
  store i32 %add5, ptr addrspace(4) %arrayidx7, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %14 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add8 = add nsw i32 %14, 1
  store i32 %add8, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!opencl.compiler.options = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"openmp-device", i32 51}
!3 = !{i32 7, !"PIC Level", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{}

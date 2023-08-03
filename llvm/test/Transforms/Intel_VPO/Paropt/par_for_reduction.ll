; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -mtriple=i686-unknown-linux-gnu -vpo-paropt-fast-reduction=false -S %s | FileCheck %s -check-prefix=CRITICAL -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -mtriple=i686-unknown-linux-gnu -vpo-paropt-fast-reduction=false -S %s | FileCheck %s -check-prefix=CRITICAL -check-prefix=ALL
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -mtriple=x86_64-unknown-linux-gnu -vpo-paropt-fast-reduction=false -S %s | FileCheck %s -check-prefix=CRITICAL -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -mtriple=x86_64-unknown-linux-gnu -vpo-paropt-fast-reduction=false -S %s | FileCheck %s -check-prefix=CRITICAL -check-prefix=ALL

; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -mtriple=i686-unknown-linux-gnu -S %s | FileCheck %s -check-prefix=FASTRED -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -mtriple=i686-unknown-linux-gnu -S %s | FileCheck %s -check-prefix=FASTRED -check-prefix=ALL
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -mtriple=x86_64-unknown-linux-gnu -S %s | FileCheck %s -check-prefix=FASTRED -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -mtriple=x86_64-unknown-linux-gnu -S %s | FileCheck %s -check-prefix=FASTRED -check-prefix=ALL

;
; It tests whether the VPOParopt can generate the call @__kmpc_critical
; and the call @__kmpc_end_critical at the end of the loop.
;
; Test source:
;
; extern double x;
;
; void foo() {
;   int i;
; #pragma omp parallel for reduction(+:x)
;   for (i = 0; i < 10; i++) {
;     x++;
;   }
; }

; ALL-NOT: call token @llvm.directive.region.entry()
; ALL-NOT: call token @llvm.directive.region.exit()

; CRITICAL: call void @__kmpc_critical
; FASTRED: call i32 @__kmpc_reduce

; ALL: store double %{{.*}}, ptr @x
; CRITICAL: call void @__kmpc_end_critical
; FASTRED: call void @__kmpc_end_reduce

; ModuleID = 'reduction.c'
source_filename = "reduction.c"

@x = external dso_local global double, align 8

define dso_local void @foo() {
entry:
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32 0, ptr %.omp.lb, align 4
  store i32 9, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr @x, double 0.000000e+00, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0) ]

  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %5 = load double, ptr @x, align 8
  %inc = fadd fast double %5, 1.000000e+00
  store double %inc, ptr @x, align 8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %6 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %6, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}


; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; The test case checks whether the omp parlalel loop is partitioned as expected
; on top of OCL.
;
; #define MAX 10000
;
; int A[MAX], B[MAX], C[MAX];
;
; void __attribute__ ((noinline)) Compute(int n)
; {
;      #pragma omp target map(tofrom: A, B, C) map(to: n)
;      {
;            int i;
;            #pragma omp parallel for
;            for (i = 1; i < n; i++) {
;               A[i] = B[i] + C[i];
;            }
;       }
; }

; CHECK: %{{.*}} = call spir_func i64 @_Z14get_local_sizej(i32 0)
; CHECK: %{{.*}} = call spir_func i64 @_Z12get_local_idj(i32 0)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

@A = external dso_local addrspace(1) global [10000 x i32], align 4
@B = external dso_local addrspace(1) global [10000 x i32], align 4
@C = external dso_local addrspace(1) global [10000 x i32], align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @Compute(i32 %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %0 = addrspacecast ptr %n.addr to ptr addrspace(4)
  %i = alloca i32, align 4
  %1 = addrspacecast ptr %i to ptr addrspace(4)
  %.omp.iv = alloca i32, align 4
  %2 = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %tmp = alloca i32, align 4
  %3 = addrspacecast ptr %tmp to ptr addrspace(4)
  %.capture_expr. = alloca i32, align 4
  %4 = addrspacecast ptr %.capture_expr. to ptr addrspace(4)
  %.capture_expr.1 = alloca i32, align 4
  %5 = addrspacecast ptr %.capture_expr.1 to ptr addrspace(4)
  %.omp.lb = alloca i32, align 4
  %6 = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub = alloca i32, align 4
  %7 = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  store i32 %n, ptr addrspace(4) %0, align 4
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @A to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @A to ptr addrspace(4)), i64 40000, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @B to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @B to ptr addrspace(4)), i64 40000, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @C to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @C to ptr addrspace(4)), i64 40000, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TO"(ptr addrspace(4) %0, ptr addrspace(4) %0, i64 4, i64 33, ptr null, ptr null), ; MAP type: 33 = 0x21 = TARGET_PARAM (0x20) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %1, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %4, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %5, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %6, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %7, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %2, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %3, i32 0, i32 1) ]

  %9 = load i32, ptr addrspace(4) %0, align 4
  store i32 %9, ptr addrspace(4) %4, align 4
  %10 = load i32, ptr addrspace(4) %4, align 4
  %sub = sub nsw i32 %10, 1
  %sub2 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub2, 1
  %div = sdiv i32 %add, 1
  %sub3 = sub nsw i32 %div, 1
  store i32 %sub3, ptr addrspace(4) %5, align 4
  %11 = load i32, ptr addrspace(4) %4, align 4
  %cmp = icmp slt i32 1, %11
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, ptr addrspace(4) %6, align 4
  %12 = load i32, ptr addrspace(4) %5, align 4
  store i32 %12, ptr addrspace(4) %7, align 4
  %13 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %6, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %2, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %7, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %1, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @A to ptr addrspace(4)), i32 0, i64 10000),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @B to ptr addrspace(4)), i32 0, i64 10000),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @C to ptr addrspace(4)), i32 0, i64 10000) ]

  %14 = load i32, ptr addrspace(4) %6, align 4
  store i32 %14, ptr addrspace(4) %2, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %15 = load i32, ptr addrspace(4) %2, align 4
  %16 = load i32, ptr addrspace(4) %7, align 4
  %cmp4 = icmp sle i32 %15, %16
  br i1 %cmp4, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %17 = load i32, ptr addrspace(4) %2, align 4
  %mul = mul nsw i32 %17, 1
  %add5 = add nsw i32 1, %mul
  store i32 %add5, ptr addrspace(4) %1, align 4
  %18 = load i32, ptr addrspace(4) %1, align 4
  %idxprom = sext i32 %18 to i64
  %arrayidx = getelementptr inbounds [10000 x i32], ptr addrspace(4) addrspacecast (ptr addrspace(1) @B to ptr addrspace(4)), i64 0, i64 %idxprom
  %19 = load i32, ptr addrspace(4) %arrayidx, align 4
  %20 = load i32, ptr addrspace(4) %1, align 4
  %idxprom6 = sext i32 %20 to i64
  %arrayidx7 = getelementptr inbounds [10000 x i32], ptr addrspace(4) addrspacecast (ptr addrspace(1) @C to ptr addrspace(4)), i64 0, i64 %idxprom6
  %21 = load i32, ptr addrspace(4) %arrayidx7, align 4
  %add8 = add nsw i32 %19, %21
  %22 = load i32, ptr addrspace(4) %1, align 4
  %idxprom9 = sext i32 %22 to i64
  %arrayidx10 = getelementptr inbounds [10000 x i32], ptr addrspace(4) addrspacecast (ptr addrspace(1) @A to ptr addrspace(4)), i64 0, i64 %idxprom9
  store i32 %add8, ptr addrspace(4) %arrayidx10, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %23 = load i32, ptr addrspace(4) %2, align 4
  %add11 = add nsw i32 %23, 1
  store i32 %add11, ptr addrspace(4) %2, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %13) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2052, i32 85985690, !"Compute", i32 7, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 9.0.0"}


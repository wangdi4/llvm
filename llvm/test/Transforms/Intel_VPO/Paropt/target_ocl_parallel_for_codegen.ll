; RUN: opt < %s -switch-to-offload=true -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-cfg-restructuring -vpo-paropt  -S | FileCheck %s

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

target triple = "spir64"
target device_triples = "spir64"

@A = external global [10000 x i32], align 4
@B = external global [10000 x i32], align 4
@C = external global [10000 x i32], align 4

; Function Attrs: noinline nounwind optnone uwtable
define spir_func void @Compute(i32 %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr. = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.MAP.TOFROM"([10000 x i32]* @A), "QUAL.OMP.MAP.TOFROM"([10000 x i32]* @B), "QUAL.OMP.MAP.TOFROM"([10000 x i32]* @C), "QUAL.OMP.MAP.TO"(i32* %n.addr), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.1), "QUAL.OMP.PRIVATE"(i32* %.omp.stride), "QUAL.OMP.PRIVATE"(i32* %.omp.is_last) ]
  %1 = load i32, i32* %n.addr, align 4
  store i32 %1, i32* %.capture_expr., align 4
  %2 = load i32, i32* %.capture_expr., align 4
  %sub = sub nsw i32 %2, 1
  %sub2 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub2, 1
  %div = sdiv i32 %add, 1
  %sub3 = sub nsw i32 %div, 1
  store i32 %sub3, i32* %.capture_expr.1, align 4
  %3 = load i32, i32* %.capture_expr., align 4
  %cmp = icmp slt i32 1, %3
  br i1 %cmp, label %omp.precond.then, label %DIR.OMP.END.TARGET.5

omp.precond.then:                                 ; preds = %DIR.OMP.TARGET.1
  store i32 0, i32* %.omp.lb, align 4
  %4 = load i32, i32* %.capture_expr.1, align 4
  store volatile i32 %4, i32* %.omp.ub, align 4
  store i32 1, i32* %.omp.stride, align 4
  store i32 0, i32* %.omp.is_last, align 4
  br label %DIR.OMP.PARALLEL.LOOP.3

DIR.OMP.PARALLEL.LOOP.3:                          ; preds = %omp.precond.then
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.NUM_THREADS"(i32 16), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"([10000 x i32]* @B), "QUAL.OMP.SHARED"([10000 x i32]* @C), "QUAL.OMP.SHARED"([10000 x i32]* @A) ]
  %6 = call i8* @llvm.launder.invariant.group.p0i8(i8* bitcast ([10000 x i32]* @B to i8*))
  %7 = bitcast i8* %6 to [10000 x i32]*
  %8 = call i8* @llvm.launder.invariant.group.p0i8(i8* bitcast ([10000 x i32]* @C to i8*))
  %9 = bitcast i8* %8 to [10000 x i32]*
  %10 = call i8* @llvm.launder.invariant.group.p0i8(i8* bitcast ([10000 x i32]* @A to i8*))
  %11 = bitcast i8* %10 to [10000 x i32]*
  %12 = load i32, i32* %.omp.lb, align 4
  store volatile i32 %12, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.PARALLEL.LOOP.3
  %13 = load volatile i32, i32* %.omp.iv, align 4
  %14 = load volatile i32, i32* %.omp.ub, align 4
  %cmp4 = icmp sle i32 %13, %14
  br i1 %cmp4, label %omp.inner.for.body, label %omp.loop.exit

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %15 = load volatile i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %15, 1
  %add5 = add nsw i32 1, %mul
  store i32 %add5, i32* %i, align 4
  %16 = load i32, i32* %i, align 4
  %idxprom = sext i32 %16 to i64
  %arrayidx = getelementptr inbounds [10000 x i32], [10000 x i32]* %7, i64 0, i64 %idxprom
  %17 = load i32, i32* %arrayidx, align 4
  %18 = load i32, i32* %i, align 4
  %idxprom6 = sext i32 %18 to i64
  %arrayidx7 = getelementptr inbounds [10000 x i32], [10000 x i32]* %9, i64 0, i64 %idxprom6
  %19 = load i32, i32* %arrayidx7, align 4
  %add8 = add nsw i32 %17, %19
  %20 = load i32, i32* %i, align 4
  %idxprom9 = sext i32 %20 to i64
  %arrayidx10 = getelementptr inbounds [10000 x i32], [10000 x i32]* %11, i64 0, i64 %idxprom9
  store i32 %add8, i32* %arrayidx10, align 4
  %21 = load volatile i32, i32* %.omp.iv, align 4
  %add11 = add nsw i32 %21, 1
  store volatile i32 %add11, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.TARGET.5

DIR.OMP.END.TARGET.5:                             ; preds = %omp.loop.exit, %DIR.OMP.TARGET.1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: inaccessiblememonly nounwind speculatable
declare i8* @llvm.launder.invariant.group.p0i8(i8*) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { inaccessiblememonly nounwind speculatable }

!llvm.module.flags = !{!0, !1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{}
!3 = !{!"cl_doubles"}
!4 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 12022194aae7a486b5537a51f5eb9d5f116e2ab1)"}

; CHECK: %{{.*}} = call i64 @_Z14get_local_sizej(i32 0)
; CHECK: %{{.*}} = call i64 @_Z12get_local_idj(i32 0)

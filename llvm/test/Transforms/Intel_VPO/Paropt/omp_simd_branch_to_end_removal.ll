; RUN: opt -O0 -paropt=11 -S %s | FileCheck %s

; Verify that removeBranchesFromBeginToEndDirective is called under -fiopenmp-simd
;
; Compiling at -fiopenmp-simd sets the paropt mode to "11" which turns on
; the ParPrepare, ParTrans, and OmpVec bits, but does not turn on OmpPar bit.
; At Paropt Prepare routine addBranchToEndDirective adds an aritifial branch to
; the end of OMP loop. At Paropt Transform removeBranchesFromBeginToEndDirective
; removes the artificial branch. This test verifies that the branch is removed
; even when the OmpPar bit is not set.

; C source
; void foo(double *A) {
;   int i;
;   #pragma omp simd
;   for (i=0; i<100; i++) {
;     A[i] = 1.0;
;   }
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(double* %A) #0 {
entry:
  %A.addr = alloca double*, align 8
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store double* %A, double** %A.addr, align 8
  store i32 99, i32* %.omp.ub, align 4

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV"(i32* %i, i32 1), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub) ]

; After addBranchToEndDirective the code above becomes something like this
;   %end.dir.temp = alloca i1, align 1
;  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV"(i32* %i, i32 1), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.OPERAND.ADDR"(i32* %i, i32** %i.addr), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]
;  %temp.load = load volatile i1, i1* %end.dir.temp, align 1
;  %cmp2 = icmp ne i1 %temp.load, false
;  br i1 %cmp2, label %omp.loop.exit.split, label %1
;
; Check that the artificial branch is removed
; CHECK-NOT: %temp.load = load volatile i1, i1* %end.dir.temp, align 1

  store i32 0, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond


omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %1 = load i32, i32* %.omp.iv, align 4
  %2 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %1, %2
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %3 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %3, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %4 = load double*, double** %A.addr, align 8
  %5 = load i32, i32* %i, align 4
  %idxprom = sext i32 %5 to i64
  %ptridx = getelementptr inbounds double, double* %4, i64 %idxprom
  store double 1.000000e+00, double* %ptridx, align 8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %6 = load i32, i32* %.omp.iv, align 4
  %add1 = add nsw i32 %6, 1
  store i32 %add1, i32* %.omp.iv, align 4
  %7 = load i32, i32* %i, align 4
  %add2 = add nsw i32 %7, 1
  store i32 %add2, i32* %i, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}

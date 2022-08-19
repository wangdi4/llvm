; RUN: opt -O0 -paropt=11 -S %s | FileCheck %s

; Test src:
;
; void foo(double *A) {
;   int i;
; #pragma omp simd
;   for (i = 0; i < 100; i++) {
;     A[i] = 1.0;
;   }
; }

; Verify that removeBranchesFromBeginToEndDirective is called under -fiopenmp-simd
;
; Compiling at -fiopenmp-simd sets the paropt mode to "11" which turns on
; the ParPrepare, ParTrans, and OmpVec bits, but does not turn on OmpPar bit.
; At Paropt Prepare routine addBranchToEndDirective adds an aritifial branch to
; the end of OMP loop. At Paropt Transform removeBranchesFromBeginToEndDirective
; removes the artificial branch. This test verifies that the branch is removed
; even when the OmpPar bit is not set.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(ptr noundef %A) #0 {
entry:
  %A.addr = alloca ptr, align 8
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store ptr %A, ptr %A.addr, align 8
  store i32 99, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i, i32 0, i32 1, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0) ]

; After addBranchToEndDirective the code above becomes something like this
;
;  %end.dir.temp = alloca i1, align 1
;  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i, i32 0, i32 1, i32 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0), "QUAL.OMP.OPERAND.ADDR"(ptr %i, ptr %i.addr), "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]
;  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
;  %cmp4 = icmp ne i1 %temp.load, false
;  br i1 %cmp4, label %omp.loop.exit.split, label %1
;
; Check that the artificial branch is removed
; CHECK-NOT: %temp.load = load volatile i1, ptr %end.dir.temp, align 1

  store i32 0, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %1 = load i32, ptr %.omp.iv, align 4
  %2 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %1, %2
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %3 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %3, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %4 = load ptr, ptr %A.addr, align 8
  %5 = load i32, ptr %i, align 4
  %idxprom = sext i32 %5 to i64
  %arrayidx = getelementptr inbounds double, ptr %4, i64 %idxprom
  store double 1.000000e+00, ptr %arrayidx, align 8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %6 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %6, 1
  store i32 %add1, ptr %.omp.iv, align 4
  %7 = load i32, ptr %i, align 4
  %add2 = add nsw i32 %7, 1
  store i32 %add2, ptr %i, align 4
  br label %omp.inner.for.cond, !llvm.loop !5

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

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.vectorize.enable", i1 true}

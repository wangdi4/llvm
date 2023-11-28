; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-wrncollection -analyze -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(print<vpo-wrncollection>)' -debug -S %s 2>&1 | FileCheck %s

; Test src:
;
; void foo() {
;   float a, b;
; #pragma omp simd if (a > b) private(a) lastprivate(b)
;   for (int i = 0; i < 10000; ++i) {
;     a = i;
;     b = i;
;   }
; }

; CHECK:  %{{.*}} = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.IF"(i1 %{{.*}}), "QUAL.OMP.PRIVATE:TYPED"(ptr %a, float 0.000000e+00, i32 1), "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %b, float 0.000000e+00, i32 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i, i32 0, i32 1, i32 1) ]

; CHECK: BEGIN SIMD ID=1 {
; CHECK:  IF:   %cmp = fcmp fast ogt float %{{.*}}, %{{.*}}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %a = alloca float, align 4
  %b = alloca float, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 9999, ptr %.omp.ub, align 4
  %0 = load float, ptr %a, align 4
  %1 = load float, ptr %b, align 4
  %cmp = fcmp fast ogt float %0, %1
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.IF"(i1 %cmp),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %a, float 0.000000e+00, i32 1),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %b, float 0.000000e+00, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  store i32 0, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %DIR.OMP.SIMD.2
  %3 = load i32, ptr %.omp.iv, align 4
  %4 = load i32, ptr %.omp.ub, align 4
  %cmp1 = icmp sle i32 %3, %4
  br i1 %cmp1, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %6 = load i32, ptr %i, align 4
  %conv = sitofp i32 %6 to float
  store float %conv, ptr %a, align 4
  %7 = load i32, ptr %i, align 4
  %conv2 = sitofp i32 %7 to float
  store float %conv2, ptr %b, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4
  %add3 = add nsw i32 %8, 1
  store i32 %add3, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond, !llvm.loop !5

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %omp.loop.exit
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

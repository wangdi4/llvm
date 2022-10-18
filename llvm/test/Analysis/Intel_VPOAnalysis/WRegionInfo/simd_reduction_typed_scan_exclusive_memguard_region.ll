; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s

; Test src:

; int x = 0;
; int a[10];
; int p[10];
;
; void foo() {
; #pragma omp simd reduction(inscan , +: x)
;   for (int i = 0; i < 10; i++) {
;     p[i] = x;
; #pragma omp scan exclusive(x)
;     x += a[i];
;   }
; }

; The IR is hand-modified because typed EXCLUSIVE clause is not supported.

; CHECK: BEGIN SIMD ID=1 {
; ...
; CHECK:   REDUCTION-INSCAN maps: (1: EXCLUSIVE)
; CHECK:   REDUCTION clause (size=1): (ADD: ptr @x, TYPED (TYPE: i32, NUM_ELEMENTS: i32 1) INSCAN<1>)
; CHECK:   LINEAR clause (size=1): IV(ptr %i, TYPED (TYPE: i32, NUM_ELEMENTS: i32 1), i32 1)
; ...
; CHECK:   BEGIN GUARD.MEM.MOTION ID=2 {
; ...
; CHECK:    BEGIN SCAN ID=3 {
; ...
; CHECK:      INSCAN-REDUCTION maps: (1: ADD)
; CHECK:      INCLUSIVE clause: UNSPECIFIED
; CHECK:      EXCLUSIVE clause (size=1): (ptr @x INSCAN<1>)
; ...
; CHECK:    } END SCAN ID=3

; CHECK:  } END GUARD.MEM.MOTION ID=2
;
; CHECK: } END SIMD ID=1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local global [10 x i32] zeroinitializer, align 16
@p = dso_local global [10 x i32] zeroinitializer, align 16
@x = dso_local global i32 0, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 9, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.REDUCTION.ADD:INSCAN.TYPED"(ptr @x, i32 0, i32 1, i64 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i, i32 0, i32 1, i32 1) ]
  store i32 0, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %1 = load i32, ptr %.omp.iv, align 4
  %2 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %1, %2
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %guard.start = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(),
    "QUAL.OMP.LIVEIN"(i32* @x) ]
  %3 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %3, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %4 = load i32, ptr @x, align 4
  %5 = load i32, ptr %i, align 4
  %idxprom = sext i32 %5 to i64
  %arrayidx = getelementptr inbounds [10 x i32], ptr @p, i64 0, i64 %idxprom
  store i32 %4, ptr %arrayidx, align 4
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(),
    "QUAL.OMP.EXCLUSIVE"(ptr @x, i64 1) ]
  fence acq_rel
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.SCAN"() ]
  %7 = load i32, ptr %i, align 4
  %idxprom1 = sext i32 %7 to i64
  %arrayidx2 = getelementptr inbounds [10 x i32], ptr @a, i64 0, i64 %idxprom1
  %8 = load i32, ptr %arrayidx2, align 4
  %9 = load i32, ptr @x, align 4
  %add3 = add nsw i32 %9, %8
  store i32 %add3, ptr @x, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, ptr %.omp.iv, align 4
  call void @llvm.directive.region.exit(token %guard.start) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  %add4 = add nsw i32 %10, 1
  store i32 %add4, ptr %.omp.iv, align 4
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

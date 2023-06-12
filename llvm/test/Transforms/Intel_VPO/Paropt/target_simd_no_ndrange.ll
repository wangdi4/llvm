; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -S %s | FileCheck %s
; Test src:

; #include <stdio.h>
; void test_target_simd() {
;   #pragma omp target
;   #pragma omp simd
;   for (int i0 = 0 ; i0 < 10 ; i0++ ) {
;     printf("%d\n", i0); // Should print each number from 0 to 9 only once.
;   }
; }

; Check that ND-Range parallelization is not enabled for SIMD loops in target regions.
; CHECK-NOT: "QUAL.OMP.OFFLOAD.NDRANGE"
; CHECK-NOT: "QUAL.OMP.OFFLOAD.KNOWN.NDRANGE"

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define dso_local void @test_target_simd() {
entry:
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i0 = alloca i32, align 4
  store i32 9, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i0, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i0, i32 0, i32 1, i32 1) ]

  store i32 0, ptr %.omp.iv, align 4
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
  store i32 %add, ptr %i0, align 4
  %5 = load i32, ptr %i0, align 4
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %5)
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
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local i32 @printf(ptr noundef, ...)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 64773, i32 3825469, !"_Z16test_target_simd", i32 3, i32 0, i32 0, i32 0}

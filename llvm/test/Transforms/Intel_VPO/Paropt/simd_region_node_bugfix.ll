; This file tests qualifies the fix for bug identified by CMPLRS-52186

; The bug was caused due to incorrect handling of basic-blocks associated
; with a WRegion within the 'regularizeOMPLoop() function, in preparation of
; generation for privatized vars.This test in particular asserts that
; we generate correct private copy of the array.

; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes="function(vpo-cfg-restructuring),vpo-paropt" -S %s | FileCheck %s


; CHECK: %arr.priv = alloca [1000 x i32]

; #include <omp.h>
; void baz(int i);
; int arr[1000];
; void foo() {
;   int i;
;   int x;
; #pragma omp simd private(arr, x)
;   for (i = 0; i < 1000; i++)
;     arr[i] = arr[i] + 10;
; }


; ModuleID = 'simd_region_node_bugfix.c'
source_filename = "simd_region_node_bugfix.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = dso_local global [1000 x i32] zeroinitializer, align 16

define dso_local void @foo() {
entry:
  %i = alloca i32, align 4
  %x = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32 999, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr @arr, i32 0, i64 1000),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %x, i32 0, i32 1),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i, i32 0, i32 1, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0) ]

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
  %4 = load i32, ptr %i, align 4
  %idxprom = sext i32 %4 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @arr, i64 0, i64 %idxprom
  %5 = load i32, ptr %arrayidx, align 4
  %add1 = add nsw i32 %5, 10
  %6 = load i32, ptr %i, align 4
  %idxprom2 = sext i32 %6 to i64
  %arrayidx3 = getelementptr inbounds [1000 x i32], ptr @arr, i64 0, i64 %idxprom2
  store i32 %add1, ptr %arrayidx3, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, ptr %.omp.iv, align 4
  %add4 = add nsw i32 %7, 1
  store i32 %add4, ptr %.omp.iv, align 4
  %8 = load i32, ptr %i, align 4
  %add5 = add nsw i32 %8, 1
  store i32 %add5, ptr %i, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]

  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

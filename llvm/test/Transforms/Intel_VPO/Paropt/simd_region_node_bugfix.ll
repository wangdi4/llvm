; This file tests qualifies the fix for bug identified by CMPLRS-52186

; The bug was caused due to incorrect handling of basic-blocks associated
; with a WRegion within the 'regularizeOMPLoop() function, in preparation of
; generation for privatized vars.This test in particular asserts that
; we generate correct private copy of the array.


; RUN: opt -vpo-cfg-restructuring -vpo-paropt -vpo-cfg-restructuring -vpo-paropt -S < %s | FileCheck %s


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


; ModuleID = 't2.c'
source_filename = "t2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = common dso_local global [1000 x i32] zeroinitializer, align 16

; Function Attrs: noinline nounwind uwtable
define dso_local void @foo() #0 {
entry:
  %i = alloca i32, align 4
  %x = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32 999, i32* %.omp.ub, align 4
  call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", [1000 x i32]* @arr)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", i32* %x)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.NORMALIZED.IV", i32* %.omp.iv)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.NORMALIZED.UB", i32* %.omp.ub)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  store i32 0, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %0 = load i32, i32* %.omp.iv, align 4
  %1 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %0, %1
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %2 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %2, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %3 = load i32, i32* %i, align 4
  %idxprom = sext i32 %3 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @arr, i64 0, i64 %idxprom
  %4 = load i32, i32* %arrayidx, align 4
  %add1 = add nsw i32 %4, 10
  %5 = load i32, i32* %i, align 4
  %idxprom2 = sext i32 %5 to i64
  %arrayidx3 = getelementptr inbounds [1000 x i32], [1000 x i32]* @arr, i64 0, i64 %idxprom2
  store i32 %add1, i32* %arrayidx3, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %6 = load i32, i32* %.omp.iv, align 4
  %add4 = add nsw i32 %6, 1
  store i32 %add4, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  ret void
}


; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #2

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0"}

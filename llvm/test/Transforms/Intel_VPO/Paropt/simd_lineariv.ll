; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:

; #include <stdio.h>
; int main() {
;   int indexK = 0;
;
; //#pragma omp parallel num_threads(4)
;   {
; #pragma omp simd
;     for (indexK = 0; indexK < 2; indexK++) {
;       printf("%d\n", indexK);
;     }
;   }
;   printf("%d\n", indexK);
; }
;
; Check for allocation of the private copy of %indexK
; CHECK: %indexK.linear.iv = alloca i32, align 4
; Check that the private copy of %indexK is used on the SIMD directive.
; CHECK: %{{.+}} = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %indexK.linear.iv, i32 0, i32 1, i32 1), {{.*}} ]
; Check that %indexK is replaced by its private copy inside the region.
; CHECK: store i32 %add2, ptr %indexK.linear.iv, align 4
; Check for the copy-out of the private copy of %indexK back to the original, after the loop.
; CHECK: [[FINAL_VAL:%[^ ]+]] = load i32, ptr %indexK.linear.iv
; CHECK: store i32 [[FINAL_VAL]], ptr %indexK
;
; ModuleID = 'simd_lineariv.cpp'
source_filename = "simd_lineariv.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %indexK = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  store i32 0, ptr %indexK, align 4
  store i32 1, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %indexK, i32 0, i32 1, i32 1),
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
  store i32 %add, ptr %indexK, align 4
  %4 = load i32, ptr %indexK, align 4
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %4) #1
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %5 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %5, 1
  store i32 %add1, ptr %.omp.iv, align 4
  %6 = load i32, ptr %indexK, align 4
  %add2 = add nsw i32 %6, 1
  store i32 %add2, ptr %indexK, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]

  %7 = load i32, ptr %indexK, align 4
  %call3 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %7)
  %8 = load i32, ptr %retval, align 4
  ret i32 %8
}

declare token @llvm.directive.region.entry() 

declare void @llvm.directive.region.exit(token)

declare dso_local i32 @printf(ptr noundef, ...)

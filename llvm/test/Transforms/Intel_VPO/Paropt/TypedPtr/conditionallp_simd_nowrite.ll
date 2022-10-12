; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; short x;
; void foo() {
; #pragma omp simd lastprivate(conditional : x)
;  for (int i = 0; i < 10; i++)
;   ;
; }

; Check that we initialize the lastprivate copy of x with its incoming value,
; and store the final value of the lastprivate copy of x back to the original.

; CHECK: %x.lpriv = alloca i16, align 2
; CHECK: [[XLOAD:%.+]] = load i16, i16* @x, align 2
; CHECK: store i16 [[XLOAD]], i16* %x.lpriv, align 2

; CHECK: "QUAL.OMP.LASTPRIVATE:CONDITIONAL"(i16* %x.lpriv)
; CHECK: "DIR.OMP.END.SIMD"()

; CHECK: [[XLP_LOAD:%.+]] = load i16, i16* %x.lpriv, align 2
; CHECK: store i16 [[XLP_LOAD]], i16* @x, align 2

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = dso_local global i16 0, align 2

define dso_local void @_Z3foov() {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 9, i32* %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LASTPRIVATE:CONDITIONAL"(i16* @x),
    "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub),
    "QUAL.OMP.LINEAR:IV"(i32* %i, i32 1) ]

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
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %4 = load i32, i32* %.omp.iv, align 4
  %add1 = add nsw i32 %4, 1
  store i32 %add1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

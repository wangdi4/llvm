; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s
;
; Check parsing of the reproduciable and unconstrained modifier on order clause.
;
; Test src:
;
; void foo() {
;   #pragma omp parallel loop order(concurrent)
;   for (int i=0; i<1000; ++i) {}
;   #pragma omp parallel loop order(reproducible:concurrent)
;   for (int i=0; i<1000; ++i) {}
;   #pragma omp parallel loop order(unconstrained:concurrent)
;   for (int i=0; i<1000; ++i) {}
; }

; CHECK: BEGIN PARALLEL ID=1 {
; CHECK: BEGIN GENERICLOOP ID=2 {
; CHECK: LOOPORDER: UNCONSTRAINED CONCURRENT
; CHECK: } END GENERICLOOP ID=2
; CHECK: } END PARALLEL ID=1

; CHECK: BEGIN PARALLEL ID=3 {
; CHECK: BEGIN GENERICLOOP ID=4 {
; CHECK: LOOPORDER: REPRODUCIBLE CONCURRENT
; CHECK: } END GENERICLOOP ID=4
; CHECK: } END PARALLEL ID=3

; CHECK: BEGIN PARALLEL ID=5 {
; CHECK: BEGIN GENERICLOOP ID=6 {
; CHECK: LOOPORDER: UNCONSTRAINED CONCURRENT
; CHECK: } END GENERICLOOP ID=6
; CHECK: } END PARALLEL ID=5

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo()  {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp2 = alloca i32, align 4
  %.omp.iv3 = alloca i32, align 4
  %.omp.lb4 = alloca i32, align 4
  %.omp.ub5 = alloca i32, align 4
  %i9 = alloca i32, align 4
  %tmp17 = alloca i32, align 4
  %.omp.iv18 = alloca i32, align 4
  %.omp.lb19 = alloca i32, align 4
  %.omp.ub20 = alloca i32, align 4
  %i24 = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]

  store i32 0, ptr %.omp.lb, align 4
  store i32 999, ptr %.omp.ub, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.ORDER.CONCURRENT"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  %2 = load i32, ptr %.omp.lb, align 4
  store i32 %2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, ptr %.omp.iv, align 4
  %4 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
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
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.GENERICLOOP"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv3, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb4, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub5, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i9, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp2, i32 0, i32 1) ]

  store i32 0, ptr %.omp.lb4, align 4
  store i32 999, ptr %.omp.ub5, align 4
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.ORDER.CONCURRENT:REPRODUCIBLE"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv3, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb4, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub5, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i9, i32 0, i32 1) ]

  %9 = load i32, ptr %.omp.lb4, align 4
  store i32 %9, ptr %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.cond6:                              ; preds = %omp.inner.for.inc13, %omp.loop.exit
  %10 = load i32, ptr %.omp.iv3, align 4
  %11 = load i32, ptr %.omp.ub5, align 4
  %cmp7 = icmp sle i32 %10, %11
  br i1 %cmp7, label %omp.inner.for.body8, label %omp.inner.for.end15

omp.inner.for.body8:                              ; preds = %omp.inner.for.cond6
  %12 = load i32, ptr %.omp.iv3, align 4
  %mul10 = mul nsw i32 %12, 1
  %add11 = add nsw i32 0, %mul10
  store i32 %add11, ptr %i9, align 4
  br label %omp.body.continue12

omp.body.continue12:                              ; preds = %omp.inner.for.body8
  br label %omp.inner.for.inc13

omp.inner.for.inc13:                              ; preds = %omp.body.continue12
  %13 = load i32, ptr %.omp.iv3, align 4
  %add14 = add nsw i32 %13, 1
  store i32 %add14, ptr %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.end15:                              ; preds = %omp.inner.for.cond6
  br label %omp.loop.exit16

omp.loop.exit16:                                  ; preds = %omp.inner.for.end15
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.GENERICLOOP"() ]

  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.PARALLEL"() ]

  %14 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv18, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb19, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub20, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i24, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp17, i32 0, i32 1) ]

  store i32 0, ptr %.omp.lb19, align 4
  store i32 999, ptr %.omp.ub20, align 4
  %15 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.ORDER.CONCURRENT:UNCONSTRAINED"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv18, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb19, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub20, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i24, i32 0, i32 1) ]

  %16 = load i32, ptr %.omp.lb19, align 4
  store i32 %16, ptr %.omp.iv18, align 4
  br label %omp.inner.for.cond21

omp.inner.for.cond21:                             ; preds = %omp.inner.for.inc28, %omp.loop.exit16
  %17 = load i32, ptr %.omp.iv18, align 4
  %18 = load i32, ptr %.omp.ub20, align 4
  %cmp22 = icmp sle i32 %17, %18
  br i1 %cmp22, label %omp.inner.for.body23, label %omp.inner.for.end30

omp.inner.for.body23:                             ; preds = %omp.inner.for.cond21
  %19 = load i32, ptr %.omp.iv18, align 4
  %mul25 = mul nsw i32 %19, 1
  %add26 = add nsw i32 0, %mul25
  store i32 %add26, ptr %i24, align 4
  br label %omp.body.continue27

omp.body.continue27:                              ; preds = %omp.inner.for.body23
  br label %omp.inner.for.inc28

omp.inner.for.inc28:                              ; preds = %omp.body.continue27
  %20 = load i32, ptr %.omp.iv18, align 4
  %add29 = add nsw i32 %20, 1
  store i32 %add29, ptr %.omp.iv18, align 4
  br label %omp.inner.for.cond21

omp.inner.for.end30:                              ; preds = %omp.inner.for.cond21
  br label %omp.loop.exit31

omp.loop.exit31:                                  ; preds = %omp.inner.for.end30
  call void @llvm.directive.region.exit(token %15) [ "DIR.OMP.END.GENERICLOOP"() ]

  call void @llvm.directive.region.exit(token %14) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

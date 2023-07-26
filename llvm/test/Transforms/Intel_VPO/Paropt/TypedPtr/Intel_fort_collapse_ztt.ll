; INTEL_CUSTOMIZATION
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-paropt-loop-collapse -S <%s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-paropt-loop-collapse)' -S <%s | FileCheck %s
;
; Loop collapsing ZTT check was zero-extending the normalized i32 UBs instead of sign-extending them.
; For zero-trip loops the normalized UB is negative, so sign-extension is required.
;
; Fortran test case:
;        subroutine foo(N,M)
;          integer N, M, i, j
;        !$omp parallel do private(i,j) collapse(2)
;          do i = 2 , N
;             do j = 2 , M
;               call bar()
;             enddo
;          enddo
;        end subroutine foo

; For ZTTs loop collapsing must sign-extend (instead of zero-extend) the normalized UBs below.
; CHECK: [[UB1LOAD:%[^ ]+]] = load i32, i32* %omp.pdo.norm.ub
; CHECK: [[UB1SEXT:%[^ ]+]] = sext i32 [[UB1LOAD]] to i64
; CHECK: icmp slt i64 [[UB1SEXT]], 0

; CHECK: [[UB2LOAD:%[^ ]+]] = load i32, i32* %do.norm.ub
; CHECK: [[UB2SEXT:%[^ ]+]] = sext i32 [[UB2LOAD]] to i64
; CHECK: icmp slt i64 [[UB2SEXT]], 0

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo_(i32* noalias dereferenceable(4) %"foo_$N$argptr", i32* noalias dereferenceable(4) %"foo_$M$argptr") {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8
  %"foo_$N$locptr" = alloca i32*, align 8
  %"foo_$M$locptr" = alloca i32*, align 8
  %"foo_$J" = alloca i32, align 8
  %"foo_$I" = alloca i32, align 8
  store i32* %"foo_$N$argptr", i32** %"foo_$N$locptr", align 1
  %"foo_$N.1" = load i32*, i32** %"foo_$N$locptr", align 1
  store i32* %"foo_$M$argptr", i32** %"foo_$M$locptr", align 1
  %"foo_$M.2" = load i32*, i32** %"foo_$M$locptr", align 1
  %do.start = alloca i32, align 4
  store i32 2, i32* %do.start, align 4
  %do.end = alloca i32, align 4
  %"foo_$M.2_fetch.13" = load i32, i32* %"foo_$M.2", align 1
  store i32 %"foo_$M.2_fetch.13", i32* %do.end, align 4
  %do.step = alloca i32, align 4
  store i32 1, i32* %do.step, align 4
  %do.norm.lb = alloca i32, align 4
  store i32 0, i32* %do.norm.lb, align 4
  %do.norm.ub = alloca i32, align 4
  %do.end_fetch.14 = load i32, i32* %do.end, align 4
  %do.start_fetch.15 = load i32, i32* %do.start, align 4
  %sub.2 = sub nsw i32 %do.end_fetch.14, %do.start_fetch.15
  %do.step_fetch.16 = load i32, i32* %do.step, align 4
  %div.2 = sdiv i32 %sub.2, %do.step_fetch.16
  store i32 %div.2, i32* %do.norm.ub, align 4
  %do.norm.iv = alloca i32, align 4
  %omp.pdo.start = alloca i32, align 4
  store i32 2, i32* %omp.pdo.start, align 4
  %omp.pdo.end = alloca i32, align 4
  %"foo_$N.1_fetch.3" = load i32, i32* %"foo_$N.1", align 1
  store i32 %"foo_$N.1_fetch.3", i32* %omp.pdo.end, align 4
  %omp.pdo.step = alloca i32, align 4
  store i32 1, i32* %omp.pdo.step, align 4
  %omp.pdo.norm.iv = alloca i32, align 4
  %omp.pdo.norm.lb = alloca i32, align 4
  store i32 0, i32* %omp.pdo.norm.lb, align 4
  %omp.pdo.norm.ub = alloca i32, align 4
  %omp.pdo.end_fetch.4 = load i32, i32* %omp.pdo.end, align 4
  %omp.pdo.start_fetch.5 = load i32, i32* %omp.pdo.start, align 4
  %sub.1 = sub nsw i32 %omp.pdo.end_fetch.4, %omp.pdo.start_fetch.5
  %omp.pdo.step_fetch.6 = load i32, i32* %omp.pdo.step, align 4
  %div.1 = sdiv i32 %sub.1, %omp.pdo.step_fetch.6
  store i32 %div.1, i32* %omp.pdo.norm.ub, align 4
  br label %bb_new6

bb_new6:                                          ; preds = %alloca_0
  br label %DIR.OMP.PARALLEL.LOOP.1

DIR.OMP.PARALLEL.LOOP.1:                          ; preds = %bb_new6

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.COLLAPSE"(i32 2),
    "QUAL.OMP.SHARED"(i32* %"foo_$M.2"),
    "QUAL.OMP.SHARED"(i32* %"foo_$N.1"),
    "QUAL.OMP.PRIVATE"(i32* %"foo_$J"),
    "QUAL.OMP.PRIVATE"(i32* %"foo_$I"),
    "QUAL.OMP.SHARED"(i32* %do.step),
    "QUAL.OMP.SHARED"(i32* %do.start),
    "QUAL.OMP.SHARED"(i32* %omp.pdo.step),
    "QUAL.OMP.SHARED"(i32* %omp.pdo.start),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %do.norm.lb),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %omp.pdo.norm.lb),
    "QUAL.OMP.NORMALIZED.IV"(i32* %omp.pdo.norm.iv, i32* %do.norm.iv),
    "QUAL.OMP.NORMALIZED.UB"(i32* %omp.pdo.norm.ub, i32* %do.norm.ub) ]
  br label %DIR.OMP.PARALLEL.LOOP.2

DIR.OMP.PARALLEL.LOOP.2:                          ; preds = %DIR.OMP.PARALLEL.LOOP.1
  %omp.pdo.norm.lb_fetch.7 = load i32, i32* %omp.pdo.norm.lb, align 4
  store i32 %omp.pdo.norm.lb_fetch.7, i32* %omp.pdo.norm.iv, align 4
  br label %omp.pdo.cond3

omp.pdo.cond3:                                    ; preds = %do.epilog11, %DIR.OMP.PARALLEL.LOOP.2
  %omp.pdo.norm.iv_fetch.8 = load i32, i32* %omp.pdo.norm.iv, align 4
  %omp.pdo.norm.ub_fetch.9 = load i32, i32* %omp.pdo.norm.ub, align 4
  %rel.1 = icmp sle i32 %omp.pdo.norm.iv_fetch.8, %omp.pdo.norm.ub_fetch.9
  br i1 %rel.1, label %omp.pdo.body4, label %omp.pdo.epilog5

omp.pdo.body4:                                    ; preds = %omp.pdo.cond3
  %omp.pdo.norm.iv_fetch.10 = load i32, i32* %omp.pdo.norm.iv, align 4
  %omp.pdo.step_fetch.11 = load i32, i32* %omp.pdo.step, align 4
  %mul.1 = mul nsw i32 %omp.pdo.norm.iv_fetch.10, %omp.pdo.step_fetch.11
  %omp.pdo.start_fetch.12 = load i32, i32* %omp.pdo.start, align 4
  %add.1 = add nsw i32 %mul.1, %omp.pdo.start_fetch.12
  store i32 %add.1, i32* %"foo_$I", align 8
  %do.norm.lb_fetch.17 = load i32, i32* %do.norm.lb, align 4
  store i32 %do.norm.lb_fetch.17, i32* %do.norm.iv, align 4
  br label %do.cond9

do.cond9:                                         ; preds = %do.body10, %omp.pdo.body4
  %do.norm.iv_fetch.18 = load i32, i32* %do.norm.iv, align 4
  %do.norm.ub_fetch.19 = load i32, i32* %do.norm.ub, align 4
  %rel.2 = icmp sle i32 %do.norm.iv_fetch.18, %do.norm.ub_fetch.19
  br i1 %rel.2, label %do.body10, label %do.epilog11

do.body10:                                        ; preds = %do.cond9
  %do.norm.iv_fetch.20 = load i32, i32* %do.norm.iv, align 4
  %do.step_fetch.21 = load i32, i32* %do.step, align 4
  %mul.2 = mul nsw i32 %do.norm.iv_fetch.20, %do.step_fetch.21
  %do.start_fetch.22 = load i32, i32* %do.start, align 4
  %add.2 = add nsw i32 %mul.2, %do.start_fetch.22
  store i32 %add.2, i32* %"foo_$J", align 8
  call void @bar_.void()
  %do.norm.iv_fetch.23 = load i32, i32* %do.norm.iv, align 4
  %add.3 = add nsw i32 %do.norm.iv_fetch.23, 1
  store i32 %add.3, i32* %do.norm.iv, align 4
  br label %do.cond9

do.epilog11:                                      ; preds = %do.cond9
  %omp.pdo.norm.iv_fetch.24 = load i32, i32* %omp.pdo.norm.iv, align 4
  %add.4 = add nsw i32 %omp.pdo.norm.iv_fetch.24, 1
  store i32 %add.4, i32* %omp.pdo.norm.iv, align 4
  br label %omp.pdo.cond3

omp.pdo.epilog5:                                  ; preds = %omp.pdo.cond3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.3

DIR.OMP.END.PARALLEL.LOOP.3:                      ; preds = %omp.pdo.epilog5
  ret void
}

declare token @llvm.directive.region.entry()

define internal void @bar_.void() {
wrap_start18:
  call void (...) @bar_()
  ret void
}

declare void @llvm.directive.region.exit(token)
declare void @bar_(...)
; end INTEL_CUSTOMIZATION

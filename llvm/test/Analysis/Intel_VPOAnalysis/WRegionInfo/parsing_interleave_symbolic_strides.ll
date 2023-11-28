; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -debug-only=wrninfo -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -debug-only=wrninfo -S %s 2>&1 | FileCheck %s
;
; Current FE doesn't yet emit IR for the interleave construct.
; This LIT test is hand-modified from a LIT test for the tile construct (parsing_tile_symbolic_sizes.ll).
;
; INTEL_CUSTOMIZATION
; Test src:
;
;   subroutine test(M,N,S,T)
;   integer   :: i, j, M, S
;   integer*8 :: N, T
;   !$omp tile sizes(S, T, 2)   ! hand-modified for !$ompx interleave strides(S, T, 2)
;   do i = 1, M
;     do j = 1, 100
;       do k = 1, N
;         call bar(i,j,k)
;       end do
;     end do
;   end do
;   end subroutine
; end INTEL_CUSTOMIZATION

; Case with symbolic strides of different data types:
; Check the WRN for INTERLEAVE STRIDES(S,T,2) where S is i32 and T is i64

; CHECK: BEGIN INTERLEAVE ID=1 {
; CHECK:  LIVEIN clause (size=3): (ptr %do.norm.lb4) (ptr %do.norm.lb) (ptr %omp.pdo.norm.lb)
; CHECK:  STRIDES clause (size=3): (ptr %"test_$S.3") (ptr %"test_$T.4") (i32 2)
; CHECK:   IV clause:   %omp.pdo.norm.iv{{.*}};   %do.norm.iv{{.*}};   %do.norm.iv6
; CHECK:   UB clause:   %omp.pdo.norm.ub{{.*}};   %do.norm.ub{{.*}};   %do.norm.ub5

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @test_(ptr noalias dereferenceable(4) %"test_$M$argptr", ptr noalias dereferenceable(8) %"test_$N$argptr", ptr noalias dereferenceable(4) %"test_$S$argptr", ptr noalias dereferenceable(8) %"test_$T$argptr") {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8
  %"test_$M$locptr" = alloca ptr, align 8
  %"test_$N$locptr" = alloca ptr, align 8
  %"test_$S$locptr" = alloca ptr, align 8
  %"test_$T$locptr" = alloca ptr, align 8
  %"test_$K" = alloca i32, align 8
  %"test_$J" = alloca i32, align 8
  %"test_$I" = alloca i32, align 8
  store ptr %"test_$M$argptr", ptr %"test_$M$locptr", align 1
  %"test_$M.1" = load ptr, ptr %"test_$M$locptr", align 1
  store ptr %"test_$N$argptr", ptr %"test_$N$locptr", align 1
  %"test_$N.2" = load ptr, ptr %"test_$N$locptr", align 1
  store ptr %"test_$S$argptr", ptr %"test_$S$locptr", align 1
  %"test_$S.3" = load ptr, ptr %"test_$S$locptr", align 1
  store ptr %"test_$T$argptr", ptr %"test_$T$locptr", align 1
  %"test_$T.4" = load ptr, ptr %"test_$T$locptr", align 1
  %do.start = alloca i32, align 4
  store i32 1, ptr %do.start, align 4
  %do.end = alloca i32, align 4
  store i32 100, ptr %do.end, align 4
  %do.step = alloca i32, align 4
  store i32 1, ptr %do.step, align 4
  %do.norm.lb = alloca i32, align 4
  store i32 0, ptr %do.norm.lb, align 4
  %do.norm.ub = alloca i32, align 4
  %do.end_fetch.16 = load i32, ptr %do.end, align 4
  %do.start_fetch.17 = load i32, ptr %do.start, align 4
  %sub.3 = sub nsw i32 %do.end_fetch.16, %do.start_fetch.17
  %do.step_fetch.18 = load i32, ptr %do.step, align 4
  %add.3 = add nsw i32 %sub.3, %do.step_fetch.18
  %do.step_fetch.19 = load i32, ptr %do.step, align 4
  %div.2 = sdiv i32 %add.3, %do.step_fetch.19
  %sub.4 = sub nsw i32 %div.2, 1
  store i32 %sub.4, ptr %do.norm.ub, align 4
  %do.norm.iv = alloca i32, align 4
  %do.start1 = alloca i32, align 4
  store i32 1, ptr %do.start1, align 4
  %do.end2 = alloca i32, align 4
  %"test_$N.2_fetch.26" = load i64, ptr %"test_$N.2", align 1
  %int_sext = trunc i64 %"test_$N.2_fetch.26" to i32
  store i32 %int_sext, ptr %do.end2, align 4
  %do.step3 = alloca i32, align 4
  store i32 1, ptr %do.step3, align 4
  %do.norm.lb4 = alloca i32, align 4
  store i32 0, ptr %do.norm.lb4, align 4
  %do.norm.ub5 = alloca i32, align 4
  %do.end_fetch.27 = load i32, ptr %do.end2, align 4
  %do.start_fetch.28 = load i32, ptr %do.start1, align 4
  %sub.5 = sub nsw i32 %do.end_fetch.27, %do.start_fetch.28
  %do.step_fetch.29 = load i32, ptr %do.step3, align 4
  %add.5 = add nsw i32 %sub.5, %do.step_fetch.29
  %do.step_fetch.30 = load i32, ptr %do.step3, align 4
  %div.3 = sdiv i32 %add.5, %do.step_fetch.30
  %sub.6 = sub nsw i32 %div.3, 1
  store i32 %sub.6, ptr %do.norm.ub5, align 4
  %do.norm.iv6 = alloca i32, align 4
  %omp.pdo.start = alloca i32, align 4
  store i32 1, ptr %omp.pdo.start, align 4
  %omp.pdo.end = alloca i32, align 4
  %"test_$M.1_fetch.5" = load i32, ptr %"test_$M.1", align 1
  store i32 %"test_$M.1_fetch.5", ptr %omp.pdo.end, align 4
  %omp.pdo.step = alloca i32, align 4
  store i32 1, ptr %omp.pdo.step, align 4
  %omp.pdo.norm.iv = alloca i32, align 4
  %omp.pdo.norm.lb = alloca i32, align 4
  store i32 0, ptr %omp.pdo.norm.lb, align 4
  %omp.pdo.norm.ub = alloca i32, align 4
  %omp.pdo.end_fetch.6 = load i32, ptr %omp.pdo.end, align 4
  %omp.pdo.start_fetch.7 = load i32, ptr %omp.pdo.start, align 4
  %sub.1 = sub nsw i32 %omp.pdo.end_fetch.6, %omp.pdo.start_fetch.7
  %omp.pdo.step_fetch.8 = load i32, ptr %omp.pdo.step, align 4
  %add.1 = add nsw i32 %sub.1, %omp.pdo.step_fetch.8
  %omp.pdo.step_fetch.9 = load i32, ptr %omp.pdo.step, align 4
  %div.1 = sdiv i32 %add.1, %omp.pdo.step_fetch.9
  %sub.2 = sub nsw i32 %div.1, 1
  store i32 %sub.2, ptr %omp.pdo.norm.ub, align 4
  br label %bb_new6

bb_new6:  ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTERLEAVE"(), ; was "DIR.OMP.TILE"
    "QUAL.OMP.STRIDES"(ptr %"test_$S.3", ptr %"test_$T.4", i32 2),  ; was "QUAL.OMP.SIZES"
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %omp.pdo.norm.iv, i32 0, ptr %do.norm.iv, i32 0, ptr %do.norm.iv6, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %omp.pdo.norm.ub, i32 0, ptr %do.norm.ub, i32 0, ptr %do.norm.ub5, i32 0),
    "QUAL.OMP.LIVEIN"(ptr %do.norm.lb4),
    "QUAL.OMP.LIVEIN"(ptr %do.norm.lb),
    "QUAL.OMP.LIVEIN"(ptr %omp.pdo.norm.lb) ]
  %omp.pdo.norm.lb_fetch.10 = load i32, ptr %omp.pdo.norm.lb, align 4
  store i32 %omp.pdo.norm.lb_fetch.10, ptr %omp.pdo.norm.iv, align 4
  br label %omp.pdo.cond3

omp.pdo.cond3:  ; preds = %do.epilog11, %bb_new6
  %omp.pdo.norm.iv_fetch.11 = load i32, ptr %omp.pdo.norm.iv, align 4
  %omp.pdo.norm.ub_fetch.12 = load i32, ptr %omp.pdo.norm.ub, align 4
  %rel.1 = icmp sle i32 %omp.pdo.norm.iv_fetch.11, %omp.pdo.norm.ub_fetch.12
  br i1 %rel.1, label %omp.pdo.body4, label %omp.pdo.epilog5

omp.pdo.body4:  ; preds = %omp.pdo.cond3
  %omp.pdo.norm.iv_fetch.13 = load i32, ptr %omp.pdo.norm.iv, align 4
  %omp.pdo.step_fetch.14 = load i32, ptr %omp.pdo.step, align 4
  %mul.1 = mul nsw i32 %omp.pdo.norm.iv_fetch.13, %omp.pdo.step_fetch.14
  %omp.pdo.start_fetch.15 = load i32, ptr %omp.pdo.start, align 4
  %add.2 = add nsw i32 %mul.1, %omp.pdo.start_fetch.15
  store i32 %add.2, ptr %"test_$I", align 8
  %do.norm.lb_fetch.20 = load i32, ptr %do.norm.lb, align 4
  store i32 %do.norm.lb_fetch.20, ptr %do.norm.iv, align 4
  br label %do.cond9

do.cond9:  ; preds = %omp.pdo.body4, %do.epilog15
  %do.norm.iv_fetch.21 = load i32, ptr %do.norm.iv, align 4
  %do.norm.ub_fetch.22 = load i32, ptr %do.norm.ub, align 4
  %rel.2 = icmp sle i32 %do.norm.iv_fetch.21, %do.norm.ub_fetch.22
  br i1 %rel.2, label %do.body10, label %do.epilog11

do.body10:  ; preds = %do.cond9
  %do.norm.iv_fetch.23 = load i32, ptr %do.norm.iv, align 4
  %do.step_fetch.24 = load i32, ptr %do.step, align 4
  %mul.2 = mul nsw i32 %do.norm.iv_fetch.23, %do.step_fetch.24
  %do.start_fetch.25 = load i32, ptr %do.start, align 4
  %add.4 = add nsw i32 %mul.2, %do.start_fetch.25
  store i32 %add.4, ptr %"test_$J", align 8
  %do.norm.lb4_fetch.31 = load i32, ptr %do.norm.lb4, align 4
  store i32 %do.norm.lb4_fetch.31, ptr %do.norm.iv6, align 4
  br label %do.cond13

do.cond13:  ; preds = %do.body10, %do.body14
  %do.norm.iv6_fetch.32 = load i32, ptr %do.norm.iv6, align 4
  %do.norm.ub5_fetch.33 = load i32, ptr %do.norm.ub5, align 4
  %rel.3 = icmp sle i32 %do.norm.iv6_fetch.32, %do.norm.ub5_fetch.33
  br i1 %rel.3, label %do.body14, label %do.epilog15

do.body14:  ; preds = %do.cond13
  %do.norm.iv6_fetch.34 = load i32, ptr %do.norm.iv6, align 4
  %do.step3_fetch.35 = load i32, ptr %do.step3, align 4
  %mul.3 = mul nsw i32 %do.norm.iv6_fetch.34, %do.step3_fetch.35
  %do.start1_fetch.36 = load i32, ptr %do.start1, align 4
  %add.6 = add nsw i32 %mul.3, %do.start1_fetch.36
  store i32 %add.6, ptr %"test_$K", align 8
  call void @bar_.t0p.t0p.t0p(ptr %"test_$I", ptr %"test_$J", ptr %"test_$K")
  %do.norm.iv6_fetch.37 = load i32, ptr %do.norm.iv6, align 4
  %add.7 = add nsw i32 %do.norm.iv6_fetch.37, 1
  store i32 %add.7, ptr %do.norm.iv6, align 4
  br label %do.cond13

do.epilog15:  ; preds = %do.cond13
  %do.norm.iv_fetch.38 = load i32, ptr %do.norm.iv, align 4
  %add.8 = add nsw i32 %do.norm.iv_fetch.38, 1
  store i32 %add.8, ptr %do.norm.iv, align 4
  br label %do.cond9

do.epilog11:  ; preds = %do.cond9
  %omp.pdo.norm.iv_fetch.39 = load i32, ptr %omp.pdo.norm.iv, align 4
  %add.9 = add nsw i32 %omp.pdo.norm.iv_fetch.39, 1
  store i32 %add.9, ptr %omp.pdo.norm.iv, align 4
  br label %omp.pdo.cond3

omp.pdo.epilog5:  ; preds = %omp.pdo.cond3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.INTERLEAVE"() ]
  ret void

}

declare token @llvm.directive.region.entry()

define internal void @bar_.t0p.t0p.t0p(ptr %arg0, ptr %arg1, ptr %arg2) {
wrap_start22:
  call void (...) @bar_(ptr %arg0, ptr %arg1, ptr %arg2)
  ret void

}

declare void @llvm.directive.region.exit(token %0)

declare void @bar_(...)

; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-wrncollection -analyze -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -debug -S %s 2>&1 | FileCheck %s
;
; INTEL_CUSTOMIZATION
; Test src:
;   subroutine test(M,N,S,T)
;   integer   :: i, j, M, S
;   integer*8 :: N, T
;   !$omp tile sizes(S, T, 2)
;   do i = 1, M
;     do j = 1, 100
;       do k = 1, N
;         call bar(i,j,k)
;       end do
;     end do
;   end do
;   end subroutine
;
; end INTEL_CUSTOMIZATION
; Case with symbolic sizes of different data types:
; Check the WRN for TILE SIZES(S,T,2) where S is i32 and T is i64
;
; CHECK: BEGIN TILE ID=1 {
; CHECK:   FIRSTPRIVATE clause (size=3): (i64* %do.norm.lb8) (i64* %do.norm.lb) (i64* %omp.pdo.norm.lb)
; CHECK:   SIZES clause (size=3): (i32 %"test_$S.3_fetch.5") (i64 %"test_$T.4_fetch.6") (i32 2)
; CHECK:   IV clause:   %omp.pdo.norm.iv{{.*}};   %do.norm.iv{{.*}};   %do.norm.iv11
; CHECK:   UB clause:   %omp.pdo.norm.ub{{.*}};   %do.norm.ub{{.*}};   %do.norm.ub9

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define void @test_(i32* dereferenceable(4) %"test_$M$argptr", i64* dereferenceable(8) %"test_$N$argptr", i32* dereferenceable(4) %"test_$S$argptr", i64* dereferenceable(8) %"test_$T$argptr") #0 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8
  %"test_$M$locptr" = alloca i32*, align 8
  %"test_$N$locptr" = alloca i64*, align 8
  %"test_$S$locptr" = alloca i32*, align 8
  %"test_$T$locptr" = alloca i64*, align 8
  %"test_$K" = alloca i32, align 8
  %"test_$J" = alloca i32, align 8
  %"test_$I" = alloca i32, align 8
  store i32* %"test_$M$argptr", i32** %"test_$M$locptr", align 1
  %"test_$M.1" = load i32*, i32** %"test_$M$locptr", align 1
  store i64* %"test_$N$argptr", i64** %"test_$N$locptr", align 1
  %"test_$N.2" = load i64*, i64** %"test_$N$locptr", align 1
  store i32* %"test_$S$argptr", i32** %"test_$S$locptr", align 1
  %"test_$S.3" = load i32*, i32** %"test_$S$locptr", align 1
  store i64* %"test_$T$argptr", i64** %"test_$T$locptr", align 1
  %"test_$T.4" = load i64*, i64** %"test_$T$locptr", align 1
  %do.start = alloca i32, align 4
  store i32 1, i32* %do.start, align 1
  %do.end = alloca i32, align 4
  store i32 100, i32* %do.end, align 1
  %do.step = alloca i32, align 4
  store i32 1, i32* %do.step, align 1
  %do.norm.lb = alloca i64, align 8
  store i64 0, i64* %do.norm.lb, align 1
  %do.norm.ub = alloca i64, align 8
  %do.end_fetch.17 = load i32, i32* %do.end, align 1
  %do.start_fetch.18 = load i32, i32* %do.start, align 1
  %sub.2 = sub nsw i32 %do.end_fetch.17, %do.start_fetch.18
  %do.step_fetch.19 = load i32, i32* %do.step, align 1
  %div.2 = sdiv i32 %sub.2, %do.step_fetch.19
  %int_sext1 = sext i32 %div.2 to i64
  store i64 %int_sext1, i64* %do.norm.ub, align 1
  %do.norm.iv = alloca i64, align 8
  %do.start4 = alloca i32, align 4
  store i32 1, i32* %do.start4, align 1
  %do.end5 = alloca i32, align 4
  %"test_$N.2_fetch.26" = load i64, i64* %"test_$N.2", align 1
  %int_sext6 = trunc i64 %"test_$N.2_fetch.26" to i32
  store i32 %int_sext6, i32* %do.end5, align 1
  %do.step7 = alloca i32, align 4
  store i32 1, i32* %do.step7, align 1
  %do.norm.lb8 = alloca i64, align 8
  store i64 0, i64* %do.norm.lb8, align 1
  %do.norm.ub9 = alloca i64, align 8
  %do.end_fetch.27 = load i32, i32* %do.end5, align 1
  %do.start_fetch.28 = load i32, i32* %do.start4, align 1
  %sub.3 = sub nsw i32 %do.end_fetch.27, %do.start_fetch.28
  %do.step_fetch.29 = load i32, i32* %do.step7, align 1
  %div.3 = sdiv i32 %sub.3, %do.step_fetch.29
  %int_sext10 = sext i32 %div.3 to i64
  store i64 %int_sext10, i64* %do.norm.ub9, align 1
  %do.norm.iv11 = alloca i64, align 8
  %"test_$S.3_fetch.5" = load i32, i32* %"test_$S.3", align 1
  %"test_$T.4_fetch.6" = load i64, i64* %"test_$T.4", align 1
  %omp.pdo.start = alloca i32, align 4
  store i32 1, i32* %omp.pdo.start, align 1
  %omp.pdo.end = alloca i32, align 4
  %"test_$M.1_fetch.7" = load i32, i32* %"test_$M.1", align 1
  store i32 %"test_$M.1_fetch.7", i32* %omp.pdo.end, align 1
  %omp.pdo.step = alloca i32, align 4
  store i32 1, i32* %omp.pdo.step, align 1
  %omp.pdo.norm.iv = alloca i64, align 8
  %omp.pdo.norm.lb = alloca i64, align 8
  store i64 0, i64* %omp.pdo.norm.lb, align 1
  %omp.pdo.norm.ub = alloca i64, align 8
  %omp.pdo.end_fetch.8 = load i32, i32* %omp.pdo.end, align 1
  %omp.pdo.start_fetch.9 = load i32, i32* %omp.pdo.start, align 1
  %sub.1 = sub nsw i32 %omp.pdo.end_fetch.8, %omp.pdo.start_fetch.9
  %omp.pdo.step_fetch.10 = load i32, i32* %omp.pdo.step, align 1
  %div.1 = sdiv i32 %sub.1, %omp.pdo.step_fetch.10
  %int_sext14 = sext i32 %div.1 to i64
  store i64 %int_sext14, i64* %omp.pdo.norm.ub, align 1
  br label %bb_new6

bb_new6:                                          ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(), "QUAL.OMP.SIZES"(i32 %"test_$S.3_fetch.5", i64 %"test_$T.4_fetch.6", i32 2), "QUAL.OMP.FIRSTPRIVATE"(i64* %do.norm.lb8), "QUAL.OMP.FIRSTPRIVATE"(i64* %do.norm.lb), "QUAL.OMP.FIRSTPRIVATE"(i64* %omp.pdo.norm.lb), "QUAL.OMP.NORMALIZED.IV"(i64* %omp.pdo.norm.iv, i64* %do.norm.iv, i64* %do.norm.iv11), "QUAL.OMP.NORMALIZED.UB"(i64* %omp.pdo.norm.ub, i64* %do.norm.ub, i64* %do.norm.ub9) ]
  %omp.pdo.norm.lb_fetch.11 = load i64, i64* %omp.pdo.norm.lb, align 1
  store i64 %omp.pdo.norm.lb_fetch.11, i64* %omp.pdo.norm.iv, align 1
  br label %omp.pdo.cond3

omp.pdo.cond3:                                    ; preds = %do.epilog11, %bb_new6
  %omp.pdo.norm.iv_fetch.12 = load i64, i64* %omp.pdo.norm.iv, align 1
  %omp.pdo.norm.ub_fetch.13 = load i64, i64* %omp.pdo.norm.ub, align 1
  %rel.1 = icmp sle i64 %omp.pdo.norm.iv_fetch.12, %omp.pdo.norm.ub_fetch.13
  br i1 %rel.1, label %omp.pdo.body4, label %omp.pdo.epilog5

omp.pdo.body4:                                    ; preds = %omp.pdo.cond3
  %omp.pdo.norm.iv_fetch.14 = load i64, i64* %omp.pdo.norm.iv, align 1
  %int_sext = trunc i64 %omp.pdo.norm.iv_fetch.14 to i32
  %omp.pdo.step_fetch.15 = load i32, i32* %omp.pdo.step, align 1
  %mul.1 = mul nsw i32 %int_sext, %omp.pdo.step_fetch.15
  %omp.pdo.start_fetch.16 = load i32, i32* %omp.pdo.start, align 1
  %add.1 = add nsw i32 %mul.1, %omp.pdo.start_fetch.16
  store i32 %add.1, i32* %"test_$I", align 1
  %do.norm.lb_fetch.20 = load i64, i64* %do.norm.lb, align 1
  store i64 %do.norm.lb_fetch.20, i64* %do.norm.iv, align 1
  br label %do.cond9

do.cond9:                                         ; preds = %do.epilog15, %omp.pdo.body4
  %do.norm.iv_fetch.21 = load i64, i64* %do.norm.iv, align 1
  %do.norm.ub_fetch.22 = load i64, i64* %do.norm.ub, align 1
  %rel.2 = icmp sle i64 %do.norm.iv_fetch.21, %do.norm.ub_fetch.22
  br i1 %rel.2, label %do.body10, label %do.epilog11

do.body10:                                        ; preds = %do.cond9
  %do.norm.iv_fetch.23 = load i64, i64* %do.norm.iv, align 1
  %int_sext3 = trunc i64 %do.norm.iv_fetch.23 to i32
  %do.step_fetch.24 = load i32, i32* %do.step, align 1
  %mul.2 = mul nsw i32 %int_sext3, %do.step_fetch.24
  %do.start_fetch.25 = load i32, i32* %do.start, align 1
  %add.2 = add nsw i32 %mul.2, %do.start_fetch.25
  store i32 %add.2, i32* %"test_$J", align 1
  %do.norm.lb8_fetch.30 = load i64, i64* %do.norm.lb8, align 1
  store i64 %do.norm.lb8_fetch.30, i64* %do.norm.iv11, align 1
  br label %do.cond13

do.cond13:                                        ; preds = %do.body14, %do.body10
  %do.norm.iv11_fetch.31 = load i64, i64* %do.norm.iv11, align 1
  %do.norm.ub9_fetch.32 = load i64, i64* %do.norm.ub9, align 1
  %rel.3 = icmp sle i64 %do.norm.iv11_fetch.31, %do.norm.ub9_fetch.32
  br i1 %rel.3, label %do.body14, label %do.epilog15

do.body14:                                        ; preds = %do.cond13
  %do.norm.iv11_fetch.33 = load i64, i64* %do.norm.iv11, align 1
  %int_sext13 = trunc i64 %do.norm.iv11_fetch.33 to i32
  %do.step7_fetch.34 = load i32, i32* %do.step7, align 1
  %mul.3 = mul nsw i32 %int_sext13, %do.step7_fetch.34
  %do.start4_fetch.35 = load i32, i32* %do.start4, align 1
  %add.3 = add nsw i32 %mul.3, %do.start4_fetch.35
  store i32 %add.3, i32* %"test_$K", align 1
  call void @bar_.t0p.t0p.t0p(i32* %"test_$I", i32* %"test_$J", i32* %"test_$K")
  %do.norm.iv11_fetch.36 = load i64, i64* %do.norm.iv11, align 1
  %add.4 = add nsw i64 %do.norm.iv11_fetch.36, 1
  store i64 %add.4, i64* %do.norm.iv11, align 1
  br label %do.cond13

do.epilog15:                                      ; preds = %do.cond13
  %do.norm.iv_fetch.37 = load i64, i64* %do.norm.iv, align 1
  %add.5 = add nsw i64 %do.norm.iv_fetch.37, 1
  store i64 %add.5, i64* %do.norm.iv, align 1
  br label %do.cond9

do.epilog11:                                      ; preds = %do.cond9
  %omp.pdo.norm.iv_fetch.38 = load i64, i64* %omp.pdo.norm.iv, align 1
  %add.6 = add nsw i64 %omp.pdo.norm.iv_fetch.38, 1
  store i64 %add.6, i64* %omp.pdo.norm.iv, align 1
  br label %omp.pdo.cond3

omp.pdo.epilog5:                                  ; preds = %omp.pdo.cond3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TILE"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: noinline nounwind optnone uwtable
define internal void @bar_.t0p.t0p.t0p(i32* %arg0, i32* %arg1, i32* %arg2) #2 {
wrap_start22:
  call void (...) @bar_(i32* %arg0, i32* %arg1, i32* %arg2)
  ret void
}

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #1

declare void @bar_(...)

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
attributes #2 = { noinline nounwind optnone uwtable "frame-pointer"="all" "intel-lang"="fortran" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

!omp_offload.info = !{}
!llvm.module.flags = !{!0}

!0 = !{i32 7, !"openmp", i32 50}

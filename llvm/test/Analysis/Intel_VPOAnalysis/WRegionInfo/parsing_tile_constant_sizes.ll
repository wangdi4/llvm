; REQUIRES: asserts
; RUN: opt -opaque-pointers=1 -enable-new-pm=0 -vpo-cfg-restructuring -vpo-wrncollection -analyze -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers=1 -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -debug -S %s 2>&1 | FileCheck %s
;
; INTEL_CUSTOMIZATION
; Test src:
;   subroutine test()
;   integer :: i, j
;   !$omp tile sizes(4, 8)
;   do i = 1, 100
;     do j = 1, 48
;       call bar(i,j)
;     end do
;   end do
;   end subroutine
;
; end INTEL_CUSTOMIZATION
; Simple case with constant sizes:
; Check the WRN for TILE SIZES(4,8)
;
; CHECK: BEGIN TILE ID=1 {
; CHECK:   LIVEIN clause (size=2): (ptr %do.norm.lb) (ptr %omp.pdo.norm.lb)
; CHECK:   SIZES clause (size=2): (i32 4) (i32 8)
; CHECK:   IV clause:   %omp.pdo.norm.iv = alloca i32,{{.*}};   %do.norm.iv = alloca i32,
; CHECK:   UB clause:   %omp.pdo.norm.ub = alloca i32,{{.*}};   %do.norm.ub = alloca i32,

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define void @test_() #0 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8, !llfort.type_idx !1
  %"test_$J" = alloca i32, align 8, !llfort.type_idx !2
  %"test_$I" = alloca i32, align 8, !llfort.type_idx !3
  %do.start = alloca i32, align 4, !llfort.type_idx !4
  store i32 1, ptr %do.start, align 4
  %do.end = alloca i32, align 4, !llfort.type_idx !4
  store i32 48, ptr %do.end, align 4
  %do.step = alloca i32, align 4, !llfort.type_idx !4
  store i32 1, ptr %do.step, align 4
  %do.norm.lb = alloca i32, align 4, !llfort.type_idx !4
  store i32 0, ptr %do.norm.lb, align 4
  %do.norm.ub = alloca i32, align 4, !llfort.type_idx !4
  %do.end_fetch.10 = load i32, ptr %do.end, align 4
  %do.start_fetch.11 = load i32, ptr %do.start, align 4
  %sub.2 = sub nsw i32 %do.end_fetch.10, %do.start_fetch.11
  %do.step_fetch.12 = load i32, ptr %do.step, align 4
  %div.2 = sdiv i32 %sub.2, %do.step_fetch.12
  store i32 %div.2, ptr %do.norm.ub, align 4
  %do.norm.iv = alloca i32, align 4, !llfort.type_idx !4
  %omp.pdo.start = alloca i32, align 4, !llfort.type_idx !4
  store i32 1, ptr %omp.pdo.start, align 4
  %omp.pdo.end = alloca i32, align 4, !llfort.type_idx !4
  store i32 100, ptr %omp.pdo.end, align 4
  %omp.pdo.step = alloca i32, align 4, !llfort.type_idx !4
  store i32 1, ptr %omp.pdo.step, align 4
  %omp.pdo.norm.iv = alloca i32, align 4, !llfort.type_idx !4
  %omp.pdo.norm.lb = alloca i32, align 4, !llfort.type_idx !4
  store i32 0, ptr %omp.pdo.norm.lb, align 4
  %omp.pdo.norm.ub = alloca i32, align 4, !llfort.type_idx !4
  %omp.pdo.end_fetch.1 = load i32, ptr %omp.pdo.end, align 4
  %omp.pdo.start_fetch.2 = load i32, ptr %omp.pdo.start, align 4
  %sub.1 = sub nsw i32 %omp.pdo.end_fetch.1, %omp.pdo.start_fetch.2
  %omp.pdo.step_fetch.3 = load i32, ptr %omp.pdo.step, align 4
  %div.1 = sdiv i32 %sub.1, %omp.pdo.step_fetch.3
  store i32 %div.1, ptr %omp.pdo.norm.ub, align 4
  br label %bb_new6

omp.pdo.cond3:                                    ; preds = %bb_new6, %do.epilog11
  %omp.pdo.norm.iv_fetch.5 = load i32, ptr %omp.pdo.norm.iv, align 4
  %omp.pdo.norm.ub_fetch.6 = load i32, ptr %omp.pdo.norm.ub, align 4
  %rel.1 = icmp sle i32 %omp.pdo.norm.iv_fetch.5, %omp.pdo.norm.ub_fetch.6
  br i1 %rel.1, label %omp.pdo.body4, label %omp.pdo.epilog5

omp.pdo.body4:                                    ; preds = %omp.pdo.cond3
  %omp.pdo.norm.iv_fetch.7 = load i32, ptr %omp.pdo.norm.iv, align 4
  %omp.pdo.step_fetch.8 = load i32, ptr %omp.pdo.step, align 4
  %mul.1 = mul nsw i32 %omp.pdo.norm.iv_fetch.7, %omp.pdo.step_fetch.8
  %omp.pdo.start_fetch.9 = load i32, ptr %omp.pdo.start, align 4
  %add.1 = add nsw i32 %mul.1, %omp.pdo.start_fetch.9
  store i32 %add.1, ptr %"test_$I", align 8
  %do.norm.lb_fetch.13 = load i32, ptr %do.norm.lb, align 4
  store i32 %do.norm.lb_fetch.13, ptr %do.norm.iv, align 4
  br label %do.cond9

do.cond9:                                         ; preds = %do.body10, %omp.pdo.body4
  %do.norm.iv_fetch.14 = load i32, ptr %do.norm.iv, align 4
  %do.norm.ub_fetch.15 = load i32, ptr %do.norm.ub, align 4
  %rel.2 = icmp sle i32 %do.norm.iv_fetch.14, %do.norm.ub_fetch.15
  br i1 %rel.2, label %do.body10, label %do.epilog11

do.body10:                                        ; preds = %do.cond9
  %do.norm.iv_fetch.16 = load i32, ptr %do.norm.iv, align 4
  %do.step_fetch.17 = load i32, ptr %do.step, align 4
  %mul.2 = mul nsw i32 %do.norm.iv_fetch.16, %do.step_fetch.17
  %do.start_fetch.18 = load i32, ptr %do.start, align 4
  %add.2 = add nsw i32 %mul.2, %do.start_fetch.18
  store i32 %add.2, ptr %"test_$J", align 8
  call void @bar_.t0p.t0p(ptr %"test_$I", ptr %"test_$J"), !llfort.type_idx !5
  %do.norm.iv_fetch.19 = load i32, ptr %do.norm.iv, align 4
  %add.3 = add nsw i32 %do.norm.iv_fetch.19, 1
  store i32 %add.3, ptr %do.norm.iv, align 4
  br label %do.cond9

do.epilog11:                                      ; preds = %do.cond9
  %omp.pdo.norm.iv_fetch.20 = load i32, ptr %omp.pdo.norm.iv, align 4
  %add.4 = add nsw i32 %omp.pdo.norm.iv_fetch.20, 1
  store i32 %add.4, ptr %omp.pdo.norm.iv, align 4
  br label %omp.pdo.cond3

bb_new6:                                          ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(),
    "QUAL.OMP.SIZES"(i32 4, i32 8),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %omp.pdo.norm.iv, i32 0, ptr %do.norm.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %omp.pdo.norm.ub, i32 0, ptr %do.norm.ub, i32 0),
    "QUAL.OMP.LIVEIN"(ptr %do.norm.lb),
    "QUAL.OMP.LIVEIN"(ptr %omp.pdo.norm.lb) ]
  %omp.pdo.norm.lb_fetch.4 = load i32, ptr %omp.pdo.norm.lb, align 4
  store i32 %omp.pdo.norm.lb_fetch.4, ptr %omp.pdo.norm.iv, align 4
  br label %omp.pdo.cond3

omp.pdo.epilog5:                                  ; preds = %omp.pdo.cond3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TILE"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: noinline nounwind optnone uwtable
define internal void @bar_.t0p.t0p(ptr %arg0, ptr %arg1) #2 {
wrap_start18:
  call void (...) @bar_(ptr %arg0, ptr %arg1)
  ret void
}

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare void @bar_(...)

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "intel-lang"="fortran" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
attributes #2 = { noinline nounwind optnone uwtable "frame-pointer"="all" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

!omp_offload.info = !{}
!llvm.module.flags = !{!0}

!0 = !{i32 7, !"openmp", i32 50}
!1 = !{i64 18}
!2 = !{i64 23}
!3 = !{i64 24}
!4 = !{i64 2}
!5 = !{i64 26}

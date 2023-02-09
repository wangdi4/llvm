; INTEL_CUSTOMIZATION
;RUN: opt -enable-new-pm=0 -vpo-paropt-loop-transform -disable-vpo-paropt-tile=false -S < %s  | FileCheck %s
;RUN: opt -opaque-pointers=0 -passes='function(vpo-paropt-loop-transform)' -disable-vpo-paropt-tile=false -S < %s | FileCheck %s

; Verify that omp loop tile construct is in effect. Notice that normalized
; induction variables and loop upperbound have to be added to the outer region's
; entry by the result of inner region's tiling.
; Specifically in this test, inner region's floor loop's information is fed
; into outer region's entry.

; Test src:

; subroutine test()
; integer :: i, j
; !$omp do
; !$omp tile sizes(4)
; do i = 1, 100
;   call bar(i)
; end do
; end subroutine

; CHECK-DAG: FLOOR.LATCH
; CHECK-DAG: FLOOR.PREHEAD
; CHECK-DAG: FLOOR.HEAD
; CHECK-DAG: @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.PRIVATE"(i32* %"test_$I"), "QUAL.OMP.NORMALIZED.IV:TYPED"(i64* %floor_iv, i64 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(i64* %floor_ub, i64 0), "QUAL.OMP.FIRSTPRIVATE:TYPED"(i64* %floor_lb, i64 0, i32 1) ]

; ModuleID = 'do1_tile1.f90'
source_filename = "do1_tile1.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define void @test_() #0 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8, !llfort.type_idx !1
  %"test_$J" = alloca i32, align 8, !llfort.type_idx !2
  %"test_$I" = alloca i32, align 8, !llfort.type_idx !3
  %omp.pdo.start = alloca i32, align 4, !llfort.type_idx !4
  store i32 1, i32* %omp.pdo.start, align 4
  %omp.pdo.end = alloca i32, align 4, !llfort.type_idx !4
  store i32 100, i32* %omp.pdo.end, align 4
  %omp.pdo.step = alloca i32, align 4, !llfort.type_idx !4
  store i32 1, i32* %omp.pdo.step, align 4
  %omp.pdo.norm.iv = alloca i64, align 8, !llfort.type_idx !5
  %omp.pdo.norm.lb = alloca i64, align 8, !llfort.type_idx !5
  store i64 0, i64* %omp.pdo.norm.lb, align 8
  %omp.pdo.norm.ub = alloca i64, align 8, !llfort.type_idx !5
  %omp.pdo.end_fetch.1 = load i32, i32* %omp.pdo.end, align 4
  %omp.pdo.start_fetch.2 = load i32, i32* %omp.pdo.start, align 4
  %sub.1 = sub nsw i32 %omp.pdo.end_fetch.1, %omp.pdo.start_fetch.2
  %omp.pdo.step_fetch.3 = load i32, i32* %omp.pdo.step, align 4
  %div.1 = sdiv i32 %sub.1, %omp.pdo.step_fetch.3
  %int_sext1 = sext i32 %div.1 to i64
  store i64 %int_sext1, i64* %omp.pdo.norm.ub, align 8
  br label %bb_new2

bb_new2:  ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
     "QUAL.OMP.PRIVATE"(i32* %"test_$I") ]
  br label %bb_new7

bb_new7:  ; preds = %bb_new2
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(),
     "QUAL.OMP.SIZES"(i32 4),
     "QUAL.OMP.NORMALIZED.IV"(i64* %omp.pdo.norm.iv),
     "QUAL.OMP.NORMALIZED.UB"(i64* %omp.pdo.norm.ub),
     "QUAL.OMP.LIVEIN"(i32* %"test_$I"),
     "QUAL.OMP.LIVEIN"(i64* %omp.pdo.norm.lb) ]
  br label %DIR.OMP.TILE.3

DIR.OMP.TILE.3:  ; preds = %bb_new7
  %omp.pdo.norm.lb_fetch.4 = load i64, i64* %omp.pdo.norm.lb, align 8
  store i64 %omp.pdo.norm.lb_fetch.4, i64* %omp.pdo.norm.iv, align 8
  br label %omp.pdo.cond4

omp.pdo.cond4:  ; preds = %omp.pdo.body5, %DIR.OMP.TILE.3
  %omp.pdo.norm.iv_fetch.5 = load i64, i64* %omp.pdo.norm.iv, align 8
  %omp.pdo.norm.ub_fetch.6 = load i64, i64* %omp.pdo.norm.ub, align 8
  %rel.1 = icmp sle i64 %omp.pdo.norm.iv_fetch.5, %omp.pdo.norm.ub_fetch.6
  br i1 %rel.1, label %omp.pdo.body5, label %omp.pdo.epilog6

omp.pdo.body5:  ; preds = %omp.pdo.cond4
  %omp.pdo.norm.iv_fetch.7 = load i64, i64* %omp.pdo.norm.iv, align 8
  %int_sext = trunc i64 %omp.pdo.norm.iv_fetch.7 to i32
  %omp.pdo.step_fetch.8 = load i32, i32* %omp.pdo.step, align 4
  %mul.1 = mul nsw i32 %int_sext, %omp.pdo.step_fetch.8
  %omp.pdo.start_fetch.9 = load i32, i32* %omp.pdo.start, align 4
  %add.1 = add nsw i32 %mul.1, %omp.pdo.start_fetch.9
  store i32 %add.1, i32* %"test_$I", align 8
  call void @bar_.t0p(i32* %"test_$I"), !llfort.type_idx !6
  %omp.pdo.norm.iv_fetch.10 = load i64, i64* %omp.pdo.norm.iv, align 8
  %add.2 = add nsw i64 %omp.pdo.norm.iv_fetch.10, 1
  store i64 %add.2, i64* %omp.pdo.norm.iv, align 8
  br label %omp.pdo.cond4

omp.pdo.epilog6:  ; preds = %omp.pdo.cond4
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TILE"() ]
  br label %DIR.OMP.END.TILE.1

DIR.OMP.END.TILE.1:  ; preds = %omp.pdo.epilog6
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]
  br label %DIR.OMP.END.LOOP.2

DIR.OMP.END.LOOP.2:  ; preds = %DIR.OMP.END.TILE.1
  ret void

}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: noinline nounwind optnone uwtable
define internal void @bar_.t0p(i32* %arg0) #2 {
wrap_start15:
  call void (...) @bar_(i32* %arg0)
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
!5 = !{i64 3}
!6 = !{i64 26}
; end INTEL_CUSTOMIZATION

; INTEL_CUSTOMIZATION
; RUN: opt -enable-new-pm=0 -vpo-paropt-loop-transform  -disable-vpo-paropt-tile=false -S < %s | FileCheck %s
; RUN: opt -passes='function(vpo-paropt-loop-transform)' -disable-vpo-paropt-tile=false -S < %s | FileCheck %s

; Verify that #pragma omp tile generate loop-tiled code.
; Notice that outer loop's tile pragma is expecting two level normalized loops to be tiled.
; The inner level loop of those two level loops should come from the tiling of the inner loop & its tile pragma.
; Also, notice that the input IR is manually edited. From omp.pdo.body4, the commented out parts are hoisted to alloca_0.
; This is because outer-loop's upper bound is needed before DIR.OMP.TILE3.
;
;
; Test src:
;
;  subroutine test()
;  integer :: i, j
;  !$omp tile sizes(4,2)
;  do i = 1, 100
;    !$omp tile sizes(8)
;    do j = 1, 48
;      call bar(i,j)
;    end do
;  end do
;  end subroutine

; CHECK: bb_new6:                                          ; preds = %alloca_0
; CHECK-NEXT:  br label %FLOOR.PREHEAD[[V45:[0-9]*]]
;
; CHECK-DAG: FLOOR.PREHEAD[[V45]]:                                  ; preds = %bb_new6
;   %floor.lb46 = load i32, i32* %floor_lb27, align 4
; CHECK:  store i32 [[T:%.*]], i32* [[FIV31:%.*]], align 4
; CHECK:  br label %FLOOR.HEAD[[V41:[0-9]*]]
;
; CHECK-DAG: FLOOR.HEAD[[V41]]:                                     ; preds = %FLOOR.LATCH[[V47:[0-9]*]], %FLOOR.PREHEAD[[V45]]
; CHECK:  [[FIVLD:%.*]] = load i32, i32* [[FIV31]], align 4
; CHECK:  [[FUBLD:%.*]]  = load i32, i32* [[FUB28:%.*]], align 4
; CHECK:  [[T:%.*]] = icmp sle i32 [[FIVLD]], [[FUBLD]]
; CHECK:  br i1 [[T]], label %FLOOR.PREHEAD[[V22:[0-9]*]], label %omp.pdo.epilog5
;
; CHECK-DAG: FLOOR.PREHEAD[[V22]]:                                  ; preds = %FLOOR.HEAD[[V41]]
; CHECK:  [[T:%.*]] = load i32, i32* [[FLB:%.*]], align 4
; CHECK:  store i32 [[T]], i32* [[FIV8:%.*]], align 4
; CHECK:  br label %FLOOR.HEAD[[V18:[0-9]*]]
;
; CHECK-DAG: FLOOR.HEAD[[V18]]:                                     ; preds = %FLOOR.LATCH[[V24:[0-9]*]], %FLOOR.PREHEAD[[V22]]
; CHECK:  [[FIVLD:%.*]] = load i32, i32* [[FIV8]], align 4
;   %floor.ub20 = load i32, i32* %floor_ub5, align 4
; CHECK:  icmp sle i32 [[FIVLD]]
; CHECK:  br i1 [[C:%.*]], label %DIR.OMP.TILE.1, label %FLOOR.LATCH[[V47]]
;
; CHECK-DAG: FLOOR.LATCH[[V47]]:                                    ; preds = %FLOOR.HEAD[[V18]]
; CHECK:  [[T:%.*]] = load i32, i32* [[FIV31]], align 4
; CHECK:  [[V:%.*]] = add i32 [[T]], 1
; CHECK:  store i32 [[V]], i32* [[FIV31]], align 4
; CHECK:  br label %FLOOR.HEAD[[V41]]
;
; CHECK-DAG: DIR.OMP.TILE.1:                                   ; preds = %FLOOR.HEAD[[V18]]
;   %omp.pdo.norm.lb_fetch.1 = load i32, i32* %omp.pdo.norm.lb2, align 4, !llfort.type_idx !5
;   store i32 %omp.pdo.norm.lb_fetch.1, i32* %omp.pdo.norm.iv1, align 4
;   %norm.floor.iv.val34 = load i32, i32* %floor_iv31, align 4
;   %tile.lb35 = mul i32 4, %norm.floor.iv.val34
;   store i32 %tile.lb35, i32* %omp.pdo.norm.iv1, align 4
;   %norm.orig.ub.val36 = load i32, i32* %orig_ub33, align 4
;   %add37 = add i32 %tile.lb35, 4
;   %dec38 = sub i32 %add37, 1
;   %cond39 = icmp sle i32 %dec38, %norm.orig.ub.val36
;   %tile.ub40 = select i1 %cond39, i32 %dec38, i32 %norm.orig.ub.val36
;   store i32 %tile.ub40, i32* %omp.pdo.norm.ub3, align 4
; CHECK:  br label %omp.pdo.cond3
;
; CHECK-DAG: omp.pdo.cond3:                                    ; preds = %DIR.OMP.END.TILE.3, %DIR.OMP.TILE.1
;   %omp.pdo.norm.iv_fetch.2 = load i32, i32* %omp.pdo.norm.iv1, align 4, !llfort.type_idx !5
;   %omp.pdo.norm.ub_fetch.3 = load i32, i32* %omp.pdo.norm.ub3, align 4, !llfort.type_idx !5
;   %rel.1 = icmp sle i32 %omp.pdo.norm.iv_fetch.2, %omp.pdo.norm.ub_fetch.3
; CHECK:  br i1 %rel.1, label %omp.pdo.body4, label %FLOOR.LATCH[[V24]]
;
; CHECK: FLOOR.LATCH[[V24]]:                                    ; preds = %omp.pdo.cond3
;   %floor.iv25 = load i32, i32* [[FIV8]], align 4
;   %inc26 = add i32 %floor.iv25, 1
;   store i32 %inc26, i32* [[FIV8]], align 4
; CHECK:  br label %FLOOR.HEAD
;
; omp.pdo.body4:                                    ; preds = %omp.pdo.cond3
;   %omp.pdo.norm.iv_fetch.4 = load i32, i32* %omp.pdo.norm.iv1, align 4, !llfort.type_idx !5
;   %add.2 = add nsw i32 %omp.pdo.norm.iv_fetch.4, 1
;   store i32 %add.2, i32* %"test_$I", align 8
;   br label %bb_new12
;
; bb_new12:                                         ; preds = %omp.pdo.body4
; CHECK-DAG:  br label %FLOOR.PREHEAD
;
; CHECK-DAG: FLOOR.PREHEAD:                                    ; preds = %bb_new12
;   %floor.lb2 = load i32, i32* %floor_lb, align 4
;   store i32 %floor.lb2, i32* %floor_iv, align 4
; CHECK:  %norm.floor.iv.val[[V11:[0-9]*]] = load i32, i32* [[FIV8]], align 4
;   %tile.lb12 = mul i32 2, %norm.floor.iv.val[[V11]]
;   store i32 %tile.lb12, i32* %floor_iv, align 4
;   %norm.orig.ub.val13 = load i32, i32* %orig_ub10, align 4
;   %add14 = add i32 %tile.lb12, 2
;   %dec15 = sub i32 %add14, 1
;   %cond16 = icmp sle i32 %dec15, %norm.orig.ub.val13
;   %tile.ub17 = select i1 %cond16, i32 %dec15, i32 %norm.orig.ub.val13
;   store i32 %tile.ub17, i32* %floor_ub, align 4
; CHECK: br label %FLOOR.HEAD
;
; CHECK-DAG: FLOOR.HEAD:                                       ; preds = %FLOOR.LATCH, %FLOOR.PREHEAD
;   %floor.iv = load i32, i32* %floor_iv, align 4
;   %floor.ub = load i32, i32* %floor_ub, align 4
;   %tile.loop.cond = icmp sle i32 %floor.iv, %floor.ub
;   br i1 %tile.loop.cond, label %DIR.OMP.TILE.2, label %omp.pdo.epilog11
;
; DIR.OMP.TILE.2:                                   ; preds = %FLOOR.HEAD
;   %omp.pdo.norm.lb_fetch.5 = load i32, i32* %omp.pdo.norm.lb, align 4, !llfort.type_idx !5
;   store i32 %omp.pdo.norm.lb_fetch.5, i32* %omp.pdo.norm.iv, align 4
;   %norm.floor.iv.val = load i32, i32* %floor_iv, align 4
;   %tile.lb = mul i32 8, %norm.floor.iv.val
;   store i32 %tile.lb, i32* %omp.pdo.norm.iv, align 4
;   %norm.orig.ub.val1 = load i32, i32* %orig_ub, align 4
;   %add = add i32 %tile.lb, 8
;   %dec = sub i32 %add, 1
;   %cond = icmp sle i32 %dec, %norm.orig.ub.val1
;   %tile.ub = select i1 %cond, i32 %dec, i32 %norm.orig.ub.val1
;   store i32 %tile.ub, i32* %omp.pdo.norm.ub, align 4
;   br label %omp.pdo.cond9
;
; CHECK: omp.pdo.cond9:                                    ; preds = %omp.pdo.body10, %DIR.OMP.TILE.2
;   %omp.pdo.norm.iv_fetch.6 = load i32, i32* %omp.pdo.norm.iv, align 4, !llfort.type_idx !5
;   %omp.pdo.norm.ub_fetch.7 = load i32, i32* %omp.pdo.norm.ub, align 4, !llfort.type_idx !5
;   %rel.2 = icmp sle i32 %omp.pdo.norm.iv_fetch.6, %omp.pdo.norm.ub_fetch.7
; CHECK:  br i1 [[C:%.*]], label %omp.pdo.body10, label %FLOOR.LATCH
;
; CHECK: FLOOR.LATCH:                                      ; preds = %omp.pdo.cond9
;   %floor.iv3 = load i32, i32* %floor_iv, align 4
;   %inc = add i32 %floor.iv3, 1
;   store i32 %inc, i32* %floor_iv, align 4
; CHECK:  br label %FLOOR.HEAD
;
; CHECK: omp.pdo.body10:                                   ; preds = %omp.pdo.cond9
;   %omp.pdo.norm.iv_fetch.8 = load i32, i32* %omp.pdo.norm.iv, align 4, !llfort.type_idx !5
;   %add.4 = add nsw i32 %omp.pdo.norm.iv_fetch.8, 1
;   store i32 %add.4, i32* %"test_$J", align 8
;   call void @bar_.t33p.t34p(i32* %"test_$I", i32* %"test_$J"), !llfort.type_idx !6
;   %"test_$J_fetch.9" = load i32, i32* %"test_$J", align 8, !llfort.type_idx !3
;   %add.5 = add nsw i32 %"test_$J_fetch.9", 1
;   store i32 %add.5, i32* %"test_$J", align 8
;   %omp.pdo.norm.iv_fetch.10 = load i32, i32* %omp.pdo.norm.iv, align 4, !llfort.type_idx !5
;   %add.6 = add nsw i32 %omp.pdo.norm.iv_fetch.10, 1
;   store i32 %add.6, i32* %omp.pdo.norm.iv, align 4
; CHECK:  br label %omp.pdo.cond9

;
; ModuleID = 'tile2__tile1.f90'
source_filename = "tile2__tile1.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @test_() #0 !llfort.type_idx !1 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8, !llfort.type_idx !2
  %"test_$J" = alloca i32, align 8, !llfort.type_idx !3
  %"test_$I" = alloca i32, align 8, !llfort.type_idx !4
  %omp.pdo.norm.iv1 = alloca i32, align 4, !llfort.type_idx !5
  %omp.pdo.norm.lb2 = alloca i32, align 4, !llfort.type_idx !5
  store i32 0, i32* %omp.pdo.norm.lb2, align 4
  %omp.pdo.norm.ub3 = alloca i32, align 4, !llfort.type_idx !5
  store i32 99, i32* %omp.pdo.norm.ub3, align 4
  %omp.pdo.norm.iv = alloca i32, align 4, !llfort.type_idx !5
  %omp.pdo.norm.lb = alloca i32, align 4, !llfort.type_idx !5
  store i32 0, i32* %omp.pdo.norm.lb, align 4
  %omp.pdo.norm.ub = alloca i32, align 4, !llfort.type_idx !5
  store i32 47, i32* %omp.pdo.norm.ub, align 4
  br label %bb_new6

bb_new6:  ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(),
     "QUAL.OMP.SIZES"(i32 4, i32 2),
     "QUAL.OMP.NORMALIZED.IV"(i32* %omp.pdo.norm.iv1),
     "QUAL.OMP.NORMALIZED.UB"(i32* %omp.pdo.norm.ub3),
     "QUAL.OMP.LIVEIN"(i32* %omp.pdo.norm.lb2) ]
  br label %DIR.OMP.TILE.1

DIR.OMP.TILE.1:  ; preds = %bb_new6
  %omp.pdo.norm.lb_fetch.1 = load i32, i32* %omp.pdo.norm.lb2, align 4, !llfort.type_idx !5
  store i32 %omp.pdo.norm.lb_fetch.1, i32* %omp.pdo.norm.iv1, align 4
  br label %omp.pdo.cond3

omp.pdo.cond3:  ; preds = %DIR.OMP.TILE.1, %DIR.OMP.END.TILE.3
  %omp.pdo.norm.iv_fetch.2 = load i32, i32* %omp.pdo.norm.iv1, align 4, !llfort.type_idx !5
  %omp.pdo.norm.ub_fetch.3 = load i32, i32* %omp.pdo.norm.ub3, align 4, !llfort.type_idx !5
  %rel.1 = icmp sle i32 %omp.pdo.norm.iv_fetch.2, %omp.pdo.norm.ub_fetch.3
  br i1 %rel.1, label %omp.pdo.body4, label %omp.pdo.epilog5

omp.pdo.body4:  ; preds = %omp.pdo.cond3
  %omp.pdo.norm.iv_fetch.4 = load i32, i32* %omp.pdo.norm.iv1, align 4, !llfort.type_idx !5
  %add.2 = add nsw i32 %omp.pdo.norm.iv_fetch.4, 1
  store i32 %add.2, i32* %"test_$I", align 8
  br label %bb_new12

bb_new12:  ; preds = %omp.pdo.body4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(),
     "QUAL.OMP.SIZES"(i32 8),
     "QUAL.OMP.NORMALIZED.IV"(i32* %omp.pdo.norm.iv),
     "QUAL.OMP.NORMALIZED.UB"(i32* %omp.pdo.norm.ub),
     "QUAL.OMP.LIVEIN"(i32* %omp.pdo.norm.lb) ]
  br label %DIR.OMP.TILE.2

DIR.OMP.TILE.2:  ; preds = %bb_new12
  %omp.pdo.norm.lb_fetch.5 = load i32, i32* %omp.pdo.norm.lb, align 4, !llfort.type_idx !5
  store i32 %omp.pdo.norm.lb_fetch.5, i32* %omp.pdo.norm.iv, align 4
  br label %omp.pdo.cond9

omp.pdo.cond9:  ; preds = %DIR.OMP.TILE.2, %omp.pdo.body10
  %omp.pdo.norm.iv_fetch.6 = load i32, i32* %omp.pdo.norm.iv, align 4, !llfort.type_idx !5
  %omp.pdo.norm.ub_fetch.7 = load i32, i32* %omp.pdo.norm.ub, align 4, !llfort.type_idx !5
  %rel.2 = icmp sle i32 %omp.pdo.norm.iv_fetch.6, %omp.pdo.norm.ub_fetch.7
  br i1 %rel.2, label %omp.pdo.body10, label %omp.pdo.epilog11

omp.pdo.body10:  ; preds = %omp.pdo.cond9
  %omp.pdo.norm.iv_fetch.8 = load i32, i32* %omp.pdo.norm.iv, align 4, !llfort.type_idx !5
  %add.4 = add nsw i32 %omp.pdo.norm.iv_fetch.8, 1
  store i32 %add.4, i32* %"test_$J", align 8
  call void @bar_.t33p.t34p(i32* %"test_$I", i32* %"test_$J"), !llfort.type_idx !6
  %"test_$J_fetch.9" = load i32, i32* %"test_$J", align 8, !llfort.type_idx !3
  %add.5 = add nsw i32 %"test_$J_fetch.9", 1
  store i32 %add.5, i32* %"test_$J", align 8
  %omp.pdo.norm.iv_fetch.10 = load i32, i32* %omp.pdo.norm.iv, align 4, !llfort.type_idx !5
  %add.6 = add nsw i32 %omp.pdo.norm.iv_fetch.10, 1
  store i32 %add.6, i32* %omp.pdo.norm.iv, align 4
  br label %omp.pdo.cond9

omp.pdo.epilog11:  ; preds = %omp.pdo.cond9
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TILE"() ]
  br label %DIR.OMP.END.TILE.3

DIR.OMP.END.TILE.3:  ; preds = %omp.pdo.epilog11
  %"test_$I_fetch.11" = load i32, i32* %"test_$I", align 8, !llfort.type_idx !4
  %add.7 = add nsw i32 %"test_$I_fetch.11", 1
  store i32 %add.7, i32* %"test_$I", align 8
  %omp.pdo.norm.iv_fetch.12 = load i32, i32* %omp.pdo.norm.iv1, align 4, !llfort.type_idx !5
  %add.8 = add nsw i32 %omp.pdo.norm.iv_fetch.12, 1
  store i32 %add.8, i32* %omp.pdo.norm.iv1, align 4
  br label %omp.pdo.cond3

omp.pdo.epilog5:  ; preds = %omp.pdo.cond3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TILE"() ]
  br label %DIR.OMP.END.TILE.4

DIR.OMP.END.TILE.4:  ; preds = %omp.pdo.epilog5
  ret void

}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: noinline nounwind optnone uwtable
define internal void @bar_.t33p.t34p(i32* %arg0, i32* %arg1) #2 !llfort.type_idx !7 {
wrap_start20:
  call void (...) @bar_(i32* %arg0, i32* %arg1)
  ret void

}

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #1

declare !llfort.type_idx !8 void @bar_(...)

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "intel-lang"="fortran" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
attributes #2 = { noinline nounwind optnone uwtable "frame-pointer"="all" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

!omp_offload.info = !{}
!llvm.module.flags = !{!0}

!0 = !{i32 7, !"openmp", i32 50}
!1 = !{i64 24}
!2 = !{i64 20}
!3 = !{i64 25}
!4 = !{i64 26}
!5 = !{i64 2}
!6 = !{i64 17}
!7 = !{i64 32}
!8 = !{i64 35}
;end INTEL_CUSTOMIZATION

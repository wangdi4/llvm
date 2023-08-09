; INTEL_CUSTOMIZATION
; RUN: opt -passes='function(vpo-paropt-loop-transform)' -disable-vpo-paropt-tile=false -S < %s | FileCheck %s

; Verify that #pragma omp tile generate loop-tiled code.
; Notice that outer loop's tile pragma is expecting two level normalized loops to be tiled.
; The inner level loop of those two level loops should come from the tiling of the inner loop & its tile pragma.
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
;   %floor.lb46 = load i32, ptr %floor_lb27, align 4
; CHECK:  store i32 [[T:%.*]], ptr [[FIV31:%.*]], align 4
; CHECK:  br label %FLOOR.HEAD[[V41:[0-9]*]]
;
; CHECK-DAG: FLOOR.HEAD[[V41]]:                                     ; preds = %FLOOR.LATCH[[V47:[0-9]*]], %FLOOR.PREHEAD[[V45]]
; CHECK:  [[FIVLD:%.*]] = load i32, ptr [[FIV31]], align 4
; CHECK:  [[FUBLD:%.*]]  = load i32, ptr [[FUB28:%.*]], align 4
; CHECK:  [[T:%.*]] = icmp sle i32 [[FIVLD]], [[FUBLD]]
; CHECK:  br i1 [[T]], label %FLOOR.PREHEAD[[V22:[0-9]*]], label %omp.pdo.epilog5
;
; CHECK-DAG: FLOOR.PREHEAD[[V22]]:                                  ; preds = %FLOOR.HEAD[[V41]]
; CHECK:  [[T:%.*]] = load i32, ptr [[FLB:%.*]], align 4
; CHECK:  store i32 [[T]], ptr [[FIV8:%.*]], align 4
; CHECK:  br label %FLOOR.HEAD[[V18:[0-9]*]]
;
; CHECK-DAG: FLOOR.HEAD[[V18]]:                                     ; preds = %FLOOR.LATCH[[V24:[0-9]*]], %FLOOR.PREHEAD[[V22]]
; CHECK:  [[FIVLD:%.*]] = load i32, ptr [[FIV8]], align 4
;   %floor.ub20 = load i32, ptr %floor_ub5, align 4
; CHECK:  icmp sle i32 [[FIVLD]]
; CHECK:  br i1 [[C:%.*]], label %DIR.OMP.TILE.1, label %FLOOR.LATCH[[V47]]
;
; CHECK-DAG: FLOOR.LATCH[[V47]]:                                    ; preds = %FLOOR.HEAD[[V18]]
; CHECK:  [[T:%.*]] = load i32, ptr [[FIV31]], align 4
; CHECK:  [[V:%.*]] = add i32 [[T]], 1
; CHECK:  store i32 [[V]], ptr [[FIV31]], align 4
; CHECK:  br label %FLOOR.HEAD[[V41]]
;
; CHECK-DAG: DIR.OMP.TILE.1:                                   ; preds = %FLOOR.HEAD[[V18]]
;   %omp.pdo.norm.lb_fetch.1 = load i32, ptr %omp.pdo.norm.lb2, align 4, !llfort.type_idx !5
;   store i32 %omp.pdo.norm.lb_fetch.1, ptr %omp.pdo.norm.iv1, align 4
;   %norm.floor.iv.val34 = load i32, ptr %floor_iv31, align 4
;   %tile.lb35 = mul i32 4, %norm.floor.iv.val34
;   store i32 %tile.lb35, ptr %omp.pdo.norm.iv1, align 4
;   %norm.orig.ub.val36 = load i32, ptr %orig_ub33, align 4
;   %add37 = add i32 %tile.lb35, 4
;   %dec38 = sub i32 %add37, 1
;   %cond39 = icmp sle i32 %dec38, %norm.orig.ub.val36
;   %tile.ub40 = select i1 %cond39, i32 %dec38, i32 %norm.orig.ub.val36
;   store i32 %tile.ub40, ptr %omp.pdo.norm.ub3, align 4
; CHECK:  br label %omp.pdo.cond3
;
; CHECK-DAG: omp.pdo.cond3:                                    ; preds = %DIR.OMP.END.TILE.3, %DIR.OMP.TILE.1
;   %omp.pdo.norm.iv_fetch.2 = load i32, ptr %omp.pdo.norm.iv1, align 4, !llfort.type_idx !5
;   %omp.pdo.norm.ub_fetch.3 = load i32, ptr %omp.pdo.norm.ub3, align 4, !llfort.type_idx !5
;   %rel.1 = icmp sle i32 %omp.pdo.norm.iv_fetch.2, %omp.pdo.norm.ub_fetch.3
; CHECK:  br i1 %rel.1, label %omp.pdo.body4, label %FLOOR.LATCH[[V24]]
;
; CHECK: FLOOR.LATCH[[V24]]:                                    ; preds = %omp.pdo.cond3
;   %floor.iv25 = load i32, ptr [[FIV8]], align 4
;   %inc26 = add i32 %floor.iv25, 1
;   store i32 %inc26, ptr [[FIV8]], align 4
; CHECK:  br label %FLOOR.HEAD
;
; omp.pdo.body4:                                    ; preds = %omp.pdo.cond3
;   %omp.pdo.norm.iv_fetch.4 = load i32, ptr %omp.pdo.norm.iv1, align 4, !llfort.type_idx !5
;   %add.2 = add nsw i32 %omp.pdo.norm.iv_fetch.4, 1
;   store i32 %add.2, ptr %"test_$I", align 8
;   br label %bb_new12
;
; bb_new12:                                         ; preds = %omp.pdo.body4
; CHECK-DAG:  br label %FLOOR.PREHEAD
;
; CHECK-DAG: FLOOR.PREHEAD:                                    ; preds = %bb_new12
;   %floor.lb2 = load i32, ptr %floor_lb, align 4
;   store i32 %floor.lb2, ptr %floor_iv, align 4
; CHECK:  %norm.floor.iv.val[[V11:[0-9]*]] = load i32, ptr [[FIV8]], align 4
;   %tile.lb12 = mul i32 2, %norm.floor.iv.val[[V11]]
;   store i32 %tile.lb12, ptr %floor_iv, align 4
;   %norm.orig.ub.val13 = load i32, ptr %orig_ub10, align 4
;   %add14 = add i32 %tile.lb12, 2
;   %dec15 = sub i32 %add14, 1
;   %cond16 = icmp sle i32 %dec15, %norm.orig.ub.val13
;   %tile.ub17 = select i1 %cond16, i32 %dec15, i32 %norm.orig.ub.val13
;   store i32 %tile.ub17, ptr %floor_ub, align 4
; CHECK: br label %FLOOR.HEAD
;
; CHECK-DAG: FLOOR.HEAD:                                       ; preds = %FLOOR.LATCH, %FLOOR.PREHEAD
;   %floor.iv = load i32, ptr %floor_iv, align 4
;   %floor.ub = load i32, ptr %floor_ub, align 4
;   %tile.loop.cond = icmp sle i32 %floor.iv, %floor.ub
;   br i1 %tile.loop.cond, label %DIR.OMP.TILE.2, label %omp.pdo.epilog11
;
; DIR.OMP.TILE.2:                                   ; preds = %FLOOR.HEAD
;   %omp.pdo.norm.lb_fetch.5 = load i32, ptr %omp.pdo.norm.lb, align 4, !llfort.type_idx !5
;   store i32 %omp.pdo.norm.lb_fetch.5, ptr %omp.pdo.norm.iv, align 4
;   %norm.floor.iv.val = load i32, ptr %floor_iv, align 4
;   %tile.lb = mul i32 8, %norm.floor.iv.val
;   store i32 %tile.lb, ptr %omp.pdo.norm.iv, align 4
;   %norm.orig.ub.val1 = load i32, ptr %orig_ub, align 4
;   %add = add i32 %tile.lb, 8
;   %dec = sub i32 %add, 1
;   %cond = icmp sle i32 %dec, %norm.orig.ub.val1
;   %tile.ub = select i1 %cond, i32 %dec, i32 %norm.orig.ub.val1
;   store i32 %tile.ub, ptr %omp.pdo.norm.ub, align 4
;   br label %omp.pdo.cond9
;
; CHECK: omp.pdo.cond9:                                    ; preds = %omp.pdo.body10, %DIR.OMP.TILE.2
;   %omp.pdo.norm.iv_fetch.6 = load i32, ptr %omp.pdo.norm.iv, align 4, !llfort.type_idx !5
;   %omp.pdo.norm.ub_fetch.7 = load i32, ptr %omp.pdo.norm.ub, align 4, !llfort.type_idx !5
;   %rel.2 = icmp sle i32 %omp.pdo.norm.iv_fetch.6, %omp.pdo.norm.ub_fetch.7
; CHECK:  br i1 [[C:%.*]], label %omp.pdo.body10, label %FLOOR.LATCH
;
; CHECK: FLOOR.LATCH:                                      ; preds = %omp.pdo.cond9
;   %floor.iv3 = load i32, ptr %floor_iv, align 4
;   %inc = add i32 %floor.iv3, 1
;   store i32 %inc, ptr %floor_iv, align 4
; CHECK:  br label %FLOOR.HEAD
;
; CHECK: omp.pdo.body10:                                   ; preds = %omp.pdo.cond9
;   %omp.pdo.norm.iv_fetch.8 = load i32, ptr %omp.pdo.norm.iv, align 4, !llfort.type_idx !5
;   %add.4 = add nsw i32 %omp.pdo.norm.iv_fetch.8, 1
;   store i32 %add.4, ptr %"test_$J", align 8
;   call void @bar_.t33p.t34p(ptr %"test_$I", ptr %"test_$J"), !llfort.type_idx !6
;   %"test_$J_fetch.9" = load i32, ptr %"test_$J", align 8, !llfort.type_idx !3
;   %add.5 = add nsw i32 %"test_$J_fetch.9", 1
;   store i32 %add.5, ptr %"test_$J", align 8
;   %omp.pdo.norm.iv_fetch.10 = load i32, ptr %omp.pdo.norm.iv, align 4, !llfort.type_idx !5
;   %add.6 = add nsw i32 %omp.pdo.norm.iv_fetch.10, 1
;   store i32 %add.6, ptr %omp.pdo.norm.iv, align 4
; CHECK:  br label %omp.pdo.cond9


source_filename = "tile2__tile1.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @test_() #0 !llfort.type_idx !1 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 16, !llfort.type_idx !2
  %"test_$J" = alloca i32, align 8, !llfort.type_idx !3
  %"test_$I" = alloca i32, align 8, !llfort.type_idx !4
  %omp.pdo.norm.iv = alloca i32, align 4, !llfort.type_idx !5
  %omp.pdo.norm.lb = alloca i32, align 4, !llfort.type_idx !5
  store i32 0, ptr %omp.pdo.norm.lb, align 4, !tbaa !6
  %omp.pdo.norm.ub = alloca i32, align 4, !llfort.type_idx !5
  store i32 47, ptr %omp.pdo.norm.ub, align 4, !tbaa !6
  %omp.pdo.norm.iv1 = alloca i32, align 4, !llfort.type_idx !5
  %omp.pdo.norm.lb2 = alloca i32, align 4, !llfort.type_idx !5
  store i32 0, ptr %omp.pdo.norm.lb2, align 4, !tbaa !6
  %omp.pdo.norm.ub3 = alloca i32, align 4, !llfort.type_idx !5
  store i32 99, ptr %omp.pdo.norm.ub3, align 4, !tbaa !6
  br label %bb_new6

bb_new6:  ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(),
     "QUAL.OMP.SIZES"(i32 4, i32 2),
     "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %omp.pdo.norm.iv1, i32 0),
     "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %omp.pdo.norm.ub3, i32 0),
     "QUAL.OMP.LIVEIN"(ptr %omp.pdo.norm.lb2) ]
  br label %DIR.OMP.TILE.1

DIR.OMP.TILE.1:  ; preds = %bb_new6
  %omp.pdo.norm.lb_fetch.1 = load i32, ptr %omp.pdo.norm.lb2, align 4, !tbaa !6, !llfort.type_idx !5
  store i32 %omp.pdo.norm.lb_fetch.1, ptr %omp.pdo.norm.iv1, align 4, !tbaa !6
  br label %omp.pdo.cond3

omp.pdo.cond3:  ; preds = %DIR.OMP.TILE.1, %DIR.OMP.END.TILE.3
  %omp.pdo.norm.iv_fetch.2 = load i32, ptr %omp.pdo.norm.iv1, align 4, !tbaa !6, !llfort.type_idx !5
  %omp.pdo.norm.ub_fetch.3 = load i32, ptr %omp.pdo.norm.ub3, align 4, !tbaa !6, !llfort.type_idx !5
  %rel.1 = icmp sle i32 %omp.pdo.norm.iv_fetch.2, %omp.pdo.norm.ub_fetch.3
  br i1 %rel.1, label %omp.pdo.body4, label %omp.pdo.epilog5

omp.pdo.body4:  ; preds = %omp.pdo.cond3
  %omp.pdo.norm.iv_fetch.4 = load i32, ptr %omp.pdo.norm.iv1, align 4, !tbaa !6, !llfort.type_idx !5
  %add.2 = add nsw i32 %omp.pdo.norm.iv_fetch.4, 1
  store i32 %add.2, ptr %"test_$I", align 8, !tbaa !10
  br label %bb_new12

bb_new12:  ; preds = %omp.pdo.body4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(),
     "QUAL.OMP.SIZES"(i32 8),
     "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %omp.pdo.norm.iv, i32 0),
     "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %omp.pdo.norm.ub, i32 0),
     "QUAL.OMP.LIVEIN"(ptr %omp.pdo.norm.lb) ]
  br label %DIR.OMP.TILE.2

DIR.OMP.TILE.2:  ; preds = %bb_new12
  %omp.pdo.norm.lb_fetch.5 = load i32, ptr %omp.pdo.norm.lb, align 4, !tbaa !6, !llfort.type_idx !5
  store i32 %omp.pdo.norm.lb_fetch.5, ptr %omp.pdo.norm.iv, align 4, !tbaa !6
  br label %omp.pdo.cond9

omp.pdo.cond9:  ; preds = %DIR.OMP.TILE.2, %omp.pdo.body10
  %omp.pdo.norm.iv_fetch.6 = load i32, ptr %omp.pdo.norm.iv, align 4, !tbaa !6, !llfort.type_idx !5
  %omp.pdo.norm.ub_fetch.7 = load i32, ptr %omp.pdo.norm.ub, align 4, !tbaa !6, !llfort.type_idx !5
  %rel.2 = icmp sle i32 %omp.pdo.norm.iv_fetch.6, %omp.pdo.norm.ub_fetch.7
  br i1 %rel.2, label %omp.pdo.body10, label %omp.pdo.epilog11

omp.pdo.body10:  ; preds = %omp.pdo.cond9
  %omp.pdo.norm.iv_fetch.8 = load i32, ptr %omp.pdo.norm.iv, align 4, !tbaa !6, !llfort.type_idx !5
  %add.4 = add nsw i32 %omp.pdo.norm.iv_fetch.8, 1
  store i32 %add.4, ptr %"test_$J", align 8, !tbaa !12
  call void @bar_.t33p.t34p(ptr %"test_$I", ptr %"test_$J"), !llfort.type_idx !14
  %"test_$J_fetch.9" = load i32, ptr %"test_$J", align 8, !tbaa !12, !llfort.type_idx !3
  %add.5 = add nsw i32 %"test_$J_fetch.9", 1
  store i32 %add.5, ptr %"test_$J", align 8, !tbaa !12
  %omp.pdo.norm.iv_fetch.10 = load i32, ptr %omp.pdo.norm.iv, align 4, !tbaa !6, !llfort.type_idx !5
  %add.6 = add nsw i32 %omp.pdo.norm.iv_fetch.10, 1
  store i32 %add.6, ptr %omp.pdo.norm.iv, align 4, !tbaa !6
  br label %omp.pdo.cond9

omp.pdo.epilog11:  ; preds = %omp.pdo.cond9
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TILE"() ]
  br label %DIR.OMP.END.TILE.3

DIR.OMP.END.TILE.3:  ; preds = %omp.pdo.epilog11
  %"test_$I_fetch.11" = load i32, ptr %"test_$I", align 8, !tbaa !10, !llfort.type_idx !4
  %add.7 = add nsw i32 %"test_$I_fetch.11", 1
  store i32 %add.7, ptr %"test_$I", align 8, !tbaa !10
  %omp.pdo.norm.iv_fetch.12 = load i32, ptr %omp.pdo.norm.iv1, align 4, !tbaa !6, !llfort.type_idx !5
  %add.8 = add nsw i32 %omp.pdo.norm.iv_fetch.12, 1
  store i32 %add.8, ptr %omp.pdo.norm.iv1, align 4, !tbaa !6
  br label %omp.pdo.cond3

omp.pdo.epilog5:  ; preds = %omp.pdo.cond3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TILE"() ]
  br label %DIR.OMP.END.TILE.4

DIR.OMP.END.TILE.4:  ; preds = %omp.pdo.epilog5
  ret void

}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind uwtable
define internal void @bar_.t33p.t34p(ptr %arg0, ptr %arg1) #2 !llfort.type_idx !15 {
wrap_start20:
  call void (...) @bar_(ptr %arg0, ptr %arg1)
  ret void

}

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #1

declare !llfort.type_idx !16 void @bar_(...)

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
attributes #2 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

!omp_offload.info = !{}
!llvm.module.flags = !{!0}

!0 = !{i32 7, !"openmp", i32 50}
!1 = !{i64 24}
!2 = !{i64 20}
!3 = !{i64 25}
!4 = !{i64 26}
!5 = !{i64 2}
!6 = !{!7, !7, i64 0}
!7 = !{!"Fortran Data Symbol", !8, i64 0}
!8 = !{!"Generic Fortran Symbol", !9, i64 0}
!9 = !{!"ifx$root$1$test_"}
!10 = !{!11, !11, i64 0}
!11 = !{!"ifx$unique_sym$1", !7, i64 0}
!12 = !{!13, !13, i64 0}
!13 = !{!"ifx$unique_sym$2", !7, i64 0}
!14 = !{i64 17}
!15 = !{i64 32}
!16 = !{i64 35}

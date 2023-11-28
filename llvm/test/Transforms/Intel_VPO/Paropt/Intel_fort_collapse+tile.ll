; INTEL_CUSTOMIZATION
; RUN: opt -passes='function(vpo-paropt-loop-transform,vpo-paropt-loop-collapse)' -disable-vpo-paropt-tile=false -S < %s | FileCheck %s
; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-loop-transform -vpo-paropt-loop-collapse -disable-vpo-paropt-tile=false -S < %s | FileCheck %s

; Verify that collapse pass works correctly after tile pass.

; Test src:
;
;  subroutine test()
;  integer :: i, j
;  !$omp do collapse(2)
;  do i = 1, 100
;    !$omp tile sizes(8)
;    do j = 1, 48
;      call bar(i,j)
;    end do
;  end do
;  end subroutine

; Function Attrs: nounwind uwtable
; define void @test_() #0 !llfort.type_idx !1 {
; alloca_0:
;CHECK:   %omp.collapsed.iv = alloca i64, align 8
;CHECK:   %omp.collapsed.lb = alloca i64, align 8
;CHECK:   %omp.collapsed.ub = alloca i64, align 8
;   %"$io_ctx" = alloca [8 x i64], align 16, !llfort.type_idx !2
;   %"test_$J" = alloca i32, align 8, !llfort.type_idx !3
;   %"test_$I" = alloca i32, align 8, !llfort.type_idx !4
;   %omp.pdo.norm.iv1 = alloca i32, align 4, !llfort.type_idx !5
;   %omp.pdo.norm.lb2 = alloca i32, align 4, !llfort.type_idx !5
;   store i32 0, ptr %omp.pdo.norm.lb2, align 4, !tbaa !6
;   %omp.pdo.norm.ub3 = alloca i32, align 4, !llfort.type_idx !5
;   store i32 99, ptr %omp.pdo.norm.ub3, align 4, !tbaa !6
;   %omp.pdo.norm.iv = alloca i32, align 4, !llfort.type_idx !5
;   %omp.pdo.norm.lb = alloca i32, align 4, !llfort.type_idx !5
;   store i32 0, ptr %omp.pdo.norm.lb, align 4, !tbaa !6
;   %omp.pdo.norm.ub = alloca i32, align 4, !llfort.type_idx !5
;   store i32 47, ptr %omp.pdo.norm.ub, align 4, !tbaa !6
;   %floor_lb = alloca i32, align 4
;   store i32 0, ptr %floor_lb, align 4
;   %floor_ub = alloca i32, align 4
;   %norm.orig.ub.val = load i32, ptr %omp.pdo.norm.ub, align 4
;   %norm.floor.ub.val = sdiv i32 %norm.orig.ub.val, 8
;   store i32 %norm.floor.ub.val, ptr %floor_ub, align 4
;   %floor_iv = alloca i32, align 4
;   %floor.lb = load i32, ptr %floor_lb, align 4
;   store i32 %floor.lb, ptr %floor_iv, align 4
;   %orig_ub = alloca i32, align 4
;   store i32 %norm.orig.ub.val, ptr %orig_ub, align 4
;   br label %bb_new6
;
; bb_new6:                                          ; preds = %alloca_0
;   %omp.pdo.norm.ub3.val = load i32, ptr %omp.pdo.norm.ub3, align 4
;   %.sext = sext i32 %omp.pdo.norm.ub3.val to i64
;   %0 = add nuw nsw i64 %.sext, 1
;   %1 = icmp slt i64 %.sext, 0
;   %floor_ub.val = load i32, ptr %floor_ub, align 4
;   %.sext4 = sext i32 %floor_ub.val to i64
;   %2 = add nuw nsw i64 %.sext4, 1
;   %3 = icmp slt i64 %.sext4, 0
;   %4 = or i1 %1, %3
;   %5 = mul nuw nsw i64 %0, %2
;   %6 = select i1 %4, i64 0, i64 %5
;   %omp.collapsed.ub.value = sub nuw nsw i64 %6, 1
;   store i64 0, ptr %omp.collapsed.lb, align 8
;   store i64 %omp.collapsed.ub.value, ptr %omp.collapsed.ub, align 8

; CHECK:  [[V:%.*]] = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.COLLAPSE"(i32 2), "QUAL.OMP.PRIVATE:TYPED"(ptr %"test_$I", i32 0, i32 1), "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %omp.pdo.norm.lb, i32 0, i32 1), "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %omp.pdo.norm.lb2, i32 0, i32 1), "QUAL.OMP.LIVEIN"(ptr %"test_$J"), "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %floor_lb, i32 0, i32 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %omp.collapsed.iv, i64 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %omp.collapsed.ub, i64 0), "QUAL.OMP.PRIVATE:TYPED"(ptr %omp.pdo.norm.iv1, i32 0, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %floor_iv, i32 0, i32 1), "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %omp.collapsed.lb, i64 0, i32 1), "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %omp.pdo.norm.ub3, i32 0, i32 1), "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %floor_ub, i32 0, i32 1) ]

;   br label %bb_new6.split
;
; bb_new6.split:                                    ; preds = %bb_new6
;   %floor_ub.val5 = load i32, ptr %floor_ub, align 4
;   %.zext = zext i32 %floor_ub.val5 to i64
;   %8 = add nuw nsw i64 %.zext, 1
;   %9 = mul nuw nsw i64 %8, 1
;   br label %DIR.OMP.LOOP.1
;
; CHECK: DIR.OMP.LOOP.1:                                   ; preds = %bb_new6.split
;   %omp.pdo.norm.lb_fetch.1 = load i32, ptr %omp.pdo.norm.lb2, align 4, !tbaa !6, !llfort.type_idx !5
;   store i32 %omp.pdo.norm.lb_fetch.1, ptr %omp.pdo.norm.iv1, align 4, !tbaa !6
;CHECK:   br label %DIR.OMP.LOOP.1.split
;
; CHECK: DIR.OMP.LOOP.1.split:                             ; preds = %DIR.OMP.LOOP.1
;   %10 = load i64, ptr %omp.collapsed.lb, align 8
;   store i64 %10, ptr %omp.collapsed.iv, align 8
;CHECK:   br label %omp.collapsed.loop.cond
;
; CHECK: omp.collapsed.loop.cond:                          ; preds = %omp.collapsed.loop.inc, %DIR.OMP.LOOP.1.split
;   %11 = load i64, ptr %omp.collapsed.iv, align 8
;   %12 = load i64, ptr %omp.collapsed.ub, align 8
;   %13 = icmp sle i64 %11, %12
;   br i1 %13, label %omp.collapsed.loop.body, label %omp.collapsed.loop.exit, !prof !10
;
; CHECK: omp.collapsed.loop.body:                          ; preds = %omp.collapsed.loop.cond
;   %omp.collapsed.iv.val = load i64, ptr %omp.collapsed.iv, align 8
;   %14 = sdiv i64 %omp.collapsed.iv.val, %9
;   %15 = trunc i64 %14 to i32
;   store i32 %15, ptr %omp.pdo.norm.lb2, align 4
;   %16 = trunc i64 %14 to i32
;   store i32 %16, ptr %omp.pdo.norm.ub3, align 4
;   %17 = srem i64 %omp.collapsed.iv.val, %9
;   %18 = trunc i64 %14 to i32
;   store i32 %18, ptr %omp.pdo.norm.iv1, align 4
;   %19 = sdiv i64 %17, 1
;   %20 = trunc i64 %19 to i32
;   store i32 %20, ptr %floor_lb, align 4
;   %21 = trunc i64 %19 to i32
;   store i32 %21, ptr %floor_ub, align 4
;   %22 = srem i64 %17, 1
;   br label %omp.pdo.cond3
;
; CHECK: omp.collapsed.loop.exit:                          ; preds = %omp.collapsed.loop.cond
;   br label %omp.collapsed.loop.postexit
;
; CHECK: omp.pdo.cond3:                                    ; preds = %omp.collapsed.loop.body, %DIR.OMP.END.TILE.3
;   %omp.pdo.norm.iv_fetch.2 = load i32, ptr %omp.pdo.norm.iv1, align 4, !tbaa !6, !llfort.type_idx !5
;   %omp.pdo.norm.ub_fetch.3 = load i32, ptr %omp.pdo.norm.ub3, align 4, !tbaa !6, !llfort.type_idx !5
;   %rel.1 = icmp sle i32 %omp.pdo.norm.iv_fetch.2, %omp.pdo.norm.ub_fetch.3
;   br i1 %rel.1, label %omp.pdo.body4, label %omp.collapsed.loop.inc
;
; CHECK: omp.pdo.body4:                                    ; preds = %omp.pdo.cond3
;   %omp.pdo.norm.iv_fetch.4 = load i32, ptr %omp.pdo.norm.iv1, align 4, !tbaa !6, !llfort.type_idx !5
;   %add.2 = add nsw i32 %omp.pdo.norm.iv_fetch.4, 1
;   store i32 %add.2, ptr %"test_$I", align 8, !tbaa !11
;   br label %bb_new12
;
; CHECK: bb_new12:                                         ; preds = %omp.pdo.body4
;   br label %FLOOR.PREHEAD
;
; CHECK: FLOOR.PREHEAD:                                    ; preds = %bb_new12
;   %floor.lb2 = load i32, ptr %floor_lb, align 4
;   store i32 %floor.lb2, ptr %floor_iv, align 4
;   br label %FLOOR.HEAD
;
; CHECK: FLOOR.HEAD:                                       ; preds = %FLOOR.LATCH, %FLOOR.PREHEAD
;   %floor.iv = load i32, ptr %floor_iv, align 4
;   %floor.ub = load i32, ptr %floor_ub, align 4
;   %tile.loop.cond = icmp sle i32 %floor.iv, %floor.ub
;   br i1 %tile.loop.cond, label %DIR.OMP.TILE.2, label %omp.pdo.epilog11
;
; CHECK: DIR.OMP.TILE.2:                                   ; preds = %FLOOR.HEAD
;   %omp.pdo.norm.lb_fetch.5 = load i32, ptr %omp.pdo.norm.lb, align 4, !tbaa !6, !llfort.type_idx !5
;   store i32 %omp.pdo.norm.lb_fetch.5, ptr %omp.pdo.norm.iv, align 4, !tbaa !6
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
;   %omp.pdo.norm.iv_fetch.6 = load i32, ptr %omp.pdo.norm.iv, align 4, !tbaa !6, !llfort.type_idx !5
;   %omp.pdo.norm.ub_fetch.7 = load i32, ptr %omp.pdo.norm.ub, align 4, !tbaa !6, !llfort.type_idx !5
;   %rel.2 = icmp sle i32 %omp.pdo.norm.iv_fetch.6, %omp.pdo.norm.ub_fetch.7
;   br i1 %rel.2, label %omp.pdo.body10, label %FLOOR.LATCH
;
; CHECK: FLOOR.LATCH:                                      ; preds = %omp.pdo.cond9
;   %floor.iv3 = load i32, ptr %floor_iv, align 4
;   %inc = add i32 %floor.iv3, 1
;   store i32 %inc, ptr %floor_iv, align 4
;   br label %FLOOR.HEAD
;
; CHECK: omp.pdo.body10:                                   ; preds = %omp.pdo.cond9
;   %omp.pdo.norm.iv_fetch.8 = load i32, ptr %omp.pdo.norm.iv, align 4, !tbaa !6, !llfort.type_idx !5
;   %add.4 = add nsw i32 %omp.pdo.norm.iv_fetch.8, 1
;   store i32 %add.4, ptr %"test_$J", align 8, !tbaa !13
;   call void @bar_.t33p.t34p(ptr %"test_$I", ptr %"test_$J"), !llfort.type_idx !15
;   %"test_$J_fetch.9" = load i32, ptr %"test_$J", align 8, !tbaa !13, !llfort.type_idx !3
;   %add.5 = add nsw i32 %"test_$J_fetch.9", 1
;   store i32 %add.5, ptr %"test_$J", align 8, !tbaa !13
;   %omp.pdo.norm.iv_fetch.10 = load i32, ptr %omp.pdo.norm.iv, align 4, !tbaa !6, !llfort.type_idx !5
;   %add.6 = add nsw i32 %omp.pdo.norm.iv_fetch.10, 1
;   store i32 %add.6, ptr %omp.pdo.norm.iv, align 4, !tbaa !6
;   br label %omp.pdo.cond9
;
; omp.pdo.epilog11:                                 ; preds = %FLOOR.HEAD
;   br label %DIR.OMP.END.TILE.3
;
; DIR.OMP.END.TILE.3:                               ; preds = %omp.pdo.epilog11
;   %omp.pdo.norm.iv_fetch.11 = load i32, ptr %omp.pdo.norm.iv1, align 4, !tbaa !6, !llfort.type_idx !5
;   %add.7 = add nsw i32 %omp.pdo.norm.iv_fetch.11, 1
;   store i32 %add.7, ptr %omp.pdo.norm.iv1, align 4, !tbaa !6
;   br label %omp.pdo.cond3
;
; CHECK: omp.collapsed.loop.inc:                           ; preds = %omp.pdo.cond3
;   %23 = load i64, ptr %omp.collapsed.iv, align 8
;   %24 = add nuw nsw i64 %23, 1
;   store i64 %24, ptr %omp.collapsed.iv, align 8
;   br label %omp.collapsed.loop.cond
;
; CHECK: omp.collapsed.loop.postexit:                      ; preds = %omp.collapsed.loop.exit
;   call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.LOOP"() ]
;   br label %DIR.OMP.END.LOOP.4
;
; DIR.OMP.END.LOOP.4:                               ; preds = %omp.collapsed.loop.postexit
;   ret void
; }

; ModuleID = 'op_ptr_input.ll.reordered'
source_filename = "do2__tile1.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @test_() #0 !llfort.type_idx !1 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 16, !llfort.type_idx !2
  %"test_$J" = alloca i32, align 8, !llfort.type_idx !3
  %"test_$I" = alloca i32, align 8, !llfort.type_idx !4
  %omp.pdo.norm.iv1 = alloca i32, align 4, !llfort.type_idx !5
  %omp.pdo.norm.lb2 = alloca i32, align 4, !llfort.type_idx !5
  store i32 0, ptr %omp.pdo.norm.lb2, align 4, !tbaa !6
  %omp.pdo.norm.ub3 = alloca i32, align 4, !llfort.type_idx !5
  store i32 99, ptr %omp.pdo.norm.ub3, align 4, !tbaa !6
  %omp.pdo.norm.iv = alloca i32, align 4, !llfort.type_idx !5
  %omp.pdo.norm.lb = alloca i32, align 4, !llfort.type_idx !5
  store i32 0, ptr %omp.pdo.norm.lb, align 4, !tbaa !6
  %omp.pdo.norm.ub = alloca i32, align 4, !llfort.type_idx !5
  store i32 47, ptr %omp.pdo.norm.ub, align 4, !tbaa !6
  br label %bb_new6

bb_new6:                                          ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.COLLAPSE"(i32 2),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"test_$I", i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %omp.pdo.norm.lb, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %omp.pdo.norm.lb2, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %omp.pdo.norm.iv1, i32 0, ptr %omp.pdo.norm.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %omp.pdo.norm.ub3, i32 0, ptr %omp.pdo.norm.ub, i32 0),
    "QUAL.OMP.LIVEIN"(ptr %"test_$J") ]
  br label %DIR.OMP.LOOP.1

DIR.OMP.LOOP.1:                                   ; preds = %bb_new6
  %omp.pdo.norm.lb_fetch.1 = load i32, ptr %omp.pdo.norm.lb2, align 4, !tbaa !6, !llfort.type_idx !5
  store i32 %omp.pdo.norm.lb_fetch.1, ptr %omp.pdo.norm.iv1, align 4, !tbaa !6
  br label %omp.pdo.cond3

omp.pdo.cond3:                                    ; preds = %DIR.OMP.END.TILE.3, %DIR.OMP.LOOP.1
  %omp.pdo.norm.iv_fetch.2 = load i32, ptr %omp.pdo.norm.iv1, align 4, !tbaa !6, !llfort.type_idx !5
  %omp.pdo.norm.ub_fetch.3 = load i32, ptr %omp.pdo.norm.ub3, align 4, !tbaa !6, !llfort.type_idx !5
  %rel.1 = icmp sle i32 %omp.pdo.norm.iv_fetch.2, %omp.pdo.norm.ub_fetch.3
  br i1 %rel.1, label %omp.pdo.body4, label %omp.pdo.epilog5

omp.pdo.body4:                                    ; preds = %omp.pdo.cond3
  %omp.pdo.norm.iv_fetch.4 = load i32, ptr %omp.pdo.norm.iv1, align 4, !tbaa !6, !llfort.type_idx !5
  %add.2 = add nsw i32 %omp.pdo.norm.iv_fetch.4, 1
  store i32 %add.2, ptr %"test_$I", align 8, !tbaa !10
  br label %bb_new12

bb_new12:                                         ; preds = %omp.pdo.body4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(), "QUAL.OMP.SIZES"(i32 8), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %omp.pdo.norm.iv, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %omp.pdo.norm.ub, i32 0), "QUAL.OMP.LIVEIN"(ptr %"test_$J"), "QUAL.OMP.LIVEIN"(ptr %"test_$I"), "QUAL.OMP.LIVEIN"(ptr %omp.pdo.norm.lb) ]
  br label %DIR.OMP.TILE.2

DIR.OMP.TILE.2:                                   ; preds = %bb_new12
  %omp.pdo.norm.lb_fetch.5 = load i32, ptr %omp.pdo.norm.lb, align 4, !tbaa !6, !llfort.type_idx !5
  store i32 %omp.pdo.norm.lb_fetch.5, ptr %omp.pdo.norm.iv, align 4, !tbaa !6
  br label %omp.pdo.cond9

omp.pdo.cond9:                                    ; preds = %omp.pdo.body10, %DIR.OMP.TILE.2
  %omp.pdo.norm.iv_fetch.6 = load i32, ptr %omp.pdo.norm.iv, align 4, !tbaa !6, !llfort.type_idx !5
  %omp.pdo.norm.ub_fetch.7 = load i32, ptr %omp.pdo.norm.ub, align 4, !tbaa !6, !llfort.type_idx !5
  %rel.2 = icmp sle i32 %omp.pdo.norm.iv_fetch.6, %omp.pdo.norm.ub_fetch.7
  br i1 %rel.2, label %omp.pdo.body10, label %omp.pdo.epilog11

omp.pdo.body10:                                   ; preds = %omp.pdo.cond9
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

omp.pdo.epilog11:                                 ; preds = %omp.pdo.cond9
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TILE"() ]
  br label %DIR.OMP.END.TILE.3

DIR.OMP.END.TILE.3:                               ; preds = %omp.pdo.epilog11
  %omp.pdo.norm.iv_fetch.11 = load i32, ptr %omp.pdo.norm.iv1, align 4, !tbaa !6, !llfort.type_idx !5
  %add.7 = add nsw i32 %omp.pdo.norm.iv_fetch.11, 1
  store i32 %add.7, ptr %omp.pdo.norm.iv1, align 4, !tbaa !6
  br label %omp.pdo.cond3

omp.pdo.epilog5:                                  ; preds = %omp.pdo.cond3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]
  br label %DIR.OMP.END.LOOP.4

DIR.OMP.END.LOOP.4:                               ; preds = %omp.pdo.epilog5
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
declare void @llvm.directive.region.exit(token) #1

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
; end INTEL_CUSTOMIZATION

; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-transform  -disable-vpo-paropt-tile=false -S < %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-transform)' -disable-vpo-paropt-tile=false -S < %s | FileCheck %s
; Test src:
;
;         subroutine test(A)
;         integer :: i, j
;         !$omp tile sizes(4, 8, 2)
;         do i = 1, 100
;           do j = 1, 48
;             do k = 1, 26
;               call bar(i,j,k)
;             end do
;           end do
;         end do
;         end subroutine

; Three floor loops are introduced.
; Loops are not in rotated forms. A loop header is an exiting block, exits to the outer loop's latch.

; CHECK-DAG: FLOOR.PREHEAD{{[0-9]*}}
; CHECK-DAG: FLOOR.HEAD{{[0-9]*}}
; CHECK-DAG: FLOOR.LATCH{{[0-9]*}}: ; preds = %FLOOR.HEAD{{[0-9]*}}

; CHECK-DAG: FLOOR.PREHEAD{{[0-9]*}}
; CHECK-DAG: FLOOR.HEAD{{[0-9]*}}
; CHECK-DAG: FLOOR.LATCH{{[0-9]*}}: ; preds = %FLOOR.HEAD{{[0-9]*}}

; CHECK-DAG: FLOOR.PREHEAD{{[0-9]*}}
; CHECK-DAG: FLOOR.HEAD{{[0-9]*}}
; CHECK-DAG: FLOOR.LATCH{{[0-9]*}}

; CHECK-NOT:  call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(),

source_filename = "tile3.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @test_(ptr noalias dereferenceable(4) %"test_$A") #0 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 16, !llfort.type_idx !1
  %"test_$K" = alloca i32, align 8, !llfort.type_idx !2
  %"test_$J" = alloca i32, align 8, !llfort.type_idx !3
  %"test_$I" = alloca i32, align 8, !llfort.type_idx !4
  %do.start = alloca i32, align 4, !llfort.type_idx !5
  store i32 1, ptr %do.start, align 4, !tbaa !6
  %do.end = alloca i32, align 4, !llfort.type_idx !5
  store i32 48, ptr %do.end, align 4, !tbaa !6
  %do.step = alloca i32, align 4, !llfort.type_idx !5
  store i32 1, ptr %do.step, align 4, !tbaa !6
  %do.norm.lb = alloca i32, align 4, !llfort.type_idx !5
  store i32 0, ptr %do.norm.lb, align 4, !tbaa !6
  %do.norm.ub = alloca i32, align 4, !llfort.type_idx !5
  %do.end_fetch.11 = load i32, ptr %do.end, align 4, !tbaa !6
  %do.start_fetch.12 = load i32, ptr %do.start, align 4, !tbaa !6
  %sub.3 = sub nsw i32 %do.end_fetch.11, %do.start_fetch.12
  %do.step_fetch.13 = load i32, ptr %do.step, align 4, !tbaa !6
  %add.3 = add nsw i32 %sub.3, %do.step_fetch.13
  %do.step_fetch.14 = load i32, ptr %do.step, align 4, !tbaa !6
  %div.2 = sdiv i32 %add.3, %do.step_fetch.14
  %sub.4 = sub nsw i32 %div.2, 1
  store i32 %sub.4, ptr %do.norm.ub, align 4, !tbaa !6
  %do.norm.iv = alloca i32, align 4, !llfort.type_idx !5
  %do.start1 = alloca i32, align 4, !llfort.type_idx !5
  store i32 1, ptr %do.start1, align 4, !tbaa !6
  %do.end2 = alloca i32, align 4, !llfort.type_idx !5
  store i32 26, ptr %do.end2, align 4, !tbaa !6
  %do.step3 = alloca i32, align 4, !llfort.type_idx !5
  store i32 1, ptr %do.step3, align 4, !tbaa !6
  %do.norm.lb4 = alloca i32, align 4, !llfort.type_idx !5
  store i32 0, ptr %do.norm.lb4, align 4, !tbaa !6
  %do.norm.ub5 = alloca i32, align 4, !llfort.type_idx !5
  %do.end_fetch.21 = load i32, ptr %do.end2, align 4, !tbaa !6
  %do.start_fetch.22 = load i32, ptr %do.start1, align 4, !tbaa !6
  %sub.5 = sub nsw i32 %do.end_fetch.21, %do.start_fetch.22
  %do.step_fetch.23 = load i32, ptr %do.step3, align 4, !tbaa !6
  %add.5 = add nsw i32 %sub.5, %do.step_fetch.23
  %do.step_fetch.24 = load i32, ptr %do.step3, align 4, !tbaa !6
  %div.3 = sdiv i32 %add.5, %do.step_fetch.24
  %sub.6 = sub nsw i32 %div.3, 1
  store i32 %sub.6, ptr %do.norm.ub5, align 4, !tbaa !6
  %do.norm.iv6 = alloca i32, align 4, !llfort.type_idx !5
  %omp.pdo.start = alloca i32, align 4, !llfort.type_idx !5
  store i32 1, ptr %omp.pdo.start, align 4, !tbaa !6
  %omp.pdo.end = alloca i32, align 4, !llfort.type_idx !5
  store i32 100, ptr %omp.pdo.end, align 4, !tbaa !6
  %omp.pdo.step = alloca i32, align 4, !llfort.type_idx !5
  store i32 1, ptr %omp.pdo.step, align 4, !tbaa !6
  %omp.pdo.norm.iv = alloca i32, align 4, !llfort.type_idx !5
  %omp.pdo.norm.lb = alloca i32, align 4, !llfort.type_idx !5
  store i32 0, ptr %omp.pdo.norm.lb, align 4, !tbaa !6
  %omp.pdo.norm.ub = alloca i32, align 4, !llfort.type_idx !5
  %omp.pdo.end_fetch.1 = load i32, ptr %omp.pdo.end, align 4, !tbaa !6
  %omp.pdo.start_fetch.2 = load i32, ptr %omp.pdo.start, align 4, !tbaa !6
  %sub.1 = sub nsw i32 %omp.pdo.end_fetch.1, %omp.pdo.start_fetch.2
  %omp.pdo.step_fetch.3 = load i32, ptr %omp.pdo.step, align 4, !tbaa !6
  %add.1 = add nsw i32 %sub.1, %omp.pdo.step_fetch.3
  %omp.pdo.step_fetch.4 = load i32, ptr %omp.pdo.step, align 4, !tbaa !6
  %div.1 = sdiv i32 %add.1, %omp.pdo.step_fetch.4
  %sub.2 = sub nsw i32 %div.1, 1
  store i32 %sub.2, ptr %omp.pdo.norm.ub, align 4, !tbaa !6
  br label %bb_new6

bb_new6:  ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(),
     "QUAL.OMP.SIZES"(i32 4, i32 8, i32 2),
     "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %omp.pdo.norm.iv, i32 0, ptr %do.norm.iv, i32 0, ptr %do.norm.iv6, i32 0),
     "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %omp.pdo.norm.ub, i32 0, ptr %do.norm.ub, i32 0, ptr %do.norm.ub5, i32 0),
     "QUAL.OMP.LIVEIN"(ptr %do.norm.lb4),
     "QUAL.OMP.LIVEIN"(ptr %do.norm.lb),
     "QUAL.OMP.LIVEIN"(ptr %omp.pdo.norm.lb) ]
  %omp.pdo.norm.lb_fetch.5 = load i32, ptr %omp.pdo.norm.lb, align 4, !tbaa !6
  store i32 %omp.pdo.norm.lb_fetch.5, ptr %omp.pdo.norm.iv, align 4, !tbaa !6
  br label %omp.pdo.cond3

omp.pdo.cond3:  ; preds = %do.epilog11, %bb_new6
  %omp.pdo.norm.iv_fetch.6 = load i32, ptr %omp.pdo.norm.iv, align 4, !tbaa !6
  %omp.pdo.norm.ub_fetch.7 = load i32, ptr %omp.pdo.norm.ub, align 4, !tbaa !6
  %rel.1 = icmp sle i32 %omp.pdo.norm.iv_fetch.6, %omp.pdo.norm.ub_fetch.7
  br i1 %rel.1, label %omp.pdo.body4, label %omp.pdo.epilog5

omp.pdo.body4:  ; preds = %omp.pdo.cond3
  %omp.pdo.norm.iv_fetch.8 = load i32, ptr %omp.pdo.norm.iv, align 4, !tbaa !6
  %omp.pdo.step_fetch.9 = load i32, ptr %omp.pdo.step, align 4, !tbaa !6
  %mul.1 = mul nsw i32 %omp.pdo.norm.iv_fetch.8, %omp.pdo.step_fetch.9
  %omp.pdo.start_fetch.10 = load i32, ptr %omp.pdo.start, align 4, !tbaa !6
  %add.2 = add nsw i32 %mul.1, %omp.pdo.start_fetch.10
  store i32 %add.2, ptr %"test_$I", align 8, !tbaa !9
  %do.norm.lb_fetch.15 = load i32, ptr %do.norm.lb, align 4, !tbaa !6
  store i32 %do.norm.lb_fetch.15, ptr %do.norm.iv, align 4, !tbaa !6
  br label %do.cond9

do.cond9:  ; preds = %omp.pdo.body4, %do.epilog15
  %do.norm.iv_fetch.16 = load i32, ptr %do.norm.iv, align 4, !tbaa !6
  %do.norm.ub_fetch.17 = load i32, ptr %do.norm.ub, align 4, !tbaa !6
  %rel.2 = icmp sle i32 %do.norm.iv_fetch.16, %do.norm.ub_fetch.17
  br i1 %rel.2, label %do.body10, label %do.epilog11

do.body10:  ; preds = %do.cond9
  %do.norm.iv_fetch.18 = load i32, ptr %do.norm.iv, align 4, !tbaa !6
  %do.step_fetch.19 = load i32, ptr %do.step, align 4, !tbaa !6
  %mul.2 = mul nsw i32 %do.norm.iv_fetch.18, %do.step_fetch.19
  %do.start_fetch.20 = load i32, ptr %do.start, align 4, !tbaa !6
  %add.4 = add nsw i32 %mul.2, %do.start_fetch.20
  store i32 %add.4, ptr %"test_$J", align 8, !tbaa !11
  %do.norm.lb4_fetch.25 = load i32, ptr %do.norm.lb4, align 4, !tbaa !6
  store i32 %do.norm.lb4_fetch.25, ptr %do.norm.iv6, align 4, !tbaa !6
  br label %do.cond13

do.cond13:  ; preds = %do.body10, %do.body14
  %do.norm.iv6_fetch.26 = load i32, ptr %do.norm.iv6, align 4, !tbaa !6
  %do.norm.ub5_fetch.27 = load i32, ptr %do.norm.ub5, align 4, !tbaa !6
  %rel.3 = icmp sle i32 %do.norm.iv6_fetch.26, %do.norm.ub5_fetch.27
  br i1 %rel.3, label %do.body14, label %do.epilog15

do.body14:  ; preds = %do.cond13
  %do.norm.iv6_fetch.28 = load i32, ptr %do.norm.iv6, align 4, !tbaa !6
  %do.step3_fetch.29 = load i32, ptr %do.step3, align 4, !tbaa !6
  %mul.3 = mul nsw i32 %do.norm.iv6_fetch.28, %do.step3_fetch.29
  %do.start1_fetch.30 = load i32, ptr %do.start1, align 4, !tbaa !6
  %add.6 = add nsw i32 %mul.3, %do.start1_fetch.30
  store i32 %add.6, ptr %"test_$K", align 8, !tbaa !13
  call void @bar_.t0p.t0p.t0p(ptr %"test_$I", ptr %"test_$J", ptr %"test_$K"), !llfort.type_idx !15
  %do.norm.iv6_fetch.31 = load i32, ptr %do.norm.iv6, align 4, !tbaa !6
  %add.7 = add nsw i32 %do.norm.iv6_fetch.31, 1
  store i32 %add.7, ptr %do.norm.iv6, align 4, !tbaa !6
  br label %do.cond13

do.epilog15:  ; preds = %do.cond13
  %do.norm.iv_fetch.32 = load i32, ptr %do.norm.iv, align 4, !tbaa !6
  %add.8 = add nsw i32 %do.norm.iv_fetch.32, 1
  store i32 %add.8, ptr %do.norm.iv, align 4, !tbaa !6
  br label %do.cond9

do.epilog11:  ; preds = %do.cond9
  %omp.pdo.norm.iv_fetch.33 = load i32, ptr %omp.pdo.norm.iv, align 4, !tbaa !6
  %add.9 = add nsw i32 %omp.pdo.norm.iv_fetch.33, 1
  store i32 %add.9, ptr %omp.pdo.norm.iv, align 4, !tbaa !6
  br label %omp.pdo.cond3

omp.pdo.epilog5:  ; preds = %omp.pdo.cond3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TILE"() ]
  ret void

}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind uwtable
define internal void @bar_.t0p.t0p.t0p(ptr %arg0, ptr %arg1, ptr %arg2) #2 {
wrap_start22:
  call void (...) @bar_(ptr %arg0, ptr %arg1, ptr %arg2)
  ret void

}

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #1

declare void @bar_(...)

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
attributes #2 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

!omp_offload.info = !{}
!llvm.module.flags = !{!0}

!0 = !{i32 7, !"openmp", i32 50}
!1 = !{i64 20}
!2 = !{i64 26}
!3 = !{i64 27}
!4 = !{i64 28}
!5 = !{i64 2}
!6 = !{!7, !7, i64 0}
!7 = !{!"Generic Fortran Symbol", !8, i64 0}
!8 = !{!"ifx$root$1$test_"}
!9 = !{!10, !10, i64 0}
!10 = !{!"ifx$unique_sym$1", !7, i64 0}
!11 = !{!12, !12, i64 0}
!12 = !{!"ifx$unique_sym$2", !7, i64 0}
!13 = !{!14, !14, i64 0}
!14 = !{!"ifx$unique_sym$3", !7, i64 0}
!15 = !{i64 30}
; end INTEL_CUSTOMIZATION

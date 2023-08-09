; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-transform  -disable-vpo-paropt-tile=false -S < %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-transform)' -disable-vpo-paropt-tile=false -S < %s | FileCheck %s
; Test src:
;
;         subroutine test()
;         integer :: i, j
;         !$omp tile sizes(2)
;         !$omp tile sizes(4)
;         do i = 1, 100
;           call bar(i)
;         end do
;         end subroutine

; CHECK-DAG: FLOOR.HEAD{{[0-9]*}}
; CHECK-DAG: FLOOR.PREHEAD{{[0-9]*}}

; CHECK-DAG: FLOOR.HEAD{{[0-9]*}}
; CHECK-DAG: FLOOR.PREHEAD{{[0-9]*}}

; CHECK-DAG: FLOOR.LATCH{{[0-9]*}}: ; preds = %FLOOR.HEAD{{[0-9]*}}
; CHECK-DAG: FLOOR.LATCH{{[0-9]*}}

; CHECK-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(),

source_filename = "tile1+tile1.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @test_() #0 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 16, !llfort.type_idx !1
  %"test_$J" = alloca i32, align 8, !llfort.type_idx !2
  %"test_$I" = alloca i32, align 8, !llfort.type_idx !3
  %omp.pdo.start = alloca i32, align 4, !llfort.type_idx !4
  store i32 1, ptr %omp.pdo.start, align 4, !tbaa !5
  %omp.pdo.end = alloca i32, align 4, !llfort.type_idx !4
  store i32 100, ptr %omp.pdo.end, align 4, !tbaa !5
  %omp.pdo.step = alloca i32, align 4, !llfort.type_idx !4
  store i32 1, ptr %omp.pdo.step, align 4, !tbaa !5
  %omp.pdo.norm.iv = alloca i32, align 4, !llfort.type_idx !4
  %omp.pdo.norm.lb = alloca i32, align 4, !llfort.type_idx !4
  store i32 0, ptr %omp.pdo.norm.lb, align 4, !tbaa !5
  %omp.pdo.norm.ub = alloca i32, align 4, !llfort.type_idx !4
  %omp.pdo.end_fetch.1 = load i32, ptr %omp.pdo.end, align 4, !tbaa !5
  %omp.pdo.start_fetch.2 = load i32, ptr %omp.pdo.start, align 4, !tbaa !5
  %sub.1 = sub nsw i32 %omp.pdo.end_fetch.1, %omp.pdo.start_fetch.2
  %omp.pdo.step_fetch.3 = load i32, ptr %omp.pdo.step, align 4, !tbaa !5
  %add.1 = add nsw i32 %sub.1, %omp.pdo.step_fetch.3
  %omp.pdo.step_fetch.4 = load i32, ptr %omp.pdo.step, align 4, !tbaa !5
  %div.1 = sdiv i32 %add.1, %omp.pdo.step_fetch.4
  %sub.2 = sub nsw i32 %div.1, 1
  store i32 %sub.2, ptr %omp.pdo.norm.ub, align 4, !tbaa !5
  br label %bb_new2

bb_new2:  ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(),
     "QUAL.OMP.SIZES"(i32 2) ]
  br label %bb_new7

bb_new7:  ; preds = %bb_new2
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(),
     "QUAL.OMP.SIZES"(i32 4),
     "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %omp.pdo.norm.iv, i32 0),
     "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %omp.pdo.norm.ub, i32 0),
     "QUAL.OMP.LIVEIN"(ptr %omp.pdo.norm.lb) ]
  %omp.pdo.norm.lb_fetch.5 = load i32, ptr %omp.pdo.norm.lb, align 4, !tbaa !5
  store i32 %omp.pdo.norm.lb_fetch.5, ptr %omp.pdo.norm.iv, align 4, !tbaa !5
  br label %omp.pdo.cond4

omp.pdo.cond4:  ; preds = %omp.pdo.body5, %bb_new7
  %omp.pdo.norm.iv_fetch.6 = load i32, ptr %omp.pdo.norm.iv, align 4, !tbaa !5
  %omp.pdo.norm.ub_fetch.7 = load i32, ptr %omp.pdo.norm.ub, align 4, !tbaa !5
  %rel.1 = icmp sle i32 %omp.pdo.norm.iv_fetch.6, %omp.pdo.norm.ub_fetch.7
  br i1 %rel.1, label %omp.pdo.body5, label %omp.pdo.epilog6

omp.pdo.body5:  ; preds = %omp.pdo.cond4
  %omp.pdo.norm.iv_fetch.8 = load i32, ptr %omp.pdo.norm.iv, align 4, !tbaa !5
  %omp.pdo.step_fetch.9 = load i32, ptr %omp.pdo.step, align 4, !tbaa !5
  %mul.1 = mul nsw i32 %omp.pdo.norm.iv_fetch.8, %omp.pdo.step_fetch.9
  %omp.pdo.start_fetch.10 = load i32, ptr %omp.pdo.start, align 4, !tbaa !5
  %add.2 = add nsw i32 %mul.1, %omp.pdo.start_fetch.10
  store i32 %add.2, ptr %"test_$I", align 8, !tbaa !8
  call void @bar_.t0p(ptr %"test_$I"), !llfort.type_idx !10
  %omp.pdo.norm.iv_fetch.11 = load i32, ptr %omp.pdo.norm.iv, align 4, !tbaa !5
  %add.3 = add nsw i32 %omp.pdo.norm.iv_fetch.11, 1
  store i32 %add.3, ptr %omp.pdo.norm.iv, align 4, !tbaa !5
  br label %omp.pdo.cond4

omp.pdo.epilog6:  ; preds = %omp.pdo.cond4
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TILE"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TILE"() ]
  ret void

}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind uwtable
define internal void @bar_.t0p(ptr %arg0) #2 {
wrap_start15:
  call void (...) @bar_(ptr %arg0)
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
!1 = !{i64 19}
!2 = !{i64 24}
!3 = !{i64 25}
!4 = !{i64 2}
!5 = !{!6, !6, i64 0}
!6 = !{!"Generic Fortran Symbol", !7, i64 0}
!7 = !{!"ifx$root$1$test_"}
!8 = !{!9, !9, i64 0}
!9 = !{!"ifx$unique_sym$1", !6, i64 0}
!10 = !{i64 27}
; end INTEL_CUSTOMIZATION

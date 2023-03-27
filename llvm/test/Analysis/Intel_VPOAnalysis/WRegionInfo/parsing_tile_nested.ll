; REQUIRES: asserts
; RUN: opt -opaque-pointers=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers=1 -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -debug -S %s 2>&1 | FileCheck %s
;
; INTEL_CUSTOMIZATION
; Test src:
;   subroutine test()
;   integer :: i, j
;   !$omp tile sizes(4,2)
;   do i = 1, 100
;     !$omp tile sizes(8)
;     do j = 1, 48
;       call bar(i,j)
;     end do
;   end do
;   end subroutine
;
; end INTEL_CUSTOMIZATION
; Nested Case:
; Check the WRN graph for an outer TILE SIZES(4,2) and an
; inner TILE SIZES(8), with one intervening loop level between them.
;
; CHECK: BEGIN TILE ID=1 {
; CHECK:   LIVEIN clause (size=1): (ptr %omp.pdo.norm.{{.*}})
; CHECK:   SIZES clause (size=2): (i32 4) (i32 2)
; CHECK:   IV clause:   %omp.pdo.norm.{{.*}}
; CHECK:   UB clause:   %omp.pdo.norm.{{.*}}
; CHECK:   BEGIN TILE ID=2 {
; CHECK:     LIVEIN clause (size=1): (ptr %omp.pdo.norm.lb)
; CHECK:     SIZES clause (size=1): (i32 8)
; CHECK:     IV clause:   %omp.pdo.norm.iv
; CHECK:     UB clause:   %omp.pdo.norm.ub
; CHECK:   } END TILE ID=2
; CHECK: } END TILE ID=1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define void @test_() #0 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8, !llfort.type_idx !1
  %"test_$J" = alloca i32, align 8, !llfort.type_idx !2
  %"test_$I" = alloca i32, align 8, !llfort.type_idx !3
  %omp.pdo.start1 = alloca i32, align 4, !llfort.type_idx !4
  store i32 1, ptr %omp.pdo.start1, align 4
  %omp.pdo.end2 = alloca i32, align 4, !llfort.type_idx !4
  store i32 100, ptr %omp.pdo.end2, align 4
  %omp.pdo.step3 = alloca i32, align 4, !llfort.type_idx !4
  store i32 1, ptr %omp.pdo.step3, align 4
  %omp.pdo.norm.iv4 = alloca i32, align 4, !llfort.type_idx !4
  %omp.pdo.norm.lb5 = alloca i32, align 4, !llfort.type_idx !4
  store i32 0, ptr %omp.pdo.norm.lb5, align 4
  %omp.pdo.norm.ub6 = alloca i32, align 4, !llfort.type_idx !4
  %omp.pdo.end_fetch.1 = load i32, ptr %omp.pdo.end2, align 4
  %omp.pdo.start_fetch.2 = load i32, ptr %omp.pdo.start1, align 4
  %sub.1 = sub nsw i32 %omp.pdo.end_fetch.1, %omp.pdo.start_fetch.2
  %omp.pdo.step_fetch.3 = load i32, ptr %omp.pdo.step3, align 4
  %div.1 = sdiv i32 %sub.1, %omp.pdo.step_fetch.3
  store i32 %div.1, ptr %omp.pdo.norm.ub6, align 4
  br label %bb_new6

omp.pdo.cond3:                                    ; preds = %bb_new6, %omp.pdo.epilog11
  %omp.pdo.norm.iv_fetch.5 = load i32, ptr %omp.pdo.norm.iv4, align 4
  %omp.pdo.norm.ub_fetch.6 = load i32, ptr %omp.pdo.norm.ub6, align 4
  %rel.1 = icmp sle i32 %omp.pdo.norm.iv_fetch.5, %omp.pdo.norm.ub_fetch.6
  br i1 %rel.1, label %omp.pdo.body4, label %omp.pdo.epilog5

omp.pdo.body4:                                    ; preds = %omp.pdo.cond3
  %omp.pdo.norm.iv_fetch.7 = load i32, ptr %omp.pdo.norm.iv4, align 4
  %omp.pdo.step_fetch.8 = load i32, ptr %omp.pdo.step3, align 4
  %mul.1 = mul nsw i32 %omp.pdo.norm.iv_fetch.7, %omp.pdo.step_fetch.8
  %omp.pdo.start_fetch.9 = load i32, ptr %omp.pdo.start1, align 4
  %add.1 = add nsw i32 %mul.1, %omp.pdo.start_fetch.9
  store i32 %add.1, ptr %"test_$I", align 8
  %omp.pdo.start = alloca i32, align 4, !llfort.type_idx !4
  store i32 1, ptr %omp.pdo.start, align 4
  %omp.pdo.end = alloca i32, align 4, !llfort.type_idx !4
  store i32 48, ptr %omp.pdo.end, align 4
  %omp.pdo.step = alloca i32, align 4, !llfort.type_idx !4
  store i32 1, ptr %omp.pdo.step, align 4
  %omp.pdo.norm.iv = alloca i32, align 4, !llfort.type_idx !4
  %omp.pdo.norm.lb = alloca i32, align 4, !llfort.type_idx !4
  store i32 0, ptr %omp.pdo.norm.lb, align 4
  %omp.pdo.norm.ub = alloca i32, align 4, !llfort.type_idx !4
  %omp.pdo.end_fetch.10 = load i32, ptr %omp.pdo.end, align 4
  %omp.pdo.start_fetch.11 = load i32, ptr %omp.pdo.start, align 4
  %sub.2 = sub nsw i32 %omp.pdo.end_fetch.10, %omp.pdo.start_fetch.11
  %omp.pdo.step_fetch.12 = load i32, ptr %omp.pdo.step, align 4
  %div.2 = sdiv i32 %sub.2, %omp.pdo.step_fetch.12
  store i32 %div.2, ptr %omp.pdo.norm.ub, align 4
  br label %bb_new12

omp.pdo.cond9:                                    ; preds = %bb_new12, %omp.pdo.body10
  %omp.pdo.norm.iv_fetch.14 = load i32, ptr %omp.pdo.norm.iv, align 4
  %omp.pdo.norm.ub_fetch.15 = load i32, ptr %omp.pdo.norm.ub, align 4
  %rel.2 = icmp sle i32 %omp.pdo.norm.iv_fetch.14, %omp.pdo.norm.ub_fetch.15
  br i1 %rel.2, label %omp.pdo.body10, label %omp.pdo.epilog11

omp.pdo.body10:                                   ; preds = %omp.pdo.cond9
  %omp.pdo.norm.iv_fetch.16 = load i32, ptr %omp.pdo.norm.iv, align 4
  %omp.pdo.step_fetch.17 = load i32, ptr %omp.pdo.step, align 4
  %mul.2 = mul nsw i32 %omp.pdo.norm.iv_fetch.16, %omp.pdo.step_fetch.17
  %omp.pdo.start_fetch.18 = load i32, ptr %omp.pdo.start, align 4
  %add.2 = add nsw i32 %mul.2, %omp.pdo.start_fetch.18
  store i32 %add.2, ptr %"test_$J", align 8
  call void @bar_.t0p.t0p(ptr %"test_$I", ptr %"test_$J"), !llfort.type_idx !5
  %omp.pdo.norm.iv_fetch.19 = load i32, ptr %omp.pdo.norm.iv, align 4
  %add.3 = add nsw i32 %omp.pdo.norm.iv_fetch.19, 1
  store i32 %add.3, ptr %omp.pdo.norm.iv, align 4
  br label %omp.pdo.cond9

bb_new12:                                         ; preds = %omp.pdo.body4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(),
    "QUAL.OMP.SIZES"(i32 8),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %omp.pdo.norm.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %omp.pdo.norm.ub, i32 0),
    "QUAL.OMP.LIVEIN"(ptr %omp.pdo.norm.lb) ]
  %omp.pdo.norm.lb_fetch.13 = load i32, ptr %omp.pdo.norm.lb, align 4
  store i32 %omp.pdo.norm.lb_fetch.13, ptr %omp.pdo.norm.iv, align 4
  br label %omp.pdo.cond9

omp.pdo.epilog11:                                 ; preds = %omp.pdo.cond9
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TILE"() ]
  %omp.pdo.norm.iv_fetch.20 = load i32, ptr %omp.pdo.norm.iv4, align 4
  %add.4 = add nsw i32 %omp.pdo.norm.iv_fetch.20, 1
  store i32 %add.4, ptr %omp.pdo.norm.iv4, align 4
  br label %omp.pdo.cond3

bb_new6:                                          ; preds = %alloca_0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TILE"(),
    "QUAL.OMP.SIZES"(i32 4, i32 2),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %omp.pdo.norm.iv4, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %omp.pdo.norm.ub6, i32 0),
    "QUAL.OMP.LIVEIN"(ptr %omp.pdo.norm.lb5) ]
  %omp.pdo.norm.lb_fetch.4 = load i32, ptr %omp.pdo.norm.lb5, align 4
  store i32 %omp.pdo.norm.lb_fetch.4, ptr %omp.pdo.norm.iv4, align 4
  br label %omp.pdo.cond3

omp.pdo.epilog5:                                  ; preds = %omp.pdo.cond3
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TILE"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: noinline nounwind optnone uwtable
define internal void @bar_.t0p.t0p(ptr %arg0, ptr %arg1) #2 {
wrap_start20:
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

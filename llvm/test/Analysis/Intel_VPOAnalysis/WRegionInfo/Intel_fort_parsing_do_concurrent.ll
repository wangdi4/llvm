; INTEL_CUSTOMIZATION
; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-wrncollection -analyze -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(print<vpo-wrncollection>)' -debug -S %s 2>&1 | FileCheck %s
;
; Test src:
; subroutine foo()
;   real a, b
;  do concurrent (i = 0:10000) !! Did not add TARGET and TEAMS to make it simple
;    a = i
;    b = i
;  end do
; end subroutine

; CHECK:  %{{.*}} = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), "QUAL.EXT.DO.CONCURRENT"(), {{.*}} ]
; CHECK: BEGIN GENERICLOOP ID=1 {
; CHECK:   EXT_DO_CONCURRENT

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define void @foo_() #0 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8, !llfort.type_idx !1
  %"foo_$I$_1" = alloca i32, align 8, !llfort.type_idx !2
  %"foo_$B" = alloca float, align 8, !llfort.type_idx !3
  %"foo_$A" = alloca float, align 8, !llfort.type_idx !4
  %do.start = alloca i32, align 4, !llfort.type_idx !5
  store i32 0, ptr %do.start, align 4
  %do.end = alloca i32, align 4, !llfort.type_idx !5
  store i32 10000, ptr %do.end, align 4
  %do.step = alloca i32, align 4, !llfort.type_idx !5
  store i32 1, ptr %do.step, align 4
  %do.norm.lb = alloca i32, align 4, !llfort.type_idx !5
  store i32 0, ptr %do.norm.lb, align 4
  %do.norm.ub = alloca i32, align 4, !llfort.type_idx !5
  %do.end_fetch.1 = load i32, ptr %do.end, align 4
  %do.start_fetch.2 = load i32, ptr %do.start, align 4
  %sub.1 = sub nsw i32 %do.end_fetch.1, %do.start_fetch.2
  %do.step_fetch.3 = load i32, ptr %do.step, align 4
  %div.1 = sdiv i32 %sub.1, %do.step_fetch.3
  store i32 %div.1, ptr %do.norm.ub, align 4
  %do.norm.iv = alloca i32, align 4, !llfort.type_idx !5
  %"foo_$A$local_init" = alloca float, align 4, !llfort.type_idx !4
  %"foo_$B$local_init" = alloca float, align 4, !llfort.type_idx !3
  %"foo_$A_fetch.11" = load float, ptr %"foo_$A", align 8
  store float %"foo_$A_fetch.11", ptr %"foo_$A$local_init", align 4
  %"foo_$B_fetch.14" = load float, ptr %"foo_$B", align 8
  store float %"foo_$B_fetch.14", ptr %"foo_$B$local_init", align 4
  br label %bb_new2

do.cond4:                                         ; preds = %DIR.OMP.GENERICLOOP.2, %do.body5
  %do.norm.iv_fetch.5 = load i32, ptr %do.norm.iv, align 4
  %do.norm.ub_fetch.6 = load i32, ptr %do.norm.ub, align 4
  %rel.1 = icmp sle i32 %do.norm.iv_fetch.5, %do.norm.ub_fetch.6
  br i1 %rel.1, label %do.body5, label %do.epilog6

do.body5:                                         ; preds = %do.cond4
  %do.norm.iv_fetch.7 = load i32, ptr %do.norm.iv, align 4
  %do.step_fetch.8 = load i32, ptr %do.step, align 4
  %mul.1 = mul nsw i32 %do.norm.iv_fetch.7, %do.step_fetch.8
  %do.start_fetch.9 = load i32, ptr %do.start, align 4
  %add.1 = add nsw i32 %mul.1, %do.start_fetch.9
  store i32 %add.1, ptr %"foo_$I$_1", align 8
  %"foo_$A$local_init_fetch.12" = load float, ptr %"foo_$A$local_init", align 4
  store float %"foo_$A$local_init_fetch.12", ptr %"foo_$A", align 8
  %"foo_$B$local_init_fetch.15" = load float, ptr %"foo_$B$local_init", align 4
  store float %"foo_$B$local_init_fetch.15", ptr %"foo_$B", align 8
  %"foo_$I$_1_fetch.10" = load i32, ptr %"foo_$I$_1", align 8
  %"(float)foo_$I$_1_fetch.10$" = sitofp i32 %"foo_$I$_1_fetch.10" to float
  store float %"(float)foo_$I$_1_fetch.10$", ptr %"foo_$A", align 8
  %"foo_$I$_1_fetch.13" = load i32, ptr %"foo_$I$_1", align 8
  %"(float)foo_$I$_1_fetch.13$" = sitofp i32 %"foo_$I$_1_fetch.13" to float
  store float %"(float)foo_$I$_1_fetch.13$", ptr %"foo_$B", align 8
  %do.norm.iv_fetch.16 = load i32, ptr %do.norm.iv, align 4
  %add.2 = add nsw i32 %do.norm.iv_fetch.16, 1
  store i32 %add.2, ptr %do.norm.iv, align 4
  br label %do.cond4

do.epilog6:                                       ; preds = %do.cond4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.GENERICLOOP"() ]
  br label %DIR.OMP.END.GENERICLOOP.1

DIR.OMP.END.GENERICLOOP.1:                        ; preds = %do.epilog6
  ret void

bb_new2:                                          ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.EXT.DO.CONCURRENT"(),
    "QUAL.OMP.COLLAPSE"(i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %do.step, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %do.start, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"foo_$B", float 0.000000e+00, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"foo_$A", float 0.000000e+00, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"foo_$I$_1", i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %"foo_$B$local_init", float 0.000000e+00, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %"foo_$A$local_init", float 0.000000e+00, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %do.norm.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %do.norm.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %do.norm.ub, i32 0) ]
  br label %DIR.OMP.GENERICLOOP.2

DIR.OMP.GENERICLOOP.2:                            ; preds = %bb_new2
  %do.norm.lb_fetch.4 = load i32, ptr %do.norm.lb, align 4
  store i32 %do.norm.lb_fetch.4, ptr %do.norm.iv, align 4
  br label %do.cond4
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "intel-lang"="fortran" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0}

!0 = !{i32 7, !"openmp", i32 50}
!1 = !{i64 18}
!2 = !{i64 23}
!3 = !{i64 24}
!4 = !{i64 25}
!5 = !{i64 2}
; end INTEL_CUSTOMIZATION

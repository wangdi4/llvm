; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -S %s | FileCheck %s

; INTEL_CUSTOMIZATION
; CMPLRLLVM-32892:
; Empty section creates a single dominating switch-case which was not
; accounted for in the DomTree.
; LoopInfo must be updated after section-loop and switch creation.

; Test src:
;
;       integer b,c
;
; !$omp sections
; !$omp end sections
;
; !$omp do collapse(2)
;       do b=1,2
;       do c=1,2
;       end do
;       end do
;       end
; end INTEL_CUSTOMIZATION

; CHECK: entry{{.*}}OMP.SECTIONS
; CHECK-DAG: header{{.*}}:
; CHECK-DAG: body{{.*}}:
; CHECK-DAG: case{{.*}}:
; CHECK-DAG: SECTIONS{{.*}}:
; CHECK-DAG: sw.succ{{.*}}:
; CHECK: exit{{.*}}OMP.END.SECTIONS
; CHECK: entry{{.*}}OMP.LOOP
; CHECK: exit{{.*}}OMP.END.LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@0 = internal unnamed_addr constant i32 2

; Function Attrs: noinline nounwind optnone uwtable
define void @MAIN__() #0 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8, !llfort.type_idx !1
  %"_unnamed_main$$_$C" = alloca i32, align 8, !llfort.type_idx !2
  %"_unnamed_main$$_$B" = alloca i32, align 8, !llfort.type_idx !3
  %func_result = call i32 @for_set_reentrancy(ptr @0), !llfort.type_idx !4
  br label %bb_new4

bb_new4:  ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTIONS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SECTIONS"() ]
  %do.start = alloca i32, align 4, !llfort.type_idx !4
  store i32 1, ptr %do.start, align 4
  %do.end = alloca i32, align 4, !llfort.type_idx !4
  store i32 2, ptr %do.end, align 4
  %do.step = alloca i32, align 4, !llfort.type_idx !4
  store i32 1, ptr %do.step, align 4
  %do.norm.lb = alloca i32, align 4, !llfort.type_idx !4
  store i32 0, ptr %do.norm.lb, align 4
  %do.norm.ub = alloca i32, align 4, !llfort.type_idx !4
  %do.end_fetch.11 = load i32, ptr %do.end, align 4
  %do.start_fetch.12 = load i32, ptr %do.start, align 4
  %sub.3 = sub nsw i32 %do.end_fetch.11, %do.start_fetch.12
  %do.step_fetch.13 = load i32, ptr %do.step, align 4
  %add.3 = add nsw i32 %sub.3, %do.step_fetch.13
  %do.step_fetch.14 = load i32, ptr %do.step, align 4
  %div.2 = sdiv i32 %add.3, %do.step_fetch.14
  %sub.4 = sub nsw i32 %div.2, 1
  store i32 %sub.4, ptr %do.norm.ub, align 4
  %do.norm.iv = alloca i32, align 4, !llfort.type_idx !4
  %omp.pdo.start = alloca i32, align 4, !llfort.type_idx !4
  store i32 1, ptr %omp.pdo.start, align 4
  %omp.pdo.end = alloca i32, align 4, !llfort.type_idx !4
  store i32 2, ptr %omp.pdo.end, align 4
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
  %add.1 = add nsw i32 %sub.1, %omp.pdo.step_fetch.3
  %omp.pdo.step_fetch.4 = load i32, ptr %omp.pdo.step, align 4
  %div.1 = sdiv i32 %add.1, %omp.pdo.step_fetch.4
  %sub.2 = sub nsw i32 %div.1, 1
  store i32 %sub.2, ptr %omp.pdo.norm.ub, align 4
  br label %bb_new9

bb_new9:  ; preds = %bb_new4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.COLLAPSE"(i32 2),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"_unnamed_main$$_$C", i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %"_unnamed_main$$_$B", i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %do.norm.lb, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %omp.pdo.norm.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %omp.pdo.norm.iv, i32 0, ptr %do.norm.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %omp.pdo.norm.ub, i32 0, ptr %do.norm.ub, i32 0) ]
  %omp.pdo.norm.lb_fetch.5 = load i32, ptr %omp.pdo.norm.lb, align 4
  store i32 %omp.pdo.norm.lb_fetch.5, ptr %omp.pdo.norm.iv, align 4
  br label %omp.pdo.cond6

omp.pdo.cond6:  ; preds = %do.epilog14, %bb_new9
  %omp.pdo.norm.iv_fetch.6 = load i32, ptr %omp.pdo.norm.iv, align 4
  %omp.pdo.norm.ub_fetch.7 = load i32, ptr %omp.pdo.norm.ub, align 4
  %rel.1 = icmp sle i32 %omp.pdo.norm.iv_fetch.6, %omp.pdo.norm.ub_fetch.7
  br i1 %rel.1, label %omp.pdo.body7, label %omp.pdo.epilog8

omp.pdo.body7:  ; preds = %omp.pdo.cond6
  %omp.pdo.norm.iv_fetch.8 = load i32, ptr %omp.pdo.norm.iv, align 4
  %omp.pdo.step_fetch.9 = load i32, ptr %omp.pdo.step, align 4
  %mul.1 = mul nsw i32 %omp.pdo.norm.iv_fetch.8, %omp.pdo.step_fetch.9
  %omp.pdo.start_fetch.10 = load i32, ptr %omp.pdo.start, align 4
  %add.2 = add nsw i32 %mul.1, %omp.pdo.start_fetch.10
  store i32 %add.2, ptr %"_unnamed_main$$_$B", align 8
  %do.norm.lb_fetch.15 = load i32, ptr %do.norm.lb, align 4
  store i32 %do.norm.lb_fetch.15, ptr %do.norm.iv, align 4
  br label %do.cond12

do.cond12:  ; preds = %omp.pdo.body7, %do.body13
  %do.norm.iv_fetch.16 = load i32, ptr %do.norm.iv, align 4
  %do.norm.ub_fetch.17 = load i32, ptr %do.norm.ub, align 4
  %rel.2 = icmp sle i32 %do.norm.iv_fetch.16, %do.norm.ub_fetch.17
  br i1 %rel.2, label %do.body13, label %do.epilog14

do.body13:  ; preds = %do.cond12
  %do.norm.iv_fetch.18 = load i32, ptr %do.norm.iv, align 4
  %do.step_fetch.19 = load i32, ptr %do.step, align 4
  %mul.2 = mul nsw i32 %do.norm.iv_fetch.18, %do.step_fetch.19
  %do.start_fetch.20 = load i32, ptr %do.start, align 4
  %add.4 = add nsw i32 %mul.2, %do.start_fetch.20
  store i32 %add.4, ptr %"_unnamed_main$$_$C", align 8
  %do.norm.iv_fetch.21 = load i32, ptr %do.norm.iv, align 4
  %add.5 = add nsw i32 %do.norm.iv_fetch.21, 1
  store i32 %add.5, ptr %do.norm.iv, align 4
  br label %do.cond12

do.epilog14:  ; preds = %do.cond12
  %omp.pdo.norm.iv_fetch.22 = load i32, ptr %omp.pdo.norm.iv, align 4
  %add.6 = add nsw i32 %omp.pdo.norm.iv_fetch.22, 1
  store i32 %add.6, ptr %omp.pdo.norm.iv, align 4
  br label %omp.pdo.cond6

omp.pdo.epilog8:  ; preds = %omp.pdo.cond6
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.LOOP"() ]
  ret void

}

declare !llfort.intrin_id !5 !llfort.type_idx !6 !intel_dtrans_type !7 !intel.dtrans.func.type !10 i32 @for_set_reentrancy(ptr nocapture readonly "intel_dtrans_func_index"="1" %0)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "intel-lang"="fortran" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0}

!0 = !{i32 7, !"openmp", i32 50}
!1 = !{i64 18}
!2 = !{i64 22}
!3 = !{i64 23}
!4 = !{i64 2}
!5 = !{i32 99}
!6 = !{i64 24}
!7 = !{!8, i32 0}
!8 = !{!"F", i1 false, i32 1, !9, !9}
!9 = !{i32 0, i32 0}
!10 = distinct !{!9}

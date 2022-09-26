; RUN: opt -enable-new-pm=0 -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,simplifycfg,loop-simplifycfg,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; subroutine ntscale(A, B, N)
;   implicit none
;
;   integer, intent(in) :: N
;   real, intent(in) :: B(N)
;   real, intent(out) :: A(:)
;
;   integer :: i
;
; !$OMP SIMD NONTEMPORAL(A)
;   do i=1,N
;     A(i) = 2.0*B(i)
;   end do
; end subroutine ntscale

; Check that paropt pass annotates store to A with nontemporal metadata.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$float*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

; Function Attrs: noinline nounwind optnone uwtable
define void @ntscale_(ptr noalias dereferenceable(72) "assumed_shape" "ptrnoalias" %"ntscale_$A$argptr", ptr noalias readonly dereferenceable(4) %"ntscale_$B$argptr", ptr noalias readonly dereferenceable(4) %"ntscale_$N$argptr") #0 !llfort.type_idx !1 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8, !llfort.type_idx !2
  %"ntscale_$A$locptr" = alloca ptr, align 8
  %"ntscale_$B$locptr" = alloca ptr, align 8
  %"ntscale_$N$locptr" = alloca ptr, align 8
  %"ntscale_$I" = alloca i32, align 8, !llfort.type_idx !3
  %"var$2" = alloca i32, align 4, !llfort.type_idx !4
  store ptr %"ntscale_$A$argptr", ptr %"ntscale_$A$locptr", align 1
  %"ntscale_$A.1" = load ptr, ptr %"ntscale_$A$locptr", align 1
  store ptr %"ntscale_$B$argptr", ptr %"ntscale_$B$locptr", align 1
  %"ntscale_$B.2" = load ptr, ptr %"ntscale_$B$locptr", align 1
  store ptr %"ntscale_$N$argptr", ptr %"ntscale_$N$locptr", align 1
  %"ntscale_$N.3" = load ptr, ptr %"ntscale_$N$locptr", align 1
  %"ntscale_$N.3_fetch.4" = load i32, ptr %"ntscale_$N.3", align 1, !llfort.type_idx !5
  store i32 %"ntscale_$N.3_fetch.4", ptr %"var$2", align 4
  %"ntscale_$A.1.addr_a0$5" = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %"ntscale_$A.1", i32 0, i32 0
  %"ntscale_$A.1.addr_a0$_fetch.5" = load ptr, ptr %"ntscale_$A.1.addr_a0$5", align 1, !llfort.type_idx !6
  %omp.pdo.start = alloca i32, align 4, !llfort.type_idx !4
  store i32 1, ptr %omp.pdo.start, align 4
  %omp.pdo.end = alloca i32, align 4, !llfort.type_idx !4
  %"ntscale_$N.3_fetch.6" = load i32, ptr %"ntscale_$N.3", align 1, !llfort.type_idx !5
  store i32 %"ntscale_$N.3_fetch.6", ptr %omp.pdo.end, align 4
  %omp.pdo.step = alloca i32, align 4, !llfort.type_idx !4
  store i32 1, ptr %omp.pdo.step, align 4
  %omp.pdo.norm.iv = alloca i32, align 4, !llfort.type_idx !4
  %omp.pdo.norm.lb = alloca i32, align 4, !llfort.type_idx !4
  store i32 0, ptr %omp.pdo.norm.lb, align 4
  %omp.pdo.norm.ub = alloca i32, align 4, !llfort.type_idx !4
  %omp.pdo.end_fetch.7 = load i32, ptr %omp.pdo.end, align 4, !llfort.type_idx !4
  %omp.pdo.start_fetch.8 = load i32, ptr %omp.pdo.start, align 4, !llfort.type_idx !4
  %sub.1 = sub nsw i32 %omp.pdo.end_fetch.7, %omp.pdo.start_fetch.8
  %omp.pdo.step_fetch.9 = load i32, ptr %omp.pdo.step, align 4, !llfort.type_idx !4
  %add.1 = add nsw i32 %sub.1, %omp.pdo.step_fetch.9
  %omp.pdo.step_fetch.10 = load i32, ptr %omp.pdo.step, align 4, !llfort.type_idx !4
  %div.1 = sdiv i32 %add.1, %omp.pdo.step_fetch.10
  %sub.2 = sub nsw i32 %div.1, 1
  store i32 %sub.2, ptr %omp.pdo.norm.ub, align 4
  br label %bb_new7

bb_new7:  ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LINEAR:TYPED.IV"(ptr %"ntscale_$I", i32 0, i32 1, i32 1),
    "QUAL.OMP.NONTEMPORAL:F90_DV"(ptr %"ntscale_$A.1"),
    "QUAL.OMP.LIVEIN:F90_DV"(ptr %"ntscale_$A.1"),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %omp.pdo.norm.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %omp.pdo.norm.ub, i32 0),
    "QUAL.OMP.LIVEIN"(ptr %"ntscale_$B.2"),
    "QUAL.OMP.LIVEIN"(ptr %"ntscale_$N.3") ]
  %omp.pdo.norm.lb_fetch.11 = load i32, ptr %omp.pdo.norm.lb, align 4, !llfort.type_idx !4
  store i32 %omp.pdo.norm.lb_fetch.11, ptr %omp.pdo.norm.iv, align 4
  br label %omp.pdo.cond4

omp.pdo.cond4:  ; preds = %omp.pdo.body5, %bb_new7
  %omp.pdo.norm.iv_fetch.12 = load i32, ptr %omp.pdo.norm.iv, align 4, !llfort.type_idx !4
  %omp.pdo.norm.ub_fetch.13 = load i32, ptr %omp.pdo.norm.ub, align 4, !llfort.type_idx !4
  %rel.1 = icmp sle i32 %omp.pdo.norm.iv_fetch.12, %omp.pdo.norm.ub_fetch.13
  br i1 %rel.1, label %omp.pdo.body5, label %omp.pdo.epilog6

omp.pdo.body5:  ; preds = %omp.pdo.cond4
  %omp.pdo.norm.iv_fetch.14 = load i32, ptr %omp.pdo.norm.iv, align 4, !llfort.type_idx !4
  %omp.pdo.step_fetch.15 = load i32, ptr %omp.pdo.step, align 4, !llfort.type_idx !4
  %mul.1 = mul nsw i32 %omp.pdo.norm.iv_fetch.14, %omp.pdo.step_fetch.15
  %omp.pdo.start_fetch.16 = load i32, ptr %omp.pdo.start, align 4, !llfort.type_idx !4
  %add.2 = add nsw i32 %mul.1, %omp.pdo.start_fetch.16
  store i32 %add.2, ptr %"ntscale_$I", align 8
  %"ntscale_$I_fetch.17" = load i32, ptr %"ntscale_$I", align 8, !llfort.type_idx !3
  %int_sext = sext i32 %"ntscale_$I_fetch.17" to i64, !llfort.type_idx !7
  %"ntscale_$B.2[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %"ntscale_$B.2", i64 %int_sext), !llfort.type_idx !8
  %"ntscale_$B.2[]_fetch.18" = load float, ptr %"ntscale_$B.2[]", align 1, !llfort.type_idx !8
  %mul.2 = fmul reassoc ninf nsz arcp contract afn float 2.000000e+00, %"ntscale_$B.2[]_fetch.18"
  %"ntscale_$A.1.addr_a0$" = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %"ntscale_$A.1", i32 0, i32 0
  %"ntscale_$A.1.addr_a0$_fetch.19" = load ptr, ptr %"ntscale_$A.1.addr_a0$", align 1, !llfort.type_idx !6
  %"ntscale_$A.1.dim_info$" = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %"ntscale_$A.1", i32 0, i32 6, i32 0
  %"ntscale_$A.1.dim_info$.spacing$" = getelementptr inbounds { i64, i64, i64 }, ptr %"ntscale_$A.1.dim_info$", i32 0, i32 1
  %"ntscale_$A.1.dim_info$.spacing$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) %"ntscale_$A.1.dim_info$.spacing$", i32 0), !llfort.type_idx !9
  %"ntscale_$A.1.dim_info$.spacing$[]_fetch.20" = load i64, ptr %"ntscale_$A.1.dim_info$.spacing$[]", align 1, !llfort.type_idx !9
  %"ntscale_$I_fetch.21" = load i32, ptr %"ntscale_$I", align 8, !llfort.type_idx !3
  %int_sext1 = sext i32 %"ntscale_$I_fetch.21" to i64, !llfort.type_idx !7
  %"ntscale_$A.1.dim_info$2" = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %"ntscale_$A.1", i32 0, i32 6, i32 0
  %"ntscale_$A.1.dim_info$.spacing$3" = getelementptr inbounds { i64, i64, i64 }, ptr %"ntscale_$A.1.dim_info$2", i32 0, i32 1
  %"ntscale_$A.1.dim_info$.spacing$[]4" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) %"ntscale_$A.1.dim_info$.spacing$3", i32 0), !llfort.type_idx !10
  %"ntscale_$A.1.dim_info$.spacing$[]_fetch.22" = load i64, ptr %"ntscale_$A.1.dim_info$.spacing$[]4", align 1, !llfort.type_idx !10
  %"ntscale_$A.1.addr_a0$_fetch.19[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %"ntscale_$A.1.dim_info$.spacing$[]_fetch.20", ptr elementtype(float) %"ntscale_$A.1.addr_a0$_fetch.19", i64 %int_sext1), !llfort.type_idx !11
  store float %mul.2, ptr %"ntscale_$A.1.addr_a0$_fetch.19[]", align 1
; CHECK: store float %mul.2, ptr %"ntscale_$A.1.addr_a0$_fetch.19[]", align 1, !nontemporal ![[NTMD:[0-9]+]]
  %"ntscale_$I_fetch.23" = load i32, ptr %"ntscale_$I", align 8, !llfort.type_idx !3
  %omp.pdo.step_fetch.24 = load i32, ptr %omp.pdo.step, align 4, !llfort.type_idx !4
  %add.3 = add nsw i32 %"ntscale_$I_fetch.23", %omp.pdo.step_fetch.24
  store i32 %add.3, ptr %"ntscale_$I", align 8
  %omp.pdo.norm.iv_fetch.25 = load i32, ptr %omp.pdo.norm.iv, align 4, !llfort.type_idx !4
  %add.4 = add nsw i32 %omp.pdo.norm.iv_fetch.25, 1
  store i32 %add.4, ptr %omp.pdo.norm.iv, align 4
  br label %omp.pdo.cond4

omp.pdo.epilog6:  ; preds = %omp.pdo.cond4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void

}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #1

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 %0, i64 %1, i64 %2, ptr %3, i64 %4) #2

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 %0, i64 %1, i32 %2, ptr %3, i32 %4) #2

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "intel-lang"="fortran" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
attributes #2 = { nounwind readnone speculatable }

!omp_offload.info = !{}
!llvm.module.flags = !{!0}

!0 = !{i32 7, !"openmp", i32 50}
!1 = !{i64 58}
!2 = !{i64 41}
!3 = !{i64 59}
!4 = !{i64 2}
!5 = !{i64 57}
!6 = !{i64 5}
!7 = !{i64 3}
!8 = !{i64 62}
!9 = !{i64 63}
!10 = !{i64 64}
!11 = !{i64 65}

; CHECK: ![[NTMD]] = !{i32 1}

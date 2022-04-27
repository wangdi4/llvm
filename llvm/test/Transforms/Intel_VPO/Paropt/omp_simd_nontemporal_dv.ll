; RUN: opt -enable-new-pm=0 -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,simplifycfg,loop-simplifycfg,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:
;
; subroutine ntscale(A, B, N)
;   implicit none
;
;   integer, intent(in) :: N
;   real, intent(in) :: B(N)
;   real, intent(out) :: A(N)
;
;   integer :: i
;
; !$OMP SIMD NONTEMPORAL(A)
;   do i=1,N
;     A(i) = 2.0*B(i)
;   end do
; end subroutine ntscale
;
; Check that paropt pass annotates store to A with nontemporal metadata.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$float*$rank1$.0" = type { float*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank1$.1" = type { float*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

define void @ntscale_(float* dereferenceable(4) "ptrnoalias" %"ntscale_$A", float* readonly dereferenceable(4) "ptrnoalias" %"ntscale_$B", i32* readonly dereferenceable(4) %"ntscale_$N") #1 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 16
  %"var$3" = alloca %"QNCA_a0$float*$rank1$.0", align 8
  %"var$4" = alloca i32, align 4
  %"var$5" = alloca %"QNCA_a0$float*$rank1$.1", align 8
  %"ntscale_$I" = alloca i32, align 8
  %"ntscale_$N_fetch.5" = load i32, i32* %"ntscale_$N", align 1
  store i32 %"ntscale_$N_fetch.5", i32* %"var$4", align 1
  %"var$3.flags$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", %"QNCA_a0$float*$rank1$.0"* %"var$3", i32 0, i32 3
  store i64 1, i64* %"var$3.flags$", align 1
  %"var$4_fetch.1" = load i32, i32* %"var$4", align 1
  %int_sext = sext i32 %"var$4_fetch.1" to i64
  %rel.1 = icmp sgt i64 0, %int_sext
  %slct.1 = select i1 %rel.1, i64 0, i64 %int_sext
  %"var$3.addr_length$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", %"QNCA_a0$float*$rank1$.0"* %"var$3", i32 0, i32 1
  store i64 4, i64* %"var$3.addr_length$", align 1
  %"var$3.dim$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", %"QNCA_a0$float*$rank1$.0"* %"var$3", i32 0, i32 4
  store i64 1, i64* %"var$3.dim$", align 1
  %"var$3.codim$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", %"QNCA_a0$float*$rank1$.0"* %"var$3", i32 0, i32 2
  store i64 0, i64* %"var$3.codim$", align 1
  %"var$3.dim_info$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", %"QNCA_a0$float*$rank1$.0"* %"var$3", i32 0, i32 6, i32 0
  %"var$3.dim_info$.spacing$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$3.dim_info$", i32 0, i32 1
  %"var$3.dim_info$.spacing$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"var$3.dim_info$.spacing$", i32 0)
  store i64 4, i64* %"var$3.dim_info$.spacing$[]", align 1
  %"var$3.dim_info$1" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", %"QNCA_a0$float*$rank1$.0"* %"var$3", i32 0, i32 6, i32 0
  %"var$3.dim_info$.lower_bound$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$3.dim_info$1", i32 0, i32 2
  %"var$3.dim_info$.lower_bound$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"var$3.dim_info$.lower_bound$", i32 0)
  store i64 1, i64* %"var$3.dim_info$.lower_bound$[]", align 1
  %"var$3.dim_info$2" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", %"QNCA_a0$float*$rank1$.0"* %"var$3", i32 0, i32 6, i32 0
  %"var$3.dim_info$.extent$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$3.dim_info$2", i32 0, i32 0
  %"var$3.dim_info$.extent$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"var$3.dim_info$.extent$", i32 0)
  store i64 %slct.1, i64* %"var$3.dim_info$.extent$[]", align 1
  %"var$3.addr_a0$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", %"QNCA_a0$float*$rank1$.0"* %"var$3", i32 0, i32 0
  store float* %"ntscale_$A", float** %"var$3.addr_a0$", align 1
  %"var$3.flags$3" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", %"QNCA_a0$float*$rank1$.0"* %"var$3", i32 0, i32 3
  %"var$3.flags$_fetch.2" = load i64, i64* %"var$3.flags$3", align 1
  %or.1 = or i64 %"var$3.flags$_fetch.2", 1
  %"var$3.flags$4" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", %"QNCA_a0$float*$rank1$.0"* %"var$3", i32 0, i32 3
  store i64 %or.1, i64* %"var$3.flags$4", align 1
  %"var$5.flags$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", %"QNCA_a0$float*$rank1$.1"* %"var$5", i32 0, i32 3
  store i64 1, i64* %"var$5.flags$", align 1
  %"var$4_fetch.3" = load i32, i32* %"var$4", align 1
  %int_sext5 = sext i32 %"var$4_fetch.3" to i64
  %rel.2 = icmp sgt i64 0, %int_sext5
  %slct.2 = select i1 %rel.2, i64 0, i64 %int_sext5
  %"var$5.addr_length$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", %"QNCA_a0$float*$rank1$.1"* %"var$5", i32 0, i32 1
  store i64 4, i64* %"var$5.addr_length$", align 1
  %"var$5.dim$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", %"QNCA_a0$float*$rank1$.1"* %"var$5", i32 0, i32 4
  store i64 1, i64* %"var$5.dim$", align 1
  %"var$5.codim$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", %"QNCA_a0$float*$rank1$.1"* %"var$5", i32 0, i32 2
  store i64 0, i64* %"var$5.codim$", align 1
  %"var$5.dim_info$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", %"QNCA_a0$float*$rank1$.1"* %"var$5", i32 0, i32 6, i32 0
  %"var$5.dim_info$.spacing$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$5.dim_info$", i32 0, i32 1
  %"var$5.dim_info$.spacing$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"var$5.dim_info$.spacing$", i32 0)
  store i64 4, i64* %"var$5.dim_info$.spacing$[]", align 1
  %"var$5.dim_info$6" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", %"QNCA_a0$float*$rank1$.1"* %"var$5", i32 0, i32 6, i32 0
  %"var$5.dim_info$.lower_bound$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$5.dim_info$6", i32 0, i32 2
  %"var$5.dim_info$.lower_bound$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"var$5.dim_info$.lower_bound$", i32 0)
  store i64 1, i64* %"var$5.dim_info$.lower_bound$[]", align 1
  %"var$5.dim_info$7" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", %"QNCA_a0$float*$rank1$.1"* %"var$5", i32 0, i32 6, i32 0
  %"var$5.dim_info$.extent$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$5.dim_info$7", i32 0, i32 0
  %"var$5.dim_info$.extent$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"var$5.dim_info$.extent$", i32 0)
  store i64 %slct.2, i64* %"var$5.dim_info$.extent$[]", align 1
  %"var$5.addr_a0$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", %"QNCA_a0$float*$rank1$.1"* %"var$5", i32 0, i32 0
  store float* %"ntscale_$B", float** %"var$5.addr_a0$", align 1
  %"var$5.flags$8" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", %"QNCA_a0$float*$rank1$.1"* %"var$5", i32 0, i32 3
  %"var$5.flags$_fetch.4" = load i64, i64* %"var$5.flags$8", align 1
  %or.2 = or i64 %"var$5.flags$_fetch.4", 1
  %"var$5.flags$9" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", %"QNCA_a0$float*$rank1$.1"* %"var$5", i32 0, i32 3
  store i64 %or.2, i64* %"var$5.flags$9", align 1
  %"ntscale_$N_fetch.7" = load i32, i32* %"ntscale_$N", align 1
  %omp.pdo.start = alloca i32, align 4
  %omp.pdo.end = alloca i32, align 4
  %omp.pdo.step = alloca i32, align 4
  store i32 1, i32* %omp.pdo.start, align 1
  store i32 %"ntscale_$N_fetch.7", i32* %omp.pdo.end, align 1
  store i32 1, i32* %omp.pdo.step, align 1
  %omp.pdo.start_fetch.8 = load i32, i32* %omp.pdo.start, align 1
  store i32 %omp.pdo.start_fetch.8, i32* %"ntscale_$I", align 1
  %omp.pdo.norm.lb = alloca i32, align 4
  %omp.pdo.norm.iv = alloca i32, align 4
  %omp.pdo.norm.ub = alloca i32, align 4
  store i32 0, i32* %omp.pdo.norm.lb, align 1
  store i32 0, i32* %omp.pdo.norm.iv, align 1
  %omp.pdo.end_fetch.9 = load i32, i32* %omp.pdo.end, align 1
  %omp.pdo.start_fetch.10 = load i32, i32* %omp.pdo.start, align 1
  %sub.3 = sub nsw i32 %omp.pdo.end_fetch.9, %omp.pdo.start_fetch.10
  %omp.pdo.step_fetch.11 = load i32, i32* %omp.pdo.step, align 1
  %div.1 = sdiv i32 %sub.3, %omp.pdo.step_fetch.11
  store i32 %div.1, i32* %omp.pdo.norm.ub, align 1
  br label %bb_new5

bb_new5:                                          ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV"(i32* %"ntscale_$I", i32 1), "QUAL.OMP.NONTEMPORAL:F90_DV"(%"QNCA_a0$float*$rank1$.0"* %"var$3"), "QUAL.OMP.NORMALIZED.IV"(i32* %omp.pdo.norm.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %omp.pdo.norm.ub) ]
  %omp.pdo.norm.lb_fetch.12 = load i32, i32* %omp.pdo.norm.lb, align 1
  store i32 %omp.pdo.norm.lb_fetch.12, i32* %omp.pdo.norm.iv, align 1
  %omp.pdo.norm.ub_fetch.13 = load i32, i32* %omp.pdo.norm.ub, align 1
  %omp.pdo.norm.iv_fetch.14 = load i32, i32* %omp.pdo.norm.iv, align 1
  %rel.3 = icmp slt i32 %omp.pdo.norm.ub_fetch.13, %omp.pdo.norm.iv_fetch.14
  br i1 %rel.3, label %omp.pdo.after9, label %omp.pdo.top7

omp.pdo.top7:                                     ; preds = %bb_new5
  store i32 0, i32* %omp.pdo.norm.iv, align 1
  %omp.pdo.norm.iv_fetch.15 = load i32, i32* %omp.pdo.norm.iv, align 1
  %omp.pdo.step_fetch.16 = load i32, i32* %omp.pdo.step, align 1
  %mul.1 = mul nsw i32 %omp.pdo.norm.iv_fetch.15, %omp.pdo.step_fetch.16
  %omp.pdo.start_fetch.17 = load i32, i32* %omp.pdo.start, align 1
  %add.1 = add nsw i32 %mul.1, %omp.pdo.start_fetch.17
  store i32 %add.1, i32* %"ntscale_$I", align 1
  br label %bb1

bb1:                                              ; preds = %bb1, %omp.pdo.top7
  %omp.pdo.norm.iv_fetch.18 = load i32, i32* %omp.pdo.norm.iv, align 1
  %omp.pdo.step_fetch.19 = load i32, i32* %omp.pdo.step, align 1
  %mul.2 = mul nsw i32 %omp.pdo.norm.iv_fetch.18, %omp.pdo.step_fetch.19
  %omp.pdo.start_fetch.20 = load i32, i32* %omp.pdo.start, align 1
  %add.2 = add nsw i32 %mul.2, %omp.pdo.start_fetch.20
  store i32 %add.2, i32* %"ntscale_$I", align 1
  %"var$5.addr_a0$10" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", %"QNCA_a0$float*$rank1$.1"* %"var$5", i32 0, i32 0
  %"var$5.addr_a0$_fetch.21" = load float*, float** %"var$5.addr_a0$10", align 1
  %"ntscale_$I_fetch.22" = load i32, i32* %"ntscale_$I", align 1
  %int_sext11 = sext i32 %"ntscale_$I_fetch.22" to i64
  %"var$5.addr_a0$_fetch.21[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %"var$5.addr_a0$_fetch.21", i64 %int_sext11)
  %"var$5.addr_a0$_fetch.21[]_fetch.23" = load float, float* %"var$5.addr_a0$_fetch.21[]", align 1
  %mul.3 = fmul reassoc ninf nsz arcp contract afn float 2.000000e+00, %"var$5.addr_a0$_fetch.21[]_fetch.23"
  %"var$3.addr_a0$12" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", %"QNCA_a0$float*$rank1$.0"* %"var$3", i32 0, i32 0
  %"var$3.addr_a0$_fetch.24" = load float*, float** %"var$3.addr_a0$12", align 1
  %"ntscale_$I_fetch.25" = load i32, i32* %"ntscale_$I", align 1
  %int_sext13 = sext i32 %"ntscale_$I_fetch.25" to i64
  %"var$3.addr_a0$_fetch.24[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %"var$3.addr_a0$_fetch.24", i64 %int_sext13)
  store float %mul.3, float* %"var$3.addr_a0$_fetch.24[]", align 1
; CHECK: store float %mul.3, float* %"var$3.addr_a0$_fetch.24[]", align 1, !nontemporal ![[NTMD:[0-9]+]]
  %omp.pdo.step_fetch.26 = load i32, i32* %omp.pdo.step, align 1
  %omp.pdo.norm.iv_fetch.27 = load i32, i32* %omp.pdo.norm.iv, align 1
  %add.3 = add nsw i32 %omp.pdo.norm.iv_fetch.27, 1
  store i32 %add.3, i32* %omp.pdo.norm.iv, align 1
  %omp.pdo.norm.iv_fetch.28 = load i32, i32* %omp.pdo.norm.iv, align 1
  %omp.pdo.norm.ub_fetch.29 = load i32, i32* %omp.pdo.norm.ub, align 1
  %rel.4 = icmp sle i32 %omp.pdo.norm.iv_fetch.28, %omp.pdo.norm.ub_fetch.29
  br i1 %rel.4, label %bb1, label %omp.pdo.after9

omp.pdo.after9:                                   ; preds = %bb1, %bb_new5
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64)

declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32)

; CHECK: ![[NTMD]] = !{i32 1}

; INTEL_CUSTOMIZATION
; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:
; subroutine test_01(a, b, size)
;   implicit none
;
;   integer, intent(in) :: size
;   real, intent(in) :: b(size)
;   real, intent(out) :: a(size)
;
;   integer :: i
;   real :: tmp
;
; !$OMP SIMD NONTEMPORAL(a, b)
;   do i=1,size
;     tmp = b(i)
;     a(i) = tmp * tmp + tmp + 1.0
;   end do
; end subroutine test_01
;
; Check that subscript intrinsics as generated in Fortran do not interfere with
; the translation of the NONTEMPORAL clause into !nontemporal metadata.
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$float*$rank1$.0" = type { float*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank1$.1" = type { float*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

; Function Attrs: nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #0

; Function Attrs: nounwind uwtable
define void @test_01_(float* dereferenceable(4) "ptrnoalias" %"test_01_$A", float* readonly dereferenceable(4) "ptrnoalias" %"test_01_$B", i64* readonly dereferenceable(8) %"test_01_$SIZE") local_unnamed_addr #1 {
DIR.OMP.SIMD.2:
  %"var$3" = alloca %"QNCA_a0$float*$rank1$.0", align 8
  %"var$5" = alloca %"QNCA_a0$float*$rank1$.1", align 8
  %"test_01_$I" = alloca i64, align 8
  %"test_01_$SIZE_fetch" = load i64, i64* %"test_01_$SIZE", align 1
  %"var$3.flags$1" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", %"QNCA_a0$float*$rank1$.0"* %"var$3", i64 0, i32 3
  store i64 1, i64* %"var$3.flags$1", align 8
  %0 = icmp sgt i64 %"test_01_$SIZE_fetch", 0
  %slct4 = select i1 %0, i64 %"test_01_$SIZE_fetch", i64 0
  %"var$3.addr_length$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", %"QNCA_a0$float*$rank1$.0"* %"var$3", i64 0, i32 1
  store i64 4, i64* %"var$3.addr_length$", align 8
  %"var$3.dim$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", %"QNCA_a0$float*$rank1$.0"* %"var$3", i64 0, i32 4
  store i64 1, i64* %"var$3.dim$", align 8
  %"var$3.codim$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", %"QNCA_a0$float*$rank1$.0"* %"var$3", i64 0, i32 2
  store i64 0, i64* %"var$3.codim$", align 8
  %"var$3.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", %"QNCA_a0$float*$rank1$.0"* %"var$3", i64 0, i32 6, i64 0, i32 1
  %"var$3.dim_info$.spacing$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$3.dim_info$.spacing$", i32 0)
  store i64 4, i64* %"var$3.dim_info$.spacing$[]", align 1
  %"var$3.dim_info$.lower_bound$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", %"QNCA_a0$float*$rank1$.0"* %"var$3", i64 0, i32 6, i64 0, i32 2
  %"var$3.dim_info$.lower_bound$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$3.dim_info$.lower_bound$", i32 0)
  store i64 1, i64* %"var$3.dim_info$.lower_bound$[]", align 1
  %"var$3.dim_info$.extent$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", %"QNCA_a0$float*$rank1$.0"* %"var$3", i64 0, i32 6, i64 0, i32 0
  %"var$3.dim_info$.extent$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$3.dim_info$.extent$", i32 0)
  store i64 %slct4, i64* %"var$3.dim_info$.extent$[]", align 1
  %"var$3.addr_a0$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.0", %"QNCA_a0$float*$rank1$.0"* %"var$3", i64 0, i32 0
  store float* %"test_01_$A", float** %"var$3.addr_a0$", align 8
  %"var$3.flags$_fetch" = load i64, i64* %"var$3.flags$1", align 8
  %or = or i64 %"var$3.flags$_fetch", 1
  store i64 %or, i64* %"var$3.flags$1", align 8
  %"var$5.flags$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", %"QNCA_a0$float*$rank1$.1"* %"var$5", i64 0, i32 3
  store i64 1, i64* %"var$5.flags$", align 8
  %"var$5.addr_length$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", %"QNCA_a0$float*$rank1$.1"* %"var$5", i64 0, i32 1
  store i64 4, i64* %"var$5.addr_length$", align 8
  %"var$5.dim$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", %"QNCA_a0$float*$rank1$.1"* %"var$5", i64 0, i32 4
  store i64 1, i64* %"var$5.dim$", align 8
  %"var$5.codim$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", %"QNCA_a0$float*$rank1$.1"* %"var$5", i64 0, i32 2
  store i64 0, i64* %"var$5.codim$", align 8
  %"var$5.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", %"QNCA_a0$float*$rank1$.1"* %"var$5", i64 0, i32 6, i64 0, i32 1
  %"var$5.dim_info$.spacing$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$5.dim_info$.spacing$", i32 0)
  store i64 4, i64* %"var$5.dim_info$.spacing$[]", align 1
  %"var$5.dim_info$.lower_bound$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", %"QNCA_a0$float*$rank1$.1"* %"var$5", i64 0, i32 6, i64 0, i32 2
  %"var$5.dim_info$.lower_bound$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$5.dim_info$.lower_bound$", i32 0)
  store i64 1, i64* %"var$5.dim_info$.lower_bound$[]", align 1
  %"var$5.dim_info$.extent$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", %"QNCA_a0$float*$rank1$.1"* %"var$5", i64 0, i32 6, i64 0, i32 0
  %"var$5.dim_info$.extent$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$5.dim_info$.extent$", i32 0)
  store i64 %slct4, i64* %"var$5.dim_info$.extent$[]", align 1
  %"var$5.addr_a0$" = getelementptr inbounds %"QNCA_a0$float*$rank1$.1", %"QNCA_a0$float*$rank1$.1"* %"var$5", i64 0, i32 0
  store float* %"test_01_$B", float** %"var$5.addr_a0$", align 8
  %"var$5.flags$10_fetch" = load i64, i64* %"var$5.flags$", align 8
  %or12 = or i64 %"var$5.flags$10_fetch", 1
  store i64 %or12, i64* %"var$5.flags$", align 8
  %"var$3.addr_a0$_fetch" = load float*, float** %"var$3.addr_a0$", align 8
  %"var$5.addr_a0$_fetch" = load float*, float** %"var$5.addr_a0$", align 8
  %"test_01_$SIZE_fetch58" = load i64, i64* %"test_01_$SIZE", align 1
  store i64 1, i64* %"test_01_$I", align 8
  %temp63 = alloca i64, align 8
  %temp64 = alloca i64, align 8
  store volatile i64 0, i64* %temp63, align 8
  %sub = add nsw i64 %"test_01_$SIZE_fetch58", -1
  store volatile i64 %sub, i64* %temp64, align 8
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.SIMD.182

DIR.OMP.SIMD.182:                                 ; preds = %DIR.OMP.SIMD.2
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV"(i64* %"test_01_$I", i64 1), "QUAL.OMP.NONTEMPORAL"(float* %"var$5.addr_a0$_fetch", float* %"var$3.addr_a0$_fetch"), "QUAL.OMP.NORMALIZED.IV"(i64* %temp63), "QUAL.OMP.NORMALIZED.UB"(i64* %temp64), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]
  br label %DIR.OMP.SIMD.283

DIR.OMP.SIMD.283:                                 ; preds = %DIR.OMP.SIMD.182
  %temp.load = load volatile i1, i1* %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.SIMD.4, label %DIR.OMP.SIMD.1

bb6:                                              ; preds = %DIR.OMP.SIMD.1
  store volatile i64 0, i64* %temp63, align 8
  %temp_fetch9 = load volatile i64, i64* %temp63, align 8
  %add = add nsw i64 %temp_fetch9, 1
  store i64 %add, i64* %"test_01_$I", align 1
  br label %bb10

bb10:                                             ; preds = %bb12, %bb6
  %temp_fetch15 = load volatile i64, i64* %temp63, align 8
  %add19 = add nsw i64 %temp_fetch15, 1
  store i64 %add19, i64* %"test_01_$I", align 1
  %"test_01_$SIZE_fetch21" = load i64, i64* %"test_01_$SIZE", align 1
  store i64 1, i64* %"test_01_$I", align 1
  %rel23 = icmp slt i64 %"test_01_$SIZE_fetch21", 1
  br i1 %rel23, label %bb12, label %bb11

bb11:                                             ; preds = %bb10, %bb11
  %"test_01_$I_fetch" = load i64, i64* %"test_01_$I", align 1
  %"var$5.addr_a0$_fetch[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %"var$5.addr_a0$_fetch", i64 %"test_01_$I_fetch")
  ; CHECK: load float, float* %"var$5.addr_a0$_fetch[]", align 1, !nontemporal
  %"var$5.addr_a0$_fetch[]_fetch" = load float, float* %"var$5.addr_a0$_fetch[]", align 1
  %mul26 = fmul fast float %"var$5.addr_a0$_fetch[]_fetch", %"var$5.addr_a0$_fetch[]_fetch"
  %add28 = fadd fast float %mul26, %"var$5.addr_a0$_fetch[]_fetch"
  %add29 = fadd fast float %add28, 1.000000e+00
  %"var$3.addr_a0$_fetch[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %"var$3.addr_a0$_fetch", i64 %"test_01_$I_fetch")
  ; CHECK: store float %add29, float* %"var$3.addr_a0$_fetch[]", align 1, !nontemporal
  store float %add29, float* %"var$3.addr_a0$_fetch[]", align 1
  %"test_01_$I_fetch33" = load i64, i64* %"test_01_$I", align 1
  %add35 = add nsw i64 %"test_01_$I_fetch33", 1
  store i64 %add35, i64* %"test_01_$I", align 1
  %rel41.not.not = icmp slt i64 %"test_01_$I_fetch33", %"test_01_$SIZE_fetch21"
  br i1 %rel41.not.not, label %bb11, label %bb12

bb12:                                             ; preds = %bb11, %bb10
  %temp_fetch45 = load volatile i64, i64* %temp63, align 8
  %add47 = add nsw i64 %temp_fetch45, 1
  store volatile i64 %add47, i64* %temp63, align 8
  %temp_fetch49 = load volatile i64, i64* %temp64, align 8
  %temp_fetch51 = load volatile i64, i64* %temp63, align 8
  %rel53.not = icmp sgt i64 %temp_fetch51, %temp_fetch49
  br i1 %rel53.not, label %DIR.OMP.END.SIMD.4, label %bb10

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.283
  store volatile i64 0, i64* %temp63, align 8
  %temp_fetch69 = load volatile i64, i64* %temp63, align 8
  %temp_fetch70 = load volatile i64, i64* %temp64, align 8
  %rel71 = icmp slt i64 %temp_fetch70, %temp_fetch69
  br i1 %rel71, label %DIR.OMP.END.SIMD.4, label %bb6

DIR.OMP.END.SIMD.4:                               ; preds = %bb12, %DIR.OMP.SIMD.1, %DIR.OMP.SIMD.283
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.4
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.484

DIR.OMP.END.SIMD.484:                             ; preds = %DIR.OMP.END.SIMD.3
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #2 = { nounwind }

!omp_offload.info = !{}
; end INTEL_CUSTOMIZATION

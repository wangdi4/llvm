; RUN: opt -opaque-pointers < %s -passes=dopevectorconstprop -dope-vector-global-const-prop=true -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s

; This test case checks that deallocating global dope vectors won't affect
; the constant propagation. Also, it checks that the optimization was applied
; since the global dope vectors aren't candidates for nested dope vectors,
; the pointer address is not a pointer to a structure. It was created from
; the following test case:

;   module assumedshape_perf_mod
;   contains
;     subroutine assumedshape_add(nx, ny, nz, array_source, array_target, array_add)
;       implicit none
;       integer, intent(in) :: nx, ny, nz
;       real, intent(inout) :: array_source(:,:,:)
;       real, intent(inout) :: array_target(nx,ny,nz)
;       real, intent(inout) :: array_add(:,:,:)
;
;       integer :: i, j, k
;
;       do k=1,nz
;         do j=1,ny
;           do i=1,nx
;             array_target(i,j,k) = array_source(i,j,k) + array_add(i,j,k)
;           end do
;         end do
;       end do
;     end subroutine assumedshape_add
;
;   end module assumedshape_perf_mod
;
;
;   program assumedshape_perf
;     use assumedshape_perf_mod
;     use iso_fortran_env, only : real64
;     implicit none
;
;     integer, parameter :: NX = 10, NY = 64, NZ = 128, nit = 100000
;     real, allocatable :: array_source(:,:,:), array_target(:,:,:), array_add(:,:,:)
;     integer :: i, j, k, it
;
;     allocate(array_source(NX,NY,NZ), array_target(NX,NY,NZ), array_add(NX,NY,NZ))
;
;     do it=1,nit
;       call assumedshape_add(NX, NY, NZ, array_source, array_target, array_add)
;     end do
;
;     deallocate(array_source, array_target)
;
;   end program assumedshape_perf

; ifx -g -O3 -xCORE-AVX512 -qopt-zmm-usage=high -fpp -traceback -reentrancy -flto -align array64byte -what -V assumedshape_perf.F90 -mllvm -print-before=dopevectorconstprop

; This is the same test case as global_dvcp20.ll, but it checks the IR.

; CHECK: %i87 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 2560, ptr elementtype(float) %i72, i64 %i86)
; CHECK: %i88 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 2560, ptr elementtype(float) %i79, i64 %i86)

; CHECK: %i92 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i87, i64 %i91)
; CHECK: %i93 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i88, i64 %i91)

; CHECK: store i64 10, ptr %i57
; CHECK: store i64 40, ptr %i119

; CHECK: store i64 64, ptr %i120
; CHECK: store i64 2560, ptr %i59

; CHECK: store i64 128, ptr %i60

; CHECK: store i64 10, ptr %i66
; CHECK: store i64 40, ptr %i127

; CHECK: store i64 64, ptr %i128
; CHECK: store i64 2560, ptr %i68

; CHECK: store i64 128, ptr %i69

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$float*$rank3$.2" = type { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }

@"assumedshape_perf_$ARRAY_ADD" = internal global %"QNCA_a0$float*$rank3$.2" { ptr null, i64 0, i64 0, i64 1610612864, i64 3, i64 0, [3 x { i64, i64, i64 }] zeroinitializer }
@"assumedshape_perf_$ARRAY_TARGET" = internal global %"QNCA_a0$float*$rank3$.2" { ptr null, i64 0, i64 0, i64 1610612864, i64 3, i64 0, [3 x { i64, i64, i64 }] zeroinitializer }
@"assumedshape_perf_$ARRAY_SOURCE" = internal global %"QNCA_a0$float*$rank3$.2" { ptr null, i64 0, i64 0, i64 1610612864, i64 3, i64 0, [3 x { i64, i64, i64 }] zeroinitializer }
@anon.502ad313df8268a2c46c880a3472eedc.0 = internal unnamed_addr constant i32 65536
@anon.502ad313df8268a2c46c880a3472eedc.1 = internal unnamed_addr constant i32 2

; Function Attrs: nounwind uwtable
define dso_local void @MAIN__() #0 {
bb:
  %i = alloca %"QNCA_a0$float*$rank3$.2", align 8
  %i1 = alloca %"QNCA_a0$float*$rank3$.2", align 8
  %i2 = tail call i32 @for_set_fpe_(ptr nonnull @anon.502ad313df8268a2c46c880a3472eedc.0) #3
  %i3 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.502ad313df8268a2c46c880a3472eedc.1) #3
  %i4 = load i64, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_SOURCE", i64 0, i32 3), align 8
  %i5 = and i64 %i4, 1030792151296
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_SOURCE", i64 0, i32 5), align 8
  store i64 4, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_SOURCE", i64 0, i32 1), align 8
  store i64 3, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_SOURCE", i64 0, i32 4), align 8
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_SOURCE", i64 0, i32 2), align 8
  %i6 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_SOURCE", i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %i6, align 8
  %i7 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_SOURCE", i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 10, ptr %i7, align 8
  %i8 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_SOURCE", i64 0, i32 6, i64 0, i32 2), i32 1)
  store i64 1, ptr %i8, align 8
  %i9 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_SOURCE", i64 0, i32 6, i64 0, i32 0), i32 1)
  store i64 64, ptr %i9, align 8
  %i10 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_SOURCE", i64 0, i32 6, i64 0, i32 2), i32 2)
  store i64 1, ptr %i10, align 8
  %i11 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_SOURCE", i64 0, i32 6, i64 0, i32 0), i32 2)
  store i64 128, ptr %i11, align 8
  %i12 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_SOURCE", i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 4, ptr %i12, align 8
  %i13 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_SOURCE", i64 0, i32 6, i64 0, i32 1), i32 1)
  store i64 40, ptr %i13, align 8
  %i14 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_SOURCE", i64 0, i32 6, i64 0, i32 1), i32 2)
  store i64 2560, ptr %i14, align 8
  %i15 = or i64 %i5, 1610612869
  store i64 %i15, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_SOURCE", i64 0, i32 3), align 8
  %i16 = lshr i64 %i5, 15
  %i17 = trunc i64 %i16 to i32
  %i18 = or i32 %i17, 393218
  %i19 = tail call i32 @for_alloc_allocatable_handle(i64 327680, ptr @"assumedshape_perf_$ARRAY_SOURCE", i32 %i18, ptr null) #3
  %i20 = load i64, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_TARGET", i64 0, i32 3), align 8
  %i21 = and i64 %i20, 1030792151296
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_TARGET", i64 0, i32 5), align 8
  store i64 4, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_TARGET", i64 0, i32 1), align 8
  store i64 3, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_TARGET", i64 0, i32 4), align 8
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_TARGET", i64 0, i32 2), align 8
  %i22 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_TARGET", i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %i22, align 8
  %i23 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_TARGET", i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 10, ptr %i23, align 8
  %i24 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_TARGET", i64 0, i32 6, i64 0, i32 2), i32 1)
  store i64 1, ptr %i24, align 8
  %i25 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_TARGET", i64 0, i32 6, i64 0, i32 0), i32 1)
  store i64 64, ptr %i25, align 8
  %i26 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_TARGET", i64 0, i32 6, i64 0, i32 2), i32 2)
  store i64 1, ptr %i26, align 8
  %i27 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_TARGET", i64 0, i32 6, i64 0, i32 0), i32 2)
  store i64 128, ptr %i27, align 8
  %i28 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_TARGET", i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 4, ptr %i28, align 8
  %i29 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_TARGET", i64 0, i32 6, i64 0, i32 1), i32 1)
  store i64 40, ptr %i29, align 8
  %i30 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_TARGET", i64 0, i32 6, i64 0, i32 1), i32 2)
  store i64 2560, ptr %i30, align 8
  %i31 = or i64 %i21, 1610612869
  store i64 %i31, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_TARGET", i64 0, i32 3), align 8
  %i32 = lshr i64 %i21, 15
  %i33 = trunc i64 %i32 to i32
  %i34 = or i32 %i33, 393218
  %i35 = tail call i32 @for_alloc_allocatable_handle(i64 327680, ptr @"assumedshape_perf_$ARRAY_TARGET", i32 %i34, ptr null) #3
  %i36 = load i64, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_ADD", i64 0, i32 3), align 8
  %i37 = and i64 %i36, 1030792151296
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_ADD", i64 0, i32 5), align 8
  store i64 4, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_ADD", i64 0, i32 1), align 8
  store i64 3, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_ADD", i64 0, i32 4), align 8
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_ADD", i64 0, i32 2), align 8
  %i38 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_ADD", i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %i38, align 8
  %i39 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_ADD", i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 10, ptr %i39, align 8
  %i40 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_ADD", i64 0, i32 6, i64 0, i32 2), i32 1)
  store i64 1, ptr %i40, align 8
  %i41 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_ADD", i64 0, i32 6, i64 0, i32 0), i32 1)
  store i64 64, ptr %i41, align 8
  %i42 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_ADD", i64 0, i32 6, i64 0, i32 2), i32 2)
  store i64 1, ptr %i42, align 8
  %i43 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_ADD", i64 0, i32 6, i64 0, i32 0), i32 2)
  store i64 128, ptr %i43, align 8
  %i44 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_ADD", i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 4, ptr %i44, align 8
  %i45 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_ADD", i64 0, i32 6, i64 0, i32 1), i32 1)
  store i64 40, ptr %i45, align 8
  %i46 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_ADD", i64 0, i32 6, i64 0, i32 1), i32 2)
  store i64 2560, ptr %i46, align 8
  %i47 = or i64 %i37, 1610612869
  store i64 %i47, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_ADD", i64 0, i32 3), align 8
  %i48 = lshr i64 %i37, 15
  %i49 = trunc i64 %i48 to i32
  %i50 = or i32 %i49, 393218
  %i51 = tail call i32 @for_alloc_allocatable_handle(i64 327680, ptr @"assumedshape_perf_$ARRAY_ADD", i32 %i50, ptr null) #3
  %i52 = getelementptr inbounds %"QNCA_a0$float*$rank3$.2", ptr %i, i64 0, i32 3
  %i53 = getelementptr inbounds %"QNCA_a0$float*$rank3$.2", ptr %i, i64 0, i32 4
  %i54 = getelementptr inbounds %"QNCA_a0$float*$rank3$.2", ptr %i, i64 0, i32 6, i64 0, i32 1
  %i55 = getelementptr inbounds %"QNCA_a0$float*$rank3$.2", ptr %i, i64 0, i32 6, i64 0, i32 2
  %i56 = getelementptr inbounds %"QNCA_a0$float*$rank3$.2", ptr %i, i64 0, i32 6, i64 0, i32 0
  %i57 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i56, i32 0)
  %i58 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i55, i32 1)
  %i59 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i54, i32 2)
  %i60 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i56, i32 2)
  %i61 = getelementptr inbounds %"QNCA_a0$float*$rank3$.2", ptr %i1, i64 0, i32 3
  %i62 = getelementptr inbounds %"QNCA_a0$float*$rank3$.2", ptr %i1, i64 0, i32 4
  %i63 = getelementptr inbounds %"QNCA_a0$float*$rank3$.2", ptr %i1, i64 0, i32 6, i64 0, i32 1
  %i64 = getelementptr inbounds %"QNCA_a0$float*$rank3$.2", ptr %i1, i64 0, i32 6, i64 0, i32 2
  %i65 = getelementptr inbounds %"QNCA_a0$float*$rank3$.2", ptr %i1, i64 0, i32 6, i64 0, i32 0
  %i66 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i65, i32 0)
  %i67 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i64, i32 1)
  %i68 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i63, i32 2)
  %i69 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i65, i32 2)
  br label %bb70

bb70:                                             ; preds = %bb111, %bb
  %i71 = phi i32 [ 1, %bb ], [ %i112, %bb111 ]
  %i72 = load ptr, ptr @"assumedshape_perf_$ARRAY_SOURCE", align 8
  %i73 = load i64, ptr %i7, align 8
  %i74 = load i64, ptr %i13, align 8
  %i75 = load i64, ptr %i9, align 8
  %i76 = load i64, ptr %i14, align 8
  %i77 = load i64, ptr %i11, align 8
  %i78 = load ptr, ptr @"assumedshape_perf_$ARRAY_TARGET", align 8
  %i79 = load ptr, ptr @"assumedshape_perf_$ARRAY_ADD", align 8
  %i80 = load i64, ptr %i39, align 8
  %i81 = load i64, ptr %i45, align 8
  %i82 = load i64, ptr %i41, align 8
  %i83 = load i64, ptr %i46, align 8
  %i84 = load i64, ptr %i43, align 8
  br label %bb85

bb85:                                             ; preds = %bb108, %bb70
  %i86 = phi i64 [ %i109, %bb108 ], [ 1, %bb70 ]
  %i87 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %i76, ptr elementtype(float) %i72, i64 %i86) #3
  %i88 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %i83, ptr elementtype(float) %i79, i64 %i86) #3
  %i89 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 2560, ptr nonnull elementtype(float) %i78, i64 %i86) #3
  br label %bb90

bb90:                                             ; preds = %bb105, %bb85
  %i91 = phi i64 [ %i106, %bb105 ], [ 1, %bb85 ]
  %i92 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i74, ptr elementtype(float) %i87, i64 %i91) #3
  %i93 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i81, ptr elementtype(float) %i88, i64 %i91) #3
  %i94 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr nonnull elementtype(float) %i89, i64 %i91) #3
  br label %bb95

bb95:                                             ; preds = %bb95, %bb90
  %i96 = phi i64 [ %i103, %bb95 ], [ 1, %bb90 ]
  %i97 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i92, i64 %i96) #3
  %i98 = load float, ptr %i97, align 4
  %i99 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i93, i64 %i96) #3
  %i100 = load float, ptr %i99, align 4
  %i101 = fadd reassoc ninf nsz arcp contract afn float %i98, %i100
  %i102 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %i94, i64 %i96) #3
  store float %i101, ptr %i102, align 4
  %i103 = add nuw nsw i64 %i96, 1
  %i104 = icmp eq i64 %i103, 11
  br i1 %i104, label %bb105, label %bb95

bb105:                                            ; preds = %bb95
  %i106 = add nuw nsw i64 %i91, 1
  %i107 = icmp eq i64 %i106, 65
  br i1 %i107, label %bb108, label %bb90

bb108:                                            ; preds = %bb105
  %i109 = add nuw nsw i64 %i86, 1
  %i110 = icmp eq i64 %i109, 129
  br i1 %i110, label %bb111, label %bb85

bb111:                                            ; preds = %bb108
  %i112 = add nuw nsw i32 %i71, 1
  %i113 = icmp eq i32 %i112, 100001
  br i1 %i113, label %bb114, label %bb70

bb114:                                            ; preds = %bb111
  %i115 = getelementptr inbounds %"QNCA_a0$float*$rank3$.2", ptr %i, i64 0, i32 1
  %i116 = getelementptr inbounds %"QNCA_a0$float*$rank3$.2", ptr %i, i64 0, i32 2
  %i117 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i54, i32 0)
  %i118 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i55, i32 0)
  %i119 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i54, i32 1)
  %i120 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i56, i32 1)
  %i121 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i55, i32 2)
  %i122 = getelementptr inbounds %"QNCA_a0$float*$rank3$.2", ptr %i, i64 0, i32 0
  %i123 = getelementptr inbounds %"QNCA_a0$float*$rank3$.2", ptr %i1, i64 0, i32 1
  %i124 = getelementptr inbounds %"QNCA_a0$float*$rank3$.2", ptr %i1, i64 0, i32 2
  %i125 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i63, i32 0)
  %i126 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i64, i32 0)
  %i127 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i63, i32 1)
  %i128 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i65, i32 1)
  %i129 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i64, i32 2)
  %i130 = getelementptr inbounds %"QNCA_a0$float*$rank3$.2", ptr %i1, i64 0, i32 0
  store i64 129, ptr %i52, align 8
  store i64 4, ptr %i115, align 8
  store i64 3, ptr %i53, align 8
  store i64 0, ptr %i116, align 8
  store i64 4, ptr %i117, align 8
  store i64 1, ptr %i118, align 8
  store i64 %i73, ptr %i57, align 8
  store i64 %i74, ptr %i119, align 8
  store i64 1, ptr %i58, align 8
  store i64 %i75, ptr %i120, align 8
  store i64 %i76, ptr %i59, align 8
  store i64 1, ptr %i121, align 8
  store i64 %i77, ptr %i60, align 8
  store ptr %i72, ptr %i122, align 8
  store i64 129, ptr %i61, align 8
  store i64 4, ptr %i123, align 8
  store i64 3, ptr %i62, align 8
  store i64 0, ptr %i124, align 8
  store i64 4, ptr %i125, align 8
  store i64 1, ptr %i126, align 8
  store i64 %i80, ptr %i66, align 8
  store i64 %i81, ptr %i127, align 8
  store i64 1, ptr %i67, align 8
  store i64 %i82, ptr %i128, align 8
  store i64 %i83, ptr %i68, align 8
  store i64 1, ptr %i129, align 8
  store i64 %i84, ptr %i69, align 8
  store ptr %i79, ptr %i130, align 8
  %i131 = load ptr, ptr @"assumedshape_perf_$ARRAY_SOURCE", align 8
  %i132 = load i64, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_SOURCE", i64 0, i32 3), align 8
  %i133 = trunc i64 %i132 to i32
  %i134 = shl i32 %i133, 1
  %i135 = and i32 %i134, 6
  %i136 = lshr i32 %i133, 3
  %i137 = and i32 %i136, 256
  %i138 = lshr i64 %i132, 15
  %i139 = trunc i64 %i138 to i32
  %i140 = and i32 %i139, 31457280
  %i141 = and i32 %i139, 33554432
  %i142 = or i32 %i137, %i135
  %i143 = or i32 %i142, %i140
  %i144 = or i32 %i143, %i141
  %i145 = or i32 %i144, 393216
  %i146 = load i64, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_SOURCE", i64 0, i32 5), align 8
  %i147 = inttoptr i64 %i146 to ptr
  %i148 = tail call i32 @for_dealloc_allocatable_handle(ptr %i131, i32 %i145, ptr %i147) #3
  %i149 = icmp eq i32 %i148, 0
  br i1 %i149, label %bb150, label %bb153

bb150:                                            ; preds = %bb114
  store ptr null, ptr @"assumedshape_perf_$ARRAY_SOURCE", align 8
  %i151 = load i64, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_SOURCE", i64 0, i32 3), align 8
  %i152 = and i64 %i151, -1030792153090
  store i64 %i152, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_SOURCE", i64 0, i32 3), align 8
  br label %bb153

bb153:                                            ; preds = %bb150, %bb114
  %i154 = load ptr, ptr @"assumedshape_perf_$ARRAY_TARGET", align 16
  %i155 = load i64, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_TARGET", i64 0, i32 3), align 8
  %i156 = trunc i64 %i155 to i32
  %i157 = shl i32 %i156, 1
  %i158 = and i32 %i157, 6
  %i159 = lshr i32 %i156, 3
  %i160 = and i32 %i159, 256
  %i161 = lshr i64 %i155, 15
  %i162 = trunc i64 %i161 to i32
  %i163 = and i32 %i162, 31457280
  %i164 = and i32 %i162, 33554432
  %i165 = or i32 %i160, %i158
  %i166 = or i32 %i165, %i163
  %i167 = or i32 %i166, %i164
  %i168 = or i32 %i167, 393216
  %i169 = load i64, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_TARGET", i64 0, i32 5), align 8
  %i170 = inttoptr i64 %i169 to ptr
  %i171 = tail call i32 @for_dealloc_allocatable_handle(ptr %i154, i32 %i168, ptr %i170) #3
  %i172 = icmp eq i32 %i171, 0
  br i1 %i172, label %bb173, label %bb176

bb173:                                            ; preds = %bb153
  store ptr null, ptr @"assumedshape_perf_$ARRAY_TARGET", align 16
  %i174 = load i64, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_TARGET", i64 0, i32 3), align 8
  %i175 = and i64 %i174, -1030792153090
  store i64 %i175, ptr getelementptr inbounds (%"QNCA_a0$float*$rank3$.2", ptr @"assumedshape_perf_$ARRAY_TARGET", i64 0, i32 3), align 8
  br label %bb176

bb176:                                            ; preds = %bb173, %bb153
  ret void
}

declare dso_local i32 @for_set_fpe_(ptr nocapture readonly) local_unnamed_addr

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_alloc_allocatable_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_dealloc_allocatable_handle(ptr nocapture readonly, i32, ptr) local_unnamed_addr #1

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #2

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="non-leaf" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "prefer-vector-width"="512" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nounwind readnone speculatable }
attributes #3 = { nounwind }

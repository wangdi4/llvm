; RUN: opt -opaque-pointers < %s -S -passes=dopevectorconstprop -debug-only=dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

; CHECK: define internal void @myoutput
; CHECK-NOT: tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40
; CHECK-NOT: tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4

; Check that dope vector constant propagation will happen for the loads
; indicated in the trace, but that it will not happen within @myoutput,
; because it is called with multiple arrays of different sizes.

; This test is similar to global_dvcp17.ll but checks the IRs in @myoutput.

; The IR below was generated from the following Fortran source code:
;
;        module physpropmod
;        type :: physprop_type
;        real, pointer :: sw_hygro_ext(:,:)
;        endtype physprop_type
;        type (physprop_type), pointer :: physprop(:)
;        integer, parameter :: physpropcount = 10
;        integer, parameter :: ndvsize = 10
;        end module
;
;        subroutine myinit(myinteger)
;        use physpropmod
;        integer, intent(in) :: myinteger
;        do i = 1, ndvsize
;          do j = 1, ndvsize
;            physprop(myinteger)%sw_hygro_ext(i,j) = i + j
;          end do
;        end do
;        print *, physprop(myinteger)%sw_hygro_ext
;        end subroutine
;
;        program main
;        use physpropmod
;        interface
;        subroutine myinit(myinteger)
;        integer, intent(in) :: myinteger
;        end subroutine
;        subroutine myoutput(myallarray)
;        real, intent(in) :: myallarray(:,:)
;        end subroutine
;        end interface
;        real, allocatable, target :: mylocalarray(:,:)
;        allocate(physprop(physpropcount))
;        allocate(mylocalarray(5,5))
;        mylocalarray = 5
;        do i = 1, phypropscount
;          allocate(physprop(i)%sw_hygro_ext(ndvsize,ndvsize))
;          call myinit(i)
;          call myoutput(physprop(i)%sw_hygro_ext)
;        end do
;        call myoutput(mylocalarray)
;        end program
;        subroutine myoutput(myallarray)
;        real, intent(in) :: myallarray(:,:)
;        integer lb1, lb2
;        lb1 = lbound(myallarray, 1)
;        lb1 = lbound(myallarray, 2)
;        print *, myallarray(lb1, lb2)
;        end subroutine

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%PHYSPROPMOD$.btPHYSPROP_TYPE*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"PHYSPROPMOD$.btPHYSPROP_TYPE" = type { %"QNCA_a0$float*$rank2$" }

@anon.2bc5e417881711fb3ab09da8e681987f.0 = internal unnamed_addr constant i32 2
@physpropmod_mp_physprop_ = internal global %"QNCA_a0$%PHYSPROPMOD$.btPHYSPROP_TYPE*$rank1$" { ptr null, i64 0, i64 0, i64 0, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@"main_$MYLOCALARRAY" = internal global %"QNCA_a0$float*$rank2$" { ptr null, i64 0, i64 0, i64 1073741952, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }

; Function Attrs: nofree noinline nounwind uwtable
define internal void @myinit_(ptr noalias nocapture readonly dereferenceable(4) %arg) #0 {
bb:
  %i = alloca [8 x i64], align 16
  %i1 = alloca [4 x i8], align 1
  %i2 = alloca { i64, ptr }, align 8
  %i3 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%PHYSPROPMOD$.btPHYSPROP_TYPE*$rank1$", ptr @physpropmod_mp_physprop_, i32 0, i32 6, i32 0, i32 1), i32 0)
  %i4 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%PHYSPROPMOD$.btPHYSPROP_TYPE*$rank1$", ptr @physpropmod_mp_physprop_, i32 0, i32 6, i32 0, i32 2), i32 0)
  %i5 = load i32, ptr %arg, align 1
  %i6 = sext i32 %i5 to i64
  br label %bb7

bb7:                                              ; preds = %bb36, %bb
  %i8 = phi i64 [ %i37, %bb36 ], [ 1, %bb ]
  br label %bb9

bb9:                                              ; preds = %bb9, %bb7
  %i10 = phi i64 [ %i34, %bb9 ], [ 1, %bb7 ]
  %i11 = add nuw nsw i64 %i10, %i8
  %i12 = trunc i64 %i11 to i32
  %i13 = sitofp i32 %i12 to float
  %i14 = load ptr, ptr @physpropmod_mp_physprop_, align 1
  %i15 = load i64, ptr %i3, align 1
  %i16 = load i64, ptr %i4, align 1
  %i17 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i16, i64 %i15, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i14, i64 %i6)
  %i18 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i17, i32 0, i32 0
  %i19 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i18, i32 0, i32 0
  %i20 = load ptr, ptr %i19, align 1
  %i21 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i18, i32 0, i32 6, i32 0
  %i22 = getelementptr inbounds { i64, i64, i64 }, ptr %i21, i32 0, i32 1
  %i23 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i22, i32 0)
  %i24 = load i64, ptr %i23, align 1
  %i25 = getelementptr inbounds { i64, i64, i64 }, ptr %i21, i32 0, i32 2
  %i26 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i25, i32 0)
  %i27 = load i64, ptr %i26, align 1
  %i28 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i22, i32 1)
  %i29 = load i64, ptr %i28, align 1
  %i30 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i25, i32 1)
  %i31 = load i64, ptr %i30, align 1
  %i32 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i31, i64 %i29, ptr elementtype(float) %i20, i64 %i10)
  %i33 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i27, i64 %i24, ptr elementtype(float) %i32, i64 %i8)
  store float %i13, ptr %i33, align 1
  %i34 = add nuw nsw i64 %i10, 1
  %i35 = icmp ne i64 %i34, 11
  br i1 %i35, label %bb9, label %bb36

bb36:                                             ; preds = %bb9
  %i37 = add nuw nsw i64 %i8, 1
  %i38 = icmp ne i64 %i37, 11
  br i1 %i38, label %bb7, label %bb39

bb39:                                             ; preds = %bb36
  %i40 = tail call ptr @llvm.stacksave()
  %i41 = load ptr, ptr @physpropmod_mp_physprop_, align 1
  %i42 = load i64, ptr %i3, align 1
  %i43 = load i64, ptr %i4, align 1
  %i44 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i43, i64 %i42, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i41, i64 %i6)
  %i45 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i44, i32 0, i32 0
  %i46 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i45, i32 0, i32 3
  %i47 = load i64, ptr %i46, align 1
  %i48 = and i64 %i47, 4
  %i49 = icmp ne i64 %i48, 0
  %i50 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i45, i32 0, i32 6, i32 0
  %i51 = getelementptr inbounds { i64, i64, i64 }, ptr %i50, i32 0, i32 1
  %i52 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i51, i32 0)
  %i53 = load i64, ptr %i52, align 1, !range !3
  %i54 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i45, i32 0, i32 1
  %i55 = load i64, ptr %i54, align 1
  %i56 = icmp eq i64 %i53, %i55
  %i57 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i51, i32 1)
  %i58 = load i64, ptr %i57, align 1, !range !3
  %i59 = getelementptr inbounds { i64, i64, i64 }, ptr %i50, i32 0, i32 0
  %i60 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i59, i32 0)
  %i61 = load i64, ptr %i60, align 1
  %i62 = mul nsw i64 %i61, %i53
  %i63 = icmp eq i64 %i58, %i62
  %i64 = and i1 %i56, %i63
  %i65 = or i1 %i49, %i64
  %i66 = xor i1 %i65, true
  %i67 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i45, i32 0, i32 0
  %i68 = load ptr, ptr %i67, align 1
  %i69 = getelementptr inbounds { i64, i64, i64 }, ptr %i50, i32 0, i32 2
  %i70 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i69, i32 0)
  %i71 = load i64, ptr %i70, align 1
  %i72 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i69, i32 1)
  %i73 = load i64, ptr %i72, align 1
  %i74 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i59, i32 1)
  %i75 = load i64, ptr %i74, align 1
  %i76 = mul nsw i64 %i61, 4
  br i1 %i66, label %bb77, label %bb109

bb77:                                             ; preds = %bb39
  %i78 = mul nsw i64 %i75, %i76
  %i79 = sdiv i64 %i78, 4
  %i80 = alloca float, i64 %i79, align 4
  %i81 = icmp sle i64 1, %i75
  br i1 %i81, label %bb82, label %bb107

bb82:                                             ; preds = %bb77
  %i83 = icmp sle i64 1, %i61
  %i84 = add i64 %i61, 1
  %i85 = add nuw i64 %i75, 1
  br label %bb100

bb86:                                             ; preds = %bb103, %bb86
  %i87 = phi i64 [ %i71, %bb103 ], [ %i92, %bb86 ]
  %i88 = phi i64 [ 1, %bb103 ], [ %i93, %bb86 ]
  %i89 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i71, i64 %i53, ptr elementtype(float) %i104, i64 %i87)
  %i90 = load float, ptr %i89, align 1
  %i91 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %i105, i64 %i88)
  store float %i90, ptr %i91, align 1
  %i92 = add nsw i64 %i87, 1
  %i93 = add nuw nsw i64 %i88, 1
  %i94 = icmp ne i64 %i93, %i84
  br i1 %i94, label %bb86, label %bb95

bb95:                                             ; preds = %bb86
  br label %bb96

bb96:                                             ; preds = %bb100, %bb95
  %i97 = add nsw i64 %i101, 1
  %i98 = add nuw nsw i64 %i102, 1
  %i99 = icmp ne i64 %i98, %i85
  br i1 %i99, label %bb100, label %bb106

bb100:                                            ; preds = %bb96, %bb82
  %i101 = phi i64 [ %i73, %bb82 ], [ %i97, %bb96 ]
  %i102 = phi i64 [ 1, %bb82 ], [ %i98, %bb96 ]
  br i1 %i83, label %bb103, label %bb96

bb103:                                            ; preds = %bb100
  %i104 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i73, i64 %i58, ptr elementtype(float) %i68, i64 %i101)
  %i105 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i76, ptr nonnull elementtype(float) %i80, i64 %i102)
  br label %bb86

bb106:                                            ; preds = %bb96
  br label %bb107

bb107:                                            ; preds = %bb106, %bb77
  %i108 = bitcast ptr %i80 to ptr
  br label %bb112

bb109:                                            ; preds = %bb39
  %i110 = bitcast ptr %i68 to ptr
  %i111 = mul nsw i64 %i75, %i76
  br label %bb112

bb112:                                            ; preds = %bb109, %bb107
  %i113 = phi i64 [ %i111, %bb109 ], [ %i78, %bb107 ]
  %i114 = phi ptr [ %i108, %bb107 ], [ %i110, %bb109 ]
  %i115 = getelementptr inbounds [4 x i8], ptr %i1, i32 0, i32 0
  store i8 26, ptr %i115, align 1
  %i116 = getelementptr inbounds [4 x i8], ptr %i1, i32 0, i32 1
  store i8 5, ptr %i116, align 1
  %i117 = getelementptr inbounds [4 x i8], ptr %i1, i32 0, i32 2
  store i8 1, ptr %i117, align 1
  %i118 = getelementptr inbounds [4 x i8], ptr %i1, i32 0, i32 3
  store i8 0, ptr %i118, align 1
  %i119 = getelementptr inbounds { i64, ptr }, ptr %i2, i32 0, i32 0
  store i64 %i113, ptr %i119, align 1
  %i120 = getelementptr inbounds { i64, ptr }, ptr %i2, i32 0, i32 1
  store ptr %i114, ptr %i120, align 1
  %i121 = bitcast ptr %i to ptr
  %i122 = bitcast ptr %i1 to ptr
  %i123 = bitcast ptr %i2 to ptr
  %i124 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i121, i32 -1, i64 1239157112576, ptr nonnull %i122, ptr nonnull %i123)
  call void @llvm.stackrestore(ptr %i40)
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare ptr @llvm.stacksave() #1

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) #2

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore(ptr) #1

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #3 {
bb:
  %i = alloca i32, align 8
  %i1 = tail call i32 @for_set_reentrancy(ptr @anon.2bc5e417881711fb3ab09da8e681987f.0)
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%PHYSPROPMOD$.btPHYSPROP_TYPE*$rank1$", ptr @physpropmod_mp_physprop_, i32 0, i32 5), align 1
  store i64 96, ptr getelementptr inbounds (%"QNCA_a0$%PHYSPROPMOD$.btPHYSPROP_TYPE*$rank1$", ptr @physpropmod_mp_physprop_, i32 0, i32 1), align 1
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$%PHYSPROPMOD$.btPHYSPROP_TYPE*$rank1$", ptr @physpropmod_mp_physprop_, i32 0, i32 4), align 1
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%PHYSPROPMOD$.btPHYSPROP_TYPE*$rank1$", ptr @physpropmod_mp_physprop_, i32 0, i32 2), align 1
  %i2 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%PHYSPROPMOD$.btPHYSPROP_TYPE*$rank1$", ptr @physpropmod_mp_physprop_, i32 0, i32 6, i32 0, i32 2), i32 0)
  store i64 1, ptr %i2, align 1
  %i3 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%PHYSPROPMOD$.btPHYSPROP_TYPE*$rank1$", ptr @physpropmod_mp_physprop_, i32 0, i32 6, i32 0, i32 0), i32 0)
  store i64 10, ptr %i3, align 1
  %i4 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%PHYSPROPMOD$.btPHYSPROP_TYPE*$rank1$", ptr @physpropmod_mp_physprop_, i32 0, i32 6, i32 0, i32 1), i32 0)
  store i64 96, ptr %i4, align 1
  store i64 1073741829, ptr getelementptr inbounds (%"QNCA_a0$%PHYSPROPMOD$.btPHYSPROP_TYPE*$rank1$", ptr @physpropmod_mp_physprop_, i32 0, i32 3), align 1
  %i5 = tail call i32 @for_allocate_handle(i64 960, ptr @physpropmod_mp_physprop_, i32 262144, ptr null)
  %i6 = load i64, ptr getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @"main_$MYLOCALARRAY", i32 0, i32 3), align 1
  %i7 = and i64 %i6, 256
  %i8 = lshr i64 %i7, 8
  %i9 = shl nuw nsw i64 %i8, 8
  %i10 = add nuw nsw i64 %i9, 133
  %i11 = and i64 %i6, 1030792151040
  %i12 = lshr i64 %i11, 36
  %i13 = and i64 %i10, -1030792151041
  %i14 = shl nuw nsw i64 %i12, 36
  %i15 = add nuw nsw i64 %i13, %i14
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @"main_$MYLOCALARRAY", i32 0, i32 5), align 1
  store i64 4, ptr getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @"main_$MYLOCALARRAY", i32 0, i32 1), align 1
  store i64 2, ptr getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @"main_$MYLOCALARRAY", i32 0, i32 4), align 1
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @"main_$MYLOCALARRAY", i32 0, i32 2), align 1
  %i16 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @"main_$MYLOCALARRAY", i32 0, i32 6, i32 0, i32 2), i32 0)
  store i64 1, ptr %i16, align 1
  %i17 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @"main_$MYLOCALARRAY", i32 0, i32 6, i32 0, i32 0), i32 0)
  store i64 5, ptr %i17, align 1
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @"main_$MYLOCALARRAY", i32 0, i32 6, i32 0, i32 2), i32 1)
  store i64 1, ptr %i18, align 1
  %i19 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @"main_$MYLOCALARRAY", i32 0, i32 6, i32 0, i32 0), i32 1)
  store i64 5, ptr %i19, align 1
  %i20 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @"main_$MYLOCALARRAY", i32 0, i32 6, i32 0, i32 1), i32 0)
  store i64 4, ptr %i20, align 1
  %i21 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @"main_$MYLOCALARRAY", i32 0, i32 6, i32 0, i32 1), i32 1)
  store i64 20, ptr %i21, align 1
  %i22 = and i64 %i15, -68451041281
  %i23 = or i64 %i22, 1073741824
  store i64 %i23, ptr getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @"main_$MYLOCALARRAY", i32 0, i32 3), align 1
  %i24 = and i64 %i15, 1
  %i25 = shl nuw nsw i64 %i24, 1
  %i26 = trunc i64 %i25 to i32
  %i27 = and i32 %i26, -2097169
  %i28 = and i64 %i15, 1030792151040
  %i29 = lshr i64 %i28, 36
  %i30 = and i32 %i27, -31457281
  %i31 = shl nuw nsw i64 %i29, 21
  %i32 = trunc i64 %i31 to i32
  %i33 = or i32 %i30, %i32
  %i34 = and i64 %i15, 1099511627776
  %i35 = lshr i64 %i34, 40
  %i36 = and i32 %i33, -33554433
  %i37 = shl nuw nsw i64 %i35, 25
  %i38 = trunc i64 %i37 to i32
  %i39 = or i32 %i36, %i38
  %i40 = and i32 %i39, -2031617
  %i41 = or i32 %i40, 262144
  %i42 = tail call i32 @for_alloc_allocatable_handle(i64 100, ptr @"main_$MYLOCALARRAY", i32 %i41, ptr null)
  %i43 = load i64, ptr %i16, align 1
  %i44 = load i64, ptr %i18, align 1
  %i45 = load i64, ptr %i21, align 1, !range !3
  %i46 = load ptr, ptr @"main_$MYLOCALARRAY", align 1
  %i47 = load i64, ptr %i17, align 1
  %i48 = load i64, ptr %i19, align 1
  %i49 = icmp sle i64 1, %i48
  br i1 %i49, label %bb50, label %bb72

bb50:                                             ; preds = %bb
  %i51 = icmp sle i64 1, %i47
  %i52 = add i64 %i47, 1
  %i53 = add nuw i64 %i48, 1
  br label %bb66

bb54:                                             ; preds = %bb69, %bb54
  %i55 = phi i64 [ 1, %bb69 ], [ %i59, %bb54 ]
  %i56 = phi i64 [ %i43, %bb69 ], [ %i58, %bb54 ]
  %i57 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i43, i64 4, ptr elementtype(float) %i70, i64 %i56)
  store float 5.000000e+00, ptr %i57, align 1
  %i58 = add nsw i64 %i56, 1
  %i59 = add nuw nsw i64 %i55, 1
  %i60 = icmp ne i64 %i59, %i52
  br i1 %i60, label %bb54, label %bb61

bb61:                                             ; preds = %bb54
  br label %bb62

bb62:                                             ; preds = %bb66, %bb61
  %i63 = add nsw i64 %i67, 1
  %i64 = add nuw nsw i64 %i68, 1
  %i65 = icmp ne i64 %i64, %i53
  br i1 %i65, label %bb66, label %bb71

bb66:                                             ; preds = %bb62, %bb50
  %i67 = phi i64 [ %i44, %bb50 ], [ %i63, %bb62 ]
  %i68 = phi i64 [ 1, %bb50 ], [ %i64, %bb62 ]
  br i1 %i51, label %bb69, label %bb62

bb69:                                             ; preds = %bb66
  %i70 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i44, i64 %i45, ptr elementtype(float) %i46, i64 %i67)
  br label %bb54

bb71:                                             ; preds = %bb62
  br label %bb72

bb72:                                             ; preds = %bb71, %bb
  store i32 1, ptr %i, align 1
  br i1 false, label %bb73, label %bb74

bb73:                                             ; preds = %bb72
  br label %bb116

bb74:                                             ; preds = %bb72
  br label %bb75

bb75:                                             ; preds = %bb75, %bb74
  %i76 = phi i32 [ %i114, %bb75 ], [ 1, %bb74 ]
  %i77 = load ptr, ptr @physpropmod_mp_physprop_, align 1
  %i78 = load i64, ptr %i4, align 1
  %i79 = load i64, ptr %i2, align 1
  %i80 = sext i32 %i76 to i64
  %i81 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i79, i64 %i78, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i77, i64 %i80)
  %i82 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i81, i32 0, i32 0
  %i83 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i82, i32 0, i32 3
  store i64 5, ptr %i83, align 1
  %i84 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i82, i32 0, i32 5
  store i64 0, ptr %i84, align 1
  %i85 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i82, i32 0, i32 1
  store i64 4, ptr %i85, align 1
  %i86 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i82, i32 0, i32 4
  store i64 2, ptr %i86, align 1
  %i87 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i82, i32 0, i32 2
  store i64 0, ptr %i87, align 1
  %i88 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i82, i32 0, i32 6, i32 0
  %i89 = getelementptr inbounds { i64, i64, i64 }, ptr %i88, i32 0, i32 2
  %i90 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i89, i32 0)
  store i64 1, ptr %i90, align 1
  %i91 = getelementptr inbounds { i64, i64, i64 }, ptr %i88, i32 0, i32 0
  %i92 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i91, i32 0)
  store i64 10, ptr %i92, align 1
  %i93 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i89, i32 1)
  store i64 1, ptr %i93, align 1
  %i94 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i91, i32 1)
  store i64 10, ptr %i94, align 1
  %i95 = getelementptr inbounds { i64, i64, i64 }, ptr %i88, i32 0, i32 1
  %i96 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i95, i32 0)
  store i64 4, ptr %i96, align 1
  %i97 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i95, i32 1)
  store i64 40, ptr %i97, align 1
  %i98 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i82, i32 0, i32 0
  %i99 = load i64, ptr %i83, align 1
  %i100 = and i64 %i99, -68451041281
  %i101 = or i64 %i100, 1073741824
  store i64 %i101, ptr %i83, align 1
  %i102 = load i64, ptr %i84, align 1
  %i103 = inttoptr i64 %i102 to ptr
  %i104 = bitcast ptr %i98 to ptr
  %i105 = call i32 @for_allocate_handle(i64 400, ptr %i104, i32 262144, ptr %i103)
  call void @myinit_(ptr nonnull %i)
  %i106 = load ptr, ptr @physpropmod_mp_physprop_, align 1
  %i107 = load i64, ptr %i4, align 1
  %i108 = load i64, ptr %i2, align 1
  %i109 = load i32, ptr %i, align 1
  %i110 = sext i32 %i109 to i64
  %i111 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i108, i64 %i107, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i106, i64 %i110)
  %i112 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i111, i32 0, i32 0
  call void @myoutput_.1(ptr %i112)
  %i113 = load i32, ptr %i, align 1
  %i114 = add nsw i32 %i113, 1
  store i32 %i114, ptr %i, align 1
  br i1 true, label %bb75, label %bb115

bb115:                                            ; preds = %bb75
  br label %bb116

bb116:                                            ; preds = %bb115, %bb73
  call void @myoutput_.1(ptr @"main_$MYLOCALARRAY")
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) #2

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, ptr nocapture, i32, ptr) #2

; Function Attrs: nofree
declare dso_local i32 @for_alloc_allocatable_handle(i64, ptr nocapture, i32, ptr) #2

; Function Attrs: nofree noinline nounwind uwtable
define internal void @myoutput_.1(ptr noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %arg) #0 {
bb:
  %i = alloca [8 x i64], align 16
  %i1 = alloca [4 x i8], align 1
  %i2 = alloca { float }, align 8
  %i3 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %arg, i32 0, i32 6, i32 0
  br label %bb4

bb4:                                              ; preds = %bb
  br label %bb6

bb5:                                              ; No predecessors!
  br label %bb6

bb6:                                              ; preds = %bb5, %bb4
  br label %bb7

bb7:                                              ; preds = %bb6
  br label %bb9

bb8:                                              ; No predecessors!
  br label %bb9

bb9:                                              ; preds = %bb8, %bb7
  %i10 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %arg, i32 0, i32 0
  %i11 = load ptr, ptr %i10, align 1
  %i12 = getelementptr inbounds { i64, i64, i64 }, ptr %i3, i32 0, i32 1
  %i13 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i12, i32 0)
  %i14 = load i64, ptr %i13, align 1
  %i15 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i12, i32 1)
  %i16 = load i64, ptr %i15, align 1
  %i17 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i16, ptr elementtype(float) %i11, i64 0)
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i14, ptr nonnull elementtype(float) %i17, i64 1)
  %i19 = load float, ptr %i18, align 1
  %i20 = getelementptr inbounds [4 x i8], ptr %i1, i32 0, i32 0
  store i8 26, ptr %i20, align 1
  %i21 = getelementptr inbounds [4 x i8], ptr %i1, i32 0, i32 1
  store i8 1, ptr %i21, align 1
  %i22 = getelementptr inbounds [4 x i8], ptr %i1, i32 0, i32 2
  store i8 1, ptr %i22, align 1
  %i23 = getelementptr inbounds [4 x i8], ptr %i1, i32 0, i32 3
  store i8 0, ptr %i23, align 1
  %i24 = getelementptr inbounds { float }, ptr %i2, i32 0, i32 0
  store float %i19, ptr %i24, align 1
  %i25 = bitcast ptr %i to ptr
  %i26 = bitcast ptr %i1 to ptr
  %i27 = bitcast ptr %i2 to ptr
  %i28 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i25, i32 -1, i64 1239157112576, ptr nonnull %i26, ptr nonnull %i27)
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #4

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #4

attributes #0 = { nofree noinline nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nocallback nofree nosync nounwind willreturn }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #4 = { nounwind readnone speculatable }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
!3 = !{i64 1, i64 -9223372036854775808}

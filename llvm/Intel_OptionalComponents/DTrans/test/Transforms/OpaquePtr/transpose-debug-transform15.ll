; REQUIRES: asserts
; RUN: opt -opaque-pointers < %s -disable-output -passes=dtrans-transpose -debug-only=dtrans-transpose-transform 2>&1 | FileCheck %s

; Check that physpropmod_mp_physprop_ field 0 is transposed even though it is assigned through
; a pointer assignment, propagated up the call chain to an AllocaInst and then
; propagated down the call chain.

; CHECK-LABEL: Transform candidate: physpropmod_mp_physprop_[0]
; CHECK: Before: get_hygro_rad_props_:  %[[N0:[A-Za-z0-9]+]]  = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %[[X0:[A-Za-z0-9]+]], ptr elementtype(float) %[[I0:[A-Za-z0-9]+]], i64 %[[J0:[A-Za-z0-9]+]])
; CHECK: After : get_hygro_rad_props_:  %[[N0]] = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %[[I0]], i64 %[[J0]])
; CHECK: Before: get_hygro_rad_props_:  %[[N1:[A-Za-z0-9]+]] = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %[[X1:[A-Za-z0-9]+]], ptr elementtype(float) %[[I1:[A-Za-z0-9]+]], i64 %[[J1:[A-Za-z0-9]+]])
; CHECK: After : get_hygro_rad_props_:  %[[N1]] = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 76, ptr elementtype(float) %[[I1]], i64 %[[J1]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"PHYSPROPMOD$.btPHYSPROP_TYPE" = type { %"QNCA_a0$float*$rank2$" }
%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@physpropmod_mp_physprop_ = internal global %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" { ptr null, i64 0, i64 0, i64 0, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@"main_$TAU" = internal global [19 x [26 x [4 x float]]] zeroinitializer, align 16
@"main_$KRH" = internal global [26 x [4 x i32]] zeroinitializer, align 16
@anon.5b3c5dff38f14be85a38ea3af0f7d558.0 = internal unnamed_addr constant i32 2
@anon.5b3c5dff38f14be85a38ea3af0f7d558.1 = internal unnamed_addr constant i32 10

; Function Attrs: nofree noinline nounwind uwtable
define internal void @physprop_init_(ptr noalias nocapture readonly dereferenceable(4) %arg) #0 {
bb:
  %i = alloca i64, align 8
  store i64 5, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 5), align 8
  store i64 96, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 1), align 8
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 4), align 8
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 2), align 8
  %i1 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %i1, align 1
  %i2 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 10, ptr %i2, align 1
  %i3 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 96, ptr %i3, align 1
  %i4 = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %i, i32 2, i64 10, i64 96) #9
  %i5 = load i64, ptr %i, align 8
  %i6 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  %i7 = and i64 %i6, -68451041281
  %i8 = or i64 %i7, 1073741824
  store i64 %i8, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  %i9 = shl i32 %i4, 4
  %i10 = and i32 %i9, 16
  %i11 = or i32 %i10, 262144
  %i12 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 5), align 8
  %i13 = inttoptr i64 %i12 to ptr
  %i14 = tail call i32 @for_allocate_handle(i64 %i5, ptr @physpropmod_mp_physprop_, i32 %i11, ptr %i13) #9
  br label %bb15

bb15:                                             ; preds = %bb15, %bb
  %i16 = phi float [ %i27, %bb15 ], [ 1.000000e+00, %bb ]
  %i17 = phi i64 [ %i28, %bb15 ], [ 10, %bb ]
  %i18 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i19 = load i64, ptr %i3, align 1
  %i20 = load i64, ptr %i1, align 1
  %i21 = fptosi float %i16 to i32
  %i22 = sext i32 %i21 to i64
  %i23 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i20, i64 %i19, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i18, i64 %i22)
  %i24 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i23, i64 0, i32 0
  %i25 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i24, i64 0, i32 0
  store ptr null, ptr %i25, align 1
  %i26 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i24, i64 0, i32 3
  store i64 0, ptr %i26, align 1
  %i27 = fadd reassoc ninf nsz arcp contract afn float %i16, 1.000000e+00
  %i28 = add nsw i64 %i17, -1
  %i29 = icmp sgt i64 %i17, 1
  br i1 %i29, label %bb15, label %bb30

bb30:                                             ; preds = %bb30, %bb15
  %i31 = phi float [ %i62, %bb30 ], [ 1.000000e+00, %bb15 ]
  %i32 = phi i64 [ %i63, %bb30 ], [ 10, %bb15 ]
  %i33 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i34 = load i64, ptr %i3, align 1
  %i35 = load i64, ptr %i1, align 1
  %i36 = fptosi float %i31 to i32
  %i37 = sext i32 %i36 to i64
  %i38 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i35, i64 %i34, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i33, i64 %i37)
  %i39 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i38, i64 0, i32 0
  %i40 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i39, i64 0, i32 3
  store i64 5, ptr %i40, align 1
  %i41 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i39, i64 0, i32 5
  store i64 0, ptr %i41, align 1
  %i42 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i39, i64 0, i32 1
  store i64 4, ptr %i42, align 1
  %i43 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i39, i64 0, i32 4
  store i64 2, ptr %i43, align 1
  %i44 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i39, i64 0, i32 2
  store i64 0, ptr %i44, align 1
  %i45 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i39, i64 0, i32 6, i64 0
  %i46 = getelementptr inbounds { i64, i64, i64 }, ptr %i45, i64 0, i32 2
  %i47 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i46, i32 0)
  store i64 1, ptr %i47, align 1
  %i48 = getelementptr inbounds { i64, i64, i64 }, ptr %i45, i64 0, i32 0
  %i49 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i48, i32 0)
  store i64 1000, ptr %i49, align 1
  %i50 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i46, i32 1)
  store i64 1, ptr %i50, align 1
  %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i48, i32 1)
  store i64 19, ptr %i51, align 1
  %i52 = getelementptr inbounds { i64, i64, i64 }, ptr %i45, i64 0, i32 1
  %i53 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i52, i32 0)
  store i64 4, ptr %i53, align 1
  %i54 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i52, i32 1)
  store i64 4000, ptr %i54, align 1
  %i55 = load i64, ptr %i40, align 1
  %i56 = and i64 %i55, -68451041281
  %i57 = or i64 %i56, 1073741824
  store i64 %i57, ptr %i40, align 1
  %i58 = load i64, ptr %i41, align 1
  %i59 = inttoptr i64 %i58 to ptr
  %i60 = bitcast ptr %i38 to ptr
  %i61 = tail call i32 @for_allocate_handle(i64 76000, ptr %i60, i32 262144, ptr %i59) #9
  %i62 = fadd reassoc ninf nsz arcp contract afn float %i31, 1.000000e+00
  %i63 = add nsw i64 %i32, -1
  %i64 = icmp sgt i64 %i32, 1
  br i1 %i64, label %bb30, label %bb65

bb65:                                             ; preds = %bb30
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_check_mult_overflow64(ptr nocapture, i32, ...) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: mustprogress nofree noinline nosync nounwind willreturn uwtable
define internal void @physprop_get_(ptr noalias nocapture readonly dereferenceable(4) %arg, ptr noalias nocapture dereferenceable(96) %arg1) #2 {
bb:
  %i = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i2 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  %i3 = load i64, ptr %i2, align 1
  %i4 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %i5 = load i64, ptr %i4, align 1
  %i6 = load i32, ptr %arg, align 1
  %i7 = sext i32 %i6 to i64
  %i8 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i5, i64 %i3, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i, i64 %i7)
  %i9 = bitcast ptr %i8 to ptr
  %i10 = load %"QNCA_a0$float*$rank2$", ptr %i9, align 1
  store %"QNCA_a0$float*$rank2$" %i10, ptr %arg1, align 1
  ret void
}

; Function Attrs: nofree noinline nosync nounwind uwtable
define internal void @get_hygro_rad_props_(ptr noalias nocapture readonly dereferenceable(4) %arg, ptr noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %arg1, ptr noalias nocapture readnone dereferenceable(4) %arg2) #3 {
bb:
  %i = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %arg1, i64 0, i32 0
  %i3 = load ptr, ptr %i, align 1
  %i4 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %arg1, i64 0, i32 6, i64 0
  %i5 = getelementptr inbounds { i64, i64, i64 }, ptr %i4, i64 0, i32 1
  %i6 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i5, i32 0)
  %i7 = load i64, ptr %i6, align 1
  %i8 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i5, i32 1)
  %i9 = load i64, ptr %i8, align 1
  br label %bb10

bb10:                                             ; preds = %bb33, %bb
  %i11 = phi i64 [ %i34, %bb33 ], [ 1, %bb ]
  %i12 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i9, ptr elementtype(float) %i3, i64 %i11)
  %i13 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 416, ptr nonnull elementtype(float) @"main_$TAU", i64 %i11)
  br label %bb14

bb14:                                             ; preds = %bb30, %bb10
  %i15 = phi i64 [ %i31, %bb30 ], [ 1, %bb10 ]
  br label %bb16

bb16:                                             ; preds = %bb16, %bb14
  %i17 = phi i64 [ %i28, %bb16 ], [ 1, %bb14 ]
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 16, ptr nonnull elementtype(i32) @"main_$KRH", i64 %i17)
  %i19 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %i18, i64 %i15)
  %i20 = load i32, ptr %i19, align 1
  %i21 = add nsw i32 %i20, 1
  %i22 = sext i32 %i21 to i64
  %i23 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i7, ptr elementtype(float) %i12, i64 %i22)
  %i24 = load float, ptr %i23, align 1
  %i25 = fadd reassoc ninf nsz arcp contract afn float %i24, %i24
  %i26 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 16, ptr nonnull elementtype(float) %i13, i64 %i17)
  %i27 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %i26, i64 %i15)
  store float %i25, ptr %i27, align 1
  %i28 = add nuw nsw i64 %i17, 1
  %i29 = icmp eq i64 %i28, 27
  br i1 %i29, label %bb30, label %bb16

bb30:                                             ; preds = %bb16
  %i31 = add nuw nsw i64 %i15, 1
  %i32 = icmp eq i64 %i31, 5
  br i1 %i32, label %bb33, label %bb14

bb33:                                             ; preds = %bb30
  %i34 = add nuw nsw i64 %i11, 1
  %i35 = icmp eq i64 %i34, 20
  br i1 %i35, label %bb36, label %bb10

bb36:                                             ; preds = %bb33
  ret void
}

; Function Attrs: nofree noinline nosync nounwind writeonly uwtable
define internal void @init_krh_(ptr noalias nocapture readnone dereferenceable(4) %arg) #4 {
bb:
  br label %bb1

bb1:                                              ; preds = %bb10, %bb
  %i = phi i64 [ %i11, %bb10 ], [ 1, %bb ]
  br label %bb2

bb2:                                              ; preds = %bb2, %bb1
  %i3 = phi i64 [ %i8, %bb2 ], [ 1, %bb1 ]
  %i4 = add nuw nsw i64 %i3, %i
  %i5 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 16, ptr nonnull elementtype(i32) @"main_$KRH", i64 %i3)
  %i6 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %i5, i64 %i)
  %i7 = trunc i64 %i4 to i32
  store i32 %i7, ptr %i6, align 1
  %i8 = add nuw nsw i64 %i3, 1
  %i9 = icmp eq i64 %i8, 27
  br i1 %i9, label %bb10, label %bb2

bb10:                                             ; preds = %bb2
  %i11 = add nuw nsw i64 %i, 1
  %i12 = icmp eq i64 %i11, 5
  br i1 %i12, label %bb13, label %bb1

bb13:                                             ; preds = %bb10
  ret void
}

; Function Attrs: nofree noinline noreturn nosync nounwind uwtable
define internal void @bigloop_(ptr noalias nocapture readnone dereferenceable(4) %arg, ptr noalias nocapture readonly dereferenceable(4) %arg1) #5 {
bb:
  %i = alloca i32, align 8
  %i2 = alloca %"QNCA_a0$float*$rank2$", align 8
  %i3 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i2, i64 0, i32 0
  store ptr null, ptr %i3, align 8
  %i4 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i2, i64 0, i32 3
  store i64 0, ptr %i4, align 8
  store i32 1, ptr %i, align 8
  br label %bb5

bb5:                                              ; preds = %bb5, %bb
  call void @physprop_get_(ptr nonnull %i, ptr nonnull %i2)
  call void @get_hygro_rad_props_(ptr nonnull @"main_$KRH", ptr nonnull %i2, ptr nonnull @"main_$TAU")
  %i6 = load i32, ptr %i, align 8
  %i7 = add nsw i32 %i6, 1
  store i32 %i7, ptr %i, align 8
  br label %bb5
}

; Function Attrs: nofree noreturn nounwind uwtable
define dso_local void @MAIN__() #6 {
bb:
  %i = tail call i32 @for_set_reentrancy(ptr nonnull @anon.5b3c5dff38f14be85a38ea3af0f7d558.0) #9
  tail call void @init_krh_(ptr @"main_$KRH")
  tail call void @physprop_init_(ptr nonnull @anon.5b3c5dff38f14be85a38ea3af0f7d558.1)
  tail call void @bigloop_(ptr @"main_$TAU", ptr @"main_$KRH")
  unreachable
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: nocallback nofree nosync nounwind readnone willreturn
declare i64 @llvm.ssa.copy.i64(i64 returned) #7

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #8

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #8

attributes #0 = { nofree noinline nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { mustprogress nofree noinline nosync nounwind willreturn uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #3 = { nofree noinline nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #4 = { nofree noinline nosync nounwind writeonly uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #5 = { nofree noinline noreturn nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #6 = { nofree noreturn nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #7 = { nocallback nofree nosync nounwind readnone willreturn }
attributes #8 = { nounwind readnone speculatable }
attributes #9 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}

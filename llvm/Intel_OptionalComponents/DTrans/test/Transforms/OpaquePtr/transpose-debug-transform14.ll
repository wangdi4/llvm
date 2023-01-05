; REQUIRES: asserts
; RUN: opt -opaque-pointers < %s -dva-check-dtrans-outofboundsok -dtrans-outofboundsok=false -disable-output -passes=dtrans-transpose -debug-only=dtrans-transpose-transform 2>&1 | FileCheck %s

; Check that neither field 0 nor field 1 of physpropmod_mp_physprop_ is
; transposed, because the accesses are in the natural order.

; CHECK-LABEL: Transform candidate: physpropmod_mp_physprop_[0]
; CHECK-NOT: Before
; CHECK-NOT: After

; CHECK-LABEL: Transform candidate: physpropmod_mp_physprop_[1]
; CHECK-NOT: Before
; CHECK-NOT: After

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"PHYSPROPMOD$.btPHYSPROP_TYPE" = type { %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$", ptr }
%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@physpropmod_mp_physprop_ = internal global %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" { ptr null, i64 0, i64 0, i64 0, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@anon.39d3b15ada3f55b3c535c1be4b71a62a.0 = internal unnamed_addr constant i32 2

; Function Attrs: nofree noinline nounwind uwtable
define internal void @physprop_init_(ptr noalias nocapture readonly dereferenceable(4) %arg) #0 {
bb:
  %i = alloca i64, align 8
  store i64 5, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 5), align 8
  store i64 200, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 1), align 8
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 4), align 8
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 2), align 8
  %i1 = load i32, ptr %arg, align 1
  %i2 = sext i32 %i1 to i64
  %i3 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %i3, align 1
  %i4 = icmp sgt i64 %i2, 0
  %i5 = select i1 %i4, i64 %i2, i64 0
  %i6 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 %i5, ptr %i6, align 1
  %i7 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 200, ptr %i7, align 1
  %i8 = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %i, i32 2, i64 %i5, i64 200) #7
  %i9 = load i64, ptr %i, align 8
  %i10 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  %i11 = and i64 %i10, -68451041281
  %i12 = or i64 %i11, 1073741824
  store i64 %i12, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  %i13 = shl i32 %i8, 4
  %i14 = and i32 %i13, 16
  %i15 = or i32 %i14, 262144
  %i16 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 5), align 8
  %i17 = inttoptr i64 %i16 to ptr
  %i18 = tail call i32 @for_allocate_handle(i64 %i9, ptr @physpropmod_mp_physprop_, i32 %i15, ptr %i17) #7
  %i19 = icmp slt i32 %i1, 1
  br i1 %i19, label %bb100, label %bb20

bb20:                                             ; preds = %bb
  %i21 = add nuw nsw i32 %i1, 1
  %i22 = zext i32 %i21 to i64
  br label %bb23

bb23:                                             ; preds = %bb23, %bb20
  %i24 = phi i64 [ 1, %bb20 ], [ %i39, %bb23 ]
  %i25 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i26 = load i64, ptr %i7, align 1
  %i27 = load i64, ptr %i3, align 1
  %i28 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i27, i64 %i26, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i25, i64 %i24)
  %i29 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i28, i64 0, i32 0
  %i30 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i29, i64 0, i32 0
  store ptr null, ptr %i30, align 1
  %i31 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i29, i64 0, i32 3
  store i64 0, ptr %i31, align 1
  %i32 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i33 = load i64, ptr %i7, align 1
  %i34 = load i64, ptr %i3, align 1
  %i35 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i34, i64 %i33, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i32, i64 %i24)
  %i36 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i35, i64 0, i32 1
  %i37 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i36, i64 0, i32 0
  store ptr null, ptr %i37, align 1
  %i38 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i36, i64 0, i32 3
  store i64 0, ptr %i38, align 1
  %i39 = add nuw nsw i64 %i24, 1
  %i40 = icmp eq i64 %i39, %i22
  br i1 %i40, label %bb41, label %bb23

bb41:                                             ; preds = %bb41, %bb23
  %i42 = phi i64 [ %i97, %bb41 ], [ 1, %bb23 ]
  %i43 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i44 = load i64, ptr %i7, align 1
  %i45 = load i64, ptr %i3, align 1
  %i46 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i45, i64 %i44, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i43, i64 %i42)
  %i47 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i46, i64 0, i32 0
  %i48 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i47, i64 0, i32 3
  store i64 5, ptr %i48, align 1
  %i49 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i47, i64 0, i32 5
  store i64 0, ptr %i49, align 1
  %i50 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i47, i64 0, i32 1
  store i64 4, ptr %i50, align 1
  %i51 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i47, i64 0, i32 4
  store i64 2, ptr %i51, align 1
  %i52 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i47, i64 0, i32 2
  store i64 0, ptr %i52, align 1
  %i53 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i47, i64 0, i32 6, i64 0
  %i54 = getelementptr inbounds { i64, i64, i64 }, ptr %i53, i64 0, i32 2
  %i55 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i54, i32 0)
  store i64 1, ptr %i55, align 1
  %i56 = getelementptr inbounds { i64, i64, i64 }, ptr %i53, i64 0, i32 0
  %i57 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i56, i32 0)
  store i64 1000, ptr %i57, align 1
  %i58 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i54, i32 1)
  store i64 1, ptr %i58, align 1
  %i59 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i56, i32 1)
  store i64 19, ptr %i59, align 1
  %i60 = getelementptr inbounds { i64, i64, i64 }, ptr %i53, i64 0, i32 1
  %i61 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i60, i32 0)
  store i64 4, ptr %i61, align 1
  %i62 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i60, i32 1)
  store i64 4000, ptr %i62, align 1
  %i63 = load i64, ptr %i48, align 1
  %i64 = and i64 %i63, -68451041281
  %i65 = or i64 %i64, 1073741824
  store i64 %i65, ptr %i48, align 1
  %i66 = load i64, ptr %i49, align 1
  %i67 = inttoptr i64 %i66 to ptr
  %i68 = bitcast ptr %i46 to ptr
  %i69 = tail call i32 @for_allocate_handle(i64 76000, ptr %i68, i32 262144, ptr %i67) #7
  %i70 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i71 = load i64, ptr %i7, align 1
  %i72 = load i64, ptr %i3, align 1
  %i73 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i72, i64 %i71, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i70, i64 %i42)
  %i74 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i73, i64 0, i32 1
  %i75 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i74, i64 0, i32 3
  store i64 5, ptr %i75, align 1
  %i76 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i74, i64 0, i32 5
  store i64 0, ptr %i76, align 1
  %i77 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i74, i64 0, i32 1
  store i64 4, ptr %i77, align 1
  %i78 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i74, i64 0, i32 4
  store i64 2, ptr %i78, align 1
  %i79 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i74, i64 0, i32 2
  store i64 0, ptr %i79, align 1
  %i80 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i74, i64 0, i32 6, i64 0
  %i81 = getelementptr inbounds { i64, i64, i64 }, ptr %i80, i64 0, i32 2
  %i82 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i81, i32 0)
  store i64 1, ptr %i82, align 1
  %i83 = getelementptr inbounds { i64, i64, i64 }, ptr %i80, i64 0, i32 0
  %i84 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i83, i32 0)
  store i64 1000, ptr %i84, align 1
  %i85 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i81, i32 1)
  store i64 1, ptr %i85, align 1
  %i86 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i83, i32 1)
  store i64 19, ptr %i86, align 1
  %i87 = getelementptr inbounds { i64, i64, i64 }, ptr %i80, i64 0, i32 1
  %i88 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i87, i32 0)
  store i64 4, ptr %i88, align 1
  %i89 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i87, i32 1)
  store i64 4000, ptr %i89, align 1
  %i90 = load i64, ptr %i75, align 1
  %i91 = and i64 %i90, -68451041281
  %i92 = or i64 %i91, 1073741824
  store i64 %i92, ptr %i75, align 1
  %i93 = load i64, ptr %i76, align 1
  %i94 = inttoptr i64 %i93 to ptr
  %i95 = bitcast ptr %i74 to ptr
  %i96 = tail call i32 @for_allocate_handle(i64 76000, ptr nonnull %i95, i32 262144, ptr %i94) #7
  %i97 = add nuw i64 %i42, 1
  %i98 = trunc i64 %i97 to i32
  %i99 = icmp slt i32 %i1, %i98
  br i1 %i99, label %bb100, label %bb41

bb100:                                            ; preds = %bb41, %bb
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_check_mult_overflow64(ptr nocapture, i32, ...) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind willreturn writeonly uwtable
define internal void @physprop_pass_(ptr noalias nocapture dereferenceable(4) %arg) #2 {
bb:
  store float 1.500000e+01, ptr %arg, align 1
  ret void
}

; Function Attrs: nofree noinline nosync nounwind uwtable
define internal void @physprop_use_(ptr noalias nocapture readonly dereferenceable(4) %arg) #3 {
bb:
  %i = load i32, ptr %arg, align 1
  %i1 = sitofp i32 %i to float
  %i2 = fptosi float %i1 to i64
  %i3 = icmp slt i64 %i2, 1
  br i1 %i3, label %bb68, label %bb4

bb4:                                              ; preds = %bb
  %i5 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  %i6 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  br label %bb7

bb7:                                              ; preds = %bb64, %bb4
  %i8 = phi float [ %i65, %bb64 ], [ 1.000000e+00, %bb4 ]
  %i9 = phi i64 [ %i66, %bb64 ], [ %i2, %bb4 ]
  %i10 = fptosi float %i8 to i32
  %i11 = sext i32 %i10 to i64
  br label %bb12

bb12:                                             ; preds = %bb61, %bb7
  %i13 = phi i64 [ %i62, %bb61 ], [ 1, %bb7 ]
  br label %bb14

bb14:                                             ; preds = %bb14, %bb12
  %i15 = phi i64 [ %i59, %bb14 ], [ 1, %bb12 ]
  %i16 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i17 = load i64, ptr %i5, align 1
  %i18 = load i64, ptr %i6, align 1
  %i19 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i18, i64 %i17, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i16, i64 %i11)
  %i20 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i19, i64 0, i32 1
  %i21 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i20, i64 0, i32 0
  %i22 = load ptr, ptr %i21, align 1
  %i23 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i20, i64 0, i32 6, i64 0
  %i24 = getelementptr inbounds { i64, i64, i64 }, ptr %i23, i64 0, i32 1
  %i25 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i24, i32 0)
  %i26 = load i64, ptr %i25, align 1
  %i27 = getelementptr inbounds { i64, i64, i64 }, ptr %i23, i64 0, i32 2
  %i28 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i27, i32 0)
  %i29 = load i64, ptr %i28, align 1
  %i30 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i24, i32 1)
  %i31 = load i64, ptr %i30, align 1
  %i32 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i27, i32 1)
  %i33 = load i64, ptr %i32, align 1
  %i34 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i33, i64 %i31, ptr elementtype(float) %i22, i64 %i15)
  %i35 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i29, i64 %i26, ptr elementtype(float) %i34, i64 %i13)
  %i36 = load float, ptr %i35, align 1
  %i37 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i19, i64 0, i32 0
  %i38 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i37, i64 0, i32 0
  %i39 = load ptr, ptr %i38, align 1
  %i40 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i37, i64 0, i32 6, i64 0
  %i41 = getelementptr inbounds { i64, i64, i64 }, ptr %i40, i64 0, i32 1
  %i42 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i41, i32 0)
  %i43 = load i64, ptr %i42, align 1
  %i44 = getelementptr inbounds { i64, i64, i64 }, ptr %i40, i64 0, i32 2
  %i45 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i44, i32 0)
  %i46 = load i64, ptr %i45, align 1
  %i47 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i41, i32 1)
  %i48 = load i64, ptr %i47, align 1
  %i49 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i44, i32 1)
  %i50 = load i64, ptr %i49, align 1
  %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i50, i64 %i48, ptr elementtype(float) %i39, i64 %i15)
  %i52 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i46, i64 %i43, ptr elementtype(float) %i51, i64 %i13)
  store float %i36, ptr %i52, align 1
  %i53 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i54 = load i64, ptr %i5, align 1
  %i55 = load i64, ptr %i6, align 1
  %i56 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i55, i64 %i54, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i53, i64 %i11)
  %i57 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i56, i64 0, i32 2
  %i58 = load ptr, ptr %i57, align 1
  tail call void @physprop_pass_(ptr %i58)
  %i59 = add nuw nsw i64 %i15, 1
  %i60 = icmp eq i64 %i59, 20
  br i1 %i60, label %bb61, label %bb14

bb61:                                             ; preds = %bb14
  %i62 = add nuw nsw i64 %i13, 1
  %i63 = icmp eq i64 %i62, 1001
  br i1 %i63, label %bb64, label %bb12

bb64:                                             ; preds = %bb61
  %i65 = fadd reassoc ninf nsz arcp contract afn float %i8, 1.000000e+00
  %i66 = add nsw i64 %i9, -1
  %i67 = icmp sgt i64 %i9, 1
  br i1 %i67, label %bb7, label %bb68

bb68:                                             ; preds = %bb64, %bb
  ret void
}

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #4 {
bb:
  %i = alloca i32, align 8
  %i1 = alloca i32, align 8
  %i2 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.39d3b15ada3f55b3c535c1be4b71a62a.0) #7
  call void @physprop_init_(ptr nonnull %i1)
  call void @physprop_use_(ptr nonnull %i)
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: nocallback nofree nosync nounwind readnone willreturn
declare i32 @llvm.ssa.copy.i32(i32 returned) #5

; Function Attrs: nocallback nofree nosync nounwind readnone willreturn
declare i64 @llvm.ssa.copy.i64(i64 returned) #5

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #6

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #6

attributes #0 = { nofree noinline nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { mustprogress nofree noinline norecurse nosync nounwind willreturn writeonly uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #3 = { nofree noinline nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #4 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #5 = { nocallback nofree nosync nounwind readnone willreturn }
attributes #6 = { nounwind readnone speculatable }
attributes #7 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}

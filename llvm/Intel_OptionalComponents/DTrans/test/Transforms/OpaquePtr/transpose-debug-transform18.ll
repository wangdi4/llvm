; REQUIRES; asserts
; RUN: opt -opaque-pointers < %s -dva-check-dtrans-outofboundsok -dtrans-outofboundsok=false -disable-output -dtrans-transpose  -debug-only=dtrans-transpose-transform 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers < %s -dva-check-dtrans-outofboundsok -dtrans-outofboundsok=false -disable-output -passes=dtrans-transpose -debug-only=dtrans-transpose-transform 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Check that physpropmod_mp_physprop_ field 0 is transposed even though field 1
; is a variable of type real and passed down the call chain to an external C
; routine which treats it as a pointer to a character array, as long as
; -dtrans-outofboundsok=false.

; CHECK-LABEL: Transform candidate: physpropmod_mp_physprop_[0]
; CHECK: Before: get_hygro_rad_props_:  %[[N0:[A-Za-z0-9]+]] = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %[[X0:[A-Za-z0-9]+]], ptr elementtype(float) %[[I0:[A-Za-z0-9]+]], i64 %[[J0:[A-Za-z0-9]+]])
; CHECK: After : get_hygro_rad_props_:  %[[N0]] = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %[[I0]], i64 %[[J0]])
; CHECK: Before: get_hygro_rad_props_:  %[[N1:[A-Za-z0-9]+]] = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %[[X1:[A-Za-z0-9]+]], ptr elementtype(float) %[[I1:[A-Za-z0-9]+]], i64 %[[J1:[A-Za-z0-9]+]])
; CHECK: After : get_hygro_rad_props_:  %[[N1]] = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 76, ptr elementtype(float) %[[I1]], i64 %[[J1]])

%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"PHYSPROPMOD$.btPHYSPROP_TYPE" = type { %"QNCA_a0$float*$rank2$", float }
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
  store i64 104, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 1), align 8
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 4), align 8
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 2), align 8
  %i1 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %i1, align 1
  %i2 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 10, ptr %i2, align 1
  %i3 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 104, ptr %i3, align 1
  %i4 = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %i, i32 2, i64 10, i64 104) #10
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
  %i14 = tail call i32 @for_allocate_handle(i64 %i5, ptr @physpropmod_mp_physprop_, i32 %i11, ptr %i13) #10
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
  %i31 = phi float [ %i68, %bb30 ], [ 1.000000e+00, %bb15 ]
  %i32 = phi i64 [ %i69, %bb30 ], [ 10, %bb15 ]
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
  %i61 = tail call i32 @for_allocate_handle(i64 76000, ptr %i60, i32 262144, ptr %i59) #10
  %i62 = fmul reassoc ninf nsz arcp contract afn float %i31, 5.000000e+00
  %i63 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i64 = load i64, ptr %i3, align 1
  %i65 = load i64, ptr %i1, align 1
  %i66 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i65, i64 %i64, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i63, i64 %i37)
  %i67 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i66, i64 0, i32 1
  store float %i62, ptr %i67, align 1
  %i68 = fadd reassoc ninf nsz arcp contract afn float %i31, 1.000000e+00
  %i69 = add nsw i64 %i32, -1
  %i70 = icmp sgt i64 %i32, 1
  br i1 %i70, label %bb30, label %bb71

bb71:                                             ; preds = %bb30
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

; Function Attrs: noinline nounwind uwtable
define internal void @get_hygro_rad_props_(ptr noalias nocapture readonly dereferenceable(4) %arg, ptr noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %arg1, ptr noalias nocapture readnone dereferenceable(4) %arg2, ptr noalias readonly dereferenceable(4) %arg3) #3 {
bb:
  %i = alloca float, align 8
  call void (...) @getbytes_(ptr nonnull %arg3, ptr nonnull %i) #10
  %i4 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %arg1, i64 0, i32 0
  %i5 = load ptr, ptr %i4, align 1
  %i6 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %arg1, i64 0, i32 6, i64 0
  %i7 = getelementptr inbounds { i64, i64, i64 }, ptr %i6, i64 0, i32 1
  %i8 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i7, i32 0)
  %i9 = load i64, ptr %i8, align 1
  %i10 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i7, i32 1)
  %i11 = load i64, ptr %i10, align 1
  br label %bb12

bb12:                                             ; preds = %bb35, %bb
  %i13 = phi i64 [ %i36, %bb35 ], [ 1, %bb ]
  %i14 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i11, ptr elementtype(float) %i5, i64 %i13)
  %i15 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 416, ptr nonnull elementtype(float) @"main_$TAU", i64 %i13)
  br label %bb16

bb16:                                             ; preds = %bb32, %bb12
  %i17 = phi i64 [ %i33, %bb32 ], [ 1, %bb12 ]
  br label %bb18

bb18:                                             ; preds = %bb18, %bb16
  %i19 = phi i64 [ %i30, %bb18 ], [ 1, %bb16 ]
  %i20 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 16, ptr nonnull elementtype(i32) @"main_$KRH", i64 %i19)
  %i21 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %i20, i64 %i17)
  %i22 = load i32, ptr %i21, align 1
  %i23 = add nsw i32 %i22, 1
  %i24 = sext i32 %i23 to i64
  %i25 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i9, ptr elementtype(float) %i14, i64 %i24)
  %i26 = load float, ptr %i25, align 1
  %i27 = fadd reassoc ninf nsz arcp contract afn float %i26, %i26
  %i28 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 16, ptr nonnull elementtype(float) %i15, i64 %i19)
  %i29 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %i28, i64 %i17)
  store float %i27, ptr %i29, align 1
  %i30 = add nuw nsw i64 %i19, 1
  %i31 = icmp eq i64 %i30, 27
  br i1 %i31, label %bb32, label %bb18

bb32:                                             ; preds = %bb18
  %i33 = add nuw nsw i64 %i17, 1
  %i34 = icmp eq i64 %i33, 5
  br i1 %i34, label %bb35, label %bb16

bb35:                                             ; preds = %bb32
  %i36 = add nuw nsw i64 %i13, 1
  %i37 = icmp eq i64 %i36, 20
  br i1 %i37, label %bb38, label %bb12

bb38:                                             ; preds = %bb35
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

; Function Attrs: noinline noreturn nounwind uwtable
define internal void @bigloop_(ptr noalias nocapture readnone dereferenceable(4) %arg, ptr noalias nocapture readonly dereferenceable(4) %arg1) #5 {
bb:
  %i = alloca i32, align 8
  %i2 = alloca %"QNCA_a0$float*$rank2$", align 8
  %i3 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i2, i64 0, i32 0
  store ptr null, ptr %i3, align 8
  %i4 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i2, i64 0, i32 3
  store i64 0, ptr %i4, align 8
  store i32 1, ptr %i, align 8
  %i5 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  %i6 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  br label %bb7

bb7:                                              ; preds = %bb7, %bb
  %i8 = phi i32 [ %i15, %bb7 ], [ 1, %bb ]
  call void @physprop_get_(ptr nonnull %i, ptr nonnull %i2)
  %i9 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i10 = load i64, ptr %i5, align 1
  %i11 = load i64, ptr %i6, align 1
  %i12 = zext i32 %i8 to i64
  %i13 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i11, i64 %i10, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i9, i64 %i12)
  %i14 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i13, i64 0, i32 1
  call void @get_hygro_rad_props_(ptr nonnull @"main_$KRH", ptr nonnull %i2, ptr nonnull @"main_$TAU", ptr nonnull %i14)
  %i15 = add nuw nsw i32 %i8, 1
  store i32 %i15, ptr %i, align 8
  br label %bb7
}

; Function Attrs: noreturn nounwind uwtable
define dso_local void @MAIN__() #6 {
bb:
  %i = tail call i32 @for_set_reentrancy(ptr nonnull @anon.5b3c5dff38f14be85a38ea3af0f7d558.0) #10
  tail call void @init_krh_(ptr @"main_$KRH")
  tail call void @physprop_init_(ptr nonnull @anon.5b3c5dff38f14be85a38ea3af0f7d558.1)
  tail call void @bigloop_(ptr @"main_$TAU", ptr @"main_$KRH")
  unreachable
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
define internal void @getbytes_(ptr nocapture readonly %arg, ptr nocapture %arg1) #7 {
bb:
  br label %bb3

bb2:                                              ; preds = %bb3
  ret void

bb3:                                              ; preds = %bb3, %bb
  %i = phi i32 [ 0, %bb ], [ %i9, %bb3 ]
  %i4 = phi ptr [ %arg1, %bb ], [ %i8, %bb3 ]
  %i5 = phi ptr [ %arg, %bb ], [ %i6, %bb3 ]
  %i6 = getelementptr inbounds i8, ptr %i5, i64 1
  %i7 = load i8, ptr %i5, align 1, !tbaa !7
  %i8 = getelementptr inbounds i8, ptr %i4, i64 1
  store i8 %i7, ptr %i4, align 1, !tbaa !7
  %i9 = add nuw nsw i32 %i, 1
  %i10 = icmp eq i32 %i9, 8
  br i1 %i10, label %bb2, label %bb3, !llvm.loop !10
}

; Function Attrs: nocallback nofree nosync nounwind readnone willreturn
declare i64 @llvm.ssa.copy.i64(i64 returned) #8

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #9

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #9

attributes #0 = { nofree noinline nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { mustprogress nofree noinline nosync nounwind willreturn uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #3 = { noinline nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #4 = { nofree noinline nosync nounwind writeonly uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #5 = { noinline noreturn nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #6 = { noreturn nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #7 = { mustprogress nofree norecurse nosync nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #8 = { nocallback nofree nosync nounwind readnone willreturn }
attributes #9 = { nounwind readnone speculatable }
attributes #10 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2, !3, !4, !5}
!llvm.ident = !{!6}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"wchar_size", i32 4}
!3 = !{i32 1, !"Virtual Function Elim", i32 0}
!4 = !{i32 7, !"uwtable", i32 1}
!5 = !{i32 1, !"LTOPostLink", i32 1}
!6 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!7 = !{!8, !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = distinct !{!10, !11}
!11 = !{!"llvm.loop.mustprogress"}

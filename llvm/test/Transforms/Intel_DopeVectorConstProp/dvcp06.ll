; REQUIRES: asserts
; RUN: opt < %s -dope-vector-local-const-prop=false -disable-output -passes=dopevectorconstprop -debug-only=dopevectorconstprop 2>&1 | FileCheck %s

; Check that dope vector constants are not recognized because AVX2 is not
; specified.

; Check the trace output.

; CHECK: DOPE VECTOR CONSTANT PROPAGATION: BEGIN
; CHECK: NOT AVX2
; CHECK: DOPE VECTOR CONSTANT PROPAGATION: END

@"main_$PART" = internal global [9 x [9 x i32]] zeroinitializer, align 16
@"main_$BLOCK" = internal global [9 x [9 x [9 x i32]]] zeroinitializer, align 16
@0 = internal unnamed_addr constant i32 2
@anon.a87c7c812e60d4624ad0dfec6a834de1.0 = internal unnamed_addr constant i32 2

declare dso_local i32 @for_set_reentrancy(ptr) local_unnamed_addr

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare ptr @llvm.stacksave() #0

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore(ptr) #0

define dso_local void @MAIN__() #1 {
bb:
  %i = alloca { i64, ptr }, align 8
  %i1 = alloca [4 x i8], align 1
  %i2 = alloca { i64, ptr }, align 8
  %i3 = alloca [4 x i8], align 1
  %i4 = alloca { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, align 8
  %i5 = alloca { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %i6 = alloca [8 x i64], align 16
  %i7 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i5, i64 0, i32 1
  %i8 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i5, i64 0, i32 3
  %i9 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i5, i64 0, i32 6, i64 0
  %i10 = getelementptr inbounds { i64, i64, i64 }, ptr %i9, i64 0, i32 0
  %i11 = getelementptr inbounds { i64, i64, i64 }, ptr %i9, i64 0, i32 2
  %i12 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i4, i64 0, i32 1
  %i13 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i4, i64 0, i32 3
  %i14 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i4, i64 0, i32 6, i64 0
  %i15 = getelementptr inbounds { i64, i64, i64 }, ptr %i14, i64 0, i32 1
  %i16 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.a87c7c812e60d4624ad0dfec6a834de1.0)
  br label %bb17

bb17:                                             ; preds = %bb27, %bb
  %i18 = phi i64 [ %i28, %bb27 ], [ 1, %bb ]
  br label %bb19

bb19:                                             ; preds = %bb19, %bb17
  %i20 = phi i64 [ %i25, %bb19 ], [ 1, %bb17 ]
  %i21 = sub nsw i64 %i18, %i20
  %i22 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) @"main_$PART", i64 %i20)
  %i23 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i22, i64 %i18)
  %i24 = trunc i64 %i21 to i32
  store i32 %i24, ptr %i23, align 4
  %i25 = add nuw nsw i64 %i20, 1
  %i26 = icmp eq i64 %i25, 10
  br i1 %i26, label %bb27, label %bb19

bb27:                                             ; preds = %bb19
  %i28 = add nuw nsw i64 %i18, 1
  %i29 = icmp eq i64 %i28, 10
  br i1 %i29, label %bb30, label %bb17

bb30:                                             ; preds = %bb27
  %i31 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i5, i64 0, i32 0
  %i32 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i5, i64 0, i32 2
  %i33 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i5, i64 0, i32 4
  %i34 = getelementptr inbounds { i64, i64, i64 }, ptr %i9, i64 0, i32 1
  %i35 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i4, i64 0, i32 0
  %i36 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i4, i64 0, i32 2
  %i37 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i4, i64 0, i32 4
  %i38 = getelementptr inbounds { i64, i64, i64 }, ptr %i14, i64 0, i32 0
  %i39 = getelementptr inbounds { i64, i64, i64 }, ptr %i14, i64 0, i32 2
  store i64 4, ptr %i7, align 8
  store i64 2, ptr %i33, align 8
  store i64 0, ptr %i32, align 8
  %i40 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i34, i32 0)
  store i64 4, ptr %i40, align 8
  %i41 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i11, i32 0)
  store i64 1, ptr %i41, align 8
  %i42 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i10, i32 0)
  store i64 9, ptr %i42, align 8
  %i43 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i34, i32 1)
  store i64 36, ptr %i43, align 8
  %i44 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i11, i32 1)
  store i64 1, ptr %i44, align 8
  %i45 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i10, i32 1)
  store i64 9, ptr %i45, align 8
  store ptr @"main_$PART", ptr %i31, align 8
  store i64 1, ptr %i8, align 8
  store i64 4, ptr %i12, align 8
  store i64 3, ptr %i37, align 8
  store i64 0, ptr %i36, align 8
  %i46 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i15, i32 0)
  store i64 4, ptr %i46, align 8
  %i47 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i39, i32 0)
  store i64 1, ptr %i47, align 8
  %i48 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i38, i32 0)
  store i64 9, ptr %i48, align 8
  %i49 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i15, i32 1)
  store i64 36, ptr %i49, align 8
  %i50 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i39, i32 1)
  store i64 1, ptr %i50, align 8
  %i51 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i38, i32 1)
  store i64 9, ptr %i51, align 8
  %i52 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i15, i32 2)
  store i64 324, ptr %i52, align 8
  %i53 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i39, i32 2)
  store i64 1, ptr %i53, align 8
  %i54 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i38, i32 2)
  store i64 9, ptr %i54, align 8
  store ptr @"main_$BLOCK", ptr %i35, align 8
  store i64 1, ptr %i13, align 8
  call void @new_solver_(ptr nonnull %i5, ptr nonnull %i4)
  ret void
}

define internal void @new_solver_(ptr noalias nocapture readonly %arg, ptr noalias nocapture readonly %arg1) #1 {
bb:
  %i = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %arg1, i64 0, i32 0
  %i2 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %arg1, i64 0, i32 6, i64 0
  %i3 = getelementptr inbounds { i64, i64, i64 }, ptr %i2, i64 0, i32 0
  %i4 = getelementptr inbounds { i64, i64, i64 }, ptr %i2, i64 0, i32 1
  %i5 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %arg, i64 0, i32 0
  %i6 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %arg, i64 0, i32 6, i64 0
  %i7 = getelementptr inbounds { i64, i64, i64 }, ptr %i6, i64 0, i32 1
  %i8 = load ptr, ptr %i, align 8
  %i9 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i4, i32 0)
  %i10 = load i64, ptr %i9, align 8
  %i11 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i3, i32 0)
  %i12 = load i64, ptr %i11, align 8
  %i13 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i4, i32 1)
  %i14 = load i64, ptr %i13, align 8
  %i15 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i3, i32 1)
  %i16 = load i64, ptr %i15, align 8
  %i17 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i4, i32 2)
  %i18 = load i64, ptr %i17, align 8
  %i19 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i3, i32 2)
  %i20 = load i64, ptr %i19, align 8
  %i21 = icmp slt i64 %i20, 1
  %i22 = icmp slt i64 %i16, 1
  %i23 = or i1 %i21, %i22
  %i24 = icmp slt i64 %i12, 1
  %i25 = or i1 %i23, %i24
  br i1 %i25, label %bb43, label %bb40

bb26:                                             ; preds = %bb34, %bb26
  %i27 = phi i64 [ 1, %bb34 ], [ %i29, %bb26 ]
  %i28 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i10, ptr elementtype(i32) %i36, i64 %i27)
  store i32 0, ptr %i28, align 4
  %i29 = add nuw i64 %i27, 1
  %i30 = icmp eq i64 %i27, %i12
  br i1 %i30, label %bb31, label %bb26

bb31:                                             ; preds = %bb26
  %i32 = add nuw i64 %i35, 1
  %i33 = icmp eq i64 %i35, %i16
  br i1 %i33, label %bb37, label %bb34

bb34:                                             ; preds = %bb40, %bb31
  %i35 = phi i64 [ 1, %bb40 ], [ %i32, %bb31 ]
  %i36 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i14, ptr elementtype(i32) %i42, i64 %i35)
  br label %bb26

bb37:                                             ; preds = %bb31
  %i38 = add nuw i64 %i41, 1
  %i39 = icmp eq i64 %i41, %i20
  br i1 %i39, label %bb43, label %bb40

bb40:                                             ; preds = %bb37, %bb
  %i41 = phi i64 [ %i38, %bb37 ], [ 1, %bb ]
  %i42 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %i18, ptr elementtype(i32) %i8, i64 %i41)
  br label %bb34

bb43:                                             ; preds = %bb37, %bb
  %i44 = load ptr, ptr %i5, align 8
  %i45 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i7, i32 0)
  %i46 = load i64, ptr %i45, align 8
  %i47 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i7, i32 1)
  %i48 = load i64, ptr %i47, align 8
  br label %bb49

bb49:                                             ; preds = %bb81, %bb43
  %i50 = phi i64 [ 1, %bb43 ], [ %i82, %bb81 ]
  br label %bb51

bb51:                                             ; preds = %bb78, %bb49
  %i52 = phi i64 [ %i79, %bb78 ], [ 1, %bb49 ]
  %i53 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i48, ptr elementtype(i32) %i44, i64 %i52)
  %i54 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i46, ptr elementtype(i32) %i53, i64 %i50)
  %i55 = load i32, ptr %i54, align 4
  %i56 = icmp eq i32 %i55, 0
  br i1 %i56, label %bb74, label %bb78

bb57:                                             ; preds = %bb74, %bb57
  %i58 = phi i64 [ 1, %bb74 ], [ %i61, %bb57 ]
  %i59 = phi i32 [ 1, %bb74 ], [ %i62, %bb57 ]
  %i60 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %i77, i64 %i58)
  store i32 %i59, ptr %i60, align 4
  %i61 = add nuw nsw i64 %i58, 1
  %i62 = add nuw nsw i32 %i59, 1
  %i63 = icmp eq i64 %i61, 10
  br i1 %i63, label %bb64, label %bb57

bb64:                                             ; preds = %bb64, %bb57
  %i65 = phi i64 [ %i71, %bb64 ], [ 1, %bb57 ]
  %i66 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %i77, i64 %i65)
  %i67 = load i32, ptr %i66, align 4
  %i68 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %i18, ptr elementtype(i32) %i8, i64 %i65)
  %i69 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i14, ptr elementtype(i32) %i68, i64 %i52)
  %i70 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i10, ptr elementtype(i32) %i69, i64 %i50)
  store i32 %i67, ptr %i70, align 4
  %i71 = add nuw nsw i64 %i65, 1
  %i72 = icmp eq i64 %i71, 10
  br i1 %i72, label %bb73, label %bb64

bb73:                                             ; preds = %bb64
  tail call void @llvm.stackrestore(ptr %i75)
  br label %bb78

bb74:                                             ; preds = %bb51
  %i75 = tail call ptr @llvm.stacksave()
  %i76 = alloca [9 x i32], align 4
  %i77 = getelementptr inbounds [9 x i32], ptr %i76, i64 0, i64 0
  br label %bb57

bb78:                                             ; preds = %bb73, %bb51
  %i79 = add nuw nsw i64 %i52, 1
  %i80 = icmp eq i64 %i79, 10
  br i1 %i80, label %bb81, label %bb51

bb81:                                             ; preds = %bb78
  %i82 = add nuw nsw i64 %i50, 1
  %i83 = icmp eq i64 %i82, 10
  br i1 %i83, label %bb84, label %bb49

bb84:                                             ; preds = %bb81
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #2

attributes #0 = { nocallback nofree nosync nounwind willreturn }
attributes #1 = { "intel-lang"="fortran" }
attributes #2 = { nounwind readnone speculatable }

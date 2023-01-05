; REQUIRES: asserts
; RUN: opt < %s -opaque-pointers -dope-vector-local-const-prop=false -disable-output -passes=dopevectorconstprop -debug-only=dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

; Check that full load, stride, and extent dope vector constant values are
; determined for ARG #1 of new_solver_, but only some lower bound and stride
; constant values are determined for ARG #0.

; Check the trace output.

; CHECK: DOPE VECTOR CONSTANT PROPAGATION: BEGIN
; CHECK: DV FOUND: ARG #0 new_solver_ 2 x <UNKNOWN ELEMENT TYPE>
; CHECK: VALID
; CHECK: LB[0] = 1
; CHECK: ST[0] = 4
; CHECK-NOT: EX[0] = 9
; CHECK: LB[1] = 1
; CHECK-NOT: ST[1] = 36
; CHECK-NOT: EX[1] = 9
; CHECK: REPLACING 1 LOAD WITH 4
; CHECK-NOT: REPLACING 1 LOAD WITH 36
; CHECK: DV FOUND: ARG #1 new_solver_ 3 x <UNKNOWN ELEMENT TYPE>
; CHECK: VALID
; CHECK: LB[0] = 1
; CHECK: ST[0] = 4
; CHECK: EX[0] = 9
; CHECK: LB[1] = 1
; CHECK: ST[1] = 36
; CHECK: EX[1] = 9
; CHECK: LB[2] = 1
; CHECK: ST[2] = 324
; CHECK: EX[2] = 9
; CHECK: REPLACING 1 LOAD WITH 9
; CHECK: REPLACING 1 LOAD WITH 9
; CHECK: REPLACING 1 LOAD WITH 9
; CHECK: REPLACING 1 LOAD WITH 4
; CHECK: REPLACING 1 LOAD WITH 36
; CHECK: REPLACING 1 LOAD WITH 324
; CHECK: DOPE VECTOR CONSTANT PROPAGATION: END

@"main_$PART1" = internal global [9 x [9 x i32]] zeroinitializer, align 16
@"main_$PART2" = internal global [10 x [10 x i32]] zeroinitializer, align 16
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
  %i4 = alloca { i64, ptr }, align 8
  %i5 = alloca [4 x i8], align 1
  %i6 = alloca { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, align 8
  %i7 = alloca { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %i8 = alloca { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, align 8
  %i9 = alloca { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %i10 = alloca [8 x i64], align 16
  %i11 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i9, i64 0, i32 1
  %i12 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i9, i64 0, i32 3
  %i13 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i9, i64 0, i32 6, i64 0
  %i14 = getelementptr inbounds { i64, i64, i64 }, ptr %i13, i64 0, i32 0
  %i15 = getelementptr inbounds { i64, i64, i64 }, ptr %i13, i64 0, i32 2
  %i16 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i8, i64 0, i32 1
  %i17 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i8, i64 0, i32 3
  %i18 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i8, i64 0, i32 6, i64 0
  %i19 = getelementptr inbounds { i64, i64, i64 }, ptr %i18, i64 0, i32 0
  %i20 = getelementptr inbounds { i64, i64, i64 }, ptr %i18, i64 0, i32 2
  %i21 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i7, i64 0, i32 1
  %i22 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i7, i64 0, i32 3
  %i23 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i7, i64 0, i32 6, i64 0
  %i24 = getelementptr inbounds { i64, i64, i64 }, ptr %i23, i64 0, i32 0
  %i25 = getelementptr inbounds { i64, i64, i64 }, ptr %i23, i64 0, i32 2
  %i26 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i6, i64 0, i32 1
  %i27 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i6, i64 0, i32 3
  %i28 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i6, i64 0, i32 6, i64 0
  %i29 = getelementptr inbounds { i64, i64, i64 }, ptr %i28, i64 0, i32 1
  %i30 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.a87c7c812e60d4624ad0dfec6a834de1.0)
  br label %bb31

bb31:                                             ; preds = %bb41, %bb
  %i32 = phi i64 [ %i42, %bb41 ], [ 1, %bb ]
  br label %bb33

bb33:                                             ; preds = %bb33, %bb31
  %i34 = phi i64 [ %i39, %bb33 ], [ 1, %bb31 ]
  %i35 = sub nsw i64 %i32, %i34
  %i36 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) @"main_$PART1", i64 %i34)
  %i37 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i36, i64 %i32)
  %i38 = trunc i64 %i35 to i32
  store i32 %i38, ptr %i37, align 4
  %i39 = add nuw nsw i64 %i34, 1
  %i40 = icmp eq i64 %i39, 10
  br i1 %i40, label %bb41, label %bb33

bb41:                                             ; preds = %bb33
  %i42 = add nuw nsw i64 %i32, 1
  %i43 = icmp eq i64 %i42, 10
  br i1 %i43, label %bb44, label %bb31

bb44:                                             ; preds = %bb41
  %i45 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i9, i64 0, i32 0
  %i46 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i9, i64 0, i32 2
  %i47 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i9, i64 0, i32 4
  %i48 = getelementptr inbounds { i64, i64, i64 }, ptr %i13, i64 0, i32 1
  %i49 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i8, i64 0, i32 0
  %i50 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i8, i64 0, i32 2
  %i51 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i8, i64 0, i32 4
  %i52 = getelementptr inbounds { i64, i64, i64 }, ptr %i18, i64 0, i32 1
  %i53 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i7, i64 0, i32 0
  %i54 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i7, i64 0, i32 2
  %i55 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i7, i64 0, i32 4
  %i56 = getelementptr inbounds { i64, i64, i64 }, ptr %i23, i64 0, i32 1
  %i57 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i6, i64 0, i32 0
  %i58 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i6, i64 0, i32 2
  %i59 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i6, i64 0, i32 4
  %i60 = getelementptr inbounds { i64, i64, i64 }, ptr %i28, i64 0, i32 0
  %i61 = getelementptr inbounds { i64, i64, i64 }, ptr %i28, i64 0, i32 2
  br label %bb62

bb62:                                             ; preds = %bb72, %bb44
  %i63 = phi i64 [ 0, %bb44 ], [ %i73, %bb72 ]
  br label %bb64

bb64:                                             ; preds = %bb64, %bb62
  %i65 = phi i64 [ %i70, %bb64 ], [ 0, %bb62 ]
  %i66 = sub nsw i64 %i63, %i65
  %i67 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 40, ptr elementtype(i32) @"main_$PART2", i64 %i65)
  %i68 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 4, ptr elementtype(i32) %i67, i64 %i63)
  %i69 = trunc i64 %i66 to i32
  store i32 %i69, ptr %i68, align 4
  %i70 = add nuw nsw i64 %i65, 1
  %i71 = icmp eq i64 %i70, 10
  br i1 %i71, label %bb72, label %bb64

bb72:                                             ; preds = %bb64
  %i73 = add nuw nsw i64 %i63, 1
  %i74 = icmp eq i64 %i73, 10
  br i1 %i74, label %bb75, label %bb62

bb75:                                             ; preds = %bb72
  store i64 4, ptr %i11, align 8
  store i64 2, ptr %i47, align 8
  store i64 0, ptr %i46, align 8
  %i76 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i48, i32 0)
  store i64 4, ptr %i76, align 8
  %i77 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i15, i32 0)
  store i64 1, ptr %i77, align 8
  %i78 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i14, i32 0)
  store i64 9, ptr %i78, align 8
  %i79 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i48, i32 1)
  store i64 36, ptr %i79, align 8
  %i80 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i15, i32 1)
  store i64 1, ptr %i80, align 8
  %i81 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i14, i32 1)
  store i64 9, ptr %i81, align 8
  store ptr @"main_$PART1", ptr %i45, align 8
  store i64 1, ptr %i12, align 8
  store i64 4, ptr %i16, align 8
  store i64 3, ptr %i51, align 8
  store i64 0, ptr %i50, align 8
  %i82 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i52, i32 0)
  store i64 4, ptr %i82, align 8
  %i83 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i20, i32 0)
  store i64 1, ptr %i83, align 8
  %i84 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i19, i32 0)
  store i64 9, ptr %i84, align 8
  %i85 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i52, i32 1)
  store i64 36, ptr %i85, align 8
  %i86 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i20, i32 1)
  store i64 1, ptr %i86, align 8
  %i87 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i19, i32 1)
  store i64 9, ptr %i87, align 8
  %i88 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i52, i32 2)
  store i64 324, ptr %i88, align 8
  %i89 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i20, i32 2)
  store i64 1, ptr %i89, align 8
  %i90 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i19, i32 2)
  store i64 9, ptr %i90, align 8
  store ptr @"main_$BLOCK", ptr %i49, align 8
  store i64 1, ptr %i17, align 8
  call void @new_solver_(ptr nonnull %i9, ptr nonnull %i8)
  store i64 4, ptr %i21, align 8
  store i64 2, ptr %i55, align 8
  store i64 0, ptr %i54, align 8
  %i91 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i56, i32 0)
  store i64 4, ptr %i91, align 8
  %i92 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i25, i32 0)
  store i64 1, ptr %i92, align 8
  %i93 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i24, i32 0)
  store i64 10, ptr %i93, align 8
  %i94 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i56, i32 1)
  store i64 40, ptr %i94, align 8
  %i95 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i25, i32 1)
  store i64 1, ptr %i95, align 8
  %i96 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i24, i32 1)
  store i64 10, ptr %i96, align 8
  store ptr @"main_$PART2", ptr %i53, align 8
  store i64 1, ptr %i22, align 8
  store i64 4, ptr %i26, align 8
  store i64 3, ptr %i59, align 8
  store i64 0, ptr %i58, align 8
  %i97 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i29, i32 0)
  store i64 4, ptr %i97, align 8
  %i98 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i61, i32 0)
  store i64 1, ptr %i98, align 8
  %i99 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i60, i32 0)
  store i64 9, ptr %i99, align 8
  %i100 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i29, i32 1)
  store i64 36, ptr %i100, align 8
  %i101 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i61, i32 1)
  store i64 1, ptr %i101, align 8
  %i102 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i60, i32 1)
  store i64 9, ptr %i102, align 8
  %i103 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i29, i32 2)
  store i64 324, ptr %i103, align 8
  %i104 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i61, i32 2)
  store i64 1, ptr %i104, align 8
  %i105 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i60, i32 2)
  store i64 9, ptr %i105, align 8
  store ptr @"main_$BLOCK", ptr %i57, align 8
  store i64 1, ptr %i27, align 8
  call void @new_solver_(ptr nonnull %i7, ptr nonnull %i6)
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

; REQUIRES: asserts
; RUN: opt < %s -dope-vector-local-const-prop=false -disable-output -passes=dopevectorconstprop -debug-only=dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

; Check that full load, stride, and extent dope vector constant values are
; determined for ARG #0 and ARG #1 of new_solver_, but that they are not
; replaced because the base array of the per dimension arrays have been folded.

; Check the trace output.

; CHECK: DOPE VECTOR CONSTANT PROPAGATION: BEGIN
; CHECK: DV FOUND: ARG #0 new_solver_ 2 x <UNKNOWN ELEMENT TYPE>
; CHECK: VALID
; CHECK: LB[0] = 1
; CHECK: ST[0] = 4
; CHECK: EX[0] = 9
; CHECK: LB[1] = 1
; CHECK: ST[1] = 36
; CHECK: EX[1] = 9
; CHECK: COULD NOT FIND PER DIMENSION ARRAY
; CHECK: COULD NOT FIND PER DIMENSION ARRAY
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
; CHECK: COULD NOT FIND PER DIMENSION ARRAY
; CHECK: COULD NOT FIND PER DIMENSION ARRAY
; CHECK: COULD NOT FIND PER DIMENSION ARRAY
; CHECK: DOPE VECTOR CONSTANT PROPAGATION: END

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

@"main_$PART" = internal global [9 x [9 x i32]] zeroinitializer, align 16

@"main_$BLOCK" = internal global [9 x [9 x [9 x i32]]] zeroinitializer, align 16

@0 = internal unnamed_addr constant i32 2

@anon.a87c7c812e60d4624ad0dfec6a834de1.0 = internal unnamed_addr constant i32 2

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #2

declare dso_local i32 @for_set_reentrancy(ptr) local_unnamed_addr

declare ptr @llvm.stacksave() #0

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
  %i7 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i5, i64 0, i32 0
  %i8 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i5, i64 0, i32 2
  %i9 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i5, i64 0, i32 4
  %i10 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i5, i64 0, i32 6, i64 0, i32 1
  %i11 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i4, i64 0, i32 0
  %i12 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i4, i64 0, i32 2
  %i13 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i4, i64 0, i32 4
  %i14 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i4, i64 0, i32 6, i64 0, i32 1
  %i15 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.a87c7c812e60d4624ad0dfec6a834de1.0)
  br label %bb16

bb16:                                             ; preds = %bb26, %bb
  %i17 = phi i64 [ %i27, %bb26 ], [ 1, %bb ]
  br label %bb18

bb18:                                             ; preds = %bb18, %bb16
  %i19 = phi i64 [ %i24, %bb18 ], [ 1, %bb16 ]
  %i20 = sub nsw i64 %i17, %i19
  %i21 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) @"main_$PART", i64 %i19)
  %i22 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i21, i64 %i17)
  %i23 = trunc i64 %i20 to i32
  store i32 %i23, ptr %i22, align 4
  %i24 = add nuw nsw i64 %i19, 1
  %i25 = icmp eq i64 %i24, 10
  br i1 %i25, label %bb26, label %bb18

bb26:                                             ; preds = %bb18
  %i27 = add nuw nsw i64 %i17, 1
  %i28 = icmp eq i64 %i27, 10
  br i1 %i28, label %bb29, label %bb16

bb29:                                             ; preds = %bb26
  %i30 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i5, i64 0, i32 1
  %i31 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i5, i64 0, i32 3
  %i32 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i5, i64 0, i32 6, i64 0, i32 0
  %i33 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i5, i64 0, i32 6, i64 0, i32 2
  %i34 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i4, i64 0, i32 1
  %i35 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i4, i64 0, i32 3
  %i36 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i4, i64 0, i32 6, i64 0, i32 0
  %i37 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %i4, i64 0, i32 6, i64 0, i32 2
  store i64 4, ptr %i30, align 8
  store i64 2, ptr %i9, align 8
  store i64 0, ptr %i8, align 8
  %i38 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i10, i32 0)
  store i64 4, ptr %i38, align 8
  %i39 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i33, i32 0)
  store i64 1, ptr %i39, align 8
  %i40 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i32, i32 0)
  store i64 9, ptr %i40, align 8
  %i41 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i10, i32 1)
  store i64 36, ptr %i41, align 8
  %i42 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i33, i32 1)
  store i64 1, ptr %i42, align 8
  %i43 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i32, i32 1)
  store i64 9, ptr %i43, align 8
  store ptr @"main_$PART", ptr %i7, align 8
  store i64 1, ptr %i31, align 8
  store i64 4, ptr %i34, align 8
  store i64 3, ptr %i13, align 8
  store i64 0, ptr %i12, align 8
  %i44 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i14, i32 0)
  store i64 4, ptr %i44, align 8
  %i45 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i37, i32 0)
  store i64 1, ptr %i45, align 8
  %i46 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i36, i32 0)
  store i64 9, ptr %i46, align 8
  %i47 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i14, i32 1)
  store i64 36, ptr %i47, align 8
  %i48 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i37, i32 1)
  store i64 1, ptr %i48, align 8
  %i49 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i36, i32 1)
  store i64 9, ptr %i49, align 8
  %i50 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i14, i32 2)
  store i64 324, ptr %i50, align 8
  %i51 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i37, i32 2)
  store i64 1, ptr %i51, align 8
  %i52 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i36, i32 2)
  store i64 9, ptr %i52, align 8
  store ptr @"main_$BLOCK", ptr %i11, align 8
  store i64 1, ptr %i35, align 8
  call void @new_solver_(ptr nonnull %i5, ptr nonnull %i4)
  ret void
}

define internal void @new_solver_(ptr noalias nocapture readonly %arg, ptr noalias nocapture readonly %arg1) #1 {
bb:
  %i = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %arg1, i64 0, i32 0
  %i2 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %arg1, i64 0, i32 6, i64 0, i32 0
  %i3 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %arg1, i64 0, i32 6, i64 0, i32 1
  %i4 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %arg, i64 0, i32 0
  %i5 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %arg, i64 0, i32 6, i64 0, i32 1
  %i6 = load ptr, ptr %i, align 8
  %i7 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i3, i32 0)
  %i8 = load i64, ptr %i7, align 8
  %i9 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i2, i32 0)
  %i10 = load i64, ptr %i9, align 8
  %i11 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i3, i32 1)
  %i12 = load i64, ptr %i11, align 8
  %i13 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i2, i32 1)
  %i14 = load i64, ptr %i13, align 8
  %i15 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i3, i32 2)
  %i16 = load i64, ptr %i15, align 8
  %i17 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i2, i32 2)
  %i18 = load i64, ptr %i17, align 8
  %i19 = icmp slt i64 %i18, 1
  %i20 = icmp slt i64 %i14, 1
  %i21 = or i1 %i19, %i20
  %i22 = icmp slt i64 %i10, 1
  %i23 = or i1 %i21, %i22
  br i1 %i23, label %bb41, label %bb38

bb24:                                             ; preds = %bb32, %bb24
  %i25 = phi i64 [ 1, %bb32 ], [ %i27, %bb24 ]
  %i26 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i8, ptr elementtype(i32) %i34, i64 %i25)
  store i32 0, ptr %i26, align 4
  %i27 = add nuw i64 %i25, 1
  %i28 = icmp eq i64 %i25, %i10
  br i1 %i28, label %bb29, label %bb24

bb29:                                             ; preds = %bb24
  %i30 = add nuw i64 %i33, 1
  %i31 = icmp eq i64 %i33, %i14
  br i1 %i31, label %bb35, label %bb32

bb32:                                             ; preds = %bb38, %bb29
  %i33 = phi i64 [ 1, %bb38 ], [ %i30, %bb29 ]
  %i34 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i12, ptr elementtype(i32) %i40, i64 %i33)
  br label %bb24

bb35:                                             ; preds = %bb29
  %i36 = add nuw i64 %i39, 1
  %i37 = icmp eq i64 %i39, %i18
  br i1 %i37, label %bb41, label %bb38

bb38:                                             ; preds = %bb35, %bb
  %i39 = phi i64 [ %i36, %bb35 ], [ 1, %bb ]
  %i40 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %i16, ptr elementtype(i32) %i6, i64 %i39)
  br label %bb32

bb41:                                             ; preds = %bb35, %bb
  %i42 = load ptr, ptr %i4, align 8
  %i43 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i5, i32 0)
  %i44 = load i64, ptr %i43, align 8
  %i45 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i5, i32 1)
  %i46 = load i64, ptr %i45, align 8
  br label %bb47

bb47:                                             ; preds = %bb79, %bb41
  %i48 = phi i64 [ 1, %bb41 ], [ %i80, %bb79 ]
  br label %bb49

bb49:                                             ; preds = %bb76, %bb47
  %i50 = phi i64 [ %i77, %bb76 ], [ 1, %bb47 ]
  %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i46, ptr elementtype(i32) %i42, i64 %i50)
  %i52 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i44, ptr elementtype(i32) %i51, i64 %i48)
  %i53 = load i32, ptr %i52, align 4
  %i54 = icmp eq i32 %i53, 0
  br i1 %i54, label %bb72, label %bb76

bb55:                                             ; preds = %bb72, %bb55
  %i56 = phi i64 [ 1, %bb72 ], [ %i59, %bb55 ]
  %i57 = phi i32 [ 1, %bb72 ], [ %i60, %bb55 ]
  %i58 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %i75, i64 %i56)
  store i32 %i57, ptr %i58, align 4
  %i59 = add nuw nsw i64 %i56, 1
  %i60 = add nuw nsw i32 %i57, 1
  %i61 = icmp eq i64 %i59, 10
  br i1 %i61, label %bb62, label %bb55

bb62:                                             ; preds = %bb62, %bb55
  %i63 = phi i64 [ %i69, %bb62 ], [ 1, %bb55 ]
  %i64 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %i75, i64 %i63)
  %i65 = load i32, ptr %i64, align 4
  %i66 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %i16, ptr elementtype(i32) %i6, i64 %i63)
  %i67 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i12, ptr elementtype(i32) %i66, i64 %i50)
  %i68 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i8, ptr elementtype(i32) %i67, i64 %i48)
  store i32 %i65, ptr %i68, align 4
  %i69 = add nuw nsw i64 %i63, 1
  %i70 = icmp eq i64 %i69, 10
  br i1 %i70, label %bb71, label %bb62

bb71:                                             ; preds = %bb62
  tail call void @llvm.stackrestore(ptr %i73)
  br label %bb76

bb72:                                             ; preds = %bb49
  %i73 = tail call ptr @llvm.stacksave()
  %i74 = alloca [9 x i32], align 4
  %i75 = getelementptr inbounds [9 x i32], ptr %i74, i64 0, i64 0
  br label %bb55

bb76:                                             ; preds = %bb71, %bb49
  %i77 = add nuw nsw i64 %i50, 1
  %i78 = icmp eq i64 %i77, 10
  br i1 %i78, label %bb79, label %bb49

bb79:                                             ; preds = %bb76
  %i80 = add nuw nsw i64 %i48, 1
  %i81 = icmp eq i64 %i80, 10
  br i1 %i81, label %bb82, label %bb47

bb82:                                             ; preds = %bb79
  ret void
}

attributes #0 = { nocallback nofree nosync nounwind willreturn }
attributes #1 = { "intel-lang"="fortran" }
attributes #2 = { nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }

; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts
; RUN: opt -opaque-pointers < %s -passes='module(ip-cloning)' -ip-gen-cloning-force-enable-dtrans -debug-only=ipcloning -S 2>&1 | FileCheck %s

; Test that the function @bad1 is recognized as a recursive progression clone
; and eight clones of it are created, but that it is not a candidate for
; creating an extra clone because its first loop does not have the desired
; form.

; CHECK: Enter IP cloning: (Before inlining)
; CHECK: Cloning Analysis for:  bad1
; CHECK: Selected RecProgression cloning
; CHECK: Function: bad1.1
; CHECK: ArgPos : 0
; CHECK: Argument : ptr %arg
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 1
; CHECK: Function: bad1.2
; CHECK: ArgPos : 0
; CHECK: Argument : ptr %arg
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 2
; CHECK: Function: bad1.3
; CHECK: ArgPos : 0
; CHECK: Argument : ptr %arg
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 3
; CHECK: Function: bad1.4
; CHECK: ArgPos : 0
; CHECK: Argument : ptr %arg
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 4
; CHECK: Function: bad1.5
; CHECK: ArgPos : 0
; CHECK: Argument : ptr %arg
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 5
; CHECK: Function: bad1.6
; CHECK: ArgPos : 0
; CHECK: Argument : ptr %arg
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 6
; CHECK: Function: bad1.7
; CHECK: ArgPos : 0
; CHECK: Argument : ptr %arg
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 7
; CHECK: Function: bad1.8
; CHECK: ArgPos : 0
; CHECK: Argument : ptr %arg
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 8
; CHECK-NOT: Extra RecProClone Candidate: bad1.8

; CHECK: define dso_local void @MAIN__()
; CHECK: call void @bad1.1
; CHECK: define internal void @bad1.1
; CHECK: call void @bad1.2
; CHECK: define internal void @bad1.2
; CHECK: call void @bad1.3
; CHECK: define internal void @bad1.3
; CHECK: call void @bad1.4
; CHECK: define internal void @bad1.4
; CHECK: call void @bad1.5
; CHECK: define internal void @bad1.5
; CHECK: call void @bad1.6
; CHECK: define internal void @bad1.6
; CHECK: call void @bad1.7
; CHECK: define internal void @bad1.7
; CHECK: call void @bad1.8
; CHECK: define internal void @bad1.8

@brute_force_mp_sudoku1_ = common dso_local global [9 x [9 x i32]] zeroinitializer, align 8
@brute_force_mp_sudoku2_ = common dso_local global [9 x [9 x i32]] zeroinitializer, align 8
@brute_force_mp_sudoku3_ = common dso_local global [9 x [9 x i32]] zeroinitializer, align 8
@brute_force_mp_block_ = common dso_local global [9 x [9 x [9 x i32]]] zeroinitializer, align 8
@brute_force_mp_soln_ = common dso_local global i32 0, align 8
@myc = internal unnamed_addr constant i32 20

declare i32 @brute_force_mp_covered_(ptr noalias nocapture readonly, ptr noalias nocapture readonly)

define dso_local void @MAIN__() {
bb:
  %i = alloca i32, align 4
  store i32 1, ptr %i, align 4
  call void @bad1(ptr nonnull %i)
  ret void
}

define internal void @bad1(ptr noalias nocapture readonly %arg) {
bb:
  %i = alloca i32, align 4
  %i1 = alloca { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %i2 = alloca { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %i3 = alloca [9 x i32], align 16
  %i4 = alloca [9 x i32], align 16
  %i5 = alloca [9 x i32], align 16
  %i6 = alloca [9 x i32], align 16
  %i7 = alloca [9 x i32], align 16
  %i8 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i2, i64 0, i32 1
  %i9 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i2, i64 0, i32 3
  %i10 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i2, i64 0, i32 6, i64 0
  %i11 = getelementptr inbounds { i64, i64, i64 }, ptr %i10, i64 0, i32 0
  %i12 = getelementptr inbounds { i64, i64, i64 }, ptr %i10, i64 0, i32 2
  %i13 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i1, i64 0, i32 1
  %i14 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i1, i64 0, i32 3
  %i15 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i1, i64 0, i32 6, i64 0
  %i16 = getelementptr inbounds { i64, i64, i64 }, ptr %i15, i64 0, i32 1
  %i17 = load i32, ptr %arg, align 4
  %i18 = sext i32 %i17 to i64
  %i19 = getelementptr inbounds [9 x i32], ptr %i5, i64 0, i64 0
  br label %bb31

bb20:                                             ; preds = %bb31
  %i21 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i2, i64 0, i32 0
  %i22 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i2, i64 0, i32 2
  %i23 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i2, i64 0, i32 4
  %i24 = getelementptr inbounds { i64, i64, i64 }, ptr %i10, i64 0, i32 1
  %i25 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i1, i64 0, i32 0
  %i26 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i1, i64 0, i32 2
  %i27 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i1, i64 0, i32 4
  %i28 = getelementptr inbounds { i64, i64, i64 }, ptr %i15, i64 0, i32 0
  %i29 = getelementptr inbounds { i64, i64, i64 }, ptr %i15, i64 0, i32 2
  %i30 = getelementptr inbounds [9 x i32], ptr %i6, i64 0, i64 0
  br label %bb43

bb31:                                             ; preds = %bb31, %bb
  %i32 = phi i64 [ 1, %bb ], [ %i39, %bb31 ]
  %i33 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x i32]], ptr @brute_force_mp_sudoku1_, i64 0, i64 0, i64 0), i64 %i32)
  %i34 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i33, i64 %i18)
  %i35 = load i32, ptr %i34, align 4
  %i36 = icmp ne i32 %i35, 0
  %i37 = sext i1 %i36 to i32
  %i38 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i19, i64 %i32)
  store i32 %i37, ptr %i38, align 4
  %i39 = add nuw nsw i64 %i32, 1
  %i40 = icmp eq i64 %i39, 25
  br i1 %i40, label %bb20, label %bb31

bb41:                                             ; preds = %bb54
  %i42 = getelementptr inbounds [9 x i32], ptr %i7, i64 0, i64 0
  br label %bb57

bb43:                                             ; preds = %bb54, %bb20
  %i44 = phi i64 [ 1, %bb20 ], [ %i55, %bb54 ]
  %i45 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i19, i64 %i44)
  %i46 = load i32, ptr %i45, align 4
  %i47 = and i32 %i46, 1
  %i48 = icmp eq i32 %i47, 0
  br i1 %i48, label %bb54, label %bb49

bb49:                                             ; preds = %bb43
  %i50 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x i32]], ptr @brute_force_mp_sudoku1_, i64 0, i64 0, i64 0), i64 %i44)
  %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i50, i64 %i18)
  %i52 = load i32, ptr %i51, align 4
  %i53 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i30, i64 %i44)
  store i32 %i52, ptr %i53, align 4
  br label %bb54

bb54:                                             ; preds = %bb49, %bb43
  %i55 = add nuw nsw i64 %i44, 1
  %i56 = icmp eq i64 %i55, 10
  br i1 %i56, label %bb41, label %bb43

bb57:                                             ; preds = %bb67, %bb41
  %i58 = phi i64 [ 1, %bb41 ], [ %i68, %bb67 ]
  %i59 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i19, i64 %i58)
  %i60 = load i32, ptr %i59, align 4
  %i61 = and i32 %i60, 1
  %i62 = icmp eq i32 %i61, 0
  br i1 %i62, label %bb67, label %bb63

bb63:                                             ; preds = %bb57
  %i64 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i30, i64 %i58)
  %i65 = load i32, ptr %i64, align 4
  %i66 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i42, i64 %i58)
  store i32 %i65, ptr %i66, align 4
  br label %bb67

bb67:                                             ; preds = %bb63, %bb57
  %i68 = add nuw nsw i64 %i58, 1
  %i69 = icmp eq i64 %i68, 10
  br i1 %i69, label %bb70, label %bb57

bb70:                                             ; preds = %bb78, %bb67
  %i71 = phi i64 [ %i79, %bb78 ], [ 1, %bb67 ]
  %i72 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i19, i64 %i71)
  %i73 = load i32, ptr %i72, align 4
  %i74 = and i32 %i73, 1
  %i75 = icmp eq i32 %i74, 0
  br i1 %i75, label %bb76, label %bb78

bb76:                                             ; preds = %bb70
  %i77 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i30, i64 %i71)
  store i32 1, ptr %i77, align 4
  br label %bb78

bb78:                                             ; preds = %bb76, %bb70
  %i79 = add nuw nsw i64 %i71, 1
  %i80 = icmp eq i64 %i79, 10
  br i1 %i80, label %bb81, label %bb70

bb81:                                             ; preds = %bb89, %bb78
  %i82 = phi i64 [ %i90, %bb89 ], [ 1, %bb78 ]
  %i83 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i19, i64 %i82)
  %i84 = load i32, ptr %i83, align 4
  %i85 = and i32 %i84, 1
  %i86 = icmp eq i32 %i85, 0
  br i1 %i86, label %bb87, label %bb89

bb87:                                             ; preds = %bb81
  %i88 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i42, i64 %i82)
  store i32 9, ptr %i88, align 4
  br label %bb89

bb89:                                             ; preds = %bb87, %bb81
  %i90 = add nuw nsw i64 %i82, 1
  %i91 = icmp eq i64 %i90, 10
  br i1 %i91, label %bb92, label %bb81

bb92:                                             ; preds = %bb89
  %i93 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i30, i64 1)
  %i94 = load i32, ptr %i93, align 4
  %i95 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i42, i64 1)
  %i96 = load i32, ptr %i95, align 4
  %i97 = icmp slt i32 %i96, %i94
  br i1 %i97, label %bb1271, label %bb98

bb98:                                             ; preds = %bb92
  %i99 = add i32 %i17, 3
  %i100 = add nsw i32 %i17, -1
  %i101 = srem i32 %i100, 3
  %i102 = sub i32 %i99, %i101
  %i103 = sext i32 %i102 to i64
  %i104 = sub nsw i64 10, %i103
  %i105 = icmp sgt i32 %i102, 9
  %i106 = srem i32 %i17, 3
  %i107 = sext i32 %i106 to i64
  %i108 = add nsw i32 %i17, 1
  %i109 = sext i32 %i108 to i64
  %i110 = add nsw i32 %i17, 2
  %i111 = sext i32 %i110 to i64
  %i112 = sub nsw i64 1, %i109
  %i113 = add nsw i64 %i112, %i111
  %i114 = icmp slt i64 %i113, 1
  %i115 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x i32]], ptr @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 1)
  %i116 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i115, i64 %i18)
  %i117 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i30, i64 2)
  %i118 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i42, i64 2)
  %i119 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x i32]], ptr @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 2)
  %i120 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i30, i64 3)
  %i121 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i42, i64 3)
  %i122 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x i32]], ptr @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 3)
  %i123 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i30, i64 4)
  %i124 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i42, i64 4)
  %i125 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x i32]], ptr @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 4)
  %i126 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i30, i64 5)
  %i127 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i42, i64 5)
  %i128 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x i32]], ptr @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 5)
  %i129 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i30, i64 6)
  %i130 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i42, i64 6)
  %i131 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x i32]], ptr @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 6)
  %i132 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i30, i64 7)
  %i133 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i42, i64 7)
  %i134 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x i32]], ptr @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 7)
  %i135 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i30, i64 8)
  %i136 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i42, i64 8)
  %i137 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x i32]], ptr @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 8)
  %i138 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x i32]], ptr @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 9)
  %i139 = getelementptr inbounds [9 x i32], ptr %i4, i64 0, i64 0
  %i140 = getelementptr inbounds [9 x i32], ptr %i3, i64 0, i64 0
  %i141 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i32) nonnull %i24, i32 0)
  %i142 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i32) nonnull %i12, i32 0)
  %i143 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i32) nonnull %i11, i32 0)
  %i144 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i32) nonnull %i24, i32 1)
  %i145 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i32) nonnull %i12, i32 1)
  %i146 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i32) nonnull %i11, i32 1)
  %i147 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i32) nonnull %i16, i32 0)
  %i148 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i32) nonnull %i29, i32 0)
  %i149 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i32) nonnull %i28, i32 0)
  %i150 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i32) nonnull %i16, i32 1)
  %i151 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i32) nonnull %i29, i32 1)
  %i152 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i32) nonnull %i28, i32 1)
  %i153 = add nsw i64 %i111, 2
  %i154 = sub nsw i64 %i153, %i109
  %i155 = sext i32 %i94 to i64
  %i156 = sext i32 %i96 to i64
  %i157 = sext i32 %i17 to i64
  %i158 = srem i32 %i17, 3
  %i159 = sext i32 %i158 to i64
  %i160 = add nsw i32 %i17, 1
  %i161 = sext i32 %i160 to i64
  %i162 = add nsw i32 %i17, 2
  %i163 = sext i32 %i162 to i64
  %i164 = sub nsw i64 1, %i161
  %i165 = add nsw i64 %i164, %i163
  %i166 = icmp slt i64 %i165, 1
  %i167 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i119, i64 %i157)
  %i168 = add nsw i64 %i163, 2
  %i169 = sub nsw i64 %i168, %i161
  %i170 = sext i32 %i17 to i64
  %i171 = srem i32 %i17, 3
  %i172 = sext i32 %i171 to i64
  %i173 = add nsw i32 %i17, 1
  %i174 = sext i32 %i173 to i64
  %i175 = add nsw i32 %i17, 2
  %i176 = sext i32 %i175 to i64
  %i177 = sub nsw i64 1, %i174
  %i178 = add nsw i64 %i177, %i176
  %i179 = icmp slt i64 %i178, 1
  %i180 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i122, i64 %i170)
  %i181 = add nsw i64 %i176, 2
  %i182 = sub nsw i64 %i181, %i174
  %i183 = sext i32 %i17 to i64
  %i184 = srem i32 %i17, 3
  %i185 = sext i32 %i184 to i64
  %i186 = add nsw i32 %i17, 1
  %i187 = sext i32 %i186 to i64
  %i188 = add nsw i32 %i17, 2
  %i189 = sext i32 %i188 to i64
  %i190 = sub nsw i64 1, %i187
  %i191 = add nsw i64 %i190, %i189
  %i192 = icmp slt i64 %i191, 1
  %i193 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i125, i64 %i183)
  %i194 = add nsw i64 %i189, 2
  %i195 = sub nsw i64 %i194, %i187
  %i196 = sext i32 %i17 to i64
  %i197 = srem i32 %i17, 3
  %i198 = sext i32 %i197 to i64
  %i199 = add nsw i32 %i17, 1
  %i200 = sext i32 %i199 to i64
  %i201 = add nsw i32 %i17, 2
  %i202 = sext i32 %i201 to i64
  %i203 = sub nsw i64 1, %i200
  %i204 = add nsw i64 %i203, %i202
  %i205 = icmp slt i64 %i204, 1
  %i206 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i128, i64 %i196)
  %i207 = add nsw i64 %i202, 2
  %i208 = sub nsw i64 %i207, %i200
  %i209 = sext i32 %i17 to i64
  %i210 = srem i32 %i17, 3
  %i211 = sext i32 %i210 to i64
  %i212 = add nsw i32 %i17, 1
  %i213 = sext i32 %i212 to i64
  %i214 = add nsw i32 %i17, 2
  %i215 = sext i32 %i214 to i64
  %i216 = sub nsw i64 1, %i213
  %i217 = add nsw i64 %i216, %i215
  %i218 = icmp slt i64 %i217, 1
  %i219 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i131, i64 %i209)
  %i220 = add nsw i64 %i215, 2
  %i221 = sub nsw i64 %i220, %i213
  %i222 = sext i32 %i17 to i64
  %i223 = srem i32 %i17, 3
  %i224 = sext i32 %i223 to i64
  %i225 = add nsw i32 %i17, 1
  %i226 = sext i32 %i225 to i64
  %i227 = add nsw i32 %i17, 2
  %i228 = sext i32 %i227 to i64
  %i229 = sub nsw i64 1, %i226
  %i230 = add nsw i64 %i229, %i228
  %i231 = icmp slt i64 %i230, 1
  %i232 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i134, i64 %i222)
  %i233 = add nsw i64 %i228, 2
  %i234 = sub nsw i64 %i233, %i226
  %i235 = sext i32 %i17 to i64
  %i236 = srem i32 %i17, 3
  %i237 = sext i32 %i236 to i64
  %i238 = add nsw i32 %i17, 1
  %i239 = sext i32 %i238 to i64
  %i240 = add nsw i32 %i17, 2
  %i241 = sext i32 %i240 to i64
  %i242 = sub nsw i64 1, %i239
  %i243 = add nsw i64 %i242, %i241
  %i244 = icmp slt i64 %i243, 1
  %i245 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i137, i64 %i235)
  %i246 = add nsw i64 %i241, 2
  %i247 = sub nsw i64 %i246, %i239
  %i248 = sext i32 %i17 to i64
  %i249 = srem i32 %i17, 3
  %i250 = sext i32 %i249 to i64
  %i251 = add nsw i32 %i17, 1
  %i252 = sext i32 %i251 to i64
  %i253 = add nsw i32 %i17, 2
  %i254 = sext i32 %i253 to i64
  %i255 = sub nsw i64 1, %i252
  %i256 = add nsw i64 %i255, %i254
  %i257 = icmp slt i64 %i256, 1
  %i258 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i138, i64 %i248)
  %i259 = icmp eq i32 %i17, 5
  %i260 = icmp eq i32 %i17, 8
  %i261 = add nsw i64 %i254, 2
  %i262 = sub nsw i64 %i261, %i252
  br label %bb263

bb263:                                            ; preds = %bb1268, %bb98
  %i264 = phi i64 [ %i155, %bb98 ], [ %i1269, %bb1268 ]
  %i265 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x [9 x i32]]], ptr @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %i264)
  %i266 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i265, i64 1)
  %i267 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i266, i64 %i18)
  %i268 = load i32, ptr %i267, align 4
  %i269 = icmp slt i32 %i268, 1
  br i1 %i269, label %bb1268, label %bb270

bb270:                                            ; preds = %bb270, %bb263
  %i271 = phi i64 [ %i276, %bb270 ], [ 2, %bb263 ]
  %i272 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i265, i64 %i271)
  %i273 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i272, i64 %i18)
  %i274 = load i32, ptr %i273, align 4
  %i275 = add nsw i32 %i274, -10
  store i32 %i275, ptr %i273, align 4
  %i276 = add nuw nsw i64 %i271, 1
  %i277 = icmp eq i64 %i276, 10
  br i1 %i277, label %bb278, label %bb270

bb278:                                            ; preds = %bb270
  br i1 %i105, label %bb288, label %bb279

bb279:                                            ; preds = %bb279, %bb278
  %i280 = phi i64 [ %i286, %bb279 ], [ 1, %bb278 ]
  %i281 = phi i64 [ %i285, %bb279 ], [ %i103, %bb278 ]
  %i282 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i266, i64 %i281)
  %i283 = load i32, ptr %i282, align 4
  %i284 = add nsw i32 %i283, -10
  store i32 %i284, ptr %i282, align 4
  %i285 = add nsw i64 %i281, 1
  %i286 = add nuw nsw i64 %i280, 1
  %i287 = icmp slt i64 %i280, %i104
  br i1 %i287, label %bb279, label %bb288

bb288:                                            ; preds = %bb279, %bb278
  switch i64 %i107, label %bb313 [
    i64 1, label %bb289
    i64 2, label %bb305
  ]

bb289:                                            ; preds = %bb288
  br i1 %i114, label %bb313, label %bb290

bb290:                                            ; preds = %bb302, %bb289
  %i291 = phi i64 [ %i303, %bb302 ], [ 1, %bb289 ]
  %i292 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i265, i64 %i291)
  br label %bb293

bb293:                                            ; preds = %bb293, %bb290
  %i294 = phi i64 [ 1, %bb290 ], [ %i300, %bb293 ]
  %i295 = phi i64 [ %i109, %bb290 ], [ %i299, %bb293 ]
  %i296 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i292, i64 %i295)
  %i297 = load i32, ptr %i296, align 4
  %i298 = add nsw i32 %i297, -10
  store i32 %i298, ptr %i296, align 4
  %i299 = add nsw i64 %i295, 1
  %i300 = add nuw nsw i64 %i294, 1
  %i301 = icmp eq i64 %i300, %i154
  br i1 %i301, label %bb302, label %bb293

bb302:                                            ; preds = %bb293
  %i303 = add nuw nsw i64 %i291, 1
  %i304 = icmp eq i64 %i303, 4
  br i1 %i304, label %bb313, label %bb290

bb305:                                            ; preds = %bb305, %bb288
  %i306 = phi i64 [ %i311, %bb305 ], [ 1, %bb288 ]
  %i307 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i265, i64 %i306)
  %i308 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i307, i64 %i109)
  %i309 = load i32, ptr %i308, align 4
  %i310 = add nsw i32 %i309, -10
  store i32 %i310, ptr %i308, align 4
  %i311 = add nuw nsw i64 %i306, 1
  %i312 = icmp eq i64 %i311, 4
  br i1 %i312, label %bb313, label %bb305

bb313:                                            ; preds = %bb305, %bb302, %bb289, %bb288
  %i314 = trunc i64 %i264 to i32
  store i32 %i314, ptr %i116, align 4
  %i315 = load i32, ptr %i117, align 4
  %i316 = load i32, ptr %i118, align 4
  %i317 = icmp slt i32 %i316, %i315
  br i1 %i317, label %bb1224, label %bb318

bb318:                                            ; preds = %bb313
  %i319 = sub i32 45, %i314
  %i320 = sext i32 %i315 to i64
  %i321 = sext i32 %i316 to i64
  br label %bb322

bb322:                                            ; preds = %bb1221, %bb318
  %i323 = phi i64 [ %i320, %bb318 ], [ %i1222, %bb1221 ]
  %i324 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x [9 x i32]]], ptr @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %i323)
  %i325 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i324, i64 2)
  %i326 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i325, i64 %i157)
  %i327 = load i32, ptr %i326, align 4
  %i328 = icmp slt i32 %i327, 1
  br i1 %i328, label %bb1221, label %bb330

bb329:                                            ; preds = %bb330
  br i1 %i105, label %bb347, label %bb338

bb330:                                            ; preds = %bb330, %bb322
  %i331 = phi i64 [ %i336, %bb330 ], [ 3, %bb322 ]
  %i332 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i324, i64 %i331)
  %i333 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i332, i64 %i157)
  %i334 = load i32, ptr %i333, align 4
  %i335 = add nsw i32 %i334, -10
  store i32 %i335, ptr %i333, align 4
  %i336 = add nuw nsw i64 %i331, 1
  %i337 = icmp eq i64 %i336, 10
  br i1 %i337, label %bb329, label %bb330

bb338:                                            ; preds = %bb338, %bb329
  %i339 = phi i64 [ %i345, %bb338 ], [ 1, %bb329 ]
  %i340 = phi i64 [ %i344, %bb338 ], [ %i103, %bb329 ]
  %i341 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i325, i64 %i340)
  %i342 = load i32, ptr %i341, align 4
  %i343 = add nsw i32 %i342, -10
  store i32 %i343, ptr %i341, align 4
  %i344 = add nsw i64 %i340, 1
  %i345 = add nuw nsw i64 %i339, 1
  %i346 = icmp slt i64 %i339, %i104
  br i1 %i346, label %bb338, label %bb347

bb347:                                            ; preds = %bb338, %bb329
  switch i64 %i159, label %bb372 [
    i64 1, label %bb348
    i64 2, label %bb364
  ]

bb348:                                            ; preds = %bb347
  br i1 %i166, label %bb372, label %bb349

bb349:                                            ; preds = %bb361, %bb348
  %i350 = phi i64 [ %i362, %bb361 ], [ 1, %bb348 ]
  %i351 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i324, i64 %i350)
  br label %bb352

bb352:                                            ; preds = %bb352, %bb349
  %i353 = phi i64 [ 1, %bb349 ], [ %i359, %bb352 ]
  %i354 = phi i64 [ %i161, %bb349 ], [ %i358, %bb352 ]
  %i355 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i351, i64 %i354)
  %i356 = load i32, ptr %i355, align 4
  %i357 = add nsw i32 %i356, -10
  store i32 %i357, ptr %i355, align 4
  %i358 = add nsw i64 %i354, 1
  %i359 = add nuw nsw i64 %i353, 1
  %i360 = icmp eq i64 %i359, %i169
  br i1 %i360, label %bb361, label %bb352

bb361:                                            ; preds = %bb352
  %i362 = add nuw nsw i64 %i350, 1
  %i363 = icmp eq i64 %i362, 4
  br i1 %i363, label %bb372, label %bb349

bb364:                                            ; preds = %bb364, %bb347
  %i365 = phi i64 [ %i370, %bb364 ], [ 1, %bb347 ]
  %i366 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i324, i64 %i365)
  %i367 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i366, i64 %i161)
  %i368 = load i32, ptr %i367, align 4
  %i369 = add nsw i32 %i368, -10
  store i32 %i369, ptr %i367, align 4
  %i370 = add nuw nsw i64 %i365, 1
  %i371 = icmp eq i64 %i370, 4
  br i1 %i371, label %bb372, label %bb364

bb372:                                            ; preds = %bb364, %bb361, %bb348, %bb347
  %i373 = trunc i64 %i323 to i32
  store i32 %i373, ptr %i167, align 4
  %i374 = load i32, ptr %i120, align 4
  %i375 = load i32, ptr %i121, align 4
  %i376 = icmp slt i32 %i375, %i374
  br i1 %i376, label %bb1177, label %bb377

bb377:                                            ; preds = %bb372
  %i378 = sub i32 %i319, %i373
  %i379 = sext i32 %i374 to i64
  %i380 = sext i32 %i375 to i64
  br label %bb381

bb381:                                            ; preds = %bb1174, %bb377
  %i382 = phi i64 [ %i379, %bb377 ], [ %i1175, %bb1174 ]
  %i383 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x [9 x i32]]], ptr @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %i382)
  %i384 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i383, i64 3)
  %i385 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i384, i64 %i170)
  %i386 = load i32, ptr %i385, align 4
  %i387 = icmp slt i32 %i386, 1
  br i1 %i387, label %bb1174, label %bb389

bb388:                                            ; preds = %bb389
  br i1 %i105, label %bb406, label %bb397

bb389:                                            ; preds = %bb389, %bb381
  %i390 = phi i64 [ %i395, %bb389 ], [ 4, %bb381 ]
  %i391 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i383, i64 %i390)
  %i392 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i391, i64 %i170)
  %i393 = load i32, ptr %i392, align 4
  %i394 = add nsw i32 %i393, -10
  store i32 %i394, ptr %i392, align 4
  %i395 = add nuw nsw i64 %i390, 1
  %i396 = icmp eq i64 %i395, 10
  br i1 %i396, label %bb388, label %bb389

bb397:                                            ; preds = %bb397, %bb388
  %i398 = phi i64 [ %i404, %bb397 ], [ 1, %bb388 ]
  %i399 = phi i64 [ %i403, %bb397 ], [ %i103, %bb388 ]
  %i400 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i384, i64 %i399)
  %i401 = load i32, ptr %i400, align 4
  %i402 = add nsw i32 %i401, -10
  store i32 %i402, ptr %i400, align 4
  %i403 = add nsw i64 %i399, 1
  %i404 = add nuw nsw i64 %i398, 1
  %i405 = icmp slt i64 %i398, %i104
  br i1 %i405, label %bb397, label %bb406

bb406:                                            ; preds = %bb397, %bb388
  switch i64 %i172, label %bb431 [
    i64 1, label %bb407
    i64 2, label %bb423
  ]

bb407:                                            ; preds = %bb406
  br i1 %i179, label %bb431, label %bb408

bb408:                                            ; preds = %bb420, %bb407
  %i409 = phi i64 [ %i421, %bb420 ], [ 1, %bb407 ]
  %i410 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i383, i64 %i409)
  br label %bb411

bb411:                                            ; preds = %bb411, %bb408
  %i412 = phi i64 [ 1, %bb408 ], [ %i418, %bb411 ]
  %i413 = phi i64 [ %i174, %bb408 ], [ %i417, %bb411 ]
  %i414 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i410, i64 %i413)
  %i415 = load i32, ptr %i414, align 4
  %i416 = add nsw i32 %i415, -10
  store i32 %i416, ptr %i414, align 4
  %i417 = add nsw i64 %i413, 1
  %i418 = add nuw nsw i64 %i412, 1
  %i419 = icmp eq i64 %i418, %i182
  br i1 %i419, label %bb420, label %bb411

bb420:                                            ; preds = %bb411
  %i421 = add nuw nsw i64 %i409, 1
  %i422 = icmp eq i64 %i421, 4
  br i1 %i422, label %bb431, label %bb408

bb423:                                            ; preds = %bb423, %bb406
  %i424 = phi i64 [ %i429, %bb423 ], [ 1, %bb406 ]
  %i425 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i383, i64 %i424)
  %i426 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i425, i64 %i174)
  %i427 = load i32, ptr %i426, align 4
  %i428 = add nsw i32 %i427, -10
  store i32 %i428, ptr %i426, align 4
  %i429 = add nuw nsw i64 %i424, 1
  %i430 = icmp eq i64 %i429, 4
  br i1 %i430, label %bb431, label %bb423

bb431:                                            ; preds = %bb423, %bb420, %bb407, %bb406
  %i432 = trunc i64 %i382 to i32
  store i32 %i432, ptr %i180, align 4
  %i433 = load i32, ptr %i123, align 4
  %i434 = load i32, ptr %i124, align 4
  %i435 = icmp slt i32 %i434, %i433
  br i1 %i435, label %bb1130, label %bb436

bb436:                                            ; preds = %bb431
  %i437 = sub i32 %i378, %i432
  %i438 = sext i32 %i433 to i64
  %i439 = sext i32 %i434 to i64
  br label %bb440

bb440:                                            ; preds = %bb1127, %bb436
  %i441 = phi i64 [ %i438, %bb436 ], [ %i1128, %bb1127 ]
  %i442 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x [9 x i32]]], ptr @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %i441)
  %i443 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i442, i64 4)
  %i444 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i443, i64 %i183)
  %i445 = load i32, ptr %i444, align 4
  %i446 = icmp slt i32 %i445, 1
  br i1 %i446, label %bb1127, label %bb448

bb447:                                            ; preds = %bb448
  br i1 %i105, label %bb465, label %bb456

bb448:                                            ; preds = %bb448, %bb440
  %i449 = phi i64 [ %i454, %bb448 ], [ 5, %bb440 ]
  %i450 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i442, i64 %i449)
  %i451 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i450, i64 %i183)
  %i452 = load i32, ptr %i451, align 4
  %i453 = add nsw i32 %i452, -10
  store i32 %i453, ptr %i451, align 4
  %i454 = add nuw nsw i64 %i449, 1
  %i455 = icmp eq i64 %i454, 10
  br i1 %i455, label %bb447, label %bb448

bb456:                                            ; preds = %bb456, %bb447
  %i457 = phi i64 [ %i463, %bb456 ], [ 1, %bb447 ]
  %i458 = phi i64 [ %i462, %bb456 ], [ %i103, %bb447 ]
  %i459 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i443, i64 %i458)
  %i460 = load i32, ptr %i459, align 4
  %i461 = add nsw i32 %i460, -10
  store i32 %i461, ptr %i459, align 4
  %i462 = add nsw i64 %i458, 1
  %i463 = add nuw nsw i64 %i457, 1
  %i464 = icmp slt i64 %i457, %i104
  br i1 %i464, label %bb456, label %bb465

bb465:                                            ; preds = %bb456, %bb447
  switch i64 %i185, label %bb490 [
    i64 1, label %bb466
    i64 2, label %bb482
  ]

bb466:                                            ; preds = %bb465
  br i1 %i192, label %bb490, label %bb467

bb467:                                            ; preds = %bb479, %bb466
  %i468 = phi i64 [ %i480, %bb479 ], [ 4, %bb466 ]
  %i469 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i442, i64 %i468)
  br label %bb470

bb470:                                            ; preds = %bb470, %bb467
  %i471 = phi i64 [ 1, %bb467 ], [ %i477, %bb470 ]
  %i472 = phi i64 [ %i187, %bb467 ], [ %i476, %bb470 ]
  %i473 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i469, i64 %i472)
  %i474 = load i32, ptr %i473, align 4
  %i475 = add nsw i32 %i474, -10
  store i32 %i475, ptr %i473, align 4
  %i476 = add nsw i64 %i472, 1
  %i477 = add nuw nsw i64 %i471, 1
  %i478 = icmp eq i64 %i477, %i195
  br i1 %i478, label %bb479, label %bb470

bb479:                                            ; preds = %bb470
  %i480 = add nuw nsw i64 %i468, 1
  %i481 = icmp eq i64 %i480, 7
  br i1 %i481, label %bb490, label %bb467

bb482:                                            ; preds = %bb482, %bb465
  %i483 = phi i64 [ %i488, %bb482 ], [ 4, %bb465 ]
  %i484 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i442, i64 %i483)
  %i485 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i484, i64 %i187)
  %i486 = load i32, ptr %i485, align 4
  %i487 = add nsw i32 %i486, -10
  store i32 %i487, ptr %i485, align 4
  %i488 = add nuw nsw i64 %i483, 1
  %i489 = icmp eq i64 %i488, 7
  br i1 %i489, label %bb490, label %bb482

bb490:                                            ; preds = %bb482, %bb479, %bb466, %bb465
  %i491 = trunc i64 %i441 to i32
  store i32 %i491, ptr %i193, align 4
  %i492 = load i32, ptr %i126, align 4
  %i493 = load i32, ptr %i127, align 4
  %i494 = icmp slt i32 %i493, %i492
  br i1 %i494, label %bb1083, label %bb495

bb495:                                            ; preds = %bb490
  %i496 = sub i32 %i437, %i491
  %i497 = sext i32 %i492 to i64
  %i498 = sext i32 %i493 to i64
  br label %bb499

bb499:                                            ; preds = %bb1080, %bb495
  %i500 = phi i64 [ %i497, %bb495 ], [ %i1081, %bb1080 ]
  %i501 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x [9 x i32]]], ptr @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %i500)
  %i502 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i501, i64 5)
  %i503 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i502, i64 %i196)
  %i504 = load i32, ptr %i503, align 4
  %i505 = icmp slt i32 %i504, 1
  br i1 %i505, label %bb1080, label %bb507

bb506:                                            ; preds = %bb507
  br i1 %i105, label %bb524, label %bb515

bb507:                                            ; preds = %bb507, %bb499
  %i508 = phi i64 [ %i513, %bb507 ], [ 6, %bb499 ]
  %i509 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i501, i64 %i508)
  %i510 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i509, i64 %i196)
  %i511 = load i32, ptr %i510, align 4
  %i512 = add nsw i32 %i511, -10
  store i32 %i512, ptr %i510, align 4
  %i513 = add nuw nsw i64 %i508, 1
  %i514 = icmp eq i64 %i513, 10
  br i1 %i514, label %bb506, label %bb507

bb515:                                            ; preds = %bb515, %bb506
  %i516 = phi i64 [ %i522, %bb515 ], [ 1, %bb506 ]
  %i517 = phi i64 [ %i521, %bb515 ], [ %i103, %bb506 ]
  %i518 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i502, i64 %i517)
  %i519 = load i32, ptr %i518, align 4
  %i520 = add nsw i32 %i519, -10
  store i32 %i520, ptr %i518, align 4
  %i521 = add nsw i64 %i517, 1
  %i522 = add nuw nsw i64 %i516, 1
  %i523 = icmp slt i64 %i516, %i104
  br i1 %i523, label %bb515, label %bb524

bb524:                                            ; preds = %bb515, %bb506
  switch i64 %i198, label %bb549 [
    i64 1, label %bb525
    i64 2, label %bb541
  ]

bb525:                                            ; preds = %bb524
  br i1 %i205, label %bb549, label %bb526

bb526:                                            ; preds = %bb538, %bb525
  %i527 = phi i64 [ %i539, %bb538 ], [ 4, %bb525 ]
  %i528 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i501, i64 %i527)
  br label %bb529

bb529:                                            ; preds = %bb529, %bb526
  %i530 = phi i64 [ 1, %bb526 ], [ %i536, %bb529 ]
  %i531 = phi i64 [ %i200, %bb526 ], [ %i535, %bb529 ]
  %i532 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i528, i64 %i531)
  %i533 = load i32, ptr %i532, align 4
  %i534 = add nsw i32 %i533, -10
  store i32 %i534, ptr %i532, align 4
  %i535 = add nsw i64 %i531, 1
  %i536 = add nuw nsw i64 %i530, 1
  %i537 = icmp eq i64 %i536, %i208
  br i1 %i537, label %bb538, label %bb529

bb538:                                            ; preds = %bb529
  %i539 = add nuw nsw i64 %i527, 1
  %i540 = icmp eq i64 %i539, 7
  br i1 %i540, label %bb549, label %bb526

bb541:                                            ; preds = %bb541, %bb524
  %i542 = phi i64 [ %i547, %bb541 ], [ 4, %bb524 ]
  %i543 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i501, i64 %i542)
  %i544 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i543, i64 %i200)
  %i545 = load i32, ptr %i544, align 4
  %i546 = add nsw i32 %i545, -10
  store i32 %i546, ptr %i544, align 4
  %i547 = add nuw nsw i64 %i542, 1
  %i548 = icmp eq i64 %i547, 7
  br i1 %i548, label %bb549, label %bb541

bb549:                                            ; preds = %bb541, %bb538, %bb525, %bb524
  %i550 = trunc i64 %i500 to i32
  store i32 %i550, ptr %i206, align 4
  %i551 = load i32, ptr %i129, align 4
  %i552 = load i32, ptr %i130, align 4
  %i553 = icmp slt i32 %i552, %i551
  br i1 %i553, label %bb1036, label %bb554

bb554:                                            ; preds = %bb549
  %i555 = sub i32 %i496, %i550
  %i556 = sext i32 %i551 to i64
  %i557 = sext i32 %i552 to i64
  br label %bb558

bb558:                                            ; preds = %bb1033, %bb554
  %i559 = phi i64 [ %i556, %bb554 ], [ %i1034, %bb1033 ]
  %i560 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x [9 x i32]]], ptr @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %i559)
  %i561 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i560, i64 6)
  %i562 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i561, i64 %i209)
  %i563 = load i32, ptr %i562, align 4
  %i564 = icmp slt i32 %i563, 1
  br i1 %i564, label %bb1033, label %bb566

bb565:                                            ; preds = %bb566
  br i1 %i105, label %bb583, label %bb574

bb566:                                            ; preds = %bb566, %bb558
  %i567 = phi i64 [ %i572, %bb566 ], [ 7, %bb558 ]
  %i568 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i560, i64 %i567)
  %i569 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i568, i64 %i209)
  %i570 = load i32, ptr %i569, align 4
  %i571 = add nsw i32 %i570, -10
  store i32 %i571, ptr %i569, align 4
  %i572 = add nuw nsw i64 %i567, 1
  %i573 = icmp eq i64 %i572, 10
  br i1 %i573, label %bb565, label %bb566

bb574:                                            ; preds = %bb574, %bb565
  %i575 = phi i64 [ %i581, %bb574 ], [ 1, %bb565 ]
  %i576 = phi i64 [ %i580, %bb574 ], [ %i103, %bb565 ]
  %i577 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i561, i64 %i576)
  %i578 = load i32, ptr %i577, align 4
  %i579 = add nsw i32 %i578, -10
  store i32 %i579, ptr %i577, align 4
  %i580 = add nsw i64 %i576, 1
  %i581 = add nuw nsw i64 %i575, 1
  %i582 = icmp slt i64 %i575, %i104
  br i1 %i582, label %bb574, label %bb583

bb583:                                            ; preds = %bb574, %bb565
  switch i64 %i211, label %bb608 [
    i64 1, label %bb584
    i64 2, label %bb600
  ]

bb584:                                            ; preds = %bb583
  br i1 %i218, label %bb608, label %bb585

bb585:                                            ; preds = %bb597, %bb584
  %i586 = phi i64 [ %i598, %bb597 ], [ 4, %bb584 ]
  %i587 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i560, i64 %i586)
  br label %bb588

bb588:                                            ; preds = %bb588, %bb585
  %i589 = phi i64 [ 1, %bb585 ], [ %i595, %bb588 ]
  %i590 = phi i64 [ %i213, %bb585 ], [ %i594, %bb588 ]
  %i591 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i587, i64 %i590)
  %i592 = load i32, ptr %i591, align 4
  %i593 = add nsw i32 %i592, -10
  store i32 %i593, ptr %i591, align 4
  %i594 = add nsw i64 %i590, 1
  %i595 = add nuw nsw i64 %i589, 1
  %i596 = icmp eq i64 %i595, %i221
  br i1 %i596, label %bb597, label %bb588

bb597:                                            ; preds = %bb588
  %i598 = add nuw nsw i64 %i586, 1
  %i599 = icmp eq i64 %i598, 7
  br i1 %i599, label %bb608, label %bb585

bb600:                                            ; preds = %bb600, %bb583
  %i601 = phi i64 [ %i606, %bb600 ], [ 4, %bb583 ]
  %i602 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i560, i64 %i601)
  %i603 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i602, i64 %i213)
  %i604 = load i32, ptr %i603, align 4
  %i605 = add nsw i32 %i604, -10
  store i32 %i605, ptr %i603, align 4
  %i606 = add nuw nsw i64 %i601, 1
  %i607 = icmp eq i64 %i606, 7
  br i1 %i607, label %bb608, label %bb600

bb608:                                            ; preds = %bb600, %bb597, %bb584, %bb583
  %i609 = trunc i64 %i559 to i32
  store i32 %i609, ptr %i219, align 4
  %i610 = load i32, ptr %i132, align 4
  %i611 = load i32, ptr %i133, align 4
  %i612 = icmp slt i32 %i611, %i610
  br i1 %i612, label %bb989, label %bb613

bb613:                                            ; preds = %bb608
  %i614 = sub i32 %i555, %i609
  %i615 = sext i32 %i610 to i64
  %i616 = sext i32 %i611 to i64
  br label %bb617

bb617:                                            ; preds = %bb986, %bb613
  %i618 = phi i64 [ %i615, %bb613 ], [ %i987, %bb986 ]
  %i619 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x [9 x i32]]], ptr @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %i618)
  %i620 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i619, i64 7)
  %i621 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i620, i64 %i222)
  %i622 = load i32, ptr %i621, align 4
  %i623 = icmp slt i32 %i622, 1
  br i1 %i623, label %bb986, label %bb625

bb624:                                            ; preds = %bb625
  br i1 %i105, label %bb642, label %bb633

bb625:                                            ; preds = %bb625, %bb617
  %i626 = phi i64 [ %i631, %bb625 ], [ 8, %bb617 ]
  %i627 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i619, i64 %i626)
  %i628 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i627, i64 %i222)
  %i629 = load i32, ptr %i628, align 4
  %i630 = add nsw i32 %i629, -10
  store i32 %i630, ptr %i628, align 4
  %i631 = add nuw nsw i64 %i626, 1
  %i632 = icmp eq i64 %i631, 10
  br i1 %i632, label %bb624, label %bb625

bb633:                                            ; preds = %bb633, %bb624
  %i634 = phi i64 [ %i640, %bb633 ], [ 1, %bb624 ]
  %i635 = phi i64 [ %i639, %bb633 ], [ %i103, %bb624 ]
  %i636 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i620, i64 %i635)
  %i637 = load i32, ptr %i636, align 4
  %i638 = add nsw i32 %i637, -10
  store i32 %i638, ptr %i636, align 4
  %i639 = add nsw i64 %i635, 1
  %i640 = add nuw nsw i64 %i634, 1
  %i641 = icmp slt i64 %i634, %i104
  br i1 %i641, label %bb633, label %bb642

bb642:                                            ; preds = %bb633, %bb624
  switch i64 %i224, label %bb667 [
    i64 1, label %bb643
    i64 2, label %bb659
  ]

bb643:                                            ; preds = %bb642
  br i1 %i231, label %bb667, label %bb644

bb644:                                            ; preds = %bb656, %bb643
  %i645 = phi i64 [ %i657, %bb656 ], [ 7, %bb643 ]
  %i646 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i619, i64 %i645)
  br label %bb647

bb647:                                            ; preds = %bb647, %bb644
  %i648 = phi i64 [ 1, %bb644 ], [ %i654, %bb647 ]
  %i649 = phi i64 [ %i226, %bb644 ], [ %i653, %bb647 ]
  %i650 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i646, i64 %i649)
  %i651 = load i32, ptr %i650, align 4
  %i652 = add nsw i32 %i651, -10
  store i32 %i652, ptr %i650, align 4
  %i653 = add nsw i64 %i649, 1
  %i654 = add nuw nsw i64 %i648, 1
  %i655 = icmp eq i64 %i654, %i234
  br i1 %i655, label %bb656, label %bb647

bb656:                                            ; preds = %bb647
  %i657 = add nuw nsw i64 %i645, 1
  %i658 = icmp eq i64 %i657, 10
  br i1 %i658, label %bb667, label %bb644

bb659:                                            ; preds = %bb659, %bb642
  %i660 = phi i64 [ %i665, %bb659 ], [ 7, %bb642 ]
  %i661 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i619, i64 %i660)
  %i662 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i661, i64 %i226)
  %i663 = load i32, ptr %i662, align 4
  %i664 = add nsw i32 %i663, -10
  store i32 %i664, ptr %i662, align 4
  %i665 = add nuw nsw i64 %i660, 1
  %i666 = icmp eq i64 %i665, 10
  br i1 %i666, label %bb667, label %bb659

bb667:                                            ; preds = %bb659, %bb656, %bb643, %bb642
  %i668 = trunc i64 %i618 to i32
  store i32 %i668, ptr %i232, align 4
  %i669 = load i32, ptr %i135, align 4
  %i670 = load i32, ptr %i136, align 4
  %i671 = icmp slt i32 %i670, %i669
  br i1 %i671, label %bb942, label %bb672

bb672:                                            ; preds = %bb667
  %i673 = sub i32 %i614, %i668
  %i674 = sext i32 %i669 to i64
  %i675 = sext i32 %i670 to i64
  br label %bb676

bb676:                                            ; preds = %bb939, %bb672
  %i677 = phi i64 [ %i674, %bb672 ], [ %i940, %bb939 ]
  %i678 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x [9 x i32]]], ptr @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %i677)
  %i679 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i678, i64 8)
  %i680 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i679, i64 %i235)
  %i681 = load i32, ptr %i680, align 4
  %i682 = icmp slt i32 %i681, 1
  br i1 %i682, label %bb939, label %bb683

bb683:                                            ; preds = %bb676
  %i684 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i678, i64 9)
  %i685 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i684, i64 %i235)
  %i686 = load i32, ptr %i685, align 4
  %i687 = add nsw i32 %i686, -10
  store i32 %i687, ptr %i685, align 4
  br i1 %i105, label %bb697, label %bb688

bb688:                                            ; preds = %bb688, %bb683
  %i689 = phi i64 [ %i695, %bb688 ], [ 1, %bb683 ]
  %i690 = phi i64 [ %i694, %bb688 ], [ %i103, %bb683 ]
  %i691 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i679, i64 %i690)
  %i692 = load i32, ptr %i691, align 4
  %i693 = add nsw i32 %i692, -10
  store i32 %i693, ptr %i691, align 4
  %i694 = add nsw i64 %i690, 1
  %i695 = add nuw nsw i64 %i689, 1
  %i696 = icmp slt i64 %i689, %i104
  br i1 %i696, label %bb688, label %bb697

bb697:                                            ; preds = %bb688, %bb683
  switch i64 %i237, label %bb722 [
    i64 1, label %bb698
    i64 2, label %bb714
  ]

bb698:                                            ; preds = %bb697
  br i1 %i244, label %bb722, label %bb699

bb699:                                            ; preds = %bb711, %bb698
  %i700 = phi i64 [ %i712, %bb711 ], [ 7, %bb698 ]
  %i701 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i678, i64 %i700)
  br label %bb702

bb702:                                            ; preds = %bb702, %bb699
  %i703 = phi i64 [ 1, %bb699 ], [ %i709, %bb702 ]
  %i704 = phi i64 [ %i239, %bb699 ], [ %i708, %bb702 ]
  %i705 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i701, i64 %i704)
  %i706 = load i32, ptr %i705, align 4
  %i707 = add nsw i32 %i706, -10
  store i32 %i707, ptr %i705, align 4
  %i708 = add nsw i64 %i704, 1
  %i709 = add nuw nsw i64 %i703, 1
  %i710 = icmp eq i64 %i709, %i247
  br i1 %i710, label %bb711, label %bb702

bb711:                                            ; preds = %bb702
  %i712 = add nuw nsw i64 %i700, 1
  %i713 = icmp eq i64 %i712, 10
  br i1 %i713, label %bb722, label %bb699

bb714:                                            ; preds = %bb714, %bb697
  %i715 = phi i64 [ %i720, %bb714 ], [ 7, %bb697 ]
  %i716 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i678, i64 %i715)
  %i717 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i716, i64 %i239)
  %i718 = load i32, ptr %i717, align 4
  %i719 = add nsw i32 %i718, -10
  store i32 %i719, ptr %i717, align 4
  %i720 = add nuw nsw i64 %i715, 1
  %i721 = icmp eq i64 %i720, 10
  br i1 %i721, label %bb722, label %bb714

bb722:                                            ; preds = %bb714, %bb711, %bb698, %bb697
  %i723 = trunc i64 %i677 to i32
  store i32 %i723, ptr %i245, align 4
  %i724 = sub i32 %i673, %i723
  %i725 = sext i32 %i724 to i64
  %i726 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x [9 x i32]]], ptr @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %i725)
  %i727 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i726, i64 9)
  %i728 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i727, i64 %i248)
  %i729 = load i32, ptr %i728, align 4
  %i730 = icmp slt i32 %i729, 1
  br i1 %i730, label %bb902, label %bb731

bb731:                                            ; preds = %bb722
  br i1 %i105, label %bb741, label %bb732

bb732:                                            ; preds = %bb732, %bb731
  %i733 = phi i64 [ %i739, %bb732 ], [ 1, %bb731 ]
  %i734 = phi i64 [ %i738, %bb732 ], [ %i103, %bb731 ]
  %i735 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i727, i64 %i734)
  %i736 = load i32, ptr %i735, align 4
  %i737 = add nsw i32 %i736, -10
  store i32 %i737, ptr %i735, align 4
  %i738 = add nsw i64 %i734, 1
  %i739 = add nuw nsw i64 %i733, 1
  %i740 = icmp slt i64 %i733, %i104
  br i1 %i740, label %bb732, label %bb741

bb741:                                            ; preds = %bb732, %bb731
  switch i64 %i250, label %bb766 [
    i64 1, label %bb742
    i64 2, label %bb758
  ]

bb742:                                            ; preds = %bb741
  br i1 %i257, label %bb766, label %bb743

bb743:                                            ; preds = %bb755, %bb742
  %i744 = phi i64 [ %i756, %bb755 ], [ 7, %bb742 ]
  %i745 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i726, i64 %i744)
  br label %bb746

bb746:                                            ; preds = %bb746, %bb743
  %i747 = phi i64 [ 1, %bb743 ], [ %i753, %bb746 ]
  %i748 = phi i64 [ %i252, %bb743 ], [ %i752, %bb746 ]
  %i749 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i745, i64 %i748)
  %i750 = load i32, ptr %i749, align 4
  %i751 = add nsw i32 %i750, -10
  store i32 %i751, ptr %i749, align 4
  %i752 = add nsw i64 %i748, 1
  %i753 = add nuw nsw i64 %i747, 1
  %i754 = icmp eq i64 %i753, %i262
  br i1 %i754, label %bb755, label %bb746

bb755:                                            ; preds = %bb746
  %i756 = add nuw nsw i64 %i744, 1
  %i757 = icmp eq i64 %i756, 10
  br i1 %i757, label %bb766, label %bb743

bb758:                                            ; preds = %bb758, %bb741
  %i759 = phi i64 [ %i764, %bb758 ], [ 7, %bb741 ]
  %i760 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i726, i64 %i759)
  %i761 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i760, i64 %i252)
  %i762 = load i32, ptr %i761, align 4
  %i763 = add nsw i32 %i762, -10
  store i32 %i763, ptr %i761, align 4
  %i764 = add nuw nsw i64 %i759, 1
  %i765 = icmp eq i64 %i764, 10
  br i1 %i765, label %bb766, label %bb758

bb766:                                            ; preds = %bb758, %bb755, %bb742, %bb741
  store i32 %i724, ptr %i258, align 4
  br i1 %i259, label %bb767, label %bb790

bb767:                                            ; preds = %bb787, %bb766
  %i768 = phi i64 [ %i788, %bb787 ], [ 1, %bb766 ]
  %i769 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x i32]], ptr @brute_force_mp_sudoku1_, i64 0, i64 0, i64 0), i64 %i768)
  %i770 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i769, i64 %i252)
  %i771 = load i32, ptr %i770, align 4
  %i772 = icmp eq i32 %i771, 0
  br i1 %i772, label %bb773, label %bb787

bb773:                                            ; preds = %bb773, %bb767
  %i774 = phi i64 [ %i782, %bb773 ], [ 1, %bb767 ]
  %i775 = phi i32 [ %i781, %bb773 ], [ 0, %bb767 ]
  %i776 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x [9 x i32]]], ptr @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %i774)
  %i777 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i776, i64 %i768)
  %i778 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i777, i64 %i252)
  %i779 = load i32, ptr %i778, align 4
  %i780 = icmp sgt i32 %i779, 0
  %i781 = select i1 %i780, i32 -1, i32 %i775
  %i782 = add nuw nsw i64 %i774, 1
  %i783 = icmp eq i64 %i782, 10
  br i1 %i783, label %bb784, label %bb773

bb784:                                            ; preds = %bb773
  %i785 = and i32 %i781, 1
  %i786 = icmp eq i32 %i785, 0
  br i1 %i786, label %bb790, label %bb787

bb787:                                            ; preds = %bb784, %bb767
  %i788 = add nuw nsw i64 %i768, 1
  %i789 = icmp eq i64 %i788, 10
  br i1 %i789, label %bb790, label %bb767

bb790:                                            ; preds = %bb787, %bb784, %bb766
  %i791 = phi i32 [ 1, %bb766 ], [ 1, %bb787 ], [ 0, %bb784 ]
  br i1 %i260, label %bb792, label %bb862

bb792:                                            ; preds = %bb792, %bb790
  %i793 = phi i64 [ %i795, %bb792 ], [ 1, %bb790 ]
  %i794 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i139, i64 %i793)
  store i32 0, ptr %i794, align 4
  %i795 = add nuw nsw i64 %i793, 1
  %i796 = icmp eq i64 %i795, 10
  br i1 %i796, label %bb797, label %bb792

bb797:                                            ; preds = %bb810, %bb792
  %i798 = phi i64 [ %i811, %bb810 ], [ 1, %bb792 ]
  %i799 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x i32]], ptr @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 %i798)
  %i800 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i139, i64 %i798)
  %i801 = load i32, ptr %i800, align 4
  br label %bb802

bb802:                                            ; preds = %bb802, %bb797
  %i803 = phi i32 [ %i801, %bb797 ], [ %i807, %bb802 ]
  %i804 = phi i64 [ 1, %bb797 ], [ %i808, %bb802 ]
  %i805 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i799, i64 %i804)
  %i806 = load i32, ptr %i805, align 4
  %i807 = add nsw i32 %i803, %i806
  %i808 = add nuw nsw i64 %i804, 1
  %i809 = icmp eq i64 %i808, 9
  br i1 %i809, label %bb810, label %bb802

bb810:                                            ; preds = %bb802
  store i32 %i807, ptr %i800, align 4
  %i811 = add nuw nsw i64 %i798, 1
  %i812 = icmp eq i64 %i811, 10
  br i1 %i812, label %bb813, label %bb797

bb813:                                            ; preds = %bb813, %bb810
  %i814 = phi i64 [ %i819, %bb813 ], [ 1, %bb810 ]
  %i815 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i139, i64 %i814)
  %i816 = load i32, ptr %i815, align 4
  %i817 = sub nsw i32 45, %i816
  %i818 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i140, i64 %i814)
  store i32 %i817, ptr %i818, align 4
  %i819 = add nuw nsw i64 %i814, 1
  %i820 = icmp eq i64 %i819, 10
  br i1 %i820, label %bb821, label %bb813

bb821:                                            ; preds = %bb821, %bb813
  %i822 = phi i64 [ %i827, %bb821 ], [ 1, %bb813 ]
  %i823 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x i32]], ptr @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 %i822)
  %i824 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i823, i64 9)
  %i825 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i140, i64 %i822)
  %i826 = load i32, ptr %i825, align 4
  store i32 %i826, ptr %i824, align 4
  %i827 = add nuw nsw i64 %i822, 1
  %i828 = icmp eq i64 %i827, 10
  br i1 %i828, label %bb829, label %bb821

bb829:                                            ; preds = %bb829, %bb821
  %i830 = phi i64 [ %i836, %bb829 ], [ 1, %bb821 ]
  %i831 = phi i32 [ %i835, %bb829 ], [ 0, %bb821 ]
  %i832 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x i32]], ptr @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 %i830)
  %i833 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i832, i64 9)
  %i834 = load i32, ptr %i833, align 4
  %i835 = add nsw i32 %i834, %i831
  %i836 = add nuw nsw i64 %i830, 1
  %i837 = icmp eq i64 %i836, 10
  br i1 %i837, label %bb838, label %bb829

bb838:                                            ; preds = %bb829
  %i839 = icmp eq i32 %i835, 45
  br i1 %i839, label %bb840, label %bb902

bb840:                                            ; preds = %bb838
  %i841 = load i32, ptr @brute_force_mp_soln_, align 8
  %i842 = add nsw i32 %i841, 1
  store i32 %i842, ptr @brute_force_mp_soln_, align 8
  br label %bb843

bb843:                                            ; preds = %bb854, %bb840
  %i844 = phi i64 [ 1, %bb840 ], [ %i855, %bb854 ]
  %i845 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x i32]], ptr @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 %i844)
  %i846 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x i32]], ptr @brute_force_mp_sudoku3_, i64 0, i64 0, i64 0), i64 %i844)
  br label %bb847

bb847:                                            ; preds = %bb847, %bb843
  %i848 = phi i64 [ 1, %bb843 ], [ %i852, %bb847 ]
  %i849 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i845, i64 %i848)
  %i850 = load i32, ptr %i849, align 4
  %i851 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i846, i64 %i848)
  store i32 %i850, ptr %i851, align 4
  %i852 = add nuw nsw i64 %i848, 1
  %i853 = icmp eq i64 %i852, 10
  br i1 %i853, label %bb854, label %bb847

bb854:                                            ; preds = %bb847
  %i855 = add nuw nsw i64 %i844, 1
  %i856 = icmp eq i64 %i855, 10
  br i1 %i856, label %bb857, label %bb843

bb857:                                            ; preds = %bb854
  store i64 4, ptr %i8, align 8
  store i64 2, ptr %i23, align 8
  store i64 0, ptr %i22, align 8
  store i64 4, ptr %i141, align 8
  store i64 1, ptr %i142, align 8
  store i64 9, ptr %i143, align 8
  store i64 36, ptr %i144, align 8
  store i64 1, ptr %i145, align 8
  store i64 9, ptr %i146, align 8
  store ptr getelementptr inbounds ([9 x [9 x i32]], ptr @brute_force_mp_sudoku3_, i64 0, i64 0, i64 0), ptr %i21, align 8
  store i64 1, ptr %i9, align 8
  store i64 4, ptr %i13, align 8
  store i64 2, ptr %i27, align 8
  store i64 0, ptr %i26, align 8
  store i64 4, ptr %i147, align 8
  store i64 1, ptr %i148, align 8
  store i64 9, ptr %i149, align 8
  store i64 36, ptr %i150, align 8
  store i64 1, ptr %i151, align 8
  store i64 9, ptr %i152, align 8
  store ptr getelementptr inbounds ([9 x [9 x i32]], ptr @brute_force_mp_sudoku1_, i64 0, i64 0, i64 0), ptr %i25, align 8
  store i64 1, ptr %i14, align 8
  %i858 = call i32 @brute_force_mp_covered_(ptr nonnull %i2, ptr nonnull %i1)
  %i859 = and i32 %i858, 1
  %i860 = icmp eq i32 %i859, 0
  br i1 %i860, label %bb861, label %bb867

bb861:                                            ; preds = %bb857
  store i32 2, ptr @brute_force_mp_soln_, align 8
  br label %bb867

bb862:                                            ; preds = %bb790
  %i863 = icmp eq i32 %i791, 0
  br i1 %i863, label %bb867, label %bb864

bb864:                                            ; preds = %bb862
  store i32 %i251, ptr %i, align 4
  call void @bad1(ptr nonnull %i)
  %i865 = load i32, ptr @brute_force_mp_soln_, align 8
  %i866 = icmp sgt i32 %i865, 1
  br i1 %i866, label %bb1271, label %bb867

bb867:                                            ; preds = %bb864, %bb862, %bb861, %bb857
  br i1 %i105, label %bb877, label %bb868

bb868:                                            ; preds = %bb868, %bb867
  %i869 = phi i64 [ %i875, %bb868 ], [ 1, %bb867 ]
  %i870 = phi i64 [ %i874, %bb868 ], [ %i103, %bb867 ]
  %i871 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i727, i64 %i870)
  %i872 = load i32, ptr %i871, align 4
  %i873 = add nsw i32 %i872, 10
  store i32 %i873, ptr %i871, align 4
  %i874 = add nsw i64 %i870, 1
  %i875 = add nuw nsw i64 %i869, 1
  %i876 = icmp slt i64 %i869, %i104
  br i1 %i876, label %bb868, label %bb877

bb877:                                            ; preds = %bb868, %bb867
  switch i64 %i250, label %bb902 [
    i64 1, label %bb878
    i64 2, label %bb894
  ]

bb878:                                            ; preds = %bb877
  br i1 %i257, label %bb902, label %bb879

bb879:                                            ; preds = %bb891, %bb878
  %i880 = phi i64 [ %i892, %bb891 ], [ 7, %bb878 ]
  %i881 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i726, i64 %i880)
  br label %bb882

bb882:                                            ; preds = %bb882, %bb879
  %i883 = phi i64 [ 1, %bb879 ], [ %i889, %bb882 ]
  %i884 = phi i64 [ %i252, %bb879 ], [ %i888, %bb882 ]
  %i885 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i881, i64 %i884)
  %i886 = load i32, ptr %i885, align 4
  %i887 = add nsw i32 %i886, 10
  store i32 %i887, ptr %i885, align 4
  %i888 = add nsw i64 %i884, 1
  %i889 = add nuw nsw i64 %i883, 1
  %i890 = icmp eq i64 %i889, %i262
  br i1 %i890, label %bb891, label %bb882

bb891:                                            ; preds = %bb882
  %i892 = add nuw nsw i64 %i880, 1
  %i893 = icmp eq i64 %i892, 10
  br i1 %i893, label %bb902, label %bb879

bb894:                                            ; preds = %bb894, %bb877
  %i895 = phi i64 [ %i900, %bb894 ], [ 7, %bb877 ]
  %i896 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i726, i64 %i895)
  %i897 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i896, i64 %i252)
  %i898 = load i32, ptr %i897, align 4
  %i899 = add nsw i32 %i898, 10
  store i32 %i899, ptr %i897, align 4
  %i900 = add nuw nsw i64 %i895, 1
  %i901 = icmp eq i64 %i900, 10
  br i1 %i901, label %bb902, label %bb894

bb902:                                            ; preds = %bb894, %bb891, %bb878, %bb877, %bb838, %bb722
  %i903 = load i32, ptr %i685, align 4
  %i904 = add nsw i32 %i903, 10
  store i32 %i904, ptr %i685, align 4
  br i1 %i105, label %bb914, label %bb905

bb905:                                            ; preds = %bb905, %bb902
  %i906 = phi i64 [ %i912, %bb905 ], [ 1, %bb902 ]
  %i907 = phi i64 [ %i911, %bb905 ], [ %i103, %bb902 ]
  %i908 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i679, i64 %i907)
  %i909 = load i32, ptr %i908, align 4
  %i910 = add nsw i32 %i909, 10
  store i32 %i910, ptr %i908, align 4
  %i911 = add nsw i64 %i907, 1
  %i912 = add nuw nsw i64 %i906, 1
  %i913 = icmp slt i64 %i906, %i104
  br i1 %i913, label %bb905, label %bb914

bb914:                                            ; preds = %bb905, %bb902
  switch i64 %i237, label %bb939 [
    i64 1, label %bb915
    i64 2, label %bb931
  ]

bb915:                                            ; preds = %bb914
  br i1 %i244, label %bb939, label %bb916

bb916:                                            ; preds = %bb928, %bb915
  %i917 = phi i64 [ %i929, %bb928 ], [ 7, %bb915 ]
  %i918 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i678, i64 %i917)
  br label %bb919

bb919:                                            ; preds = %bb919, %bb916
  %i920 = phi i64 [ 1, %bb916 ], [ %i926, %bb919 ]
  %i921 = phi i64 [ %i239, %bb916 ], [ %i925, %bb919 ]
  %i922 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i918, i64 %i921)
  %i923 = load i32, ptr %i922, align 4
  %i924 = add nsw i32 %i923, 10
  store i32 %i924, ptr %i922, align 4
  %i925 = add nsw i64 %i921, 1
  %i926 = add nuw nsw i64 %i920, 1
  %i927 = icmp eq i64 %i926, %i247
  br i1 %i927, label %bb928, label %bb919

bb928:                                            ; preds = %bb919
  %i929 = add nuw nsw i64 %i917, 1
  %i930 = icmp eq i64 %i929, 10
  br i1 %i930, label %bb939, label %bb916

bb931:                                            ; preds = %bb931, %bb914
  %i932 = phi i64 [ %i937, %bb931 ], [ 7, %bb914 ]
  %i933 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i678, i64 %i932)
  %i934 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i933, i64 %i239)
  %i935 = load i32, ptr %i934, align 4
  %i936 = add nsw i32 %i935, 10
  store i32 %i936, ptr %i934, align 4
  %i937 = add nuw nsw i64 %i932, 1
  %i938 = icmp eq i64 %i937, 10
  br i1 %i938, label %bb939, label %bb931

bb939:                                            ; preds = %bb931, %bb928, %bb915, %bb914, %bb676
  %i940 = add nsw i64 %i677, 1
  %i941 = icmp slt i64 %i677, %i675
  br i1 %i941, label %bb676, label %bb942

bb942:                                            ; preds = %bb939, %bb667
  br label %bb944

bb943:                                            ; preds = %bb944
  br i1 %i105, label %bb961, label %bb952

bb944:                                            ; preds = %bb944, %bb942
  %i945 = phi i64 [ 8, %bb942 ], [ %i950, %bb944 ]
  %i946 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i619, i64 %i945)
  %i947 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i946, i64 %i222)
  %i948 = load i32, ptr %i947, align 4
  %i949 = add nsw i32 %i948, 10
  store i32 %i949, ptr %i947, align 4
  %i950 = add nuw nsw i64 %i945, 1
  %i951 = icmp eq i64 %i950, 10
  br i1 %i951, label %bb943, label %bb944

bb952:                                            ; preds = %bb952, %bb943
  %i953 = phi i64 [ %i959, %bb952 ], [ 1, %bb943 ]
  %i954 = phi i64 [ %i958, %bb952 ], [ %i103, %bb943 ]
  %i955 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i620, i64 %i954)
  %i956 = load i32, ptr %i955, align 4
  %i957 = add nsw i32 %i956, 10
  store i32 %i957, ptr %i955, align 4
  %i958 = add nsw i64 %i954, 1
  %i959 = add nuw nsw i64 %i953, 1
  %i960 = icmp slt i64 %i953, %i104
  br i1 %i960, label %bb952, label %bb961

bb961:                                            ; preds = %bb952, %bb943
  switch i64 %i224, label %bb986 [
    i64 1, label %bb962
    i64 2, label %bb978
  ]

bb962:                                            ; preds = %bb961
  br i1 %i231, label %bb986, label %bb963

bb963:                                            ; preds = %bb975, %bb962
  %i964 = phi i64 [ %i976, %bb975 ], [ 7, %bb962 ]
  %i965 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i619, i64 %i964)
  br label %bb966

bb966:                                            ; preds = %bb966, %bb963
  %i967 = phi i64 [ 1, %bb963 ], [ %i973, %bb966 ]
  %i968 = phi i64 [ %i226, %bb963 ], [ %i972, %bb966 ]
  %i969 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i965, i64 %i968)
  %i970 = load i32, ptr %i969, align 4
  %i971 = add nsw i32 %i970, 10
  store i32 %i971, ptr %i969, align 4
  %i972 = add nsw i64 %i968, 1
  %i973 = add nuw nsw i64 %i967, 1
  %i974 = icmp eq i64 %i973, %i234
  br i1 %i974, label %bb975, label %bb966

bb975:                                            ; preds = %bb966
  %i976 = add nuw nsw i64 %i964, 1
  %i977 = icmp eq i64 %i976, 10
  br i1 %i977, label %bb986, label %bb963

bb978:                                            ; preds = %bb978, %bb961
  %i979 = phi i64 [ %i984, %bb978 ], [ 7, %bb961 ]
  %i980 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i619, i64 %i979)
  %i981 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i980, i64 %i226)
  %i982 = load i32, ptr %i981, align 4
  %i983 = add nsw i32 %i982, 10
  store i32 %i983, ptr %i981, align 4
  %i984 = add nuw nsw i64 %i979, 1
  %i985 = icmp eq i64 %i984, 10
  br i1 %i985, label %bb986, label %bb978

bb986:                                            ; preds = %bb978, %bb975, %bb962, %bb961, %bb617
  %i987 = add nsw i64 %i618, 1
  %i988 = icmp slt i64 %i618, %i616
  br i1 %i988, label %bb617, label %bb989

bb989:                                            ; preds = %bb986, %bb608
  br label %bb991

bb990:                                            ; preds = %bb991
  br i1 %i105, label %bb1008, label %bb999

bb991:                                            ; preds = %bb991, %bb989
  %i992 = phi i64 [ 7, %bb989 ], [ %i997, %bb991 ]
  %i993 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i560, i64 %i992)
  %i994 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i993, i64 %i209)
  %i995 = load i32, ptr %i994, align 4
  %i996 = add nsw i32 %i995, 10
  store i32 %i996, ptr %i994, align 4
  %i997 = add nuw nsw i64 %i992, 1
  %i998 = icmp eq i64 %i997, 10
  br i1 %i998, label %bb990, label %bb991

bb999:                                            ; preds = %bb999, %bb990
  %i1000 = phi i64 [ %i1006, %bb999 ], [ 1, %bb990 ]
  %i1001 = phi i64 [ %i1005, %bb999 ], [ %i103, %bb990 ]
  %i1002 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i561, i64 %i1001)
  %i1003 = load i32, ptr %i1002, align 4
  %i1004 = add nsw i32 %i1003, 10
  store i32 %i1004, ptr %i1002, align 4
  %i1005 = add nsw i64 %i1001, 1
  %i1006 = add nuw nsw i64 %i1000, 1
  %i1007 = icmp slt i64 %i1000, %i104
  br i1 %i1007, label %bb999, label %bb1008

bb1008:                                           ; preds = %bb999, %bb990
  switch i64 %i211, label %bb1033 [
    i64 1, label %bb1009
    i64 2, label %bb1025
  ]

bb1009:                                           ; preds = %bb1008
  br i1 %i218, label %bb1033, label %bb1010

bb1010:                                           ; preds = %bb1022, %bb1009
  %i1011 = phi i64 [ %i1023, %bb1022 ], [ 4, %bb1009 ]
  %i1012 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i560, i64 %i1011)
  br label %bb1013

bb1013:                                           ; preds = %bb1013, %bb1010
  %i1014 = phi i64 [ 1, %bb1010 ], [ %i1020, %bb1013 ]
  %i1015 = phi i64 [ %i213, %bb1010 ], [ %i1019, %bb1013 ]
  %i1016 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i1012, i64 %i1015)
  %i1017 = load i32, ptr %i1016, align 4
  %i1018 = add nsw i32 %i1017, 10
  store i32 %i1018, ptr %i1016, align 4
  %i1019 = add nsw i64 %i1015, 1
  %i1020 = add nuw nsw i64 %i1014, 1
  %i1021 = icmp eq i64 %i1020, %i221
  br i1 %i1021, label %bb1022, label %bb1013

bb1022:                                           ; preds = %bb1013
  %i1023 = add nuw nsw i64 %i1011, 1
  %i1024 = icmp eq i64 %i1023, 7
  br i1 %i1024, label %bb1033, label %bb1010

bb1025:                                           ; preds = %bb1025, %bb1008
  %i1026 = phi i64 [ %i1031, %bb1025 ], [ 4, %bb1008 ]
  %i1027 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i560, i64 %i1026)
  %i1028 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i1027, i64 %i213)
  %i1029 = load i32, ptr %i1028, align 4
  %i1030 = add nsw i32 %i1029, 10
  store i32 %i1030, ptr %i1028, align 4
  %i1031 = add nuw nsw i64 %i1026, 1
  %i1032 = icmp eq i64 %i1031, 7
  br i1 %i1032, label %bb1033, label %bb1025

bb1033:                                           ; preds = %bb1025, %bb1022, %bb1009, %bb1008, %bb558
  %i1034 = add nsw i64 %i559, 1
  %i1035 = icmp slt i64 %i559, %i557
  br i1 %i1035, label %bb558, label %bb1036

bb1036:                                           ; preds = %bb1033, %bb549
  br label %bb1038

bb1037:                                           ; preds = %bb1038
  br i1 %i105, label %bb1055, label %bb1046

bb1038:                                           ; preds = %bb1038, %bb1036
  %i1039 = phi i64 [ 6, %bb1036 ], [ %i1044, %bb1038 ]
  %i1040 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i501, i64 %i1039)
  %i1041 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i1040, i64 %i196)
  %i1042 = load i32, ptr %i1041, align 4
  %i1043 = add nsw i32 %i1042, 10
  store i32 %i1043, ptr %i1041, align 4
  %i1044 = add nuw nsw i64 %i1039, 1
  %i1045 = icmp eq i64 %i1044, 10
  br i1 %i1045, label %bb1037, label %bb1038

bb1046:                                           ; preds = %bb1046, %bb1037
  %i1047 = phi i64 [ %i1053, %bb1046 ], [ 1, %bb1037 ]
  %i1048 = phi i64 [ %i1052, %bb1046 ], [ %i103, %bb1037 ]
  %i1049 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i502, i64 %i1048)
  %i1050 = load i32, ptr %i1049, align 4
  %i1051 = add nsw i32 %i1050, 10
  store i32 %i1051, ptr %i1049, align 4
  %i1052 = add nsw i64 %i1048, 1
  %i1053 = add nuw nsw i64 %i1047, 1
  %i1054 = icmp slt i64 %i1047, %i104
  br i1 %i1054, label %bb1046, label %bb1055

bb1055:                                           ; preds = %bb1046, %bb1037
  switch i64 %i198, label %bb1080 [
    i64 1, label %bb1056
    i64 2, label %bb1072
  ]

bb1056:                                           ; preds = %bb1055
  br i1 %i205, label %bb1080, label %bb1057

bb1057:                                           ; preds = %bb1069, %bb1056
  %i1058 = phi i64 [ %i1070, %bb1069 ], [ 4, %bb1056 ]
  %i1059 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i501, i64 %i1058)
  br label %bb1060

bb1060:                                           ; preds = %bb1060, %bb1057
  %i1061 = phi i64 [ 1, %bb1057 ], [ %i1067, %bb1060 ]
  %i1062 = phi i64 [ %i200, %bb1057 ], [ %i1066, %bb1060 ]
  %i1063 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i1059, i64 %i1062)
  %i1064 = load i32, ptr %i1063, align 4
  %i1065 = add nsw i32 %i1064, 10
  store i32 %i1065, ptr %i1063, align 4
  %i1066 = add nsw i64 %i1062, 1
  %i1067 = add nuw nsw i64 %i1061, 1
  %i1068 = icmp eq i64 %i1067, %i208
  br i1 %i1068, label %bb1069, label %bb1060

bb1069:                                           ; preds = %bb1060
  %i1070 = add nuw nsw i64 %i1058, 1
  %i1071 = icmp eq i64 %i1070, 7
  br i1 %i1071, label %bb1080, label %bb1057

bb1072:                                           ; preds = %bb1072, %bb1055
  %i1073 = phi i64 [ %i1078, %bb1072 ], [ 4, %bb1055 ]
  %i1074 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i501, i64 %i1073)
  %i1075 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i1074, i64 %i200)
  %i1076 = load i32, ptr %i1075, align 4
  %i1077 = add nsw i32 %i1076, 10
  store i32 %i1077, ptr %i1075, align 4
  %i1078 = add nuw nsw i64 %i1073, 1
  %i1079 = icmp eq i64 %i1078, 7
  br i1 %i1079, label %bb1080, label %bb1072

bb1080:                                           ; preds = %bb1072, %bb1069, %bb1056, %bb1055, %bb499
  %i1081 = add nsw i64 %i500, 1
  %i1082 = icmp slt i64 %i500, %i498
  br i1 %i1082, label %bb499, label %bb1083

bb1083:                                           ; preds = %bb1080, %bb490
  br label %bb1085

bb1084:                                           ; preds = %bb1085
  br i1 %i105, label %bb1102, label %bb1093

bb1085:                                           ; preds = %bb1085, %bb1083
  %i1086 = phi i64 [ 5, %bb1083 ], [ %i1091, %bb1085 ]
  %i1087 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i442, i64 %i1086)
  %i1088 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i1087, i64 %i183)
  %i1089 = load i32, ptr %i1088, align 4
  %i1090 = add nsw i32 %i1089, 10
  store i32 %i1090, ptr %i1088, align 4
  %i1091 = add nuw nsw i64 %i1086, 1
  %i1092 = icmp eq i64 %i1091, 10
  br i1 %i1092, label %bb1084, label %bb1085

bb1093:                                           ; preds = %bb1093, %bb1084
  %i1094 = phi i64 [ %i1100, %bb1093 ], [ 1, %bb1084 ]
  %i1095 = phi i64 [ %i1099, %bb1093 ], [ %i103, %bb1084 ]
  %i1096 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i443, i64 %i1095)
  %i1097 = load i32, ptr %i1096, align 4
  %i1098 = add nsw i32 %i1097, 10
  store i32 %i1098, ptr %i1096, align 4
  %i1099 = add nsw i64 %i1095, 1
  %i1100 = add nuw nsw i64 %i1094, 1
  %i1101 = icmp slt i64 %i1094, %i104
  br i1 %i1101, label %bb1093, label %bb1102

bb1102:                                           ; preds = %bb1093, %bb1084
  switch i64 %i185, label %bb1127 [
    i64 1, label %bb1103
    i64 2, label %bb1119
  ]

bb1103:                                           ; preds = %bb1102
  br i1 %i192, label %bb1127, label %bb1104

bb1104:                                           ; preds = %bb1116, %bb1103
  %i1105 = phi i64 [ %i1117, %bb1116 ], [ 4, %bb1103 ]
  %i1106 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i442, i64 %i1105)
  br label %bb1107

bb1107:                                           ; preds = %bb1107, %bb1104
  %i1108 = phi i64 [ 1, %bb1104 ], [ %i1114, %bb1107 ]
  %i1109 = phi i64 [ %i187, %bb1104 ], [ %i1113, %bb1107 ]
  %i1110 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i1106, i64 %i1109)
  %i1111 = load i32, ptr %i1110, align 4
  %i1112 = add nsw i32 %i1111, 10
  store i32 %i1112, ptr %i1110, align 4
  %i1113 = add nsw i64 %i1109, 1
  %i1114 = add nuw nsw i64 %i1108, 1
  %i1115 = icmp eq i64 %i1114, %i195
  br i1 %i1115, label %bb1116, label %bb1107

bb1116:                                           ; preds = %bb1107
  %i1117 = add nuw nsw i64 %i1105, 1
  %i1118 = icmp eq i64 %i1117, 7
  br i1 %i1118, label %bb1127, label %bb1104

bb1119:                                           ; preds = %bb1119, %bb1102
  %i1120 = phi i64 [ %i1125, %bb1119 ], [ 4, %bb1102 ]
  %i1121 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i442, i64 %i1120)
  %i1122 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i1121, i64 %i187)
  %i1123 = load i32, ptr %i1122, align 4
  %i1124 = add nsw i32 %i1123, 10
  store i32 %i1124, ptr %i1122, align 4
  %i1125 = add nuw nsw i64 %i1120, 1
  %i1126 = icmp eq i64 %i1125, 7
  br i1 %i1126, label %bb1127, label %bb1119

bb1127:                                           ; preds = %bb1119, %bb1116, %bb1103, %bb1102, %bb440
  %i1128 = add nsw i64 %i441, 1
  %i1129 = icmp slt i64 %i441, %i439
  br i1 %i1129, label %bb440, label %bb1130

bb1130:                                           ; preds = %bb1127, %bb431
  br label %bb1132

bb1131:                                           ; preds = %bb1132
  br i1 %i105, label %bb1149, label %bb1140

bb1132:                                           ; preds = %bb1132, %bb1130
  %i1133 = phi i64 [ 4, %bb1130 ], [ %i1138, %bb1132 ]
  %i1134 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i383, i64 %i1133)
  %i1135 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i1134, i64 %i170)
  %i1136 = load i32, ptr %i1135, align 4
  %i1137 = add nsw i32 %i1136, 10
  store i32 %i1137, ptr %i1135, align 4
  %i1138 = add nuw nsw i64 %i1133, 1
  %i1139 = icmp eq i64 %i1138, 10
  br i1 %i1139, label %bb1131, label %bb1132

bb1140:                                           ; preds = %bb1140, %bb1131
  %i1141 = phi i64 [ %i1147, %bb1140 ], [ 1, %bb1131 ]
  %i1142 = phi i64 [ %i1146, %bb1140 ], [ %i103, %bb1131 ]
  %i1143 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i384, i64 %i1142)
  %i1144 = load i32, ptr %i1143, align 4
  %i1145 = add nsw i32 %i1144, 10
  store i32 %i1145, ptr %i1143, align 4
  %i1146 = add nsw i64 %i1142, 1
  %i1147 = add nuw nsw i64 %i1141, 1
  %i1148 = icmp slt i64 %i1141, %i104
  br i1 %i1148, label %bb1140, label %bb1149

bb1149:                                           ; preds = %bb1140, %bb1131
  switch i64 %i172, label %bb1174 [
    i64 1, label %bb1150
    i64 2, label %bb1166
  ]

bb1150:                                           ; preds = %bb1149
  br i1 %i179, label %bb1174, label %bb1151

bb1151:                                           ; preds = %bb1163, %bb1150
  %i1152 = phi i64 [ %i1164, %bb1163 ], [ 1, %bb1150 ]
  %i1153 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i383, i64 %i1152)
  br label %bb1154

bb1154:                                           ; preds = %bb1154, %bb1151
  %i1155 = phi i64 [ 1, %bb1151 ], [ %i1161, %bb1154 ]
  %i1156 = phi i64 [ %i174, %bb1151 ], [ %i1160, %bb1154 ]
  %i1157 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i1153, i64 %i1156)
  %i1158 = load i32, ptr %i1157, align 4
  %i1159 = add nsw i32 %i1158, 10
  store i32 %i1159, ptr %i1157, align 4
  %i1160 = add nsw i64 %i1156, 1
  %i1161 = add nuw nsw i64 %i1155, 1
  %i1162 = icmp eq i64 %i1161, %i182
  br i1 %i1162, label %bb1163, label %bb1154

bb1163:                                           ; preds = %bb1154
  %i1164 = add nuw nsw i64 %i1152, 1
  %i1165 = icmp eq i64 %i1164, 4
  br i1 %i1165, label %bb1174, label %bb1151

bb1166:                                           ; preds = %bb1166, %bb1149
  %i1167 = phi i64 [ %i1172, %bb1166 ], [ 1, %bb1149 ]
  %i1168 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i383, i64 %i1167)
  %i1169 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i1168, i64 %i174)
  %i1170 = load i32, ptr %i1169, align 4
  %i1171 = add nsw i32 %i1170, 10
  store i32 %i1171, ptr %i1169, align 4
  %i1172 = add nuw nsw i64 %i1167, 1
  %i1173 = icmp eq i64 %i1172, 4
  br i1 %i1173, label %bb1174, label %bb1166

bb1174:                                           ; preds = %bb1166, %bb1163, %bb1150, %bb1149, %bb381
  %i1175 = add nsw i64 %i382, 1
  %i1176 = icmp slt i64 %i382, %i380
  br i1 %i1176, label %bb381, label %bb1177

bb1177:                                           ; preds = %bb1174, %bb372
  br label %bb1179

bb1178:                                           ; preds = %bb1179
  br i1 %i105, label %bb1196, label %bb1187

bb1179:                                           ; preds = %bb1179, %bb1177
  %i1180 = phi i64 [ 3, %bb1177 ], [ %i1185, %bb1179 ]
  %i1181 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i324, i64 %i1180)
  %i1182 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i1181, i64 %i157)
  %i1183 = load i32, ptr %i1182, align 4
  %i1184 = add nsw i32 %i1183, 10
  store i32 %i1184, ptr %i1182, align 4
  %i1185 = add nuw nsw i64 %i1180, 1
  %i1186 = icmp eq i64 %i1185, 10
  br i1 %i1186, label %bb1178, label %bb1179

bb1187:                                           ; preds = %bb1187, %bb1178
  %i1188 = phi i64 [ %i1194, %bb1187 ], [ 1, %bb1178 ]
  %i1189 = phi i64 [ %i1193, %bb1187 ], [ %i103, %bb1178 ]
  %i1190 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i325, i64 %i1189)
  %i1191 = load i32, ptr %i1190, align 4
  %i1192 = add nsw i32 %i1191, 10
  store i32 %i1192, ptr %i1190, align 4
  %i1193 = add nsw i64 %i1189, 1
  %i1194 = add nuw nsw i64 %i1188, 1
  %i1195 = icmp slt i64 %i1188, %i104
  br i1 %i1195, label %bb1187, label %bb1196

bb1196:                                           ; preds = %bb1187, %bb1178
  switch i64 %i159, label %bb1221 [
    i64 1, label %bb1197
    i64 2, label %bb1213
  ]

bb1197:                                           ; preds = %bb1196
  br i1 %i166, label %bb1221, label %bb1198

bb1198:                                           ; preds = %bb1210, %bb1197
  %i1199 = phi i64 [ %i1211, %bb1210 ], [ 1, %bb1197 ]
  %i1200 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i324, i64 %i1199)
  br label %bb1201

bb1201:                                           ; preds = %bb1201, %bb1198
  %i1202 = phi i64 [ %i161, %bb1198 ], [ %i1207, %bb1201 ]
  %i1203 = phi i64 [ 1, %bb1198 ], [ %i1208, %bb1201 ]
  %i1204 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i1200, i64 %i1202)
  %i1205 = load i32, ptr %i1204, align 4
  %i1206 = add nsw i32 %i1205, 10
  store i32 %i1206, ptr %i1204, align 4
  %i1207 = add nsw i64 %i1202, 1
  %i1208 = add nuw nsw i64 %i1203, 1
  %i1209 = icmp eq i64 %i1208, %i169
  br i1 %i1209, label %bb1210, label %bb1201

bb1210:                                           ; preds = %bb1201
  %i1211 = add nuw nsw i64 %i1199, 1
  %i1212 = icmp eq i64 %i1211, 4
  br i1 %i1212, label %bb1221, label %bb1198

bb1213:                                           ; preds = %bb1213, %bb1196
  %i1214 = phi i64 [ %i1219, %bb1213 ], [ 1, %bb1196 ]
  %i1215 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i324, i64 %i1214)
  %i1216 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i1215, i64 %i161)
  %i1217 = load i32, ptr %i1216, align 4
  %i1218 = add nsw i32 %i1217, 10
  store i32 %i1218, ptr %i1216, align 4
  %i1219 = add nuw nsw i64 %i1214, 1
  %i1220 = icmp eq i64 %i1219, 4
  br i1 %i1220, label %bb1221, label %bb1213

bb1221:                                           ; preds = %bb1213, %bb1210, %bb1197, %bb1196, %bb322
  %i1222 = add nsw i64 %i323, 1
  %i1223 = icmp slt i64 %i323, %i321
  br i1 %i1223, label %bb322, label %bb1224

bb1224:                                           ; preds = %bb1221, %bb313
  br label %bb1226

bb1225:                                           ; preds = %bb1226
  br i1 %i105, label %bb1243, label %bb1234

bb1226:                                           ; preds = %bb1226, %bb1224
  %i1227 = phi i64 [ 2, %bb1224 ], [ %i1232, %bb1226 ]
  %i1228 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i265, i64 %i1227)
  %i1229 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i1228, i64 %i18)
  %i1230 = load i32, ptr %i1229, align 4
  %i1231 = add nsw i32 %i1230, 10
  store i32 %i1231, ptr %i1229, align 4
  %i1232 = add nuw nsw i64 %i1227, 1
  %i1233 = icmp eq i64 %i1232, 10
  br i1 %i1233, label %bb1225, label %bb1226

bb1234:                                           ; preds = %bb1234, %bb1225
  %i1235 = phi i64 [ %i1240, %bb1234 ], [ %i103, %bb1225 ]
  %i1236 = phi i64 [ %i1241, %bb1234 ], [ 1, %bb1225 ]
  %i1237 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) nonnull %i266, i64 %i1235)
  %i1238 = load i32, ptr %i1237, align 4
  %i1239 = add nsw i32 %i1238, 10
  store i32 %i1239, ptr %i1237, align 4
  %i1240 = add nsw i64 %i1235, 1
  %i1241 = add nuw nsw i64 %i1236, 1
  %i1242 = icmp slt i64 %i1236, %i104
  br i1 %i1242, label %bb1234, label %bb1243

bb1243:                                           ; preds = %bb1234, %bb1225
  switch i64 %i107, label %bb1268 [
    i64 1, label %bb1244
    i64 2, label %bb1260
  ]

bb1244:                                           ; preds = %bb1243
  br i1 %i114, label %bb1268, label %bb1245

bb1245:                                           ; preds = %bb1257, %bb1244
  %i1246 = phi i64 [ %i1258, %bb1257 ], [ 1, %bb1244 ]
  %i1247 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i265, i64 %i1246)
  br label %bb1248

bb1248:                                           ; preds = %bb1248, %bb1245
  %i1249 = phi i64 [ %i109, %bb1245 ], [ %i1254, %bb1248 ]
  %i1250 = phi i64 [ 1, %bb1245 ], [ %i1255, %bb1248 ]
  %i1251 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i1247, i64 %i1249)
  %i1252 = load i32, ptr %i1251, align 4
  %i1253 = add nsw i32 %i1252, 10
  store i32 %i1253, ptr %i1251, align 4
  %i1254 = add nsw i64 %i1249, 1
  %i1255 = add nuw nsw i64 %i1250, 1
  %i1256 = icmp eq i64 %i1255, %i154
  br i1 %i1256, label %bb1257, label %bb1248

bb1257:                                           ; preds = %bb1248
  %i1258 = add nuw nsw i64 %i1246, 1
  %i1259 = icmp eq i64 %i1258, 4
  br i1 %i1259, label %bb1268, label %bb1245

bb1260:                                           ; preds = %bb1260, %bb1243
  %i1261 = phi i64 [ %i1266, %bb1260 ], [ 1, %bb1243 ]
  %i1262 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) nonnull %i265, i64 %i1261)
  %i1263 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i1262, i64 %i109)
  %i1264 = load i32, ptr %i1263, align 4
  %i1265 = add nsw i32 %i1264, 10
  store i32 %i1265, ptr %i1263, align 4
  %i1266 = add nuw nsw i64 %i1261, 1
  %i1267 = icmp eq i64 %i1266, 4
  br i1 %i1267, label %bb1268, label %bb1260

bb1268:                                           ; preds = %bb1260, %bb1257, %bb1244, %bb1243, %bb263
  %i1269 = add nsw i64 %i264, 1
  %i1270 = icmp slt i64 %i264, %i156
  br i1 %i1270, label %bb263, label %bb1271

bb1271:                                           ; preds = %bb1268, %bb864, %bb92
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #0

attributes #0 = { nounwind readnone speculatable }
; end INTEL_FEATURE_SW_ADVANCED

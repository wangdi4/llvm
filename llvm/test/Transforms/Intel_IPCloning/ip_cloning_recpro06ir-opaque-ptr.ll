; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers < %s -passes='module(ip-cloning)' -ip-gen-cloning-force-enable-dtrans -S 2>&1 | FileCheck %s

; Test that the function main_IP_digits_2_ is recognized as a recursive progression clone
; and eight clones of it are created. Also test that the recursive progression
; is not cyclic.
; This is the same test as ip_cloning_recpro06-opaque-ptr.ll, but checks for
; IR without requiring asserts.

; CHECK: define dso_local void @MAIN__
; CHECK: define internal void @main_IP_digits_2_
; CHECK: call void @main_IP_digits_2_
; CHECK: define internal void @main_IP_digits_2_.1
; CHECK: call void @main_IP_digits_2_.2
; CHECK: define internal void @main_IP_digits_2_.2
; CHECK: call void @main_IP_digits_2_.3
; CHECK: define internal void @main_IP_digits_2_.3
; CHECK: call void @main_IP_digits_2_.4
; CHECK: define internal void @main_IP_digits_2_.4
; CHECK: call void @main_IP_digits_2_.5
; CHECK: define internal void @main_IP_digits_2_.5
; CHECK: call void @main_IP_digits_2_.6
; CHECK: define internal void @main_IP_digits_2_.6
; CHECK: call void @main_IP_digits_2_.7
; CHECK: define internal void @main_IP_digits_2_.7
; CHECK: call void @main_IP_digits_2_.8
; CHECK: define internal void @main_IP_digits_2_.8
; CHECK-NOT: call void @main_IP_digits_2_

@brute_force_mp_block_ = internal global [9 x [9 x [9 x i32]]] zeroinitializer, align 8
@brute_force_mp_count_ = internal global i32 0, align 8
@anon.2e0c92f0a1367b9abe0b93b08f29c950.0 = internal unnamed_addr constant [9 x i8] c" block = "

define dso_local void @MAIN__() {
bb:
  %i = alloca { i32 }, align 8
  %i1 = alloca [4 x i8], align 1
  %i2 = alloca { i64, ptr }, align 8
  %i3 = alloca [4 x i8], align 1
  %i4 = alloca i32, align 4
  %i5 = alloca i32, align 4
  %i6 = alloca [8 x i64], align 16
  store i32 2, ptr %i5, align 4
  %i7 = call i32 @for_set_reentrancy(ptr nonnull %i5)
  br label %bb8

bb8:                                              ; preds = %bb22, %bb
  %i9 = phi i64 [ 1, %bb ], [ %i23, %bb22 ]
  %i10 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x [9 x i32]]], ptr @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %i9)
  br label %bb11

bb11:                                             ; preds = %bb19, %bb8
  %i12 = phi i64 [ 1, %bb8 ], [ %i20, %bb19 ]
  %i13 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i10, i64 %i12)
  br label %bb14

bb14:                                             ; preds = %bb14, %bb11
  %i15 = phi i64 [ 1, %bb11 ], [ %i17, %bb14 ]
  %i16 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i13, i64 %i15)
  store i32 2, ptr %i16, align 4
  %i17 = add nuw nsw i64 %i15, 1
  %i18 = icmp eq i64 %i17, 10
  br i1 %i18, label %bb19, label %bb14

bb19:                                             ; preds = %bb14
  %i20 = add nuw nsw i64 %i12, 1
  %i21 = icmp eq i64 %i20, 10
  br i1 %i21, label %bb22, label %bb11

bb22:                                             ; preds = %bb19
  %i23 = add nuw nsw i64 %i9, 1
  %i24 = icmp eq i64 %i23, 10
  br i1 %i24, label %bb25, label %bb8

bb25:                                             ; preds = %bb22
  store i32 1, ptr @brute_force_mp_count_, align 8
  store i32 1, ptr %i4, align 4
  call void @main_IP_digits_2_(ptr nonnull %i4)
  %i26 = getelementptr inbounds [4 x i8], ptr %i3, i64 0, i64 0
  store i8 56, ptr %i26, align 1
  %i27 = getelementptr inbounds [4 x i8], ptr %i3, i64 0, i64 1
  store i8 4, ptr %i27, align 1
  %i28 = getelementptr inbounds [4 x i8], ptr %i3, i64 0, i64 2
  store i8 2, ptr %i28, align 1
  %i29 = getelementptr inbounds [4 x i8], ptr %i3, i64 0, i64 3
  store i8 0, ptr %i29, align 1
  %i30 = getelementptr inbounds { i64, ptr }, ptr %i2, i64 0, i32 0
  store i64 9, ptr %i30, align 8
  %i31 = getelementptr inbounds { i64, ptr }, ptr %i2, i64 0, i32 1
  store ptr getelementptr inbounds ([9 x i8], ptr @anon.2e0c92f0a1367b9abe0b93b08f29c950.0, i64 0, i64 0), ptr %i31, align 8
  %i34 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i6, i32 -1, i64 1239157112576, ptr nonnull %i26, ptr nonnull %i2)
  br label %bb35

bb35:                                             ; preds = %bb54, %bb25
  %i36 = phi i64 [ 1, %bb25 ], [ %i55, %bb54 ]
  %i37 = phi i32 [ 0, %bb25 ], [ %i48, %bb54 ]
  %i38 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x [9 x i32]]], ptr @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %i36)
  br label %bb39

bb39:                                             ; preds = %bb51, %bb35
  %i40 = phi i64 [ 1, %bb35 ], [ %i52, %bb51 ]
  %i41 = phi i32 [ %i37, %bb35 ], [ %i48, %bb51 ]
  %i42 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i38, i64 %i40)
  br label %bb43

bb43:                                             ; preds = %bb43, %bb39
  %i44 = phi i64 [ 1, %bb39 ], [ %i49, %bb43 ]
  %i45 = phi i32 [ %i41, %bb39 ], [ %i48, %bb43 ]
  %i46 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i42, i64 %i44)
  %i47 = load i32, ptr %i46, align 4
  %i48 = add nsw i32 %i47, %i45
  %i49 = add nuw nsw i64 %i44, 1
  %i50 = icmp eq i64 %i49, 10
  br i1 %i50, label %bb51, label %bb43

bb51:                                             ; preds = %bb43
  %i52 = add nuw nsw i64 %i40, 1
  %i53 = icmp eq i64 %i52, 10
  br i1 %i53, label %bb54, label %bb39

bb54:                                             ; preds = %bb51
  %i55 = add nuw nsw i64 %i36, 1
  %i56 = icmp eq i64 %i55, 10
  br i1 %i56, label %bb57, label %bb35

bb57:                                             ; preds = %bb54
  %i58 = getelementptr inbounds [4 x i8], ptr %i1, i64 0, i64 0
  store i8 9, ptr %i58, align 1
  %i59 = getelementptr inbounds [4 x i8], ptr %i1, i64 0, i64 1
  store i8 1, ptr %i59, align 1
  %i60 = getelementptr inbounds [4 x i8], ptr %i1, i64 0, i64 2
  store i8 1, ptr %i60, align 1
  %i61 = getelementptr inbounds [4 x i8], ptr %i1, i64 0, i64 3
  store i8 0, ptr %i61, align 1
  %i62 = getelementptr inbounds { i32 }, ptr %i, i64 0, i32 0
  store i32 %i48, ptr %i62, align 8
  %i64 = call i32 @for_write_seq_lis_xmit(ptr nonnull %i6, ptr nonnull %i58, ptr nonnull %i)
  ret void
}

declare dso_local i32 @for_set_reentrancy(ptr) local_unnamed_addr

define internal void @main_IP_digits_2_(ptr noalias nocapture readonly %arg) {
bb:
  %i = alloca i32, align 4
  %i1 = load i32, ptr %arg, align 4
  %i2 = sext i32 %i1 to i64
  %i3 = icmp eq i32 %i1, 8
  %i4 = add nsw i32 %i1, 1
  br label %bb5

bb5:                                              ; preds = %bb106, %bb
  %i6 = phi i64 [ %i107, %bb106 ], [ 1, %bb ]
  %i7 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x [9 x i32]]], ptr @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %i6)
  %i8 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i7, i64 1)
  %i9 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i8, i64 %i2)
  %i10 = load i32, ptr %i9, align 4
  %i11 = icmp slt i32 %i10, 1
  br i1 %i11, label %bb106, label %bb12

bb12:                                             ; preds = %bb5
  %i13 = trunc i64 %i6 to i32
  store i32 %i13, ptr %i9, align 4
  br label %bb14

bb14:                                             ; preds = %bb103, %bb12
  %i15 = phi i64 [ %i104, %bb103 ], [ 1, %bb12 ]
  %i16 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x [9 x i32]]], ptr @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %i15)
  %i17 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i16, i64 2)
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i17, i64 %i2)
  %i19 = load i32, ptr %i18, align 4
  %i20 = icmp slt i32 %i19, 1
  br i1 %i20, label %bb103, label %bb21

bb21:                                             ; preds = %bb100, %bb14
  %i22 = phi i64 [ %i101, %bb100 ], [ 1, %bb14 ]
  %i23 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x [9 x i32]]], ptr @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %i22)
  %i24 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i23, i64 3)
  %i25 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i24, i64 %i2)
  %i26 = load i32, ptr %i25, align 4
  %i27 = icmp slt i32 %i26, 0
  br i1 %i27, label %bb100, label %bb28

bb28:                                             ; preds = %bb97, %bb21
  %i29 = phi i64 [ %i98, %bb97 ], [ 1, %bb21 ]
  %i30 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x [9 x i32]]], ptr @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %i29)
  %i31 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i30, i64 4)
  %i32 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i31, i64 %i2)
  %i33 = load i32, ptr %i32, align 4
  %i34 = icmp slt i32 %i33, 0
  br i1 %i34, label %bb97, label %bb35

bb35:                                             ; preds = %bb94, %bb28
  %i36 = phi i64 [ %i95, %bb94 ], [ 1, %bb28 ]
  %i37 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x [9 x i32]]], ptr @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %i36)
  %i38 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i37, i64 5)
  %i39 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i38, i64 %i2)
  %i40 = load i32, ptr %i39, align 4
  %i41 = icmp slt i32 %i40, 0
  br i1 %i41, label %bb94, label %bb42

bb42:                                             ; preds = %bb91, %bb35
  %i43 = phi i64 [ %i92, %bb91 ], [ 1, %bb35 ]
  %i44 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x [9 x i32]]], ptr @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %i43)
  %i45 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i44, i64 6)
  %i46 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i45, i64 %i2)
  %i47 = load i32, ptr %i46, align 4
  %i48 = icmp slt i32 %i47, 1
  br i1 %i48, label %bb91, label %bb49

bb49:                                             ; preds = %bb42
  %i50 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i44, i64 7)
  %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i50, i64 %i2)
  %i52 = trunc i64 %i43 to i32
  store i32 %i52, ptr %i51, align 4
  br label %bb53

bb53:                                             ; preds = %bb88, %bb49
  %i54 = phi i64 [ %i89, %bb88 ], [ 1, %bb49 ]
  %i55 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x [9 x i32]]], ptr @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %i54)
  %i56 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i55, i64 7)
  %i57 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i56, i64 %i2)
  %i58 = load i32, ptr %i57, align 4
  %i59 = icmp slt i32 %i58, 1
  br i1 %i59, label %bb88, label %bb60

bb60:                                             ; preds = %bb53
  %i61 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i55, i64 1)
  %i62 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i61, i64 %i2)
  %i63 = trunc i64 %i54 to i32
  store i32 %i63, ptr %i62, align 4
  br label %bb64

bb64:                                             ; preds = %bb85, %bb60
  %i65 = phi i64 [ %i86, %bb85 ], [ 1, %bb60 ]
  %i66 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324, ptr elementtype(i32) getelementptr inbounds ([9 x [9 x [9 x i32]]], ptr @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %i65)
  %i67 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i66, i64 8)
  %i68 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i67, i64 %i2)
  %i69 = load i32, ptr %i68, align 4
  %i70 = icmp slt i32 %i69, 1
  br i1 %i70, label %bb85, label %bb71

bb71:                                             ; preds = %bb64
  %i72 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %i66, i64 9)
  %i73 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i72, i64 %i2)
  %i74 = trunc i64 %i65 to i32
  store i32 %i74, ptr %i73, align 4
  %i75 = load i32, ptr @brute_force_mp_count_, align 8
  %i76 = add nsw i32 %i75, 1
  store i32 %i76, ptr @brute_force_mp_count_, align 8
  br i1 %i3, label %bb77, label %bb79

bb77:                                             ; preds = %bb71
  %i78 = add nsw i32 %i75, 2
  store i32 %i78, ptr @brute_force_mp_count_, align 8
  br label %bb82

bb79:                                             ; preds = %bb71
  %i80 = icmp slt i32 %i76, 500000
  br i1 %i80, label %bb81, label %bb82

bb81:                                             ; preds = %bb79
  store i32 %i4, ptr %i, align 4
  call void @main_IP_digits_2_(ptr nonnull %i)
  br label %bb82

bb82:                                             ; preds = %bb81, %bb79, %bb77
  %i83 = load i32, ptr %i73, align 4
  %i84 = add nsw i32 %i83, 10
  store i32 %i84, ptr %i73, align 4
  br label %bb85

bb85:                                             ; preds = %bb82, %bb64
  %i86 = add nuw nsw i64 %i65, 1
  %i87 = icmp eq i64 %i86, 3
  br i1 %i87, label %bb88, label %bb64

bb88:                                             ; preds = %bb85, %bb53
  %i89 = add nuw nsw i64 %i54, 1
  %i90 = icmp eq i64 %i89, 4
  br i1 %i90, label %bb91, label %bb53

bb91:                                             ; preds = %bb88, %bb42
  %i92 = add nuw nsw i64 %i43, 1
  %i93 = icmp eq i64 %i92, 5
  br i1 %i93, label %bb94, label %bb42

bb94:                                             ; preds = %bb91, %bb35
  %i95 = add nuw nsw i64 %i36, 1
  %i96 = icmp eq i64 %i95, 4
  br i1 %i96, label %bb97, label %bb35

bb97:                                             ; preds = %bb94, %bb28
  %i98 = add nuw nsw i64 %i29, 1
  %i99 = icmp eq i64 %i98, 3
  br i1 %i99, label %bb100, label %bb28

bb100:                                            ; preds = %bb97, %bb21
  %i101 = add nuw nsw i64 %i22, 1
  %i102 = icmp eq i64 %i101, 7
  br i1 %i102, label %bb103, label %bb21

bb103:                                            ; preds = %bb100, %bb14
  %i104 = add nuw nsw i64 %i15, 1
  %i105 = icmp eq i64 %i104, 3
  br i1 %i105, label %bb106, label %bb14

bb106:                                            ; preds = %bb103, %bb5
  %i107 = add nuw nsw i64 %i6, 1
  %i108 = icmp eq i64 %i107, 4
  br i1 %i108, label %bb109, label %bb5

bb109:                                            ; preds = %bb106
  ret void
}

declare dso_local i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr

declare dso_local i32 @for_write_seq_lis_xmit(ptr, ptr, ptr) local_unnamed_addr

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

attributes #0 = { nounwind readnone speculatable }
; end INTEL_FEATURE_SW_ADVANCED

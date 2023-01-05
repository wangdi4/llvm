; REQUIRES: asserts
; RUN: opt -opaque-pointers < %s -dva-check-dtrans-outofboundsok -dtrans-outofboundsok=false -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck %s

; Check that field 1 of physpropmod_mp_physprop_ is determined to be a valid candidate for transpose
; even though field 0 contains a character string which is passed down to another subroutine and
; concatenated with another string and then passed down to another function.

; CHECK: Transpose candidate: physpropmod_mp_physprop_
; CHECK: Nested Field Number : 1
; CHECK: IsValid{{ *}}: true
; CHECK: IsProfitable{{ *}}: false

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"PHYSPROPMOD$.btPHYSPROP_TYPE" = type { [256 x i8], %"QNCA_a0$float*$rank2$" }
%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@strlit = internal unnamed_addr constant [5 x i8] c"hello"
@physpropmod_mp_physprop_ = internal global %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" { ptr null, i64 0, i64 0, i64 0, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@anon.dd98b7be2d4a48a7cca189d7201608bc.0 = internal unnamed_addr constant i32 2
@strlit.1 = internal unnamed_addr constant [13 x i8] c" and goodbye "

; Function Attrs: nofree noinline nounwind uwtable
define internal void @myinit_(ptr noalias nocapture readonly dereferenceable(4) %arg) #0 {
bb:
  %i = alloca [8 x i64], align 16
  %i1 = alloca [4 x i8], align 1
  %i2 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  %i3 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %i4 = load i32, ptr %arg, align 1
  %i5 = sext i32 %i4 to i64
  br label %bb6

bb6:                                              ; preds = %bb35, %bb
  %i7 = phi i64 [ %i36, %bb35 ], [ 1, %bb ]
  br label %bb8

bb8:                                              ; preds = %bb8, %bb6
  %i9 = phi i64 [ %i33, %bb8 ], [ 1, %bb6 ]
  %i10 = add nuw nsw i64 %i9, %i7
  %i11 = trunc i64 %i10 to i32
  %i12 = sitofp i32 %i11 to float
  %i13 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i14 = load i64, ptr %i2, align 1
  %i15 = load i64, ptr %i3, align 1
  %i16 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i15, i64 %i14, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i13, i64 %i5)
  %i17 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i16, i64 0, i32 1
  %i18 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i17, i64 0, i32 0
  %i19 = load ptr, ptr %i18, align 1
  %i20 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i17, i64 0, i32 6, i64 0
  %i21 = getelementptr inbounds { i64, i64, i64 }, ptr %i20, i64 0, i32 1
  %i22 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i21, i32 0)
  %i23 = load i64, ptr %i22, align 1
  %i24 = getelementptr inbounds { i64, i64, i64 }, ptr %i20, i64 0, i32 2
  %i25 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i24, i32 0)
  %i26 = load i64, ptr %i25, align 1
  %i27 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i21, i32 1)
  %i28 = load i64, ptr %i27, align 1
  %i29 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i24, i32 1)
  %i30 = load i64, ptr %i29, align 1
  %i31 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i30, i64 %i28, ptr elementtype(float) %i19, i64 %i9)
  %i32 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i26, i64 %i23, ptr elementtype(float) %i31, i64 %i7)
  store float %i12, ptr %i32, align 1
  %i33 = add nuw nsw i64 %i9, 1
  %i34 = icmp eq i64 %i33, 11
  br i1 %i34, label %bb35, label %bb8

bb35:                                             ; preds = %bb8
  %i36 = add nuw nsw i64 %i7, 1
  %i37 = icmp eq i64 %i36, 11
  br i1 %i37, label %bb38, label %bb6

bb38:                                             ; preds = %bb35
  %i39 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i40 = load i64, ptr %i2, align 1
  %i41 = load i64, ptr %i3, align 1
  %i42 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i41, i64 %i40, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i39, i64 %i5)
  %i43 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i42, i64 0, i32 1
  %i44 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i43, i64 0, i32 3
  %i45 = load i64, ptr %i44, align 1
  %i46 = and i64 %i45, 4
  %i47 = icmp ne i64 %i46, 0
  %i48 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i43, i64 0, i32 6, i64 0
  %i49 = getelementptr inbounds { i64, i64, i64 }, ptr %i48, i64 0, i32 1
  %i50 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i49, i32 0)
  %i51 = load i64, ptr %i50, align 1, !range !3
  %i52 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i43, i64 0, i32 1
  %i53 = load i64, ptr %i52, align 1
  %i54 = icmp eq i64 %i51, %i53
  %i55 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i49, i32 1)
  %i56 = load i64, ptr %i55, align 1, !range !3
  %i57 = getelementptr inbounds { i64, i64, i64 }, ptr %i48, i64 0, i32 0
  %i58 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i57, i32 0)
  %i59 = load i64, ptr %i58, align 1
  %i60 = mul nsw i64 %i59, %i51
  %i61 = icmp eq i64 %i56, %i60
  %i62 = and i1 %i54, %i61
  %i63 = or i1 %i47, %i62
  %i64 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i43, i64 0, i32 0
  %i65 = load ptr, ptr %i64, align 1
  %i66 = getelementptr inbounds { i64, i64, i64 }, ptr %i48, i64 0, i32 2
  %i67 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i66, i32 0)
  %i68 = load i64, ptr %i67, align 1
  %i69 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i66, i32 1)
  %i70 = load i64, ptr %i69, align 1
  %i71 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i57, i32 1)
  %i72 = load i64, ptr %i71, align 1
  %i73 = shl nsw i64 %i59, 2
  br i1 %i63, label %bb74, label %bb76

bb74:                                             ; preds = %bb38
  %i75 = mul nsw i64 %i72, %i73
  br label %bb104

bb76:                                             ; preds = %bb38
  %i77 = mul nsw i64 %i72, %i73
  %i78 = sdiv i64 %i77, 4
  %i79 = alloca float, i64 %i78, align 4
  %i80 = icmp slt i64 %i72, 1
  br i1 %i80, label %bb104, label %bb81

bb81:                                             ; preds = %bb76
  %i82 = icmp slt i64 %i59, 1
  %i83 = add nsw i64 %i59, 1
  %i84 = add nuw nsw i64 %i72, 1
  br label %bb98

bb85:                                             ; preds = %bb101, %bb85
  %i86 = phi i64 [ %i68, %bb101 ], [ %i91, %bb85 ]
  %i87 = phi i64 [ 1, %bb101 ], [ %i92, %bb85 ]
  %i88 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i68, i64 %i51, ptr elementtype(float) %i102, i64 %i86)
  %i89 = load float, ptr %i88, align 1
  %i90 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %i103, i64 %i87)
  store float %i89, ptr %i90, align 1
  %i91 = add nsw i64 %i86, 1
  %i92 = add nuw nsw i64 %i87, 1
  %i93 = icmp eq i64 %i92, %i83
  br i1 %i93, label %bb94, label %bb85

bb94:                                             ; preds = %bb98, %bb85
  %i95 = add nsw i64 %i99, 1
  %i96 = add nuw nsw i64 %i100, 1
  %i97 = icmp eq i64 %i96, %i84
  br i1 %i97, label %bb104, label %bb98

bb98:                                             ; preds = %bb94, %bb81
  %i99 = phi i64 [ %i70, %bb81 ], [ %i95, %bb94 ]
  %i100 = phi i64 [ 1, %bb81 ], [ %i96, %bb94 ]
  br i1 %i82, label %bb94, label %bb101

bb101:                                            ; preds = %bb98
  %i102 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i70, i64 %i56, ptr elementtype(float) %i65, i64 %i99)
  %i103 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i73, ptr nonnull elementtype(float) %i79, i64 %i100)
  br label %bb85

bb104:                                            ; preds = %bb94, %bb76, %bb74
  %i105 = phi i64 [ %i75, %bb74 ], [ %i77, %bb76 ], [ %i77, %bb94 ]
  %i106 = phi ptr [ %i65, %bb74 ], [ %i79, %bb76 ], [ %i79, %bb94 ]
  %i107 = bitcast ptr %i106 to ptr
  %i108 = getelementptr inbounds [4 x i8], ptr %i1, i64 0, i64 0
  store i8 26, ptr %i108, align 1
  %i109 = getelementptr inbounds [4 x i8], ptr %i1, i64 0, i64 1
  store i8 5, ptr %i109, align 1
  %i110 = getelementptr inbounds [4 x i8], ptr %i1, i64 0, i64 2
  store i8 1, ptr %i110, align 1
  %i111 = getelementptr inbounds [4 x i8], ptr %i1, i64 0, i64 3
  store i8 0, ptr %i111, align 1
  %i112 = alloca { i64, ptr }, align 8
  %i113 = getelementptr inbounds { i64, ptr }, ptr %i112, i64 0, i32 0
  store i64 %i105, ptr %i113, align 8
  %i114 = getelementptr inbounds { i64, ptr }, ptr %i112, i64 0, i32 1
  store ptr %i107, ptr %i114, align 8
  %i115 = bitcast ptr %i to ptr
  %i116 = bitcast ptr %i112 to ptr
  %i117 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i115, i32 -1, i64 1239157112576, ptr nonnull %i108, ptr nonnull %i116) #6
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr #1

; Function Attrs: nofree noreturn nounwind uwtable
define dso_local void @MAIN__() #2 {
bb:
  %i = alloca i32, align 8
  %i1 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.dd98b7be2d4a48a7cca189d7201608bc.0) #6
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 5), align 8
  store i64 352, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 1), align 8
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 4), align 8
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 2), align 8
  %i2 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %i2, align 1
  %i3 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 10, ptr %i3, align 1
  %i4 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 352, ptr %i4, align 1
  store i64 1073741829, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  %i5 = tail call i32 @for_allocate_handle(i64 3520, ptr @physpropmod_mp_physprop_, i32 262144, ptr null) #6
  store i32 1, ptr %i, align 8
  br label %bb6

bb6:                                              ; preds = %bb6, %bb
  %i7 = phi i32 [ %i52, %bb6 ], [ 1, %bb ]
  %i8 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i9 = load i64, ptr %i4, align 1
  %i10 = load i64, ptr %i2, align 1
  %i11 = sext i32 %i7 to i64
  %i12 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i10, i64 %i9, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i8, i64 %i11)
  %i13 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i12, i64 0, i32 0
  %i14 = getelementptr [256 x i8], ptr %i13, i64 0, i64 0
  %i15 = getelementptr i8, ptr %i14, i64 5
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 1 dereferenceable(5) %i14, ptr noundef nonnull align 1 dereferenceable(5) @strlit, i64 5, i1 false)
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 1 dereferenceable(251) %i15, i8 32, i64 251, i1 false)
  %i16 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i17 = load i64, ptr %i4, align 1
  %i18 = load i64, ptr %i2, align 1
  %i19 = load i32, ptr %i, align 8
  %i20 = sext i32 %i19 to i64
  %i21 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i18, i64 %i17, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i16, i64 %i20)
  %i22 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i21, i64 0, i32 1
  %i23 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i22, i64 0, i32 3
  store i64 5, ptr %i23, align 1
  %i24 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i22, i64 0, i32 5
  store i64 0, ptr %i24, align 1
  %i25 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i22, i64 0, i32 1
  store i64 4, ptr %i25, align 1
  %i26 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i22, i64 0, i32 4
  store i64 2, ptr %i26, align 1
  %i27 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i22, i64 0, i32 2
  store i64 0, ptr %i27, align 1
  %i28 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i22, i64 0, i32 6, i64 0
  %i29 = getelementptr inbounds { i64, i64, i64 }, ptr %i28, i64 0, i32 2
  %i30 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i29, i32 0)
  store i64 1, ptr %i30, align 1
  %i31 = getelementptr inbounds { i64, i64, i64 }, ptr %i28, i64 0, i32 0
  %i32 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i31, i32 0)
  store i64 10, ptr %i32, align 1
  %i33 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i29, i32 1)
  store i64 1, ptr %i33, align 1
  %i34 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i31, i32 1)
  store i64 10, ptr %i34, align 1
  %i35 = getelementptr inbounds { i64, i64, i64 }, ptr %i28, i64 0, i32 1
  %i36 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i35, i32 0)
  store i64 4, ptr %i36, align 1
  %i37 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i35, i32 1)
  store i64 40, ptr %i37, align 1
  %i38 = load i64, ptr %i23, align 1
  %i39 = and i64 %i38, -68451041281
  %i40 = or i64 %i39, 1073741824
  store i64 %i40, ptr %i23, align 1
  %i41 = load i64, ptr %i24, align 1
  %i42 = inttoptr i64 %i41 to ptr
  %i43 = bitcast ptr %i22 to ptr
  %i44 = call i32 @for_allocate_handle(i64 400, ptr nonnull %i43, i32 262144, ptr %i42) #6
  %i45 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i46 = load i64, ptr %i4, align 1
  %i47 = load i64, ptr %i2, align 1
  %i48 = load i32, ptr %i, align 8
  %i49 = sext i32 %i48 to i64
  %i50 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i47, i64 %i46, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i45, i64 %i49)
  call void @myoutput_(ptr %i50) #6
  call void @myinit_(ptr nonnull %i) #6
  %i51 = load i32, ptr %i, align 8
  %i52 = add nsw i32 %i51, 1
  store i32 %i52, ptr %i, align 8
  br label %bb6
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: nofree noinline nounwind uwtable
define internal void @myoutput_(ptr noalias dereferenceable(352) %arg) #0 {
bb:
  %i = alloca [2 x { ptr, i64 }], align 8
  %i1 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %arg, i64 0, i32 0
  %i2 = getelementptr inbounds [2 x { ptr, i64 }], ptr %i, i64 0, i64 0
  %i3 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 1, i32 16, ptr nonnull elementtype({ ptr, i64 }) %i2, i32 1)
  %i4 = getelementptr inbounds { ptr, i64 }, ptr %i3, i64 0, i32 0
  %i5 = getelementptr [256 x i8], ptr %i1, i64 0, i64 0
  store ptr %i5, ptr %i4, align 1
  %i6 = getelementptr inbounds { ptr, i64 }, ptr %i3, i64 0, i32 1
  store i64 256, ptr %i6, align 1
  %i7 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 1, i32 16, ptr nonnull elementtype({ ptr, i64 }) %i2, i32 2)
  %i8 = getelementptr inbounds { ptr, i64 }, ptr %i7, i64 0, i32 0
  store ptr @strlit.1, ptr %i8, align 1
  %i9 = getelementptr inbounds { ptr, i64 }, ptr %i7, i64 0, i32 1
  store i64 13, ptr %i9, align 1
  %i10 = alloca [269 x i8], align 1
  %i11 = getelementptr inbounds [269 x i8], ptr %i10, i64 0, i64 0
  %i12 = bitcast ptr %i to ptr
  call void @for_concat(ptr nonnull %i12, i64 2, ptr nonnull %i11, i64 269) #6
  call void @myerror_(ptr nonnull %i11, i64 269) #6
  ret void
}

; Function Attrs: nofree
declare dso_local void @for_concat(ptr nocapture readonly, i64, ptr nocapture, i64) local_unnamed_addr #1

; Function Attrs: nofree noinline nounwind uwtable
define internal void @myerror_(ptr noalias readonly %arg, i64 %arg1) #0 {
bb:
  %i = alloca [8 x i64], align 16
  %i2 = alloca [4 x i8], align 1
  %i3 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 0
  store i8 56, ptr %i3, align 1
  %i4 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 1
  store i8 4, ptr %i4, align 1
  %i5 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 2
  store i8 1, ptr %i5, align 1
  %i6 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 3
  store i8 0, ptr %i6, align 1
  %i7 = alloca { i64, ptr }, align 8
  %i8 = getelementptr inbounds { i64, ptr }, ptr %i7, i64 0, i32 0
  store i64 269, ptr %i8, align 8
  %i9 = getelementptr inbounds { i64, ptr }, ptr %i7, i64 0, i32 1
  store ptr %arg, ptr %i9, align 8
  %i10 = bitcast ptr %i to ptr
  %i11 = bitcast ptr %i7 to ptr
  %i12 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i10, i32 -1, i64 1239157112576, ptr nonnull %i3, ptr nonnull %i11) #6
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #3

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #3

; Function Attrs: argmemonly nocallback nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #4

; Function Attrs: argmemonly nocallback nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #5

attributes #0 = { nofree noinline nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nofree noreturn nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #3 = { nounwind readnone speculatable }
attributes #4 = { argmemonly nocallback nofree nounwind willreturn }
attributes #5 = { argmemonly nocallback nofree nounwind willreturn writeonly }
attributes #6 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
!3 = !{i64 1, i64 -9223372036854775808}

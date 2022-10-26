; REQUIRES: asserts
; RUN: opt -opaque-pointers < %s -dva-check-dtrans-outofboundsok -dtrans-outofboundsok=true -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck %s

; Check that with -dtrans-outofboundsok=true, we get no transpose candidates
; as our analysis is too weak at this point.

; CHECK: No transpose candidates

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"PHYSPROPMOD$.btPHYSPROP_TYPE" = type { i32, %"QNCA_a0$float*$rank2$" }
%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@physpropmod_mp_physprop_ = internal global %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" { ptr null, i64 0, i64 0, i64 0, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@anon.263b53731fe38c4199c4a10e662745ed.0 = internal unnamed_addr constant i32 2

; Function Attrs: nofree noinline nounwind uwtable
define internal void @myinit_(ptr noalias nocapture readonly dereferenceable(4) %arg, ptr noalias nocapture readonly dereferenceable(4) %arg1) #0 {
bb:
  %i = alloca [8 x i64], align 16
  %i2 = alloca [4 x i8], align 1
  %i3 = alloca { i64, ptr }, align 8
  %i4 = load i32, ptr %arg1, align 1
  %i5 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  %i6 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %i7 = load i32, ptr %arg, align 1
  %i8 = sext i32 %i7 to i64
  br label %bb9

bb9:                                              ; preds = %bb39, %bb
  %i10 = phi i64 [ %i40, %bb39 ], [ 1, %bb ]
  br label %bb11

bb11:                                             ; preds = %bb11, %bb9
  %i12 = phi i64 [ %i37, %bb11 ], [ 1, %bb9 ]
  %i13 = add nuw nsw i64 %i10, %i12
  %i14 = trunc i64 %i13 to i32
  %i15 = add nsw i32 %i4, %i14
  %i16 = sitofp i32 %i15 to float
  %i17 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i18 = load i64, ptr %i5, align 1
  %i19 = load i64, ptr %i6, align 1
  %i20 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i19, i64 %i18, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i17, i64 %i8)
  %i21 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i20, i64 0, i32 1
  %i22 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i21, i64 0, i32 0
  %i23 = load ptr, ptr %i22, align 1
  %i24 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i21, i64 0, i32 6, i64 0
  %i25 = getelementptr inbounds { i64, i64, i64 }, ptr %i24, i64 0, i32 1
  %i26 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i25, i32 0)
  %i27 = load i64, ptr %i26, align 1
  %i28 = getelementptr inbounds { i64, i64, i64 }, ptr %i24, i64 0, i32 2
  %i29 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i28, i32 0)
  %i30 = load i64, ptr %i29, align 1
  %i31 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i25, i32 1)
  %i32 = load i64, ptr %i31, align 1
  %i33 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i28, i32 1)
  %i34 = load i64, ptr %i33, align 1
  %i35 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i34, i64 %i32, ptr elementtype(float) %i23, i64 %i12)
  %i36 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i30, i64 %i27, ptr elementtype(float) %i35, i64 %i10)
  store float %i16, ptr %i36, align 1
  %i37 = add nuw nsw i64 %i12, 1
  %i38 = icmp eq i64 %i37, 11
  br i1 %i38, label %bb39, label %bb11

bb39:                                             ; preds = %bb11
  %i40 = add nuw nsw i64 %i10, 1
  %i41 = icmp eq i64 %i40, 11
  br i1 %i41, label %bb42, label %bb9

bb42:                                             ; preds = %bb39
  %i43 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i44 = load i64, ptr %i5, align 1
  %i45 = load i64, ptr %i6, align 1
  %i46 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i45, i64 %i44, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i43, i64 %i8)
  %i47 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i46, i64 0, i32 1
  %i48 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i47, i64 0, i32 3
  %i49 = load i64, ptr %i48, align 1
  %i50 = and i64 %i49, 4
  %i51 = icmp ne i64 %i50, 0
  %i52 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i47, i64 0, i32 6, i64 0
  %i53 = getelementptr inbounds { i64, i64, i64 }, ptr %i52, i64 0, i32 1
  %i54 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i53, i32 0)
  %i55 = load i64, ptr %i54, align 1, !range !3
  %i56 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i47, i64 0, i32 1
  %i57 = load i64, ptr %i56, align 1
  %i58 = icmp eq i64 %i55, %i57
  %i59 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i53, i32 1)
  %i60 = load i64, ptr %i59, align 1, !range !3
  %i61 = getelementptr inbounds { i64, i64, i64 }, ptr %i52, i64 0, i32 0
  %i62 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i61, i32 0)
  %i63 = load i64, ptr %i62, align 1
  %i64 = mul nsw i64 %i63, %i55
  %i65 = icmp eq i64 %i60, %i64
  %i66 = and i1 %i58, %i65
  %i67 = or i1 %i51, %i66
  %i68 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i47, i64 0, i32 0
  %i69 = load ptr, ptr %i68, align 1
  %i70 = getelementptr inbounds { i64, i64, i64 }, ptr %i52, i64 0, i32 2
  %i71 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i70, i32 0)
  %i72 = load i64, ptr %i71, align 1
  %i73 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i70, i32 1)
  %i74 = load i64, ptr %i73, align 1
  %i75 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i61, i32 1)
  %i76 = load i64, ptr %i75, align 1
  %i77 = shl nsw i64 %i63, 2
  br i1 %i67, label %bb78, label %bb80

bb78:                                             ; preds = %bb42
  %i79 = mul nsw i64 %i76, %i77
  br label %bb108

bb80:                                             ; preds = %bb42
  %i81 = mul nsw i64 %i76, %i77
  %i82 = sdiv i64 %i81, 4
  %i83 = alloca float, i64 %i82, align 4
  %i84 = icmp slt i64 %i76, 1
  br i1 %i84, label %bb108, label %bb85

bb85:                                             ; preds = %bb80
  %i86 = icmp slt i64 %i63, 1
  %i87 = add nsw i64 %i63, 1
  %i88 = add nuw nsw i64 %i76, 1
  br label %bb102

bb89:                                             ; preds = %bb105, %bb89
  %i90 = phi i64 [ %i72, %bb105 ], [ %i95, %bb89 ]
  %i91 = phi i64 [ 1, %bb105 ], [ %i96, %bb89 ]
  %i92 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i72, i64 %i55, ptr elementtype(float) %i106, i64 %i90)
  %i93 = load float, ptr %i92, align 1
  %i94 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %i107, i64 %i91)
  store float %i93, ptr %i94, align 1
  %i95 = add nsw i64 %i90, 1
  %i96 = add nuw nsw i64 %i91, 1
  %i97 = icmp eq i64 %i96, %i87
  br i1 %i97, label %bb98, label %bb89

bb98:                                             ; preds = %bb102, %bb89
  %i99 = add nsw i64 %i103, 1
  %i100 = add nuw nsw i64 %i104, 1
  %i101 = icmp eq i64 %i100, %i88
  br i1 %i101, label %bb108, label %bb102

bb102:                                            ; preds = %bb98, %bb85
  %i103 = phi i64 [ %i74, %bb85 ], [ %i99, %bb98 ]
  %i104 = phi i64 [ 1, %bb85 ], [ %i100, %bb98 ]
  br i1 %i86, label %bb98, label %bb105

bb105:                                            ; preds = %bb102
  %i106 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i74, i64 %i60, ptr elementtype(float) %i69, i64 %i103)
  %i107 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i77, ptr nonnull elementtype(float) %i83, i64 %i104)
  br label %bb89

bb108:                                            ; preds = %bb98, %bb80, %bb78
  %i109 = phi i64 [ %i79, %bb78 ], [ %i81, %bb80 ], [ %i81, %bb98 ]
  %i110 = phi ptr [ %i69, %bb78 ], [ %i83, %bb80 ], [ %i83, %bb98 ]
  %i111 = bitcast ptr %i110 to ptr
  %i112 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 0
  store i8 26, ptr %i112, align 1
  %i113 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 1
  store i8 5, ptr %i113, align 1
  %i114 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 2
  store i8 1, ptr %i114, align 1
  %i115 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 3
  store i8 0, ptr %i115, align 1
  %i116 = getelementptr inbounds { i64, ptr }, ptr %i3, i64 0, i32 0
  store i64 %i109, ptr %i116, align 8
  %i117 = getelementptr inbounds { i64, ptr }, ptr %i3, i64 0, i32 1
  store ptr %i111, ptr %i117, align 8
  %i118 = bitcast ptr %i to ptr
  %i119 = bitcast ptr %i3 to ptr
  %i120 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i118, i32 -1, i64 1239157112576, ptr nonnull %i112, ptr nonnull %i119) #4
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr #1

; Function Attrs: nofree noreturn nounwind uwtable
define dso_local void @MAIN__() #2 {
bb:
  %i = alloca i32, align 8
  %i1 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.263b53731fe38c4199c4a10e662745ed.0) #4
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 5), align 8
  store i64 104, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 1), align 8
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 4), align 8
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 2), align 8
  %i2 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %i2, align 1
  %i3 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 10, ptr %i3, align 1
  %i4 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 104, ptr %i4, align 1
  store i64 1073741829, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  %i5 = tail call i32 @for_allocate_handle(i64 1040, ptr @physpropmod_mp_physprop_, i32 262144, ptr null) #4
  store i32 1, ptr %i, align 8
  br label %bb6

bb6:                                              ; preds = %bb6, %bb
  %i7 = phi i32 [ %i51, %bb6 ], [ 1, %bb ]
  %i8 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i9 = load i64, ptr %i4, align 1
  %i10 = load i64, ptr %i2, align 1
  %i11 = sext i32 %i7 to i64
  %i12 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i10, i64 %i9, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i8, i64 %i11)
  %i13 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i12, i64 0, i32 0
  store i32 50, ptr %i13, align 1
  %i14 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i15 = load i64, ptr %i4, align 1
  %i16 = load i64, ptr %i2, align 1
  %i17 = load i32, ptr %i, align 8
  %i18 = sext i32 %i17 to i64
  %i19 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i16, i64 %i15, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i14, i64 %i18)
  %i20 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i19, i64 0, i32 1
  %i21 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i20, i64 0, i32 3
  store i64 5, ptr %i21, align 1
  %i22 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i20, i64 0, i32 5
  store i64 0, ptr %i22, align 1
  %i23 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i20, i64 0, i32 1
  store i64 4, ptr %i23, align 1
  %i24 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i20, i64 0, i32 4
  store i64 2, ptr %i24, align 1
  %i25 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i20, i64 0, i32 2
  store i64 0, ptr %i25, align 1
  %i26 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i20, i64 0, i32 6, i64 0
  %i27 = getelementptr inbounds { i64, i64, i64 }, ptr %i26, i64 0, i32 2
  %i28 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i27, i32 0)
  store i64 1, ptr %i28, align 1
  %i29 = getelementptr inbounds { i64, i64, i64 }, ptr %i26, i64 0, i32 0
  %i30 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i29, i32 0)
  store i64 10, ptr %i30, align 1
  %i31 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i27, i32 1)
  store i64 1, ptr %i31, align 1
  %i32 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i29, i32 1)
  store i64 10, ptr %i32, align 1
  %i33 = getelementptr inbounds { i64, i64, i64 }, ptr %i26, i64 0, i32 1
  %i34 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i33, i32 0)
  store i64 4, ptr %i34, align 1
  %i35 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i33, i32 1)
  store i64 40, ptr %i35, align 1
  %i36 = load i64, ptr %i21, align 1
  %i37 = and i64 %i36, -68451041281
  %i38 = or i64 %i37, 1073741824
  store i64 %i38, ptr %i21, align 1
  %i39 = load i64, ptr %i22, align 1
  %i40 = inttoptr i64 %i39 to ptr
  %i41 = bitcast ptr %i20 to ptr
  %i42 = call i32 @for_allocate_handle(i64 400, ptr nonnull %i41, i32 262144, ptr %i40) #4
  %i43 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i44 = load i64, ptr %i4, align 1
  %i45 = load i64, ptr %i2, align 1
  %i46 = load i32, ptr %i, align 8
  %i47 = sext i32 %i46 to i64
  %i48 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i45, i64 %i44, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i43, i64 %i47)
  %i49 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i48, i64 0, i32 0
  call void @myinit_(ptr nonnull %i, ptr %i49) #4
  %i50 = load i32, ptr %i, align 8
  %i51 = add nsw i32 %i50, 1
  store i32 %i51, ptr %i, align 8
  br label %bb6
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #3

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #3

attributes #0 = { nofree noinline nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nofree noreturn nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #3 = { nounwind readnone speculatable }
attributes #4 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
!3 = !{i64 1, i64 -9223372036854775808}

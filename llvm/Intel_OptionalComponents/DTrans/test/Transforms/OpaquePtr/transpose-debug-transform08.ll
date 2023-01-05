; REQUIRES: asserts
; RUN: opt -opaque-pointers < %s -disable-output -passes=dtrans-transpose -debug-only=dtrans-transpose-transform 2>&1 | FileCheck %s

; Check that the array represented by the dope vector in
; physpropmod_mp_physprop_ field 2 is transposed even though the allocate
; statements that allocate their arrays are in different routines than the
; routine in which their array accesses appear and there are multiple allocate
; statements for these two fields.

; Check that the array represented by field 0 of physpropmod_mp_physprop_
; is not transposed because the indirectly subscripted index is not the
; fastest varying subscript.

; CHECK-LABEL: Transform candidate: physpropmod_mp_physprop_[0]
; CHECK-NOT: Before
; CHECK-NOT: After

; Check that the array represented by field 2 of physpropmod_mp_physprop_
; is transposed to ensure that the indirectly subscripted index is not the
; fastest varying subscript.

; CHECK-LABEL: Transform candidate: physpropmod_mp_physprop_[2]
; CHECK-NEXT: Before: MAIN__: %[[N0:[A-Za-z0-9]+]] = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %[[I0:[A-Za-z0-9]+]],
; CHECK-NEXT: After : MAIN__: %[[N0]] = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4,
; CHECK-NEXT: Before: MAIN__: %[[N1:[A-Za-z0-9]+]] = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %[[I1:[A-Za-z0-9]+]],
; CHECK-NEXT: After : MAIN__: %[[N1]] = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 4000,

; Check that the array through which the indirect subscripting is not
; transposed.

; CHECK-LABEL: Transform candidate: main_$MYK
; CHECK-NOT: Before
; CHECK-NOT: After

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"PHYSPROPMOD$.btPHYSPROP_TYPE" = type { %"QNCA_a0$float*$rank2$", i32, %"QNCA_a0$float*$rank2$" }
%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@physpropmod_mp_physprop_ = internal global %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" { ptr null, i64 0, i64 0, i64 0, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@"main_$MYK" = internal unnamed_addr global [1000 x [19 x i32]] zeroinitializer, align 16
@anon.834d714ac9ab6fb7516b01a96e7925b0.0 = internal unnamed_addr constant i32 2

; Function Attrs: nofree noinline nounwind uwtable
define internal void @initfield0_(ptr noalias nocapture dereferenceable(200) %arg) #0 {
bb:
  %i = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %arg, i64 0, i32 0
  %i1 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 3
  %i2 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 5
  store i64 0, ptr %i2, align 1
  %i3 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 1
  store i64 4, ptr %i3, align 1
  %i4 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 4
  store i64 2, ptr %i4, align 1
  %i5 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 2
  store i64 0, ptr %i5, align 1
  %i6 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 6, i64 0
  %i7 = getelementptr inbounds { i64, i64, i64 }, ptr %i6, i64 0, i32 2
  %i8 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i7, i32 0)
  store i64 1, ptr %i8, align 1
  %i9 = getelementptr inbounds { i64, i64, i64 }, ptr %i6, i64 0, i32 0
  %i10 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i9, i32 0)
  store i64 19, ptr %i10, align 1
  %i11 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i7, i32 1)
  store i64 1, ptr %i11, align 1
  %i12 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i9, i32 1)
  store i64 1000, ptr %i12, align 1
  %i13 = getelementptr inbounds { i64, i64, i64 }, ptr %i6, i64 0, i32 1
  %i14 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i13, i32 0)
  store i64 4, ptr %i14, align 1
  %i15 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i13, i32 1)
  store i64 76, ptr %i15, align 1
  store i64 1073741829, ptr %i1, align 1
  %i16 = bitcast ptr %arg to ptr
  %i17 = tail call i32 @for_allocate_handle(i64 76000, ptr nonnull %i16, i32 262144, ptr null) #5
  %i18 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %arg, i64 0, i32 2
  %i19 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i18, i64 0, i32 3
  %i20 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i18, i64 0, i32 5
  store i64 0, ptr %i20, align 1
  %i21 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i18, i64 0, i32 1
  store i64 4, ptr %i21, align 1
  %i22 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i18, i64 0, i32 4
  store i64 2, ptr %i22, align 1
  %i23 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i18, i64 0, i32 2
  store i64 0, ptr %i23, align 1
  %i24 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i18, i64 0, i32 6, i64 0
  %i25 = getelementptr inbounds { i64, i64, i64 }, ptr %i24, i64 0, i32 2
  %i26 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i25, i32 0)
  store i64 1, ptr %i26, align 1
  %i27 = getelementptr inbounds { i64, i64, i64 }, ptr %i24, i64 0, i32 0
  %i28 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i27, i32 0)
  store i64 19, ptr %i28, align 1
  %i29 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i25, i32 1)
  store i64 1, ptr %i29, align 1
  %i30 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i27, i32 1)
  store i64 1000, ptr %i30, align 1
  %i31 = getelementptr inbounds { i64, i64, i64 }, ptr %i24, i64 0, i32 1
  %i32 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i31, i32 0)
  store i64 4, ptr %i32, align 1
  %i33 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i31, i32 1)
  store i64 76, ptr %i33, align 1
  store i64 1073741829, ptr %i19, align 1
  %i34 = bitcast ptr %i18 to ptr
  %i35 = tail call i32 @for_allocate_handle(i64 76000, ptr nonnull %i34, i32 262144, ptr null) #5
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: nofree noinline nounwind uwtable
define internal void @initfield1_(ptr noalias nocapture dereferenceable(200) %arg) #0 {
bb:
  %i = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %arg, i64 0, i32 2
  %i1 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 3
  %i2 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 5
  store i64 0, ptr %i2, align 1
  %i3 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 1
  store i64 4, ptr %i3, align 1
  %i4 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 4
  store i64 2, ptr %i4, align 1
  %i5 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 2
  store i64 0, ptr %i5, align 1
  %i6 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 6, i64 0
  %i7 = getelementptr inbounds { i64, i64, i64 }, ptr %i6, i64 0, i32 2
  %i8 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i7, i32 0)
  store i64 1, ptr %i8, align 1
  %i9 = getelementptr inbounds { i64, i64, i64 }, ptr %i6, i64 0, i32 0
  %i10 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i9, i32 0)
  store i64 19, ptr %i10, align 1
  %i11 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i7, i32 1)
  store i64 1, ptr %i11, align 1
  %i12 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i9, i32 1)
  store i64 1000, ptr %i12, align 1
  %i13 = getelementptr inbounds { i64, i64, i64 }, ptr %i6, i64 0, i32 1
  %i14 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i13, i32 0)
  store i64 4, ptr %i14, align 1
  %i15 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i13, i32 1)
  store i64 76, ptr %i15, align 1
  store i64 1073741829, ptr %i1, align 1
  %i16 = bitcast ptr %i to ptr
  %i17 = tail call i32 @for_allocate_handle(i64 76000, ptr nonnull %i16, i32 262144, ptr null) #5
  %i18 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %arg, i64 0, i32 0
  %i19 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i18, i64 0, i32 3
  %i20 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i18, i64 0, i32 5
  store i64 0, ptr %i20, align 1
  %i21 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i18, i64 0, i32 1
  store i64 4, ptr %i21, align 1
  %i22 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i18, i64 0, i32 4
  store i64 2, ptr %i22, align 1
  %i23 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i18, i64 0, i32 2
  store i64 0, ptr %i23, align 1
  %i24 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i18, i64 0, i32 6, i64 0
  %i25 = getelementptr inbounds { i64, i64, i64 }, ptr %i24, i64 0, i32 2
  %i26 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i25, i32 0)
  store i64 1, ptr %i26, align 1
  %i27 = getelementptr inbounds { i64, i64, i64 }, ptr %i24, i64 0, i32 0
  %i28 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i27, i32 0)
  store i64 19, ptr %i28, align 1
  %i29 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i25, i32 1)
  store i64 1, ptr %i29, align 1
  %i30 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i27, i32 1)
  store i64 1000, ptr %i30, align 1
  %i31 = getelementptr inbounds { i64, i64, i64 }, ptr %i24, i64 0, i32 1
  %i32 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i31, i32 0)
  store i64 4, ptr %i32, align 1
  %i33 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i31, i32 1)
  store i64 76, ptr %i33, align 1
  store i64 1073741829, ptr %i19, align 1
  %i34 = bitcast ptr %arg to ptr
  %i35 = tail call i32 @for_allocate_handle(i64 76000, ptr nonnull %i34, i32 262144, ptr null) #5
  ret void
}

; Function Attrs: nofree noinline nounwind uwtable
define internal void @initfield2_(ptr noalias nocapture dereferenceable(200) %arg) #0 {
bb:
  tail call void @initfield1_(ptr nonnull %arg)
  ret void
}

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #2 {
bb:
  %i = tail call i32 @for_set_reentrancy(ptr nonnull @anon.834d714ac9ab6fb7516b01a96e7925b0.0) #5
  br label %bb1

bb1:                                              ; preds = %bb10, %bb
  %i2 = phi i64 [ %i11, %bb10 ], [ 1, %bb ]
  %i3 = trunc i64 %i2 to i32
  br label %bb4

bb4:                                              ; preds = %bb4, %bb1
  %i5 = phi i64 [ %i8, %bb4 ], [ 1, %bb1 ]
  %i6 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 76, ptr elementtype(i32) @"main_$MYK", i64 %i5)
  %i7 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i6, i64 %i2)
  store i32 %i3, ptr %i7, align 1
  %i8 = add nuw nsw i64 %i5, 1
  %i9 = icmp eq i64 %i8, 1001
  br i1 %i9, label %bb10, label %bb4

bb10:                                             ; preds = %bb4
  %i11 = add nuw nsw i64 %i2, 1
  %i12 = icmp eq i64 %i11, 20
  br i1 %i12, label %bb13, label %bb1

bb13:                                             ; preds = %bb10
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 5), align 8
  store i64 200, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 1), align 8
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 4), align 8
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 2), align 8
  %i14 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %i14, align 1
  %i15 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 100, ptr %i15, align 1
  %i16 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 200, ptr %i16, align 1
  store i64 1073741829, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  %i17 = tail call i32 @for_allocate_handle(i64 20000, ptr @physpropmod_mp_physprop_, i32 262144, ptr null) #5
  br label %bb18

bb18:                                             ; preds = %bb18, %bb13
  %i19 = phi i64 [ %i28, %bb18 ], [ 1, %bb13 ]
  %i20 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i21 = load i64, ptr %i16, align 1
  %i22 = load i64, ptr %i14, align 1
  %i23 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i22, i64 %i21, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i20, i64 %i19)
  tail call void @initfield0_(ptr %i23)
  %i24 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i25 = load i64, ptr %i16, align 1
  %i26 = load i64, ptr %i14, align 1
  %i27 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i26, i64 %i25, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i24, i64 %i19)
  tail call void @initfield2_(ptr %i27)
  %i28 = add nuw nsw i64 %i19, 1
  %i29 = icmp eq i64 %i28, 101
  br i1 %i29, label %bb30, label %bb18

bb30:                                             ; preds = %bb93, %bb18
  %i31 = phi i64 [ %i94, %bb93 ], [ 1, %bb18 ]
  br label %bb32

bb32:                                             ; preds = %bb90, %bb30
  %i33 = phi i64 [ %i91, %bb90 ], [ 1, %bb30 ]
  %i34 = sub nsw i64 %i31, %i33
  %i35 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 76, ptr elementtype(i32) @"main_$MYK", i64 %i33)
  br label %bb36

bb36:                                             ; preds = %bb36, %bb32
  %i37 = phi i64 [ %i88, %bb36 ], [ 2, %bb32 ]
  %i38 = add nsw i64 %i34, %i37
  %i39 = trunc i64 %i38 to i32
  %i40 = sitofp i32 %i39 to float
  %i41 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i42 = load i64, ptr %i16, align 1
  %i43 = load i64, ptr %i14, align 1
  %i44 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i43, i64 %i42, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i41, i64 101)
  %i45 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i44, i64 0, i32 2
  %i46 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i45, i64 0, i32 0
  %i47 = load ptr, ptr %i46, align 1
  %i48 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i45, i64 0, i32 6, i64 0
  %i49 = getelementptr inbounds { i64, i64, i64 }, ptr %i48, i64 0, i32 1
  %i50 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i49, i32 0)
  %i51 = load i64, ptr %i50, align 1
  %i52 = getelementptr inbounds { i64, i64, i64 }, ptr %i48, i64 0, i32 2
  %i53 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i52, i32 0)
  %i54 = load i64, ptr %i53, align 1
  %i55 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i35, i64 %i37)
  %i56 = load i32, ptr %i55, align 1
  %i57 = add nsw i32 %i56, 1
  %i58 = sext i32 %i57 to i64
  %i59 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i49, i32 1)
  %i60 = load i64, ptr %i59, align 1
  %i61 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i52, i32 1)
  %i62 = load i64, ptr %i61, align 1
  %i63 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i62, i64 %i60, ptr elementtype(float) %i47, i64 %i33)
  %i64 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i54, i64 %i51, ptr elementtype(float) %i63, i64 %i58)
  store float %i40, ptr %i64, align 1
  %i65 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i66 = load i64, ptr %i16, align 1
  %i67 = load i64, ptr %i14, align 1
  %i68 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i67, i64 %i66, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i65, i64 101)
  %i69 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i68, i64 0, i32 0
  %i70 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i69, i64 0, i32 0
  %i71 = load ptr, ptr %i70, align 1
  %i72 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i69, i64 0, i32 6, i64 0
  %i73 = getelementptr inbounds { i64, i64, i64 }, ptr %i72, i64 0, i32 1
  %i74 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i73, i32 0)
  %i75 = load i64, ptr %i74, align 1
  %i76 = getelementptr inbounds { i64, i64, i64 }, ptr %i72, i64 0, i32 2
  %i77 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i76, i32 0)
  %i78 = load i64, ptr %i77, align 1
  %i79 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i73, i32 1)
  %i80 = load i64, ptr %i79, align 1
  %i81 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i76, i32 1)
  %i82 = load i64, ptr %i81, align 1
  %i83 = load i32, ptr %i55, align 1
  %i84 = add nsw i32 %i83, -1
  %i85 = sext i32 %i84 to i64
  %i86 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i82, i64 %i80, ptr elementtype(float) %i71, i64 %i85)
  %i87 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i78, i64 %i75, ptr elementtype(float) %i86, i64 %i33)
  store float %i40, ptr %i87, align 1
  %i88 = add nuw nsw i64 %i37, 1
  %i89 = icmp eq i64 %i88, 19
  br i1 %i89, label %bb90, label %bb36

bb90:                                             ; preds = %bb36
  %i91 = add nuw nsw i64 %i33, 1
  %i92 = icmp eq i64 %i91, 1001
  br i1 %i92, label %bb93, label %bb32

bb93:                                             ; preds = %bb90
  %i94 = add nuw nsw i64 %i31, 1
  %i95 = icmp eq i64 %i94, 101
  br i1 %i95, label %bb96, label %bb30

bb96:                                             ; preds = %bb93
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: nocallback nofree nosync nounwind readnone willreturn
declare i64 @llvm.ssa.copy.i64(i64 returned) #3

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #4

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #4

attributes #0 = { nofree noinline nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #3 = { nocallback nofree nosync nounwind readnone willreturn }
attributes #4 = { nounwind readnone speculatable }
attributes #5 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}

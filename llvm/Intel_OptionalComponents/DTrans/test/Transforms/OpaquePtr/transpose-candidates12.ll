; REQUIRES: asserts
; RUN: opt -opaque-pointers < %s -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck %s

; Check that physpropmod_mp_physprop_ fields 0 and 1 are both valid transpose
; candidates even though the allocate statements that allocate their arrays
; are in different routines than the routine in which their array accesses
; appear, there are multiple allocate statements for these two fields, and
; neither the allocation nor the accessing of the candidates are in main.

; Check that the array represented by field 0 of physpropmod_mp_physprop_
; can but should NOT be transposed because the indirectly
; subscripted index is not the fastest varying subscript.

; CHECK: Transpose candidate: physpropmod_mp_physprop_
; CHECK: Nested Field Number : 0
; CHECK: IsValid{{ *}}: true
; CHECK: IsProfitable{{ *}}: false

; Check that the array represented by field 1 of physpropmod_mp_physprop_
; can and should be transposed to ensure that the indirectly
; subscripted index is not the fastest varying subscript.

; CHECK: Transpose candidate: physpropmod_mp_physprop_
; CHECK: Nested Field Number : 1
; CHECK: IsValid{{ *}}: true
; CHECK: IsProfitable{{ *}}: true

; Check that the array through which the indirect subscripting is occurring is
; cannot be transposed, as transposing of simple arrays accessed
; from multiple functions is not yet supported.

; CHECK: Transpose candidate: main_$MYK
; CHECK: IsValid{{ *}}: false
; CHECK: IsProfitable{{ *}}: false

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"PHYSPROPMOD$.btPHYSPROP_TYPE" = type { %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$" }
%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@physpropmod_mp_physprop_ = internal global %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" { ptr null, i64 0, i64 0, i64 0, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@"main_$MYK" = internal global [1000 x [19 x i32]] zeroinitializer, align 16
@anon.ed4b0e6d055126a7c60c6cbe5819f596.0 = internal unnamed_addr constant i32 2

; Function Attrs: nofree noinline nounwind uwtable
define internal void @initfield0_(ptr noalias nocapture dereferenceable(288) %arg) #0 {
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
  %i17 = tail call i32 @for_allocate_handle(i64 76000, ptr nonnull %i16, i32 262144, ptr null) #6
  %i18 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %arg, i64 0, i32 1
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
  %i35 = tail call i32 @for_allocate_handle(i64 76000, ptr nonnull %i34, i32 262144, ptr null) #6
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: nofree noinline nounwind uwtable
define internal void @initfield1_(ptr noalias nocapture dereferenceable(288) %arg) #0 {
bb:
  %i = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %arg, i64 0, i32 1
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
  %i17 = tail call i32 @for_allocate_handle(i64 76000, ptr nonnull %i16, i32 262144, ptr null) #6
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
  %i35 = tail call i32 @for_allocate_handle(i64 76000, ptr nonnull %i34, i32 262144, ptr null) #6
  ret void
}

; Function Attrs: nofree noinline nounwind uwtable
define internal void @initfield2_(ptr noalias nocapture dereferenceable(288) %arg) #0 {
bb:
  tail call void @initfield1_(ptr nonnull %arg)
  ret void
}

; Function Attrs: nofree noinline nosync nounwind uwtable
define internal void @writefield_(ptr noalias nocapture readonly dereferenceable(288) %arg, ptr noalias nocapture readonly dereferenceable(4) %arg1) #2 {
bb:
  %i = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %arg, i64 0, i32 1
  %i2 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 0
  %i3 = load ptr, ptr %i2, align 1
  %i4 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 6, i64 0
  %i5 = getelementptr inbounds { i64, i64, i64 }, ptr %i4, i64 0, i32 1
  %i6 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i5, i32 0)
  %i7 = load i64, ptr %i6, align 1
  %i8 = getelementptr inbounds { i64, i64, i64 }, ptr %i4, i64 0, i32 2
  %i9 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i8, i32 0)
  %i10 = load i64, ptr %i9, align 1
  %i11 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i5, i32 1)
  %i12 = load i64, ptr %i11, align 1
  %i13 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i8, i32 1)
  %i14 = load i64, ptr %i13, align 1
  %i15 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %arg, i64 0, i32 0
  %i16 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i15, i64 0, i32 0
  %i17 = load ptr, ptr %i16, align 1
  %i18 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i15, i64 0, i32 6, i64 0
  %i19 = getelementptr inbounds { i64, i64, i64 }, ptr %i18, i64 0, i32 1
  %i20 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i19, i32 0)
  %i21 = load i64, ptr %i20, align 1
  %i22 = getelementptr inbounds { i64, i64, i64 }, ptr %i18, i64 0, i32 2
  %i23 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i22, i32 0)
  %i24 = load i64, ptr %i23, align 1
  %i25 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i19, i32 1)
  %i26 = load i64, ptr %i25, align 1
  %i27 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i22, i32 1)
  %i28 = load i64, ptr %i27, align 1
  br label %bb29

bb29:                                             ; preds = %bb55, %bb
  %i30 = phi i64 [ %i56, %bb55 ], [ 1, %bb ]
  br label %bb31

bb31:                                             ; preds = %bb52, %bb29
  %i32 = phi i64 [ %i53, %bb52 ], [ 1, %bb29 ]
  %i33 = sub nsw i64 %i30, %i32
  %i34 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 76, ptr nonnull elementtype(i32) @"main_$MYK", i64 %i32)
  %i35 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i14, i64 %i12, ptr elementtype(float) %i3, i64 %i32)
  br label %bb36

bb36:                                             ; preds = %bb36, %bb31
  %i37 = phi i64 [ %i50, %bb36 ], [ 2, %bb31 ]
  %i38 = add nsw i64 %i33, %i37
  %i39 = trunc i64 %i38 to i32
  %i40 = sitofp i32 %i39 to float
  %i41 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %i34, i64 %i37)
  %i42 = load i32, ptr %i41, align 1
  %i43 = add nsw i32 %i42, 1
  %i44 = sext i32 %i43 to i64
  %i45 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i10, i64 %i7, ptr elementtype(float) %i35, i64 %i44)
  store float %i40, ptr %i45, align 1
  %i46 = add nsw i32 %i42, -1
  %i47 = sext i32 %i46 to i64
  %i48 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i28, i64 %i26, ptr elementtype(float) %i17, i64 %i47)
  %i49 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i24, i64 %i21, ptr elementtype(float) %i48, i64 %i32)
  store float %i40, ptr %i49, align 1
  %i50 = add nuw nsw i64 %i37, 1
  %i51 = icmp eq i64 %i50, 19
  br i1 %i51, label %bb52, label %bb36

bb52:                                             ; preds = %bb36
  %i53 = add nuw nsw i64 %i32, 1
  %i54 = icmp eq i64 %i53, 1001
  br i1 %i54, label %bb55, label %bb31

bb55:                                             ; preds = %bb52
  %i56 = add nuw nsw i64 %i30, 1
  %i57 = icmp eq i64 %i56, 101
  br i1 %i57, label %bb58, label %bb29

bb58:                                             ; preds = %bb55
  ret void
}

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #3 {
bb:
  %i = tail call i32 @for_set_reentrancy(ptr nonnull @anon.ed4b0e6d055126a7c60c6cbe5819f596.0) #6
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
  store i64 288, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 1), align 8
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 4), align 8
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 2), align 8
  %i14 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %i14, align 1
  %i15 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 100, ptr %i15, align 1
  %i16 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 288, ptr %i16, align 1
  store i64 1073741829, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  %i17 = tail call i32 @for_allocate_handle(i64 28800, ptr @physpropmod_mp_physprop_, i32 262144, ptr null) #6
  br label %bb18

bb18:                                             ; preds = %bb18, %bb13
  %i19 = phi i64 [ %i32, %bb18 ], [ 1, %bb13 ]
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
  %i28 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i29 = load i64, ptr %i16, align 1
  %i30 = load i64, ptr %i14, align 1
  %i31 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i30, i64 %i29, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i28, i64 %i19)
  tail call void @writefield_(ptr %i31, ptr @"main_$MYK")
  %i32 = add nuw nsw i64 %i19, 1
  %i33 = icmp eq i64 %i32, 101
  br i1 %i33, label %bb34, label %bb18

bb34:                                             ; preds = %bb18
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: nocallback nofree nosync nounwind readnone willreturn
declare i64 @llvm.ssa.copy.i64(i64 returned) #4

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #5

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #5

attributes #0 = { nofree noinline nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nofree noinline nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #3 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #4 = { nocallback nofree nosync nounwind readnone willreturn }
attributes #5 = { nounwind readnone speculatable }
attributes #6 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}

; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers -passes='require<wholeprogram>,cgscc(inline)' -inline-report=0xe807 --whole-program-assume-read -lto-inline-cost -dtrans-inline-heuristics -intel-libirc-allowed -inline-expose-local-arrays-min-calls=2 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-BEFORE
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -inline-expose-local-arrays-min-calls=2 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-AFTER

; Check that mycopy_ was NOT inlined according to the 'Exposes local arrays'
; inline heuristic, because mycopy_ does not have enough arguments.

; CHECK-BEFORE: call{{.*}}@mycopy_
; CHECK-NOT: INLINE: mycopy_{{.*}}Exposes local arrays
; CHECK-AFTER: call{{.*}}@mycopy_

declare dso_local i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr #0

declare dso_local i32 @for_write_seq_lis_xmit(ptr nocapture readonly, ptr nocapture readonly, ptr) local_unnamed_addr #0

define dso_local void @MAIN__() local_unnamed_addr #0 {
bb:
  %i = alloca [100 x [100 x [100 x float]]], align 8
  %i1 = alloca [100 x [100 x [100 x float]]], align 8
  %i2 = getelementptr inbounds [100 x [100 x [100 x float]]], ptr %i, i64 0, i64 0, i64 0, i64 0
  %i3 = getelementptr inbounds [100 x [100 x [100 x float]]], ptr %i1, i64 0, i64 0, i64 0, i64 0
  tail call fastcc void @mycopy_(ptr %i2, ptr %i3)
  tail call fastcc void @mycopy_(ptr %i2, ptr %i3)
  ret void
}

define internal fastcc void @mycopy_(ptr noalias nocapture dereferenceable(4) %arg, ptr noalias nocapture readonly dereferenceable(4) %arg1) unnamed_addr #0 {
bb:
  %i = alloca [8 x i64], align 16
  %i2 = alloca [4 x i8], align 1
  %i3 = alloca { float }, align 8
  %i4 = alloca [4 x i8], align 1
  %i5 = alloca { float }, align 8
  br label %bb6

bb6:                                              ; preds = %bb24, %bb
  %i7 = phi i64 [ 1, %bb ], [ %i25, %bb24 ]
  br label %bb8

bb8:                                              ; preds = %bb21, %bb6
  %i9 = phi i64 [ 1, %bb6 ], [ %i22, %bb21 ]
  br label %bb10

bb10:                                             ; preds = %bb10, %bb8
  %i11 = phi i64 [ 1, %bb8 ], [ %i19, %bb10 ]
  %i12 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 40000, ptr nonnull elementtype(float) %arg1, i64 %i11)
  %i13 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 400, ptr nonnull elementtype(float) %i12, i64 %i9)
  %i14 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %i13, i64 %i7)
  %i15 = load float, ptr %i14, align 1
  %i16 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 40000, ptr nonnull elementtype(float) %arg, i64 %i11)
  %i17 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 400, ptr nonnull elementtype(float) %i16, i64 %i9)
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %i17, i64 %i7)
  store float %i15, ptr %i18, align 1
  %i19 = add nuw nsw i64 %i11, 1
  %i20 = icmp eq i64 %i19, 101
  br i1 %i20, label %bb21, label %bb10

bb21:                                             ; preds = %bb10
  %i22 = add nuw nsw i64 %i9, 1
  %i23 = icmp eq i64 %i22, 101
  br i1 %i23, label %bb24, label %bb8

bb24:                                             ; preds = %bb21
  %i25 = add nuw nsw i64 %i7, 1
  %i26 = icmp eq i64 %i25, 101
  br i1 %i26, label %bb27, label %bb6

bb27:                                             ; preds = %bb24
  %i28 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 40000, ptr nonnull elementtype(float) %arg, i64 1)
  %i29 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 400, ptr nonnull elementtype(float) %i28, i64 1)
  %i30 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %i29, i64 1)
  %i31 = load float, ptr %i30, align 1
  %i32 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 0
  store i8 26, ptr %i32, align 1
  %i33 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 1
  store i8 1, ptr %i33, align 1
  %i34 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 2
  store i8 2, ptr %i34, align 1
  %i35 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 3
  store i8 0, ptr %i35, align 1
  %i36 = getelementptr inbounds { float }, ptr %i3, i64 0, i32 0
  store float %i31, ptr %i36, align 8
  %i39 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i, i32 -1, i64 1239157112576, ptr nonnull %i32, ptr nonnull %i3)
  %i40 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 40000, ptr nonnull elementtype(float) %arg1, i64 1)
  %i41 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 400, ptr nonnull elementtype(float) %i40, i64 1)
  %i42 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %i41, i64 1)
  %i43 = load float, ptr %i42, align 1
  %i44 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 0
  store i8 26, ptr %i44, align 1
  %i45 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 1
  store i8 1, ptr %i45, align 1
  %i46 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 2
  store i8 1, ptr %i46, align 1
  %i47 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 3
  store i8 0, ptr %i47, align 1
  %i48 = getelementptr inbounds { float }, ptr %i5, i64 0, i32 0
  store float %i43, ptr %i48, align 8
  %i50 = call i32 @for_write_seq_lis_xmit(ptr nonnull %i, ptr nonnull %i44, ptr nonnull %i5)
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { "intel-lang"="fortran" }
attributes #1 = { nounwind readnone speculatable }
; end INTEL_FEATURE_SW_ADVANCED

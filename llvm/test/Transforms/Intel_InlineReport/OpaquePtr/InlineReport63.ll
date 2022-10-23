; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -opaque-pointers -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe807 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-BEFORE
; Inline report via metadata
; RUN: opt -opaque-pointers -inlinereportsetup -inline-report=0xe886 < %s -S | opt -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe886 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-AFTER

; Check that all no instances of @daxpy are inlined, because they fail to pass
; the matching actual parameter test for the callsites.

; CHECK-BEFORE: call{{.*}}daxpy_
; CHECK-BEFORE: call{{.*}}daxpy_
; CHECK-BEFORE: call{{.*}}daxpy_
; CHECK-NOT: INLINE: daxpy_{{.*}}Has inline budget for small application
; CHECK-NOT: INLINE: daxpy_{{.*}}Callee has single callsite and local linkage
; CHECK-AFTER: call{{.*}}daxpy_
; CHECK-AFTER: call{{.*}}daxpy_
; CHECK-AFTER: call{{.*}}daxpy_

@anon.179c04c108271c6ee1aba768bef6092b.0 = internal unnamed_addr constant i32 2
@"linpk_$A" = internal unnamed_addr global [2500 x [2501 x double]] zeroinitializer, align 16
@"linpk_$B" = internal unnamed_addr global [2500 x double] zeroinitializer, align 16
@"linpk_$IPVT" = internal unnamed_addr global [2500 x i32] zeroinitializer, align 16
@"linpk_$X" = internal unnamed_addr global [2500 x double] zeroinitializer, align 16
@"linpk_$format_pack" = internal unnamed_addr global [120 x i8] c"6\00\00\00\0A\00\00\00\01\00\00\00\01\00\00\00\1E\00\00\08\05\00\00\00\10\00\00\007\00\00\006\00\00\00\1C\00,\00     norm. resid      resid           machep\1C\00\1B\00         x(1)          x(n)\007\00\00\00", align 4

define internal fastcc void @dscal_(ptr noalias nocapture readonly %arg, ptr noalias nocapture readonly %arg1, ptr noalias nocapture %arg2) unnamed_addr #0 {
bb:
  %i = load i32, ptr %arg, align 1
  %i3 = icmp sgt i32 %i, 0
  br i1 %i3, label %bb47, label %bb50

bb4:                                              ; preds = %bb43, %bb4
  %i5 = phi i64 [ 1, %bb43 ], [ %i9, %bb4 ]
  %i6 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg2, i64 %i5)
  %i7 = load double, ptr %i6, align 1
  %i8 = fmul fast double %i7, %i44
  store double %i8, ptr %i6, align 1
  %i9 = add nuw nsw i64 %i5, 1
  %i10 = icmp eq i64 %i9, %i46
  br i1 %i10, label %bb11, label %bb4

bb11:                                             ; preds = %bb4
  %i12 = icmp sgt i32 %i, 4
  %i13 = icmp sgt i32 %i, %i48
  %i14 = and i1 %i12, %i13
  br i1 %i14, label %bb15, label %bb50

bb15:                                             ; preds = %bb47, %bb11
  %i16 = load double, ptr %arg1, align 1
  %i17 = add nuw nsw i32 %i48, 1
  %i18 = zext i32 %i17 to i64
  br label %bb19

bb19:                                             ; preds = %bb19, %bb15
  %i20 = phi i64 [ %i18, %bb15 ], [ %i40, %bb19 ]
  %i21 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg2, i64 %i20)
  %i22 = load double, ptr %i21, align 1
  %i23 = fmul fast double %i22, %i16
  store double %i23, ptr %i21, align 1
  %i24 = add nuw nsw i64 %i20, 1
  %i25 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg2, i64 %i24)
  %i26 = load double, ptr %i25, align 1
  %i27 = fmul fast double %i26, %i16
  store double %i27, ptr %i25, align 1
  %i28 = add nuw nsw i64 %i20, 2
  %i29 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg2, i64 %i28)
  %i30 = load double, ptr %i29, align 1
  %i31 = fmul fast double %i30, %i16
  store double %i31, ptr %i29, align 1
  %i32 = add nuw nsw i64 %i20, 3
  %i33 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg2, i64 %i32)
  %i34 = load double, ptr %i33, align 1
  %i35 = fmul fast double %i34, %i16
  store double %i35, ptr %i33, align 1
  %i36 = add nuw nsw i64 %i20, 4
  %i37 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg2, i64 %i36)
  %i38 = load double, ptr %i37, align 1
  %i39 = fmul fast double %i38, %i16
  store double %i39, ptr %i37, align 1
  %i40 = add nuw i64 %i20, 5
  %i41 = trunc i64 %i40 to i32
  %i42 = icmp slt i32 %i, %i41
  br i1 %i42, label %bb50, label %bb19

bb43:                                             ; preds = %bb47
  %i44 = load double, ptr %arg1, align 1
  %i45 = add nuw nsw i32 %i48, 1
  %i46 = zext i32 %i45 to i64
  br label %bb4

bb47:                                             ; preds = %bb
  %i48 = urem i32 %i, 5
  %i49 = icmp eq i32 %i48, 0
  br i1 %i49, label %bb15, label %bb43

bb50:                                             ; preds = %bb19, %bb11, %bb
  ret void
}

define internal fastcc void @daxpy_(ptr noalias nocapture readonly %arg, ptr noalias nocapture readonly %arg1, ptr noalias nocapture readonly %arg2, ptr noalias nocapture %arg3) unnamed_addr #0 {
bb:
  %i = load i32, ptr %arg, align 1
  %i4 = icmp sgt i32 %i, 0
  br i1 %i4, label %bb60, label %bb63

bb5:                                              ; preds = %bb54, %bb5
  %i6 = phi i64 [ 1, %bb54 ], [ %i13, %bb5 ]
  %i7 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg3, i64 %i6)
  %i8 = load double, ptr %i7, align 1
  %i9 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg2, i64 %i6)
  %i10 = load double, ptr %i9, align 1
  %i11 = fmul fast double %i10, %i61
  %i12 = fadd fast double %i11, %i8
  store double %i12, ptr %i7, align 1
  %i13 = add nuw nsw i64 %i6, 1
  %i14 = icmp eq i64 %i13, %i56
  br i1 %i14, label %bb15, label %bb5

bb15:                                             ; preds = %bb5
  %i16 = icmp slt i32 %i, 4
  br i1 %i16, label %bb63, label %bb19

bb17:                                             ; preds = %bb57
  %i18 = icmp sgt i32 %i, 3
  br i1 %i18, label %bb19, label %bb63

bb19:                                             ; preds = %bb17, %bb15
  %i20 = add nuw nsw i32 %i58, 1
  %i21 = zext i32 %i20 to i64
  %i22 = zext i32 %i to i64
  br label %bb23

bb23:                                             ; preds = %bb23, %bb19
  %i24 = phi i64 [ %i21, %bb19 ], [ %i52, %bb23 ]
  %i25 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg3, i64 %i24)
  %i26 = load double, ptr %i25, align 1
  %i27 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg2, i64 %i24)
  %i28 = load double, ptr %i27, align 1
  %i29 = fmul fast double %i28, %i61
  %i30 = fadd fast double %i29, %i26
  store double %i30, ptr %i25, align 1
  %i31 = add nuw nsw i64 %i24, 1
  %i32 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg3, i64 %i31)
  %i33 = load double, ptr %i32, align 1
  %i34 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg2, i64 %i31)
  %i35 = load double, ptr %i34, align 1
  %i36 = fmul fast double %i35, %i61
  %i37 = fadd fast double %i36, %i33
  store double %i37, ptr %i32, align 1
  %i38 = add nuw nsw i64 %i24, 2
  %i39 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg3, i64 %i38)
  %i40 = load double, ptr %i39, align 1
  %i41 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg2, i64 %i38)
  %i42 = load double, ptr %i41, align 1
  %i43 = fmul fast double %i42, %i61
  %i44 = fadd fast double %i43, %i40
  store double %i44, ptr %i39, align 1
  %i45 = add nuw nsw i64 %i24, 3
  %i46 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg3, i64 %i45)
  %i47 = load double, ptr %i46, align 1
  %i48 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg2, i64 %i45)
  %i49 = load double, ptr %i48, align 1
  %i50 = fmul fast double %i49, %i61
  %i51 = fadd fast double %i50, %i47
  store double %i51, ptr %i46, align 1
  %i52 = add nuw nsw i64 %i24, 4
  %i53 = icmp ugt i64 %i52, %i22
  br i1 %i53, label %bb63, label %bb23

bb54:                                             ; preds = %bb57
  %i55 = add nuw nsw i32 %i58, 1
  %i56 = zext i32 %i55 to i64
  br label %bb5

bb57:                                             ; preds = %bb60
  %i58 = and i32 %i, 3
  %i59 = icmp eq i32 %i58, 0
  br i1 %i59, label %bb17, label %bb54

bb60:                                             ; preds = %bb
  %i61 = load double, ptr %arg1, align 1
  %i62 = fcmp fast ueq double %i61, 0.000000e+00
  br i1 %i62, label %bb63, label %bb57

bb63:                                             ; preds = %bb60, %bb23, %bb17, %bb15, %bb
  ret void
}

define internal fastcc void @dgefa_(ptr noalias nocapture %arg) unnamed_addr #0 {
bb:
  %i = alloca double, align 8
  %i1 = alloca i32, align 4
  %i2 = alloca i32, align 4
  store i32 0, ptr %arg, align 1
  br label %bb4

bb4:                                              ; preds = %bb81, %bb
  %i5 = phi i64 [ 1, %bb ], [ %i7, %bb81 ]
  %i6 = phi i64 [ 2, %bb ], [ %i84, %bb81 ]
  %i7 = add nuw i64 %i5, 1
  %i8 = trunc i64 %i5 to i32
  %i9 = sub nsw i32 2500, %i8
  %i10 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i5)
  %i11 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i10, i64 %i5)
  %i12 = icmp slt i32 %i8, 2501
  br i1 %i12, label %bb33, label %bb35

bb13:                                             ; preds = %bb26, %bb13
  %i14 = phi i64 [ 2, %bb26 ], [ %i24, %bb13 ]
  %i15 = phi double [ %i29, %bb26 ], [ %i21, %bb13 ]
  %i16 = phi i32 [ 1, %bb26 ], [ %i23, %bb13 ]
  %i17 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %i11, i64 %i14)
  %i18 = load double, ptr %i17, align 1
  %i19 = tail call fast double @llvm.fabs.f64(double %i18)
  %i20 = fcmp fast ogt double %i19, %i15
  %i21 = select i1 %i20, double %i19, double %i15
  %i22 = trunc i64 %i14 to i32
  %i23 = select i1 %i20, i32 %i22, i32 %i16
  %i24 = add nuw nsw i64 %i14, 1
  %i25 = icmp eq i64 %i24, %i32
  br i1 %i25, label %bb35, label %bb13

bb26:                                             ; preds = %bb33
  %i27 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i11, i64 1)
  %i28 = load double, ptr %i27, align 1
  %i29 = tail call fast double @llvm.fabs.f64(double %i28)
  %i30 = shl i64 %i5, 32
  %i31 = sub i64 10746008174592, %i30
  %i32 = ashr exact i64 %i31, 32
  br label %bb13

bb33:                                             ; preds = %bb4
  %i34 = icmp eq i32 %i9, 0
  br i1 %i34, label %bb35, label %bb26

bb35:                                             ; preds = %bb33, %bb13, %bb4
  %i36 = phi i32 [ 1, %bb33 ], [ 0, %bb4 ], [ %i23, %bb13 ]
  %i37 = add i32 %i8, -1
  %i38 = add i32 %i37, %i36
  %i39 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) @"linpk_$IPVT", i64 %i5)
  store i32 %i38, ptr %i39, align 1
  %i40 = sext i32 %i38 to i64
  %i41 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i10, i64 %i40)
  %i42 = load double, ptr %i41, align 1
  %i43 = fcmp fast oeq double %i42, 0.000000e+00
  br i1 %i43, label %bb77, label %bb78

bb44:                                             ; preds = %bb78
  store double %i42, ptr %i, align 8
  %i46 = load i64, ptr %i11, align 1
  store i64 %i46, ptr %i41, align 1
  store double %i42, ptr %i11, align 1
  br label %bb48

bb48:                                             ; preds = %bb78, %bb44
  %i49 = load double, ptr %i11, align 1
  %i50 = fdiv fast double -1.000000e+00, %i49
  store double %i50, ptr %i, align 8
  %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %i10, i64 %i7)
  store i32 %i9, ptr %i1, align 4
  call fastcc void @dscal_(ptr nonnull %i1, ptr nonnull %i, ptr %i51)
  %i52 = icmp slt i64 %i5, 2500
  br i1 %i52, label %bb53, label %bb81

bb53:                                             ; preds = %bb48
  br i1 %i80, label %bb54, label %bb67

bb54:                                             ; preds = %bb54, %bb53
  %i55 = phi i64 [ %i64, %bb54 ], [ %i6, %bb53 ]
  %i56 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr nonnull elementtype(double) @"linpk_$A", i64 %i55)
  %i57 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i56, i64 %i40)
  %i59 = load i64, ptr %i57, align 1
  store i64 %i59, ptr %i, align 8
  %i60 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i56, i64 %i5)
  %i62 = load i64, ptr %i60, align 1
  store i64 %i62, ptr %i57, align 1
  store i64 %i59, ptr %i60, align 1
  %i63 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i56, i64 %i7)
  store i32 %i9, ptr %i2, align 4
  call fastcc void @daxpy_(ptr nonnull %i1, ptr nonnull %i, ptr nonnull %i, ptr %i63)
  %i64 = add i64 %i55, 1
  %i65 = trunc i64 %i64 to i32
  %i66 = icmp sgt i32 %i65, 2500
  br i1 %i66, label %bb81, label %bb54

bb67:                                             ; preds = %bb67, %bb53
  %i68 = phi i64 [ %i74, %bb67 ], [ %i6, %bb53 ]
  %i69 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr nonnull elementtype(double) @"linpk_$A", i64 %i68)
  %i70 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i69, i64 %i40)
  %i72 = load i64, ptr %i70, align 1
  store i64 %i72, ptr %i, align 8
  %i73 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i69, i64 %i7)
  store i32 %i9, ptr %i2, align 4
  call fastcc void @daxpy_(ptr nonnull %i2, ptr nonnull %i, ptr nonnull %i51, ptr %i73)
  %i74 = add i64 %i68, 1
  %i75 = trunc i64 %i74 to i32
  %i76 = icmp sgt i32 %i75, 2500
  br i1 %i76, label %bb81, label %bb67

bb77:                                             ; preds = %bb35
  store i32 %i8, ptr %arg, align 1
  br label %bb81

bb78:                                             ; preds = %bb35
  %i79 = zext i32 %i38 to i64
  %i80 = icmp ne i64 %i5, %i79
  br i1 %i80, label %bb44, label %bb48

bb81:                                             ; preds = %bb77, %bb67, %bb54, %bb48
  %i82 = trunc i64 %i7 to i32
  %i83 = icmp slt i32 %i82, 2500
  %i84 = add i64 %i6, 1
  br i1 %i83, label %bb4, label %bb85

bb85:                                             ; preds = %bb81
  %i86 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) @"linpk_$IPVT", i64 2500)
  store i32 2500, ptr %i86, align 1
  %i87 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 2500)
  %i88 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i87, i64 2500)
  %i89 = load double, ptr %i88, align 1
  %i90 = fcmp fast oeq double %i89, 0.000000e+00
  br i1 %i90, label %bb91, label %bb92

bb91:                                             ; preds = %bb85
  store i32 2500, ptr %arg, align 1
  br label %bb92

bb92:                                             ; preds = %bb91, %bb85
  ret void
}

define internal fastcc void @dgesl_() unnamed_addr #0 {
bb:
  %i = alloca double, align 8
  %i1 = alloca i32, align 4
  %i2 = alloca i32, align 4
  br label %bb4

bb4:                                              ; preds = %bb18, %bb
  %i5 = phi i64 [ 1, %bb ], [ %i21, %bb18 ]
  %i6 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) @"linpk_$IPVT", i64 %i5)
  %i7 = load i32, ptr %i6, align 1
  %i8 = sext i32 %i7 to i64
  %i9 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$B", i64 %i8)
  %i11 = load i64, ptr %i9, align 1
  store i64 %i11, ptr %i, align 8
  %i12 = zext i32 %i7 to i64
  %i13 = icmp eq i64 %i5, %i12
  br i1 %i13, label %bb18, label %bb14

bb14:                                             ; preds = %bb4
  %i15 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) @"linpk_$B", i64 %i5)
  %i17 = load i64, ptr %i15, align 1
  store i64 %i17, ptr %i9, align 1
  store i64 %i11, ptr %i15, align 1
  br label %bb18

bb18:                                             ; preds = %bb14, %bb4
  %i19 = trunc i64 %i5 to i32
  %i20 = sub nsw i32 2500, %i19
  %i21 = add nuw i64 %i5, 1
  %i22 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i5)
  %i23 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i22, i64 %i21)
  %i24 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) @"linpk_$B", i64 %i21)
  store i32 %i20, ptr %i1, align 4
  call fastcc void @daxpy_(ptr nonnull %i1, ptr nonnull %i23, ptr %i23, ptr %i24)
  %i25 = trunc i64 %i21 to i32
  %i26 = icmp slt i32 %i25, 2500
  br i1 %i26, label %bb4, label %bb27

bb27:                                             ; preds = %bb18
  %i28 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$B", i64 1)
  br label %bb29

bb29:                                             ; preds = %bb29, %bb27
  %i30 = phi i64 [ 1, %bb27 ], [ %i44, %bb29 ]
  %i31 = trunc i64 %i30 to i32
  %i32 = shl i64 %i30, 32
  %i33 = sub i64 10741713207296, %i32
  %i34 = ashr exact i64 %i33, 32
  %i35 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$B", i64 %i34)
  %i36 = load double, ptr %i35, align 1
  %i37 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i34)
  %i38 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i37, i64 %i34)
  %i39 = load double, ptr %i38, align 1
  %i40 = fdiv fast double %i36, %i39
  store double %i40, ptr %i35, align 1
  %i41 = fneg fast double %i40
  store double %i41, ptr %i, align 8
  %i42 = sub i32 2500, %i31
  %i43 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i37, i64 1)
  store i32 %i42, ptr %i2, align 4
  call fastcc void @daxpy_(ptr nonnull %i2, ptr nonnull %i, ptr %i43, ptr %i28)
  %i44 = add nuw i64 %i30, 1
  %i45 = icmp sgt i64 %i44, 2500
  br i1 %i45, label %bb46, label %bb29

bb46:                                             ; preds = %bb29
  ret void
}

define internal fastcc void @dmxpy_() unnamed_addr {
bb:
  %i = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 1)
  %i1 = load double, ptr %i, align 1
  %i2 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 1)
  %i3 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 2)
  %i4 = load double, ptr %i3, align 1
  %i5 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 2)
  %i6 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 3)
  %i7 = load double, ptr %i6, align 1
  %i8 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 3)
  %i9 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 4)
  %i10 = load double, ptr %i9, align 1
  %i11 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 4)
  br label %bb12

bb12:                                             ; preds = %bb12, %bb
  %i13 = phi i64 [ 1, %bb ], [ %i32, %bb12 ]
  %i14 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$B", i64 %i13)
  %i15 = load double, ptr %i14, align 1
  %i16 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i2, i64 %i13)
  %i17 = load double, ptr %i16, align 1
  %i18 = fmul fast double %i17, %i1
  %i19 = fadd fast double %i18, %i15
  %i20 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i5, i64 %i13)
  %i21 = load double, ptr %i20, align 1
  %i22 = fmul fast double %i21, %i4
  %i23 = fadd fast double %i19, %i22
  %i24 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i8, i64 %i13)
  %i25 = load double, ptr %i24, align 1
  %i26 = fmul fast double %i25, %i7
  %i27 = fadd fast double %i23, %i26
  %i28 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i11, i64 %i13)
  %i29 = load double, ptr %i28, align 1
  %i30 = fmul fast double %i29, %i10
  %i31 = fadd fast double %i27, %i30
  store double %i31, ptr %i14, align 1
  %i32 = add nuw nsw i64 %i13, 1
  %i33 = icmp eq i64 %i32, 2501
  br i1 %i33, label %bb34, label %bb12

bb34:                                             ; preds = %bb169, %bb12
  %i35 = phi i64 [ %i170, %bb169 ], [ 20, %bb12 ]
  %i36 = add nsw i64 %i35, -15
  %i37 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 %i36)
  %i38 = load double, ptr %i37, align 1
  %i39 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i36)
  %i40 = add nsw i64 %i35, -14
  %i41 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 %i40)
  %i42 = load double, ptr %i41, align 1
  %i43 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i40)
  %i44 = add nsw i64 %i35, -13
  %i45 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 %i44)
  %i46 = load double, ptr %i45, align 1
  %i47 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i44)
  %i48 = add nsw i64 %i35, -12
  %i49 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 %i48)
  %i50 = load double, ptr %i49, align 1
  %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i48)
  %i52 = add nsw i64 %i35, -11
  %i53 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 %i52)
  %i54 = load double, ptr %i53, align 1
  %i55 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i52)
  %i56 = add nsw i64 %i35, -10
  %i57 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 %i56)
  %i58 = load double, ptr %i57, align 1
  %i59 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i56)
  %i60 = add nsw i64 %i35, -9
  %i61 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 %i60)
  %i62 = load double, ptr %i61, align 1
  %i63 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i60)
  %i64 = add nsw i64 %i35, -8
  %i65 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 %i64)
  %i66 = load double, ptr %i65, align 1
  %i67 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i64)
  %i68 = add nsw i64 %i35, -7
  %i69 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 %i68)
  %i70 = load double, ptr %i69, align 1
  %i71 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i68)
  %i72 = add nsw i64 %i35, -6
  %i73 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 %i72)
  %i74 = load double, ptr %i73, align 1
  %i75 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i72)
  %i76 = add nsw i64 %i35, -5
  %i77 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 %i76)
  %i78 = load double, ptr %i77, align 1
  %i79 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i76)
  %i80 = add nsw i64 %i35, -4
  %i81 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 %i80)
  %i82 = load double, ptr %i81, align 1
  %i83 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i80)
  %i84 = add nsw i64 %i35, -3
  %i85 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 %i84)
  %i86 = load double, ptr %i85, align 1
  %i87 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i84)
  %i88 = add nsw i64 %i35, -2
  %i89 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 %i88)
  %i90 = load double, ptr %i89, align 1
  %i91 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i88)
  %i92 = add nsw i64 %i35, -1
  %i93 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 %i92)
  %i94 = load double, ptr %i93, align 1
  %i95 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i92)
  %i96 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 %i35)
  %i97 = load double, ptr %i96, align 1
  %i98 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i35)
  br label %bb99

bb99:                                             ; preds = %bb99, %bb34
  %i100 = phi i64 [ 1, %bb34 ], [ %i167, %bb99 ]
  %i101 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$B", i64 %i100)
  %i102 = load double, ptr %i101, align 1
  %i103 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i39, i64 %i100)
  %i104 = load double, ptr %i103, align 1
  %i105 = fmul fast double %i104, %i38
  %i106 = fadd fast double %i105, %i102
  %i107 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i43, i64 %i100)
  %i108 = load double, ptr %i107, align 1
  %i109 = fmul fast double %i108, %i42
  %i110 = fadd fast double %i106, %i109
  %i111 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i47, i64 %i100)
  %i112 = load double, ptr %i111, align 1
  %i113 = fmul fast double %i112, %i46
  %i114 = fadd fast double %i110, %i113
  %i115 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i51, i64 %i100)
  %i116 = load double, ptr %i115, align 1
  %i117 = fmul fast double %i116, %i50
  %i118 = fadd fast double %i114, %i117
  %i119 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i55, i64 %i100)
  %i120 = load double, ptr %i119, align 1
  %i121 = fmul fast double %i120, %i54
  %i122 = fadd fast double %i118, %i121
  %i123 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i59, i64 %i100)
  %i124 = load double, ptr %i123, align 1
  %i125 = fmul fast double %i124, %i58
  %i126 = fadd fast double %i122, %i125
  %i127 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i63, i64 %i100)
  %i128 = load double, ptr %i127, align 1
  %i129 = fmul fast double %i128, %i62
  %i130 = fadd fast double %i126, %i129
  %i131 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i67, i64 %i100)
  %i132 = load double, ptr %i131, align 1
  %i133 = fmul fast double %i132, %i66
  %i134 = fadd fast double %i130, %i133
  %i135 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i71, i64 %i100)
  %i136 = load double, ptr %i135, align 1
  %i137 = fmul fast double %i136, %i70
  %i138 = fadd fast double %i134, %i137
  %i139 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i75, i64 %i100)
  %i140 = load double, ptr %i139, align 1
  %i141 = fmul fast double %i140, %i74
  %i142 = fadd fast double %i138, %i141
  %i143 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i79, i64 %i100)
  %i144 = load double, ptr %i143, align 1
  %i145 = fmul fast double %i144, %i78
  %i146 = fadd fast double %i142, %i145
  %i147 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i83, i64 %i100)
  %i148 = load double, ptr %i147, align 1
  %i149 = fmul fast double %i148, %i82
  %i150 = fadd fast double %i146, %i149
  %i151 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i87, i64 %i100)
  %i152 = load double, ptr %i151, align 1
  %i153 = fmul fast double %i152, %i86
  %i154 = fadd fast double %i150, %i153
  %i155 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i91, i64 %i100)
  %i156 = load double, ptr %i155, align 1
  %i157 = fmul fast double %i156, %i90
  %i158 = fadd fast double %i154, %i157
  %i159 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i95, i64 %i100)
  %i160 = load double, ptr %i159, align 1
  %i161 = fmul fast double %i160, %i94
  %i162 = fadd fast double %i158, %i161
  %i163 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i98, i64 %i100)
  %i164 = load double, ptr %i163, align 1
  %i165 = fmul fast double %i164, %i97
  %i166 = fadd fast double %i162, %i165
  store double %i166, ptr %i101, align 1
  %i167 = add nuw nsw i64 %i100, 1
  %i168 = icmp eq i64 %i167, 2501
  br i1 %i168, label %bb169, label %bb99

bb169:                                            ; preds = %bb99
  %i170 = add nuw nsw i64 %i35, 16
  %i171 = icmp ugt i64 %i35, 2484
  br i1 %i171, label %bb172, label %bb34

bb172:                                            ; preds = %bb169
  ret void
}

define dso_local void @MAIN__() local_unnamed_addr #0 {
bb:
  %i = alloca [8 x i64], align 16
  %i1 = alloca i32, align 8
  %i2 = alloca [2 x i8], align 1
  %i3 = alloca [4 x i8], align 1
  %i4 = alloca { double }, align 8
  %i5 = alloca [4 x i8], align 1
  %i6 = alloca { double }, align 8
  %i7 = alloca [4 x i8], align 1
  %i8 = alloca { double }, align 8
  %i9 = alloca [4 x i8], align 1
  %i10 = alloca { double }, align 8
  %i11 = alloca [4 x i8], align 1
  %i12 = alloca { double }, align 8
  %i13 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.179c04c108271c6ee1aba768bef6092b.0)
  br label %bb14

bb14:                                             ; preds = %bb30, %bb
  %i15 = phi i64 [ 1, %bb ], [ %i31, %bb30 ]
  %i16 = phi i32 [ 1325, %bb ], [ %i22, %bb30 ]
  %i17 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i15)
  br label %bb18

bb18:                                             ; preds = %bb18, %bb14
  %i19 = phi i64 [ 1, %bb14 ], [ %i28, %bb18 ]
  %i20 = phi i32 [ %i16, %bb14 ], [ %i22, %bb18 ]
  %i21 = mul nsw i32 %i20, 3125
  %i22 = srem i32 %i21, 65536
  %i23 = sitofp i32 %i22 to float
  %i24 = fmul fast float %i23, 0x3F10000000000000
  %i25 = fadd fast float %i24, -2.000000e+00
  %i26 = fpext float %i25 to double
  %i27 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i17, i64 %i19)
  store double %i26, ptr %i27, align 1
  %i28 = add nuw nsw i64 %i19, 1
  %i29 = icmp eq i64 %i28, 2501
  br i1 %i29, label %bb30, label %bb18

bb30:                                             ; preds = %bb18
  %i31 = add nuw nsw i64 %i15, 1
  %i32 = icmp eq i64 %i31, 2501
  br i1 %i32, label %bb33, label %bb14

bb33:                                             ; preds = %bb33, %bb30
  %i34 = phi i64 [ %i36, %bb33 ], [ 1, %bb30 ]
  %i35 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$B", i64 %i34)
  store double 0.000000e+00, ptr %i35, align 1
  %i36 = add nuw nsw i64 %i34, 1
  %i37 = icmp eq i64 %i36, 2501
  br i1 %i37, label %bb38, label %bb33

bb38:                                             ; preds = %bb50, %bb33
  %i39 = phi i64 [ %i51, %bb50 ], [ 1, %bb33 ]
  %i40 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i39)
  br label %bb41

bb41:                                             ; preds = %bb41, %bb38
  %i42 = phi i64 [ 1, %bb38 ], [ %i48, %bb41 ]
  %i43 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) @"linpk_$B", i64 %i42)
  %i44 = load double, ptr %i43, align 1
  %i45 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i40, i64 %i42)
  %i46 = load double, ptr %i45, align 1
  %i47 = fadd fast double %i46, %i44
  store double %i47, ptr %i43, align 1
  %i48 = add nuw nsw i64 %i42, 1
  %i49 = icmp eq i64 %i48, 2501
  br i1 %i49, label %bb50, label %bb41

bb50:                                             ; preds = %bb41
  %i51 = add nuw nsw i64 %i39, 1
  %i52 = icmp eq i64 %i51, 2501
  br i1 %i52, label %bb53, label %bb38

bb53:                                             ; preds = %bb50
  call fastcc void @dgefa_(ptr nonnull %i1)
  tail call fastcc void @dgesl_()
  br label %bb54

bb54:                                             ; preds = %bb54, %bb53
  %i55 = phi i64 [ %i61, %bb54 ], [ 1, %bb53 ]
  %i56 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$B", i64 %i55)
  %i58 = load i64, ptr %i56, align 1
  %i59 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 %i55)
  store i64 %i58, ptr %i59, align 1
  %i61 = add nuw nsw i64 %i55, 1
  %i62 = icmp eq i64 %i61, 2501
  br i1 %i62, label %bb63, label %bb54

bb63:                                             ; preds = %bb83, %bb54
  %i64 = phi double [ %i80, %bb83 ], [ 0.000000e+00, %bb54 ]
  %i65 = phi i64 [ %i84, %bb83 ], [ 1, %bb54 ]
  %i66 = phi i32 [ %i73, %bb83 ], [ 1325, %bb54 ]
  %i67 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i65)
  br label %bb68

bb68:                                             ; preds = %bb68, %bb63
  %i69 = phi i64 [ 1, %bb63 ], [ %i81, %bb68 ]
  %i70 = phi double [ %i64, %bb63 ], [ %i80, %bb68 ]
  %i71 = phi i32 [ %i66, %bb63 ], [ %i73, %bb68 ]
  %i72 = mul nsw i32 %i71, 3125
  %i73 = srem i32 %i72, 65536
  %i74 = sitofp i32 %i73 to float
  %i75 = fmul fast float %i74, 0x3F10000000000000
  %i76 = fadd fast float %i75, -2.000000e+00
  %i77 = fpext float %i76 to double
  %i78 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i67, i64 %i69)
  store double %i77, ptr %i78, align 1
  %i79 = fcmp fast ole double %i70, %i77
  %i80 = select fast i1 %i79, double %i77, double %i70
  %i81 = add nuw nsw i64 %i69, 1
  %i82 = icmp eq i64 %i81, 2501
  br i1 %i82, label %bb83, label %bb68

bb83:                                             ; preds = %bb68
  %i84 = add nuw nsw i64 %i65, 1
  %i85 = icmp eq i64 %i84, 2501
  br i1 %i85, label %bb86, label %bb63

bb86:                                             ; preds = %bb86, %bb83
  %i87 = phi i64 [ %i89, %bb86 ], [ 1, %bb83 ]
  %i88 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$B", i64 %i87)
  store double 0.000000e+00, ptr %i88, align 1
  %i89 = add nuw nsw i64 %i87, 1
  %i90 = icmp eq i64 %i89, 2501
  br i1 %i90, label %bb91, label %bb86

bb91:                                             ; preds = %bb103, %bb86
  %i92 = phi i64 [ %i104, %bb103 ], [ 1, %bb86 ]
  %i93 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 20008, ptr elementtype(double) @"linpk_$A", i64 %i92)
  br label %bb94

bb94:                                             ; preds = %bb94, %bb91
  %i95 = phi i64 [ 1, %bb91 ], [ %i101, %bb94 ]
  %i96 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) @"linpk_$B", i64 %i95)
  %i97 = load double, ptr %i96, align 1
  %i98 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i93, i64 %i95)
  %i99 = load double, ptr %i98, align 1
  %i100 = fadd fast double %i99, %i97
  store double %i100, ptr %i96, align 1
  %i101 = add nuw nsw i64 %i95, 1
  %i102 = icmp eq i64 %i101, 2501
  br i1 %i102, label %bb103, label %bb94

bb103:                                            ; preds = %bb94
  %i104 = add nuw nsw i64 %i92, 1
  %i105 = icmp eq i64 %i104, 2501
  br i1 %i105, label %bb106, label %bb91

bb106:                                            ; preds = %bb106, %bb103
  %i107 = phi i64 [ %i111, %bb106 ], [ 1, %bb103 ]
  %i108 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$B", i64 %i107)
  %i109 = load double, ptr %i108, align 1
  %i110 = fneg fast double %i109
  store double %i110, ptr %i108, align 1
  %i111 = add nuw nsw i64 %i107, 1
  %i112 = icmp eq i64 %i111, 2501
  br i1 %i112, label %bb113, label %bb106

bb113:                                            ; preds = %bb106
  tail call fastcc void @dmxpy_()
  br label %bb114

bb114:                                            ; preds = %bb114, %bb113
  %i115 = phi i64 [ %i126, %bb114 ], [ 1, %bb113 ]
  %i116 = phi double [ %i121, %bb114 ], [ 0.000000e+00, %bb113 ]
  %i117 = phi double [ %i125, %bb114 ], [ 0.000000e+00, %bb113 ]
  %i118 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$B", i64 %i115)
  %i119 = load double, ptr %i118, align 1
  %i120 = tail call fast double @llvm.fabs.f64(double %i119)
  %i121 = tail call fast double @llvm.maxnum.f64(double %i116, double %i120)
  %i122 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 %i115)
  %i123 = load double, ptr %i122, align 1
  %i124 = tail call fast double @llvm.fabs.f64(double %i123)
  %i125 = tail call fast double @llvm.maxnum.f64(double %i117, double %i124)
  %i126 = add nuw nsw i64 %i115, 1
  %i127 = icmp eq i64 %i126, 2501
  br i1 %i127, label %bb128, label %bb114

bb128:                                            ; preds = %bb114
  %i129 = fmul fast double %i80, 0x3D63880000000000
  %i130 = fmul fast double %i129, %i125
  %i131 = fdiv fast double %i121, %i130
  %i132 = getelementptr inbounds [2 x i8], ptr %i2, i64 0, i64 0
  store i8 1, ptr %i132, align 1
  %i133 = getelementptr inbounds [2 x i8], ptr %i2, i64 0, i64 1
  store i8 0, ptr %i133, align 1
  %i135 = call i32 (ptr, i32, i64, ptr, ptr, ptr, ...) @for_write_seq_fmt(ptr nonnull %i, i32 -1, i64 1239157112576, ptr nonnull %i132, ptr null, ptr getelementptr inbounds ([120 x i8], ptr @"linpk_$format_pack", i64 0, i64 32))
  %i136 = getelementptr inbounds [4 x i8], ptr %i3, i64 0, i64 0
  store i8 48, ptr %i136, align 1
  %i137 = getelementptr inbounds [4 x i8], ptr %i3, i64 0, i64 1
  store i8 1, ptr %i137, align 1
  %i138 = getelementptr inbounds [4 x i8], ptr %i3, i64 0, i64 2
  store i8 2, ptr %i138, align 1
  %i139 = getelementptr inbounds [4 x i8], ptr %i3, i64 0, i64 3
  store i8 0, ptr %i139, align 1
  %i140 = getelementptr inbounds { double }, ptr %i4, i64 0, i32 0
  store double %i131, ptr %i140, align 8
  %i142 = call i32 (ptr, i32, i64, ptr, ptr, ptr, ...) @for_write_seq_fmt(ptr nonnull %i, i32 -1, i64 1239157112576, ptr nonnull %i136, ptr nonnull %i4, ptr @"linpk_$format_pack")
  %i143 = getelementptr inbounds [4 x i8], ptr %i5, i64 0, i64 0
  store i8 48, ptr %i143, align 1
  %i144 = getelementptr inbounds [4 x i8], ptr %i5, i64 0, i64 1
  store i8 1, ptr %i144, align 1
  %i145 = getelementptr inbounds [4 x i8], ptr %i5, i64 0, i64 2
  store i8 2, ptr %i145, align 1
  %i146 = getelementptr inbounds [4 x i8], ptr %i5, i64 0, i64 3
  store i8 0, ptr %i146, align 1
  %i147 = getelementptr inbounds { double }, ptr %i6, i64 0, i32 0
  store double %i121, ptr %i147, align 8
  %i149 = call i32 @for_write_seq_fmt_xmit(ptr nonnull %i, ptr nonnull %i143, ptr nonnull %i6)
  %i150 = getelementptr inbounds [4 x i8], ptr %i7, i64 0, i64 0
  store i8 48, ptr %i150, align 1
  %i151 = getelementptr inbounds [4 x i8], ptr %i7, i64 0, i64 1
  store i8 1, ptr %i151, align 1
  %i152 = getelementptr inbounds [4 x i8], ptr %i7, i64 0, i64 2
  store i8 2, ptr %i152, align 1
  %i153 = getelementptr inbounds [4 x i8], ptr %i7, i64 0, i64 3
  store i8 0, ptr %i153, align 1
  %i154 = getelementptr inbounds { double }, ptr %i8, i64 0, i32 0
  store double 0x3CB0000000000000, ptr %i154, align 8
  %i156 = call i32 @for_write_seq_fmt_xmit(ptr nonnull %i, ptr nonnull %i150, ptr nonnull %i8)
  %i157 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 1)
  %i159 = load i64, ptr %i157, align 1
  %i160 = getelementptr inbounds [4 x i8], ptr %i9, i64 0, i64 0
  store i8 48, ptr %i160, align 1
  %i161 = getelementptr inbounds [4 x i8], ptr %i9, i64 0, i64 1
  store i8 1, ptr %i161, align 1
  %i162 = getelementptr inbounds [4 x i8], ptr %i9, i64 0, i64 2
  store i8 2, ptr %i162, align 1
  %i163 = getelementptr inbounds [4 x i8], ptr %i9, i64 0, i64 3
  store i8 0, ptr %i163, align 1
  store i64 %i159, ptr %i10, align 8
  %i166 = call i32 @for_write_seq_fmt_xmit(ptr nonnull %i, ptr nonnull %i160, ptr nonnull %i10)
  %i167 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"linpk_$X", i64 2500)
  %i169 = load i64, ptr %i167, align 1
  %i170 = getelementptr inbounds [4 x i8], ptr %i11, i64 0, i64 0
  store i8 48, ptr %i170, align 1
  %i171 = getelementptr inbounds [4 x i8], ptr %i11, i64 0, i64 1
  store i8 1, ptr %i171, align 1
  %i172 = getelementptr inbounds [4 x i8], ptr %i11, i64 0, i64 2
  store i8 1, ptr %i172, align 1
  %i173 = getelementptr inbounds [4 x i8], ptr %i11, i64 0, i64 3
  store i8 0, ptr %i173, align 1
  store i64 %i169, ptr %i12, align 8
  %i176 = call i32 @for_write_seq_fmt_xmit(ptr nonnull %i, ptr nonnull %i170, ptr nonnull %i12)
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare double @llvm.fabs.f64(double) #1

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare double @llvm.maxnum.f64(double, double) #1

declare dso_local i32 @for_set_reentrancy(ptr) local_unnamed_addr

declare dso_local i32 @for_write_seq_fmt(ptr, i32, i64, ptr, ptr, ptr, ...) local_unnamed_addr

declare dso_local i32 @for_write_seq_fmt_xmit(ptr, ptr, ptr) local_unnamed_addr

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

attributes #0 = { "intel-lang"="fortran" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { nounwind readnone speculatable }
; end INTEL_FEATURE_SW_ADVANCED

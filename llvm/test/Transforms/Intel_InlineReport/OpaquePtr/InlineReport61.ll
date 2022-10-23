; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -opaque-pointers -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe807 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-BEFORE
; Inline report via metadata
; RUN: opt -opaque-pointers -inlinereportsetup -inline-report=0xe886 < %s -S | opt -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe886 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-AFTER

; Check that all instances of @wolff_ are not inlined due to the inline budget
; and single callsite local linkage heuristics because only one call to @wolff_
; is inside a loop.

; CHECK-BEFORE: call{{.*}}@wolff_
; CHECK-BEFORE: call{{.*}}@wolff_
; CHECK-BEFORE: call{{.*}}@wolff_
; CHECK-NOT: INLINE: wolff_{{.*}}Has inline budget for small application
; CHECK-NOT: INLINE: wolff_{{.*}}Callee has single callsite and local linkage
; CHECK-AFTER: call{{.*}}@wolff_
; CHECK-AFTER: call{{.*}}@wolff_
; CHECK-AFTER: call{{.*}}@wolff_

@"_unnamed_main$$_$IN1" = internal global [16 x i32] zeroinitializer, align 16
@"_unnamed_main$$_$IP1" = internal global [16 x i32] zeroinitializer, align 16
@"_unnamed_main$$_$ISTACK" = internal global [2 x [256 x i32]] zeroinitializer, align 16
@"_unnamed_main$$_$IZ" = internal global [16 x [16 x i32]] zeroinitializer, align 16
@"wolff_$NN" = internal unnamed_addr global [2 x [4 x i32]] zeroinitializer, align 16

declare double @fmod(double, double) local_unnamed_addr

define internal fastcc void @wolff_(ptr noalias nocapture readonly %arg, ptr noalias nocapture %arg1, ptr noalias nocapture %arg2, ptr noalias nocapture %arg3) unnamed_addr #0 {
bb:
  store i32 0, ptr %arg2, align 1
  store i32 1, ptr %arg3, align 1
  %i = load double, ptr %arg1, align 1
  %i4 = fmul fast double %i, 1.680700e+04
  %i5 = tail call fast double @fmod(double %i4, double 0x41DFFFFFFFC00000)
  %i6 = fmul fast double %i5, 0x3E00000000200000
  %i7 = fptrunc double %i6 to float
  %i8 = fmul fast double %i5, 1.680700e+04
  %i9 = tail call fast double @fmod(double %i8, double 0x41DFFFFFFFC00000)
  store double %i9, ptr %arg1, align 1
  %i10 = fmul fast double %i9, 0x3E00000000200000
  %i11 = fptrunc double %i10 to float
  %i12 = fmul fast float %i7, 1.600000e+01
  %i13 = fadd fast float %i12, 1.000000e+00
  %i14 = fptosi float %i13 to i32
  %i15 = icmp slt i32 %i14, 16
  %i16 = select i1 %i15, i32 %i14, i32 16
  %i17 = fmul fast float %i11, 1.600000e+01
  %i18 = fadd fast float %i17, 1.000000e+00
  %i19 = fptosi float %i18 to i32
  %i20 = icmp slt i32 %i19, 16
  %i21 = select i1 %i20, i32 %i19, i32 16
  %i22 = sext i32 %i16 to i64
  %i23 = sext i32 %i21 to i64
  %i24 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 64, ptr elementtype(i32) @"_unnamed_main$$_$IZ", i64 %i23)
  %i25 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i24, i64 %i22)
  %i26 = load i32, ptr %i25, align 1
  %i27 = sub nsw i32 0, %i26
  store i32 %i27, ptr %i25, align 1
  %i28 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 16, ptr elementtype(i32) @"wolff_$NN", i64 1)
  %i29 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i28, i64 1)
  %i30 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 16, ptr elementtype(i32) @"wolff_$NN", i64 2)
  %i31 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i30, i64 1)
  %i32 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i28, i64 2)
  %i33 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i30, i64 2)
  %i34 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i28, i64 3)
  %i35 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i30, i64 3)
  %i36 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i28, i64 4)
  %i37 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i30, i64 4)
  %i38 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 1024, ptr elementtype(i32) @"_unnamed_main$$_$ISTACK", i64 1)
  %i39 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 1024, ptr elementtype(i32) @"_unnamed_main$$_$ISTACK", i64 2)
  br label %bb40

bb40:                                             ; preds = %bb97, %bb
  %i41 = phi i32 [ 1, %bb ], [ %i89, %bb97 ]
  %i42 = phi double [ %i9, %bb ], [ %i90, %bb97 ]
  %i43 = phi i32 [ 1, %bb ], [ %i91, %bb97 ]
  %i44 = phi i32 [ 0, %bb ], [ %i103, %bb97 ]
  %i45 = phi i32 [ %i16, %bb ], [ %i100, %bb97 ]
  %i46 = phi i32 [ %i21, %bb ], [ %i102, %bb97 ]
  store i32 %i45, ptr %i29, align 1
  %i47 = sext i32 %i46 to i64
  %i48 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) @"_unnamed_main$$_$IN1", i64 %i47)
  %i49 = load i32, ptr %i48, align 1
  store i32 %i49, ptr %i31, align 1
  store i32 %i45, ptr %i32, align 1
  %i50 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) @"_unnamed_main$$_$IP1", i64 %i47)
  %i51 = load i32, ptr %i50, align 1
  store i32 %i51, ptr %i33, align 1
  %i52 = sext i32 %i45 to i64
  %i53 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) @"_unnamed_main$$_$IN1", i64 %i52)
  %i54 = load i32, ptr %i53, align 1
  store i32 %i54, ptr %i34, align 1
  store i32 %i46, ptr %i35, align 1
  %i55 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) @"_unnamed_main$$_$IP1", i64 %i52)
  %i56 = load i32, ptr %i55, align 1
  store i32 %i56, ptr %i36, align 1
  store i32 %i46, ptr %i37, align 1
  br label %bb57

bb57:                                             ; preds = %bb88, %bb40
  %i58 = phi i32 [ %i89, %bb88 ], [ %i41, %bb40 ]
  %i59 = phi double [ %i90, %bb88 ], [ %i42, %bb40 ]
  %i60 = phi i64 [ %i93, %bb88 ], [ 1, %bb40 ]
  %i61 = phi i32 [ %i91, %bb88 ], [ %i43, %bb40 ]
  %i62 = phi i32 [ %i92, %bb88 ], [ %i44, %bb40 ]
  %i63 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %i28, i64 %i60)
  %i64 = load i32, ptr %i63, align 1
  %i65 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %i30, i64 %i60)
  %i66 = load i32, ptr %i65, align 1
  %i67 = sext i32 %i64 to i64
  %i68 = sext i32 %i66 to i64
  %i69 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 64, ptr nonnull elementtype(i32) @"_unnamed_main$$_$IZ", i64 %i68)
  %i70 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i69, i64 %i67)
  %i71 = load i32, ptr %i70, align 1
  %i72 = icmp eq i32 %i26, %i71
  br i1 %i72, label %bb73, label %bb88

bb73:                                             ; preds = %bb57
  %i74 = add nsw i32 %i61, 1
  %i75 = fmul fast double %i59, 1.680700e+04
  %i76 = tail call fast double @fmod(double %i75, double 0x41DFFFFFFFC00000)
  store double %i76, ptr %arg1, align 1
  %i77 = fmul fast double %i76, 0x3E00000000200000
  %i78 = fptrunc double %i77 to float
  %i79 = fpext float %i78 to double
  %i80 = load double, ptr %arg, align 1
  %i81 = fcmp fast olt double %i80, %i79
  br i1 %i81, label %bb88, label %bb82

bb82:                                             ; preds = %bb73
  store i32 %i27, ptr %i70, align 1
  %i83 = add nsw i32 %i58, 1
  store i32 %i83, ptr %arg3, align 1
  %i84 = add nsw i32 %i62, 1
  %i85 = sext i32 %i84 to i64
  %i86 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i38, i64 %i85)
  store i32 %i64, ptr %i86, align 1
  %i87 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i39, i64 %i85)
  store i32 %i66, ptr %i87, align 1
  br label %bb88

bb88:                                             ; preds = %bb82, %bb73, %bb57
  %i89 = phi i32 [ %i58, %bb57 ], [ %i58, %bb73 ], [ %i83, %bb82 ]
  %i90 = phi double [ %i59, %bb57 ], [ %i76, %bb73 ], [ %i76, %bb82 ]
  %i91 = phi i32 [ %i61, %bb57 ], [ %i74, %bb73 ], [ %i74, %bb82 ]
  %i92 = phi i32 [ %i62, %bb57 ], [ %i62, %bb73 ], [ %i84, %bb82 ]
  %i93 = add nuw nsw i64 %i60, 1
  %i94 = icmp eq i64 %i93, 5
  br i1 %i94, label %bb95, label %bb57

bb95:                                             ; preds = %bb88
  %i96 = icmp eq i32 %i92, 0
  br i1 %i96, label %bb104, label %bb97

bb97:                                             ; preds = %bb95
  %i98 = sext i32 %i92 to i64
  %i99 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i38, i64 %i98)
  %i100 = load i32, ptr %i99, align 1
  %i101 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i39, i64 %i98)
  %i102 = load i32, ptr %i101, align 1
  %i103 = add nsw i32 %i92, -1
  br label %bb40

bb104:                                            ; preds = %bb95
  store i32 %i91, ptr %arg2, align 1
  ret void
}

define dso_local void @MAIN__() local_unnamed_addr #0 {
L00:
  %t02 = alloca double, align 8
  %t03 = alloca double, align 8
  %t04 = alloca i32, align 8
  %t05 = alloca i32, align 8
  br label %L01

L01:                                              ; preds = %L01, %L00
  %t10 = phi i32 [ 0, %L00 ], [ %t12, %L01 ]
  call fastcc void @wolff_(ptr nonnull %t03, ptr nonnull %t02, ptr nonnull %t05, ptr nonnull %t04)
  %t11 = load i32, ptr %t05, align 8
  %t12 = add nsw i32 %t11, %t10
  %t13 = icmp slt i32 %t12, 256
  br i1 %t13, label %L01, label %L02

L02:                                              ; preds = %L01
  call fastcc void @wolff_(ptr nonnull %t03, ptr nonnull %t02, ptr nonnull %t05, ptr nonnull %t04)
  call fastcc void @wolff_(ptr nonnull %t03, ptr nonnull %t02, ptr nonnull %t05, ptr nonnull %t04)
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { "intel-lang"="fortran" }
attributes #1 = { nounwind readnone speculatable }
; end INTEL_FEATURE_SW_ADVANCED

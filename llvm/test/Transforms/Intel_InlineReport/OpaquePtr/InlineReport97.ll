; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; Inline report
; RUN: opt -opaque-pointers -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe807 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK-NPM,CHECK-AFTER
; Inline report via metadata
; RUN: opt -opaque-pointers -inlinereportsetup -inline-report=0xe886 < %s -S | opt -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe886 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-NPM,CHECK-AFTER

; Check that all instances of @wolff_ are inlined due to the inline budget and
; single callsite local linkage heuristics. This is the same test case as
; Intel_InlineReport58.ll, but it checks when an argument is a load instruction.

; CHECK-BEFORE-NOT: call{{.*}}@wolff_
; CHECK-LEGACY: INLINE: wolff_{{.*}}Has inline budget for small application
; CHECK-LEGACY: INLINE: wolff_{{.*}}Callee has single callsite and local linkage
; CHECK-LEGACY: INLINE: wolff_{{.*}}Has inline budget for small application
; CHECK-AFTER-NOT: call{{.*}}@wolff_

; CHECK-NPM: INLINE: wolff_{{.*}}Has inline budget for small application
; CHECK-NPM: INLINE: wolff_{{.*}}Has inline budget for small application
; CHECK-NPM: INLINE: wolff_{{.*}}Callee has single callsite and local linkage
; CHECK-NPM-NOT: call{{.*}}@wolff_

@"_unnamed_main$$_$IN1" = internal global [16 x i32] zeroinitializer, align 16
@"_unnamed_main$$_$IP1" = internal global [16 x i32] zeroinitializer, align 16
@"_unnamed_main$$_$ISTACK" = internal global [2 x [256 x i32]] zeroinitializer, align 16
@"_unnamed_main$$_$IZ" = internal global [16 x [16 x i32]] zeroinitializer, align 16
@"wolff_$NN" = internal unnamed_addr global [2 x [4 x i32]] zeroinitializer, align 16

declare double @fmod(double, double) local_unnamed_addr

define internal fastcc void @wolff_(double %arg, ptr noalias nocapture %arg1, ptr noalias nocapture %arg2, ptr noalias nocapture %arg3) unnamed_addr #0 {
bb:
  store i32 0, ptr %arg2, align 1
  store i32 1, ptr %arg3, align 1
  %i = load double, ptr %arg1, align 1
  %i4 = fmul fast double %i, 1.680700e+04
  %i5 = frem fast double %i4, 0x41DFFFFFFFC00000
  %i6 = fmul fast double %i5, 0x3E00000000200000
  %i7 = fptrunc double %i6 to float
  %i8 = fmul fast double %i5, 1.680700e+04
  %i9 = frem fast double %i8, 0x41DFFFFFFFC00000
  store double %i9, ptr %arg1, align 1
  %i10 = fmul fast double %i9, 0x3E00000000200000
  %i11 = fptrunc double %i10 to float
  %i12 = fmul fast float %i7, 1.600000e+01
  %i13 = fadd fast float %i12, 1.000000e+00
  %i14 = fptosi float %i13 to i32
  %i15 = icmp slt i32 16, %i14
  %i16 = select i1 %i15, i32 16, i32 %i14
  %i17 = fmul fast float %i11, 1.600000e+01
  %i18 = fadd fast float %i17, 1.000000e+00
  %i19 = fptosi float %i18 to i32
  %i20 = icmp slt i32 16, %i19
  %i21 = select i1 %i20, i32 16, i32 %i19
  %i22 = sext i32 %i16 to i64
  %i23 = sext i32 %i21 to i64
  %i24 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 64, ptr nonnull elementtype(i32) @"_unnamed_main$$_$IZ", i64 %i23)
  %i25 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %i24, i64 %i22)
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
  %i38 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 1024, ptr nonnull elementtype(i32) @"_unnamed_main$$_$ISTACK", i64 1)
  %i39 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 1024, ptr nonnull elementtype(i32) @"_unnamed_main$$_$ISTACK", i64 2)
  br label %bb40

bb40:                                             ; preds = %bb97, %bb
  %i41 = phi i32 [ 1, %bb ], [ %i88, %bb97 ]
  %i42 = phi double [ %i9, %bb ], [ %i89, %bb97 ]
  %i43 = phi i32 [ 1, %bb ], [ %i90, %bb97 ]
  %i44 = phi i32 [ 0, %bb ], [ %i103, %bb97 ]
  %i45 = phi i32 [ %i16, %bb ], [ %i100, %bb97 ]
  %i46 = phi i32 [ %i21, %bb ], [ %i102, %bb97 ]
  store i32 %i45, ptr %i29, align 1
  %i47 = sext i32 %i46 to i64
  %i48 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) @"_unnamed_main$$_$IN1", i64 %i47)
  %i49 = load i32, ptr %i48, align 1
  store i32 %i49, ptr %i31, align 1
  store i32 %i45, ptr %i32, align 1
  %i50 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) @"_unnamed_main$$_$IP1", i64 %i47)
  %i51 = load i32, ptr %i50, align 1
  store i32 %i51, ptr %i33, align 1
  %i52 = sext i32 %i45 to i64
  %i53 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) @"_unnamed_main$$_$IN1", i64 %i52)
  %i54 = load i32, ptr %i53, align 1
  store i32 %i54, ptr %i34, align 1
  store i32 %i46, ptr %i35, align 1
  %i55 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) @"_unnamed_main$$_$IP1", i64 %i52)
  %i56 = load i32, ptr %i55, align 1
  store i32 %i56, ptr %i36, align 1
  store i32 %i46, ptr %i37, align 1
  br label %bb57

bb57:                                             ; preds = %bb87, %bb40
  %i58 = phi i32 [ %i88, %bb87 ], [ %i41, %bb40 ]
  %i59 = phi double [ %i89, %bb87 ], [ %i42, %bb40 ]
  %i60 = phi i64 [ %i92, %bb87 ], [ 1, %bb40 ]
  %i61 = phi i32 [ %i90, %bb87 ], [ %i43, %bb40 ]
  %i62 = phi i32 [ %i91, %bb87 ], [ %i44, %bb40 ]
  %i63 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %i28, i64 %i60)
  %i64 = load i32, ptr %i63, align 1
  %i65 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %i30, i64 %i60)
  %i66 = load i32, ptr %i65, align 1
  %i67 = sext i32 %i64 to i64
  %i68 = sext i32 %i66 to i64
  %i69 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 64, ptr nonnull elementtype(i32) @"_unnamed_main$$_$IZ", i64 %i68)
  %i70 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %i69, i64 %i67)
  %i71 = load i32, ptr %i70, align 1
  %i72 = icmp eq i32 %i26, %i71
  br i1 %i72, label %bb73, label %bb87

bb73:                                             ; preds = %bb57
  %i74 = add nsw i32 %i61, 1
  %i75 = fmul fast double %i59, 1.680700e+04
  %i76 = frem fast double %i75, 0x41DFFFFFFFC00000
  store double %i76, ptr %arg1, align 1
  %i77 = fmul fast double %i76, 0x3E00000000200000
  %i78 = fptrunc double %i77 to float
  %i79 = fpext float %i78 to double
  %i80 = fcmp fast olt double %arg, %i79
  br i1 %i80, label %bb87, label %bb81

bb81:                                             ; preds = %bb73
  store i32 %i27, ptr %i70, align 1
  %i82 = add nsw i32 %i58, 1
  store i32 %i82, ptr %arg3, align 1
  %i83 = add nsw i32 %i62, 1
  %i84 = sext i32 %i83 to i64
  %i85 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %i38, i64 %i84)
  store i32 %i64, ptr %i85, align 1
  %i86 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %i39, i64 %i84)
  store i32 %i66, ptr %i86, align 1
  br label %bb87

bb87:                                             ; preds = %bb81, %bb73, %bb57
  %i88 = phi i32 [ %i58, %bb57 ], [ %i58, %bb73 ], [ %i82, %bb81 ]
  %i89 = phi double [ %i59, %bb57 ], [ %i76, %bb73 ], [ %i76, %bb81 ]
  %i90 = phi i32 [ %i61, %bb57 ], [ %i74, %bb73 ], [ %i74, %bb81 ]
  %i91 = phi i32 [ %i62, %bb57 ], [ %i62, %bb73 ], [ %i83, %bb81 ]
  %i92 = add nuw nsw i64 %i60, 1
  %i93 = icmp eq i64 %i92, 5
  br i1 %i93, label %bb94, label %bb57

bb94:                                             ; preds = %bb87
  %i95 = icmp eq i32 %i91, 0
  br i1 %i95, label %bb96, label %bb97

bb96:                                             ; preds = %bb94
  store i32 %i90, ptr %arg2, align 1
  ret void

bb97:                                             ; preds = %bb94
  %i98 = sext i32 %i91 to i64
  %i99 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %i38, i64 %i98)
  %i100 = load i32, ptr %i99, align 1
  %i101 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %i39, i64 %i98)
  %i102 = load i32, ptr %i101, align 1
  %i103 = add nsw i32 %i91, -1
  br label %bb40
}

define dso_local void @MAIN__() local_unnamed_addr #0 {
L00:
  %t02 = alloca double, align 8
  %t03 = alloca double, align 8
  %t04 = alloca i32, align 8
  %t05 = alloca i32, align 8
  %t06 = alloca double, align 8
  br label %L01

L01:                                              ; preds = %L01, %L00
  %t10 = phi i32 [ 0, %L00 ], [ %t12, %L01 ]
  %tload = load double, ptr %t06, align 8
  call fastcc void @wolff_(double %tload, ptr nonnull %t02, ptr nonnull %t05, ptr nonnull %t04)
  %t11 = load i32, ptr %t05, align 8
  %t12 = add nsw i32 %t11, %t10
  %t13 = icmp slt i32 %t12, 256
  br i1 %t13, label %L01, label %L02

L02:                                              ; preds = %L02, %L01
  %t20 = phi i32 [ 0, %L01 ], [ %t22, %L02 ]
  call fastcc void @wolff_(double %tload, ptr nonnull %t02, ptr nonnull %t05, ptr nonnull %t04)
  %t21 = load i32, ptr %t05, align 8
  %t22 = add nsw i32 %t21, %t10
  %t23 = icmp slt i32 %t22, 256
  br i1 %t23, label %L02, label %L03

L03:                                              ; preds = %L03, %L02
  %t30 = phi i32 [ 0, %L02 ], [ %t32, %L03 ]
  call fastcc void @wolff_(double %tload, ptr nonnull %t02, ptr nonnull %t05, ptr nonnull %t04)
  %t31 = load i32, ptr %t05, align 8
  %t32 = add nsw i32 %t31, %t10
  %t33 = icmp slt i32 %t32, 256
  br i1 %t33, label %L03, label %L04

L04:                                              ; preds = %L03
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { "intel-lang"="fortran" }
attributes #1 = { nounwind readnone speculatable }
; end INTEL_FEATURE_SW_ADVANCED

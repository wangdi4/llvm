; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -opaque-pointers -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe807 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-BEFORE
; Inline report via metadata
; RUN: opt -opaque-pointers -inlinereportsetup -inline-report=0xe886 < %s -S | opt -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe886 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-AFTER

; Check that @perdida_m_mp_perdida_ is inlined due to the single callsite local
; linkage heuristic, but @perdida_m_mp_generalized_hookes_law_ is not inlined
; due to the inline budget heuristic because the calls to it are not contained
; within loops.

; CHECK-BEFORE-NOT: call{{.*}}@perdida_m_mp_perdida_
; CHECK-BEFORE: call{{.*}}@perdida_m_mp_generalized_hookes_law_
; CHECK-BEFORE: call{{.*}}@perdida_m_mp_generalized_hookes_law_
; CHECK: INLINE: perdida_m_mp_perdida_{{.*}}Callee has single callsite and local linkage
; CHECK-NOT: INLINE: perdida_m_mp_generalized_hookes_law_{{.*}}Callee has single callsite and local linkage
; CHECK-NOT: INLINE: perdida_m_mp_generalized_hookes_law_{{.*}}Has inline budget for small application
; CHECK-AFTER-NOT: call{{.*}}@perdida_m_mp_perdida_
; CHECK-AFTER: call{{.*}}@perdida_m_mp_generalized_hookes_law_
; CHECK-AFTER: call{{.*}}@perdida_m_mp_generalized_hookes_law_

%"QNCA_a0$double*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"QNCA_a0$double*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@anon.58475115b99d6a1547144f0b5d7ba7df.120 = internal unnamed_addr constant [0 x i8] zeroinitializer
@anon.58475115b99d6a1547144f0b5d7ba7df.78 = internal unnamed_addr constant [43 x i8] c"discriminant is negative in perdida, abort."
@anon.58475115b99d6a1547144f0b5d7ba7df.77 = internal unnamed_addr constant [46 x i8] c"bad plastic strain fraction in perdida, abort."
@"iztaccihuatl_$DAMAGE" = internal global %"QNCA_a0$double*$rank1$" { ptr null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@"iztaccihuatl_$ACCUMULATED_PLASTIC_STRAIN" = internal global %"QNCA_a0$double*$rank1$" { ptr null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@"iztaccihuatl_$ISOTROPIC_HARDENING_STRESS" = internal global %"QNCA_a0$double*$rank1$" { ptr null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@"perdida_m_mp_generalized_hookes_law_$GENERALIZED_CONSTITUTIVE_TENSOR" = internal unnamed_addr global [6 x [6 x double]] zeroinitializer, align 16
@"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRAIN_VECTOR" = internal unnamed_addr global [6 x double] zeroinitializer, align 16
@"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRESS_VECTOR" = internal unnamed_addr global [6 x double] zeroinitializer, align 16
@"perdida_m_mp_perdida_$DEVIATORIC_STRESS_TENSOR" = internal unnamed_addr global [3 x [3 x double]] zeroinitializer, align 16
@"perdida_m_mp_perdida_$DAMAGED_DEV_STRESS_TENSOR" = internal unnamed_addr global [3 x [3 x double]] zeroinitializer, align 16
@"perdida_m_mp_perdida_$DEVIATORIC_STRAIN_RATE_TENSOR" = internal unnamed_addr global [3 x [3 x double]] zeroinitializer, align 16
@"perdida_m_mp_perdida_$OLD_PLASTIC_STRAIN_TENSOR" = internal unnamed_addr global [3 x [3 x double]] zeroinitializer, align 16
@"perdida_m_mp_perdida_$DEVIATORIC_STRAIN_TENSOR" = internal unnamed_addr global [3 x [3 x double]] zeroinitializer, align 16
@"perdida_m_mp_perdida_$PLASTIC_STRAIN_RATE_TENSOR" = internal unnamed_addr global [3 x [3 x double]] zeroinitializer, align 16
@"perdida_m_mp_perdida_$BACK_STRESS_RATE_TENSOR" = internal unnamed_addr global [3 x [3 x double]] zeroinitializer, align 16

declare dso_local i32 @for_stop_core_quiet(ptr, i32, i32, i64, i32, i32, ...)

declare dso_local i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...)

declare dso_local i32 @for_write_seq_lis_xmit(ptr, ptr, ptr)

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore(ptr) #0

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare ptr @llvm.stacksave() #0

define internal fastcc void @perdida_m_mp_generalized_hookes_law_(ptr noalias nocapture readonly dereferenceable(96) %arg, ptr noalias nocapture readonly dereferenceable(96) "ptrnoalias" %arg1, ptr noalias nocapture readonly %arg2, ptr noalias nocapture readonly %arg3) unnamed_addr #1 {
bb:
  %i = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg1, i64 0, i32 0
  br label %bb12

bb4:                                              ; preds = %bb12, %bb4
  %i5 = phi i64 [ 1, %bb12 ], [ %i7, %bb4 ]
  %i6 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i14, i64 %i5)
  store double 0.000000e+00, ptr %i6, align 1
  %i7 = add nuw nsw i64 %i5, 1
  %i8 = icmp eq i64 %i7, 7
  br i1 %i8, label %bb9, label %bb4

bb9:                                              ; preds = %bb4
  %i10 = add nuw nsw i64 %i13, 1
  %i11 = icmp eq i64 %i10, 7
  br i1 %i11, label %bb15, label %bb12

bb12:                                             ; preds = %bb9, %bb
  %i13 = phi i64 [ 1, %bb ], [ %i10, %bb9 ]
  %i14 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 48, ptr elementtype(double) @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_CONSTITUTIVE_TENSOR", i64 %i13)
  br label %bb4

bb15:                                             ; preds = %bb9
  %i16 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg1, i64 0, i32 6, i64 0, i32 1
  %i17 = load double, ptr %arg2, align 1
  %i18 = load double, ptr %arg3, align 1
  %i19 = fmul fast double %i18, 2.000000e+00
  %i20 = fadd fast double %i19, %i17
  %i21 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 48, ptr elementtype(double) @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_CONSTITUTIVE_TENSOR", i64 1)
  %i22 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i21, i64 1)
  store double %i20, ptr %i22, align 1
  %i23 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 48, ptr elementtype(double) @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_CONSTITUTIVE_TENSOR", i64 2)
  %i24 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i23, i64 1)
  store double %i17, ptr %i24, align 1
  %i25 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 48, ptr elementtype(double) @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_CONSTITUTIVE_TENSOR", i64 3)
  %i26 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i25, i64 1)
  store double %i17, ptr %i26, align 1
  %i27 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i21, i64 2)
  store double %i17, ptr %i27, align 1
  %i28 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i23, i64 2)
  store double %i20, ptr %i28, align 1
  %i29 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i25, i64 2)
  store double %i17, ptr %i29, align 1
  %i30 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i21, i64 3)
  store double %i17, ptr %i30, align 1
  %i31 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i23, i64 3)
  store double %i17, ptr %i31, align 1
  %i32 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i25, i64 3)
  store double %i20, ptr %i32, align 1
  %i33 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 48, ptr elementtype(double) @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_CONSTITUTIVE_TENSOR", i64 4)
  %i34 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i33, i64 4)
  store double %i18, ptr %i34, align 1
  %i35 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 48, ptr elementtype(double) @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_CONSTITUTIVE_TENSOR", i64 5)
  %i36 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i35, i64 5)
  store double %i18, ptr %i36, align 1
  %i37 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 48, ptr elementtype(double) @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_CONSTITUTIVE_TENSOR", i64 6)
  %i38 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i37, i64 6)
  store double %i18, ptr %i38, align 1
  %i39 = load ptr, ptr %i, align 1
  %i40 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i16, i32 0)
  %i41 = load i64, ptr %i40, align 1
  %i42 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i16, i32 1)
  %i43 = load i64, ptr %i42, align 1
  %i44 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i43, ptr elementtype(double) %i39, i64 1)
  %i45 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i41, ptr elementtype(double) %i44, i64 1)
  %i47 = load i64, ptr %i45, align 1
  %i48 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRAIN_VECTOR", i64 1)
  store i64 %i47, ptr %i48, align 1
  %i50 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i43, ptr elementtype(double) %i39, i64 2)
  %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i41, ptr elementtype(double) %i50, i64 2)
  %i53 = load i64, ptr %i51, align 1
  %i54 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRAIN_VECTOR", i64 2)
  store i64 %i53, ptr %i54, align 1
  %i56 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i43, ptr elementtype(double) %i39, i64 3)
  %i57 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i41, ptr elementtype(double) %i56, i64 3)
  %i59 = load i64, ptr %i57, align 1
  %i60 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRAIN_VECTOR", i64 3)
  store i64 %i59, ptr %i60, align 1
  %i62 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i41, ptr elementtype(double) %i56, i64 2)
  %i64 = load i64, ptr %i62, align 1
  %i65 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRAIN_VECTOR", i64 4)
  store i64 %i64, ptr %i65, align 1
  %i67 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i41, ptr elementtype(double) %i56, i64 1)
  %i69 = load i64, ptr %i67, align 1
  %i70 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRAIN_VECTOR", i64 5)
  store i64 %i69, ptr %i70, align 1
  %i72 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i41, ptr elementtype(double) %i50, i64 1)
  %i74 = load i64, ptr %i72, align 1
  %i75 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRAIN_VECTOR", i64 6)
  store i64 %i74, ptr %i75, align 1
  br label %bb77

bb77:                                             ; preds = %bb91, %bb15
  %i78 = phi i64 [ %i93, %bb91 ], [ 1, %bb15 ]
  br label %bb79

bb79:                                             ; preds = %bb79, %bb77
  %i80 = phi double [ 0.000000e+00, %bb77 ], [ %i88, %bb79 ]
  %i81 = phi i64 [ 1, %bb77 ], [ %i89, %bb79 ]
  %i82 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 48, ptr elementtype(double) @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_CONSTITUTIVE_TENSOR", i64 %i81)
  %i83 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i82, i64 %i78)
  %i84 = load double, ptr %i83, align 1
  %i85 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRAIN_VECTOR", i64 %i81)
  %i86 = load double, ptr %i85, align 1
  %i87 = fmul fast double %i86, %i84
  %i88 = fadd fast double %i87, %i80
  %i89 = add nuw nsw i64 %i81, 1
  %i90 = icmp eq i64 %i89, 7
  br i1 %i90, label %bb91, label %bb79

bb91:                                             ; preds = %bb79
  %i92 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRESS_VECTOR", i64 %i78)
  store double %i88, ptr %i92, align 1
  %i93 = add nuw nsw i64 %i78, 1
  %i94 = icmp eq i64 %i93, 7
  br i1 %i94, label %bb95, label %bb77

bb95:                                             ; preds = %bb91
  %i96 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg, i64 0, i32 0
  %i97 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRESS_VECTOR", i64 1)
  %i99 = load i64, ptr %i97, align 1
  %i100 = load ptr, ptr %i96, align 1
  %i101 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) %i100, i64 1)
  %i102 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i101, i64 1)
  store i64 %i99, ptr %i102, align 1
  %i104 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRESS_VECTOR", i64 2)
  %i106 = load i64, ptr %i104, align 1
  %i107 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) %i100, i64 2)
  %i108 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i107, i64 2)
  store i64 %i106, ptr %i108, align 1
  %i110 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRESS_VECTOR", i64 3)
  %i112 = load i64, ptr %i110, align 1
  %i113 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) %i100, i64 3)
  %i114 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i113, i64 3)
  store i64 %i112, ptr %i114, align 1
  %i116 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRESS_VECTOR", i64 4)
  %i118 = load i64, ptr %i116, align 1
  %i119 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i113, i64 2)
  store i64 %i118, ptr %i119, align 1
  %i121 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRESS_VECTOR", i64 5)
  %i123 = load i64, ptr %i121, align 1
  %i124 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i113, i64 1)
  store i64 %i123, ptr %i124, align 1
  %i126 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRESS_VECTOR", i64 6)
  %i128 = load i64, ptr %i126, align 1
  %i129 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i107, i64 1)
  store i64 %i128, ptr %i129, align 1
  %i131 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i107, i64 3)
  store i64 %i118, ptr %i131, align 1
  %i133 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i101, i64 3)
  store i64 %i123, ptr %i133, align 1
  %i135 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i101, i64 2)
  store i64 %i128, ptr %i135, align 1
  ret void
}

define internal fastcc void @perdida_m_mp_perdida_(ptr noalias nocapture readonly %arg, ptr noalias nocapture readonly %arg1, ptr noalias nocapture readonly %arg2, ptr noalias nocapture readonly %arg3, ptr noalias nocapture readonly %arg4, ptr noalias nocapture readonly %arg5, ptr noalias nocapture readonly %arg6, ptr noalias nocapture readonly %arg7, ptr noalias nocapture readonly %arg8, ptr noalias nocapture readonly %arg9, ptr noalias nocapture readonly dereferenceable(96) "ptrnoalias" %arg10, ptr noalias nocapture readonly dereferenceable(96) "ptrnoalias" %arg11, ptr noalias nocapture readonly dereferenceable(96) "ptrnoalias" %arg12, ptr noalias nocapture readonly dereferenceable(96) "ptrnoalias" %arg13, ptr noalias nocapture %arg14, ptr noalias nocapture readonly dereferenceable(96) "ptrnoalias" %arg15, ptr noalias nocapture %arg16, ptr noalias nocapture %arg17, ptr noalias nocapture readonly %arg18, ptr noalias nocapture readonly %arg19) unnamed_addr #1 {
bb:
  %i = alloca [8 x i64], align 16
  %i20 = alloca %"QNCA_a0$double*$rank2$", align 8
  %i21 = alloca [9 x double], align 16
  %i22 = alloca %"QNCA_a0$double*$rank2$", align 8
  %i23 = alloca %"QNCA_a0$double*$rank2$", align 8
  %i24 = alloca [9 x double], align 16
  %i25 = alloca %"QNCA_a0$double*$rank2$", align 8
  %i26 = alloca [4 x i8], align 1
  %i27 = alloca { i64, ptr }, align 8
  %i28 = alloca [4 x i8], align 1
  %i29 = alloca { i64, ptr }, align 8
  %i30 = alloca [4 x i8], align 1
  %i31 = alloca { double }, align 8
  %i32 = getelementptr inbounds [9 x double], ptr %i24, i64 0, i64 0
  %i33 = getelementptr inbounds [9 x double], ptr %i21, i64 0, i64 0
  %i34 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg10, i64 0, i32 0
  %i35 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg10, i64 0, i32 6, i64 0, i32 0
  %i36 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg10, i64 0, i32 6, i64 0, i32 1
  %i37 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg11, i64 0, i32 0
  %i38 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg11, i64 0, i32 6, i64 0, i32 0
  %i39 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg11, i64 0, i32 6, i64 0, i32 1
  %i40 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg12, i64 0, i32 0
  %i41 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg12, i64 0, i32 6, i64 0, i32 1
  %i42 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i20, i64 0, i32 0
  %i43 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i20, i64 0, i32 1
  %i44 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i20, i64 0, i32 2
  %i45 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i20, i64 0, i32 3
  %i46 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i20, i64 0, i32 4
  %i47 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i20, i64 0, i32 6, i64 0, i32 0
  %i48 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i20, i64 0, i32 6, i64 0, i32 1
  %i49 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i20, i64 0, i32 6, i64 0, i32 2
  %i50 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i22, i64 0, i32 0
  %i51 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i22, i64 0, i32 1
  %i52 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i22, i64 0, i32 2
  %i53 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i22, i64 0, i32 3
  %i54 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i22, i64 0, i32 4
  %i55 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i22, i64 0, i32 6, i64 0, i32 0
  %i56 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i22, i64 0, i32 6, i64 0, i32 1
  %i57 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i22, i64 0, i32 6, i64 0, i32 2
  %i58 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg15, i64 0, i32 0
  %i59 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg15, i64 0, i32 6, i64 0, i32 0
  %i60 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg15, i64 0, i32 6, i64 0, i32 1
  %i61 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg13, i64 0, i32 0
  %i62 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg13, i64 0, i32 6, i64 0, i32 0
  %i63 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg13, i64 0, i32 6, i64 0, i32 1
  %i64 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i23, i64 0, i32 1
  %i65 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i23, i64 0, i32 2
  %i66 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i23, i64 0, i32 3
  %i67 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i23, i64 0, i32 4
  %i68 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i23, i64 0, i32 6, i64 0, i32 0
  %i69 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i23, i64 0, i32 6, i64 0, i32 1
  %i70 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i23, i64 0, i32 6, i64 0, i32 2
  %i71 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i25, i64 0, i32 0
  %i72 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i25, i64 0, i32 1
  %i73 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i25, i64 0, i32 2
  %i74 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i25, i64 0, i32 3
  %i75 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i25, i64 0, i32 4
  %i76 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i25, i64 0, i32 6, i64 0, i32 0
  %i77 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i25, i64 0, i32 6, i64 0, i32 1
  %i78 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i25, i64 0, i32 6, i64 0, i32 2
  %i79 = load double, ptr %arg1, align 1
  %i80 = fmul fast double %i79, 5.000000e-01
  %i81 = load double, ptr %arg2, align 1
  %i82 = fadd fast double %i81, %i79
  %i83 = fdiv fast double %i80, %i82
  %i84 = fmul fast double %i83, 2.000000e+00
  %i85 = fsub fast double 1.000000e+00, %i84
  %i86 = tail call ptr @llvm.stacksave()
  %i87 = load ptr, ptr %i34, align 1
  %i88 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i36, i32 0)
  %i89 = load i64, ptr %i88, align 1
  %i90 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i35, i32 0)
  %i91 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i36, i32 1)
  %i92 = load i64, ptr %i91, align 1
  %i93 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i35, i32 1)
  %i94 = load double, ptr %arg17, align 1
  %i95 = load ptr, ptr %i37, align 1
  %i96 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i39, i32 0)
  %i97 = load i64, ptr %i96, align 1
  %i98 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i38, i32 0)
  %i99 = load i64, ptr %i98, align 1
  %i100 = icmp sgt i64 %i99, 0
  %i101 = select i1 %i100, i64 %i99, i64 0
  %i102 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i39, i32 1)
  %i103 = load i64, ptr %i102, align 1
  %i104 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i38, i32 1)
  %i105 = load i64, ptr %i104, align 1
  %i106 = icmp sgt i64 %i105, 0
  %i107 = select i1 %i106, i64 %i105, i64 0
  %i108 = load ptr, ptr %i40, align 1
  %i109 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i41, i32 0)
  %i110 = load i64, ptr %i109, align 1
  %i111 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i41, i32 1)
  %i112 = load i64, ptr %i111, align 1
  %i113 = shl nsw i64 %i101, 3
  %i114 = mul nsw i64 %i107, %i113
  %i115 = lshr exact i64 %i114, 3
  %i116 = alloca double, i64 %i115, align 1
  %i117 = icmp slt i64 %i105, 1
  %i118 = icmp slt i64 %i99, 1
  %i119 = or i1 %i117, %i118
  br i1 %i119, label %bb141, label %bb120

bb120:                                            ; preds = %bb
  %i121 = add nuw nsw i64 %i99, 1
  %i122 = add nuw nsw i64 %i105, 1
  br label %bb136

bb123:                                            ; preds = %bb136, %bb123
  %i124 = phi i64 [ 1, %bb136 ], [ %i131, %bb123 ]
  %i125 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i97, ptr elementtype(double) %i138, i64 %i124)
  %i126 = load double, ptr %i125, align 1
  %i127 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i110, ptr elementtype(double) %i139, i64 %i124)
  %i128 = load double, ptr %i127, align 1
  %i129 = fsub fast double %i126, %i128
  %i130 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %i140, i64 %i124)
  store double %i129, ptr %i130, align 1
  %i131 = add nuw nsw i64 %i124, 1
  %i132 = icmp eq i64 %i131, %i121
  br i1 %i132, label %bb133, label %bb123

bb133:                                            ; preds = %bb123
  %i134 = add nuw nsw i64 %i137, 1
  %i135 = icmp eq i64 %i134, %i122
  br i1 %i135, label %bb141, label %bb136

bb136:                                            ; preds = %bb133, %bb120
  %i137 = phi i64 [ %i134, %bb133 ], [ 1, %bb120 ]
  %i138 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i103, ptr elementtype(double) %i95, i64 %i137)
  %i139 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i112, ptr elementtype(double) %i108, i64 %i137)
  %i140 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i113, ptr nonnull elementtype(double) %i116, i64 %i137)
  br label %bb123

bb141:                                            ; preds = %bb133, %bb
  store i64 8, ptr %i43, align 8
  store i64 2, ptr %i46, align 8
  store i64 0, ptr %i44, align 8
  %i142 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i48, i32 0)
  store i64 8, ptr %i142, align 1
  %i143 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i49, i32 0)
  store i64 1, ptr %i143, align 1
  %i144 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i47, i32 0)
  store i64 %i101, ptr %i144, align 1
  %i145 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i48, i32 1)
  store i64 %i113, ptr %i145, align 1
  %i146 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i49, i32 1)
  store i64 1, ptr %i146, align 1
  %i147 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i47, i32 1)
  store i64 %i107, ptr %i147, align 1
  store ptr %i116, ptr %i42, align 8
  store i64 1, ptr %i45, align 8
  store i64 8, ptr %i51, align 8
  store i64 2, ptr %i54, align 8
  store i64 0, ptr %i52, align 8
  %i148 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i56, i32 0)
  store i64 8, ptr %i148, align 1
  %i149 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i57, i32 0)
  store i64 1, ptr %i149, align 1
  %i150 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i55, i32 0)
  store i64 3, ptr %i150, align 1
  %i151 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i56, i32 1)
  store i64 24, ptr %i151, align 1
  %i152 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i57, i32 1)
  store i64 1, ptr %i152, align 1
  %i153 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i55, i32 1)
  store i64 3, ptr %i153, align 1
  store ptr %i33, ptr %i50, align 8
  store i64 1, ptr %i53, align 8
  call fastcc void @perdida_m_mp_generalized_hookes_law_(ptr nonnull %i22, ptr nonnull %i20, ptr nonnull %arg1, ptr nonnull %arg2)
  %i154 = fsub fast double 1.000000e+00, %i94
  br label %bb166

bb155:                                            ; preds = %bb166, %bb155
  %i156 = phi i64 [ 1, %bb166 ], [ %i161, %bb155 ]
  %i157 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %i168, i64 %i156)
  %i158 = load double, ptr %i157, align 1
  %i159 = fmul fast double %i158, %i154
  %i160 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i89, ptr elementtype(double) %i169, i64 %i156)
  store double %i159, ptr %i160, align 1
  %i161 = add nuw nsw i64 %i156, 1
  %i162 = icmp eq i64 %i161, 4
  br i1 %i162, label %bb163, label %bb155

bb163:                                            ; preds = %bb155
  %i164 = add nuw nsw i64 %i167, 1
  %i165 = icmp eq i64 %i164, 4
  br i1 %i165, label %bb170, label %bb166

bb166:                                            ; preds = %bb163, %bb141
  %i167 = phi i64 [ 1, %bb141 ], [ %i164, %bb163 ]
  %i168 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr nonnull elementtype(double) %i33, i64 %i167)
  %i169 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i92, ptr elementtype(double) %i87, i64 %i167)
  br label %bb155

bb170:                                            ; preds = %bb163
  call void @llvm.stackrestore(ptr %i86)
  %i171 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i92, ptr nonnull elementtype(double) %i87, i64 1)
  %i172 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i89, ptr elementtype(double) %i171, i64 1)
  %i173 = load double, ptr %i172, align 1
  %i174 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i92, ptr nonnull elementtype(double) %i87, i64 2)
  %i175 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i89, ptr elementtype(double) %i174, i64 2)
  %i176 = load double, ptr %i175, align 1
  %i177 = fadd fast double %i176, %i173
  %i178 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i92, ptr nonnull elementtype(double) %i87, i64 3)
  %i179 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i89, ptr elementtype(double) %i178, i64 3)
  %i180 = load double, ptr %i179, align 1
  %i181 = fadd fast double %i177, %i180
  %i182 = fmul fast double %i181, 0x3FD5555555555555
  %i183 = fcmp fast olt double %i182, -1.000000e-10
  %i184 = fcmp fast ogt double %i94, 0.000000e+00
  %i185 = and i1 %i184, %i183
  br i1 %i185, label %bb207, label %bb235

bb186:                                            ; preds = %bb197, %bb186
  %i187 = phi i64 [ 1, %bb197 ], [ %i192, %bb186 ]
  %i188 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i89, ptr elementtype(double) %i199, i64 %i187)
  %i189 = load double, ptr %i188, align 1
  %i190 = fmul fast double %i189, %i215
  %i191 = fdiv fast double %i190, %i154
  store double %i191, ptr %i188, align 1
  %i192 = add nuw nsw i64 %i187, 1
  %i193 = icmp eq i64 %i192, %i217
  br i1 %i193, label %bb194, label %bb186

bb194:                                            ; preds = %bb186
  %i195 = add nuw nsw i64 %i198, 1
  %i196 = icmp eq i64 %i195, %i218
  br i1 %i196, label %bb200, label %bb197

bb197:                                            ; preds = %bb216, %bb194
  %i198 = phi i64 [ %i195, %bb194 ], [ 1, %bb216 ]
  %i199 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i92, ptr nonnull elementtype(double) %i87, i64 %i198)
  br label %bb186

bb200:                                            ; preds = %bb212, %bb207, %bb194
  %i201 = load double, ptr %i172, align 1
  %i202 = load double, ptr %i175, align 1
  %i203 = fadd fast double %i202, %i201
  %i204 = load double, ptr %i179, align 1
  %i205 = fadd fast double %i203, %i204
  %i206 = fmul fast double %i205, 0x3FD5555555555555
  br label %bb235

bb207:                                            ; preds = %bb170
  %i208 = load double, ptr %arg19, align 1
  %i209 = load i64, ptr %i90, align 1
  %i210 = load i64, ptr %i93, align 1
  %i211 = icmp slt i64 %i210, 1
  br i1 %i211, label %bb200, label %bb212

bb212:                                            ; preds = %bb207
  %i213 = icmp slt i64 %i209, 1
  %i214 = fmul fast double %i208, %i94
  %i215 = fsub fast double 1.000000e+00, %i214
  br i1 %i213, label %bb200, label %bb216

bb216:                                            ; preds = %bb212
  %i217 = add nuw nsw i64 %i209, 1
  %i218 = add nuw nsw i64 %i210, 1
  br label %bb197

bb219:                                            ; preds = %bb231, %bb219
  %i220 = phi i64 [ 1, %bb231 ], [ %i226, %bb219 ]
  %i221 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i89, ptr elementtype(double) %i233, i64 %i220)
  %i223 = load i64, ptr %i221, align 1
  %i224 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i234, i64 %i220)
  store i64 %i223, ptr %i224, align 1
  %i226 = add nuw nsw i64 %i220, 1
  %i227 = icmp eq i64 %i226, 4
  br i1 %i227, label %bb228, label %bb219

bb228:                                            ; preds = %bb219
  %i229 = add nuw nsw i64 %i232, 1
  %i230 = icmp eq i64 %i229, 4
  br i1 %i230, label %bb241, label %bb231

bb231:                                            ; preds = %bb235, %bb228
  %i232 = phi i64 [ 1, %bb235 ], [ %i229, %bb228 ]
  %i233 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i92, ptr nonnull elementtype(double) %i87, i64 %i232)
  %i234 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$DEVIATORIC_STRESS_TENSOR", i64 %i232)
  br label %bb219

bb235:                                            ; preds = %bb200, %bb170
  %i236 = phi double [ %i204, %bb200 ], [ %i180, %bb170 ]
  %i237 = phi double [ %i202, %bb200 ], [ %i176, %bb170 ]
  %i238 = phi double [ %i201, %bb200 ], [ %i173, %bb170 ]
  %i239 = phi double [ %i206, %bb200 ], [ %i182, %bb170 ]
  %i240 = phi double [ %i208, %bb200 ], [ 1.000000e+00, %bb170 ]
  br label %bb231

bb241:                                            ; preds = %bb228
  %i242 = fsub fast double %i238, %i239
  %i243 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$DEVIATORIC_STRESS_TENSOR", i64 1)
  %i244 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i243, i64 1)
  store double %i242, ptr %i244, align 1
  %i245 = fsub fast double %i237, %i239
  %i246 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$DEVIATORIC_STRESS_TENSOR", i64 2)
  %i247 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i246, i64 2)
  store double %i245, ptr %i247, align 1
  %i248 = fsub fast double %i236, %i239
  %i249 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$DEVIATORIC_STRESS_TENSOR", i64 3)
  %i250 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i249, i64 3)
  store double %i248, ptr %i250, align 1
  %i251 = fmul fast double %i240, %i94
  %i252 = fsub fast double 1.000000e+00, %i251
  br label %bb264

bb253:                                            ; preds = %bb264, %bb253
  %i254 = phi i64 [ 1, %bb264 ], [ %i259, %bb253 ]
  %i255 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i266, i64 %i254)
  %i256 = load double, ptr %i255, align 1
  %i257 = fdiv fast double %i256, %i252
  %i258 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i267, i64 %i254)
  store double %i257, ptr %i258, align 1
  %i259 = add nuw nsw i64 %i254, 1
  %i260 = icmp eq i64 %i259, 4
  br i1 %i260, label %bb261, label %bb253

bb261:                                            ; preds = %bb253
  %i262 = add nuw nsw i64 %i265, 1
  %i263 = icmp eq i64 %i262, 4
  br i1 %i263, label %bb300, label %bb264

bb264:                                            ; preds = %bb261, %bb241
  %i265 = phi i64 [ 1, %bb241 ], [ %i262, %bb261 ]
  %i266 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$DEVIATORIC_STRESS_TENSOR", i64 %i265)
  %i267 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$DAMAGED_DEV_STRESS_TENSOR", i64 %i265)
  br label %bb253

bb268:                                            ; preds = %bb283, %bb268
  %i269 = phi double [ %i284, %bb283 ], [ %i277, %bb268 ]
  %i270 = phi i64 [ 1, %bb283 ], [ %i278, %bb268 ]
  %i271 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i286, i64 %i270)
  %i272 = load double, ptr %i271, align 1
  %i273 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i303, ptr elementtype(double) %i287, i64 %i270)
  %i274 = load double, ptr %i273, align 1
  %i275 = fsub fast double %i272, %i274
  %i276 = fmul fast double %i275, %i275
  %i277 = fadd fast double %i276, %i269
  %i278 = add nuw nsw i64 %i270, 1
  %i279 = icmp eq i64 %i278, 4
  br i1 %i279, label %bb280, label %bb268

bb280:                                            ; preds = %bb268
  %i281 = add nuw nsw i64 %i285, 1
  %i282 = icmp eq i64 %i281, 4
  br i1 %i282, label %bb288, label %bb283

bb283:                                            ; preds = %bb300, %bb280
  %i284 = phi double [ 0.000000e+00, %bb300 ], [ %i277, %bb280 ]
  %i285 = phi i64 [ 1, %bb300 ], [ %i281, %bb280 ]
  %i286 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$DAMAGED_DEV_STRESS_TENSOR", i64 %i285)
  %i287 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i305, ptr elementtype(double) %i301, i64 %i285)
  br label %bb268

bb288:                                            ; preds = %bb280
  %i289 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i59, i32 0)
  %i290 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i59, i32 1)
  %i291 = fmul fast double %i277, 1.500000e+00
  %i292 = call fast double @llvm.sqrt.f64(double %i291)
  %i293 = load double, ptr %arg16, align 1
  %i294 = load double, ptr %arg3, align 1
  %i295 = fadd fast double %i294, %i293
  %i296 = fsub fast double %i292, %i295
  %i297 = fcmp fast olt double %i296, 0.000000e+00
  %i298 = fcmp fast olt double %i292, 1.000000e-10
  %i299 = or i1 %i298, %i297
  br i1 %i299, label %bb776, label %bb379

bb300:                                            ; preds = %bb261
  %i301 = load ptr, ptr %i58, align 1
  %i302 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i60, i32 0)
  %i303 = load i64, ptr %i302, align 1
  %i304 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i60, i32 1)
  %i305 = load i64, ptr %i304, align 1
  br label %bb283

bb306:                                            ; preds = %bb318, %bb306
  %i307 = phi i64 [ 1, %bb318 ], [ %i313, %bb306 ]
  %i308 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i382, ptr elementtype(double) %i320, i64 %i307)
  %i310 = load i64, ptr %i308, align 1
  %i311 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i321, i64 %i307)
  store i64 %i310, ptr %i311, align 1
  %i313 = add nuw nsw i64 %i307, 1
  %i314 = icmp eq i64 %i313, 4
  br i1 %i314, label %bb315, label %bb306

bb315:                                            ; preds = %bb306
  %i316 = add nuw nsw i64 %i319, 1
  %i317 = icmp eq i64 %i316, 4
  br i1 %i317, label %bb322, label %bb318

bb318:                                            ; preds = %bb379, %bb315
  %i319 = phi i64 [ 1, %bb379 ], [ %i316, %bb315 ]
  %i320 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i384, ptr elementtype(double) %i380, i64 %i319)
  %i321 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$DEVIATORIC_STRAIN_RATE_TENSOR", i64 %i319)
  br label %bb306

bb322:                                            ; preds = %bb315
  %i323 = fmul fast double %i395, 0x3FD5555555555555
  %i324 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i62, i32 1)
  %i325 = fsub fast double %i387, %i323
  %i326 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$DEVIATORIC_STRAIN_RATE_TENSOR", i64 1)
  %i327 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i326, i64 1)
  store double %i325, ptr %i327, align 1
  %i328 = fsub fast double %i390, %i323
  %i329 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$DEVIATORIC_STRAIN_RATE_TENSOR", i64 2)
  %i330 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i329, i64 2)
  store double %i328, ptr %i330, align 1
  %i331 = fsub fast double %i394, %i323
  %i332 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$DEVIATORIC_STRAIN_RATE_TENSOR", i64 3)
  %i333 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i332, i64 3)
  store double %i331, ptr %i333, align 1
  %i334 = load i64, ptr %i396, align 1
  %i335 = icmp sgt i64 %i334, 0
  %i336 = select i1 %i335, i64 %i334, i64 0
  %i337 = load i64, ptr %i324, align 1
  %i338 = icmp sgt i64 %i337, 0
  %i339 = select i1 %i338, i64 %i337, i64 0
  store i64 8, ptr %i64, align 8
  store i64 2, ptr %i67, align 8
  store i64 0, ptr %i65, align 8
  %i340 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i69, i32 0)
  store i64 %i382, ptr %i340, align 1
  %i341 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i70, i32 0)
  store i64 1, ptr %i341, align 1
  %i342 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i68, i32 0)
  store i64 %i336, ptr %i342, align 1
  %i343 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i69, i32 1)
  store i64 %i384, ptr %i343, align 1
  %i344 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i70, i32 1)
  store i64 1, ptr %i344, align 1
  %i345 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i68, i32 1)
  store i64 %i339, ptr %i345, align 1
  store i64 %i397, ptr %i23, align 8
  store i64 1, ptr %i66, align 8
  store i64 8, ptr %i72, align 8
  store i64 2, ptr %i75, align 8
  store i64 0, ptr %i73, align 8
  %i347 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i77, i32 0)
  store i64 8, ptr %i347, align 1
  %i348 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i78, i32 0)
  store i64 1, ptr %i348, align 1
  %i349 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i76, i32 0)
  store i64 3, ptr %i349, align 1
  %i350 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i77, i32 1)
  store i64 24, ptr %i350, align 1
  %i351 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i78, i32 1)
  store i64 1, ptr %i351, align 1
  %i352 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i76, i32 1)
  store i64 3, ptr %i352, align 1
  store ptr %i32, ptr %i71, align 8
  store i64 1, ptr %i74, align 8
  call fastcc void @perdida_m_mp_generalized_hookes_law_(ptr nonnull %i25, ptr nonnull %i23, ptr nonnull %arg1, ptr nonnull %arg2)
  br label %bb370

bb353:                                            ; preds = %bb370, %bb353
  %i354 = phi double [ %i371, %bb370 ], [ %i364, %bb353 ]
  %i355 = phi i64 [ 1, %bb370 ], [ %i365, %bb353 ]
  %i356 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i373, i64 %i355)
  %i357 = load double, ptr %i356, align 1
  %i358 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i303, ptr elementtype(double) %i374, i64 %i355)
  %i359 = load double, ptr %i358, align 1
  %i360 = fsub fast double %i357, %i359
  %i361 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %i375, i64 %i355)
  %i362 = load double, ptr %i361, align 1
  %i363 = fmul fast double %i360, %i362
  %i364 = fadd fast double %i363, %i354
  %i365 = add nuw nsw i64 %i355, 1
  %i366 = icmp eq i64 %i365, 4
  br i1 %i366, label %bb367, label %bb353

bb367:                                            ; preds = %bb353
  %i368 = add nuw nsw i64 %i372, 1
  %i369 = icmp eq i64 %i368, 4
  br i1 %i369, label %bb376, label %bb370

bb370:                                            ; preds = %bb367, %bb322
  %i371 = phi double [ 0.000000e+00, %bb322 ], [ %i364, %bb367 ]
  %i372 = phi i64 [ 1, %bb322 ], [ %i368, %bb367 ]
  %i373 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$DAMAGED_DEV_STRESS_TENSOR", i64 %i372)
  %i374 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i305, ptr elementtype(double) %i301, i64 %i372)
  %i375 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr nonnull elementtype(double) %i32, i64 %i372)
  br label %bb353

bb376:                                            ; preds = %bb367
  %i377 = fmul fast double %i364, 1.500000e+00
  %i378 = fdiv fast double %i377, %i292
  br label %bb776

bb379:                                            ; preds = %bb288
  %i380 = load ptr, ptr %i61, align 1
  %i381 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i63, i32 0)
  %i382 = load i64, ptr %i381, align 1
  %i383 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i63, i32 1)
  %i384 = load i64, ptr %i383, align 1
  %i385 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i384, ptr elementtype(double) %i380, i64 1)
  %i386 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i382, ptr elementtype(double) %i385, i64 1)
  %i387 = load double, ptr %i386, align 1
  %i388 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i384, ptr elementtype(double) %i380, i64 2)
  %i389 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i382, ptr elementtype(double) %i388, i64 2)
  %i390 = load double, ptr %i389, align 1
  %i391 = fadd fast double %i390, %i387
  %i392 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i384, ptr elementtype(double) %i380, i64 3)
  %i393 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i382, ptr elementtype(double) %i392, i64 3)
  %i394 = load double, ptr %i393, align 1
  %i395 = fadd fast double %i391, %i394
  %i396 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i62, i32 0)
  %i397 = ptrtoint ptr %i380 to i64
  br label %bb318

bb398:                                            ; preds = %bb410, %bb398
  %i399 = phi i64 [ 1, %bb410 ], [ %i405, %bb398 ]
  %i400 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i110, ptr elementtype(double) %i412, i64 %i399)
  %i402 = load i64, ptr %i400, align 1
  %i403 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i413, i64 %i399)
  store i64 %i402, ptr %i403, align 1
  %i405 = add nuw nsw i64 %i399, 1
  %i406 = icmp eq i64 %i405, 4
  br i1 %i406, label %bb407, label %bb398

bb407:                                            ; preds = %bb398
  %i408 = add nuw nsw i64 %i411, 1
  %i409 = icmp eq i64 %i408, 4
  br i1 %i409, label %bb426, label %bb410

bb410:                                            ; preds = %bb776, %bb407
  %i411 = phi i64 [ %i408, %bb407 ], [ 1, %bb776 ]
  %i412 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i112, ptr elementtype(double) %i108, i64 %i411)
  %i413 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$OLD_PLASTIC_STRAIN_TENSOR", i64 %i411)
  br label %bb398

bb414:                                            ; preds = %bb426, %bb414
  %i415 = phi double [ %i427, %bb426 ], [ %i420, %bb414 ]
  %i416 = phi i64 [ 1, %bb426 ], [ %i421, %bb414 ]
  %i417 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i429, i64 %i416)
  %i418 = load double, ptr %i417, align 1
  %i419 = fmul fast double %i418, %i418
  %i420 = fadd fast double %i419, %i415
  %i421 = add nuw nsw i64 %i416, 1
  %i422 = icmp eq i64 %i421, 4
  br i1 %i422, label %bb423, label %bb414

bb423:                                            ; preds = %bb414
  %i424 = add nuw nsw i64 %i428, 1
  %i425 = icmp eq i64 %i424, 4
  br i1 %i425, label %bb444, label %bb426

bb426:                                            ; preds = %bb423, %bb407
  %i427 = phi double [ %i420, %bb423 ], [ 0.000000e+00, %bb407 ]
  %i428 = phi i64 [ %i424, %bb423 ], [ 1, %bb407 ]
  %i429 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$DAMAGED_DEV_STRESS_TENSOR", i64 %i428)
  br label %bb414

bb430:                                            ; preds = %bb444, %bb430
  %i431 = phi double [ %i445, %bb444 ], [ %i438, %bb430 ]
  %i432 = phi i64 [ 1, %bb444 ], [ %i439, %bb430 ]
  %i433 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i447, i64 %i432)
  %i434 = load double, ptr %i433, align 1
  %i435 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i303, ptr elementtype(double) %i448, i64 %i432)
  %i436 = load double, ptr %i435, align 1
  %i437 = fmul fast double %i436, %i434
  %i438 = fadd fast double %i437, %i431
  %i439 = add nuw nsw i64 %i432, 1
  %i440 = icmp eq i64 %i439, 4
  br i1 %i440, label %bb441, label %bb430

bb441:                                            ; preds = %bb430
  %i442 = add nuw nsw i64 %i446, 1
  %i443 = icmp eq i64 %i442, 4
  br i1 %i443, label %bb449, label %bb444

bb444:                                            ; preds = %bb441, %bb423
  %i445 = phi double [ %i438, %bb441 ], [ 0.000000e+00, %bb423 ]
  %i446 = phi i64 [ %i442, %bb441 ], [ 1, %bb423 ]
  %i447 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$DAMAGED_DEV_STRESS_TENSOR", i64 %i446)
  %i448 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i305, ptr elementtype(double) %i301, i64 %i446)
  br label %bb430

bb449:                                            ; preds = %bb441
  %i450 = fmul fast double %i438, 2.000000e+00
  %i451 = load i64, ptr %i289, align 1
  %i452 = load i64, ptr %i290, align 1
  %i453 = icmp slt i64 %i452, 1
  %i454 = icmp slt i64 %i451, 1
  %i455 = or i1 %i453, %i454
  br i1 %i455, label %bb475, label %bb456

bb456:                                            ; preds = %bb449
  %i457 = add nuw nsw i64 %i451, 1
  %i458 = add nuw nsw i64 %i452, 1
  br label %bb471

bb459:                                            ; preds = %bb471, %bb459
  %i460 = phi double [ %i472, %bb471 ], [ %i465, %bb459 ]
  %i461 = phi i64 [ 1, %bb471 ], [ %i466, %bb459 ]
  %i462 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i303, ptr elementtype(double) %i474, i64 %i461)
  %i463 = load double, ptr %i462, align 1
  %i464 = fmul fast double %i463, %i463
  %i465 = fadd fast double %i464, %i460
  %i466 = add nuw nsw i64 %i461, 1
  %i467 = icmp eq i64 %i466, %i457
  br i1 %i467, label %bb468, label %bb459

bb468:                                            ; preds = %bb459
  %i469 = add nuw nsw i64 %i473, 1
  %i470 = icmp eq i64 %i469, %i458
  br i1 %i470, label %bb475, label %bb471

bb471:                                            ; preds = %bb468, %bb456
  %i472 = phi double [ %i465, %bb468 ], [ 0.000000e+00, %bb456 ]
  %i473 = phi i64 [ %i469, %bb468 ], [ 1, %bb456 ]
  %i474 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i305, ptr nonnull elementtype(double) %i301, i64 %i473)
  br label %bb459

bb475:                                            ; preds = %bb468, %bb449
  %i476 = phi double [ 0.000000e+00, %bb449 ], [ %i465, %bb468 ]
  %i477 = fmul fast double %i295, %i295
  %i478 = fmul fast double %i477, 0x3FE5555555555555
  %i479 = fsub fast double %i476, %i478
  %i480 = fmul fast double %i450, %i450
  %i481 = fmul fast double %i420, 4.000000e+00
  %i482 = fmul fast double %i481, %i479
  %i483 = fsub fast double %i480, %i482
  %i484 = fcmp fast olt double %i483, 0.000000e+00
  br i1 %i484, label %bb485, label %bb496

bb485:                                            ; preds = %bb475
  %i486 = getelementptr inbounds [4 x i8], ptr %i26, i64 0, i64 0
  store i8 56, ptr %i486, align 1
  %i487 = getelementptr inbounds [4 x i8], ptr %i26, i64 0, i64 1
  store i8 4, ptr %i487, align 1
  %i488 = getelementptr inbounds [4 x i8], ptr %i26, i64 0, i64 2
  store i8 1, ptr %i488, align 1
  %i489 = getelementptr inbounds [4 x i8], ptr %i26, i64 0, i64 3
  store i8 0, ptr %i489, align 1
  %i490 = getelementptr inbounds { i64, ptr }, ptr %i27, i64 0, i32 0
  store i64 43, ptr %i490, align 8
  %i491 = getelementptr inbounds { i64, ptr }, ptr %i27, i64 0, i32 1
  store ptr @anon.58475115b99d6a1547144f0b5d7ba7df.78, ptr %i491, align 8
  %i494 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i, i32 -1, i64 1239157112576, ptr nonnull %i486, ptr nonnull %i27)
  %i495 = call i32 (ptr, i32, i32, i64, i32, i32, ...) @for_stop_core_quiet(ptr @anon.58475115b99d6a1547144f0b5d7ba7df.120, i32 0, i32 0, i64 1239157112576, i32 0, i32 0)
  br label %bb496

bb496:                                            ; preds = %bb485, %bb475
  %i497 = call fast double @llvm.sqrt.f64(double %i483)
  %i498 = fadd fast double %i497, %i450
  %i499 = fmul fast double %i420, 2.000000e+00
  %i500 = fdiv fast double %i498, %i499
  %i501 = fcmp fast olt double %i500, 0.000000e+00
  %i502 = fcmp fast ogt double %i500, 1.000000e+00
  %i503 = or i1 %i501, %i502
  br i1 %i503, label %bb504, label %bb535

bb504:                                            ; preds = %bb496
  %i505 = getelementptr inbounds [4 x i8], ptr %i28, i64 0, i64 0
  store i8 56, ptr %i505, align 1
  %i506 = getelementptr inbounds [4 x i8], ptr %i28, i64 0, i64 1
  store i8 4, ptr %i506, align 1
  %i507 = getelementptr inbounds [4 x i8], ptr %i28, i64 0, i64 2
  store i8 2, ptr %i507, align 1
  %i508 = getelementptr inbounds [4 x i8], ptr %i28, i64 0, i64 3
  store i8 0, ptr %i508, align 1
  %i509 = getelementptr inbounds { i64, ptr }, ptr %i29, i64 0, i32 0
  store i64 46, ptr %i509, align 8
  %i510 = getelementptr inbounds { i64, ptr }, ptr %i29, i64 0, i32 1
  store ptr @anon.58475115b99d6a1547144f0b5d7ba7df.77, ptr %i510, align 8
  %i513 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i, i32 -1, i64 1239157112576, ptr nonnull %i505, ptr nonnull %i29)
  %i514 = getelementptr inbounds [4 x i8], ptr %i30, i64 0, i64 0
  store i8 48, ptr %i514, align 1
  %i515 = getelementptr inbounds [4 x i8], ptr %i30, i64 0, i64 1
  store i8 1, ptr %i515, align 1
  %i516 = getelementptr inbounds [4 x i8], ptr %i30, i64 0, i64 2
  store i8 1, ptr %i516, align 1
  %i517 = getelementptr inbounds [4 x i8], ptr %i30, i64 0, i64 3
  store i8 0, ptr %i517, align 1
  %i518 = getelementptr inbounds { double }, ptr %i31, i64 0, i32 0
  store double %i500, ptr %i518, align 8
  %i520 = call i32 @for_write_seq_lis_xmit(ptr nonnull %i, ptr nonnull %i514, ptr nonnull %i31)
  %i521 = call i32 (ptr, i32, i32, i64, i32, i32, ...) @for_stop_core_quiet(ptr @anon.58475115b99d6a1547144f0b5d7ba7df.120, i32 0, i32 0, i64 1239157112576, i32 0, i32 0)
  br label %bb535

bb522:                                            ; preds = %bb532, %bb522
  %i523 = phi i64 [ 1, %bb532 ], [ %i527, %bb522 ]
  %i524 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i534, i64 %i523)
  %i525 = load double, ptr %i524, align 1
  %i526 = fmul fast double %i525, %i500
  store double %i526, ptr %i524, align 1
  %i527 = add nuw nsw i64 %i523, 1
  %i528 = icmp eq i64 %i527, 4
  br i1 %i528, label %bb529, label %bb522

bb529:                                            ; preds = %bb522
  %i530 = add nuw nsw i64 %i533, 1
  %i531 = icmp eq i64 %i530, 4
  br i1 %i531, label %bb548, label %bb532

bb532:                                            ; preds = %bb535, %bb529
  %i533 = phi i64 [ 1, %bb535 ], [ %i530, %bb529 ]
  %i534 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$DEVIATORIC_STRESS_TENSOR", i64 %i533)
  br label %bb522

bb535:                                            ; preds = %bb504, %bb496
  br label %bb532

bb536:                                            ; preds = %bb548, %bb536
  %i537 = phi i64 [ 1, %bb548 ], [ %i543, %bb536 ]
  %i538 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i550, i64 %i537)
  %i540 = load i64, ptr %i538, align 1
  %i541 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i89, ptr elementtype(double) %i551, i64 %i537)
  store i64 %i540, ptr %i541, align 1
  %i543 = add nuw nsw i64 %i537, 1
  %i544 = icmp eq i64 %i543, 4
  br i1 %i544, label %bb545, label %bb536

bb545:                                            ; preds = %bb536
  %i546 = add nuw nsw i64 %i549, 1
  %i547 = icmp eq i64 %i546, 4
  br i1 %i547, label %bb552, label %bb548

bb548:                                            ; preds = %bb545, %bb529
  %i549 = phi i64 [ %i546, %bb545 ], [ 1, %bb529 ]
  %i550 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$DEVIATORIC_STRESS_TENSOR", i64 %i549)
  %i551 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i92, ptr elementtype(double) %i87, i64 %i549)
  br label %bb536

bb552:                                            ; preds = %bb545
  %i553 = load double, ptr %i172, align 1
  %i554 = fadd fast double %i553, %i239
  store double %i554, ptr %i172, align 1
  %i555 = load double, ptr %i175, align 1
  %i556 = fadd fast double %i555, %i239
  store double %i556, ptr %i175, align 1
  %i557 = load double, ptr %i179, align 1
  %i558 = fadd fast double %i557, %i239
  store double %i558, ptr %i179, align 1
  %i559 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i103, ptr elementtype(double) %i95, i64 1)
  %i560 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i97, ptr elementtype(double) %i559, i64 1)
  %i561 = load double, ptr %i560, align 1
  %i562 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i103, ptr elementtype(double) %i95, i64 2)
  %i563 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i97, ptr elementtype(double) %i562, i64 2)
  %i564 = load double, ptr %i563, align 1
  %i565 = fadd fast double %i564, %i561
  %i566 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i103, ptr elementtype(double) %i95, i64 3)
  %i567 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i97, ptr elementtype(double) %i566, i64 3)
  %i568 = load double, ptr %i567, align 1
  %i569 = fadd fast double %i565, %i568
  br label %bb582

bb570:                                            ; preds = %bb582, %bb570
  %i571 = phi i64 [ 1, %bb582 ], [ %i577, %bb570 ]
  %i572 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i97, ptr elementtype(double) %i584, i64 %i571)
  %i574 = load i64, ptr %i572, align 1
  %i575 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i585, i64 %i571)
  store i64 %i574, ptr %i575, align 1
  %i577 = add nuw nsw i64 %i571, 1
  %i578 = icmp eq i64 %i577, 4
  br i1 %i578, label %bb579, label %bb570

bb579:                                            ; preds = %bb570
  %i580 = add nuw nsw i64 %i583, 1
  %i581 = icmp eq i64 %i580, 4
  br i1 %i581, label %bb586, label %bb582

bb582:                                            ; preds = %bb579, %bb552
  %i583 = phi i64 [ 1, %bb552 ], [ %i580, %bb579 ]
  %i584 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i103, ptr nonnull elementtype(double) %i95, i64 %i583)
  %i585 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$DEVIATORIC_STRAIN_TENSOR", i64 %i583)
  br label %bb570

bb586:                                            ; preds = %bb579
  %i587 = fmul fast double %i569, 0x3FD5555555555555
  %i588 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$DEVIATORIC_STRAIN_TENSOR", i64 1)
  %i589 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i588, i64 1)
  %i590 = load double, ptr %i589, align 1
  %i591 = fsub fast double %i590, %i587
  store double %i591, ptr %i589, align 1
  %i592 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$DEVIATORIC_STRAIN_TENSOR", i64 2)
  %i593 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i592, i64 2)
  %i594 = load double, ptr %i593, align 1
  %i595 = fsub fast double %i594, %i587
  store double %i595, ptr %i593, align 1
  %i596 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$DEVIATORIC_STRAIN_TENSOR", i64 3)
  %i597 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i596, i64 3)
  %i598 = load double, ptr %i597, align 1
  %i599 = fsub fast double %i598, %i587
  store double %i599, ptr %i597, align 1
  br label %bb614

bb600:                                            ; preds = %bb614, %bb600
  %i601 = phi i64 [ 1, %bb614 ], [ %i609, %bb600 ]
  %i602 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i110, ptr elementtype(double) %i616, i64 %i601)
  %i603 = load double, ptr %i602, align 1
  %i604 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i617, i64 %i601)
  %i605 = load double, ptr %i604, align 1
  %i606 = fsub fast double %i603, %i605
  %i607 = fmul fast double %i606, %i500
  %i608 = fadd fast double %i607, %i605
  store double %i608, ptr %i602, align 1
  %i609 = add nuw nsw i64 %i601, 1
  %i610 = icmp eq i64 %i609, 4
  br i1 %i610, label %bb611, label %bb600

bb611:                                            ; preds = %bb600
  %i612 = add nuw nsw i64 %i615, 1
  %i613 = icmp eq i64 %i612, 4
  br i1 %i613, label %bb618, label %bb614

bb614:                                            ; preds = %bb611, %bb586
  %i615 = phi i64 [ 1, %bb586 ], [ %i612, %bb611 ]
  %i616 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i112, ptr elementtype(double) %i108, i64 %i615)
  %i617 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$DEVIATORIC_STRAIN_TENSOR", i64 %i615)
  br label %bb600

bb618:                                            ; preds = %bb611
  %i619 = fsub fast double 1.000000e+00, %i500
  %i620 = load double, ptr %arg, align 1
  %i621 = fmul fast double %i620, %i619
  %i622 = fcmp fast ugt double %i621, 0.000000e+00
  br i1 %i622, label %bb648, label %bb631

bb623:                                            ; preds = %bb631, %bb623
  %i624 = phi i64 [ 1, %bb631 ], [ %i626, %bb623 ]
  %i625 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i633, i64 %i624)
  store double 0.000000e+00, ptr %i625, align 1
  %i626 = add nuw nsw i64 %i624, 1
  %i627 = icmp eq i64 %i626, 4
  br i1 %i627, label %bb628, label %bb623

bb628:                                            ; preds = %bb623
  %i629 = add nuw nsw i64 %i632, 1
  %i630 = icmp eq i64 %i629, 4
  br i1 %i630, label %bb680, label %bb631

bb631:                                            ; preds = %bb628, %bb618
  %i632 = phi i64 [ %i629, %bb628 ], [ 1, %bb618 ]
  %i633 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$PLASTIC_STRAIN_RATE_TENSOR", i64 %i632)
  br label %bb623

bb634:                                            ; preds = %bb648, %bb634
  %i635 = phi i64 [ 1, %bb648 ], [ %i643, %bb634 ]
  %i636 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i110, ptr elementtype(double) %i650, i64 %i635)
  %i637 = load double, ptr %i636, align 1
  %i638 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i651, i64 %i635)
  %i639 = load double, ptr %i638, align 1
  %i640 = fsub fast double %i637, %i639
  %i641 = fdiv fast double %i640, %i621
  %i642 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i652, i64 %i635)
  store double %i641, ptr %i642, align 1
  %i643 = add nuw nsw i64 %i635, 1
  %i644 = icmp eq i64 %i643, 4
  br i1 %i644, label %bb645, label %bb634

bb645:                                            ; preds = %bb634
  %i646 = add nuw nsw i64 %i649, 1
  %i647 = icmp eq i64 %i646, 4
  br i1 %i647, label %bb680, label %bb648

bb648:                                            ; preds = %bb645, %bb618
  %i649 = phi i64 [ %i646, %bb645 ], [ 1, %bb618 ]
  %i650 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i112, ptr nonnull elementtype(double) %i108, i64 %i649)
  %i651 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$OLD_PLASTIC_STRAIN_TENSOR", i64 %i649)
  %i652 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$PLASTIC_STRAIN_RATE_TENSOR", i64 %i649)
  br label %bb634

bb653:                                            ; preds = %bb665, %bb653
  %i654 = phi double [ %i666, %bb665 ], [ %i659, %bb653 ]
  %i655 = phi i64 [ 1, %bb665 ], [ %i660, %bb653 ]
  %i656 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i668, i64 %i655)
  %i657 = load double, ptr %i656, align 1
  %i658 = fmul fast double %i657, %i657
  %i659 = fadd fast double %i658, %i654
  %i660 = add nuw nsw i64 %i655, 1
  %i661 = icmp eq i64 %i660, 4
  br i1 %i661, label %bb662, label %bb653

bb662:                                            ; preds = %bb653
  %i663 = add nuw nsw i64 %i667, 1
  %i664 = icmp eq i64 %i663, 4
  br i1 %i664, label %bb669, label %bb665

bb665:                                            ; preds = %bb680, %bb662
  %i666 = phi double [ 0.000000e+00, %bb680 ], [ %i659, %bb662 ]
  %i667 = phi i64 [ 1, %bb680 ], [ %i663, %bb662 ]
  %i668 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$PLASTIC_STRAIN_RATE_TENSOR", i64 %i667)
  br label %bb653

bb669:                                            ; preds = %bb662
  %i670 = fmul fast double %i659, 0x3FE5555555555555
  %i671 = call fast double @llvm.sqrt.f64(double %i670)
  %i672 = fmul fast double %i671, %i252
  %i673 = load double, ptr %arg14, align 1
  %i674 = fmul fast double %i671, %i621
  %i675 = fadd fast double %i673, %i674
  store double %i675, ptr %arg14, align 1
  %i676 = load double, ptr %arg7, align 1
  %i677 = load double, ptr %arg6, align 1
  %i678 = fmul fast double %i252, 0x3FE5555555555555
  %i679 = fmul fast double %i678, %i677
  br label %bb697

bb680:                                            ; preds = %bb645, %bb628
  br label %bb665

bb681:                                            ; preds = %bb697, %bb681
  %i682 = phi i64 [ 1, %bb697 ], [ %i692, %bb681 ]
  %i683 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i699, i64 %i682)
  %i684 = load double, ptr %i683, align 1
  %i685 = fmul fast double %i679, %i684
  %i686 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i303, ptr elementtype(double) %i700, i64 %i682)
  %i687 = load double, ptr %i686, align 1
  %i688 = fmul fast double %i687, %i672
  %i689 = fsub fast double %i685, %i688
  %i690 = fmul fast double %i689, %i676
  %i691 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i701, i64 %i682)
  store double %i690, ptr %i691, align 1
  %i692 = add nuw nsw i64 %i682, 1
  %i693 = icmp eq i64 %i692, 4
  br i1 %i693, label %bb694, label %bb681

bb694:                                            ; preds = %bb681
  %i695 = add nuw nsw i64 %i698, 1
  %i696 = icmp eq i64 %i695, 4
  br i1 %i696, label %bb702, label %bb697

bb697:                                            ; preds = %bb694, %bb669
  %i698 = phi i64 [ 1, %bb669 ], [ %i695, %bb694 ]
  %i699 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$PLASTIC_STRAIN_RATE_TENSOR", i64 %i698)
  %i700 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i305, ptr elementtype(double) %i301, i64 %i698)
  %i701 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$BACK_STRESS_RATE_TENSOR", i64 %i698)
  br label %bb681

bb702:                                            ; preds = %bb694
  %i703 = load double, ptr %arg5, align 1
  %i704 = load double, ptr %arg4, align 1
  br label %bb718

bb705:                                            ; preds = %bb718, %bb705
  %i706 = phi i64 [ 1, %bb718 ], [ %i713, %bb705 ]
  %i707 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i303, ptr elementtype(double) %i720, i64 %i706)
  %i708 = load double, ptr %i707, align 1
  %i709 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i721, i64 %i706)
  %i710 = load double, ptr %i709, align 1
  %i711 = fmul fast double %i710, %i621
  %i712 = fadd fast double %i711, %i708
  store double %i712, ptr %i707, align 1
  %i713 = add nuw nsw i64 %i706, 1
  %i714 = icmp eq i64 %i713, 4
  br i1 %i714, label %bb715, label %bb705

bb715:                                            ; preds = %bb705
  %i716 = add nuw nsw i64 %i719, 1
  %i717 = icmp eq i64 %i716, 4
  br i1 %i717, label %bb722, label %bb718

bb718:                                            ; preds = %bb715, %bb702
  %i719 = phi i64 [ 1, %bb702 ], [ %i716, %bb715 ]
  %i720 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i305, ptr nonnull elementtype(double) %i301, i64 %i719)
  %i721 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$BACK_STRESS_RATE_TENSOR", i64 %i719)
  br label %bb705

bb722:                                            ; preds = %bb715
  %i723 = fsub fast double %i704, %i293
  %i724 = fmul fast double %i672, %i621
  %i725 = fmul fast double %i724, %i703
  %i726 = fmul fast double %i725, %i723
  %i727 = fadd fast double %i726, %i293
  store double %i727, ptr %arg16, align 1
  %i728 = fcmp fast ogt double %i239, -1.000000e-10
  br i1 %i728, label %bb741, label %bb781

bb729:                                            ; preds = %bb741, %bb729
  %i730 = phi double [ %i742, %bb741 ], [ %i735, %bb729 ]
  %i731 = phi i64 [ 1, %bb741 ], [ %i736, %bb729 ]
  %i732 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i744, i64 %i731)
  %i733 = load double, ptr %i732, align 1
  %i734 = fmul fast double %i733, %i733
  %i735 = fadd fast double %i734, %i730
  %i736 = add nuw nsw i64 %i731, 1
  %i737 = icmp eq i64 %i736, 4
  br i1 %i737, label %bb738, label %bb729

bb738:                                            ; preds = %bb729
  %i739 = add nuw nsw i64 %i743, 1
  %i740 = icmp eq i64 %i739, 4
  br i1 %i740, label %bb745, label %bb741

bb741:                                            ; preds = %bb738, %bb722
  %i742 = phi double [ %i735, %bb738 ], [ 0.000000e+00, %bb722 ]
  %i743 = phi i64 [ %i739, %bb738 ], [ 1, %bb722 ]
  %i744 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 24, ptr elementtype(double) @"perdida_m_mp_perdida_$DAMAGED_DEV_STRESS_TENSOR", i64 %i743)
  br label %bb729

bb745:                                            ; preds = %bb738
  %i746 = load double, ptr %arg9, align 1
  %i747 = fcmp fast olt double %i675, %i746
  br i1 %i747, label %bb767, label %bb748

bb748:                                            ; preds = %bb745
  %i749 = fmul fast double %i83, 0x3FE5555555555555
  %i750 = fadd fast double %i749, 0x3FE5555555555555
  %i751 = fmul fast double %i85, 3.000000e+00
  %i752 = fmul fast double %i239, %i239
  %i753 = fmul fast double %i752, %i751
  %i754 = fmul fast double %i735, 1.500000e+00
  %i755 = fdiv fast double %i753, %i754
  %i756 = fadd fast double %i750, %i755
  %i757 = fmul fast double %i79, 3.000000e+00
  %i758 = fmul fast double %i81, 2.000000e+00
  %i759 = fadd fast double %i758, %i757
  %i760 = fmul fast double %i85, %i759
  %i761 = load double, ptr %arg8, align 1
  %i762 = fmul fast double %i671, 7.500000e-01
  %i763 = fmul fast double %i762, %i735
  %i764 = fmul fast double %i763, %i756
  %i765 = fmul fast double %i760, %i761
  %i766 = fdiv fast double %i764, %i765
  br label %bb767

bb767:                                            ; preds = %bb748, %bb745
  %i768 = phi double [ %i766, %bb748 ], [ 0.000000e+00, %bb745 ]
  %i769 = fmul fast double %i768, %i621
  %i770 = fadd fast double %i94, %i769
  %i771 = fcmp fast oge double %i770, 1.000000e+00
  %i772 = select fast i1 %i771, double 1.000000e+00, double %i770
  %i773 = load double, ptr %arg18, align 1
  %i774 = fcmp fast oge double %i772, %i773
  %i775 = select i1 %i774, double 1.000000e+00, double %i772
  store double %i775, ptr %arg17, align 1
  ret void

bb776:                                            ; preds = %bb376, %bb288
  %i777 = phi double [ %i378, %bb376 ], [ -1.000000e+00, %bb288 ]
  %i778 = fcmp fast ole double %i296, %i777
  %i779 = select i1 %i778, double %i296, double %i777
  %i780 = fcmp fast ult double %i779, 0.000000e+00
  br i1 %i780, label %bb781, label %bb410

bb781:                                            ; preds = %bb776, %bb722
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare double @llvm.sqrt.f64(double) #2

define dso_local void @MAIN__() #1 {
bb1:
  %t11 = alloca double, align 8
  %t12 = alloca double, align 8
  %t17 = alloca double, align 8
  %t18 = alloca double, align 8
  %t19 = alloca double, align 8
  %t20 = alloca double, align 8
  %t21 = alloca double, align 8
  %t22 = alloca double, align 8
  %t23 = alloca double, align 8
  %t24 = alloca double, align 8
  %t25 = alloca double, align 8
  %t26 = alloca double, align 8
  %t52 = alloca %"QNCA_a0$double*$rank2$", align 8
  %t53 = alloca %"QNCA_a0$double*$rank2$", align 8
  %t54 = alloca %"QNCA_a0$double*$rank2$", align 8
  %t55 = alloca %"QNCA_a0$double*$rank2$", align 8
  %t56 = alloca %"QNCA_a0$double*$rank2$", align 8
  %t661 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$double*$rank1$", ptr @"iztaccihuatl_$DAMAGE", i64 0, i32 6, i64 0, i32 2), i32 0)
  %t684 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$double*$rank1$", ptr @"iztaccihuatl_$ACCUMULATED_PLASTIC_STRAIN", i64 0, i32 6, i64 0, i32 2), i32 0)
  %t707 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$double*$rank1$", ptr @"iztaccihuatl_$ISOTROPIC_HARDENING_STRESS", i64 0, i32 6, i64 0, i32 2), i32 0)
  %t2092 = load ptr, ptr @"iztaccihuatl_$ACCUMULATED_PLASTIC_STRAIN", align 16
  %t2093 = load i64, ptr %t684, align 1
  %t2094 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %t2093, i64 8, ptr elementtype(double) %t2092, i64 1)
  %t2105 = load ptr, ptr @"iztaccihuatl_$ISOTROPIC_HARDENING_STRESS", align 16
  %t2106 = load i64, ptr %t707, align 1
  %t2107 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %t2106, i64 8, ptr elementtype(double) %t2105, i64 1)
  %t2108 = load ptr, ptr @"iztaccihuatl_$DAMAGE", align 16
  %t2109 = load i64, ptr %t661, align 1
  %t2110 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %t2109, i64 8, ptr elementtype(double) %t2108, i64 1)
  call fastcc void @perdida_m_mp_perdida_(ptr nonnull %t26, ptr nonnull %t24, ptr nonnull %t23, ptr nonnull %t25, ptr nonnull %t22, ptr nonnull %t21, ptr nonnull %t20, ptr nonnull %t19, ptr nonnull %t18, ptr nonnull %t17, ptr nonnull %t52, ptr nonnull %t53, ptr nonnull %t54, ptr nonnull %t55, ptr %t2094, ptr nonnull %t56, ptr %t2107, ptr %t2110, ptr nonnull %t12, ptr nonnull %t11)
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #3

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #3

attributes #0 = { nocallback nofree nosync nounwind willreturn }
attributes #1 = { "intel-lang"="fortran" }
attributes #2 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #3 = { nounwind readnone speculatable }
; end INTEL_FEATURE_SW_ADVANCED

; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -opaque-pointers -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe807 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-BEFORE
; Inline report via metadata
; RUN: opt -opaque-pointers -inlinereportsetup -inline-report=0xe886 < %s -S | opt -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe886 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-AFTER

; Check that all instances of @sw_IP_ddx_ and @sw_IP_ddy_ are inlined
; due to the inline budget and single callsite local linkage heuristics.

%uplevel_type = type { i32, i32 }
%"QNCA_a0$double*$rank2$.3" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

; CHECK-BEFORE-NOT: call{{.*}}sw_IP_ddx_
; CHECK-BEFORE-NOT: call{{.*}}sw_IP_ddy_
; CHECK-DAG: INLINE: sw_IP_ddx_{{.*}}Has inline budget for small application
; CHECK-DAG: INLINE: sw_IP_ddy_{{.*}}Has inline budget for small application
; CHECK-DAG: INLINE: sw_IP_ddx_{{.*}}Callee has single callsite and local linkage
; CHECK-DAG: INLINE: sw_IP_ddy_{{.*}}Callee has single callsite and local linkage
; CHECK-AFTER-NOT: call{{.*}}sw_IP_ddx_
; CHECK-AFTER-NOT: call{{.*}}sw_IP_ddy_

define internal fastcc void @sw_IP_ddx_(ptr noalias nocapture %arg, ptr noalias nocapture readonly dereferenceable(96) %arg1, ptr noalias nocapture readonly dereferenceable(96) "ptrnoalias" %arg2) unnamed_addr #0 {
bb:
  %i = getelementptr inbounds %uplevel_type, ptr %arg, i64 0, i32 0
  %i3 = getelementptr inbounds %uplevel_type, ptr %arg, i64 0, i32 1
  %i4 = getelementptr inbounds %"QNCA_a0$double*$rank2$.3", ptr %arg2, i64 0, i32 0
  %i5 = getelementptr inbounds %"QNCA_a0$double*$rank2$.3", ptr %arg2, i64 0, i32 6, i64 0, i32 0
  %i6 = getelementptr inbounds %"QNCA_a0$double*$rank2$.3", ptr %arg2, i64 0, i32 6, i64 0, i32 1
  %i7 = getelementptr inbounds %"QNCA_a0$double*$rank2$.3", ptr %arg1, i64 0, i32 0
  %i8 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i5, i32 0)
  %i9 = load i64, ptr %i8, align 1
  %i10 = icmp sgt i64 %i9, 0
  %i11 = select i1 %i10, i64 %i9, i64 0
  %i12 = trunc i64 %i11 to i32
  %i13 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i5, i32 1)
  store i32 %i12, ptr %i, align 1
  %i14 = load i64, ptr %i13, align 1
  %i15 = icmp sgt i64 %i14, 0
  %i16 = select i1 %i15, i64 %i14, i64 0
  %i17 = trunc i64 %i16 to i32
  store i32 %i17, ptr %i3, align 1
  %i18 = load ptr, ptr %i7, align 1
  %i19 = shl i64 %i11, 32
  %i20 = ashr exact i64 %i19, 32
  %i21 = ashr exact i64 %i19, 29
  %i22 = load ptr, ptr %i4, align 1
  %i23 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i6, i32 0)
  %i24 = load i64, ptr %i23, align 1
  %i25 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i6, i32 1)
  %i26 = load i64, ptr %i25, align 1
  %i27 = icmp slt i32 %i17, 1
  br i1 %i27, label %bb89, label %bb28

bb28:                                             ; preds = %bb
  %i29 = icmp slt i64 %i20, 3
  br i1 %i29, label %bb69, label %bb30

bb30:                                             ; preds = %bb28
  %i31 = add nuw nsw i64 %i20, 1
  %i32 = shl i64 %i16, 32
  %i33 = ashr exact i64 %i32, 32
  %i34 = add nsw i64 %i33, 1
  br label %bb52

bb35:                                             ; preds = %bb52, %bb35
  %i36 = phi i64 [ 3, %bb52 ], [ %i45, %bb35 ]
  %i37 = phi i64 [ 2, %bb52 ], [ %i46, %bb35 ]
  %i38 = phi i64 [ 1, %bb52 ], [ %i47, %bb35 ]
  %i39 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i24, ptr elementtype(double) %i54, i64 %i36)
  %i40 = load double, ptr %i39, align 1
  %i41 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i24, ptr elementtype(double) %i54, i64 %i38)
  %i42 = load double, ptr %i41, align 1
  %i43 = fsub fast double %i40, %i42
  %i44 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i55, i64 %i37)
  store double %i43, ptr %i44, align 1
  %i45 = add nuw nsw i64 %i36, 1
  %i46 = add nuw nsw i64 %i37, 1
  %i47 = add nuw nsw i64 %i38, 1
  %i48 = icmp eq i64 %i45, %i31
  br i1 %i48, label %bb49, label %bb35

bb49:                                             ; preds = %bb35
  %i50 = add nuw nsw i64 %i53, 1
  %i51 = icmp eq i64 %i50, %i34
  br i1 %i51, label %bb69, label %bb52

bb52:                                             ; preds = %bb49, %bb30
  %i53 = phi i64 [ %i50, %bb49 ], [ 1, %bb30 ]
  %i54 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i26, ptr elementtype(double) %i22, i64 %i53)
  %i55 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i21, ptr elementtype(double) %i18, i64 %i53)
  br label %bb35

bb56:                                             ; preds = %bb69, %bb56
  %i57 = phi i64 [ %i67, %bb56 ], [ 1, %bb69 ]
  %i58 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i26, ptr elementtype(double) %i22, i64 %i57)
  %i59 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i24, ptr elementtype(double) %i58, i64 2)
  %i60 = load double, ptr %i59, align 1
  %i61 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i24, ptr elementtype(double) %i58, i64 1)
  %i62 = load double, ptr %i61, align 1
  %i63 = fsub fast double %i60, %i62
  %i64 = fmul fast double %i63, 2.000000e+00
  %i65 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i21, ptr elementtype(double) %i18, i64 %i57)
  %i66 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i65, i64 1)
  store double %i64, ptr %i66, align 1
  %i67 = add nuw nsw i64 %i57, 1
  %i68 = icmp eq i64 %i67, %i72
  br i1 %i68, label %bb86, label %bb56

bb69:                                             ; preds = %bb49, %bb28
  %i70 = shl i64 %i16, 32
  %i71 = ashr exact i64 %i70, 32
  %i72 = add nsw i64 %i71, 1
  br label %bb56

bb73:                                             ; preds = %bb86, %bb73
  %i74 = phi i64 [ %i84, %bb73 ], [ 1, %bb86 ]
  %i75 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i26, ptr nonnull elementtype(double) %i22, i64 %i74)
  %i76 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i24, ptr elementtype(double) %i75, i64 %i20)
  %i77 = load double, ptr %i76, align 1
  %i78 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i24, ptr elementtype(double) %i75, i64 %i88)
  %i79 = load double, ptr %i78, align 1
  %i80 = fsub fast double %i77, %i79
  %i81 = fmul fast double %i80, 2.000000e+00
  %i82 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i21, ptr nonnull elementtype(double) %i18, i64 %i74)
  %i83 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i82, i64 %i20)
  store double %i81, ptr %i83, align 1
  %i84 = add nuw nsw i64 %i74, 1
  %i85 = icmp eq i64 %i84, %i72
  br i1 %i85, label %bb89, label %bb73

bb86:                                             ; preds = %bb56
  %i87 = add i64 %i19, -4294967296
  %i88 = ashr exact i64 %i87, 32
  br label %bb73

bb89:                                             ; preds = %bb73, %bb
  ret void
}

define internal fastcc void @sw_IP_ddy_(ptr noalias nocapture %arg, ptr noalias nocapture readonly dereferenceable(96) %arg1, ptr noalias nocapture readonly dereferenceable(96) "ptrnoalias" %arg2) unnamed_addr #0 {
bb:
  %i = getelementptr inbounds %uplevel_type, ptr %arg, i64 0, i32 0
  %i3 = getelementptr inbounds %uplevel_type, ptr %arg, i64 0, i32 1
  %i4 = getelementptr inbounds %"QNCA_a0$double*$rank2$.3", ptr %arg2, i64 0, i32 0
  %i5 = getelementptr inbounds %"QNCA_a0$double*$rank2$.3", ptr %arg2, i64 0, i32 6, i64 0, i32 0
  %i6 = getelementptr inbounds %"QNCA_a0$double*$rank2$.3", ptr %arg2, i64 0, i32 6, i64 0, i32 1
  %i7 = getelementptr inbounds %"QNCA_a0$double*$rank2$.3", ptr %arg1, i64 0, i32 0
  %i8 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i5, i32 0)
  %i9 = load i64, ptr %i8, align 1
  %i10 = icmp sgt i64 %i9, 0
  %i11 = select i1 %i10, i64 %i9, i64 0
  %i12 = trunc i64 %i11 to i32
  %i13 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i5, i32 1)
  store i32 %i12, ptr %i, align 1
  %i14 = load i64, ptr %i13, align 1
  %i15 = icmp sgt i64 %i14, 0
  %i16 = select i1 %i15, i64 %i14, i64 0
  %i17 = trunc i64 %i16 to i32
  store i32 %i17, ptr %i3, align 1
  %i18 = load ptr, ptr %i7, align 1
  %i19 = shl i64 %i11, 32
  %i20 = ashr exact i64 %i19, 32
  %i21 = ashr exact i64 %i19, 29
  %i22 = shl i64 %i16, 32
  %i23 = add i64 %i22, -4294967296
  %i24 = ashr exact i64 %i23, 32
  %i25 = load ptr, ptr %i4, align 1
  %i26 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i6, i32 0)
  %i27 = load i64, ptr %i26, align 1
  %i28 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i6, i32 1)
  %i29 = load i64, ptr %i28, align 1
  %i30 = icmp sgt i64 %i24, 1
  br i1 %i30, label %bb31, label %bb58

bb31:                                             ; preds = %bb
  %i32 = icmp slt i64 %i20, 1
  br i1 %i32, label %bb92, label %bb33

bb33:                                             ; preds = %bb31
  %i34 = add nsw i64 %i20, 1
  %i35 = add nuw nsw i64 %i24, 2
  br label %bb51

bb36:                                             ; preds = %bb51, %bb36
  %i37 = phi i64 [ 1, %bb51 ], [ %i44, %bb36 ]
  %i38 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i27, ptr elementtype(double) %i55, i64 %i37)
  %i39 = load double, ptr %i38, align 1
  %i40 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i27, ptr elementtype(double) %i56, i64 %i37)
  %i41 = load double, ptr %i40, align 1
  %i42 = fsub fast double %i39, %i41
  %i43 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i57, i64 %i37)
  store double %i42, ptr %i43, align 1
  %i44 = add nuw nsw i64 %i37, 1
  %i45 = icmp eq i64 %i44, %i34
  br i1 %i45, label %bb46, label %bb36

bb46:                                             ; preds = %bb36
  %i47 = add nuw nsw i64 %i52, 1
  %i48 = add nuw nsw i64 %i53, 1
  %i49 = add nuw nsw i64 %i54, 1
  %i50 = icmp eq i64 %i47, %i35
  br i1 %i50, label %bb58, label %bb51

bb51:                                             ; preds = %bb46, %bb33
  %i52 = phi i64 [ %i47, %bb46 ], [ 3, %bb33 ]
  %i53 = phi i64 [ %i48, %bb46 ], [ 2, %bb33 ]
  %i54 = phi i64 [ %i49, %bb46 ], [ 1, %bb33 ]
  %i55 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i29, ptr elementtype(double) %i25, i64 %i52)
  %i56 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i29, ptr elementtype(double) %i25, i64 %i54)
  %i57 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i21, ptr elementtype(double) %i18, i64 %i53)
  br label %bb36

bb58:                                             ; preds = %bb46, %bb
  %i59 = icmp slt i64 %i20, 1
  br i1 %i59, label %bb92, label %bb60

bb60:                                             ; preds = %bb58
  %i61 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i29, ptr elementtype(double) %i25, i64 2)
  %i62 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i29, ptr elementtype(double) %i25, i64 1)
  %i63 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i21, ptr elementtype(double) %i18, i64 1)
  %i64 = add nsw i64 %i20, 1
  br label %bb65

bb65:                                             ; preds = %bb65, %bb60
  %i66 = phi i64 [ 1, %bb60 ], [ %i74, %bb65 ]
  %i67 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i27, ptr elementtype(double) %i61, i64 %i66)
  %i68 = load double, ptr %i67, align 1
  %i69 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i27, ptr elementtype(double) %i62, i64 %i66)
  %i70 = load double, ptr %i69, align 1
  %i71 = fsub fast double %i68, %i70
  %i72 = fmul fast double %i71, 2.000000e+00
  %i73 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i63, i64 %i66)
  store double %i72, ptr %i73, align 1
  %i74 = add nuw nsw i64 %i66, 1
  %i75 = icmp eq i64 %i74, %i64
  br i1 %i75, label %bb87, label %bb65

bb76:                                             ; preds = %bb87, %bb76
  %i77 = phi i64 [ 1, %bb87 ], [ %i85, %bb76 ]
  %i78 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i27, ptr elementtype(double) %i89, i64 %i77)
  %i79 = load double, ptr %i78, align 1
  %i80 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i27, ptr elementtype(double) %i90, i64 %i77)
  %i81 = load double, ptr %i80, align 1
  %i82 = fsub fast double %i79, %i81
  %i83 = fmul fast double %i82, 2.000000e+00
  %i84 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i91, i64 %i77)
  store double %i83, ptr %i84, align 1
  %i85 = add nuw nsw i64 %i77, 1
  %i86 = icmp eq i64 %i85, %i64
  br i1 %i86, label %bb92, label %bb76

bb87:                                             ; preds = %bb65
  %i88 = ashr exact i64 %i22, 32
  %i89 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i29, ptr nonnull elementtype(double) %i25, i64 %i88)
  %i90 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i29, ptr nonnull elementtype(double) %i25, i64 %i24)
  %i91 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i21, ptr nonnull elementtype(double) %i18, i64 %i88)
  br label %bb76

bb92:                                             ; preds = %bb76, %bb58, %bb31
  ret void
}

define dso_local void @MAIN__() local_unnamed_addr #0 {
entry:
  %t2 = alloca %uplevel_type, align 8
  %t176 = alloca %"QNCA_a0$double*$rank2$.3", align 8
  %t177 = alloca %"QNCA_a0$double*$rank2$.3", align 8
  %t178 = alloca %"QNCA_a0$double*$rank2$.3", align 8
  %t179 = alloca %"QNCA_a0$double*$rank2$.3", align 8
  %t180 = alloca %"QNCA_a0$double*$rank2$.3", align 8
  %t181 = alloca %"QNCA_a0$double*$rank2$.3", align 8
  %t182 = alloca %"QNCA_a0$double*$rank2$.3", align 8
  %t183 = alloca %"QNCA_a0$double*$rank2$.3", align 8
  br label %L1161

L1161:                                            ; preds = %L1161, %entry
  %t1164 = phi i32 [ 1, %entry ], [ %t1462, %L1161 ]
  call fastcc void @sw_IP_ddx_(ptr nonnull %t2, ptr nonnull %t177, ptr nonnull %t176)
  call fastcc void @sw_IP_ddy_(ptr nonnull %t2, ptr nonnull %t179, ptr nonnull %t178)
  call fastcc void @sw_IP_ddx_(ptr nonnull %t2, ptr nonnull %t181, ptr nonnull %t180)
  call fastcc void @sw_IP_ddy_(ptr nonnull %t2, ptr nonnull %t183, ptr nonnull %t182)
  %t1462 = add nuw nsw i32 %t1164, 1
  %t1463 = icmp eq i32 %t1462, 6481
  br i1 %t1463, label %L1464, label %L1161

L1464:                                            ; preds = %L1161
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #1

attributes #0 = { "intel-lang"="fortran" }
attributes #1 = { nounwind readnone speculatable }

; end INTEL_FEATURE_SW_ADVANCED

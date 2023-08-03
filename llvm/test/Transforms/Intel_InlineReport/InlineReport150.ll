; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -passes='cgscc(inline)' -pre-lto-inline-cost -inlining-dyn-alloca-special-arg-count=3 < %s -S 2>&1 | opt -passes='cgscc(inline)' -inline-report=0xe807 -lto-inline-cost -inlining-dyn-alloca-special-arg-count=3 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-GOOD,CHECK-GOOD-CL
; RUN: opt -passes='cgscc(inline)' -pre-lto-inline-cost -inlining-dyn-alloca-special-arg-count=3 < %s -S 2>&1 | opt -passes='inlinereportsetup,cgscc(inline),inlinereportemitter' -inline-report=0xe886 -lto-inline-cost -inlining-dyn-alloca-special-arg-count=3 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-GOOD,CHECK-GOOD-ML
; RUN: opt -passes='cgscc(inline)' -pre-lto-inline-cost -inlining-dyn-alloca-special-arg-count=4 < %s -S 2>&1 | opt -passes='cgscc(inline)' -inline-report=0xe807 -lto-inline-cost -inlining-dyn-alloca-special-arg-count=4 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-BAD,CHECK-BAD-CL
; RUN: opt -passes='cgscc(inline)' -pre-lto-inline-cost -inlining-dyn-alloca-special-arg-count=4 < %s -S 2>&1 | opt -passes='inlinereportsetup,cgscc(inline),inlinereportemitter' -inline-report=0xe886 -lto-inline-cost -inlining-dyn-alloca-special-arg-count=4 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-BAD,CHECK-BAD-ML

; Check that with -inlining-dyn-alloca-special-arg-count=3 inlining happens
; with a dynamic alloca, but with -inlining-dyn-alloca-special-arg-count=4
; inlining does not happen with a dynamic alloca.

; asa3_ also gets inlined because it is a single callsite function
; which is passed down the special args from asa2_

; CHECK-GOOD-CL-NOT: call void @asa2_
; CHECK-GOOD-CL-NOT: call void @asa3_
; CHECK-GOOD-CL: DEAD STATIC FUNC: asa2_
; CHECK-GOOD-CL: DEAD STATIC FUNC: asa3_
; CHECK-GOOD-ML: DEAD STATIC FUNC: asa3_
; CHECK-GOOD-ML: DEAD STATIC FUNC: asa2_
; CHECK-GOOD: COMPILE FUNC: MAIN__
; CHECK-GOOD: asa2_ {{.*}}Callee has single callsite and local linkage
; CHECK-GOOD: asa3_ {{.*}}Callee has single callsite and local linkage
; CHECK-GOOD-ML-NOT: call void @asa2_
; CHECK-GOOD-ML-NOT: call void @asa3_

; CHECK-BAD-CL: call void @asa3_
; CHECK-BAD-CL: call void @asa2_
; CHECK-BAD: COMPILE FUNC: asa3_
; CHECK-BAD: COMPILE FUNC: asa2_
; CHECK-BAD: asa3_ {{.*}}Callee has dynamic alloca
; CHECK-BAD: COMPILE FUNC: MAIN__
; CHECK-BAD: asa2_ {{.*}}Callee has dynamic alloca
; CHECK-BAD-ML: call void @asa3_
; CHECK-BAD-ML: call void @asa2_

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$double*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@llvm.compiler.used = appending global [1 x ptr] [ptr @__intel_new_feature_proc_init], section "llvm.metadata"
@anon.de2dcb5c6f60f26f7a15b3bfef1e9c01.0 = internal unnamed_addr constant i32 65536, align 4
@anon.de2dcb5c6f60f26f7a15b3bfef1e9c01.1 = internal unnamed_addr constant i32 2, align 4

declare dso_local void @__intel_new_feature_proc_init(i32, i64)

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable
define internal void @asa3_(i32 %t0, i32 %t1, ptr noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %arg, ptr noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %arg1, ptr noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %arg2) #0 !llfort.type_idx !3 {
bb:
  %i = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg, i64 0, i32 0, !llfort.type_idx !4
  %i3 = load ptr, ptr %i, align 1, !tbaa !5, !llfort.type_idx !11
  %i4 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg, i64 0, i32 6, i64 0, i32 1
  %i5 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i4, i32 0), !llfort.type_idx !12
  %i6 = load i64, ptr %i5, align 1, !tbaa !13, !llfort.type_idx !12
  %i7 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg, i64 0, i32 6, i64 0, i32 0
  %i8 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i4, i32 1), !llfort.type_idx !14
  %i9 = load i64, ptr %i8, align 1, !tbaa !13, !llfort.type_idx !14
  %i10 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i7, i32 1), !llfort.type_idx !15
  %i11 = load i64, ptr %i10, align 1, !tbaa !16, !llfort.type_idx !15
  %i12 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg1, i64 0, i32 0, !llfort.type_idx !17
  %i13 = load ptr, ptr %i12, align 1, !tbaa !18, !llfort.type_idx !11
  %i14 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg1, i64 0, i32 6, i64 0, i32 1
  %i15 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i14, i32 0), !llfort.type_idx !20
  %i16 = load i64, ptr %i15, align 1, !tbaa !21, !llfort.type_idx !20
  %i17 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg1, i64 0, i32 6, i64 0, i32 0
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i17, i32 0), !llfort.type_idx !22
  %i19 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i14, i32 1), !llfort.type_idx !23
  %i20 = load i64, ptr %i19, align 1, !tbaa !21, !llfort.type_idx !23
  %i21 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i17, i32 1), !llfort.type_idx !24
  %i22 = icmp slt i64 %i11, 1
  br i1 %i22, label %bb44, label %bb23

bb23:                                             ; preds = %bb
  %i24 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i7, i32 0), !llfort.type_idx !25
  %i25 = load i64, ptr %i24, align 1, !tbaa !16, !llfort.type_idx !25
  %i26 = icmp slt i64 %i25, 1
  %i27 = add nsw i64 %i25, 1
  %i28 = add nuw nsw i64 %i11, 1
  br label %bb39

bb29:                                             ; preds = %bb41, %bb29
  %i30 = phi i64 [ 1, %bb41 ], [ %i34, %bb29 ]
  %i31 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i16, ptr elementtype(double) %i42, i64 %i30), !llfort.type_idx !26
  %i32 = load double, ptr %i31, align 1, !tbaa !27, !llfort.type_idx !26
  %i33 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i6, ptr elementtype(double) %i43, i64 %i30), !llfort.type_idx !30
  store double %i32, ptr %i33, align 1, !tbaa !31
  %i34 = add nuw nsw i64 %i30, 1
  %i35 = icmp eq i64 %i34, %i27
  br i1 %i35, label %bb36, label %bb29

bb36:                                             ; preds = %bb39, %bb29
  %i37 = add nuw nsw i64 %i40, 1
  %i38 = icmp eq i64 %i37, %i28
  br i1 %i38, label %bb44, label %bb39

bb39:                                             ; preds = %bb36, %bb23
  %i40 = phi i64 [ 1, %bb23 ], [ %i37, %bb36 ]
  br i1 %i26, label %bb36, label %bb41

bb41:                                             ; preds = %bb39
  %i42 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i20, ptr elementtype(double) %i13, i64 %i40), !llfort.type_idx !33
  %i43 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i9, ptr elementtype(double) %i3, i64 %i40), !llfort.type_idx !34
  br label %bb29

bb44:                                             ; preds = %bb36, %bb
  %i45 = load i64, ptr %i21, align 1, !tbaa !35, !llfort.type_idx !36
  %i46 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg2, i64 0, i32 0, !llfort.type_idx !37
  %i47 = load ptr, ptr %i46, align 1, !tbaa !38, !llfort.type_idx !11
  %i48 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg2, i64 0, i32 6, i64 0, i32 1
  %i49 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i48, i32 0), !llfort.type_idx !40
  %i50 = load i64, ptr %i49, align 1, !tbaa !41, !llfort.type_idx !40
  %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i48, i32 1), !llfort.type_idx !42
  %i52 = load i64, ptr %i51, align 1, !tbaa !41, !llfort.type_idx !42
  %i53 = icmp slt i64 %i45, 1
  br i1 %i53, label %bb74, label %bb54

bb54:                                             ; preds = %bb44
  %i55 = load i64, ptr %i18, align 1, !tbaa !35, !llfort.type_idx !43
  %i56 = icmp slt i64 %i55, 1
  %i57 = add nsw i64 %i55, 1
  %i58 = add nuw nsw i64 %i45, 1
  br label %bb69

bb59:                                             ; preds = %bb71, %bb59
  %i60 = phi i64 [ 1, %bb71 ], [ %i64, %bb59 ]
  %i61 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i50, ptr elementtype(double) %i72, i64 %i60), !llfort.type_idx !44
  %i62 = load double, ptr %i61, align 1, !tbaa !45, !llfort.type_idx !44
  %i63 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i16, ptr elementtype(double) %i73, i64 %i60), !llfort.type_idx !47
  store double %i62, ptr %i63, align 1, !tbaa !27
  %i64 = add nuw nsw i64 %i60, 1
  %i65 = icmp eq i64 %i64, %i57
  br i1 %i65, label %bb66, label %bb59

bb66:                                             ; preds = %bb69, %bb59
  %i67 = add nuw nsw i64 %i70, 1
  %i68 = icmp eq i64 %i67, %i58
  br i1 %i68, label %bb74, label %bb69

bb69:                                             ; preds = %bb66, %bb54
  %i70 = phi i64 [ 1, %bb54 ], [ %i67, %bb66 ]
  br i1 %i56, label %bb66, label %bb71

bb71:                                             ; preds = %bb69
  %i72 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i52, ptr elementtype(double) %i47, i64 %i70), !llfort.type_idx !48
  %i73 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i20, ptr elementtype(double) %i13, i64 %i70), !llfort.type_idx !49
  br label %bb59

bb74:                                             ; preds = %bb66, %bb44
  %t232 = sext i32 %t0 to i64
  %t233 = icmp sgt i64 %t232, 0
  %t234 = select i1 %t233, i64 %t232, i64 0
  %t235 = mul nuw nsw i64 %t234, 560
  %t236 = sext i32 %t1 to i64
  %t237 = icmp sgt i64 %t236, 0
  %t238 = select i1 %t237, i64 %t236, i64 0
  %t239 = mul nsw i64 %t235, %t238
  %t240 = lshr exact i64 %t239, 2
  %t241 = alloca i32, i64 %t240, align 4
  ret void
}

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable
define internal void @asa2_(i32 %t0, i32 %t1, ptr noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %arg, ptr noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %arg1, ptr noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %arg2) #0 !llfort.type_idx !3 {
bb:
  %i = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg, i64 0, i32 0, !llfort.type_idx !4
  %i3 = load ptr, ptr %i, align 1, !tbaa !5, !llfort.type_idx !11
  %i4 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg, i64 0, i32 6, i64 0, i32 1
  %i5 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i4, i32 0), !llfort.type_idx !12
  %i6 = load i64, ptr %i5, align 1, !tbaa !13, !llfort.type_idx !12
  %i7 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg, i64 0, i32 6, i64 0, i32 0
  %i8 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i4, i32 1), !llfort.type_idx !14
  %i9 = load i64, ptr %i8, align 1, !tbaa !13, !llfort.type_idx !14
  %i10 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i7, i32 1), !llfort.type_idx !15
  %i11 = load i64, ptr %i10, align 1, !tbaa !16, !llfort.type_idx !15
  %i12 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg1, i64 0, i32 0, !llfort.type_idx !17
  %i13 = load ptr, ptr %i12, align 1, !tbaa !18, !llfort.type_idx !11
  %i14 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg1, i64 0, i32 6, i64 0, i32 1
  %i15 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i14, i32 0), !llfort.type_idx !20
  %i16 = load i64, ptr %i15, align 1, !tbaa !21, !llfort.type_idx !20
  %i17 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg1, i64 0, i32 6, i64 0, i32 0
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i17, i32 0), !llfort.type_idx !22
  %i19 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i14, i32 1), !llfort.type_idx !23
  %i20 = load i64, ptr %i19, align 1, !tbaa !21, !llfort.type_idx !23
  %i21 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i17, i32 1), !llfort.type_idx !24
  %i22 = icmp slt i64 %i11, 1
  br i1 %i22, label %bb44, label %bb23

bb23:                                             ; preds = %bb
  %i24 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i7, i32 0), !llfort.type_idx !25
  %i25 = load i64, ptr %i24, align 1, !tbaa !16, !llfort.type_idx !25
  %i26 = icmp slt i64 %i25, 1
  %i27 = add nsw i64 %i25, 1
  %i28 = add nuw nsw i64 %i11, 1
  br label %bb39

bb29:                                             ; preds = %bb41, %bb29
  %i30 = phi i64 [ 1, %bb41 ], [ %i34, %bb29 ]
  %i31 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i16, ptr elementtype(double) %i42, i64 %i30), !llfort.type_idx !26
  %i32 = load double, ptr %i31, align 1, !tbaa !27, !llfort.type_idx !26
  %i33 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i6, ptr elementtype(double) %i43, i64 %i30), !llfort.type_idx !30
  store double %i32, ptr %i33, align 1, !tbaa !31
  %i34 = add nuw nsw i64 %i30, 1
  %i35 = icmp eq i64 %i34, %i27
  br i1 %i35, label %bb36, label %bb29

bb36:                                             ; preds = %bb39, %bb29
  %i37 = add nuw nsw i64 %i40, 1
  %i38 = icmp eq i64 %i37, %i28
  br i1 %i38, label %bb44, label %bb39

bb39:                                             ; preds = %bb36, %bb23
  %i40 = phi i64 [ 1, %bb23 ], [ %i37, %bb36 ]
  br i1 %i26, label %bb36, label %bb41

bb41:                                             ; preds = %bb39
  %i42 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i20, ptr elementtype(double) %i13, i64 %i40), !llfort.type_idx !33
  %i43 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i9, ptr elementtype(double) %i3, i64 %i40), !llfort.type_idx !34
  br label %bb29

bb44:                                             ; preds = %bb36, %bb
  %i45 = load i64, ptr %i21, align 1, !tbaa !35, !llfort.type_idx !36
  %i46 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg2, i64 0, i32 0, !llfort.type_idx !37
  %i47 = load ptr, ptr %i46, align 1, !tbaa !38, !llfort.type_idx !11
  %i48 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %arg2, i64 0, i32 6, i64 0, i32 1
  %i49 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i48, i32 0), !llfort.type_idx !40
  %i50 = load i64, ptr %i49, align 1, !tbaa !41, !llfort.type_idx !40
  %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i48, i32 1), !llfort.type_idx !42
  %i52 = load i64, ptr %i51, align 1, !tbaa !41, !llfort.type_idx !42
  %i53 = icmp slt i64 %i45, 1
  br i1 %i53, label %bb74, label %bb54

bb54:                                             ; preds = %bb44
  %i55 = load i64, ptr %i18, align 1, !tbaa !35, !llfort.type_idx !43
  %i56 = icmp slt i64 %i55, 1
  %i57 = add nsw i64 %i55, 1
  %i58 = add nuw nsw i64 %i45, 1
  br label %bb69

bb59:                                             ; preds = %bb71, %bb59
  %i60 = phi i64 [ 1, %bb71 ], [ %i64, %bb59 ]
  %i61 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i50, ptr elementtype(double) %i72, i64 %i60), !llfort.type_idx !44
  %i62 = load double, ptr %i61, align 1, !tbaa !45, !llfort.type_idx !44
  %i63 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i16, ptr elementtype(double) %i73, i64 %i60), !llfort.type_idx !47
  store double %i62, ptr %i63, align 1, !tbaa !27
  %i64 = add nuw nsw i64 %i60, 1
  %i65 = icmp eq i64 %i64, %i57
  br i1 %i65, label %bb66, label %bb59

bb66:                                             ; preds = %bb69, %bb59
  %i67 = add nuw nsw i64 %i70, 1
  %i68 = icmp eq i64 %i67, %i58
  br i1 %i68, label %bb74, label %bb69

bb69:                                             ; preds = %bb66, %bb54
  %i70 = phi i64 [ 1, %bb54 ], [ %i67, %bb66 ]
  br i1 %i56, label %bb66, label %bb71

bb71:                                             ; preds = %bb69
  %i72 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i52, ptr elementtype(double) %i47, i64 %i70), !llfort.type_idx !48
  %i73 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i20, ptr elementtype(double) %i13, i64 %i70), !llfort.type_idx !49
  br label %bb59

bb74:                                             ; preds = %bb66, %bb44
  %t232 = sext i32 %t0 to i64
  %t233 = icmp sgt i64 %t232, 0
  %t234 = select i1 %t233, i64 %t232, i64 0
  %t235 = mul nuw nsw i64 %t234, 560
  %t236 = sext i32 %t1 to i64
  %t237 = icmp sgt i64 %t236, 0
  %t238 = select i1 %t237, i64 %t236, i64 0
  %t239 = mul nsw i64 %t235, %t238
  %t240 = lshr exact i64 %t239, 2
  %t241 = alloca i32, i64 %t240, align 4
  call void @asa3_(i32 10, i32 20, ptr nonnull %arg, ptr nonnull %arg1, ptr nonnull %arg2), !llfort.type_idx !50
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @MAIN__() #1 !llfort.type_idx !51 {
bb:
  %"main_$A" = alloca double, i64 100, align 8
  %"main_$B" = alloca double, i64 100, align 8
  %"main_$C" = alloca double, i64 100, align 8
  %i = alloca %"QNCA_a0$double*$rank2$", align 8, !llfort.type_idx !52
  %i1 = alloca %"QNCA_a0$double*$rank2$", align 8, !llfort.type_idx !52
  %i2 = alloca %"QNCA_a0$double*$rank2$", align 8, !llfort.type_idx !52
  %i3 = tail call i32 @for_set_fpe_(ptr nonnull @anon.de2dcb5c6f60f26f7a15b3bfef1e9c01.0) #4, !llfort.type_idx !53
  %i4 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.de2dcb5c6f60f26f7a15b3bfef1e9c01.1) #4, !llfort.type_idx !53
  br label %bb5

bb5:                                              ; preds = %bb5, %bb
  %i6 = phi i64 [ 1, %bb ], [ %i9, %bb5 ]
  %i7 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 8, ptr nonnull elementtype(double) %"main_$A", i64 %i6), !llfort.type_idx !54
  %i8 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %i7, i64 1), !llfort.type_idx !55
  store double 1.000000e+00, ptr %i8, align 8, !tbaa !56
  %i9 = add nuw nsw i64 %i6, 1
  %i10 = icmp eq i64 %i9, 101
  br i1 %i10, label %bb11, label %bb5

bb11:                                             ; preds = %bb11, %bb5
  %i12 = phi i64 [ %i15, %bb11 ], [ 1, %bb5 ]
  %i13 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 8, ptr nonnull elementtype(double) %"main_$B", i64 %i12), !llfort.type_idx !61
  %i14 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %i13, i64 1), !llfort.type_idx !62
  store double 2.000000e+00, ptr %i14, align 8, !tbaa !63
  %i15 = add nuw nsw i64 %i12, 1
  %i16 = icmp eq i64 %i15, 101
  br i1 %i16, label %bb17, label %bb11

bb17:                                             ; preds = %bb17, %bb11
  %i18 = phi i64 [ %i21, %bb17 ], [ 1, %bb11 ]
  %i19 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 8, ptr nonnull elementtype(double) %"main_$C", i64 %i18), !llfort.type_idx !65
  %i20 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %i19, i64 1), !llfort.type_idx !66
  store double 3.000000e+00, ptr %i20, align 8, !tbaa !67
  %i21 = add nuw nsw i64 %i18, 1
  %i22 = icmp eq i64 %i21, 101
  br i1 %i22, label %bb23, label %bb17

bb23:                                             ; preds = %bb17
  %i24 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i, i64 0, i32 3, !llfort.type_idx !69
  %i25 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i, i64 0, i32 1, !llfort.type_idx !70
  store i64 8, ptr %i25, align 8, !tbaa !71
  %i26 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i, i64 0, i32 4, !llfort.type_idx !75
  store i64 2, ptr %i26, align 8, !tbaa !76
  %i27 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i, i64 0, i32 2, !llfort.type_idx !77
  store i64 0, ptr %i27, align 8, !tbaa !78
  %i28 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i, i64 0, i32 6, i64 0, i32 1
  %i29 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i28, i32 0), !llfort.type_idx !79
  store i64 8, ptr %i29, align 8, !tbaa !80
  %i30 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i, i64 0, i32 6, i64 0, i32 2
  %i31 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i30, i32 0), !llfort.type_idx !81
  store i64 1, ptr %i31, align 8, !tbaa !82
  %i32 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i, i64 0, i32 6, i64 0, i32 0
  %i33 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i32, i32 0), !llfort.type_idx !83
  store i64 1, ptr %i33, align 8, !tbaa !84
  %i34 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i28, i32 1), !llfort.type_idx !85
  store i64 8, ptr %i34, align 8, !tbaa !80
  %i35 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i30, i32 1), !llfort.type_idx !86
  store i64 1, ptr %i35, align 8, !tbaa !82
  %i36 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i32, i32 1), !llfort.type_idx !87
  store i64 100, ptr %i36, align 8, !tbaa !84
  %i37 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i, i64 0, i32 0, !llfort.type_idx !88
  store ptr %"main_$A", ptr %i37, align 8, !tbaa !89
  store i64 1, ptr %i24, align 8, !tbaa !90
  %i38 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i1, i64 0, i32 3, !llfort.type_idx !69
  %i39 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i1, i64 0, i32 1, !llfort.type_idx !70
  store i64 8, ptr %i39, align 8, !tbaa !91
  %i40 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i1, i64 0, i32 4, !llfort.type_idx !75
  store i64 2, ptr %i40, align 8, !tbaa !93
  %i41 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i1, i64 0, i32 2, !llfort.type_idx !77
  store i64 0, ptr %i41, align 8, !tbaa !94
  %i42 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i1, i64 0, i32 6, i64 0, i32 1
  %i43 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i42, i32 0), !llfort.type_idx !95
  store i64 8, ptr %i43, align 8, !tbaa !96
  %i44 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i1, i64 0, i32 6, i64 0, i32 2
  %i45 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i44, i32 0), !llfort.type_idx !97
  store i64 1, ptr %i45, align 8, !tbaa !98
  %i46 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i1, i64 0, i32 6, i64 0, i32 0
  %i47 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i46, i32 0), !llfort.type_idx !99
  store i64 1, ptr %i47, align 8, !tbaa !100
  %i48 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i42, i32 1), !llfort.type_idx !101
  store i64 8, ptr %i48, align 8, !tbaa !96
  %i49 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i44, i32 1), !llfort.type_idx !102
  store i64 1, ptr %i49, align 8, !tbaa !98
  %i50 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i46, i32 1), !llfort.type_idx !103
  store i64 100, ptr %i50, align 8, !tbaa !100
  %i51 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i1, i64 0, i32 0, !llfort.type_idx !88
  store ptr %"main_$B", ptr %i51, align 8, !tbaa !104
  store i64 1, ptr %i38, align 8, !tbaa !105
  %i52 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i2, i64 0, i32 3, !llfort.type_idx !69
  %i53 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i2, i64 0, i32 1, !llfort.type_idx !70
  store i64 8, ptr %i53, align 8, !tbaa !106
  %i54 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i2, i64 0, i32 4, !llfort.type_idx !75
  store i64 2, ptr %i54, align 8, !tbaa !108
  %i55 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i2, i64 0, i32 2, !llfort.type_idx !77
  store i64 0, ptr %i55, align 8, !tbaa !109
  %i56 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i2, i64 0, i32 6, i64 0, i32 1
  %i57 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i56, i32 0), !llfort.type_idx !110
  store i64 8, ptr %i57, align 8, !tbaa !111
  %i58 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i2, i64 0, i32 6, i64 0, i32 2
  %i59 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i58, i32 0), !llfort.type_idx !112
  store i64 1, ptr %i59, align 8, !tbaa !113
  %i60 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i2, i64 0, i32 6, i64 0, i32 0
  %i61 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i60, i32 0), !llfort.type_idx !114
  store i64 1, ptr %i61, align 8, !tbaa !115
  %i62 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i56, i32 1), !llfort.type_idx !116
  store i64 8, ptr %i62, align 8, !tbaa !111
  %i63 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i58, i32 1), !llfort.type_idx !117
  store i64 1, ptr %i63, align 8, !tbaa !113
  %i64 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i60, i32 1), !llfort.type_idx !118
  store i64 100, ptr %i64, align 8, !tbaa !115
  %i65 = getelementptr inbounds %"QNCA_a0$double*$rank2$", ptr %i2, i64 0, i32 0, !llfort.type_idx !88
  store ptr %"main_$C", ptr %i65, align 8, !tbaa !119
  store i64 1, ptr %i52, align 8, !tbaa !120
  call void @asa2_(i32 10, i32 20, ptr nonnull %i, ptr nonnull %i1, ptr nonnull %i2), !llfort.type_idx !50
  ret void
}

declare !llfort.type_idx !121 !llfort.intrin_id !122 dso_local i32 @for_set_fpe_(ptr nocapture readonly) local_unnamed_addr

; Function Attrs: nofree
declare !llfort.type_idx !123 !llfort.intrin_id !124 dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #2

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #3

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #3

attributes #0 = { nofree norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
attributes #4 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
!3 = !{i64 115}
!4 = !{i64 55}
!5 = !{!6, !7, i64 0}
!6 = !{!"ifx$descr$1", !7, i64 0, !7, i64 8, !7, i64 16, !7, i64 24, !7, i64 32, !7, i64 40, !7, i64 48, !7, i64 56, !7, i64 64, !7, i64 72, !7, i64 80, !7, i64 88}
!7 = !{!"ifx$descr$field", !8, i64 0}
!8 = !{!"Fortran Dope Vector Symbol", !9, i64 0}
!9 = !{!"Generic Fortran Symbol", !10, i64 0}
!10 = !{!"ifx$root$1$asa2_"}
!11 = !{i64 6}
!12 = !{i64 149}
!13 = !{!6, !7, i64 56}
!14 = !{i64 152}
!15 = !{i64 153}
!16 = !{!6, !7, i64 48}
!17 = !{i64 77}
!18 = !{!19, !7, i64 0}
!19 = !{!"ifx$descr$2", !7, i64 0, !7, i64 8, !7, i64 16, !7, i64 24, !7, i64 32, !7, i64 40, !7, i64 48, !7, i64 56, !7, i64 64, !7, i64 72, !7, i64 80, !7, i64 88}
!20 = !{i64 158}
!21 = !{!19, !7, i64 56}
!22 = !{i64 159}
!23 = !{i64 160}
!24 = !{i64 161}
!25 = !{i64 150}
!26 = !{i64 165}
!27 = !{!28, !28, i64 0}
!28 = !{!"ifx$unique_sym$1", !29, i64 0}
!29 = !{!"Fortran Data Symbol", !9, i64 0}
!30 = !{i64 157}
!31 = !{!32, !32, i64 0}
!32 = !{!"ifx$unique_sym$2", !29, i64 0}
!33 = !{i64 164}
!34 = !{i64 156}
!35 = !{!19, !7, i64 48}
!36 = !{i64 169}
!37 = !{i64 99}
!38 = !{!39, !7, i64 0}
!39 = !{!"ifx$descr$3", !7, i64 0, !7, i64 8, !7, i64 16, !7, i64 24, !7, i64 32, !7, i64 40, !7, i64 48, !7, i64 56, !7, i64 64, !7, i64 72, !7, i64 80, !7, i64 88}
!40 = !{i64 174}
!41 = !{!39, !7, i64 56}
!42 = !{i64 176}
!43 = !{i64 167}
!44 = !{i64 181}
!45 = !{!46, !46, i64 0}
!46 = !{!"ifx$unique_sym$3", !29, i64 0}
!47 = !{i64 173}
!48 = !{i64 180}
!49 = !{i64 172}
!50 = !{i64 37}
!51 = !{i64 192}
!52 = !{i64 25}
!53 = !{i64 2}
!54 = !{i64 304}
!55 = !{i64 305}
!56 = !{!57, !57, i64 0}
!57 = !{!"ifx$unique_sym$4", !58, i64 0}
!58 = !{!"Fortran Data Symbol", !59, i64 0}
!59 = !{!"Generic Fortran Symbol", !60, i64 0}
!60 = !{!"ifx$root$2$MAIN__"}
!61 = !{i64 308}
!62 = !{i64 309}
!63 = !{!64, !64, i64 0}
!64 = !{!"ifx$unique_sym$5", !58, i64 0}
!65 = !{i64 312}
!66 = !{i64 313}
!67 = !{!68, !68, i64 0}
!68 = !{!"ifx$unique_sym$6", !58, i64 0}
!69 = !{i64 29}
!70 = !{i64 27}
!71 = !{!72, !73, i64 8}
!72 = !{!"ifx$descr$4", !73, i64 0, !73, i64 8, !73, i64 16, !73, i64 24, !73, i64 32, !73, i64 40, !73, i64 48, !73, i64 56, !73, i64 64, !73, i64 72, !73, i64 80, !73, i64 88}
!73 = !{!"ifx$descr$field", !74, i64 0}
!74 = !{!"Fortran Dope Vector Symbol", !59, i64 0}
!75 = !{i64 30}
!76 = !{!72, !73, i64 32}
!77 = !{i64 28}
!78 = !{!72, !73, i64 16}
!79 = !{i64 315}
!80 = !{!72, !73, i64 56}
!81 = !{i64 316}
!82 = !{!72, !73, i64 64}
!83 = !{i64 317}
!84 = !{!72, !73, i64 48}
!85 = !{i64 318}
!86 = !{i64 319}
!87 = !{i64 320}
!88 = !{i64 26}
!89 = !{!72, !73, i64 0}
!90 = !{!72, !73, i64 24}
!91 = !{!92, !73, i64 8}
!92 = !{!"ifx$descr$5", !73, i64 0, !73, i64 8, !73, i64 16, !73, i64 24, !73, i64 32, !73, i64 40, !73, i64 48, !73, i64 56, !73, i64 64, !73, i64 72, !73, i64 80, !73, i64 88}
!93 = !{!92, !73, i64 32}
!94 = !{!92, !73, i64 16}
!95 = !{i64 323}
!96 = !{!92, !73, i64 56}
!97 = !{i64 324}
!98 = !{!92, !73, i64 64}
!99 = !{i64 325}
!100 = !{!92, !73, i64 48}
!101 = !{i64 326}
!102 = !{i64 327}
!103 = !{i64 328}
!104 = !{!92, !73, i64 0}
!105 = !{!92, !73, i64 24}
!106 = !{!107, !73, i64 8}
!107 = !{!"ifx$descr$6", !73, i64 0, !73, i64 8, !73, i64 16, !73, i64 24, !73, i64 32, !73, i64 40, !73, i64 48, !73, i64 56, !73, i64 64, !73, i64 72, !73, i64 80, !73, i64 88}
!108 = !{!107, !73, i64 32}
!109 = !{!107, !73, i64 16}
!110 = !{i64 331}
!111 = !{!107, !73, i64 56}
!112 = !{i64 332}
!113 = !{!107, !73, i64 64}
!114 = !{i64 333}
!115 = !{!107, !73, i64 48}
!116 = !{i64 334}
!117 = !{i64 335}
!118 = !{i64 336}
!119 = !{!107, !73, i64 0}
!120 = !{!107, !73, i64 24}
!121 = !{i64 299}
!122 = !{i32 97}
!123 = !{i64 301}
!124 = !{i32 98}
; end INTEL_FEATURE_SW_ADVANCED

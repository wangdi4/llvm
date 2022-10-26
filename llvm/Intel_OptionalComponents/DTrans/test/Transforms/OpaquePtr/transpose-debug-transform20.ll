; REQUIRES: asserts
; RUN: opt -opaque-pointers < %s -dva-check-dtrans-outofboundsok -dtrans-outofboundsok=false -debug-only=dtrans-transpose-transform -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck %s

; Check that physpropmod_mp_physprop_ fields 0 and 1 aren't valid transpose
; candidates since the field 2 from the structure %"PHYSPROPMOD$.btPHYSPROP_TYPE"
; is passed to a vararg function. This is the same test case as
; transpose-debug-transform13.ll but it adds a call to the vararg function @foo.

; CHECK: No transpose candidates

; CHECK-NOT: Transpose candidate: physpropmod_mp_physprop_
; CHECK-NOT: Nested Field Number : 0
; CHECK-NOT: IsValid{{ *}}: true
; CHECK-NOT: IsProfitable{{ *}}: false

; CHECK-NOT: Transpose candidate: physpropmod_mp_physprop_
; CHECK-NOT: Nested Field Number : 1
; CHECK-NOT: IsValid{{ *}}: true
; CHECK-NOT: IsProfitable{{ *}}: false

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"PHYSPROPMOD$.btPHYSPROP_TYPE" = type { %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$", ptr }
%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@physpropmod_mp_physprop_ = internal global %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" { ptr null, i64 0, i64 0, i64 0, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@anon.39d3b15ada3f55b3c535c1be4b71a62a.0 = internal unnamed_addr constant i32 2

declare void @foo(i32, ...)

; Function Attrs: nofree noinline nounwind uwtable
define internal void @physprop_init_(ptr noalias nocapture readonly dereferenceable(4) %arg) #0 {
bb:
  %i = alloca i64, align 8
  store i64 5, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 5), align 8
  store i64 200, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 1), align 8
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 4), align 8
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 2), align 8
  %i1 = load i32, ptr %arg, align 1
  %i2 = sext i32 %i1 to i64
  %i3 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %i3, align 1
  %i4 = icmp sgt i64 %i2, 0
  %i5 = select i1 %i4, i64 %i2, i64 0
  %i6 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 %i5, ptr %i6, align 1
  %i7 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 200, ptr %i7, align 1
  %i8 = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %i, i32 2, i64 %i5, i64 200) #7
  %i9 = load i64, ptr %i, align 8
  %i10 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  %i11 = and i64 %i10, -68451041281
  %i12 = or i64 %i11, 1073741824
  store i64 %i12, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  %i13 = shl i32 %i8, 4
  %i14 = and i32 %i13, 16
  %i15 = or i32 %i14, 262144
  %i16 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 5), align 8
  %i17 = inttoptr i64 %i16 to ptr
  %i18 = tail call i32 @for_allocate_handle(i64 %i9, ptr @physpropmod_mp_physprop_, i32 %i15, ptr %i17) #7
  %i19 = icmp slt i32 %i1, 1
  br i1 %i19, label %bb79, label %bb20

bb20:                                             ; preds = %bb20, %bb
  %i21 = phi i64 [ %i76, %bb20 ], [ 1, %bb ]
  %i22 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i23 = load i64, ptr %i7, align 1
  %i24 = load i64, ptr %i3, align 1
  %i25 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i24, i64 %i23, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i22, i64 %i21)
  %i26 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i25, i64 0, i32 0
  %i27 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i26, i64 0, i32 3
  store i64 5, ptr %i27, align 1
  %i28 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i26, i64 0, i32 5
  store i64 0, ptr %i28, align 1
  %i29 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i26, i64 0, i32 1
  store i64 4, ptr %i29, align 1
  %i30 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i26, i64 0, i32 4
  store i64 2, ptr %i30, align 1
  %i31 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i26, i64 0, i32 2
  store i64 0, ptr %i31, align 1
  %i32 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i26, i64 0, i32 6, i64 0
  %i33 = getelementptr inbounds { i64, i64, i64 }, ptr %i32, i64 0, i32 2
  %i34 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i33, i32 0)
  store i64 1, ptr %i34, align 1
  %i35 = getelementptr inbounds { i64, i64, i64 }, ptr %i32, i64 0, i32 0
  %i36 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i35, i32 0)
  store i64 1000, ptr %i36, align 1
  %i37 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i33, i32 1)
  store i64 1, ptr %i37, align 1
  %i38 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i35, i32 1)
  store i64 19, ptr %i38, align 1
  %i39 = getelementptr inbounds { i64, i64, i64 }, ptr %i32, i64 0, i32 1
  %i40 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i39, i32 0)
  store i64 4, ptr %i40, align 1
  %i41 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i39, i32 1)
  store i64 4000, ptr %i41, align 1
  %i42 = load i64, ptr %i27, align 1
  %i43 = and i64 %i42, -68451041281
  %i44 = or i64 %i43, 1073741824
  store i64 %i44, ptr %i27, align 1
  %i45 = load i64, ptr %i28, align 1
  %i46 = inttoptr i64 %i45 to ptr
  %i47 = bitcast ptr %i25 to ptr
  %i48 = tail call i32 @for_allocate_handle(i64 76000, ptr %i47, i32 262144, ptr %i46) #7
  %i49 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i50 = load i64, ptr %i7, align 1
  %i51 = load i64, ptr %i3, align 1
  %i52 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i51, i64 %i50, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i49, i64 %i21)
  %i53 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i52, i64 0, i32 1
  %i54 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i53, i64 0, i32 3
  store i64 5, ptr %i54, align 1
  %i55 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i53, i64 0, i32 5
  store i64 0, ptr %i55, align 1
  %i56 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i53, i64 0, i32 1
  store i64 4, ptr %i56, align 1
  %i57 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i53, i64 0, i32 4
  store i64 2, ptr %i57, align 1
  %i58 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i53, i64 0, i32 2
  store i64 0, ptr %i58, align 1
  %i59 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i53, i64 0, i32 6, i64 0
  %i60 = getelementptr inbounds { i64, i64, i64 }, ptr %i59, i64 0, i32 2
  %i61 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i60, i32 0)
  store i64 1, ptr %i61, align 1
  %i62 = getelementptr inbounds { i64, i64, i64 }, ptr %i59, i64 0, i32 0
  %i63 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i62, i32 0)
  store i64 1000, ptr %i63, align 1
  %i64 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i60, i32 1)
  store i64 1, ptr %i64, align 1
  %i65 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i62, i32 1)
  store i64 19, ptr %i65, align 1
  %i66 = getelementptr inbounds { i64, i64, i64 }, ptr %i59, i64 0, i32 1
  %i67 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i66, i32 0)
  store i64 4, ptr %i67, align 1
  %i68 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i66, i32 1)
  store i64 4000, ptr %i68, align 1
  %i69 = load i64, ptr %i54, align 1
  %i70 = and i64 %i69, -68451041281
  %i71 = or i64 %i70, 1073741824
  store i64 %i71, ptr %i54, align 1
  %i72 = load i64, ptr %i55, align 1
  %i73 = inttoptr i64 %i72 to ptr
  %i74 = bitcast ptr %i53 to ptr
  %i75 = tail call i32 @for_allocate_handle(i64 76000, ptr nonnull %i74, i32 262144, ptr %i73) #7
  %i76 = add nuw i64 %i21, 1
  %i77 = trunc i64 %i76 to i32
  %i78 = icmp slt i32 %i1, %i77
  br i1 %i78, label %bb79, label %bb20

bb79:                                             ; preds = %bb20, %bb
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_check_mult_overflow64(ptr nocapture, i32, ...) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind willreturn writeonly uwtable
define internal void @physprop_pass_(ptr noalias nocapture dereferenceable(4) %arg) #2 {
bb:
  store float 1.500000e+01, ptr %arg, align 1
  ret void
}

; Function Attrs: nofree noinline nosync nounwind uwtable
define internal void @physprop_use_(ptr noalias nocapture readonly dereferenceable(4) %arg) #3 {
bb:
  %i = load i32, ptr %arg, align 1
  %i1 = sitofp i32 %i to float
  %i2 = fptosi float %i1 to i64
  %i3 = icmp slt i64 %i2, 1
  br i1 %i3, label %bb68, label %bb4

bb4:                                              ; preds = %bb
  %i5 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  %i6 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", ptr @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  br label %bb7

bb7:                                              ; preds = %bb64, %bb4
  %i8 = phi float [ %i65, %bb64 ], [ 1.000000e+00, %bb4 ]
  %i9 = phi i64 [ %i66, %bb64 ], [ %i2, %bb4 ]
  %i10 = fptosi float %i8 to i32
  %i11 = sext i32 %i10 to i64
  br label %bb12

bb12:                                             ; preds = %bb61, %bb7
  %i13 = phi i64 [ %i62, %bb61 ], [ 1, %bb7 ]
  br label %bb14

bb14:                                             ; preds = %bb14, %bb12
  %i15 = phi i64 [ %i59, %bb14 ], [ 1, %bb12 ]
  %i16 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i17 = load i64, ptr %i5, align 1
  %i18 = load i64, ptr %i6, align 1
  %i19 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i18, i64 %i17, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i16, i64 %i11)
  %i20 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i19, i64 0, i32 1
  %i21 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i20, i64 0, i32 0
  %i22 = load ptr, ptr %i21, align 1
  %i23 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i20, i64 0, i32 6, i64 0
  %i24 = getelementptr inbounds { i64, i64, i64 }, ptr %i23, i64 0, i32 1
  %i25 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i24, i32 0)
  %i26 = load i64, ptr %i25, align 1
  %i27 = getelementptr inbounds { i64, i64, i64 }, ptr %i23, i64 0, i32 2
  %i28 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i27, i32 0)
  %i29 = load i64, ptr %i28, align 1
  %i30 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i24, i32 1)
  %i31 = load i64, ptr %i30, align 1
  %i32 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i27, i32 1)
  %i33 = load i64, ptr %i32, align 1
  %i34 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i33, i64 %i31, ptr elementtype(float) %i22, i64 %i15)
  %i35 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i29, i64 %i26, ptr elementtype(float) %i34, i64 %i13)
  %i36 = load float, ptr %i35, align 1
  %i37 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i19, i64 0, i32 0
  %i38 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i37, i64 0, i32 0
  %i39 = load ptr, ptr %i38, align 1
  %i40 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i37, i64 0, i32 6, i64 0
  %i41 = getelementptr inbounds { i64, i64, i64 }, ptr %i40, i64 0, i32 1
  %i42 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i41, i32 0)
  %i43 = load i64, ptr %i42, align 1
  %i44 = getelementptr inbounds { i64, i64, i64 }, ptr %i40, i64 0, i32 2
  %i45 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i44, i32 0)
  %i46 = load i64, ptr %i45, align 1
  %i47 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i41, i32 1)
  %i48 = load i64, ptr %i47, align 1
  %i49 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i44, i32 1)
  %i50 = load i64, ptr %i49, align 1
  %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i50, i64 %i48, ptr elementtype(float) %i39, i64 %i15)
  %i52 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i46, i64 %i43, ptr elementtype(float) %i51, i64 %i13)
  store float %i36, ptr %i52, align 1
  %i53 = load ptr, ptr @physpropmod_mp_physprop_, align 8
  %i54 = load i64, ptr %i5, align 1
  %i55 = load i64, ptr %i6, align 1
  %i56 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i55, i64 %i54, ptr elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %i53, i64 %i11)
  %i57 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", ptr %i56, i64 0, i32 2
  call void (i32, ...) @foo(i32 1, ptr %i57)
  %i58 = load ptr, ptr %i57, align 1
  tail call void @physprop_pass_(ptr %i58)
  %i59 = add nuw nsw i64 %i15, 1
  %i60 = icmp eq i64 %i59, 20
  br i1 %i60, label %bb61, label %bb14

bb61:                                             ; preds = %bb14
  %i62 = add nuw nsw i64 %i13, 1
  %i63 = icmp eq i64 %i62, 1001
  br i1 %i63, label %bb64, label %bb12

bb64:                                             ; preds = %bb61
  %i65 = fadd reassoc ninf nsz arcp contract afn float %i8, 1.000000e+00
  %i66 = add nsw i64 %i9, -1
  %i67 = icmp sgt i64 %i9, 1
  br i1 %i67, label %bb7, label %bb68

bb68:                                             ; preds = %bb64, %bb
  ret void
}

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #4 {
bb:
  %i = alloca i32, align 8
  %i1 = alloca i32, align 8
  %i2 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.39d3b15ada3f55b3c535c1be4b71a62a.0) #7
  call void @physprop_init_(ptr nonnull %i1)
  call void @physprop_use_(ptr nonnull %i)
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: nocallback nofree nosync nounwind readnone willreturn
declare i64 @llvm.ssa.copy.i64(i64 returned) #5

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #6

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #6

attributes #0 = { nofree noinline nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { mustprogress nofree noinline norecurse nosync nounwind willreturn writeonly uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #3 = { nofree noinline nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #4 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #5 = { nocallback nofree nosync nounwind readnone willreturn }
attributes #6 = { nounwind readnone speculatable }
attributes #7 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}

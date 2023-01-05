; REQUIRES: asserts
; RUN: opt < %s -dva-check-dtrans-outofboundsok -dtrans-outofboundsok=true -disable-output -passes=dtrans-transpose -debug-only=dtrans-transpose-transform 2>&1 | FileCheck %s

; Check that the array through which the indirect subscripting is occurring is
; should not be transposed.

; CHECK: Transform candidate: main_$MYK
; CHECK-NOT: Before
; CHECK-NOT: After

; Check that the array represented by the 0th field of main_$PHYSPROP
; is transposed to ensure that the indirectly subscripted index is not the
; fastest varying subscript. Also check that the strides are replaced by
; literal constants.

; CHECK-LABEL: Transform candidate: main_$PHYSPROP[0]
; CHECK-NEXT: Before: MAIN__:  %i[[N0:[0-9]+]] = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %i[[I0:[0-9]+]],
; CHECK-NEXT: After : MAIN__:  %i[[N0]] = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4
; CHECK-NEXT: Before: MAIN__:  %i[[N1:[0-9]+]] = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %i[[I1:[0-9]+]],
; CHECK-NEXT: After : MAIN__:  %i[[N1]] = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 4000,

; Check that the array represented by the 2nd field of main_$PHYSPROP
; is not transposed because the indirectly subscripted index is not the
; fastest varying subscript.

; CHECK-LABEL: Transform candidate: main_$PHYSPROP[2]
; CHECK-NOT: Before
; CHECK-NOT: After

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$" = type { %"MAIN$.btPHYSPROP_TYPE"*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"MAIN$.btPHYSPROP_TYPE" = type { %"QNCA_a0$float*$rank2$", i32, %"QNCA_a0$float*$rank2$" }
%"QNCA_a0$float*$rank2$" = type { float*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@"main_$MYK" = internal unnamed_addr constant [1000 x [19 x i32]] zeroinitializer, align 16
@"main_$PHYSPROP" = internal global %"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$" { %"MAIN$.btPHYSPROP_TYPE"* null, i64 0, i64 0, i64 0, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@anon.35d4f0c186b9bde99ef526d487a05dff.0 = internal unnamed_addr constant i32 2

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #0 {
bb:
  %i = tail call i32 @for_set_reentrancy(i32* nonnull @anon.35d4f0c186b9bde99ef526d487a05dff.0) #4
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$"* @"main_$PHYSPROP", i64 0, i32 5), align 8
  store i64 200, i64* getelementptr inbounds (%"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$"* @"main_$PHYSPROP", i64 0, i32 1), align 8
  store i64 1, i64* getelementptr inbounds (%"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$"* @"main_$PHYSPROP", i64 0, i32 4), align 16
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$"* @"main_$PHYSPROP", i64 0, i32 2), align 16
  %i1 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$"* @"main_$PHYSPROP", i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, i64* %i1, align 1
  %i2 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$"* @"main_$PHYSPROP", i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 100, i64* %i2, align 1
  %i3 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$"* @"main_$PHYSPROP", i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 200, i64* %i3, align 1
  store i64 1073741829, i64* getelementptr inbounds (%"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$"* @"main_$PHYSPROP", i64 0, i32 3), align 8
  %i4 = tail call i32 @for_allocate_handle(i64 20000, i8** bitcast (%"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$"* @"main_$PHYSPROP" to i8**), i32 262144, i8* null) #4
  br label %bb5

bb5:                                              ; preds = %bb5, %bb
  %i6 = phi i64 [ %i58, %bb5 ], [ 1, %bb ]
  %i7 = load %"MAIN$.btPHYSPROP_TYPE"*, %"MAIN$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$"* @"main_$PHYSPROP", i64 0, i32 0), align 16
  %i8 = load i64, i64* %i3, align 1
  %i9 = load i64, i64* %i1, align 1
  %i10 = tail call %"MAIN$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_MAIN$.btPHYSPROP_TYPEs.i64.i64.p0s_MAIN$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %i9, i64 %i8, %"MAIN$.btPHYSPROP_TYPE"* elementtype(%"MAIN$.btPHYSPROP_TYPE") %i7, i64 %i6)
  %i12 = getelementptr inbounds %"MAIN$.btPHYSPROP_TYPE", %"MAIN$.btPHYSPROP_TYPE"* %i10, i64 0, i32 0
  %i13 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %i12, i64 0, i32 3
  store i64 5, i64* %i13, align 1, !alias.scope !3
  %i14 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %i12, i64 0, i32 5
  store i64 0, i64* %i14, align 1, !alias.scope !3
  %i15 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %i12, i64 0, i32 1
  store i64 4, i64* %i15, align 1, !alias.scope !3
  %i16 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %i12, i64 0, i32 4
  store i64 2, i64* %i16, align 1, !alias.scope !3
  %i17 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %i12, i64 0, i32 2
  store i64 0, i64* %i17, align 1, !alias.scope !3
  %i18 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %i12, i64 0, i32 6, i64 0
  %i19 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %i18, i64 0, i32 2
  %i20 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %i19, i32 0) #4
  store i64 1, i64* %i20, align 1, !alias.scope !3
  %i21 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %i18, i64 0, i32 0
  %i22 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %i21, i32 0) #4
  store i64 19, i64* %i22, align 1, !alias.scope !3
  %i23 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %i19, i32 1) #4
  store i64 1, i64* %i23, align 1, !alias.scope !3
  %i24 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %i21, i32 1) #4
  store i64 1000, i64* %i24, align 1, !alias.scope !3
  %i25 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %i18, i64 0, i32 1
  %i26 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %i25, i32 0) #4
  store i64 4, i64* %i26, align 1, !alias.scope !3
  %i27 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %i25, i32 1) #4
  store i64 76, i64* %i27, align 1, !alias.scope !3
  %i28 = load i64, i64* %i13, align 1, !alias.scope !3
  %i29 = and i64 %i28, -68451041281
  %i30 = or i64 %i29, 1073741824
  store i64 %i30, i64* %i13, align 1, !alias.scope !3
  %i31 = load i64, i64* %i14, align 1, !alias.scope !3
  %i32 = inttoptr i64 %i31 to i8*
  %i33 = bitcast %"MAIN$.btPHYSPROP_TYPE"* %i10 to i8**
  %i34 = tail call i32 @for_allocate_handle(i64 76000, i8** nonnull %i33, i32 262144, i8* %i32) #4
  %i35 = getelementptr inbounds %"MAIN$.btPHYSPROP_TYPE", %"MAIN$.btPHYSPROP_TYPE"* %i10, i64 0, i32 2
  %i36 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %i35, i64 0, i32 3
  store i64 5, i64* %i36, align 1, !alias.scope !3
  %i37 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %i35, i64 0, i32 5
  store i64 0, i64* %i37, align 1, !alias.scope !3
  %i38 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %i35, i64 0, i32 1
  store i64 4, i64* %i38, align 1, !alias.scope !3
  %i39 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %i35, i64 0, i32 4
  store i64 2, i64* %i39, align 1, !alias.scope !3
  %i40 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %i35, i64 0, i32 2
  store i64 0, i64* %i40, align 1, !alias.scope !3
  %i41 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %i35, i64 0, i32 6, i64 0
  %i42 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %i41, i64 0, i32 2
  %i43 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %i42, i32 0) #4
  store i64 1, i64* %i43, align 1, !alias.scope !3
  %i44 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %i41, i64 0, i32 0
  %i45 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %i44, i32 0) #4
  store i64 19, i64* %i45, align 1, !alias.scope !3
  %i46 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %i42, i32 1) #4
  store i64 1, i64* %i46, align 1, !alias.scope !3
  %i47 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %i44, i32 1) #4
  store i64 1000, i64* %i47, align 1, !alias.scope !3
  %i48 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %i41, i64 0, i32 1
  %i49 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %i48, i32 0) #4
  store i64 4, i64* %i49, align 1, !alias.scope !3
  %i50 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %i48, i32 1) #4
  store i64 76, i64* %i50, align 1, !alias.scope !3
  %i51 = load i64, i64* %i36, align 1, !alias.scope !3
  %i52 = and i64 %i51, -68451041281
  %i53 = or i64 %i52, 1073741824
  store i64 %i53, i64* %i36, align 1, !alias.scope !3
  %i54 = load i64, i64* %i37, align 1, !alias.scope !3
  %i55 = inttoptr i64 %i54 to i8*
  %i56 = bitcast %"QNCA_a0$float*$rank2$"* %i35 to i8**
  %i57 = tail call i32 @for_allocate_handle(i64 76000, i8** nonnull %i56, i32 262144, i8* %i55) #4
  %i58 = add nuw nsw i64 %i6, 1
  %i59 = icmp eq i64 %i58, 101
  br i1 %i59, label %bb60, label %bb5

bb60:                                             ; preds = %bb115, %bb5
  %i61 = phi i64 [ %i116, %bb115 ], [ 1, %bb5 ]
  br label %bb62

bb62:                                             ; preds = %bb112, %bb60
  %i63 = phi i64 [ %i113, %bb112 ], [ 1, %bb60 ]
  %i64 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 76, i32* elementtype(i32) getelementptr inbounds ([1000 x [19 x i32]], [1000 x [19 x i32]]* @"main_$MYK", i64 0, i64 0, i64 0), i64 %i63)
  br label %bb65

bb65:                                             ; preds = %bb65, %bb62
  %i66 = phi i64 [ %i110, %bb65 ], [ 2, %bb62 ]
  %i67 = load %"MAIN$.btPHYSPROP_TYPE"*, %"MAIN$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$"* @"main_$PHYSPROP", i64 0, i32 0), align 16
  %i68 = load i64, i64* %i3, align 1
  %i69 = load i64, i64* %i1, align 1
  %i70 = tail call %"MAIN$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_MAIN$.btPHYSPROP_TYPEs.i64.i64.p0s_MAIN$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %i69, i64 %i68, %"MAIN$.btPHYSPROP_TYPE"* elementtype(%"MAIN$.btPHYSPROP_TYPE") %i67, i64 %i61)
  %t71 = getelementptr inbounds %"MAIN$.btPHYSPROP_TYPE", %"MAIN$.btPHYSPROP_TYPE"* %i70, i64 0, i32 1
  %t72 = load i32, i32* %t71, align 4
  %t73 = add nsw i32 %t72, 2
  store i32 %t73, i32* %t71
  %i71 = getelementptr inbounds %"MAIN$.btPHYSPROP_TYPE", %"MAIN$.btPHYSPROP_TYPE"* %i70, i64 0, i32 2
  %i72 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %i71, i64 0, i32 0
  %i73 = load float*, float** %i72, align 1
  %i74 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %i71, i64 0, i32 6, i64 0
  %i75 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %i74, i64 0, i32 1
  %i76 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %i75, i32 0)
  %i77 = load i64, i64* %i76, align 1
  %i78 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %i74, i64 0, i32 2
  %i79 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %i78, i32 0)
  %i80 = load i64, i64* %i79, align 1
  %i81 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %i75, i32 1)
  %i82 = load i64, i64* %i81, align 1
  %i83 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %i78, i32 1)
  %i84 = load i64, i64* %i83, align 1
  %i85 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %i64, i64 %i66)
  %i86 = load i32, i32* %i85, align 1
  %i87 = add nsw i32 %i86, -1
  %i88 = sext i32 %i87 to i64
  %i89 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %i84, i64 %i82, float* elementtype(float) %i73, i64 %i88)
  %i90 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %i80, i64 %i77, float* elementtype(float) %i89, i64 %i63)
  %i91 = load float, float* %i90, align 1
  %i92 = getelementptr inbounds %"MAIN$.btPHYSPROP_TYPE", %"MAIN$.btPHYSPROP_TYPE"* %i70, i64 0, i32 0
  %i93 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %i92, i64 0, i32 0
  %i94 = load float*, float** %i93, align 1
  %i95 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %i92, i64 0, i32 6, i64 0
  %i96 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %i95, i64 0, i32 1
  %i97 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %i96, i32 0)
  %i98 = load i64, i64* %i97, align 1
  %i99 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %i95, i64 0, i32 2
  %i100 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %i99, i32 0)
  %i101 = load i64, i64* %i100, align 1
  %i102 = add nsw i32 %i86, 1
  %i103 = sext i32 %i102 to i64
  %i104 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %i96, i32 1)
  %i105 = load i64, i64* %i104, align 1
  %i106 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %i99, i32 1)
  %i107 = load i64, i64* %i106, align 1
  %i108 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %i107, i64 %i105, float* elementtype(float) %i94, i64 %i63)
  %i109 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %i101, i64 %i98, float* elementtype(float) %i108, i64 %i103)
  store float %i91, float* %i109, align 1
  %i110 = add nuw nsw i64 %i66, 1
  %i111 = icmp eq i64 %i110, 19
  br i1 %i111, label %bb112, label %bb65

bb112:                                            ; preds = %bb65
  %i113 = add nuw nsw i64 %i63, 1
  %i114 = icmp eq i64 %i113, 1001
  br i1 %i114, label %bb115, label %bb62

bb115:                                            ; preds = %bb112
  %i116 = add nuw nsw i64 %i61, 1
  %i117 = icmp eq i64 %i116, 101
  br i1 %i117, label %bb118, label %bb60

bb118:                                            ; preds = %bb115
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(i32* nocapture readonly) local_unnamed_addr #1

; Function Attrs: nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #2

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, i8** nocapture, i32, i8*) local_unnamed_addr #1

; Function Attrs: nounwind readnone speculatable
declare %"MAIN$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_MAIN$.btPHYSPROP_TYPEs.i64.i64.p0s_MAIN$.btPHYSPROP_TYPEs.i64"(i8, i64, i64, %"MAIN$.btPHYSPROP_TYPE"*, i64) #2

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #2

; Function Attrs: nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #2

; Function Attrs: nofree nosync nounwind readnone willreturn
declare i64 @llvm.ssa.copy.i64(i64 returned) #3

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nounwind readnone speculatable }
attributes #3 = { nofree nosync nounwind readnone willreturn }
attributes #4 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
!3 = !{!4}
!4 = distinct !{!4, !5, !"initfield_: %initfield_$PHYSPROPFIELD"}
!5 = distinct !{!5, !"initfield_"}

; Disabled for [CMPLRLLVM-25349]: opt -passes='default<O3>' -inline-report=0xe807 -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='default<O3>' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -disable-output 2>&1 | FileCheck %s

; Check that by default at -O3, the standard and metadata inlining reports
; are only printed once.

; CHECK: Begin Inlining Report
; CHECK: End Inlining Report
; CHECK-NOT: Begin Inlining Report
; CHECK-NOT: End Inlining Report

; Function Attrs: noinline
define internal i32 @foo(ptr nocapture %arg, ptr noalias nocapture readonly %arg1) #0 {
bb:
  %i = load i32, ptr %arg1, align 4
  %i2 = icmp sgt i32 %i, 0
  br i1 %i2, label %bb3, label %bb70

bb3:                                              ; preds = %bb
  %i4 = zext i32 %i to i64
  %i5 = icmp ult i32 %i, 8
  br i1 %i5, label %bb6, label %bb8

bb6:                                              ; preds = %bb68, %bb3
  %i7 = phi i64 [ 0, %bb3 ], [ %i9, %bb68 ]
  br label %bb77

bb8:                                              ; preds = %bb3
  %i9 = and i64 %i4, 4294967288
  %i10 = add nsw i64 %i9, -8
  %i11 = lshr exact i64 %i10, 3
  %i12 = add nuw nsw i64 %i11, 1
  %i13 = and i64 %i12, 3
  %i14 = icmp ult i64 %i10, 24
  br i1 %i14, label %bb51, label %bb15

bb15:                                             ; preds = %bb8
  %i16 = and i64 %i12, 4611686018427387900
  br label %bb17

bb17:                                             ; preds = %bb17, %bb15
  %i18 = phi i64 [ 0, %bb15 ], [ %i48, %bb17 ]
  %i19 = phi <4 x i32> [ <i32 0, i32 1, i32 2, i32 3>, %bb15 ], [ %i44, %bb17 ]
  %i20 = phi i64 [ %i16, %bb15 ], [ %i49, %bb17 ]
  %i21 = getelementptr inbounds i32, ptr %arg, i64 %i18
  %i22 = add <4 x i32> %i19, <i32 4, i32 4, i32 4, i32 4>
  %i23 = add <4 x i32> %i19, <i32 8, i32 8, i32 8, i32 8>
  store <4 x i32> %i22, ptr %i21, align 4
  %i25 = getelementptr inbounds i32, ptr %i21, i64 4
  store <4 x i32> %i23, ptr %i25, align 4
  %i27 = or i64 %i18, 8
  %i28 = getelementptr inbounds i32, ptr %arg, i64 %i27
  %i29 = add <4 x i32> %i19, <i32 12, i32 12, i32 12, i32 12>
  %i30 = add <4 x i32> %i19, <i32 16, i32 16, i32 16, i32 16>
  store <4 x i32> %i29, ptr %i28, align 4
  %i32 = getelementptr inbounds i32, ptr %i28, i64 4
  store <4 x i32> %i30, ptr %i32, align 4
  %i34 = or i64 %i18, 16
  %i35 = getelementptr inbounds i32, ptr %arg, i64 %i34
  %i36 = add <4 x i32> %i19, <i32 20, i32 20, i32 20, i32 20>
  %i37 = add <4 x i32> %i19, <i32 24, i32 24, i32 24, i32 24>
  store <4 x i32> %i36, ptr %i35, align 4
  %i39 = getelementptr inbounds i32, ptr %i35, i64 4
  store <4 x i32> %i37, ptr %i39, align 4
  %i41 = or i64 %i18, 24
  %i42 = getelementptr inbounds i32, ptr %arg, i64 %i41
  %i43 = add <4 x i32> %i19, <i32 28, i32 28, i32 28, i32 28>
  %i44 = add <4 x i32> %i19, <i32 32, i32 32, i32 32, i32 32>
  store <4 x i32> %i43, ptr %i42, align 4
  %i46 = getelementptr inbounds i32, ptr %i42, i64 4
  store <4 x i32> %i44, ptr %i46, align 4
  %i48 = add i64 %i18, 32
  %i49 = add i64 %i20, -4
  %i50 = icmp eq i64 %i49, 0
  br i1 %i50, label %bb51, label %bb17

bb51:                                             ; preds = %bb17, %bb8
  %i52 = phi i64 [ 0, %bb8 ], [ %i48, %bb17 ]
  %i53 = phi <4 x i32> [ <i32 0, i32 1, i32 2, i32 3>, %bb8 ], [ %i44, %bb17 ]
  %i54 = icmp eq i64 %i13, 0
  br i1 %i54, label %bb68, label %bb55

bb55:                                             ; preds = %bb55, %bb51
  %i56 = phi i64 [ %i65, %bb55 ], [ %i52, %bb51 ]
  %i57 = phi <4 x i32> [ %i61, %bb55 ], [ %i53, %bb51 ]
  %i58 = phi i64 [ %i66, %bb55 ], [ %i13, %bb51 ]
  %i59 = getelementptr inbounds i32, ptr %arg, i64 %i56
  %i60 = add <4 x i32> %i57, <i32 4, i32 4, i32 4, i32 4>
  %i61 = add <4 x i32> %i57, <i32 8, i32 8, i32 8, i32 8>
  store <4 x i32> %i60, ptr %i59, align 4
  %i63 = getelementptr inbounds i32, ptr %i59, i64 4
  store <4 x i32> %i61, ptr %i63, align 4
  %i65 = add i64 %i56, 8
  %i66 = add i64 %i58, -1
  %i67 = icmp eq i64 %i66, 0
  br i1 %i67, label %bb68, label %bb55

bb68:                                             ; preds = %bb55, %bb51
  %i69 = icmp eq i64 %i9, %i4
  br i1 %i69, label %bb70, label %bb6

bb70:                                             ; preds = %bb77, %bb68, %bb
  %i71 = load i32, ptr %arg, align 4
  %i72 = add nsw i32 %i, -1
  %i73 = sext i32 %i72 to i64
  %i74 = getelementptr inbounds i32, ptr %arg, i64 %i73
  %i75 = load i32, ptr %i74, align 4
  %i76 = add nsw i32 %i75, %i71
  ret i32 %i76

bb77:                                             ; preds = %bb77, %bb6
  %i78 = phi i64 [ %i82, %bb77 ], [ %i7, %bb6 ]
  %i79 = getelementptr inbounds i32, ptr %arg, i64 %i78
  %i80 = trunc i64 %i78 to i32
  %i81 = add i32 %i80, 4
  store i32 %i81, ptr %i79, align 4
  %i82 = add nuw nsw i64 %i78, 1
  %i83 = icmp eq i64 %i82, %i4
  br i1 %i83, label %bb70, label %bb77
}

define dso_local i32 @main() {
bb:
  %i = alloca [10 x i32], align 16
  %i1 = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 40, ptr nonnull %i)
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i1)
  store i32 10, ptr %i1, align 4
  %i4 = getelementptr inbounds [10 x i32], ptr %i, i64 0, i64 0
  %i5 = call i32 @foo(ptr nonnull %i4, ptr nonnull %i1)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i1)
  call void @llvm.lifetime.end.p0(i64 40, ptr nonnull %i)
  ret i32 %i5
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { noinline }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }

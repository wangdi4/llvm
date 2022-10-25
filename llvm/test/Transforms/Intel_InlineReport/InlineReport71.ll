; Disabled for [CMPLRLLVM-25349]: opt -passes='default<O3>' -inline-report=0xe807 -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='default<O3>' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -disable-output 2>&1 | FileCheck %s

; Check that by default at -O3, the standard and metadata inlining reports
; are only printed once.

; CHECK: Begin Inlining Report
; CHECK: End Inlining Report
; CHECK-NOT: Begin Inlining Report
; CHECK-NOT: End Inlining Report

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)

define internal i32 @foo(i32* nocapture %0, i32* noalias nocapture readonly %1) #0 {
  %3 = load i32, i32* %1, align 4
  %4 = icmp sgt i32 %3, 0
  br i1 %4, label %5, label %72

5:                                                ; preds = %2
  %6 = zext i32 %3 to i64
  %7 = icmp ult i32 %3, 8
  br i1 %7, label %8, label %10

8:                                                ; preds = %70, %5
  %9 = phi i64 [ 0, %5 ], [ %11, %70 ]
  br label %79

10:                                               ; preds = %5
  %11 = and i64 %6, 4294967288
  %12 = add nsw i64 %11, -8
  %13 = lshr exact i64 %12, 3
  %14 = add nuw nsw i64 %13, 1
  %15 = and i64 %14, 3
  %16 = icmp ult i64 %12, 24
  br i1 %16, label %53, label %17

17:                                               ; preds = %10
  %18 = and i64 %14, 4611686018427387900
  br label %19

19:                                               ; preds = %19, %17
  %20 = phi i64 [ 0, %17 ], [ %50, %19 ]
  %21 = phi <4 x i32> [ <i32 0, i32 1, i32 2, i32 3>, %17 ], [ %46, %19 ]
  %22 = phi i64 [ %18, %17 ], [ %51, %19 ]
  %23 = getelementptr inbounds i32, i32* %0, i64 %20
  %24 = add <4 x i32> %21, <i32 4, i32 4, i32 4, i32 4>
  %25 = add <4 x i32> %21, <i32 8, i32 8, i32 8, i32 8>
  %26 = bitcast i32* %23 to <4 x i32>*
  store <4 x i32> %24, <4 x i32>* %26, align 4
  %27 = getelementptr inbounds i32, i32* %23, i64 4
  %28 = bitcast i32* %27 to <4 x i32>*
  store <4 x i32> %25, <4 x i32>* %28, align 4
  %29 = or i64 %20, 8
  %30 = getelementptr inbounds i32, i32* %0, i64 %29
  %31 = add <4 x i32> %21, <i32 12, i32 12, i32 12, i32 12>
  %32 = add <4 x i32> %21, <i32 16, i32 16, i32 16, i32 16>
  %33 = bitcast i32* %30 to <4 x i32>*
  store <4 x i32> %31, <4 x i32>* %33, align 4
  %34 = getelementptr inbounds i32, i32* %30, i64 4
  %35 = bitcast i32* %34 to <4 x i32>*
  store <4 x i32> %32, <4 x i32>* %35, align 4
  %36 = or i64 %20, 16
  %37 = getelementptr inbounds i32, i32* %0, i64 %36
  %38 = add <4 x i32> %21, <i32 20, i32 20, i32 20, i32 20>
  %39 = add <4 x i32> %21, <i32 24, i32 24, i32 24, i32 24>
  %40 = bitcast i32* %37 to <4 x i32>*
  store <4 x i32> %38, <4 x i32>* %40, align 4
  %41 = getelementptr inbounds i32, i32* %37, i64 4
  %42 = bitcast i32* %41 to <4 x i32>*
  store <4 x i32> %39, <4 x i32>* %42, align 4
  %43 = or i64 %20, 24
  %44 = getelementptr inbounds i32, i32* %0, i64 %43
  %45 = add <4 x i32> %21, <i32 28, i32 28, i32 28, i32 28>
  %46 = add <4 x i32> %21, <i32 32, i32 32, i32 32, i32 32>
  %47 = bitcast i32* %44 to <4 x i32>*
  store <4 x i32> %45, <4 x i32>* %47, align 4
  %48 = getelementptr inbounds i32, i32* %44, i64 4
  %49 = bitcast i32* %48 to <4 x i32>*
  store <4 x i32> %46, <4 x i32>* %49, align 4
  %50 = add i64 %20, 32
  %51 = add i64 %22, -4
  %52 = icmp eq i64 %51, 0
  br i1 %52, label %53, label %19

53:                                               ; preds = %19, %10
  %54 = phi i64 [ 0, %10 ], [ %50, %19 ]
  %55 = phi <4 x i32> [ <i32 0, i32 1, i32 2, i32 3>, %10 ], [ %46, %19 ]
  %56 = icmp eq i64 %15, 0
  br i1 %56, label %70, label %57

57:                                               ; preds = %57, %53
  %58 = phi i64 [ %67, %57 ], [ %54, %53 ]
  %59 = phi <4 x i32> [ %63, %57 ], [ %55, %53 ]
  %60 = phi i64 [ %68, %57 ], [ %15, %53 ]
  %61 = getelementptr inbounds i32, i32* %0, i64 %58
  %62 = add <4 x i32> %59, <i32 4, i32 4, i32 4, i32 4>
  %63 = add <4 x i32> %59, <i32 8, i32 8, i32 8, i32 8>
  %64 = bitcast i32* %61 to <4 x i32>*
  store <4 x i32> %62, <4 x i32>* %64, align 4
  %65 = getelementptr inbounds i32, i32* %61, i64 4
  %66 = bitcast i32* %65 to <4 x i32>*
  store <4 x i32> %63, <4 x i32>* %66, align 4
  %67 = add i64 %58, 8
  %68 = add i64 %60, -1
  %69 = icmp eq i64 %68, 0
  br i1 %69, label %70, label %57

70:                                               ; preds = %57, %53
  %71 = icmp eq i64 %11, %6
  br i1 %71, label %72, label %8

72:                                               ; preds = %79, %70, %2
  %73 = load i32, i32* %0, align 4
  %74 = add nsw i32 %3, -1
  %75 = sext i32 %74 to i64
  %76 = getelementptr inbounds i32, i32* %0, i64 %75
  %77 = load i32, i32* %76, align 4
  %78 = add nsw i32 %77, %73
  ret i32 %78

79:                                               ; preds = %79, %8
  %80 = phi i64 [ %84, %79 ], [ %9, %8 ]
  %81 = getelementptr inbounds i32, i32* %0, i64 %80
  %82 = trunc i64 %80 to i32
  %83 = add i32 %82, 4
  store i32 %83, i32* %81, align 4
  %84 = add nuw nsw i64 %80, 1
  %85 = icmp eq i64 %84, %6
  br i1 %85, label %72, label %79
}

define dso_local i32 @main() {
  %1 = alloca [10 x i32], align 16
  %2 = alloca i32, align 4
  %3 = bitcast [10 x i32]* %1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 40, i8* nonnull %3)
  %4 = bitcast i32* %2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %4)
  store i32 10, i32* %2, align 4
  %5 = getelementptr inbounds [10 x i32], [10 x i32]* %1, i64 0, i64 0
  %6 = call i32 @foo(i32* nonnull %5, i32* nonnull %2)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %4)
  call void @llvm.lifetime.end.p0i8(i64 40, i8* nonnull %3)
  ret i32 %6
}

attributes #0 = { noinline }


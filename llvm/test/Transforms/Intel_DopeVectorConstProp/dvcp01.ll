; REQUIRES: asserts
; RUN: opt < %s -disable-output -dopevectorconstprop -debug-only=dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -passes=dopevectorconstprop -debug-only=dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

; Check that full load, stride, and extent dope vector constant values are
; determined for ARG #0 and ARG #1 of new_solver_.

; Check the trace output.

; CHECK: DOPE VECTOR CONSTANT PROPAGATION: BEGIN
; CHECK: DV FOUND: ARG #0 new_solver_ 2 x i32
; CHECK: VALID
; CHECK: LB[0] = 1
; CHECK: ST[0] = 4
; CHECK: EX[0] = 9
; CHECK: LB[1] = 1
; CHECK: ST[1] = 36
; CHECK: EX[1] = 9
; CHECK: REPLACING 1 LOAD WITH 4
; CHECK: REPLACING 1 LOAD WITH 36
; CHECK: DV FOUND: ARG #1 new_solver_ 3 x i32
; CHECK: VALID
; CHECK: LB[0] = 1
; CHECK: ST[0] = 4
; CHECK: EX[0] = 9
; CHECK: LB[1] = 1
; CHECK: ST[1] = 36
; CHECK: EX[1] = 9
; CHECK: LB[2] = 1
; CHECK: ST[2] = 324
; CHECK: EX[2] = 9
; CHECK: REPLACING 1 LOAD WITH 9
; CHECK: REPLACING 1 LOAD WITH 9
; CHECK: REPLACING 1 LOAD WITH 9
; CHECK: REPLACING 1 LOAD WITH 4
; CHECK: REPLACING 1 LOAD WITH 36
; CHECK: REPLACING 1 LOAD WITH 324
; CHECK: DOPE VECTOR CONSTANT PROPAGATION: END

@"main_$PART" = internal global [9 x [9 x i32]] zeroinitializer, align 16

@"main_$BLOCK" = internal global [9 x [9 x [9 x i32]]] zeroinitializer, align 16

@0 = internal unnamed_addr constant i32 2

@anon.a87c7c812e60d4624ad0dfec6a834de1.0 = internal unnamed_addr constant i32 2

declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #1

declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #1

declare dso_local i32 @for_set_reentrancy(i32*) local_unnamed_addr

declare i8* @llvm.stacksave()

declare void @llvm.stackrestore(i8*)

define dso_local void @MAIN__() {
  %1 = alloca { i64, i8* }, align 8
  %2 = alloca [4 x i8], align 1
  %3 = alloca { i64, i8* }, align 8
  %4 = alloca [4 x i8], align 1
  %5 = alloca { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, align 8
; NOTE: %5 is the dope vector for new_solver actual arg #1
  %6 = alloca { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
; NOTE: %6 is the dope vector for new_solver actual arg #0
  %7 = alloca [8 x i64], align 16
  %8 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %6, i64 0, i32 1
  %9 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %6, i64 0, i32 3
  %10 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %6, i64 0, i32 6, i64 0
; NOTE: %10 is the dope vector dimension base for new_solver actual arg #0
  %11 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %10, i64 0, i32 0
; NOTE: %11 is the dope vector base for the extent of new_solver actual arg #0
  %12 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %10, i64 0, i32 2
; NOTE: %12 is the dope vector base for the lower bound of new_solver actual arg #0
  %13 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %5, i64 0, i32 1
  %14 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %5, i64 0, i32 3
  %15 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %5, i64 0, i32 6, i64 0
; NOTE: %15 is the dope vector dimension base for new_solver actual arg #1
  %16 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %15, i64 0, i32 1
; NOTE: %16 is the dope vector base for stride of new_solver actual arg #1
  %17 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.a87c7c812e60d4624ad0dfec6a834de1.0)
  br label %18

18:                                               ; preds = %28, %0
  %19 = phi i64 [ %29, %28 ], [ 1, %0 ]
  br label %20

20:                                               ; preds = %20, %18
  %21 = phi i64 [ %26, %20 ], [ 1, %18 ]
  %22 = sub nsw i64 %19, %21
  %23 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @"main_$PART", i64 0, i64 0, i64 0), i64 %21)
  %24 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %23, i64 %19)
  %25 = trunc i64 %22 to i32
  store i32 %25, i32* %24, align 4
  %26 = add nuw nsw i64 %21, 1
  %27 = icmp eq i64 %26, 10
  br i1 %27, label %28, label %20

28:                                               ; preds = %20
  %29 = add nuw nsw i64 %19, 1
  %30 = icmp eq i64 %29, 10
  br i1 %30, label %31, label %18

31:                                               ; preds = %28
  %32 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %6, i64 0, i32 0
  %33 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %6, i64 0, i32 2
  %34 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %6, i64 0, i32 4
  %35 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %10, i64 0, i32 1
; NOTE: %35 is the dope vector base for stride of new_solver actual arg #0
  %36 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %5, i64 0, i32 0
  %37 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %5, i64 0, i32 2
  %38 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %5, i64 0, i32 4
  %39 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %15, i64 0, i32 0
; NOTE: %39 is the dope vector base for extent of new_solver actual arg #1
  %40 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %15, i64 0, i32 2
; NOTE: %40 is the dope vector base for lower bound of new_solver actual arg #1
  store i64 4, i64* %8, align 8
  store i64 2, i64* %34, align 8
  store i64 0, i64* %33, align 8
  %41 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %35, i32 0)
; NOTE: stride 0 of new solver arg #0 is 4
  store i64 4, i64* %41, align 8
  %42 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %12, i32 0)
; NOTE: lower bound 0 of new solver arg #0 is 1
  store i64 1, i64* %42, align 8
  %43 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %11, i32 0)
; NOTE: extent 0 of new solver arg #0 is 9
  store i64 9, i64* %43, align 8
  %44 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %35, i32 1)
; NOTE: stride 1 of new solver arg #0 is 36
  store i64 36, i64* %44, align 8
  %45 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %12, i32 1)
; NOTE: lower bound 1 of new solver arg #0 is 1
  store i64 1, i64* %45, align 8
  %46 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %11, i32 1)
; NOTE: extent 1 of new solver arg #0 is 9
  store i64 9, i64* %46, align 8
  store i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @"main_$PART", i64 0, i64 0, i64 0), i32** %32, align 8
  store i64 1, i64* %9, align 8
  store i64 4, i64* %13, align 8
  store i64 3, i64* %38, align 8
  store i64 0, i64* %37, align 8
  %47 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %16, i32 0)
; NOTE: stride 0 of new solver arg #1 is 4
  store i64 4, i64* %47, align 8
  %48 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %40, i32 0)
; NOTE: lower bound 0 of new solver arg #1 is 1
  store i64 1, i64* %48, align 8
  %49 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %39, i32 0)
; NOTE: extent 0 of new solver arg #1 is 9
  store i64 9, i64* %49, align 8
  %50 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %16, i32 1)
; NOTE: stride 1 of new solver arg #1 is 36
  store i64 36, i64* %50, align 8
  %51 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %40, i32 1)
; NOTE: lower bound 1 of new solver arg #1 is 1
  store i64 1, i64* %51, align 8
  %52 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %39, i32 1)
; NOTE: extent 1 of new solver arg #1 is 9
  store i64 9, i64* %52, align 8
  %53 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %16, i32 2)
; NOTE: stride 2 of new solver arg #1 is 324
  store i64 324, i64* %53, align 8
  %54 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %40, i32 2)
; NOTE: lower bound 2 of new solver arg #1 is 1
  store i64 1, i64* %54, align 8
  %55 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %39, i32 2)
; NOTE: extent 2 of new solver arg #1 is 9
  store i64 9, i64* %55, align 8
  store i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @"main_$BLOCK", i64 0, i64 0, i64 0, i64 0), i32** %36, align 8
  store i64 1, i64* %14, align 8
  call void @new_solver_({ i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* nonnull %6, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* nonnull %5)
  ret void
}


define internal void @new_solver_({ i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* noalias nocapture readonly %0, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* noalias nocapture readonly %1) {
  %3 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %1, i64 0, i32 0
; NOTE: Load the dope vector dimension base for arg #1 into %4
  %4 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %1, i64 0, i32 6, i64 0
; NOTE: Load the base address for the extent for arg #1 into %5
  %5 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %4, i64 0, i32 0
; NOTE: Load the base address of the stride for arg #1 into %6
  %6 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %4, i64 0, i32 1
  %7 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %0, i64 0, i32 0
; NOTE: Load the dope vector dimension base for arg #0 (%0) into %8
  %8 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %0, i64 0, i32 6, i64 0
; NOTE: Load the base address for the stride of arg #0 into %9
  %9 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %8, i64 0, i32 1
  %10 = load i32*, i32** %3, align 8
  %11 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %6, i32 0)
; NOTE: Load stride 0 value for arg #1 into %12
  %12 = load i64, i64* %11, align 8
  %13 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %5, i32 0)
; NOTE: Load extent 0 value for arg #1 into %14
  %14 = load i64, i64* %13, align 8
  %15 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %6, i32 1)
; NOTE: Load stride 1 value for arg #1 into %16
  %16 = load i64, i64* %15, align 8
  %17 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %5, i32 1)
; NOTE: Load extent 1 value for arg #1 into %18
  %18 = load i64, i64* %17, align 8
  %19 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %6, i32 2)
; NOTE: Load stride 2 value for arg #1 into %20
  %20 = load i64, i64* %19, align 8
  %21 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %5, i32 2)
; NOTE: Load extent 2 value for arg #1 into %22
  %22 = load i64, i64* %21, align 8
  %23 = icmp slt i64 %22, 1
; NOTE: replace %22 with 9
  %24 = icmp slt i64 %18, 1
; NOTE: replace %18 with 9
  %25 = or i1 %23, %24
  %26 = icmp slt i64 %14, 1
; NOTE: replace %14 with 9
  %27 = or i1 %25, %26
  br i1 %27, label %45, label %42

28:                                               ; preds = %36, %28
  %29 = phi i64 [ 1, %36 ], [ %31, %28 ]
  %30 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %12, i32* %38, i64 %29)
; NOTE: replace %12 with 4
  store i32 0, i32* %30, align 4
  %31 = add nuw i64 %29, 1
  %32 = icmp eq i64 %29, %14
; NOTE: replace %14 with 9
  br i1 %32, label %33, label %28

33:                                               ; preds = %28
  %34 = add nuw i64 %37, 1
  %35 = icmp eq i64 %37, %18
; NOTE: replace %18 with 9
  br i1 %35, label %39, label %36

36:                                               ; preds = %42, %33
  %37 = phi i64 [ 1, %42 ], [ %34, %33 ]
  %38 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %16, i32* %44, i64 %37)
; NOTE: replace %16 with 36
  br label %28

39:                                               ; preds = %33
  %40 = add nuw i64 %43, 1
  %41 = icmp eq i64 %43, %22
; NOTE: replace %22 with 9
  br i1 %41, label %45, label %42

42:                                               ; preds = %39, %2
  %43 = phi i64 [ %40, %39 ], [ 1, %2 ]
  %44 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 %20, i32* %10, i64 %43)
; NOTE: replace %20 with 324
  br label %36

45:                                               ; preds = %39, %2
  %46 = load i32*, i32** %7, align 8
  %47 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %9, i32 0)
; NOTE: Load stride 0 value for arg #0 into %48
  %48 = load i64, i64* %47, align 8
  %49 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %9, i32 1)
; NOTE: Load stride 1 value for arg #0 into %50
  %50 = load i64, i64* %49, align 8
  br label %51

51:                                               ; preds = %83, %45
  %52 = phi i64 [ 1, %45 ], [ %84, %83 ]
  br label %53

53:                                               ; preds = %80, %51
  %54 = phi i64 [ %81, %80 ], [ 1, %51 ]
  %55 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %50, i32* %46, i64 %54)
; NOTE: replace %50 with 36
  %56 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %48, i32* %55, i64 %52)
; NOTE: replace %48 with 4
  %57 = load i32, i32* %56, align 4
  %58 = icmp eq i32 %57, 0
  br i1 %58, label %76, label %80

59:                                               ; preds = %76, %59
  %60 = phi i64 [ 1, %76 ], [ %63, %59 ]
  %61 = phi i32 [ 1, %76 ], [ %64, %59 ]
  %62 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %79, i64 %60)
  store i32 %61, i32* %62, align 4
  %63 = add nuw nsw i64 %60, 1
  %64 = add nuw nsw i32 %61, 1
  %65 = icmp eq i64 %63, 10
  br i1 %65, label %66, label %59

66:                                               ; preds = %66, %59
  %67 = phi i64 [ %73, %66 ], [ 1, %59 ]
  %68 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %79, i64 %67)
  %69 = load i32, i32* %68, align 4
  %70 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 %20, i32* %10, i64 %67)
; NOTE: replace %20 with 324
  %71 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %16, i32* %70, i64 %54)
; NOTE: replace %16 with 36
  %72 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %12, i32* %71, i64 %52)
  store i32 %69, i32* %72, align 4
  %73 = add nuw nsw i64 %67, 1
  %74 = icmp eq i64 %73, 10
  br i1 %74, label %75, label %66

75:                                               ; preds = %66
  tail call void @llvm.stackrestore(i8* %77)
  br label %80

76:                                               ; preds = %53
  %77 = tail call i8* @llvm.stacksave()
  %78 = alloca [9 x i32], align 4
  %79 = getelementptr inbounds [9 x i32], [9 x i32]* %78, i64 0, i64 0
  br label %59

80:                                               ; preds = %75, %53
  %81 = add nuw nsw i64 %54, 1
  %82 = icmp eq i64 %81, 10
  br i1 %82, label %83, label %53

83:                                               ; preds = %80
  %84 = add nuw nsw i64 %52, 1
  %85 = icmp eq i64 %84, 10
  br i1 %85, label %86, label %51

86:                                               ; preds = %83
  ret void
}

; RUN: opt < %s -S -dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s
; RUN: opt < %s -S -passes=dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

; Check that full load, stride, and extent dope vector constant values are
; determined for ARG #1 of new_solver_, but only some lower bound and stride
; constant values are determined for ARG #0.

; This is the same test as dvcp02.ll, but checks the IR rather than the traces.

; Check the IR

@"main_$PART1" = internal global [9 x [9 x i32]] zeroinitializer, align 16

@"main_$PART2" = internal global [10 x [10 x i32]] zeroinitializer, align 16

@"main_$BLOCK" = internal global [9 x [9 x [9 x i32]]] zeroinitializer, align 16

@0 = internal unnamed_addr constant i32 2

@anon.a87c7c812e60d4624ad0dfec6a834de1.0 = internal unnamed_addr constant i32 2

declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #1

declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #1

declare dso_local i32 @for_set_reentrancy(i32*) local_unnamed_addr

declare i8* @llvm.stacksave()

declare void @llvm.stackrestore(i8*)

define dso_local void @MAIN__() #0 {
  %1 = alloca { i64, i8* }, align 8
  %2 = alloca [4 x i8], align 1
  %3 = alloca { i64, i8* }, align 8
  %4 = alloca [4 x i8], align 1
  %5 = alloca { i64, i8* }, align 8
  %6 = alloca [4 x i8], align 1
  %7 = alloca { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, align 8
; NOTE: %7 is the dope vector for new_solver actual arg #1 instance #2
  %8 = alloca { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
; NOTE: %8 is the dope vector for new_solver actual arg #0 instance #2
  %9 = alloca { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, align 8
; NOTE: %9 is the dope vector for new_solver actual arg #1 instance #1
  %10 = alloca { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
; NOTE: %10 is the dope vector for new_solver actual arg #0 instance #1
  %11 = alloca [8 x i64], align 16
  %12 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %10, i64 0, i32 1
  %13 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %10, i64 0, i32 3
  %14 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %10, i64 0, i32 6, i64 0
; NOTE: %14 is is the dope vector dimension base for new_solver actual arg #0 instance #1
  %15 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %14, i64 0, i32 0
; NOTE: %15 is the dope vector base for the extent of new_solver actual arg #0 instance #1
  %16 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %14, i64 0, i32 2
; NOTE: %16 is the dope vector base for the lower bound of new_solver actual arg #0 instance #1
  %17 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %9, i64 0, i32 1
  %18 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %9, i64 0, i32 3
  %19 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %9, i64 0, i32 6, i64 0
; NOTE: %19 is is the dope vector dimension base for new_solver actual arg #1 instance #1
  %20 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %19, i64 0, i32 0
; NOTE: %20 is the dope vector base for the extent of new_solver actual arg #1 instance #1
  %21 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %19, i64 0, i32 2
; NOTE: %21 is the dope vector base for the lower bound of new_solver actual arg #1 instance #1
  %22 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %8, i64 0, i32 1
  %23 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %8, i64 0, i32 3
  %24 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %8, i64 0, i32 6, i64 0
; NOTE: %24 is is the dope vector dimension base for new_solver actual arg #0 instance #2
  %25 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %24, i64 0, i32 0
; NOTE: %25 is the dope vector base for the extent of new_solver actual arg #0 instance #2
  %26 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %24, i64 0, i32 2
; NOTE: %26 is the dope vector base for the lower bound of new_solver actual arg #0 instance #2
  %27 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %7, i64 0, i32 1
  %28 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %7, i64 0, i32 3
  %29 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %7, i64 0, i32 6, i64 0
; NOTE: %29 is is the dope vector dimension base for new_solver actual arg #1 instance #2
  %30 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %29, i64 0, i32 1
; NOTE: %30 is the dope vector base for the stride of new_solver actual arg #1 instance #2
  %31 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.a87c7c812e60d4624ad0dfec6a834de1.0)
  br label %32

32:                                               ; preds = %42, %0
  %33 = phi i64 [ %43, %42 ], [ 1, %0 ]
  br label %34

34:                                               ; preds = %34, %32
  %35 = phi i64 [ %40, %34 ], [ 1, %32 ]
  %36 = sub nsw i64 %33, %35
  %37 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @"main_$PART1", i64 0, i64 0, i64 0), i64 %35)
  %38 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %37, i64 %33)
  %39 = trunc i64 %36 to i32
  store i32 %39, i32* %38, align 4
  %40 = add nuw nsw i64 %35, 1
  %41 = icmp eq i64 %40, 10
  br i1 %41, label %42, label %34

42:                                               ; preds = %34
  %43 = add nuw nsw i64 %33, 1
  %44 = icmp eq i64 %43, 10
  br i1 %44, label %45, label %32

45:                                               ; preds = %42
  %46 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %10, i64 0, i32 0
  %47 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %10, i64 0, i32 2
  %48 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %10, i64 0, i32 4
  %49 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %14, i64 0, i32 1
; NOTE: %49 is the dope vector base for the stride of new_solver actual arg #0 instance #1
  %50 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %9, i64 0, i32 0
  %51 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %9, i64 0, i32 2
  %52 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %9, i64 0, i32 4
  %53 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %19, i64 0, i32 1
; NOTE: %53 is the dope vector base for the stride of new_solver actual arg #1 instance #1
  %54 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %8, i64 0, i32 0
  %55 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %8, i64 0, i32 2
  %56 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %8, i64 0, i32 4
  %57 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %24, i64 0, i32 1
; NOTE: %57 is the dope vector base for the stride of new_solver actual arg #0 instance #2
  %58 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %7, i64 0, i32 0
  %59 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %7, i64 0, i32 2
  %60 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %7, i64 0, i32 4
  %61 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %29, i64 0, i32 0
; NOTE: %61 is the dope vector base for the extent of new_solver actual arg #1 instance #2
  %62 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %29, i64 0, i32 2
; NOTE: %62 is the dope vector base for the lower bound of new_solver actual arg #1 instance #2
  br label %63

63:                                               ; preds = %73, %45
  %64 = phi i64 [ 0, %45 ], [ %74, %73 ]
  br label %65
65:                                               ; preds = %65, %63
  %66 = phi i64 [ %71, %65 ], [ 0, %63 ]
  %67 = sub nsw i64 %64, %66
  %68 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 0, i64 40, i32* getelementptr inbounds ([10 x [10 x i32]], [10 x [10 x i32]]* @"main_$PART2", i64 0, i64 0, i64 0), i64 %66)
  %69 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 0, i64 4, i32* %68, i64 %64)
  %70 = trunc i64 %67 to i32
  store i32 %70, i32* %69, align 4
  %71 = add nuw nsw i64 %66, 1
  %72 = icmp eq i64 %71, 10
  br i1 %72, label %73, label %65

73:                                               ; preds = %65
  %74 = add nuw nsw i64 %64, 1
  %75 = icmp eq i64 %74, 10
  br i1 %75, label %76, label %63

76:                                               ; preds = %73
  store i64 4, i64* %12, align 8
  store i64 2, i64* %48, align 8
  store i64 0, i64* %47, align 8
  %77 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %49, i32 0)
; NOTE: stride 0 of new_solver actual arg #0 instance #1 is 4
  store i64 4, i64* %77, align 8
  %78 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %16, i32 0)
; NOTE: lower bound 0 of new_solver actual arg #0 instance #1 is 1
  store i64 1, i64* %78, align 8
  %79 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %15, i32 0)
; NOTE: extent 0 of new_solver actual arg #0 instance #1 is 9
  store i64 9, i64* %79, align 8
  %80 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %49, i32 1)
; NOTE: stride 1 of new_solver actual arg #0 instance #1 is 36
  store i64 36, i64* %80, align 8
  %81 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %16, i32 1)
; NOTE: lower bound 1 of new_solver actual arg #0 instance #1 is 1
  store i64 1, i64* %81, align 8
  %82 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %15, i32 1)
; NOTE: extent 1 of new_solver actual arg #0 instance #1 is 9
  store i64 9, i64* %82, align 8
  store i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @"main_$PART1", i64 0, i64 0, i64 0), i32** %46, align 8
  store i64 1, i64* %13, align 8
  store i64 4, i64* %17, align 8
  store i64 3, i64* %52, align 8
  store i64 0, i64* %51, align 8
  %83 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %53, i32 0)
; NOTE: stride 0 of new_solver actual arg #1 instance #1 is 4
  store i64 4, i64* %83, align 8
  %84 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %21, i32 0)
; NOTE: lower bound 0 of new_solver actual arg #1 instance #1 is 1
  store i64 1, i64* %84, align 8
  %85 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %20, i32 0)
; NOTE: extent 0 of new_solver actual arg #1 instance #1 is 9
  store i64 9, i64* %85, align 8
  %86 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %53, i32 1)
; NOTE: stride 1 of new_solver actual arg #1 instance #1 is 36
  store i64 36, i64* %86, align 8
  %87 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %21, i32 1)
; NOTE: lower bound 1 of new_solver actual arg #1 instance #1 is 1
  store i64 1, i64* %87, align 8
  %88 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %20, i32 1)
; NOTE: extent 1 of new_solver actual arg #1 instance #1 is 9
  store i64 9, i64* %88, align 8
  %89 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %53, i32 2)
; NOTE: stride 2 of new_solver actual arg #1 instance #1 is 324
  store i64 324, i64* %89, align 8
  %90 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %21, i32 2)
; NOTE: lower bound 2 of new_solver actual arg #1 instance #1 is 1
  store i64 1, i64* %90, align 8
  %91 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %20, i32 2)
; NOTE: extent 2 of new_solver actual arg #1 instance #1 is 9
  store i64 9, i64* %91, align 8
  store i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @"main_$BLOCK", i64 0, i64 0, i64 0, i64 0), i32** %50, align 8
  store i64 1, i64* %18, align 8
  call void @new_solver_({ i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* nonnull %10, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* nonnull %9)
  store i64 4, i64* %22, align 8
  store i64 2, i64* %56, align 8
  store i64 0, i64* %55, align 8
  %92 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %57, i32 0)
; NOTE: stride 0 of new_solver actual arg #0 instance #2 is 4
  store i64 4, i64* %92, align 8
  %93 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %26, i32 0)
; NOTE: lower bound 0 of new_solver actual arg #0 instance #2 is 1
  store i64 1, i64* %93, align 8
  %94 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %25, i32 0)
; NOTE: extent 0 of new_solver actual arg #0 instance #2 is 10
  store i64 10, i64* %94, align 8
  %95 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %57, i32 1)
; NOTE: stride 1 of new_solver actual arg #0 instance #2 is 40
  store i64 40, i64* %95, align 8
  %96 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %26, i32 1)
; NOTE: lower bound 1 of new_solver actual arg #0 instance #2 is 1
  store i64 1, i64* %96, align 8
  %97 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %25, i32 1)
; NOTE: extent 1 of new_solver actual arg #0 instance #2 is 10
  store i64 10, i64* %97, align 8
  store i32* getelementptr inbounds ([10 x [10 x i32]], [10 x [10 x i32]]* @"main_$PART2", i64 0, i64 0, i64 0), i32** %54, align 8
  store i64 1, i64* %23, align 8
  store i64 4, i64* %27, align 8
  store i64 3, i64* %60, align 8
  store i64 0, i64* %59, align 8
  %98 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %30, i32 0)
; NOTE: stride 0 of new_solver actual arg #1 instance #2 is 4
  store i64 4, i64* %98, align 8
  %99 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %62, i32 0)
; NOTE: lower bound 0 of new_solver actual arg #1 instance #2 is 1
  store i64 1, i64* %99, align 8
  %100 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %61, i32 0)
; NOTE: extent 0 of new_solver actual arg #1 instance #2 is 9
  store i64 9, i64* %100, align 8
  %101 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %30, i32 1)
; NOTE: stride 1 of new_solver actual arg #1 instance #2 is 36
  store i64 36, i64* %101, align 8
  %102 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %62, i32 1)
; NOTE: lower bound 1 of new_solver actual arg #1 instance #2 is 1
  store i64 1, i64* %102, align 8
  %103 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %61, i32 1)
; NOTE: extent 1 of new_solver actual arg #1 instance #2 is 9
  store i64 9, i64* %103, align 8
  %104 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %30, i32 2)
; NOTE: stride 2 of new_solver actual arg #1 instance #2 is 324
  store i64 324, i64* %104, align 8
  %105 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %62, i32 2)
; NOTE: lower bound 2 of new_solver actual arg #1 instance #2 is 1
  store i64 1, i64* %105, align 8
  %106 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %61, i32 2)
; NOTE: extent 2 of new_solver actual arg #1 instance #2 is 9
  store i64 9, i64* %106, align 8
  store i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @"main_$BLOCK", i64 0, i64 0, i64 0, i64 0), i32** %58, align 8
  store i64 1, i64* %28, align 8
  call void @new_solver_({ i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* nonnull %8, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* nonnull %7)
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
; CHECK: [[V:%[0-9]+]] = icmp slt i64 9, 1
  %24 = icmp slt i64 %18, 1
; NOTE: replace %18 with 9
; CHECK: [[V:%[0-9]+]] = icmp slt i64 9, 1
  %25 = or i1 %23, %24
  %26 = icmp slt i64 %14, 1
; NOTE: replace %14 with 9
; CHECK: [[V:%[0-9]+]] = icmp slt i64 9, 1
  %27 = or i1 %25, %26
  br i1 %27, label %45, label %42

28:                                               ; preds = %36, %28
  %29 = phi i64 [ 1, %36 ], [ %31, %28 ]
  %30 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %12, i32* %38, i64 %29)
; NOTE: replace %12 with 4
; CHECK: [[V:%[0-9]+]] = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4,{{.*}})
  store i32 0, i32* %30, align 4
  %31 = add nuw i64 %29, 1
  %32 = icmp eq i64 %29, %14
; CHECK: [[V:%[0-9]+]] = icmp eq i64 {{.*}}, 9
; NOTE: replace %14 with 9
  br i1 %32, label %33, label %28

33:                                               ; preds = %28
  %34 = add nuw i64 %37, 1
  %35 = icmp eq i64 %37, %18
; CHECK: [[V:%[0-9]+]] = icmp eq i64 {{.*}}, 9
; NOTE: replace %18 with 9
  br i1 %35, label %39, label %36

36:                                               ; preds = %42, %33
  %37 = phi i64 [ 1, %42 ], [ %34, %33 ]
  %38 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %16, i32* %44, i64 %37)
; NOTE: replace %16 with 36
; CHECK: [[V:%[0-9]+]] = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36,{{.*}})
  br label %28

39:                                               ; preds = %33
  %40 = add nuw i64 %43, 1
  %41 = icmp eq i64 %43, %22
; NOTE: replace %22 with 9
; CHECK: [[V:%[0-9]+]] = icmp eq i64 {{.*}}, 9
  br i1 %41, label %45, label %42

42:                                               ; preds = %39, %2
  %43 = phi i64 [ %40, %39 ], [ 1, %2 ]
  %44 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 %20, i32* %10, i64 %43)
; NOTE: replace %20 with 324
; CHECK: [[V:%[0-9]+]] = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 324,{{.*}})
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
; NOTE: DO NOT replace %50 with 36
; CHECK-NOT: [[V:%[0-9]+]] = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36,{{.*}})
  %56 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %48, i32* %55, i64 %52)
; NOTE: replace %48 with 4
; CHECK: [[V:%[0-9]+]] = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4,{{.*}})
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
; CHECK: [[V:%[0-9]+]] = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 324,{{.*}})
  %71 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %16, i32* %70, i64 %54)
; NOTE: replace %16 with 36
; CHECK: [[V:%[0-9]+]] = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36,{{.*}})
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

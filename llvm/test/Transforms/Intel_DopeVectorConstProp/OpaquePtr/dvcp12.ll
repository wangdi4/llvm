; RUN: opt < %s -opaque-pointers -S -dope-vector-local-const-prop=false -passes=dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

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

declare dso_local i32 @for_set_reentrancy(ptr) local_unnamed_addr

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare ptr @llvm.stacksave() #0

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore(ptr) #0

define dso_local void @MAIN__() #1 {
  %1 = alloca { i64, ptr }, align 8
  %2 = alloca [4 x i8], align 1
  %3 = alloca { i64, ptr }, align 8
  %4 = alloca [4 x i8], align 1
  %5 = alloca { i64, ptr }, align 8
  %6 = alloca [4 x i8], align 1
  %7 = alloca { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, align 8
  %8 = alloca { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %9 = alloca { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, align 8
  %10 = alloca { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %11 = alloca [8 x i64], align 16
  %12 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %10, i64 0, i32 1
  %13 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %10, i64 0, i32 3
  %14 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %10, i64 0, i32 6, i64 0
  %15 = getelementptr inbounds { i64, i64, i64 }, ptr %14, i64 0, i32 0
  %16 = getelementptr inbounds { i64, i64, i64 }, ptr %14, i64 0, i32 2
  %17 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %9, i64 0, i32 1
  %18 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %9, i64 0, i32 3
  %19 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %9, i64 0, i32 6, i64 0
  %20 = getelementptr inbounds { i64, i64, i64 }, ptr %19, i64 0, i32 0
  %21 = getelementptr inbounds { i64, i64, i64 }, ptr %19, i64 0, i32 2
  %22 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %8, i64 0, i32 1
  %23 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %8, i64 0, i32 3
  %24 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %8, i64 0, i32 6, i64 0
  %25 = getelementptr inbounds { i64, i64, i64 }, ptr %24, i64 0, i32 0
  %26 = getelementptr inbounds { i64, i64, i64 }, ptr %24, i64 0, i32 2
  %27 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %7, i64 0, i32 1
  %28 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %7, i64 0, i32 3
  %29 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %7, i64 0, i32 6, i64 0
  %30 = getelementptr inbounds { i64, i64, i64 }, ptr %29, i64 0, i32 1
  %31 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.a87c7c812e60d4624ad0dfec6a834de1.0)
  br label %32

32:                                               ; preds = %42, %0
  %33 = phi i64 [ %43, %42 ], [ 1, %0 ]
  br label %34

34:                                               ; preds = %34, %32
  %35 = phi i64 [ %40, %34 ], [ 1, %32 ]
  %36 = sub nsw i64 %33, %35
  %37 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) @"main_$PART1", i64 %35)
  %38 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %37, i64 %33)
  %39 = trunc i64 %36 to i32
  store i32 %39, ptr %38, align 4
  %40 = add nuw nsw i64 %35, 1
  %41 = icmp eq i64 %40, 10
  br i1 %41, label %42, label %34

42:                                               ; preds = %34
  %43 = add nuw nsw i64 %33, 1
  %44 = icmp eq i64 %43, 10
  br i1 %44, label %45, label %32

45:                                               ; preds = %42
  %46 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %10, i64 0, i32 0
  %47 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %10, i64 0, i32 2
  %48 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %10, i64 0, i32 4
  %49 = getelementptr inbounds { i64, i64, i64 }, ptr %14, i64 0, i32 1
  %50 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %9, i64 0, i32 0
  %51 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %9, i64 0, i32 2
  %52 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %9, i64 0, i32 4
  %53 = getelementptr inbounds { i64, i64, i64 }, ptr %19, i64 0, i32 1
  %54 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %8, i64 0, i32 0
  %55 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %8, i64 0, i32 2
  %56 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %8, i64 0, i32 4
  %57 = getelementptr inbounds { i64, i64, i64 }, ptr %24, i64 0, i32 1
  %58 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %7, i64 0, i32 0
  %59 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %7, i64 0, i32 2
  %60 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %7, i64 0, i32 4
  %61 = getelementptr inbounds { i64, i64, i64 }, ptr %29, i64 0, i32 0
  %62 = getelementptr inbounds { i64, i64, i64 }, ptr %29, i64 0, i32 2
  br label %63

63:                                               ; preds = %73, %45
  %64 = phi i64 [ 0, %45 ], [ %74, %73 ]
  br label %65

65:                                               ; preds = %65, %63
  %66 = phi i64 [ %71, %65 ], [ 0, %63 ]
  %67 = sub nsw i64 %64, %66
  %68 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 40, ptr elementtype(i32) @"main_$PART2", i64 %66)
  %69 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 4, ptr elementtype(i32) %68, i64 %64)
  %70 = trunc i64 %67 to i32
  store i32 %70, ptr %69, align 4
  %71 = add nuw nsw i64 %66, 1
  %72 = icmp eq i64 %71, 10
  br i1 %72, label %73, label %65

73:                                               ; preds = %65
  %74 = add nuw nsw i64 %64, 1
  %75 = icmp eq i64 %74, 10
  br i1 %75, label %76, label %63

76:                                               ; preds = %73
  store i64 4, ptr %12, align 8
  store i64 2, ptr %48, align 8
  store i64 0, ptr %47, align 8
  %77 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %49, i32 0)
  store i64 4, ptr %77, align 8
  %78 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %16, i32 0)
  store i64 1, ptr %78, align 8
  %79 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %15, i32 0)
  store i64 9, ptr %79, align 8
  %80 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %49, i32 1)
  store i64 36, ptr %80, align 8
  %81 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %16, i32 1)
  store i64 1, ptr %81, align 8
  %82 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %15, i32 1)
  store i64 9, ptr %82, align 8
  store ptr @"main_$PART1", ptr %46, align 8
  store i64 1, ptr %13, align 8
  store i64 4, ptr %17, align 8
  store i64 3, ptr %52, align 8
  store i64 0, ptr %51, align 8
  %83 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %53, i32 0)
  store i64 4, ptr %83, align 8
  %84 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %21, i32 0)
  store i64 1, ptr %84, align 8
  %85 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %20, i32 0)
  store i64 9, ptr %85, align 8
  %86 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %53, i32 1)
  store i64 36, ptr %86, align 8
  %87 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %21, i32 1)
  store i64 1, ptr %87, align 8
  %88 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %20, i32 1)
  store i64 9, ptr %88, align 8
  %89 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %53, i32 2)
  store i64 324, ptr %89, align 8
  %90 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %21, i32 2)
  store i64 1, ptr %90, align 8
  %91 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %20, i32 2)
  store i64 9, ptr %91, align 8
  store ptr @"main_$BLOCK", ptr %50, align 8
  store i64 1, ptr %18, align 8
  call void @new_solver_(ptr nonnull %10, ptr nonnull %9)
  store i64 4, ptr %22, align 8
  store i64 2, ptr %56, align 8
  store i64 0, ptr %55, align 8
  %92 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %57, i32 0)
  store i64 4, ptr %92, align 8
  %93 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %26, i32 0)
  store i64 1, ptr %93, align 8
  %94 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %25, i32 0)
  store i64 10, ptr %94, align 8
  %95 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %57, i32 1)
  store i64 40, ptr %95, align 8
  %96 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %26, i32 1)
  store i64 1, ptr %96, align 8
  %97 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %25, i32 1)
  store i64 10, ptr %97, align 8
  store ptr @"main_$PART2", ptr %54, align 8
  store i64 1, ptr %23, align 8
  store i64 4, ptr %27, align 8
  store i64 3, ptr %60, align 8
  store i64 0, ptr %59, align 8
  %98 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %30, i32 0)
  store i64 4, ptr %98, align 8
  %99 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %62, i32 0)
  store i64 1, ptr %99, align 8
  %100 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %61, i32 0)
  store i64 9, ptr %100, align 8
  %101 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %30, i32 1)
  store i64 36, ptr %101, align 8
  %102 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %62, i32 1)
  store i64 1, ptr %102, align 8
  %103 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %61, i32 1)
  store i64 9, ptr %103, align 8
  %104 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %30, i32 2)
  store i64 324, ptr %104, align 8
  %105 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %62, i32 2)
  store i64 1, ptr %105, align 8
  %106 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %61, i32 2)
  store i64 9, ptr %106, align 8
  store ptr @"main_$BLOCK", ptr %58, align 8
  store i64 1, ptr %28, align 8
  call void @new_solver_(ptr nonnull %8, ptr nonnull %7)
  ret void
}

define internal void @new_solver_(ptr noalias nocapture readonly %0, ptr noalias nocapture readonly %1) #1 {
  %3 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %1, i64 0, i32 0
  %4 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %1, i64 0, i32 6, i64 0
  %5 = getelementptr inbounds { i64, i64, i64 }, ptr %4, i64 0, i32 0
  %6 = getelementptr inbounds { i64, i64, i64 }, ptr %4, i64 0, i32 1
  %7 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %0, i64 0, i32 0
  %8 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %0, i64 0, i32 6, i64 0
  %9 = getelementptr inbounds { i64, i64, i64 }, ptr %8, i64 0, i32 1
  %10 = load ptr, ptr %3, align 8
  %11 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %6, i32 0)
  %12 = load i64, ptr %11, align 8
  %13 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %5, i32 0)
  %14 = load i64, ptr %13, align 8
  %15 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %6, i32 1)
  %16 = load i64, ptr %15, align 8
  %17 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %5, i32 1)
  %18 = load i64, ptr %17, align 8
  %19 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %6, i32 2)
  %20 = load i64, ptr %19, align 8
  %21 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %5, i32 2)
  %22 = load i64, ptr %21, align 8
  %23 = icmp slt i64 %22, 1
; NOTE: replace %22 with 9
; CHECK: %23 = icmp slt i64 9, 1
  %24 = icmp slt i64 %18, 1
; NOTE: replace %18 with 9
; CHECK: %24 = icmp slt i64 9, 1
  %25 = or i1 %23, %24
  %26 = icmp slt i64 %14, 1
; NOTE: replace %14 with 9
; CHECK: %26 = icmp slt i64 9, 1
  %27 = or i1 %25, %26
  br i1 %27, label %45, label %42

28:                                               ; preds = %36, %28
  %29 = phi i64 [ 1, %36 ], [ %31, %28 ]
  %30 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %12, ptr elementtype(i32) %38, i64 %29)
; NOTE: replace %12 with 4
; CHECK: %30 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4,{{.*}})
  store i32 0, ptr %30, align 4
  %31 = add nuw i64 %29, 1
  %32 = icmp eq i64 %29, %14
; NOTE: replace %14 with 9
; CHECK: %32 = icmp eq i64 %29, 9
  br i1 %32, label %33, label %28

33:                                               ; preds = %28
  %34 = add nuw i64 %37, 1
  %35 = icmp eq i64 %37, %18
; NOTE: replace %18 with 9
; CHECK: %35 = icmp eq i64 %37, 9
  br i1 %35, label %39, label %36

36:                                               ; preds = %42, %33
  %37 = phi i64 [ 1, %42 ], [ %34, %33 ]
  %38 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %16, ptr elementtype(i32) %44, i64 %37)
; NOTE: replace %16 with 36
; CHECK: %38 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36,{{.*}})
  br label %28

39:                                               ; preds = %33
  %40 = add nuw i64 %43, 1
  %41 = icmp eq i64 %43, %22
; NOTE: replace %22 with 9
; CHECK: %41 = icmp eq i64 %43, 9
  br i1 %41, label %45, label %42

42:                                               ; preds = %39, %2
  %43 = phi i64 [ %40, %39 ], [ 1, %2 ]
  %44 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %20, ptr elementtype(i32) %10, i64 %43)
; NOTE: replace %20 with 324
; CHECK: %44 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324,{{.*}})
  br label %36

45:                                               ; preds = %39, %2
  %46 = load ptr, ptr %7, align 8
  %47 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %9, i32 0)
  %48 = load i64, ptr %47, align 8
  %49 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %9, i32 1)
  %50 = load i64, ptr %49, align 8
  br label %51

51:                                               ; preds = %83, %45
  %52 = phi i64 [ 1, %45 ], [ %84, %83 ]
  br label %53

53:                                               ; preds = %80, %51
  %54 = phi i64 [ %81, %80 ], [ 1, %51 ]
  %55 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %50, ptr elementtype(i32) %46, i64 %54)
; NOTE: DO NOT replace %50 with 36
; CHECK: %55 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %50,{{.*}})
  %56 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %48, ptr elementtype(i32) %55, i64 %52)
; NOTE: replace %48 with 4
; CHECK: %56 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4,{{.*}})
  %57 = load i32, ptr %56, align 4
  %58 = icmp eq i32 %57, 0
  br i1 %58, label %76, label %80

59:                                               ; preds = %76, %59
  %60 = phi i64 [ 1, %76 ], [ %63, %59 ]
  %61 = phi i32 [ 1, %76 ], [ %64, %59 ]
  %62 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %79, i64 %60)
  store i32 %61, ptr %62, align 4
  %63 = add nuw nsw i64 %60, 1
  %64 = add nuw nsw i32 %61, 1
  %65 = icmp eq i64 %63, 10
  br i1 %65, label %66, label %59

66:                                               ; preds = %66, %59
  %67 = phi i64 [ %73, %66 ], [ 1, %59 ]
  %68 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %79, i64 %67)
  %69 = load i32, ptr %68, align 4
  %70 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %20, ptr elementtype(i32) %10, i64 %67)
; NOTE: replace %20 with 324
; CHECK: %70 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324,{{.*}})
  %71 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %16, ptr elementtype(i32) %70, i64 %54)
; NOTE: replace %16 with 36
; CHECK: %71 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36,{{.*}})
  %72 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %12, ptr elementtype(i32) %71, i64 %52)
  store i32 %69, ptr %72, align 4
  %73 = add nuw nsw i64 %67, 1
  %74 = icmp eq i64 %73, 10
  br i1 %74, label %75, label %66

75:                                               ; preds = %66
  tail call void @llvm.stackrestore(ptr %77)
  br label %80

76:                                               ; preds = %53
  %77 = tail call ptr @llvm.stacksave()
  %78 = alloca [9 x i32], align 4
  %79 = getelementptr inbounds [9 x i32], ptr %78, i64 0, i64 0
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

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #2

attributes #0 = { nocallback nofree nosync nounwind willreturn }
attributes #1 = { "intel-lang"="fortran" }
attributes #2 = { nounwind readnone speculatable }

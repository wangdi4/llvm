
; RUN: opt < %s -opaque-pointers -dope-vector-local-const-prop=false -S -passes=dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

; Check that full load, stride, and extent dope vector constant values are
; determined for ARG #0 and ARG #1 of new_solver_, but that they are not
; replaced because the base array of the per dimension arrays have been folded.

; This is the same test as dvcp03.ll, but checks the IR rather than the traces.

; Check the IR

@"main_$PART" = internal global [9 x [9 x i32]] zeroinitializer, align 16
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
  %5 = alloca { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, align 8
  %6 = alloca { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %7 = alloca [8 x i64], align 16
  %8 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %6, i64 0, i32 0
  %9 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %6, i64 0, i32 2
  %10 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %6, i64 0, i32 4
  %11 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %6, i64 0, i32 6, i64 0, i32 1
  %12 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %5, i64 0, i32 0
  %13 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %5, i64 0, i32 2
  %14 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %5, i64 0, i32 4
  %15 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %5, i64 0, i32 6, i64 0, i32 1
  %16 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.a87c7c812e60d4624ad0dfec6a834de1.0)
  br label %17

17:                                               ; preds = %27, %0
  %18 = phi i64 [ %28, %27 ], [ 1, %0 ]
  br label %19

19:                                               ; preds = %19, %17
  %20 = phi i64 [ %25, %19 ], [ 1, %17 ]
  %21 = sub nsw i64 %18, %20
  %22 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) @"main_$PART", i64 %20)
  %23 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %22, i64 %18)
  %24 = trunc i64 %21 to i32
  store i32 %24, ptr %23, align 4
  %25 = add nuw nsw i64 %20, 1
  %26 = icmp eq i64 %25, 10
  br i1 %26, label %27, label %19

27:                                               ; preds = %19
  %28 = add nuw nsw i64 %18, 1
  %29 = icmp eq i64 %28, 10
  br i1 %29, label %30, label %17

30:                                               ; preds = %27
  %31 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %6, i64 0, i32 1
  %32 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %6, i64 0, i32 3
  %33 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %6, i64 0, i32 6, i64 0, i32 0
  %34 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %6, i64 0, i32 6, i64 0, i32 2
  %35 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %5, i64 0, i32 1
  %36 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %5, i64 0, i32 3
  %37 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %5, i64 0, i32 6, i64 0, i32 0
  %38 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %5, i64 0, i32 6, i64 0, i32 2
  store i64 4, ptr %31, align 8
  store i64 2, ptr %10, align 8
  store i64 0, ptr %9, align 8
  %39 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %11, i32 0)
  store i64 4, ptr %39, align 8
  %40 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %34, i32 0)
  store i64 1, ptr %40, align 8
  %41 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %33, i32 0)
  store i64 9, ptr %41, align 8
  %42 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %11, i32 1)
  store i64 36, ptr %42, align 8
  %43 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %34, i32 1)
  store i64 1, ptr %43, align 8
  %44 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %33, i32 1)
  store i64 9, ptr %44, align 8
  store ptr @"main_$PART", ptr %8, align 8
  store i64 1, ptr %32, align 8
  store i64 4, ptr %35, align 8
  store i64 3, ptr %14, align 8
  store i64 0, ptr %13, align 8
  %45 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %15, i32 0)
  store i64 4, ptr %45, align 8
  %46 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %38, i32 0)
  store i64 1, ptr %46, align 8
  %47 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %37, i32 0)
  store i64 9, ptr %47, align 8
  %48 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %15, i32 1)
  store i64 36, ptr %48, align 8
  %49 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %38, i32 1)
  store i64 1, ptr %49, align 8
  %50 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %37, i32 1)
  store i64 9, ptr %50, align 8
  %51 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %15, i32 2)
  store i64 324, ptr %51, align 8
  %52 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %38, i32 2)
  store i64 1, ptr %52, align 8
  %53 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %37, i32 2)
  store i64 9, ptr %53, align 8
  store ptr @"main_$BLOCK", ptr %12, align 8
  store i64 1, ptr %36, align 8
  call void @new_solver_(ptr nonnull %6, ptr nonnull %5)
  ret void
}

define internal void @new_solver_(ptr noalias nocapture readonly %0, ptr noalias nocapture readonly %1) #1 {
  %3 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %1, i64 0, i32 0
  %4 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %1, i64 0, i32 6, i64 0, i32 0
  %5 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %1, i64 0, i32 6, i64 0, i32 1
  %6 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %0, i64 0, i32 0
  %7 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %0, i64 0, i32 6, i64 0, i32 1
  %8 = load ptr, ptr %3, align 8
  %9 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %5, i32 0)
  %10 = load i64, ptr %9, align 8
  %11 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %4, i32 0)
  %12 = load i64, ptr %11, align 8
  %13 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %5, i32 1)
  %14 = load i64, ptr %13, align 8
  %15 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %4, i32 1)
  %16 = load i64, ptr %15, align 8
  %17 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %5, i32 2)
  %18 = load i64, ptr %17, align 8
  %19 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %4, i32 2)
  %20 = load i64, ptr %19, align 8
  %21 = icmp slt i64 %20, 1
; CHECK: %21 = icmp slt i64 %20, 1
  %22 = icmp slt i64 %16, 1
; CHECK: %22 = icmp slt i64 %16, 1
  %23 = or i1 %21, %22
  %24 = icmp slt i64 %12, 1
; CHECK: %24 = icmp slt i64 %12, 1
  %25 = or i1 %23, %24
  br i1 %25, label %43, label %40

26:                                               ; preds = %34, %26
  %27 = phi i64 [ 1, %34 ], [ %29, %26 ]
  %28 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %10, ptr elementtype(i32) %36, i64 %27)
; CHECK: %28 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %10, ptr elementtype(i32) %36, i64 %27)
  store i32 0, ptr %28, align 4
  %29 = add nuw i64 %27, 1
  %30 = icmp eq i64 %27, %12
; CHECK: %30 = icmp eq i64 %27, %12
  br i1 %30, label %31, label %26

31:                                               ; preds = %26
  %32 = add nuw i64 %35, 1
  %33 = icmp eq i64 %35, %16
; CHECK: %33 = icmp eq i64 %35, %16
  br i1 %33, label %37, label %34

34:                                               ; preds = %40, %31
  %35 = phi i64 [ 1, %40 ], [ %32, %31 ]
  %36 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %14, ptr elementtype(i32) %42, i64 %35)
; CHECK: %36 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %14, ptr elementtype(i32) %42, i64 %35)
  br label %26

37:                                               ; preds = %31
  %38 = add nuw i64 %41, 1
  %39 = icmp eq i64 %41, %20
; CHECK: %39 = icmp eq i64 %41, %20
  br i1 %39, label %43, label %40

40:                                               ; preds = %37, %2
  %41 = phi i64 [ %38, %37 ], [ 1, %2 ]
  %42 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %18, ptr elementtype(i32) %8, i64 %41)
; CHECK: %42 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %18, ptr elementtype(i32) %8, i64 %41)
  br label %34

43:                                               ; preds = %37, %2
  %44 = load ptr, ptr %6, align 8
  %45 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %7, i32 0)
  %46 = load i64, ptr %45, align 8
  %47 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %7, i32 1)
  %48 = load i64, ptr %47, align 8
  br label %49

49:                                               ; preds = %81, %43
  %50 = phi i64 [ 1, %43 ], [ %82, %81 ]
  br label %51

51:                                               ; preds = %78, %49
  %52 = phi i64 [ %79, %78 ], [ 1, %49 ]
  %53 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %48, ptr elementtype(i32) %44, i64 %52)
; CHECK: %53 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %48, ptr elementtype(i32) %44, i64 %52)
  %54 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %46, ptr elementtype(i32) %53, i64 %50)
; CHECK: %54 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %46, ptr elementtype(i32) %53, i64 %50)
  %55 = load i32, ptr %54, align 4
  %56 = icmp eq i32 %55, 0
  br i1 %56, label %74, label %78

57:                                               ; preds = %74, %57
  %58 = phi i64 [ 1, %74 ], [ %61, %57 ]
  %59 = phi i32 [ 1, %74 ], [ %62, %57 ]
  %60 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %77, i64 %58)
  store i32 %59, ptr %60, align 4
  %61 = add nuw nsw i64 %58, 1
  %62 = add nuw nsw i32 %59, 1
  %63 = icmp eq i64 %61, 10
  br i1 %63, label %64, label %57

64:                                               ; preds = %64, %57
  %65 = phi i64 [ %71, %64 ], [ 1, %57 ]
  %66 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %77, i64 %65)
  %67 = load i32, ptr %66, align 4
  %68 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %18, ptr elementtype(i32) %8, i64 %65)
; CHECK: %68 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %18, ptr elementtype(i32) %8, i64 %65)
  %69 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %14, ptr elementtype(i32) %68, i64 %52)
; CHECK: %69 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %14, ptr elementtype(i32) %68, i64 %52)
  %70 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %10, ptr elementtype(i32) %69, i64 %50)
; CHECK: %70 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %10, ptr elementtype(i32) %69, i64 %50)
  store i32 %67, ptr %70, align 4
  %71 = add nuw nsw i64 %65, 1
  %72 = icmp eq i64 %71, 10
  br i1 %72, label %73, label %64

73:                                               ; preds = %64
  tail call void @llvm.stackrestore(ptr %75)
  br label %78

74:                                               ; preds = %51
  %75 = tail call ptr @llvm.stacksave()
  %76 = alloca [9 x i32], align 4
  %77 = getelementptr inbounds [9 x i32], ptr %76, i64 0, i64 0
  br label %57

78:                                               ; preds = %73, %51
  %79 = add nuw nsw i64 %52, 1
  %80 = icmp eq i64 %79, 10
  br i1 %80, label %81, label %51

81:                                               ; preds = %78
  %82 = add nuw nsw i64 %50, 1
  %83 = icmp eq i64 %82, 10
  br i1 %83, label %84, label %49

84:                                               ; preds = %81
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #2

attributes #0 = { nocallback nofree nosync nounwind willreturn }
attributes #1 = { "intel-lang"="fortran" }
attributes #2 = { nounwind readnone speculatable }

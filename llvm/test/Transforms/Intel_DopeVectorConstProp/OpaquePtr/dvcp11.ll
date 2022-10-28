; RUN: opt < %s -dope-vector-local-const-prop=false -S -passes=dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

; Check that full load, stride, and extent dope vector constant values are
; determined for ARG #0 and ARG #1 of new_solver_.

; This is the same test as dvcp01.ll, but checks the IR rather than the traces.

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
  %8 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %6, i64 0, i32 1
  %9 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %6, i64 0, i32 3
  %10 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %6, i64 0, i32 6, i64 0
  %11 = getelementptr inbounds { i64, i64, i64 }, ptr %10, i64 0, i32 0
  %12 = getelementptr inbounds { i64, i64, i64 }, ptr %10, i64 0, i32 2
  %13 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %5, i64 0, i32 1
  %14 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %5, i64 0, i32 3
  %15 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %5, i64 0, i32 6, i64 0
  %16 = getelementptr inbounds { i64, i64, i64 }, ptr %15, i64 0, i32 1
  %17 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.a87c7c812e60d4624ad0dfec6a834de1.0)
  br label %18

18:                                               ; preds = %28, %0
  %19 = phi i64 [ %29, %28 ], [ 1, %0 ]
  br label %20

20:                                               ; preds = %20, %18
  %21 = phi i64 [ %26, %20 ], [ 1, %18 ]
  %22 = sub nsw i64 %19, %21
  %23 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) @"main_$PART", i64 %21)
  %24 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %23, i64 %19)
  %25 = trunc i64 %22 to i32
  store i32 %25, ptr %24, align 4
  %26 = add nuw nsw i64 %21, 1
  %27 = icmp eq i64 %26, 10
  br i1 %27, label %28, label %20

28:                                               ; preds = %20
  %29 = add nuw nsw i64 %19, 1
  %30 = icmp eq i64 %29, 10
  br i1 %30, label %31, label %18

31:                                               ; preds = %28
  %32 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %6, i64 0, i32 0
  %33 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %6, i64 0, i32 2
  %34 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %6, i64 0, i32 4
  %35 = getelementptr inbounds { i64, i64, i64 }, ptr %10, i64 0, i32 1
  %36 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %5, i64 0, i32 0
  %37 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %5, i64 0, i32 2
  %38 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %5, i64 0, i32 4
  %39 = getelementptr inbounds { i64, i64, i64 }, ptr %15, i64 0, i32 0
  %40 = getelementptr inbounds { i64, i64, i64 }, ptr %15, i64 0, i32 2
  store i64 4, ptr %8, align 8
  store i64 2, ptr %34, align 8
  store i64 0, ptr %33, align 8
  %41 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %35, i32 0)
  store i64 4, ptr %41, align 8
  %42 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %12, i32 0)
  store i64 1, ptr %42, align 8
  %43 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %11, i32 0)
  store i64 9, ptr %43, align 8
  %44 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %35, i32 1)
  store i64 36, ptr %44, align 8
  %45 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %12, i32 1)
  store i64 1, ptr %45, align 8
  %46 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %11, i32 1)
  store i64 9, ptr %46, align 8
  store ptr @"main_$PART", ptr %32, align 8
  store i64 1, ptr %9, align 8
  store i64 4, ptr %13, align 8
  store i64 3, ptr %38, align 8
  store i64 0, ptr %37, align 8
  %47 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %16, i32 0)
  store i64 4, ptr %47, align 8
  %48 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %40, i32 0)
  store i64 1, ptr %48, align 8
  %49 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %39, i32 0)
  store i64 9, ptr %49, align 8
  %50 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %16, i32 1)
  store i64 36, ptr %50, align 8
  %51 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %40, i32 1)
  store i64 1, ptr %51, align 8
  %52 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %39, i32 1)
  store i64 9, ptr %52, align 8
  %53 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %16, i32 2)
  store i64 324, ptr %53, align 8
  %54 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %40, i32 2)
  store i64 1, ptr %54, align 8
  %55 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %39, i32 2)
  store i64 9, ptr %55, align 8
  store ptr @"main_$BLOCK", ptr %36, align 8
  store i64 1, ptr %14, align 8
  call void @new_solver_(ptr nonnull %6, ptr nonnull %5)
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
; NOTE: replace %50 with 36
; CHECK: %55 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36,{{.*}})
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

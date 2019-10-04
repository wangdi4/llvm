; REQUIRES: asserts
; RUN: opt < %s -ip-cloning -debug-only=ipcloning -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(ip-cloning)' -debug-only=ipcloning -S 2>&1 | FileCheck %s

; Test that the function main_IP_digits_2_ is recognized as a recursive progression clone
; and eight clones of it are created. Also test that the recursive progression
; is not cyclic.

; CHECK: Cloning Analysis for:  main_IP_digits_2_
; CHECK: Selected RecProgression cloning
; CHECK: Function: main_IP_digits_2_.1
; CHECK: ArgPos : 0
; CHECK: Argument : i32* %0
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 1
; CHECK: Function: main_IP_digits_2_.2
; CHECK: ArgPos : 0
; CHECK: Argument : i32* %0
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 2
; CHECK: Function: main_IP_digits_2_.3
; CHECK: ArgPos : 0
; CHECK: Argument : i32* %0
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 3
; CHECK: Function: main_IP_digits_2_.4
; CHECK: ArgPos : 0
; CHECK: Argument : i32* %0
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 4
; CHECK: Function: main_IP_digits_2_.5
; CHECK: ArgPos : 0
; CHECK: Argument : i32* %0
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 5
; CHECK: Function: main_IP_digits_2_.6
; CHECK: ArgPos : 0
; CHECK: Argument : i32* %0
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 6
; CHECK: Function: main_IP_digits_2_.7
; CHECK: ArgPos : 0
; CHECK: Argument : i32* %0
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 7
; CHECK: Function: main_IP_digits_2_.8
; CHECK: ArgPos : 0
; CHECK: Argument : i32* %0
; CHECK: IsByRef : T
; CHECK: Replacement:  i32 8

; CHECK: define dso_local void @MAIN__
; CHECK: define internal void @main_IP_digits_2_
; CHECK: call void @main_IP_digits_2_
; CHECK: define internal void @main_IP_digits_2_.1
; CHECK: call void @main_IP_digits_2_.2
; CHECK: define internal void @main_IP_digits_2_.2
; CHECK: call void @main_IP_digits_2_.3
; CHECK: define internal void @main_IP_digits_2_.3
; CHECK: call void @main_IP_digits_2_.4
; CHECK: define internal void @main_IP_digits_2_.4
; CHECK: call void @main_IP_digits_2_.5
; CHECK: define internal void @main_IP_digits_2_.5
; CHECK: call void @main_IP_digits_2_.6
; CHECK: define internal void @main_IP_digits_2_.6
; CHECK: call void @main_IP_digits_2_.7
; CHECK: define internal void @main_IP_digits_2_.7
; CHECK: call void @main_IP_digits_2_.8
; CHECK: define internal void @main_IP_digits_2_.8
; CHECK-NOT: call void @main_IP_digits_2_

@brute_force_mp_block_ = internal global [9 x [9 x [9 x i32]]] zeroinitializer, align 8
@brute_force_mp_count_ = internal global i32 0, align 8
@anon.2e0c92f0a1367b9abe0b93b08f29c950.0 = internal unnamed_addr constant [9 x i8] c" block = "

define dso_local void @MAIN__() #0 {
  %1 = alloca { i32 }, align 8
  %2 = alloca [4 x i8], align 1
  %3 = alloca { i64, i8* }, align 8
  %4 = alloca [4 x i8], align 1
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca [8 x i64], align 16
  store i32 2, i32* %6, align 4
  %8 = call i32 @for_set_reentrancy(i32* nonnull %6)
  br label %9

; <label>:9:                                      ; preds = %23, %0
  %10 = phi i64 [ 1, %0 ], [ %24, %23 ]
  %11 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 324, i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %10)
  br label %12

; <label>:12:                                     ; preds = %20, %9
  %13 = phi i64 [ 1, %9 ], [ %21, %20 ]
  %14 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %11, i64 %13)
  br label %15

; <label>:15:                                     ; preds = %15, %12
  %16 = phi i64 [ 1, %12 ], [ %18, %15 ]
  %17 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %14, i64 %16)
  store i32 2, i32* %17, align 4
  %18 = add nuw nsw i64 %16, 1
  %19 = icmp eq i64 %18, 10
  br i1 %19, label %20, label %15

; <label>:20:                                     ; preds = %15
  %21 = add nuw nsw i64 %13, 1
  %22 = icmp eq i64 %21, 10
  br i1 %22, label %23, label %12

; <label>:23:                                     ; preds = %20
  %24 = add nuw nsw i64 %10, 1
  %25 = icmp eq i64 %24, 10
  br i1 %25, label %26, label %9

; <label>:26:                                     ; preds = %23
  store i32 1, i32* @brute_force_mp_count_, align 8
  store i32 1, i32* %5, align 4
  call void @main_IP_digits_2_(i32* nonnull %5)
  %27 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 0
  store i8 56, i8* %27, align 1
  %28 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 1
  store i8 4, i8* %28, align 1
  %29 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 2
  store i8 2, i8* %29, align 1
  %30 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 3
  store i8 0, i8* %30, align 1
  %31 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %3, i64 0, i32 0
  store i64 9, i64* %31, align 8
  %32 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %3, i64 0, i32 1
  store i8* getelementptr inbounds ([9 x i8], [9 x i8]* @anon.2e0c92f0a1367b9abe0b93b08f29c950.0, i64 0, i64 0), i8** %32, align 8
  %33 = bitcast [8 x i64]* %7 to i8*
  %34 = bitcast { i64, i8* }* %3 to i8*
  %35 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %33, i32 -1, i64 1239157112576, i8* nonnull %27, i8* nonnull %34)
  br label %36

; <label>:36:                                     ; preds = %55, %26
  %37 = phi i64 [ 1, %26 ], [ %56, %55 ]
  %38 = phi i32 [ 0, %26 ], [ %49, %55 ]
  %39 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 324, i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %37)
  br label %40

; <label>:40:                                     ; preds = %52, %36
  %41 = phi i64 [ 1, %36 ], [ %53, %52 ]
  %42 = phi i32 [ %38, %36 ], [ %49, %52 ]
  %43 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %39, i64 %41)
  br label %44

; <label>:44:                                     ; preds = %44, %40
  %45 = phi i64 [ 1, %40 ], [ %50, %44 ]
  %46 = phi i32 [ %42, %40 ], [ %49, %44 ]
  %47 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %43, i64 %45)
  %48 = load i32, i32* %47, align 4
  %49 = add nsw i32 %48, %46
  %50 = add nuw nsw i64 %45, 1
  %51 = icmp eq i64 %50, 10
  br i1 %51, label %52, label %44

; <label>:52:                                     ; preds = %44
  %53 = add nuw nsw i64 %41, 1
  %54 = icmp eq i64 %53, 10
  br i1 %54, label %55, label %40

; <label>:55:                                     ; preds = %52
  %56 = add nuw nsw i64 %37, 1
  %57 = icmp eq i64 %56, 10
  br i1 %57, label %58, label %36

; <label>:58:                                     ; preds = %55
  %59 = getelementptr inbounds [4 x i8], [4 x i8]* %2, i64 0, i64 0
  store i8 9, i8* %59, align 1
  %60 = getelementptr inbounds [4 x i8], [4 x i8]* %2, i64 0, i64 1
  store i8 1, i8* %60, align 1
  %61 = getelementptr inbounds [4 x i8], [4 x i8]* %2, i64 0, i64 2
  store i8 1, i8* %61, align 1
  %62 = getelementptr inbounds [4 x i8], [4 x i8]* %2, i64 0, i64 3
  store i8 0, i8* %62, align 1
  %63 = getelementptr inbounds { i32 }, { i32 }* %1, i64 0, i32 0
  store i32 %49, i32* %63, align 8
  %64 = bitcast { i32 }* %1 to i8*
  %65 = call i32 @for_write_seq_lis_xmit(i8* nonnull %33, i8* nonnull %59, i8* nonnull %64)
  ret void
}

declare dso_local i32 @for_set_reentrancy(i32*) local_unnamed_addr

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #1

; Function Attrs: nounwind
define internal void @main_IP_digits_2_(i32* noalias nocapture readonly) #2 {
  %2 = alloca i32, align 4
  %3 = load i32, i32* %0, align 4
  %4 = sext i32 %3 to i64
  %5 = icmp eq i32 %3, 8
  %6 = add nsw i32 %3, 1
  br label %7

; <label>:7:                                      ; preds = %108, %1
  %8 = phi i64 [ %109, %108 ], [ 1, %1 ]
  %9 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 324, i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %8)
  %10 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %9, i64 1)
  %11 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %10, i64 %4)
  %12 = load i32, i32* %11, align 4
  %13 = icmp slt i32 %12, 1
  br i1 %13, label %108, label %14

; <label>:14:                                     ; preds = %7
  %15 = trunc i64 %8 to i32
  store i32 %15, i32* %11, align 4
  br label %16

; <label>:16:                                     ; preds = %105, %14
  %17 = phi i64 [ %106, %105 ], [ 1, %14 ]
  %18 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 324, i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %17)
  %19 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %18, i64 2)
  %20 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %19, i64 %4)
  %21 = load i32, i32* %20, align 4
  %22 = icmp slt i32 %21, 1
  br i1 %22, label %105, label %23

; <label>:23:                                     ; preds = %102, %16
  %24 = phi i64 [ %103, %102 ], [ 1, %16 ]
  %25 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 324, i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %24)
  %26 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %25, i64 3)
  %27 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %26, i64 %4)
  %28 = load i32, i32* %27, align 4
  %29 = icmp slt i32 %28, 0
  br i1 %29, label %102, label %30

; <label>:30:                                     ; preds = %99, %23
  %31 = phi i64 [ %100, %99 ], [ 1, %23 ]
  %32 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 324, i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %31)
  %33 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %32, i64 4)
  %34 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %33, i64 %4)
  %35 = load i32, i32* %34, align 4
  %36 = icmp slt i32 %35, 0
  br i1 %36, label %99, label %37

; <label>:37:                                     ; preds = %96, %30
  %38 = phi i64 [ %97, %96 ], [ 1, %30 ]
  %39 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 324, i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %38)
  %40 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %39, i64 5)
  %41 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %40, i64 %4)
  %42 = load i32, i32* %41, align 4
  %43 = icmp slt i32 %42, 0
  br i1 %43, label %96, label %44

; <label>:44:                                     ; preds = %93, %37
  %45 = phi i64 [ %94, %93 ], [ 1, %37 ]
  %46 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 324, i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %45)
  %47 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %46, i64 6)
  %48 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %47, i64 %4)
  %49 = load i32, i32* %48, align 4
  %50 = icmp slt i32 %49, 1
  br i1 %50, label %93, label %51

; <label>:51:                                     ; preds = %44
  %52 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %46, i64 7)
  %53 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %52, i64 %4)
  %54 = trunc i64 %45 to i32
  store i32 %54, i32* %53, align 4
  br label %55

; <label>:55:                                     ; preds = %90, %51
  %56 = phi i64 [ %91, %90 ], [ 1, %51 ]
  %57 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 324, i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %56)
  %58 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %57, i64 7)
  %59 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %58, i64 %4)
  %60 = load i32, i32* %59, align 4
  %61 = icmp slt i32 %60, 1
  br i1 %61, label %90, label %62

; <label>:62:                                     ; preds = %55
  %63 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %57, i64 1)
  %64 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %63, i64 %4)
  %65 = trunc i64 %56 to i32
  store i32 %65, i32* %64, align 4
  br label %66

; <label>:66:                                     ; preds = %87, %62
  %67 = phi i64 [ %88, %87 ], [ 1, %62 ]
  %68 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 324, i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %67)
  %69 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %68, i64 8)
  %70 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %69, i64 %4)
  %71 = load i32, i32* %70, align 4
  %72 = icmp slt i32 %71, 1
  br i1 %72, label %87, label %73

; <label>:73:                                     ; preds = %66
  %74 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %68, i64 9)
  %75 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %74, i64 %4)
  %76 = trunc i64 %67 to i32
  store i32 %76, i32* %75, align 4
  %77 = load i32, i32* @brute_force_mp_count_, align 8
  %78 = add nsw i32 %77, 1
  store i32 %78, i32* @brute_force_mp_count_, align 8
  br i1 %5, label %79, label %81

; <label>:79:                                     ; preds = %73
  %80 = add nsw i32 %77, 2
  store i32 %80, i32* @brute_force_mp_count_, align 8
  br label %84

; <label>:81:                                     ; preds = %73
  %82 = icmp slt i32 %78, 500000
  br i1 %82, label %83, label %84

; <label>:83:                                     ; preds = %81
  store i32 %6, i32* %2, align 4
  call void @main_IP_digits_2_(i32* nonnull %2)
  br label %84

; <label>:84:                                     ; preds = %83, %81, %79
  %85 = load i32, i32* %75, align 4
  %86 = add nsw i32 %85, 10
  store i32 %86, i32* %75, align 4
  br label %87

; <label>:87:                                     ; preds = %84, %66
  %88 = add nuw nsw i64 %67, 1
  %89 = icmp eq i64 %88, 3
  br i1 %89, label %90, label %66

; <label>:90:                                     ; preds = %87, %55
  %91 = add nuw nsw i64 %56, 1
  %92 = icmp eq i64 %91, 4
  br i1 %92, label %93, label %55

; <label>:93:                                     ; preds = %90, %44
  %94 = add nuw nsw i64 %45, 1
  %95 = icmp eq i64 %94, 5
  br i1 %95, label %96, label %44

; <label>:96:                                     ; preds = %93, %37
  %97 = add nuw nsw i64 %38, 1
  %98 = icmp eq i64 %97, 4
  br i1 %98, label %99, label %37

; <label>:99:                                     ; preds = %96, %30
  %100 = add nuw nsw i64 %31, 1
  %101 = icmp eq i64 %100, 3
  br i1 %101, label %102, label %30

; <label>:102:                                    ; preds = %99, %23
  %103 = add nuw nsw i64 %24, 1
  %104 = icmp eq i64 %103, 7
  br i1 %104, label %105, label %23

; <label>:105:                                    ; preds = %102, %16
  %106 = add nuw nsw i64 %17, 1
  %107 = icmp eq i64 %106, 3
  br i1 %107, label %108, label %16

; <label>:108:                                    ; preds = %105, %7
  %109 = add nuw nsw i64 %8, 1
  %110 = icmp eq i64 %109, 4
  br i1 %110, label %111, label %7

; <label>:111:                                    ; preds = %108
  ret void
}

declare dso_local i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr

declare dso_local i32 @for_write_seq_lis_xmit(i8*, i8*, i8*) local_unnamed_addr

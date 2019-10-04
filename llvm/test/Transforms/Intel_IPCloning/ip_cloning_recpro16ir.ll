; RUN: opt < %s -ip-cloning -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(ip-cloning)' -S 2>&1 | FileCheck %s

; Test that the function @good is recognized as a recursive progression clone
; and eight clones of it are created. Also that it is a candidate for creating
; an extra clone, and that AVX512 has NOT been transformed to AVX2, because
; a function has a non skylake-avx512 target-cpu.
; This is the same test as ip_cloning_recpro16.ll, but checks for IR without
; requiring asserts.

; Check for sequence of eight clones with transformed attributes:

; CHECK: define dso_local void @MAIN__()
; CHECK: call void @good.1
; CHECK: define internal void @good{{.*}}#1
; CHECK: call void @good
; CHECK: define internal void @good.1{{.*}}#2
; CHECK: call void @good.2
; CHECK: define internal void @good.2{{.*}}#2
; CHECK: call void @good.3
; CHECK: define internal void @good.3{{.*}}#2
; CHECK: call void @good.4
; CHECK: define internal void @good.4{{.*}}#2
; CHECK: call void @good.5
; CHECK: define internal void @good.5{{.*}}#2
; CHECK: call void @good.6
; CHECK: define internal void @good.6{{.*}}#2
; CHECK: call void @good.7
; CHECK: define internal void @good.7{{.*}}#2
; CHECK: call void @good.8
; CHECK: define internal void @good.8{{.*}}#2
; CHECK-NOT: call void @good

; Check for special inserted test, call to extra clone, and constant loop
; bound assignments

; CHECK: %8 = alloca [9 x i32], align 16
; CHECK: %9 = alloca [9 x i32], align 16
; CHECK: %32 = getelementptr inbounds [9 x i32], [9 x i32]* %8, i64 0, i64 0
; CHECK: %44 = getelementptr inbounds [9 x i32], [9 x i32]* %9, i64 0, i64 0
; CHECK: CondBlock:
; CHECK: [[V1:%[0-9]+]] = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %32, i64 8)
; CHECK: %LILB8 = load i32, i32* [[V1]]
; CHECK: [[V2:%[0-9]+]] = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %44, i64 8)
; CHECK: %LIUB8 = load i32, i32* [[V2]]
; CHECK: %CMP8S = icmp eq i32 %LILB8, %LIUB8
; CHECK: br i1 %CMP8S, label %CallCloneBlock, label %ConstStore
; CHECK: CallCloneBlock:
; CHECK: call void @good.8.9(i32* %0)
; CHECK: ret void
; CHECK: ConstStore:
; CHECK: store i32 1, i32* [[V1]]
; CHECK: store i32 9, i32* [[V2]]

; Check for extra clone
; CHECK: define internal void @good.8.9
; CHECK-NOT: call void @good

; Check the attributes
; CHECK: attributes #0 = { nounwind readnone speculatable }
; CHECK: attributes #1 = { nounwind "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="cannonlake" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
; CHECK: attributes #2 = { nounwind "contains-rec-pro-clone" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "prefer-inline-rec-pro-clone" "target-cpu"="cannonlake" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64)
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32)
@brute_force_mp_sudoku1_ = common dso_local global [9 x [9 x i32]] zeroinitializer, align 8
@brute_force_mp_sudoku2_ = common dso_local global [9 x [9 x i32]] zeroinitializer, align 8
@brute_force_mp_sudoku3_ = common dso_local global [9 x [9 x i32]] zeroinitializer, align 8
@brute_force_mp_block_ = common dso_local global [9 x [9 x [9 x i32]]] zeroinitializer, align 8
@brute_force_mp_soln_ = common dso_local global i32 0, align 8
declare i32 @brute_force_mp_covered_({ i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* noalias nocapture readonly %"logic_$sudoku_", { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* noalias nocapture readonly %"logic_$pattern_")

define dso_local void @MAIN__() {
  %1 = alloca i32, align 4
  store i32 1, i32* %1, align 4
  call void @good(i32* nonnull %1)
  ret void
}

@myc = internal unnamed_addr constant i32 20

define internal void @good(i32* noalias nocapture readonly) #0 {
  %2 = alloca i32, align 4
  %3 = alloca { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %4 = alloca { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %5 = alloca [9 x i32], align 16
  %6 = alloca [9 x i32], align 16
  %7 = alloca [9 x i32], align 16
  %8 = alloca [9 x i32], align 16
  %9 = alloca [9 x i32], align 16
  %10 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %4, i64 0, i32 1
  %11 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %4, i64 0, i32 3
  %12 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %4, i64 0, i32 6, i64 0
  %13 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %12, i64 0, i32 0
  %14 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %12, i64 0, i32 2
  %15 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %3, i64 0, i32 1
  %16 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %3, i64 0, i32 3
  %17 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %3, i64 0, i32 6, i64 0
  %18 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %17, i64 0, i32 1
  %19 = load i32, i32* %0, align 4
  %20 = sext i32 %19 to i64
  %21 = getelementptr inbounds [9 x i32], [9 x i32]* %7, i64 0, i64 0
  br label %33

22:                                               ; preds = %33
  %23 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %4, i64 0, i32 0
  %24 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %4, i64 0, i32 2
  %25 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %4, i64 0, i32 4
  %26 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %12, i64 0, i32 1
  %27 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %3, i64 0, i32 0
  %28 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %3, i64 0, i32 2
  %29 = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %3, i64 0, i32 4
  %30 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %17, i64 0, i32 0
  %31 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %17, i64 0, i32 2
  %32 = getelementptr inbounds [9 x i32], [9 x i32]* %8, i64 0, i64 0
  br label %45

33:                                               ; preds = %33, %1
  %34 = phi i64 [ 1, %1 ], [ %41, %33 ]
  %35 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @brute_force_mp_sudoku1_, i64 0, i64 0, i64 0), i64 %34)
  %36 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %35, i64 %20)
  %37 = load i32, i32* %36, align 4
  %38 = icmp ne i32 %37, 0
  %39 = sext i1 %38 to i32
  %40 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %21, i64 %34)
  store i32 %39, i32* %40, align 4
  %41 = add nuw nsw i64 %34, 1
  %42 = icmp eq i64 %41, 10
  br i1 %42, label %22, label %33

43:                                               ; preds = %56
  %44 = getelementptr inbounds [9 x i32], [9 x i32]* %9, i64 0, i64 0
  br label %59

45:                                               ; preds = %56, %22
  %46 = phi i64 [ 1, %22 ], [ %57, %56 ]
  %47 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %21, i64 %46)
  %48 = load i32, i32* %47, align 4
  %49 = and i32 %48, 1
  %50 = icmp eq i32 %49, 0
  br i1 %50, label %56, label %51

51:                                               ; preds = %45
  %52 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @brute_force_mp_sudoku1_, i64 0, i64 0, i64 0), i64 %46)
  %53 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %52, i64 %20)
  %54 = load i32, i32* %53, align 4
  %55 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %32, i64 %46)
  store i32 %54, i32* %55, align 4
  br label %56

56:                                               ; preds = %51, %45
  %57 = add nuw nsw i64 %46, 1
  %58 = icmp eq i64 %57, 10
  br i1 %58, label %43, label %45

59:                                               ; preds = %69, %43
  %60 = phi i64 [ 1, %43 ], [ %70, %69 ]
  %61 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %21, i64 %60)
  %62 = load i32, i32* %61, align 4
  %63 = and i32 %62, 1
  %64 = icmp eq i32 %63, 0
  br i1 %64, label %69, label %65

65:                                               ; preds = %59
  %66 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %32, i64 %60)
  %67 = load i32, i32* %66, align 4
  %68 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %44, i64 %60)
  store i32 %67, i32* %68, align 4
  br label %69

69:                                               ; preds = %65, %59
  %70 = add nuw nsw i64 %60, 1
  %71 = icmp eq i64 %70, 10
  br i1 %71, label %72, label %59

72:                                               ; preds = %80, %69
  %73 = phi i64 [ %81, %80 ], [ 1, %69 ]
  %74 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %21, i64 %73)
  %75 = load i32, i32* %74, align 4
  %76 = and i32 %75, 1
  %77 = icmp eq i32 %76, 0
  br i1 %77, label %78, label %80

78:                                               ; preds = %72
  %79 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %32, i64 %73)
  store i32 1, i32* %79, align 4
  br label %80

80:                                               ; preds = %78, %72
  %81 = add nuw nsw i64 %73, 1
  %82 = icmp eq i64 %81, 10
  br i1 %82, label %83, label %72

83:                                               ; preds = %91, %80
  %84 = phi i64 [ %92, %91 ], [ 1, %80 ]
  %85 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %21, i64 %84)
  %86 = load i32, i32* %85, align 4
  %87 = and i32 %86, 1
  %88 = icmp eq i32 %87, 0
  br i1 %88, label %89, label %91

89:                                               ; preds = %83
  %90 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %44, i64 %84)
  store i32 9, i32* %90, align 4
  br label %91

91:                                               ; preds = %89, %83
  %92 = add nuw nsw i64 %84, 1
  %93 = icmp eq i64 %92, 10
  br i1 %93, label %94, label %83

94:                                               ; preds = %91
  %95 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %32, i64 1)
  %96 = load i32, i32* %95, align 4
  %97 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %44, i64 1)
  %98 = load i32, i32* %97, align 4
  %99 = icmp slt i32 %98, %96
  br i1 %99, label %1273, label %100

100:                                              ; preds = %94
  %101 = add i32 %19, 3
  %102 = add nsw i32 %19, -1
  %103 = srem i32 %102, 3
  %104 = sub i32 %101, %103
  %105 = sext i32 %104 to i64
  %106 = sub nsw i64 10, %105
  %107 = icmp sgt i32 %104, 9
  %108 = srem i32 %19, 3
  %109 = sext i32 %108 to i64
  %110 = add nsw i32 %19, 1
  %111 = sext i32 %110 to i64
  %112 = add nsw i32 %19, 2
  %113 = sext i32 %112 to i64
  %114 = sub nsw i64 1, %111
  %115 = add nsw i64 %114, %113
  %116 = icmp slt i64 %115, 1
  %117 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 1)
  %118 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %117, i64 %20)
  %119 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %32, i64 2)
  %120 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %44, i64 2)
  %121 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 2)
  %122 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %32, i64 3)
  %123 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %44, i64 3)
  %124 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 3)
  %125 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %32, i64 4)
  %126 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %44, i64 4)
  %127 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 4)
  %128 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %32, i64 5)
  %129 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %44, i64 5)
  %130 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 5)
  %131 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %32, i64 6)
  %132 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %44, i64 6)
  %133 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 6)
  %134 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %32, i64 7)
  %135 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %44, i64 7)
  %136 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 7)
  %137 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %32, i64 8)
  %138 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %44, i64 8)
  %139 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 8)
  %140 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 9)
  %141 = getelementptr inbounds [9 x i32], [9 x i32]* %6, i64 0, i64 0
  %142 = getelementptr inbounds [9 x i32], [9 x i32]* %5, i64 0, i64 0
  %143 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %26, i32 0)
  %144 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %14, i32 0)
  %145 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %13, i32 0)
  %146 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %26, i32 1)
  %147 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %14, i32 1)
  %148 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %13, i32 1)
  %149 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %18, i32 0)
  %150 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %31, i32 0)
  %151 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %30, i32 0)
  %152 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %18, i32 1)
  %153 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %31, i32 1)
  %154 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %30, i32 1)
  %155 = add nsw i64 %113, 2
  %156 = sub nsw i64 %155, %111
  %157 = sext i32 %96 to i64
  %158 = sext i32 %98 to i64
  %159 = sext i32 %19 to i64
  %160 = srem i32 %19, 3
  %161 = sext i32 %160 to i64
  %162 = add nsw i32 %19, 1
  %163 = sext i32 %162 to i64
  %164 = add nsw i32 %19, 2
  %165 = sext i32 %164 to i64
  %166 = sub nsw i64 1, %163
  %167 = add nsw i64 %166, %165
  %168 = icmp slt i64 %167, 1
  %169 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %121, i64 %159)
  %170 = add nsw i64 %165, 2
  %171 = sub nsw i64 %170, %163
  %172 = sext i32 %19 to i64
  %173 = srem i32 %19, 3
  %174 = sext i32 %173 to i64
  %175 = add nsw i32 %19, 1
  %176 = sext i32 %175 to i64
  %177 = add nsw i32 %19, 2
  %178 = sext i32 %177 to i64
  %179 = sub nsw i64 1, %176
  %180 = add nsw i64 %179, %178
  %181 = icmp slt i64 %180, 1
  %182 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %124, i64 %172)
  %183 = add nsw i64 %178, 2
  %184 = sub nsw i64 %183, %176
  %185 = sext i32 %19 to i64
  %186 = srem i32 %19, 3
  %187 = sext i32 %186 to i64
  %188 = add nsw i32 %19, 1
  %189 = sext i32 %188 to i64
  %190 = add nsw i32 %19, 2
  %191 = sext i32 %190 to i64
  %192 = sub nsw i64 1, %189
  %193 = add nsw i64 %192, %191
  %194 = icmp slt i64 %193, 1
  %195 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %127, i64 %185)
  %196 = add nsw i64 %191, 2
  %197 = sub nsw i64 %196, %189
  %198 = sext i32 %19 to i64
  %199 = srem i32 %19, 3
  %200 = sext i32 %199 to i64
  %201 = add nsw i32 %19, 1
  %202 = sext i32 %201 to i64
  %203 = add nsw i32 %19, 2
  %204 = sext i32 %203 to i64
  %205 = sub nsw i64 1, %202
  %206 = add nsw i64 %205, %204
  %207 = icmp slt i64 %206, 1
  %208 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %130, i64 %198)
  %209 = add nsw i64 %204, 2
  %210 = sub nsw i64 %209, %202
  %211 = sext i32 %19 to i64
  %212 = srem i32 %19, 3
  %213 = sext i32 %212 to i64
  %214 = add nsw i32 %19, 1
  %215 = sext i32 %214 to i64
  %216 = add nsw i32 %19, 2
  %217 = sext i32 %216 to i64
  %218 = sub nsw i64 1, %215
  %219 = add nsw i64 %218, %217
  %220 = icmp slt i64 %219, 1
  %221 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %133, i64 %211)
  %222 = add nsw i64 %217, 2
  %223 = sub nsw i64 %222, %215
  %224 = sext i32 %19 to i64
  %225 = srem i32 %19, 3
  %226 = sext i32 %225 to i64
  %227 = add nsw i32 %19, 1
  %228 = sext i32 %227 to i64
  %229 = add nsw i32 %19, 2
  %230 = sext i32 %229 to i64
  %231 = sub nsw i64 1, %228
  %232 = add nsw i64 %231, %230
  %233 = icmp slt i64 %232, 1
  %234 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %136, i64 %224)
  %235 = add nsw i64 %230, 2
  %236 = sub nsw i64 %235, %228
  %237 = sext i32 %19 to i64
  %238 = srem i32 %19, 3
  %239 = sext i32 %238 to i64
  %240 = add nsw i32 %19, 1
  %241 = sext i32 %240 to i64
  %242 = add nsw i32 %19, 2
  %243 = sext i32 %242 to i64
  %244 = sub nsw i64 1, %241
  %245 = add nsw i64 %244, %243
  %246 = icmp slt i64 %245, 1
  %247 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %139, i64 %237)
  %248 = add nsw i64 %243, 2
  %249 = sub nsw i64 %248, %241
  %250 = sext i32 %19 to i64
  %251 = srem i32 %19, 3
  %252 = sext i32 %251 to i64
  %253 = add nsw i32 %19, 1
  %254 = sext i32 %253 to i64
  %255 = add nsw i32 %19, 2
  %256 = sext i32 %255 to i64
  %257 = sub nsw i64 1, %254
  %258 = add nsw i64 %257, %256
  %259 = icmp slt i64 %258, 1
  %260 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %140, i64 %250)
  %261 = icmp eq i32 %19, 5
  %262 = icmp eq i32 %19, 8
  %263 = add nsw i64 %256, 2
  %264 = sub nsw i64 %263, %254
  br label %265

265:                                              ; preds = %1270, %100
  %266 = phi i64 [ %157, %100 ], [ %1271, %1270 ]
  %267 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 324, i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %266)
  %268 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %267, i64 1)
  %269 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %268, i64 %20)
  %270 = load i32, i32* %269, align 4
  %271 = icmp slt i32 %270, 1
  br i1 %271, label %1270, label %272

272:                                              ; preds = %272, %265
  %273 = phi i64 [ %278, %272 ], [ 2, %265 ]
  %274 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %267, i64 %273)
  %275 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %274, i64 %20)
  %276 = load i32, i32* %275, align 4
  %277 = add nsw i32 %276, -10
  store i32 %277, i32* %275, align 4
  %278 = add nuw nsw i64 %273, 1
  %279 = icmp eq i64 %278, 10
  br i1 %279, label %280, label %272

280:                                              ; preds = %272
  br i1 %107, label %290, label %281

281:                                              ; preds = %281, %280
  %282 = phi i64 [ %288, %281 ], [ 1, %280 ]
  %283 = phi i64 [ %287, %281 ], [ %105, %280 ]
  %284 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %268, i64 %283)
  %285 = load i32, i32* %284, align 4
  %286 = add nsw i32 %285, -10
  store i32 %286, i32* %284, align 4
  %287 = add nsw i64 %283, 1
  %288 = add nuw nsw i64 %282, 1
  %289 = icmp slt i64 %282, %106
  br i1 %289, label %281, label %290

290:                                              ; preds = %281, %280
  switch i64 %109, label %315 [
    i64 1, label %291
    i64 2, label %307
  ]

291:                                              ; preds = %290
  br i1 %116, label %315, label %292

292:                                              ; preds = %304, %291
  %293 = phi i64 [ %305, %304 ], [ 1, %291 ]
  %294 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %267, i64 %293)
  br label %295

295:                                              ; preds = %295, %292
  %296 = phi i64 [ 1, %292 ], [ %302, %295 ]
  %297 = phi i64 [ %111, %292 ], [ %301, %295 ]
  %298 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %294, i64 %297)
  %299 = load i32, i32* %298, align 4
  %300 = add nsw i32 %299, -10
  store i32 %300, i32* %298, align 4
  %301 = add nsw i64 %297, 1
  %302 = add nuw nsw i64 %296, 1
  %303 = icmp eq i64 %302, %156
  br i1 %303, label %304, label %295

304:                                              ; preds = %295
  %305 = add nuw nsw i64 %293, 1
  %306 = icmp eq i64 %305, 4
  br i1 %306, label %315, label %292

307:                                              ; preds = %307, %290
  %308 = phi i64 [ %313, %307 ], [ 1, %290 ]
  %309 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %267, i64 %308)
  %310 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %309, i64 %111)
  %311 = load i32, i32* %310, align 4
  %312 = add nsw i32 %311, -10
  store i32 %312, i32* %310, align 4
  %313 = add nuw nsw i64 %308, 1
  %314 = icmp eq i64 %313, 4
  br i1 %314, label %315, label %307

315:                                              ; preds = %307, %304, %291, %290
  %316 = trunc i64 %266 to i32
  store i32 %316, i32* %118, align 4
  %317 = load i32, i32* %119, align 4
  %318 = load i32, i32* %120, align 4
  %319 = icmp slt i32 %318, %317
  br i1 %319, label %1226, label %320

320:                                              ; preds = %315
  %321 = sub i32 45, %316
  %322 = sext i32 %317 to i64
  %323 = sext i32 %318 to i64
  br label %324

324:                                              ; preds = %1223, %320
  %325 = phi i64 [ %322, %320 ], [ %1224, %1223 ]
  %326 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 324, i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %325)
  %327 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %326, i64 2)
  %328 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %327, i64 %159)
  %329 = load i32, i32* %328, align 4
  %330 = icmp slt i32 %329, 1
  br i1 %330, label %1223, label %332

331:                                              ; preds = %332
  br i1 %107, label %349, label %340

332:                                              ; preds = %332, %324
  %333 = phi i64 [ %338, %332 ], [ 3, %324 ]
  %334 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %326, i64 %333)
  %335 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %334, i64 %159)
  %336 = load i32, i32* %335, align 4
  %337 = add nsw i32 %336, -10
  store i32 %337, i32* %335, align 4
  %338 = add nuw nsw i64 %333, 1
  %339 = icmp eq i64 %338, 10
  br i1 %339, label %331, label %332

340:                                              ; preds = %340, %331
  %341 = phi i64 [ %347, %340 ], [ 1, %331 ]
  %342 = phi i64 [ %346, %340 ], [ %105, %331 ]
  %343 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %327, i64 %342)
  %344 = load i32, i32* %343, align 4
  %345 = add nsw i32 %344, -10
  store i32 %345, i32* %343, align 4
  %346 = add nsw i64 %342, 1
  %347 = add nuw nsw i64 %341, 1
  %348 = icmp slt i64 %341, %106
  br i1 %348, label %340, label %349

349:                                              ; preds = %340, %331
  switch i64 %161, label %374 [
    i64 1, label %350
    i64 2, label %366
  ]

350:                                              ; preds = %349
  br i1 %168, label %374, label %351

351:                                              ; preds = %363, %350
  %352 = phi i64 [ %364, %363 ], [ 1, %350 ]
  %353 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %326, i64 %352)
  br label %354

354:                                              ; preds = %354, %351
  %355 = phi i64 [ 1, %351 ], [ %361, %354 ]
  %356 = phi i64 [ %163, %351 ], [ %360, %354 ]
  %357 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %353, i64 %356)
  %358 = load i32, i32* %357, align 4
  %359 = add nsw i32 %358, -10
  store i32 %359, i32* %357, align 4
  %360 = add nsw i64 %356, 1
  %361 = add nuw nsw i64 %355, 1
  %362 = icmp eq i64 %361, %171
  br i1 %362, label %363, label %354

363:                                              ; preds = %354
  %364 = add nuw nsw i64 %352, 1
  %365 = icmp eq i64 %364, 4
  br i1 %365, label %374, label %351

366:                                              ; preds = %366, %349
  %367 = phi i64 [ %372, %366 ], [ 1, %349 ]
  %368 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %326, i64 %367)
  %369 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %368, i64 %163)
  %370 = load i32, i32* %369, align 4
  %371 = add nsw i32 %370, -10
  store i32 %371, i32* %369, align 4
  %372 = add nuw nsw i64 %367, 1
  %373 = icmp eq i64 %372, 4
  br i1 %373, label %374, label %366

374:                                              ; preds = %366, %363, %350, %349
  %375 = trunc i64 %325 to i32
  store i32 %375, i32* %169, align 4
  %376 = load i32, i32* %122, align 4
  %377 = load i32, i32* %123, align 4
  %378 = icmp slt i32 %377, %376
  br i1 %378, label %1179, label %379

379:                                              ; preds = %374
  %380 = sub i32 %321, %375
  %381 = sext i32 %376 to i64
  %382 = sext i32 %377 to i64
  br label %383

383:                                              ; preds = %1176, %379
  %384 = phi i64 [ %381, %379 ], [ %1177, %1176 ]
  %385 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 324, i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %384)
  %386 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %385, i64 3)
  %387 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %386, i64 %172)
  %388 = load i32, i32* %387, align 4
  %389 = icmp slt i32 %388, 1
  br i1 %389, label %1176, label %391

390:                                              ; preds = %391
  br i1 %107, label %408, label %399

391:                                              ; preds = %391, %383
  %392 = phi i64 [ %397, %391 ], [ 4, %383 ]
  %393 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %385, i64 %392)
  %394 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %393, i64 %172)
  %395 = load i32, i32* %394, align 4
  %396 = add nsw i32 %395, -10
  store i32 %396, i32* %394, align 4
  %397 = add nuw nsw i64 %392, 1
  %398 = icmp eq i64 %397, 10
  br i1 %398, label %390, label %391

399:                                              ; preds = %399, %390
  %400 = phi i64 [ %406, %399 ], [ 1, %390 ]
  %401 = phi i64 [ %405, %399 ], [ %105, %390 ]
  %402 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %386, i64 %401)
  %403 = load i32, i32* %402, align 4
  %404 = add nsw i32 %403, -10
  store i32 %404, i32* %402, align 4
  %405 = add nsw i64 %401, 1
  %406 = add nuw nsw i64 %400, 1
  %407 = icmp slt i64 %400, %106
  br i1 %407, label %399, label %408

408:                                              ; preds = %399, %390
  switch i64 %174, label %433 [
    i64 1, label %409
    i64 2, label %425
  ]

409:                                              ; preds = %408
  br i1 %181, label %433, label %410

410:                                              ; preds = %422, %409
  %411 = phi i64 [ %423, %422 ], [ 1, %409 ]
  %412 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %385, i64 %411)
  br label %413

413:                                              ; preds = %413, %410
  %414 = phi i64 [ 1, %410 ], [ %420, %413 ]
  %415 = phi i64 [ %176, %410 ], [ %419, %413 ]
  %416 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %412, i64 %415)
  %417 = load i32, i32* %416, align 4
  %418 = add nsw i32 %417, -10
  store i32 %418, i32* %416, align 4
  %419 = add nsw i64 %415, 1
  %420 = add nuw nsw i64 %414, 1
  %421 = icmp eq i64 %420, %184
  br i1 %421, label %422, label %413

422:                                              ; preds = %413
  %423 = add nuw nsw i64 %411, 1
  %424 = icmp eq i64 %423, 4
  br i1 %424, label %433, label %410

425:                                              ; preds = %425, %408
  %426 = phi i64 [ %431, %425 ], [ 1, %408 ]
  %427 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %385, i64 %426)
  %428 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %427, i64 %176)
  %429 = load i32, i32* %428, align 4
  %430 = add nsw i32 %429, -10
  store i32 %430, i32* %428, align 4
  %431 = add nuw nsw i64 %426, 1
  %432 = icmp eq i64 %431, 4
  br i1 %432, label %433, label %425

433:                                              ; preds = %425, %422, %409, %408
  %434 = trunc i64 %384 to i32
  store i32 %434, i32* %182, align 4
  %435 = load i32, i32* %125, align 4
  %436 = load i32, i32* %126, align 4
  %437 = icmp slt i32 %436, %435
  br i1 %437, label %1132, label %438

438:                                              ; preds = %433
  %439 = sub i32 %380, %434
  %440 = sext i32 %435 to i64
  %441 = sext i32 %436 to i64
  br label %442

442:                                              ; preds = %1129, %438
  %443 = phi i64 [ %440, %438 ], [ %1130, %1129 ]
  %444 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 324, i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %443)
  %445 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %444, i64 4)
  %446 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %445, i64 %185)
  %447 = load i32, i32* %446, align 4
  %448 = icmp slt i32 %447, 1
  br i1 %448, label %1129, label %450

449:                                              ; preds = %450
  br i1 %107, label %467, label %458

450:                                              ; preds = %450, %442
  %451 = phi i64 [ %456, %450 ], [ 5, %442 ]
  %452 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %444, i64 %451)
  %453 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %452, i64 %185)
  %454 = load i32, i32* %453, align 4
  %455 = add nsw i32 %454, -10
  store i32 %455, i32* %453, align 4
  %456 = add nuw nsw i64 %451, 1
  %457 = icmp eq i64 %456, 10
  br i1 %457, label %449, label %450

458:                                              ; preds = %458, %449
  %459 = phi i64 [ %465, %458 ], [ 1, %449 ]
  %460 = phi i64 [ %464, %458 ], [ %105, %449 ]
  %461 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %445, i64 %460)
  %462 = load i32, i32* %461, align 4
  %463 = add nsw i32 %462, -10
  store i32 %463, i32* %461, align 4
  %464 = add nsw i64 %460, 1
  %465 = add nuw nsw i64 %459, 1
  %466 = icmp slt i64 %459, %106
  br i1 %466, label %458, label %467

467:                                              ; preds = %458, %449
  switch i64 %187, label %492 [
    i64 1, label %468
    i64 2, label %484
  ]

468:                                              ; preds = %467
  br i1 %194, label %492, label %469

469:                                              ; preds = %481, %468
  %470 = phi i64 [ %482, %481 ], [ 4, %468 ]
  %471 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %444, i64 %470)
  br label %472

472:                                              ; preds = %472, %469
  %473 = phi i64 [ 1, %469 ], [ %479, %472 ]
  %474 = phi i64 [ %189, %469 ], [ %478, %472 ]
  %475 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %471, i64 %474)
  %476 = load i32, i32* %475, align 4
  %477 = add nsw i32 %476, -10
  store i32 %477, i32* %475, align 4
  %478 = add nsw i64 %474, 1
  %479 = add nuw nsw i64 %473, 1
  %480 = icmp eq i64 %479, %197
  br i1 %480, label %481, label %472

481:                                              ; preds = %472
  %482 = add nuw nsw i64 %470, 1
  %483 = icmp eq i64 %482, 7
  br i1 %483, label %492, label %469

484:                                              ; preds = %484, %467
  %485 = phi i64 [ %490, %484 ], [ 4, %467 ]
  %486 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %444, i64 %485)
  %487 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %486, i64 %189)
  %488 = load i32, i32* %487, align 4
  %489 = add nsw i32 %488, -10
  store i32 %489, i32* %487, align 4
  %490 = add nuw nsw i64 %485, 1
  %491 = icmp eq i64 %490, 7
  br i1 %491, label %492, label %484

492:                                              ; preds = %484, %481, %468, %467
  %493 = trunc i64 %443 to i32
  store i32 %493, i32* %195, align 4
  %494 = load i32, i32* %128, align 4
  %495 = load i32, i32* %129, align 4
  %496 = icmp slt i32 %495, %494
  br i1 %496, label %1085, label %497

497:                                              ; preds = %492
  %498 = sub i32 %439, %493
  %499 = sext i32 %494 to i64
  %500 = sext i32 %495 to i64
  br label %501

501:                                              ; preds = %1082, %497
  %502 = phi i64 [ %499, %497 ], [ %1083, %1082 ]
  %503 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 324, i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %502)
  %504 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %503, i64 5)
  %505 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %504, i64 %198)
  %506 = load i32, i32* %505, align 4
  %507 = icmp slt i32 %506, 1
  br i1 %507, label %1082, label %509

508:                                              ; preds = %509
  br i1 %107, label %526, label %517

509:                                              ; preds = %509, %501
  %510 = phi i64 [ %515, %509 ], [ 6, %501 ]
  %511 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %503, i64 %510)
  %512 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %511, i64 %198)
  %513 = load i32, i32* %512, align 4
  %514 = add nsw i32 %513, -10
  store i32 %514, i32* %512, align 4
  %515 = add nuw nsw i64 %510, 1
  %516 = icmp eq i64 %515, 10
  br i1 %516, label %508, label %509

517:                                              ; preds = %517, %508
  %518 = phi i64 [ %524, %517 ], [ 1, %508 ]
  %519 = phi i64 [ %523, %517 ], [ %105, %508 ]
  %520 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %504, i64 %519)
  %521 = load i32, i32* %520, align 4
  %522 = add nsw i32 %521, -10
  store i32 %522, i32* %520, align 4
  %523 = add nsw i64 %519, 1
  %524 = add nuw nsw i64 %518, 1
  %525 = icmp slt i64 %518, %106
  br i1 %525, label %517, label %526

526:                                              ; preds = %517, %508
  switch i64 %200, label %551 [
    i64 1, label %527
    i64 2, label %543
  ]

527:                                              ; preds = %526
  br i1 %207, label %551, label %528

528:                                              ; preds = %540, %527
  %529 = phi i64 [ %541, %540 ], [ 4, %527 ]
  %530 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %503, i64 %529)
  br label %531

531:                                              ; preds = %531, %528
  %532 = phi i64 [ 1, %528 ], [ %538, %531 ]
  %533 = phi i64 [ %202, %528 ], [ %537, %531 ]
  %534 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %530, i64 %533)
  %535 = load i32, i32* %534, align 4
  %536 = add nsw i32 %535, -10
  store i32 %536, i32* %534, align 4
  %537 = add nsw i64 %533, 1
  %538 = add nuw nsw i64 %532, 1
  %539 = icmp eq i64 %538, %210
  br i1 %539, label %540, label %531

540:                                              ; preds = %531
  %541 = add nuw nsw i64 %529, 1
  %542 = icmp eq i64 %541, 7
  br i1 %542, label %551, label %528

543:                                              ; preds = %543, %526
  %544 = phi i64 [ %549, %543 ], [ 4, %526 ]
  %545 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %503, i64 %544)
  %546 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %545, i64 %202)
  %547 = load i32, i32* %546, align 4
  %548 = add nsw i32 %547, -10
  store i32 %548, i32* %546, align 4
  %549 = add nuw nsw i64 %544, 1
  %550 = icmp eq i64 %549, 7
  br i1 %550, label %551, label %543

551:                                              ; preds = %543, %540, %527, %526
  %552 = trunc i64 %502 to i32
  store i32 %552, i32* %208, align 4
  %553 = load i32, i32* %131, align 4
  %554 = load i32, i32* %132, align 4
  %555 = icmp slt i32 %554, %553
  br i1 %555, label %1038, label %556

556:                                              ; preds = %551
  %557 = sub i32 %498, %552
  %558 = sext i32 %553 to i64
  %559 = sext i32 %554 to i64
  br label %560

560:                                              ; preds = %1035, %556
  %561 = phi i64 [ %558, %556 ], [ %1036, %1035 ]
  %562 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 324, i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %561)
  %563 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %562, i64 6)
  %564 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %563, i64 %211)
  %565 = load i32, i32* %564, align 4
  %566 = icmp slt i32 %565, 1
  br i1 %566, label %1035, label %568

567:                                              ; preds = %568
  br i1 %107, label %585, label %576

568:                                              ; preds = %568, %560
  %569 = phi i64 [ %574, %568 ], [ 7, %560 ]
  %570 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %562, i64 %569)
  %571 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %570, i64 %211)
  %572 = load i32, i32* %571, align 4
  %573 = add nsw i32 %572, -10
  store i32 %573, i32* %571, align 4
  %574 = add nuw nsw i64 %569, 1
  %575 = icmp eq i64 %574, 10
  br i1 %575, label %567, label %568

576:                                              ; preds = %576, %567
  %577 = phi i64 [ %583, %576 ], [ 1, %567 ]
  %578 = phi i64 [ %582, %576 ], [ %105, %567 ]
  %579 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %563, i64 %578)
  %580 = load i32, i32* %579, align 4
  %581 = add nsw i32 %580, -10
  store i32 %581, i32* %579, align 4
  %582 = add nsw i64 %578, 1
  %583 = add nuw nsw i64 %577, 1
  %584 = icmp slt i64 %577, %106
  br i1 %584, label %576, label %585

585:                                              ; preds = %576, %567
  switch i64 %213, label %610 [
    i64 1, label %586
    i64 2, label %602
  ]

586:                                              ; preds = %585
  br i1 %220, label %610, label %587

587:                                              ; preds = %599, %586
  %588 = phi i64 [ %600, %599 ], [ 4, %586 ]
  %589 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %562, i64 %588)
  br label %590

590:                                              ; preds = %590, %587
  %591 = phi i64 [ 1, %587 ], [ %597, %590 ]
  %592 = phi i64 [ %215, %587 ], [ %596, %590 ]
  %593 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %589, i64 %592)
  %594 = load i32, i32* %593, align 4
  %595 = add nsw i32 %594, -10
  store i32 %595, i32* %593, align 4
  %596 = add nsw i64 %592, 1
  %597 = add nuw nsw i64 %591, 1
  %598 = icmp eq i64 %597, %223
  br i1 %598, label %599, label %590

599:                                              ; preds = %590
  %600 = add nuw nsw i64 %588, 1
  %601 = icmp eq i64 %600, 7
  br i1 %601, label %610, label %587

602:                                              ; preds = %602, %585
  %603 = phi i64 [ %608, %602 ], [ 4, %585 ]
  %604 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %562, i64 %603)
  %605 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %604, i64 %215)
  %606 = load i32, i32* %605, align 4
  %607 = add nsw i32 %606, -10
  store i32 %607, i32* %605, align 4
  %608 = add nuw nsw i64 %603, 1
  %609 = icmp eq i64 %608, 7
  br i1 %609, label %610, label %602

610:                                              ; preds = %602, %599, %586, %585
  %611 = trunc i64 %561 to i32
  store i32 %611, i32* %221, align 4
  %612 = load i32, i32* %134, align 4
  %613 = load i32, i32* %135, align 4
  %614 = icmp slt i32 %613, %612
  br i1 %614, label %991, label %615

615:                                              ; preds = %610
  %616 = sub i32 %557, %611
  %617 = sext i32 %612 to i64
  %618 = sext i32 %613 to i64
  br label %619

619:                                              ; preds = %988, %615
  %620 = phi i64 [ %617, %615 ], [ %989, %988 ]
  %621 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 324, i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %620)
  %622 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %621, i64 7)
  %623 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %622, i64 %224)
  %624 = load i32, i32* %623, align 4
  %625 = icmp slt i32 %624, 1
  br i1 %625, label %988, label %627

626:                                              ; preds = %627
  br i1 %107, label %644, label %635

627:                                              ; preds = %627, %619
  %628 = phi i64 [ %633, %627 ], [ 8, %619 ]
  %629 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %621, i64 %628)
  %630 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %629, i64 %224)
  %631 = load i32, i32* %630, align 4
  %632 = add nsw i32 %631, -10
  store i32 %632, i32* %630, align 4
  %633 = add nuw nsw i64 %628, 1
  %634 = icmp eq i64 %633, 10
  br i1 %634, label %626, label %627

635:                                              ; preds = %635, %626
  %636 = phi i64 [ %642, %635 ], [ 1, %626 ]
  %637 = phi i64 [ %641, %635 ], [ %105, %626 ]
  %638 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %622, i64 %637)
  %639 = load i32, i32* %638, align 4
  %640 = add nsw i32 %639, -10
  store i32 %640, i32* %638, align 4
  %641 = add nsw i64 %637, 1
  %642 = add nuw nsw i64 %636, 1
  %643 = icmp slt i64 %636, %106
  br i1 %643, label %635, label %644

644:                                              ; preds = %635, %626
  switch i64 %226, label %669 [
    i64 1, label %645
    i64 2, label %661
  ]

645:                                              ; preds = %644
  br i1 %233, label %669, label %646

646:                                              ; preds = %658, %645
  %647 = phi i64 [ %659, %658 ], [ 7, %645 ]
  %648 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %621, i64 %647)
  br label %649

649:                                              ; preds = %649, %646
  %650 = phi i64 [ 1, %646 ], [ %656, %649 ]
  %651 = phi i64 [ %228, %646 ], [ %655, %649 ]
  %652 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %648, i64 %651)
  %653 = load i32, i32* %652, align 4
  %654 = add nsw i32 %653, -10
  store i32 %654, i32* %652, align 4
  %655 = add nsw i64 %651, 1
  %656 = add nuw nsw i64 %650, 1
  %657 = icmp eq i64 %656, %236
  br i1 %657, label %658, label %649

658:                                              ; preds = %649
  %659 = add nuw nsw i64 %647, 1
  %660 = icmp eq i64 %659, 10
  br i1 %660, label %669, label %646

661:                                              ; preds = %661, %644
  %662 = phi i64 [ %667, %661 ], [ 7, %644 ]
  %663 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %621, i64 %662)
  %664 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %663, i64 %228)
  %665 = load i32, i32* %664, align 4
  %666 = add nsw i32 %665, -10
  store i32 %666, i32* %664, align 4
  %667 = add nuw nsw i64 %662, 1
  %668 = icmp eq i64 %667, 10
  br i1 %668, label %669, label %661

669:                                              ; preds = %661, %658, %645, %644
  %670 = trunc i64 %620 to i32
  store i32 %670, i32* %234, align 4
  %671 = load i32, i32* %137, align 4
  %672 = load i32, i32* %138, align 4
  %673 = icmp slt i32 %672, %671
  br i1 %673, label %944, label %674

674:                                              ; preds = %669
  %675 = sub i32 %616, %670
  %676 = sext i32 %671 to i64
  %677 = sext i32 %672 to i64
  br label %678

678:                                              ; preds = %941, %674
  %679 = phi i64 [ %676, %674 ], [ %942, %941 ]
  %680 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 324, i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %679)
  %681 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %680, i64 8)
  %682 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %681, i64 %237)
  %683 = load i32, i32* %682, align 4
  %684 = icmp slt i32 %683, 1
  br i1 %684, label %941, label %685

685:                                              ; preds = %678
  %686 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %680, i64 9)
  %687 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %686, i64 %237)
  %688 = load i32, i32* %687, align 4
  %689 = add nsw i32 %688, -10
  store i32 %689, i32* %687, align 4
  br i1 %107, label %699, label %690

690:                                              ; preds = %690, %685
  %691 = phi i64 [ %697, %690 ], [ 1, %685 ]
  %692 = phi i64 [ %696, %690 ], [ %105, %685 ]
  %693 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %681, i64 %692)
  %694 = load i32, i32* %693, align 4
  %695 = add nsw i32 %694, -10
  store i32 %695, i32* %693, align 4
  %696 = add nsw i64 %692, 1
  %697 = add nuw nsw i64 %691, 1
  %698 = icmp slt i64 %691, %106
  br i1 %698, label %690, label %699

699:                                              ; preds = %690, %685
  switch i64 %239, label %724 [
    i64 1, label %700
    i64 2, label %716
  ]

700:                                              ; preds = %699
  br i1 %246, label %724, label %701

701:                                              ; preds = %713, %700
  %702 = phi i64 [ %714, %713 ], [ 7, %700 ]
  %703 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %680, i64 %702)
  br label %704

704:                                              ; preds = %704, %701
  %705 = phi i64 [ 1, %701 ], [ %711, %704 ]
  %706 = phi i64 [ %241, %701 ], [ %710, %704 ]
  %707 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %703, i64 %706)
  %708 = load i32, i32* %707, align 4
  %709 = add nsw i32 %708, -10
  store i32 %709, i32* %707, align 4
  %710 = add nsw i64 %706, 1
  %711 = add nuw nsw i64 %705, 1
  %712 = icmp eq i64 %711, %249
  br i1 %712, label %713, label %704

713:                                              ; preds = %704
  %714 = add nuw nsw i64 %702, 1
  %715 = icmp eq i64 %714, 10
  br i1 %715, label %724, label %701

716:                                              ; preds = %716, %699
  %717 = phi i64 [ %722, %716 ], [ 7, %699 ]
  %718 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %680, i64 %717)
  %719 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %718, i64 %241)
  %720 = load i32, i32* %719, align 4
  %721 = add nsw i32 %720, -10
  store i32 %721, i32* %719, align 4
  %722 = add nuw nsw i64 %717, 1
  %723 = icmp eq i64 %722, 10
  br i1 %723, label %724, label %716

724:                                              ; preds = %716, %713, %700, %699
  %725 = trunc i64 %679 to i32
  store i32 %725, i32* %247, align 4
  %726 = sub i32 %675, %725
  %727 = sext i32 %726 to i64
  %728 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 324, i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %727)
  %729 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %728, i64 9)
  %730 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %729, i64 %250)
  %731 = load i32, i32* %730, align 4
  %732 = icmp slt i32 %731, 1
  br i1 %732, label %904, label %733

733:                                              ; preds = %724
  br i1 %107, label %743, label %734

734:                                              ; preds = %734, %733
  %735 = phi i64 [ %741, %734 ], [ 1, %733 ]
  %736 = phi i64 [ %740, %734 ], [ %105, %733 ]
  %737 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %729, i64 %736)
  %738 = load i32, i32* %737, align 4
  %739 = add nsw i32 %738, -10
  store i32 %739, i32* %737, align 4
  %740 = add nsw i64 %736, 1
  %741 = add nuw nsw i64 %735, 1
  %742 = icmp slt i64 %735, %106
  br i1 %742, label %734, label %743

743:                                              ; preds = %734, %733
  switch i64 %252, label %768 [
    i64 1, label %744
    i64 2, label %760
  ]

744:                                              ; preds = %743
  br i1 %259, label %768, label %745

745:                                              ; preds = %757, %744
  %746 = phi i64 [ %758, %757 ], [ 7, %744 ]
  %747 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %728, i64 %746)
  br label %748

748:                                              ; preds = %748, %745
  %749 = phi i64 [ 1, %745 ], [ %755, %748 ]
  %750 = phi i64 [ %254, %745 ], [ %754, %748 ]
  %751 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %747, i64 %750)
  %752 = load i32, i32* %751, align 4
  %753 = add nsw i32 %752, -10
  store i32 %753, i32* %751, align 4
  %754 = add nsw i64 %750, 1
  %755 = add nuw nsw i64 %749, 1
  %756 = icmp eq i64 %755, %264
  br i1 %756, label %757, label %748

757:                                              ; preds = %748
  %758 = add nuw nsw i64 %746, 1
  %759 = icmp eq i64 %758, 10
  br i1 %759, label %768, label %745

760:                                              ; preds = %760, %743
  %761 = phi i64 [ %766, %760 ], [ 7, %743 ]
  %762 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %728, i64 %761)
  %763 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %762, i64 %254)
  %764 = load i32, i32* %763, align 4
  %765 = add nsw i32 %764, -10
  store i32 %765, i32* %763, align 4
  %766 = add nuw nsw i64 %761, 1
  %767 = icmp eq i64 %766, 10
  br i1 %767, label %768, label %760

768:                                              ; preds = %760, %757, %744, %743
  store i32 %726, i32* %260, align 4
  br i1 %261, label %769, label %792

769:                                              ; preds = %789, %768
  %770 = phi i64 [ %790, %789 ], [ 1, %768 ]
  %771 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @brute_force_mp_sudoku1_, i64 0, i64 0, i64 0), i64 %770)
  %772 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %771, i64 %254)
  %773 = load i32, i32* %772, align 4
  %774 = icmp eq i32 %773, 0
  br i1 %774, label %775, label %789

775:                                              ; preds = %775, %769
  %776 = phi i64 [ %784, %775 ], [ 1, %769 ]
  %777 = phi i32 [ %783, %775 ], [ 0, %769 ]
  %778 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 324, i32* getelementptr inbounds ([9 x [9 x [9 x i32]]], [9 x [9 x [9 x i32]]]* @brute_force_mp_block_, i64 0, i64 0, i64 0, i64 0), i64 %776)
  %779 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %778, i64 %770)
  %780 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %779, i64 %254)
  %781 = load i32, i32* %780, align 4
  %782 = icmp sgt i32 %781, 0
  %783 = select i1 %782, i32 -1, i32 %777
  %784 = add nuw nsw i64 %776, 1
  %785 = icmp eq i64 %784, 10
  br i1 %785, label %786, label %775

786:                                              ; preds = %775
  %787 = and i32 %783, 1
  %788 = icmp eq i32 %787, 0
  br i1 %788, label %792, label %789

789:                                              ; preds = %786, %769
  %790 = add nuw nsw i64 %770, 1
  %791 = icmp eq i64 %790, 10
  br i1 %791, label %792, label %769

792:                                              ; preds = %789, %786, %768
  %793 = phi i32 [ 1, %768 ], [ 1, %789 ], [ 0, %786 ]
  br i1 %262, label %794, label %864

794:                                              ; preds = %794, %792
  %795 = phi i64 [ %797, %794 ], [ 1, %792 ]
  %796 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %141, i64 %795)
  store i32 0, i32* %796, align 4
  %797 = add nuw nsw i64 %795, 1
  %798 = icmp eq i64 %797, 10
  br i1 %798, label %799, label %794

799:                                              ; preds = %812, %794
  %800 = phi i64 [ %813, %812 ], [ 1, %794 ]
  %801 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 %800)
  %802 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %141, i64 %800)
  %803 = load i32, i32* %802, align 4
  br label %804

804:                                              ; preds = %804, %799
  %805 = phi i32 [ %803, %799 ], [ %809, %804 ]
  %806 = phi i64 [ 1, %799 ], [ %810, %804 ]
  %807 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %801, i64 %806)
  %808 = load i32, i32* %807, align 4
  %809 = add nsw i32 %805, %808
  %810 = add nuw nsw i64 %806, 1
  %811 = icmp eq i64 %810, 9
  br i1 %811, label %812, label %804

812:                                              ; preds = %804
  store i32 %809, i32* %802, align 4
  %813 = add nuw nsw i64 %800, 1
  %814 = icmp eq i64 %813, 10
  br i1 %814, label %815, label %799

815:                                              ; preds = %815, %812
  %816 = phi i64 [ %821, %815 ], [ 1, %812 ]
  %817 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %141, i64 %816)
  %818 = load i32, i32* %817, align 4
  %819 = sub nsw i32 45, %818
  %820 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %142, i64 %816)
  store i32 %819, i32* %820, align 4
  %821 = add nuw nsw i64 %816, 1
  %822 = icmp eq i64 %821, 10
  br i1 %822, label %823, label %815

823:                                              ; preds = %823, %815
  %824 = phi i64 [ %829, %823 ], [ 1, %815 ]
  %825 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 %824)
  %826 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %825, i64 9)
  %827 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %142, i64 %824)
  %828 = load i32, i32* %827, align 4
  store i32 %828, i32* %826, align 4
  %829 = add nuw nsw i64 %824, 1
  %830 = icmp eq i64 %829, 10
  br i1 %830, label %831, label %823

831:                                              ; preds = %831, %823
  %832 = phi i64 [ %838, %831 ], [ 1, %823 ]
  %833 = phi i32 [ %837, %831 ], [ 0, %823 ]
  %834 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 %832)
  %835 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %834, i64 9)
  %836 = load i32, i32* %835, align 4
  %837 = add nsw i32 %836, %833
  %838 = add nuw nsw i64 %832, 1
  %839 = icmp eq i64 %838, 10
  br i1 %839, label %840, label %831

840:                                              ; preds = %831
  %841 = icmp eq i32 %837, 45
  br i1 %841, label %842, label %904

842:                                              ; preds = %840
  %843 = load i32, i32* @brute_force_mp_soln_, align 8
  %844 = add nsw i32 %843, 1
  store i32 %844, i32* @brute_force_mp_soln_, align 8
  br label %845

845:                                              ; preds = %856, %842
  %846 = phi i64 [ 1, %842 ], [ %857, %856 ]
  %847 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @brute_force_mp_sudoku2_, i64 0, i64 0, i64 0), i64 %846)
  %848 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @brute_force_mp_sudoku3_, i64 0, i64 0, i64 0), i64 %846)
  br label %849

849:                                              ; preds = %849, %845
  %850 = phi i64 [ 1, %845 ], [ %854, %849 ]
  %851 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %847, i64 %850)
  %852 = load i32, i32* %851, align 4
  %853 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %848, i64 %850)
  store i32 %852, i32* %853, align 4
  %854 = add nuw nsw i64 %850, 1
  %855 = icmp eq i64 %854, 10
  br i1 %855, label %856, label %849

856:                                              ; preds = %849
  %857 = add nuw nsw i64 %846, 1
  %858 = icmp eq i64 %857, 10
  br i1 %858, label %859, label %845

859:                                              ; preds = %856
  store i64 4, i64* %10, align 8
  store i64 2, i64* %25, align 8
  store i64 0, i64* %24, align 8
  store i64 4, i64* %143, align 8
  store i64 1, i64* %144, align 8
  store i64 9, i64* %145, align 8
  store i64 36, i64* %146, align 8
  store i64 1, i64* %147, align 8
  store i64 9, i64* %148, align 8
  store i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @brute_force_mp_sudoku3_, i64 0, i64 0, i64 0), i32** %23, align 8
  store i64 1, i64* %11, align 8
  store i64 4, i64* %15, align 8
  store i64 2, i64* %29, align 8
  store i64 0, i64* %28, align 8
  store i64 4, i64* %149, align 8
  store i64 1, i64* %150, align 8
  store i64 9, i64* %151, align 8
  store i64 36, i64* %152, align 8
  store i64 1, i64* %153, align 8
  store i64 9, i64* %154, align 8
  store i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @brute_force_mp_sudoku1_, i64 0, i64 0, i64 0), i32** %27, align 8
  store i64 1, i64* %16, align 8
  %860 = call i32 @brute_force_mp_covered_({ i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* nonnull %4, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* nonnull %3)
  %861 = and i32 %860, 1
  %862 = icmp eq i32 %861, 0
  br i1 %862, label %863, label %869

863:                                              ; preds = %859
  store i32 2, i32* @brute_force_mp_soln_, align 8
  br label %869

864:                                              ; preds = %792
  %865 = icmp eq i32 %793, 0
  br i1 %865, label %869, label %866

866:                                              ; preds = %864
  store i32 %253, i32* %2, align 4
  call void @good(i32* nonnull %2)
  %867 = load i32, i32* @brute_force_mp_soln_, align 8
  %868 = icmp sgt i32 %867, 1
  br i1 %868, label %1273, label %869

869:                                              ; preds = %866, %864, %863, %859
  br i1 %107, label %879, label %870

870:                                              ; preds = %870, %869
  %871 = phi i64 [ %877, %870 ], [ 1, %869 ]
  %872 = phi i64 [ %876, %870 ], [ %105, %869 ]
  %873 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %729, i64 %872)
  %874 = load i32, i32* %873, align 4
  %875 = add nsw i32 %874, 10
  store i32 %875, i32* %873, align 4
  %876 = add nsw i64 %872, 1
  %877 = add nuw nsw i64 %871, 1
  %878 = icmp slt i64 %871, %106
  br i1 %878, label %870, label %879

879:                                              ; preds = %870, %869
  switch i64 %252, label %904 [
    i64 1, label %880
    i64 2, label %896
  ]

880:                                              ; preds = %879
  br i1 %259, label %904, label %881

881:                                              ; preds = %893, %880
  %882 = phi i64 [ %894, %893 ], [ 7, %880 ]
  %883 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %728, i64 %882)
  br label %884

884:                                              ; preds = %884, %881
  %885 = phi i64 [ 1, %881 ], [ %891, %884 ]
  %886 = phi i64 [ %254, %881 ], [ %890, %884 ]
  %887 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %883, i64 %886)
  %888 = load i32, i32* %887, align 4
  %889 = add nsw i32 %888, 10
  store i32 %889, i32* %887, align 4
  %890 = add nsw i64 %886, 1
  %891 = add nuw nsw i64 %885, 1
  %892 = icmp eq i64 %891, %264
  br i1 %892, label %893, label %884

893:                                              ; preds = %884
  %894 = add nuw nsw i64 %882, 1
  %895 = icmp eq i64 %894, 10
  br i1 %895, label %904, label %881

896:                                              ; preds = %896, %879
  %897 = phi i64 [ %902, %896 ], [ 7, %879 ]
  %898 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* %728, i64 %897)
  %899 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %898, i64 %254)
  %900 = load i32, i32* %899, align 4
  %901 = add nsw i32 %900, 10
  store i32 %901, i32* %899, align 4
  %902 = add nuw nsw i64 %897, 1
  %903 = icmp eq i64 %902, 10
  br i1 %903, label %904, label %896

904:                                              ; preds = %896, %893, %880, %879, %840, %724
  %905 = load i32, i32* %687, align 4
  %906 = add nsw i32 %905, 10
  store i32 %906, i32* %687, align 4
  br i1 %107, label %916, label %907

907:                                              ; preds = %907, %904
  %908 = phi i64 [ %914, %907 ], [ 1, %904 ]
  %909 = phi i64 [ %913, %907 ], [ %105, %904 ]
  %910 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %681, i64 %909)
  %911 = load i32, i32* %910, align 4
  %912 = add nsw i32 %911, 10
  store i32 %912, i32* %910, align 4
  %913 = add nsw i64 %909, 1
  %914 = add nuw nsw i64 %908, 1
  %915 = icmp slt i64 %908, %106
  br i1 %915, label %907, label %916

916:                                              ; preds = %907, %904
  switch i64 %239, label %941 [
    i64 1, label %917
    i64 2, label %933
  ]

917:                                              ; preds = %916
  br i1 %246, label %941, label %918

918:                                              ; preds = %930, %917
  %919 = phi i64 [ %931, %930 ], [ 7, %917 ]
  %920 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %680, i64 %919)
  br label %921

921:                                              ; preds = %921, %918
  %922 = phi i64 [ 1, %918 ], [ %928, %921 ]
  %923 = phi i64 [ %241, %918 ], [ %927, %921 ]
  %924 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %920, i64 %923)
  %925 = load i32, i32* %924, align 4
  %926 = add nsw i32 %925, 10
  store i32 %926, i32* %924, align 4
  %927 = add nsw i64 %923, 1
  %928 = add nuw nsw i64 %922, 1
  %929 = icmp eq i64 %928, %249
  br i1 %929, label %930, label %921

930:                                              ; preds = %921
  %931 = add nuw nsw i64 %919, 1
  %932 = icmp eq i64 %931, 10
  br i1 %932, label %941, label %918

933:                                              ; preds = %933, %916
  %934 = phi i64 [ %939, %933 ], [ 7, %916 ]
  %935 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %680, i64 %934)
  %936 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %935, i64 %241)
  %937 = load i32, i32* %936, align 4
  %938 = add nsw i32 %937, 10
  store i32 %938, i32* %936, align 4
  %939 = add nuw nsw i64 %934, 1
  %940 = icmp eq i64 %939, 10
  br i1 %940, label %941, label %933

941:                                              ; preds = %933, %930, %917, %916, %678
  %942 = add nsw i64 %679, 1
  %943 = icmp slt i64 %679, %677
  br i1 %943, label %678, label %944

944:                                              ; preds = %941, %669
  br label %946

945:                                              ; preds = %946
  br i1 %107, label %963, label %954

946:                                              ; preds = %946, %944
  %947 = phi i64 [ 8, %944 ], [ %952, %946 ]
  %948 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %621, i64 %947)
  %949 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %948, i64 %224)
  %950 = load i32, i32* %949, align 4
  %951 = add nsw i32 %950, 10
  store i32 %951, i32* %949, align 4
  %952 = add nuw nsw i64 %947, 1
  %953 = icmp eq i64 %952, 10
  br i1 %953, label %945, label %946

954:                                              ; preds = %954, %945
  %955 = phi i64 [ %961, %954 ], [ 1, %945 ]
  %956 = phi i64 [ %960, %954 ], [ %105, %945 ]
  %957 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %622, i64 %956)
  %958 = load i32, i32* %957, align 4
  %959 = add nsw i32 %958, 10
  store i32 %959, i32* %957, align 4
  %960 = add nsw i64 %956, 1
  %961 = add nuw nsw i64 %955, 1
  %962 = icmp slt i64 %955, %106
  br i1 %962, label %954, label %963

963:                                              ; preds = %954, %945
  switch i64 %226, label %988 [
    i64 1, label %964
    i64 2, label %980
  ]

964:                                              ; preds = %963
  br i1 %233, label %988, label %965

965:                                              ; preds = %977, %964
  %966 = phi i64 [ %978, %977 ], [ 7, %964 ]
  %967 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %621, i64 %966)
  br label %968

968:                                              ; preds = %968, %965
  %969 = phi i64 [ 1, %965 ], [ %975, %968 ]
  %970 = phi i64 [ %228, %965 ], [ %974, %968 ]
  %971 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %967, i64 %970)
  %972 = load i32, i32* %971, align 4
  %973 = add nsw i32 %972, 10
  store i32 %973, i32* %971, align 4
  %974 = add nsw i64 %970, 1
  %975 = add nuw nsw i64 %969, 1
  %976 = icmp eq i64 %975, %236
  br i1 %976, label %977, label %968

977:                                              ; preds = %968
  %978 = add nuw nsw i64 %966, 1
  %979 = icmp eq i64 %978, 10
  br i1 %979, label %988, label %965

980:                                              ; preds = %980, %963
  %981 = phi i64 [ %986, %980 ], [ 7, %963 ]
  %982 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %621, i64 %981)
  %983 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %982, i64 %228)
  %984 = load i32, i32* %983, align 4
  %985 = add nsw i32 %984, 10
  store i32 %985, i32* %983, align 4
  %986 = add nuw nsw i64 %981, 1
  %987 = icmp eq i64 %986, 10
  br i1 %987, label %988, label %980

988:                                              ; preds = %980, %977, %964, %963, %619
  %989 = add nsw i64 %620, 1
  %990 = icmp slt i64 %620, %618
  br i1 %990, label %619, label %991

991:                                              ; preds = %988, %610
  br label %993

992:                                              ; preds = %993
  br i1 %107, label %1010, label %1001

993:                                              ; preds = %993, %991
  %994 = phi i64 [ 7, %991 ], [ %999, %993 ]
  %995 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %562, i64 %994)
  %996 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %995, i64 %211)
  %997 = load i32, i32* %996, align 4
  %998 = add nsw i32 %997, 10
  store i32 %998, i32* %996, align 4
  %999 = add nuw nsw i64 %994, 1
  %1000 = icmp eq i64 %999, 10
  br i1 %1000, label %992, label %993

1001:                                             ; preds = %1001, %992
  %1002 = phi i64 [ %1008, %1001 ], [ 1, %992 ]
  %1003 = phi i64 [ %1007, %1001 ], [ %105, %992 ]
  %1004 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %563, i64 %1003)
  %1005 = load i32, i32* %1004, align 4
  %1006 = add nsw i32 %1005, 10
  store i32 %1006, i32* %1004, align 4
  %1007 = add nsw i64 %1003, 1
  %1008 = add nuw nsw i64 %1002, 1
  %1009 = icmp slt i64 %1002, %106
  br i1 %1009, label %1001, label %1010

1010:                                             ; preds = %1001, %992
  switch i64 %213, label %1035 [
    i64 1, label %1011
    i64 2, label %1027
  ]

1011:                                             ; preds = %1010
  br i1 %220, label %1035, label %1012

1012:                                             ; preds = %1024, %1011
  %1013 = phi i64 [ %1025, %1024 ], [ 4, %1011 ]
  %1014 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %562, i64 %1013)
  br label %1015

1015:                                             ; preds = %1015, %1012
  %1016 = phi i64 [ 1, %1012 ], [ %1022, %1015 ]
  %1017 = phi i64 [ %215, %1012 ], [ %1021, %1015 ]
  %1018 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %1014, i64 %1017)
  %1019 = load i32, i32* %1018, align 4
  %1020 = add nsw i32 %1019, 10
  store i32 %1020, i32* %1018, align 4
  %1021 = add nsw i64 %1017, 1
  %1022 = add nuw nsw i64 %1016, 1
  %1023 = icmp eq i64 %1022, %223
  br i1 %1023, label %1024, label %1015

1024:                                             ; preds = %1015
  %1025 = add nuw nsw i64 %1013, 1
  %1026 = icmp eq i64 %1025, 7
  br i1 %1026, label %1035, label %1012

1027:                                             ; preds = %1027, %1010
  %1028 = phi i64 [ %1033, %1027 ], [ 4, %1010 ]
  %1029 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %562, i64 %1028)
  %1030 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %1029, i64 %215)
  %1031 = load i32, i32* %1030, align 4
  %1032 = add nsw i32 %1031, 10
  store i32 %1032, i32* %1030, align 4
  %1033 = add nuw nsw i64 %1028, 1
  %1034 = icmp eq i64 %1033, 7
  br i1 %1034, label %1035, label %1027

1035:                                             ; preds = %1027, %1024, %1011, %1010, %560
  %1036 = add nsw i64 %561, 1
  %1037 = icmp slt i64 %561, %559
  br i1 %1037, label %560, label %1038

1038:                                             ; preds = %1035, %551
  br label %1040

1039:                                             ; preds = %1040
  br i1 %107, label %1057, label %1048

1040:                                             ; preds = %1040, %1038
  %1041 = phi i64 [ 6, %1038 ], [ %1046, %1040 ]
  %1042 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %503, i64 %1041)
  %1043 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %1042, i64 %198)
  %1044 = load i32, i32* %1043, align 4
  %1045 = add nsw i32 %1044, 10
  store i32 %1045, i32* %1043, align 4
  %1046 = add nuw nsw i64 %1041, 1
  %1047 = icmp eq i64 %1046, 10
  br i1 %1047, label %1039, label %1040

1048:                                             ; preds = %1048, %1039
  %1049 = phi i64 [ %1055, %1048 ], [ 1, %1039 ]
  %1050 = phi i64 [ %1054, %1048 ], [ %105, %1039 ]
  %1051 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %504, i64 %1050)
  %1052 = load i32, i32* %1051, align 4
  %1053 = add nsw i32 %1052, 10
  store i32 %1053, i32* %1051, align 4
  %1054 = add nsw i64 %1050, 1
  %1055 = add nuw nsw i64 %1049, 1
  %1056 = icmp slt i64 %1049, %106
  br i1 %1056, label %1048, label %1057

1057:                                             ; preds = %1048, %1039
  switch i64 %200, label %1082 [
    i64 1, label %1058
    i64 2, label %1074
  ]

1058:                                             ; preds = %1057
  br i1 %207, label %1082, label %1059

1059:                                             ; preds = %1071, %1058
  %1060 = phi i64 [ %1072, %1071 ], [ 4, %1058 ]
  %1061 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %503, i64 %1060)
  br label %1062

1062:                                             ; preds = %1062, %1059
  %1063 = phi i64 [ 1, %1059 ], [ %1069, %1062 ]
  %1064 = phi i64 [ %202, %1059 ], [ %1068, %1062 ]
  %1065 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %1061, i64 %1064)
  %1066 = load i32, i32* %1065, align 4
  %1067 = add nsw i32 %1066, 10
  store i32 %1067, i32* %1065, align 4
  %1068 = add nsw i64 %1064, 1
  %1069 = add nuw nsw i64 %1063, 1
  %1070 = icmp eq i64 %1069, %210
  br i1 %1070, label %1071, label %1062

1071:                                             ; preds = %1062
  %1072 = add nuw nsw i64 %1060, 1
  %1073 = icmp eq i64 %1072, 7
  br i1 %1073, label %1082, label %1059

1074:                                             ; preds = %1074, %1057
  %1075 = phi i64 [ %1080, %1074 ], [ 4, %1057 ]
  %1076 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %503, i64 %1075)
  %1077 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %1076, i64 %202)
  %1078 = load i32, i32* %1077, align 4
  %1079 = add nsw i32 %1078, 10
  store i32 %1079, i32* %1077, align 4
  %1080 = add nuw nsw i64 %1075, 1
  %1081 = icmp eq i64 %1080, 7
  br i1 %1081, label %1082, label %1074

1082:                                             ; preds = %1074, %1071, %1058, %1057, %501
  %1083 = add nsw i64 %502, 1
  %1084 = icmp slt i64 %502, %500
  br i1 %1084, label %501, label %1085

1085:                                             ; preds = %1082, %492
  br label %1087

1086:                                             ; preds = %1087
  br i1 %107, label %1104, label %1095

1087:                                             ; preds = %1087, %1085
  %1088 = phi i64 [ 5, %1085 ], [ %1093, %1087 ]
  %1089 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %444, i64 %1088)
  %1090 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %1089, i64 %185)
  %1091 = load i32, i32* %1090, align 4
  %1092 = add nsw i32 %1091, 10
  store i32 %1092, i32* %1090, align 4
  %1093 = add nuw nsw i64 %1088, 1
  %1094 = icmp eq i64 %1093, 10
  br i1 %1094, label %1086, label %1087

1095:                                             ; preds = %1095, %1086
  %1096 = phi i64 [ %1102, %1095 ], [ 1, %1086 ]
  %1097 = phi i64 [ %1101, %1095 ], [ %105, %1086 ]
  %1098 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %445, i64 %1097)
  %1099 = load i32, i32* %1098, align 4
  %1100 = add nsw i32 %1099, 10
  store i32 %1100, i32* %1098, align 4
  %1101 = add nsw i64 %1097, 1
  %1102 = add nuw nsw i64 %1096, 1
  %1103 = icmp slt i64 %1096, %106
  br i1 %1103, label %1095, label %1104

1104:                                             ; preds = %1095, %1086
  switch i64 %187, label %1129 [
    i64 1, label %1105
    i64 2, label %1121
  ]

1105:                                             ; preds = %1104
  br i1 %194, label %1129, label %1106

1106:                                             ; preds = %1118, %1105
  %1107 = phi i64 [ %1119, %1118 ], [ 4, %1105 ]
  %1108 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %444, i64 %1107)
  br label %1109

1109:                                             ; preds = %1109, %1106
  %1110 = phi i64 [ 1, %1106 ], [ %1116, %1109 ]
  %1111 = phi i64 [ %189, %1106 ], [ %1115, %1109 ]
  %1112 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %1108, i64 %1111)
  %1113 = load i32, i32* %1112, align 4
  %1114 = add nsw i32 %1113, 10
  store i32 %1114, i32* %1112, align 4
  %1115 = add nsw i64 %1111, 1
  %1116 = add nuw nsw i64 %1110, 1
  %1117 = icmp eq i64 %1116, %197
  br i1 %1117, label %1118, label %1109

1118:                                             ; preds = %1109
  %1119 = add nuw nsw i64 %1107, 1
  %1120 = icmp eq i64 %1119, 7
  br i1 %1120, label %1129, label %1106

1121:                                             ; preds = %1121, %1104
  %1122 = phi i64 [ %1127, %1121 ], [ 4, %1104 ]
  %1123 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %444, i64 %1122)
  %1124 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %1123, i64 %189)
  %1125 = load i32, i32* %1124, align 4
  %1126 = add nsw i32 %1125, 10
  store i32 %1126, i32* %1124, align 4
  %1127 = add nuw nsw i64 %1122, 1
  %1128 = icmp eq i64 %1127, 7
  br i1 %1128, label %1129, label %1121

1129:                                             ; preds = %1121, %1118, %1105, %1104, %442
  %1130 = add nsw i64 %443, 1
  %1131 = icmp slt i64 %443, %441
  br i1 %1131, label %442, label %1132

1132:                                             ; preds = %1129, %433
  br label %1134

1133:                                             ; preds = %1134
  br i1 %107, label %1151, label %1142

1134:                                             ; preds = %1134, %1132
  %1135 = phi i64 [ 4, %1132 ], [ %1140, %1134 ]
  %1136 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %385, i64 %1135)
  %1137 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %1136, i64 %172)
  %1138 = load i32, i32* %1137, align 4
  %1139 = add nsw i32 %1138, 10
  store i32 %1139, i32* %1137, align 4
  %1140 = add nuw nsw i64 %1135, 1
  %1141 = icmp eq i64 %1140, 10
  br i1 %1141, label %1133, label %1134

1142:                                             ; preds = %1142, %1133
  %1143 = phi i64 [ %1149, %1142 ], [ 1, %1133 ]
  %1144 = phi i64 [ %1148, %1142 ], [ %105, %1133 ]
  %1145 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %386, i64 %1144)
  %1146 = load i32, i32* %1145, align 4
  %1147 = add nsw i32 %1146, 10
  store i32 %1147, i32* %1145, align 4
  %1148 = add nsw i64 %1144, 1
  %1149 = add nuw nsw i64 %1143, 1
  %1150 = icmp slt i64 %1143, %106
  br i1 %1150, label %1142, label %1151

1151:                                             ; preds = %1142, %1133
  switch i64 %174, label %1176 [
    i64 1, label %1152
    i64 2, label %1168
  ]

1152:                                             ; preds = %1151
  br i1 %181, label %1176, label %1153

1153:                                             ; preds = %1165, %1152
  %1154 = phi i64 [ %1166, %1165 ], [ 1, %1152 ]
  %1155 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %385, i64 %1154)
  br label %1156

1156:                                             ; preds = %1156, %1153
  %1157 = phi i64 [ 1, %1153 ], [ %1163, %1156 ]
  %1158 = phi i64 [ %176, %1153 ], [ %1162, %1156 ]
  %1159 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %1155, i64 %1158)
  %1160 = load i32, i32* %1159, align 4
  %1161 = add nsw i32 %1160, 10
  store i32 %1161, i32* %1159, align 4
  %1162 = add nsw i64 %1158, 1
  %1163 = add nuw nsw i64 %1157, 1
  %1164 = icmp eq i64 %1163, %184
  br i1 %1164, label %1165, label %1156

1165:                                             ; preds = %1156
  %1166 = add nuw nsw i64 %1154, 1
  %1167 = icmp eq i64 %1166, 4
  br i1 %1167, label %1176, label %1153

1168:                                             ; preds = %1168, %1151
  %1169 = phi i64 [ %1174, %1168 ], [ 1, %1151 ]
  %1170 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %385, i64 %1169)
  %1171 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %1170, i64 %176)
  %1172 = load i32, i32* %1171, align 4
  %1173 = add nsw i32 %1172, 10
  store i32 %1173, i32* %1171, align 4
  %1174 = add nuw nsw i64 %1169, 1
  %1175 = icmp eq i64 %1174, 4
  br i1 %1175, label %1176, label %1168

1176:                                             ; preds = %1168, %1165, %1152, %1151, %383
  %1177 = add nsw i64 %384, 1
  %1178 = icmp slt i64 %384, %382
  br i1 %1178, label %383, label %1179

1179:                                             ; preds = %1176, %374
  br label %1181

1180:                                             ; preds = %1181
  br i1 %107, label %1198, label %1189

1181:                                             ; preds = %1181, %1179
  %1182 = phi i64 [ 3, %1179 ], [ %1187, %1181 ]
  %1183 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %326, i64 %1182)
  %1184 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %1183, i64 %159)
  %1185 = load i32, i32* %1184, align 4
  %1186 = add nsw i32 %1185, 10
  store i32 %1186, i32* %1184, align 4
  %1187 = add nuw nsw i64 %1182, 1
  %1188 = icmp eq i64 %1187, 10
  br i1 %1188, label %1180, label %1181

1189:                                             ; preds = %1189, %1180
  %1190 = phi i64 [ %1196, %1189 ], [ 1, %1180 ]
  %1191 = phi i64 [ %1195, %1189 ], [ %105, %1180 ]
  %1192 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %327, i64 %1191)
  %1193 = load i32, i32* %1192, align 4
  %1194 = add nsw i32 %1193, 10
  store i32 %1194, i32* %1192, align 4
  %1195 = add nsw i64 %1191, 1
  %1196 = add nuw nsw i64 %1190, 1
  %1197 = icmp slt i64 %1190, %106
  br i1 %1197, label %1189, label %1198

1198:                                             ; preds = %1189, %1180
  switch i64 %161, label %1223 [
    i64 1, label %1199
    i64 2, label %1215
  ]

1199:                                             ; preds = %1198
  br i1 %168, label %1223, label %1200

1200:                                             ; preds = %1212, %1199
  %1201 = phi i64 [ %1213, %1212 ], [ 1, %1199 ]
  %1202 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %326, i64 %1201)
  br label %1203

1203:                                             ; preds = %1203, %1200
  %1204 = phi i64 [ %163, %1200 ], [ %1209, %1203 ]
  %1205 = phi i64 [ 1, %1200 ], [ %1210, %1203 ]
  %1206 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %1202, i64 %1204)
  %1207 = load i32, i32* %1206, align 4
  %1208 = add nsw i32 %1207, 10
  store i32 %1208, i32* %1206, align 4
  %1209 = add nsw i64 %1204, 1
  %1210 = add nuw nsw i64 %1205, 1
  %1211 = icmp eq i64 %1210, %171
  br i1 %1211, label %1212, label %1203

1212:                                             ; preds = %1203
  %1213 = add nuw nsw i64 %1201, 1
  %1214 = icmp eq i64 %1213, 4
  br i1 %1214, label %1223, label %1200

1215:                                             ; preds = %1215, %1198
  %1216 = phi i64 [ %1221, %1215 ], [ 1, %1198 ]
  %1217 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %326, i64 %1216)
  %1218 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %1217, i64 %163)
  %1219 = load i32, i32* %1218, align 4
  %1220 = add nsw i32 %1219, 10
  store i32 %1220, i32* %1218, align 4
  %1221 = add nuw nsw i64 %1216, 1
  %1222 = icmp eq i64 %1221, 4
  br i1 %1222, label %1223, label %1215

1223:                                             ; preds = %1215, %1212, %1199, %1198, %324
  %1224 = add nsw i64 %325, 1
  %1225 = icmp slt i64 %325, %323
  br i1 %1225, label %324, label %1226

1226:                                             ; preds = %1223, %315
  br label %1228

1227:                                             ; preds = %1228
  br i1 %107, label %1245, label %1236

1228:                                             ; preds = %1228, %1226
  %1229 = phi i64 [ 2, %1226 ], [ %1234, %1228 ]
  %1230 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %267, i64 %1229)
  %1231 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %1230, i64 %20)
  %1232 = load i32, i32* %1231, align 4
  %1233 = add nsw i32 %1232, 10
  store i32 %1233, i32* %1231, align 4
  %1234 = add nuw nsw i64 %1229, 1
  %1235 = icmp eq i64 %1234, 10
  br i1 %1235, label %1227, label %1228

1236:                                             ; preds = %1236, %1227
  %1237 = phi i64 [ %1242, %1236 ], [ %105, %1227 ]
  %1238 = phi i64 [ %1243, %1236 ], [ 1, %1227 ]
  %1239 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %268, i64 %1237)
  %1240 = load i32, i32* %1239, align 4
  %1241 = add nsw i32 %1240, 10
  store i32 %1241, i32* %1239, align 4
  %1242 = add nsw i64 %1237, 1
  %1243 = add nuw nsw i64 %1238, 1
  %1244 = icmp slt i64 %1238, %106
  br i1 %1244, label %1236, label %1245

1245:                                             ; preds = %1236, %1227
  switch i64 %109, label %1270 [
    i64 1, label %1246
    i64 2, label %1262
  ]

1246:                                             ; preds = %1245
  br i1 %116, label %1270, label %1247

1247:                                             ; preds = %1259, %1246
  %1248 = phi i64 [ %1260, %1259 ], [ 1, %1246 ]
  %1249 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %267, i64 %1248)
  br label %1250

1250:                                             ; preds = %1250, %1247
  %1251 = phi i64 [ %111, %1247 ], [ %1256, %1250 ]
  %1252 = phi i64 [ 1, %1247 ], [ %1257, %1250 ]
  %1253 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %1249, i64 %1251)
  %1254 = load i32, i32* %1253, align 4
  %1255 = add nsw i32 %1254, 10
  store i32 %1255, i32* %1253, align 4
  %1256 = add nsw i64 %1251, 1
  %1257 = add nuw nsw i64 %1252, 1
  %1258 = icmp eq i64 %1257, %156
  br i1 %1258, label %1259, label %1250

1259:                                             ; preds = %1250
  %1260 = add nuw nsw i64 %1248, 1
  %1261 = icmp eq i64 %1260, 4
  br i1 %1261, label %1270, label %1247

1262:                                             ; preds = %1262, %1245
  %1263 = phi i64 [ %1268, %1262 ], [ 1, %1245 ]
  %1264 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* nonnull %267, i64 %1263)
  %1265 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %1264, i64 %111)
  %1266 = load i32, i32* %1265, align 4
  %1267 = add nsw i32 %1266, 10
  store i32 %1267, i32* %1265, align 4
  %1268 = add nuw nsw i64 %1263, 1
  %1269 = icmp eq i64 %1268, 4
  br i1 %1269, label %1270, label %1262

1270:                                             ; preds = %1262, %1259, %1246, %1245, %265
  %1271 = add nsw i64 %266, 1
  %1272 = icmp slt i64 %266, %158
  br i1 %1272, label %265, label %1273

1273:                                             ; preds = %1270, %866, %94
  ret void
}

attributes #0 = { nounwind "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="cannonlake" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

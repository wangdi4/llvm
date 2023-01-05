; REQUIRES: asserts
; RUN: opt < %s -dva-check-dtrans-outofboundsok -dtrans-outofboundsok=true -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck %s

; Check that with -dtrans-outofboundsok=true, we get no transpose candidates
; as our analysis is too weak at this point.

; CHECK: No transpose candidates

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" = type { %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"PHYSPROPMOD$.btPHYSPROP_TYPE" = type { i32, %"QNCA_a0$float*$rank2$" }
%"QNCA_a0$float*$rank2$" = type { float*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@physpropmod_mp_physprop_ = internal global %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" { %"PHYSPROPMOD$.btPHYSPROP_TYPE"* null, i64 0, i64 0, i64 0, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@anon.263b53731fe38c4199c4a10e662745ed.0 = internal unnamed_addr constant i32 2

; Function Attrs: nofree noinline nounwind uwtable
define internal void @myinit_(i32* noalias nocapture readonly dereferenceable(4) %0, i32* noalias nocapture readonly dereferenceable(4) %1) #0 {
  %3 = alloca [8 x i64], align 16
  %4 = alloca [4 x i8], align 1
  %5 = alloca { i64, i8* }, align 8
  %6 = load i32, i32* %1, align 1
  %7 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  %8 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %9 = load i32, i32* %0, align 1
  %10 = sext i32 %9 to i64
  br label %11

11:                                               ; preds = %41, %2
  %12 = phi i64 [ %42, %41 ], [ 1, %2 ]
  br label %13

13:                                               ; preds = %13, %11
  %14 = phi i64 [ %39, %13 ], [ 1, %11 ]
  %15 = add nuw nsw i64 %12, %14
  %16 = trunc i64 %15 to i32
  %17 = add nsw i32 %6, %16
  %18 = sitofp i32 %17 to float
  %19 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %20 = load i64, i64* %7, align 1
  %21 = load i64, i64* %8, align 1
  %22 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %21, i64 %20, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %19, i64 %10)
  %23 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %22, i64 0, i32 1
  %24 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %23, i64 0, i32 0
  %25 = load float*, float** %24, align 1
  %26 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %23, i64 0, i32 6, i64 0
  %27 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %26, i64 0, i32 1
  %28 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %27, i32 0)
  %29 = load i64, i64* %28, align 1
  %30 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %26, i64 0, i32 2
  %31 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %30, i32 0)
  %32 = load i64, i64* %31, align 1
  %33 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %27, i32 1)
  %34 = load i64, i64* %33, align 1
  %35 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %30, i32 1)
  %36 = load i64, i64* %35, align 1
  %37 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %36, i64 %34, float* elementtype(float) %25, i64 %14)
  %38 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %32, i64 %29, float* elementtype(float) %37, i64 %12)
  store float %18, float* %38, align 1
  %39 = add nuw nsw i64 %14, 1
  %40 = icmp eq i64 %39, 11
  br i1 %40, label %41, label %13

41:                                               ; preds = %13
  %42 = add nuw nsw i64 %12, 1
  %43 = icmp eq i64 %42, 11
  br i1 %43, label %44, label %11

44:                                               ; preds = %41
  %45 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %46 = load i64, i64* %7, align 1
  %47 = load i64, i64* %8, align 1
  %48 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %47, i64 %46, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %45, i64 %10)
  %49 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %48, i64 0, i32 1
  %50 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %49, i64 0, i32 3
  %51 = load i64, i64* %50, align 1
  %52 = and i64 %51, 4
  %53 = icmp ne i64 %52, 0
  %54 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %49, i64 0, i32 6, i64 0
  %55 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %54, i64 0, i32 1
  %56 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %55, i32 0)
  %57 = load i64, i64* %56, align 1, !range !3
  %58 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %49, i64 0, i32 1
  %59 = load i64, i64* %58, align 1
  %60 = icmp eq i64 %57, %59
  %61 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %55, i32 1)
  %62 = load i64, i64* %61, align 1, !range !3
  %63 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %54, i64 0, i32 0
  %64 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %63, i32 0)
  %65 = load i64, i64* %64, align 1
  %66 = mul nsw i64 %65, %57
  %67 = icmp eq i64 %62, %66
  %68 = and i1 %60, %67
  %69 = or i1 %53, %68
  %70 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %49, i64 0, i32 0
  %71 = load float*, float** %70, align 1
  %72 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %54, i64 0, i32 2
  %73 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %72, i32 0)
  %74 = load i64, i64* %73, align 1
  %75 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %72, i32 1)
  %76 = load i64, i64* %75, align 1
  %77 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %63, i32 1)
  %78 = load i64, i64* %77, align 1
  %79 = shl nsw i64 %65, 2
  br i1 %69, label %80, label %82

80:                                               ; preds = %44
  %81 = mul nsw i64 %78, %79
  br label %110

82:                                               ; preds = %44
  %83 = mul nsw i64 %78, %79
  %84 = sdiv i64 %83, 4
  %85 = alloca float, i64 %84, align 4
  %86 = icmp slt i64 %78, 1
  br i1 %86, label %110, label %87

87:                                               ; preds = %82
  %88 = icmp slt i64 %65, 1
  %89 = add nsw i64 %65, 1
  %90 = add nuw nsw i64 %78, 1
  br label %104

91:                                               ; preds = %107, %91
  %92 = phi i64 [ %74, %107 ], [ %97, %91 ]
  %93 = phi i64 [ 1, %107 ], [ %98, %91 ]
  %94 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %74, i64 %57, float* elementtype(float) %108, i64 %92)
  %95 = load float, float* %94, align 1
  %96 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %109, i64 %93)
  store float %95, float* %96, align 1
  %97 = add nsw i64 %92, 1
  %98 = add nuw nsw i64 %93, 1
  %99 = icmp eq i64 %98, %89
  br i1 %99, label %100, label %91

100:                                              ; preds = %104, %91
  %101 = add nsw i64 %105, 1
  %102 = add nuw nsw i64 %106, 1
  %103 = icmp eq i64 %102, %90
  br i1 %103, label %110, label %104

104:                                              ; preds = %100, %87
  %105 = phi i64 [ %76, %87 ], [ %101, %100 ]
  %106 = phi i64 [ 1, %87 ], [ %102, %100 ]
  br i1 %88, label %100, label %107

107:                                              ; preds = %104
  %108 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %76, i64 %62, float* elementtype(float) %71, i64 %105)
  %109 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %79, float* elementtype(float) nonnull %85, i64 %106)
  br label %91

110:                                              ; preds = %100, %82, %80
  %111 = phi i64 [ %81, %80 ], [ %83, %82 ], [ %83, %100 ]
  %112 = phi float* [ %71, %80 ], [ %85, %82 ], [ %85, %100 ]
  %113 = bitcast float* %112 to i8*
  %114 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 0
  store i8 26, i8* %114, align 1
  %115 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 1
  store i8 5, i8* %115, align 1
  %116 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 2
  store i8 1, i8* %116, align 1
  %117 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 3
  store i8 0, i8* %117, align 1
  %118 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %5, i64 0, i32 0
  store i64 %111, i64* %118, align 8
  %119 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %5, i64 0, i32 1
  store i8* %113, i8** %119, align 8
  %120 = bitcast [8 x i64]* %3 to i8*
  %121 = bitcast { i64, i8* }* %5 to i8*
  %122 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %120, i32 -1, i64 1239157112576, i8* nonnull %114, i8* nonnull %121) #4
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 %0, i64 %1, i32 %2, i64* elementtype(i64) %3, i32 %4) #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 %0, i64 %1, i64 %2, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %3, i64 %4) #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 %0, i64 %1, i64 %2, float* elementtype(float) %3, i64 %4) #1

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(i8* %0, i32 %1, i64 %2, i8* %3, i8* %4, ...) local_unnamed_addr #2

; Function Attrs: nofree noreturn nounwind uwtable
define dso_local void @MAIN__() #3 {
  %1 = alloca i32, align 8
  %2 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.263b53731fe38c4199c4a10e662745ed.0) #4
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 5), align 8
  store i64 104, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 1), align 8
  store i64 1, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 4), align 8
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 2), align 8
  %3 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, i64* %3, align 1
  %4 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 10, i64* %4, align 1
  %5 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 104, i64* %5, align 1
  store i64 1073741829, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  %6 = tail call i32 @for_allocate_handle(i64 1040, i8** bitcast (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_ to i8**), i32 262144, i8* null) #4
  store i32 1, i32* %1, align 8
  br label %7

7:                                                ; preds = %7, %0
  %8 = phi i32 [ %52, %7 ], [ 1, %0 ]
  %9 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %10 = load i64, i64* %5, align 1
  %11 = load i64, i64* %3, align 1
  %12 = sext i32 %8 to i64
  %13 = call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %11, i64 %10, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %9, i64 %12)
  %14 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %13, i64 0, i32 0
  store i32 50, i32* %14, align 1
  %15 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %16 = load i64, i64* %5, align 1
  %17 = load i64, i64* %3, align 1
  %18 = load i32, i32* %1, align 8
  %19 = sext i32 %18 to i64
  %20 = call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %17, i64 %16, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %15, i64 %19)
  %21 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %20, i64 0, i32 1
  %22 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %21, i64 0, i32 3
  store i64 5, i64* %22, align 1
  %23 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %21, i64 0, i32 5
  store i64 0, i64* %23, align 1
  %24 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %21, i64 0, i32 1
  store i64 4, i64* %24, align 1
  %25 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %21, i64 0, i32 4
  store i64 2, i64* %25, align 1
  %26 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %21, i64 0, i32 2
  store i64 0, i64* %26, align 1
  %27 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %21, i64 0, i32 6, i64 0
  %28 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %27, i64 0, i32 2
  %29 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %28, i32 0)
  store i64 1, i64* %29, align 1
  %30 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %27, i64 0, i32 0
  %31 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %30, i32 0)
  store i64 10, i64* %31, align 1
  %32 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %28, i32 1)
  store i64 1, i64* %32, align 1
  %33 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %30, i32 1)
  store i64 10, i64* %33, align 1
  %34 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %27, i64 0, i32 1
  %35 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %34, i32 0)
  store i64 4, i64* %35, align 1
  %36 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %34, i32 1)
  store i64 40, i64* %36, align 1
  %37 = load i64, i64* %22, align 1
  %38 = and i64 %37, -68451041281
  %39 = or i64 %38, 1073741824
  store i64 %39, i64* %22, align 1
  %40 = load i64, i64* %23, align 1
  %41 = inttoptr i64 %40 to i8*
  %42 = bitcast %"QNCA_a0$float*$rank2$"* %21 to i8**
  %43 = call i32 @for_allocate_handle(i64 400, i8** nonnull %42, i32 262144, i8* %41) #4
  %44 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %45 = load i64, i64* %5, align 1
  %46 = load i64, i64* %3, align 1
  %47 = load i32, i32* %1, align 8
  %48 = sext i32 %47 to i64
  %49 = call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %46, i64 %45, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %44, i64 %48)
  %50 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %49, i64 0, i32 0
  call void @myinit_(i32* nonnull %1, i32* %50) #4
  %51 = load i32, i32* %1, align 8
  %52 = add nsw i32 %51, 1
  store i32 %52, i32* %1, align 8
  br label %7
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(i32* nocapture readonly %0) local_unnamed_addr #2

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64 %0, i8** nocapture %1, i32 %2, i8* %3) local_unnamed_addr #2

attributes #0 = { nofree noinline nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree nosync nounwind readnone speculatable }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { nofree noreturn nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #4 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
!3 = !{i64 1, i64 -9223372036854775808}

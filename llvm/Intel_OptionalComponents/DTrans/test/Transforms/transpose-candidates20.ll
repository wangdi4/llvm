; REQUIRES: asserts
; RUN: opt < %s -dva-check-dtrans-outofboundsok -dtrans-outofboundsok=false -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck %s

; Check that field 1 of physpropmod_mp_physprop_ is determined to be a valid candidate for transpose
; even though field 0 contains a character string which is passed down to another subroutine and
; concatenated with another string and then passed down to another function.

; CHECK: Transpose candidate: physpropmod_mp_physprop_
; CHECK: Nested Field Number : 1
; CHECK: IsValid{{ *}}: true
; CHECK: IsProfitable{{ *}}: false

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" = type { %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64}] }
%"PHYSPROPMOD$.btPHYSPROP_TYPE" = type { [256 x i8], %"QNCA_a0$float*$rank2$" }
%"QNCA_a0$float*$rank2$" = type { float*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@strlit = internal unnamed_addr constant [5 x i8] c"hello"
@physpropmod_mp_physprop_ = internal global %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" { %"PHYSPROPMOD$.btPHYSPROP_TYPE"* null, i64 0, i64 0, i64 0, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@anon.dd98b7be2d4a48a7cca189d7201608bc.0 = internal unnamed_addr constant i32 2
@strlit.1 = internal unnamed_addr constant [13 x i8] c" and goodbye "

; Function Attrs: nofree noinline nounwind uwtable
define internal void @myinit_(i32* noalias nocapture readonly dereferenceable(4) %0) #0 {
  %2 = alloca [8 x i64], align 16
  %3 = alloca [4 x i8], align 1
  %4 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  %5 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %6 = load i32, i32* %0, align 1
  %7 = sext i32 %6 to i64
  br label %8

8:                                                ; preds = %37, %1
  %9 = phi i64 [ %38, %37 ], [ 1, %1 ]
  br label %10

10:                                               ; preds = %10, %8
  %11 = phi i64 [ %35, %10 ], [ 1, %8 ]
  %12 = add nuw nsw i64 %11, %9
  %13 = trunc i64 %12 to i32
  %14 = sitofp i32 %13 to float
  %15 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %16 = load i64, i64* %4, align 1
  %17 = load i64, i64* %5, align 1
  %18 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %17, i64 %16, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %15, i64 %7)
  %19 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %18, i64 0, i32 1
  %20 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %19, i64 0, i32 0
  %21 = load float*, float** %20, align 1
  %22 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %19, i64 0, i32 6, i64 0
  %23 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %22, i64 0, i32 1
  %24 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %23, i32 0)
  %25 = load i64, i64* %24, align 1
  %26 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %22, i64 0, i32 2
  %27 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %26, i32 0)
  %28 = load i64, i64* %27, align 1
  %29 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %23, i32 1)
  %30 = load i64, i64* %29, align 1
  %31 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %26, i32 1)
  %32 = load i64, i64* %31, align 1
  %33 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %32, i64 %30, float* elementtype(float) %21, i64 %11)
  %34 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %28, i64 %25, float* elementtype(float) %33, i64 %9)
  store float %14, float* %34, align 1
  %35 = add nuw nsw i64 %11, 1
  %36 = icmp eq i64 %35, 11
  br i1 %36, label %37, label %10

37:                                               ; preds = %10
  %38 = add nuw nsw i64 %9, 1
  %39 = icmp eq i64 %38, 11
  br i1 %39, label %40, label %8

40:                                               ; preds = %37
  %41 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %42 = load i64, i64* %4, align 1
  %43 = load i64, i64* %5, align 1
  %44 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %43, i64 %42, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %41, i64 %7)
  %45 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %44, i64 0, i32 1
  %46 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %45, i64 0, i32 3
  %47 = load i64, i64* %46, align 1
  %48 = and i64 %47, 4
  %49 = icmp ne i64 %48, 0
  %50 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %45, i64 0, i32 6, i64 0
  %51 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %50, i64 0, i32 1
  %52 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %51, i32 0)
  %53 = load i64, i64* %52, align 1, !range !3
  %54 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %45, i64 0, i32 1
  %55 = load i64, i64* %54, align 1
  %56 = icmp eq i64 %53, %55
  %57 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %51, i32 1)
  %58 = load i64, i64* %57, align 1, !range !3
  %59 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %50, i64 0, i32 0
  %60 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %59, i32 0)
  %61 = load i64, i64* %60, align 1
  %62 = mul nsw i64 %61, %53
  %63 = icmp eq i64 %58, %62
  %64 = and i1 %56, %63
  %65 = or i1 %49, %64
  %66 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %45, i64 0, i32 0
  %67 = load float*, float** %66, align 1
  %68 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %50, i64 0, i32 2
  %69 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %68, i32 0)
  %70 = load i64, i64* %69, align 1
  %71 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %68, i32 1)
  %72 = load i64, i64* %71, align 1
  %73 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %59, i32 1)
  %74 = load i64, i64* %73, align 1
  %75 = shl nsw i64 %61, 2
  br i1 %65, label %76, label %78

76:                                               ; preds = %40
  %77 = mul nsw i64 %74, %75
  br label %106

78:                                               ; preds = %40
  %79 = mul nsw i64 %74, %75
  %80 = sdiv i64 %79, 4
  %81 = alloca float, i64 %80, align 4
  %82 = icmp slt i64 %74, 1
  br i1 %82, label %106, label %83

83:                                               ; preds = %78
  %84 = icmp slt i64 %61, 1
  %85 = add nsw i64 %61, 1
  %86 = add nuw nsw i64 %74, 1
  br label %100

87:                                               ; preds = %103, %87
  %88 = phi i64 [ %70, %103 ], [ %93, %87 ]
  %89 = phi i64 [ 1, %103 ], [ %94, %87 ]
  %90 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %70, i64 %53, float* elementtype(float) %104, i64 %88)
  %91 = load float, float* %90, align 1
  %92 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %105, i64 %89)
  store float %91, float* %92, align 1
  %93 = add nsw i64 %88, 1
  %94 = add nuw nsw i64 %89, 1
  %95 = icmp eq i64 %94, %85
  br i1 %95, label %96, label %87

96:                                               ; preds = %100, %87
  %97 = add nsw i64 %101, 1
  %98 = add nuw nsw i64 %102, 1
  %99 = icmp eq i64 %98, %86
  br i1 %99, label %106, label %100

100:                                              ; preds = %96, %83
  %101 = phi i64 [ %72, %83 ], [ %97, %96 ]
  %102 = phi i64 [ 1, %83 ], [ %98, %96 ]
  br i1 %84, label %96, label %103

103:                                              ; preds = %100
  %104 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %72, i64 %58, float* elementtype(float) %67, i64 %101)
  %105 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %75, float* elementtype(float) nonnull %81, i64 %102)
  br label %87

106:                                              ; preds = %96, %78, %76
  %107 = phi i64 [ %77, %76 ], [ %79, %78 ], [ %79, %96 ]
  %108 = phi float* [ %67, %76 ], [ %81, %78 ], [ %81, %96 ]
  %109 = bitcast float* %108 to i8*
  %110 = getelementptr inbounds [4 x i8], [4 x i8]* %3, i64 0, i64 0
  store i8 26, i8* %110, align 1
  %111 = getelementptr inbounds [4 x i8], [4 x i8]* %3, i64 0, i64 1
  store i8 5, i8* %111, align 1
  %112 = getelementptr inbounds [4 x i8], [4 x i8]* %3, i64 0, i64 2
  store i8 1, i8* %112, align 1
  %113 = getelementptr inbounds [4 x i8], [4 x i8]* %3, i64 0, i64 3
  store i8 0, i8* %113, align 1
  %114 = alloca { i64, i8* }, align 8
  %115 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %114, i64 0, i32 0
  store i64 %107, i64* %115, align 8
  %116 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %114, i64 0, i32 1
  store i8* %109, i8** %116, align 8
  %117 = bitcast [8 x i64]* %2 to i8*
  %118 = bitcast { i64, i8* }* %114 to i8*
  %119 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %117, i32 -1, i64 1239157112576, i8* nonnull %110, i8* nonnull %118) #6
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
  %2 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.dd98b7be2d4a48a7cca189d7201608bc.0) #6
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 5), align 8
  store i64 352, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 1), align 8
  store i64 1, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 4), align 8
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 2), align 8
  %3 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, i64* %3, align 1
  %4 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 10, i64* %4, align 1
  %5 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 352, i64* %5, align 1
  store i64 1073741829, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  %6 = tail call i32 @for_allocate_handle(i64 3520, i8** bitcast (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_ to i8**), i32 262144, i8* null) #6
  store i32 1, i32* %1, align 8
  br label %7

7:                                                ; preds = %7, %0
  %8 = phi i32 [ %53, %7 ], [ 1, %0 ]
  %9 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %10 = load i64, i64* %5, align 1
  %11 = load i64, i64* %3, align 1
  %12 = sext i32 %8 to i64
  %13 = call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %11, i64 %10, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %9, i64 %12)
  %14 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %13, i64 0, i32 0
  %15 = getelementptr [256 x i8], [256 x i8]* %14, i64 0, i64 0
  %16 = getelementptr i8, i8* %15, i64 5
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* noundef nonnull align 1 dereferenceable(5) %15, i8* noundef nonnull align 1 dereferenceable(5) getelementptr inbounds ([5 x i8], [5 x i8]* @strlit, i64 0, i64 0), i64 5, i1 false)
  call void @llvm.memset.p0i8.i64(i8* noundef nonnull align 1 dereferenceable(251) %16, i8 32, i64 251, i1 false)
  %17 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %18 = load i64, i64* %5, align 1
  %19 = load i64, i64* %3, align 1
  %20 = load i32, i32* %1, align 8
  %21 = sext i32 %20 to i64
  %22 = call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %19, i64 %18, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %17, i64 %21)
  %23 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %22, i64 0, i32 1
  %24 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %23, i64 0, i32 3
  store i64 5, i64* %24, align 1
  %25 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %23, i64 0, i32 5
  store i64 0, i64* %25, align 1
  %26 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %23, i64 0, i32 1
  store i64 4, i64* %26, align 1
  %27 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %23, i64 0, i32 4
  store i64 2, i64* %27, align 1
  %28 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %23, i64 0, i32 2
  store i64 0, i64* %28, align 1
  %29 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %23, i64 0, i32 6, i64 0
  %30 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %29, i64 0, i32 2
  %31 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %30, i32 0)
  store i64 1, i64* %31, align 1
  %32 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %29, i64 0, i32 0
  %33 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %32, i32 0)
  store i64 10, i64* %33, align 1
  %34 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %30, i32 1)
  store i64 1, i64* %34, align 1
  %35 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %32, i32 1)
  store i64 10, i64* %35, align 1
  %36 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %29, i64 0, i32 1
  %37 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %36, i32 0)
  store i64 4, i64* %37, align 1
  %38 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %36, i32 1)
  store i64 40, i64* %38, align 1
  %39 = load i64, i64* %24, align 1
  %40 = and i64 %39, -68451041281
  %41 = or i64 %40, 1073741824
  store i64 %41, i64* %24, align 1
  %42 = load i64, i64* %25, align 1
  %43 = inttoptr i64 %42 to i8*
  %44 = bitcast %"QNCA_a0$float*$rank2$"* %23 to i8**
  %45 = call i32 @for_allocate_handle(i64 400, i8** nonnull %44, i32 262144, i8* %43) #6
  %46 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %47 = load i64, i64* %5, align 1
  %48 = load i64, i64* %3, align 1
  %49 = load i32, i32* %1, align 8
  %50 = sext i32 %49 to i64
  %51 = call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %48, i64 %47, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %46, i64 %50)
  call void @myoutput_(%"PHYSPROPMOD$.btPHYSPROP_TYPE"* %51) #6
  call void @myinit_(i32* nonnull %1) #6
  %52 = load i32, i32* %1, align 8
  %53 = add nsw i32 %52, 1
  store i32 %53, i32* %1, align 8
  br label %7
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(i32* nocapture readonly %0) local_unnamed_addr #2

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64 %0, i8** nocapture %1, i32 %2, i8* %3) local_unnamed_addr #2

; Function Attrs: argmemonly mustprogress nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly %0, i8* noalias nocapture readonly %1, i64 %2, i1 immarg %3) #4

; Function Attrs: argmemonly mustprogress nofree nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly %0, i8 %1, i64 %2, i1 immarg %3) #5

; Function Attrs: nofree noinline nounwind uwtable
define internal void @myoutput_(%"PHYSPROPMOD$.btPHYSPROP_TYPE"* noalias dereferenceable(352) %0) #0 {
  %2 = alloca [2 x { i8*, i64 }], align 8
  %3 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %0, i64 0, i32 0
  %4 = getelementptr inbounds [2 x { i8*, i64 }], [2 x { i8*, i64 }]* %2, i64 0, i64 0
  %5 = call { i8*, i64 }* @llvm.intel.subscript.p0sl_p0i8i64s.i64.i32.p0sl_p0i8i64s.i32(i8 0, i64 1, i32 16, { i8*, i64 }* elementtype({ i8*, i64 }) nonnull %4, i32 1)
  %6 = getelementptr inbounds { i8*, i64 }, { i8*, i64 }* %5, i64 0, i32 0
  %7 = getelementptr [256 x i8], [256 x i8]* %3, i64 0, i64 0
  store i8* %7, i8** %6, align 1
  %8 = getelementptr inbounds { i8*, i64 }, { i8*, i64 }* %5, i64 0, i32 1
  store i64 256, i64* %8, align 1
  %9 = call { i8*, i64 }* @llvm.intel.subscript.p0sl_p0i8i64s.i64.i32.p0sl_p0i8i64s.i32(i8 0, i64 1, i32 16, { i8*, i64 }* elementtype({ i8*, i64 }) nonnull %4, i32 2)
  %10 = getelementptr inbounds { i8*, i64 }, { i8*, i64 }* %9, i64 0, i32 0
  store i8* getelementptr inbounds ([13 x i8], [13 x i8]* @strlit.1, i64 0, i64 0), i8** %10, align 1
  %11 = getelementptr inbounds { i8*, i64 }, { i8*, i64 }* %9, i64 0, i32 1
  store i64 13, i64* %11, align 1
  %12 = alloca [269 x i8], align 1
  %13 = getelementptr inbounds [269 x i8], [269 x i8]* %12, i64 0, i64 0
  %14 = bitcast [2 x { i8*, i64 }]* %2 to i8*
  call void @for_concat(i8* nonnull %14, i64 2, i8* nonnull %13, i64 269) #6
  call void @myerror_(i8* nonnull %13, i64 269) #6
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare { i8*, i64 }* @llvm.intel.subscript.p0sl_p0i8i64s.i64.i32.p0sl_p0i8i64s.i32(i8 %0, i64 %1, i32 %2, { i8*, i64 }* elementtype({ i8*, i64 }) %3, i32 %4) #1

; Function Attrs: nofree
declare dso_local void @for_concat(i8* nocapture readonly %0, i64 %1, i8* nocapture %2, i64 %3) local_unnamed_addr #2

; Function Attrs: nofree noinline nounwind uwtable
define internal void @myerror_(i8* noalias readonly %0, i64 %1) #0 {
  %3 = alloca [8 x i64], align 16
  %4 = alloca [4 x i8], align 1
  %5 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 0
  store i8 56, i8* %5, align 1
  %6 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 1
  store i8 4, i8* %6, align 1
  %7 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 2
  store i8 1, i8* %7, align 1
  %8 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 3
  store i8 0, i8* %8, align 1
  %9 = alloca { i64, i8* }, align 8
  %10 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %9, i64 0, i32 0
  store i64 269, i64* %10, align 8
  %11 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %9, i64 0, i32 1
  store i8* %0, i8** %11, align 8
  %12 = bitcast [8 x i64]* %3 to i8*
  %13 = bitcast { i64, i8* }* %9 to i8*
  %14 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %12, i32 -1, i64 1239157112576, i8* nonnull %5, i8* nonnull %13) #6
  ret void
}

attributes #0 = { nofree noinline nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree nosync nounwind readnone speculatable }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { nofree noreturn nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #4 = { argmemonly mustprogress nofree nounwind willreturn }
attributes #5 = { argmemonly mustprogress nofree nounwind willreturn writeonly }
attributes #6 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
!3 = !{i64 1, i64 -9223372036854775808}

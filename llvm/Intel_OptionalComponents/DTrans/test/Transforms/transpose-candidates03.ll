; REQUIRES: asserts
; RUN: opt < %s -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck %s

; Check that it is profitable to transpose main_$MYA and main_$MYB, because their
; rank 0 index is indirectly subscripted. This case is similar to
; transpose-candidates02.ll, but allows indirectly indexed subscripts to be offset by
; a constant.

; CHECK: Transpose candidate: main_$MYA
; CHECK: IsValid{{ *}}: true
; CHECK: IsProfitable{{ *}}: true

; CHECK: Transpose candidate: main_$MYB
; CHECK: IsValid{{ *}}: true
; CHECK: IsProfitable{{ *}}: true

; Check that the array through which the indirect subscripting is occurring is
; neither a candidate for transposing nor profitable.

; CHECK: Transpose candidate: main_$MYK
; CHECK: IsValid{{ *}}: false
; CHECK: IsProfitable{{ *}}: false

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$float*$rank2$" = type { float*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@anon.027ecfc0ab928ef3b2e153cdd55f152e.0 = internal unnamed_addr constant i32 2
@"main_$MYA" = internal global [1000 x [19 x float]] zeroinitializer, align 16
@"main_$MYB" = internal global [1000 x [19 x float]] zeroinitializer, align 16
@"main_$MYK" = internal global [1000 x [19 x i32]] zeroinitializer, align 16

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #0 {
  %1 = alloca %"QNCA_a0$float*$rank2$", align 8
  %2 = alloca %"QNCA_a0$float*$rank2$", align 8
  %3 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.027ecfc0ab928ef3b2e153cdd55f152e.0) #5
  %4 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %1, i64 0, i32 3
  %5 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %1, i64 0, i32 1
  store i64 4, i64* %5, align 8
  %6 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %1, i64 0, i32 4
  store i64 2, i64* %6, align 8
  %7 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %1, i64 0, i32 2
  store i64 0, i64* %7, align 8
  %8 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %1, i64 0, i32 6, i64 0
  %9 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %8, i64 0, i32 1
  %10 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %9, i32 0)
  store i64 4, i64* %10, align 1
  %11 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %8, i64 0, i32 2
  %12 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %11, i32 0)
  store i64 1, i64* %12, align 1
  %13 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %8, i64 0, i32 0
  %14 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %13, i32 0)
  store i64 19, i64* %14, align 1
  %15 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %9, i32 1)
  store i64 76, i64* %15, align 1
  %16 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %11, i32 1)
  store i64 1, i64* %16, align 1
  %17 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %13, i32 1)
  store i64 1000, i64* %17, align 1
  %18 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %1, i64 0, i32 0
  store float* getelementptr inbounds ([1000 x [19 x float]], [1000 x [19 x float]]* @"main_$MYA", i64 0, i64 0, i64 0), float** %18, align 8
  store i64 1, i64* %4, align 8
  %19 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 3
  %20 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 1
  store i64 4, i64* %20, align 8
  %21 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 4
  store i64 2, i64* %21, align 8
  %22 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 2
  store i64 0, i64* %22, align 8
  %23 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 6, i64 0
  %24 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %23, i64 0, i32 1
  %25 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %24, i32 0)
  store i64 4, i64* %25, align 1
  %26 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %23, i64 0, i32 2
  %27 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %26, i32 0)
  store i64 1, i64* %27, align 1
  %28 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %23, i64 0, i32 0
  %29 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %28, i32 0)
  store i64 19, i64* %29, align 1
  %30 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %24, i32 1)
  store i64 76, i64* %30, align 1
  %31 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %26, i32 1)
  store i64 1, i64* %31, align 1
  %32 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %28, i32 1)
  store i64 1000, i64* %32, align 1
  %33 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 0
  store float* getelementptr inbounds ([1000 x [19 x float]], [1000 x [19 x float]]* @"main_$MYB", i64 0, i64 0, i64 0), float** %33, align 8
  store i64 1, i64* %19, align 8
  call void @foo_(%"QNCA_a0$float*$rank2$"* nonnull %1, %"QNCA_a0$float*$rank2$"* nonnull %2, i32* getelementptr inbounds ([1000 x [19 x i32]], [1000 x [19 x i32]]* @"main_$MYK", i64 0, i64 0, i64 0))
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(i32* nocapture readonly %0) local_unnamed_addr #1

; Function Attrs: nosync nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 %0, i64 %1, i32 %2, i64* elementtype(i64) %3, i32 %4) #2

; Function Attrs: nofree noinline nosync nounwind uwtable
define internal void @foo_(%"QNCA_a0$float*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %0, %"QNCA_a0$float*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %1, i32* noalias nocapture readonly dereferenceable(4) %2) #3 {
  %4 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %0, i64 0, i32 6, i64 0
  %5 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %4, i64 0, i32 0
  %6 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %5, i32 1)
  %7 = load i64, i64* %6, align 1
  %8 = icmp sgt i64 %7, 0
  %9 = select i1 %8, i64 %7, i64 0
  %10 = trunc i64 %9 to i32
  %11 = icmp slt i32 %10, 1
  br i1 %11, label %61, label %12

12:                                               ; preds = %3
  %13 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %5, i32 0)
  %14 = load i64, i64* %13, align 1
  %15 = icmp sgt i64 %14, 0
  %16 = select i1 %15, i64 %14, i64 0
  %17 = trunc i64 %16 to i32
  %18 = icmp slt i32 %17, 3
  %19 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %1, i64 0, i32 0
  %20 = load float*, float** %19, align 1
  %21 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %1, i64 0, i32 6, i64 0
  %22 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %21, i64 0, i32 1
  %23 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %22, i32 0)
  %24 = load i64, i64* %23, align 1
  %25 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %22, i32 1)
  %26 = load i64, i64* %25, align 1
  %27 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %0, i64 0, i32 0
  %28 = load float*, float** %27, align 1
  %29 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %4, i64 0, i32 1
  %30 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %29, i32 0)
  %31 = load i64, i64* %30, align 1
  %32 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %29, i32 1)
  %33 = load i64, i64* %32, align 1
  %34 = shl i64 %9, 32
  %35 = add i64 %34, 4294967296
  %36 = ashr exact i64 %35, 32
  %37 = shl i64 %16, 32
  %38 = ashr exact i64 %37, 32
  br label %39

39:                                               ; preds = %58, %12
  %40 = phi i64 [ 1, %12 ], [ %59, %58 ]
  br i1 %18, label %58, label %41

41:                                               ; preds = %39
  %42 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 76, i32* elementtype(i32) nonnull getelementptr inbounds ([1000 x [19 x i32]], [1000 x [19 x i32]]* @"main_$MYK", i64 0, i64 0, i64 0), i64 %40)
  %43 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %26, float* elementtype(float) %20, i64 %40)
  %44 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %33, float* elementtype(float) %28, i64 %40)
  br label %45

45:                                               ; preds = %45, %41
  %46 = phi i64 [ 2, %41 ], [ %56, %45 ]
  %47 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) nonnull %42, i64 %46)
  %48 = load i32, i32* %47, align 1
  %49 = add nsw i32 %48, -1
  %50 = sext i32 %49 to i64
  %51 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 %24, float* elementtype(float) %43, i64 %50)
  %52 = load float, float* %51, align 1
  %53 = add nsw i32 %48, 1
  %54 = sext i32 %53 to i64
  %55 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 %31, float* elementtype(float) %44, i64 %54)
  store float %52, float* %55, align 1
  %56 = add nuw nsw i64 %46, 1
  %57 = icmp eq i64 %56, %38
  br i1 %57, label %58, label %45

58:                                               ; preds = %45, %39
  %59 = add nuw nsw i64 %40, 1
  %60 = icmp eq i64 %59, %36
  br i1 %60, label %61, label %39

61:                                               ; preds = %58, %3
  ret void
}

; Function Attrs: nosync nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 %0, i64 %1, i64 %2, i32* elementtype(i32) %3, i64 %4) #2

; Function Attrs: nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 %0, i64 %1, i64 %2, float* elementtype(float) %3, i64 %4) #2

; Function Attrs: nofree nosync nounwind readnone willreturn
declare i64 @llvm.ssa.copy.i64(i64 returned %0) #4

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nosync nounwind readnone speculatable }
attributes #3 = { nofree noinline nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #4 = { nofree nosync nounwind readnone willreturn }
attributes #5 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}

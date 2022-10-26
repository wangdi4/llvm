; REQUIRES: asserts
; RUN: opt < %s -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck %s

; Check that the array through which the indirect subscripting is occurring is
; a candidate for transposing but not profitable.

; CHECK: Transpose candidate: main_$MYK
; CHECK: IsValid{{ *}}: true
; CHECK: IsProfitable{{ *}}: false

; Check that the arrays represented by dope vector arrays main_$MYA and
; main_$MYB can and should be transposed to ensure that the indirectly
; subscripted index is not the fastest varying subscript.

; CHECK: Transpose candidate: main_$MYB
; CHECK: IsValid{{ *}}: true
; CHECK: IsProfitable{{ *}}: true

; CHECK: Transpose candidate: main_$MYA
; CHECK: IsValid{{ *}}: true
; CHECK: IsProfitable{{ *}}: true

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$float*$rank2$" = type { float*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@"main_$MYK" = internal unnamed_addr constant [1000 x [19 x i32]] zeroinitializer, align 16
@"main_$MYB" = internal global %"QNCA_a0$float*$rank2$" { float* null, i64 0, i64 0, i64 1073741952, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }
@"main_$MYA" = internal global %"QNCA_a0$float*$rank2$" { float* null, i64 0, i64 0, i64 1073741952, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }
@anon.68ba48b9c6c80ce889c10c7426f57970.0 = internal unnamed_addr constant i32 2

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #0 {
  %1 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.68ba48b9c6c80ce889c10c7426f57970.0) #4
  %2 = load i64, i64* getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYA", i64 0, i32 3), align 8
  %3 = and i64 %2, 1030792151296
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYA", i64 0, i32 5), align 8
  store i64 4, i64* getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYA", i64 0, i32 1), align 8
  store i64 2, i64* getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYA", i64 0, i32 4), align 16
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYA", i64 0, i32 2), align 16
  %4 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYA", i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, i64* %4, align 1
  %5 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYA", i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 19, i64* %5, align 1
  %6 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYA", i64 0, i32 6, i64 0, i32 2), i32 1)
  store i64 1, i64* %6, align 1
  %7 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYA", i64 0, i32 6, i64 0, i32 0), i32 1)
  store i64 1000, i64* %7, align 1
  %8 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYA", i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 4, i64* %8, align 1
  %9 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYA", i64 0, i32 6, i64 0, i32 1), i32 1)
  store i64 76, i64* %9, align 1
  %10 = or i64 %3, 1073741957
  store i64 %10, i64* getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYA", i64 0, i32 3), align 8
  %11 = lshr i64 %3, 15
  %12 = trunc i64 %11 to i32
  %13 = or i32 %12, 262146
  %14 = tail call i32 @for_alloc_allocatable_handle(i64 76000, i8** bitcast (%"QNCA_a0$float*$rank2$"* @"main_$MYA" to i8**), i32 %13, i8* null) #4
  %15 = load i64, i64* getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYB", i64 0, i32 3), align 8
  %16 = and i64 %15, 1030792151296
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYB", i64 0, i32 5), align 8
  store i64 4, i64* getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYB", i64 0, i32 1), align 8
  store i64 2, i64* getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYB", i64 0, i32 4), align 16
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYB", i64 0, i32 2), align 16
  %17 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYB", i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, i64* %17, align 1
  %18 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYB", i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 19, i64* %18, align 1
  %19 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYB", i64 0, i32 6, i64 0, i32 2), i32 1)
  store i64 1, i64* %19, align 1
  %20 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYB", i64 0, i32 6, i64 0, i32 0), i32 1)
  store i64 1000, i64* %20, align 1
  %21 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYB", i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 4, i64* %21, align 1
  %22 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYB", i64 0, i32 6, i64 0, i32 1), i32 1)
  store i64 76, i64* %22, align 1
  %23 = or i64 %16, 1073741957
  store i64 %23, i64* getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYB", i64 0, i32 3), align 8
  %24 = lshr i64 %16, 15
  %25 = trunc i64 %24 to i32
  %26 = or i32 %25, 262146
  %27 = tail call i32 @for_alloc_allocatable_handle(i64 76000, i8** bitcast (%"QNCA_a0$float*$rank2$"* @"main_$MYB" to i8**), i32 %26, i8* null) #4
  %28 = load i64, i64* %7, align 1
  %29 = trunc i64 %28 to i32
  %30 = icmp slt i32 %29, 1
  br i1 %30, label %70, label %31

31:                                               ; preds = %0
  %32 = shl i64 %28, 32
  %33 = add i64 %32, 4294967296
  %34 = ashr exact i64 %33, 32
  br label %35

35:                                               ; preds = %67, %31
  %36 = phi i64 [ 1, %31 ], [ %68, %67 ]
  %37 = load i64, i64* %5, align 1
  %38 = trunc i64 %37 to i32
  %39 = icmp slt i32 %38, 3
  br i1 %39, label %67, label %40

40:                                               ; preds = %35
  %41 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 76, i32* elementtype(i32) getelementptr inbounds ([1000 x [19 x i32]], [1000 x [19 x i32]]* @"main_$MYK", i64 0, i64 0, i64 0), i64 %36)
  %42 = shl i64 %37, 32
  %43 = ashr exact i64 %42, 32
  br label %44

44:                                               ; preds = %44, %40
  %45 = phi i64 [ 2, %40 ], [ %65, %44 ]
  %46 = load float*, float** getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYB", i64 0, i32 0), align 16
  %47 = load i64, i64* %17, align 1
  %48 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %41, i64 %45)
  %49 = load i32, i32* %48, align 1
  %50 = add nsw i32 %49, -1
  %51 = sext i32 %50 to i64
  %52 = load i64, i64* %22, align 1
  %53 = load i64, i64* %19, align 1
  %54 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %53, i64 %52, float* elementtype(float) %46, i64 %36)
  %55 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %47, i64 4, float* elementtype(float) %54, i64 %51)
  %56 = load float, float* %55, align 1
  %57 = load float*, float** getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @"main_$MYA", i64 0, i32 0), align 16
  %58 = load i64, i64* %4, align 1
  %59 = add nsw i32 %49, 1
  %60 = sext i32 %59 to i64
  %61 = load i64, i64* %9, align 1
  %62 = load i64, i64* %6, align 1
  %63 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %62, i64 %61, float* elementtype(float) %57, i64 %36)
  %64 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %58, i64 4, float* elementtype(float) %63, i64 %60)
  store float %56, float* %64, align 1
  %65 = add nuw nsw i64 %45, 1
  %66 = icmp eq i64 %65, %43
  br i1 %66, label %67, label %44

67:                                               ; preds = %44, %35
  %68 = add nuw nsw i64 %36, 1
  %69 = icmp eq i64 %68, %34
  br i1 %69, label %70, label %35

70:                                               ; preds = %67, %0
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(i32* nocapture readonly %0) local_unnamed_addr #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 %0, i64 %1, i32 %2, i64* elementtype(i64) %3, i32 %4) #2

; Function Attrs: nofree
declare dso_local i32 @for_alloc_allocatable_handle(i64 %0, i8** nocapture %1, i32 %2, i8* %3) local_unnamed_addr #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 %0, i64 %1, i64 %2, i32* elementtype(i32) %3, i64 %4) #2

; Function Attrs: nofree nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 %0, i64 %1, i64 %2, float* elementtype(float) %3, i64 %4) #2

; Function Attrs: nofree nosync nounwind readnone willreturn mustprogress
declare i64 @llvm.ssa.copy.i64(i64 returned %0) #3

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nofree nosync nounwind readnone speculatable }
attributes #3 = { nofree nosync nounwind readnone willreturn mustprogress }
attributes #4 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}

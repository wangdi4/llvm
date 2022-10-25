; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -passes='require<wholeprogram>,cgscc(inline)' -inline-report=0xe807 --whole-program-assume-read -lto-inline-cost -dtrans-inline-heuristics -intel-libirc-allowed -inline-expose-local-arrays-min-args=2 -inline-expose-local-arrays-min-calls=2 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-BEFORE
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -inline-expose-local-arrays-min-args=2 -inline-expose-local-arrays-min-calls=2 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-AFTER

; Check that mycopy_ was NOT inlined according to the 'Exposes local arrays'
; inline heuristic, because mycopy_ references global arrays.

; CHECK-BEFORE: call{{.*}}@mycopy_
; CHECK-NOT: INLINE: mycopy_{{.*}}Exposes local arrays
; CHECK-AFTER: call{{.*}}@mycopy_

declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #0

declare dso_local i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr #0

declare dso_local i32 @for_write_seq_lis_xmit(i8* nocapture readonly, i8* nocapture readonly, i8*) local_unnamed_addr #0

@"main_$AMY" = internal global [100 x [100 x [100 x float]]] zeroinitializer, align 16
@"main_$BMY" = internal global [100 x [100 x [100 x float]]] zeroinitializer, align 16

define dso_local void @MAIN__() local_unnamed_addr #0 {
  tail call fastcc void @mycopy_(float* getelementptr inbounds ([100 x [100 x [100 x float]]], [100 x [100 x [100 x float]]]* @"main_$AMY", i64 0, i64 0, i64 0, i64 0), float* getelementptr inbounds ([100 x [100 x [100 x float]]], [100 x [100 x [100 x float]]]* @"main_$BMY", i64 0, i64 0, i64 0, i64 0))
  tail call fastcc void @mycopy_(float* getelementptr inbounds ([100 x [100 x [100 x float]]], [100 x [100 x [100 x float]]]* @"main_$BMY", i64 0, i64 0, i64 0, i64 0), float* getelementptr inbounds ([100 x [100 x [100 x float]]], [100 x [100 x [100 x float]]]* @"main_$AMY", i64 0, i64 0, i64 0, i64 0))
  ret void
}

define internal fastcc void @mycopy_(float* noalias nocapture dereferenceable(4) %0, float* noalias nocapture readonly dereferenceable(4) %1) unnamed_addr #0 {
  %3 = alloca [8 x i64], align 16
  %4 = alloca [4 x i8], align 1
  %5 = alloca { float }, align 8
  %6 = alloca [4 x i8], align 1
  %7 = alloca { float }, align 8
  br label %8

8:                                                ; preds = %26, %2
  %9 = phi i64 [ 1, %2 ], [ %27, %26 ]
  br label %10

10:                                               ; preds = %23, %8
  %11 = phi i64 [ 1, %8 ], [ %24, %23 ]
  br label %12

12:                                               ; preds = %12, %10
  %13 = phi i64 [ 1, %10 ], [ %21, %12 ]
  %14 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 40000, float* elementtype(float) nonnull %1, i64 %13)
  %15 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 400, float* elementtype(float) nonnull %14, i64 %11)
  %16 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %15, i64 %9)
  %17 = load float, float* %16, align 1
  %18 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 40000, float* elementtype(float) nonnull %0, i64 %13)
  %19 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 400, float* elementtype(float) nonnull %18, i64 %11)
  %20 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %19, i64 %9)
  store float %17, float* %20, align 1
  %21 = add nuw nsw i64 %13, 1
  %22 = icmp eq i64 %21, 101
  br i1 %22, label %23, label %12

23:                                               ; preds = %12
  %24 = add nuw nsw i64 %11, 1
  %25 = icmp eq i64 %24, 101
  br i1 %25, label %26, label %10

26:                                               ; preds = %23
  %27 = add nuw nsw i64 %9, 1
  %28 = icmp eq i64 %27, 101
  br i1 %28, label %29, label %8

29:                                               ; preds = %26
  %30 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 40000, float* elementtype(float) nonnull %0, i64 1)
  %31 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 400, float* elementtype(float) nonnull %30, i64 1)
  %32 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %31, i64 1)
  %33 = load float, float* %32, align 1
  %34 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 0
  store i8 26, i8* %34, align 1
  %35 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 1
  store i8 1, i8* %35, align 1
  %36 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 2
  store i8 2, i8* %36, align 1
  %37 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 3
  store i8 0, i8* %37, align 1
  %38 = getelementptr inbounds { float }, { float }* %5, i64 0, i32 0
  store float %33, float* %38, align 8
  %39 = bitcast [8 x i64]* %3 to i8*
  %40 = bitcast { float }* %5 to i8*
  %41 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %39, i32 -1, i64 1239157112576, i8* nonnull %34, i8* nonnull %40) #3
  %42 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 40000, float* elementtype(float) nonnull %1, i64 1)
  %43 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 400, float* elementtype(float) nonnull %42, i64 1)
  %44 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %43, i64 1)
  %45 = load float, float* %44, align 1
  %46 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 0
  store i8 26, i8* %46, align 1
  %47 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 1
  store i8 1, i8* %47, align 1
  %48 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 2
  store i8 1, i8* %48, align 1
  %49 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 3
  store i8 0, i8* %49, align 1
  %50 = getelementptr inbounds { float }, { float }* %7, i64 0, i32 0
  store float %45, float* %50, align 8
  %51 = bitcast { float }* %7 to i8*
  %52 = call i32 @for_write_seq_lis_xmit(i8* nonnull %39, i8* nonnull %46, i8* nonnull %51) #3
  ret void
}

attributes #0 = { "intel-lang"="fortran" }
; end INTEL_FEATURE_SW_ADVANCED

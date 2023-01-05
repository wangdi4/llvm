; REQUIRES: asserts
; RUN: opt < %s -disable-output -passes=dtrans-transpose -debug-only=dtrans-transpose-transform 2>&1 | FileCheck %s

; Check that physpropmod_mp_physprop_ fields 0 and 1 are both valid transpose
; candidates even though the allocate statements that allocate their arrays
; are in different routines than the routine in which their array accesses
; appear, there are multiple allocate statements for these two fields, and
; neither the allocation nor the accessing of the candidates are in main.

; Check that the array represented by field 0 of physpropmod_mp_physprop_
; is not transposed because the indirectly subscripted index is not the
; fastest varying subscript.

; CHECK-LABEL: Transform candidate: physpropmod_mp_physprop_[0]
; CHECK-NOT: Before
; CHECK-NOT: After

; Check that the array represented by field 1 of physpropmod_mp_physprop_
; is transposed to ensure that the indirectly subscripted index is not the
; fastest varying subscript. Check also the strides were replaced by
; literal constants.

; CHECK-LABEL: Transform candidate: physpropmod_mp_physprop_[1]
; CHECK-NEXT: Before: writefield_:  %[[N0:[0-9]+]] = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %[[I0:[0-9]+]],
; CHECK-NEXT: After : writefield_:  %[[N0]] = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4,
; CHECK-NEXT: Before: writefield_:  %[[N1:[0-9]+]] = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %[[I1:[0-9]+]],
; CHECK-NEXT: After : writefield_:  %[[N1]] = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 4000,

; Check that the array through which the indirect subscripting is occurring is
; not transposed, as transposing of simple arrays accessed from multiple
; functions is not yet supported.

; CHECK-LABEL: Transform candidate: main_$MYK
; CHECK-NOT: Before
; CHECK-NOT: After

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" = type { %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"PHYSPROPMOD$.btPHYSPROP_TYPE" = type { %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$" }
%"QNCA_a0$float*$rank2$" = type { float*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@physpropmod_mp_physprop_ = internal global %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" { %"PHYSPROPMOD$.btPHYSPROP_TYPE"* null, i64 0, i64 0, i64 0, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@"main_$MYK" = internal global [1000 x [19 x i32]] zeroinitializer, align 16
@anon.ed4b0e6d055126a7c60c6cbe5819f596.0 = internal unnamed_addr constant i32 2

; Function Attrs: nofree noinline nounwind uwtable
define internal void @initfield0_(%"PHYSPROPMOD$.btPHYSPROP_TYPE"* noalias nocapture dereferenceable(288) %0) #0 {
  %2 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %0, i64 0, i32 0
  %3 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 3
  %4 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 5
  store i64 0, i64* %4, align 1
  %5 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 1
  store i64 4, i64* %5, align 1
  %6 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 4
  store i64 2, i64* %6, align 1
  %7 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 2
  store i64 0, i64* %7, align 1
  %8 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 6, i64 0
  %9 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %8, i64 0, i32 2
  %10 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %9, i32 0)
  store i64 1, i64* %10, align 1
  %11 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %8, i64 0, i32 0
  %12 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %11, i32 0)
  store i64 19, i64* %12, align 1
  %13 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %9, i32 1)
  store i64 1, i64* %13, align 1
  %14 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %11, i32 1)
  store i64 1000, i64* %14, align 1
  %15 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %8, i64 0, i32 1
  %16 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %15, i32 0)
  store i64 4, i64* %16, align 1
  %17 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %15, i32 1)
  store i64 76, i64* %17, align 1
  store i64 1073741829, i64* %3, align 1
  %18 = bitcast %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %0 to i8**
  %19 = tail call i32 @for_allocate_handle(i64 76000, i8** nonnull %18, i32 262144, i8* null) #6
  %20 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %0, i64 0, i32 1
  %21 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 3
  %22 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 5
  store i64 0, i64* %22, align 1
  %23 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 1
  store i64 4, i64* %23, align 1
  %24 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 4
  store i64 2, i64* %24, align 1
  %25 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 2
  store i64 0, i64* %25, align 1
  %26 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 6, i64 0
  %27 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %26, i64 0, i32 2
  %28 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %27, i32 0)
  store i64 1, i64* %28, align 1
  %29 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %26, i64 0, i32 0
  %30 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %29, i32 0)
  store i64 19, i64* %30, align 1
  %31 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %27, i32 1)
  store i64 1, i64* %31, align 1
  %32 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %29, i32 1)
  store i64 1000, i64* %32, align 1
  %33 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %26, i64 0, i32 1
  %34 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %33, i32 0)
  store i64 4, i64* %34, align 1
  %35 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %33, i32 1)
  store i64 76, i64* %35, align 1
  store i64 1073741829, i64* %21, align 1
  %36 = bitcast %"QNCA_a0$float*$rank2$"* %20 to i8**
  %37 = tail call i32 @for_allocate_handle(i64 76000, i8** nonnull %36, i32 262144, i8* null) #6
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 %0, i64 %1, i32 %2, i64* elementtype(i64) %3, i32 %4) #1

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64 %0, i8** nocapture %1, i32 %2, i8* %3) local_unnamed_addr #2

; Function Attrs: nofree noinline nounwind uwtable
define internal void @initfield1_(%"PHYSPROPMOD$.btPHYSPROP_TYPE"* noalias nocapture dereferenceable(288) %0) #0 {
  %2 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %0, i64 0, i32 1
  %3 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 3
  %4 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 5
  store i64 0, i64* %4, align 1
  %5 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 1
  store i64 4, i64* %5, align 1
  %6 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 4
  store i64 2, i64* %6, align 1
  %7 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 2
  store i64 0, i64* %7, align 1
  %8 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 6, i64 0
  %9 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %8, i64 0, i32 2
  %10 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %9, i32 0)
  store i64 1, i64* %10, align 1
  %11 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %8, i64 0, i32 0
  %12 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %11, i32 0)
  store i64 19, i64* %12, align 1
  %13 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %9, i32 1)
  store i64 1, i64* %13, align 1
  %14 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %11, i32 1)
  store i64 1000, i64* %14, align 1
  %15 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %8, i64 0, i32 1
  %16 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %15, i32 0)
  store i64 4, i64* %16, align 1
  %17 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %15, i32 1)
  store i64 76, i64* %17, align 1
  store i64 1073741829, i64* %3, align 1
  %18 = bitcast %"QNCA_a0$float*$rank2$"* %2 to i8**
  %19 = tail call i32 @for_allocate_handle(i64 76000, i8** nonnull %18, i32 262144, i8* null) #6
  %20 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %0, i64 0, i32 0
  %21 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 3
  %22 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 5
  store i64 0, i64* %22, align 1
  %23 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 1
  store i64 4, i64* %23, align 1
  %24 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 4
  store i64 2, i64* %24, align 1
  %25 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 2
  store i64 0, i64* %25, align 1
  %26 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 6, i64 0
  %27 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %26, i64 0, i32 2
  %28 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %27, i32 0)
  store i64 1, i64* %28, align 1
  %29 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %26, i64 0, i32 0
  %30 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %29, i32 0)
  store i64 19, i64* %30, align 1
  %31 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %27, i32 1)
  store i64 1, i64* %31, align 1
  %32 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %29, i32 1)
  store i64 1000, i64* %32, align 1
  %33 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %26, i64 0, i32 1
  %34 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %33, i32 0)
  store i64 4, i64* %34, align 1
  %35 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %33, i32 1)
  store i64 76, i64* %35, align 1
  store i64 1073741829, i64* %21, align 1
  %36 = bitcast %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %0 to i8**
  %37 = tail call i32 @for_allocate_handle(i64 76000, i8** nonnull %36, i32 262144, i8* null) #6
  ret void
}

; Function Attrs: nofree noinline nounwind uwtable
define internal void @initfield2_(%"PHYSPROPMOD$.btPHYSPROP_TYPE"* noalias nocapture dereferenceable(288) %0) #0 {
  tail call void @initfield1_(%"PHYSPROPMOD$.btPHYSPROP_TYPE"* nonnull %0)
  ret void
}

; Function Attrs: nofree noinline nosync nounwind uwtable
define internal void @writefield_(%"PHYSPROPMOD$.btPHYSPROP_TYPE"* noalias nocapture readonly dereferenceable(288) %0, i32* noalias nocapture readonly dereferenceable(4) %1) #3 {
  %3 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %0, i64 0, i32 1
  %4 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %3, i64 0, i32 0
  %5 = load float*, float** %4, align 1
  %6 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %3, i64 0, i32 6, i64 0
  %7 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %6, i64 0, i32 1
  %8 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %7, i32 0)
  %9 = load i64, i64* %8, align 1
  %10 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %6, i64 0, i32 2
  %11 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %10, i32 0)
  %12 = load i64, i64* %11, align 1
  %13 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %7, i32 1)
  %14 = load i64, i64* %13, align 1
  %15 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %10, i32 1)
  %16 = load i64, i64* %15, align 1
  %17 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %0, i64 0, i32 0
  %18 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %17, i64 0, i32 0
  %19 = load float*, float** %18, align 1
  %20 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %17, i64 0, i32 6, i64 0
  %21 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %20, i64 0, i32 1
  %22 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %21, i32 0)
  %23 = load i64, i64* %22, align 1
  %24 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %20, i64 0, i32 2
  %25 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %24, i32 0)
  %26 = load i64, i64* %25, align 1
  %27 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %21, i32 1)
  %28 = load i64, i64* %27, align 1
  %29 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %24, i32 1)
  %30 = load i64, i64* %29, align 1
  br label %31

31:                                               ; preds = %57, %2
  %32 = phi i64 [ %58, %57 ], [ 1, %2 ]
  br label %33

33:                                               ; preds = %54, %31
  %34 = phi i64 [ %55, %54 ], [ 1, %31 ]
  %35 = sub nsw i64 %32, %34
  %36 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 76, i32* elementtype(i32) nonnull getelementptr inbounds ([1000 x [19 x i32]], [1000 x [19 x i32]]* @"main_$MYK", i64 0, i64 0, i64 0), i64 %34)
  %37 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %16, i64 %14, float* elementtype(float) %5, i64 %34)
  br label %38

38:                                               ; preds = %38, %33
  %39 = phi i64 [ %52, %38 ], [ 2, %33 ]
  %40 = add nsw i64 %35, %39
  %41 = trunc i64 %40 to i32
  %42 = sitofp i32 %41 to float
  %43 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) nonnull %36, i64 %39)
  %44 = load i32, i32* %43, align 1
  %45 = add nsw i32 %44, 1
  %46 = sext i32 %45 to i64
  %47 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %12, i64 %9, float* elementtype(float) %37, i64 %46)
  store float %42, float* %47, align 1
  %48 = add nsw i32 %44, -1
  %49 = sext i32 %48 to i64
  %50 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %30, i64 %28, float* elementtype(float) %19, i64 %49)
  %51 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %26, i64 %23, float* elementtype(float) %50, i64 %34)
  store float %42, float* %51, align 1
  %52 = add nuw nsw i64 %39, 1
  %53 = icmp eq i64 %52, 19
  br i1 %53, label %54, label %38

54:                                               ; preds = %38
  %55 = add nuw nsw i64 %34, 1
  %56 = icmp eq i64 %55, 1001
  br i1 %56, label %57, label %33

57:                                               ; preds = %54
  %58 = add nuw nsw i64 %32, 1
  %59 = icmp eq i64 %58, 101
  br i1 %59, label %60, label %31

60:                                               ; preds = %57
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 %0, i64 %1, i64 %2, i32* elementtype(i32) %3, i64 %4) #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 %0, i64 %1, i64 %2, float* elementtype(float) %3, i64 %4) #1

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #4 {
  %1 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.ed4b0e6d055126a7c60c6cbe5819f596.0) #6
  br label %2

2:                                                ; preds = %11, %0
  %3 = phi i64 [ %12, %11 ], [ 1, %0 ]
  %4 = trunc i64 %3 to i32
  br label %5

5:                                                ; preds = %5, %2
  %6 = phi i64 [ %9, %5 ], [ 1, %2 ]
  %7 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 76, i32* elementtype(i32) getelementptr inbounds ([1000 x [19 x i32]], [1000 x [19 x i32]]* @"main_$MYK", i64 0, i64 0, i64 0), i64 %6)
  %8 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %7, i64 %3)
  store i32 %4, i32* %8, align 1
  %9 = add nuw nsw i64 %6, 1
  %10 = icmp eq i64 %9, 1001
  br i1 %10, label %11, label %5

11:                                               ; preds = %5
  %12 = add nuw nsw i64 %3, 1
  %13 = icmp eq i64 %12, 20
  br i1 %13, label %14, label %2

14:                                               ; preds = %11
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 5), align 8
  store i64 288, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 1), align 8
  store i64 1, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 4), align 8
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 2), align 8
  %15 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, i64* %15, align 1
  %16 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 100, i64* %16, align 1
  %17 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 288, i64* %17, align 1
  store i64 1073741829, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  %18 = tail call i32 @for_allocate_handle(i64 28800, i8** bitcast (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_ to i8**), i32 262144, i8* null) #6
  br label %19

19:                                               ; preds = %19, %14
  %20 = phi i64 [ %33, %19 ], [ 1, %14 ]
  %21 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %22 = load i64, i64* %17, align 1
  %23 = load i64, i64* %15, align 1
  %24 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %23, i64 %22, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %21, i64 %20)
  tail call void @initfield0_(%"PHYSPROPMOD$.btPHYSPROP_TYPE"* %24)
  %25 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %26 = load i64, i64* %17, align 1
  %27 = load i64, i64* %15, align 1
  %28 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %27, i64 %26, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %25, i64 %20)
  tail call void @initfield2_(%"PHYSPROPMOD$.btPHYSPROP_TYPE"* %28)
  %29 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %30 = load i64, i64* %17, align 1
  %31 = load i64, i64* %15, align 1
  %32 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %31, i64 %30, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %29, i64 %20)
  tail call void @writefield_(%"PHYSPROPMOD$.btPHYSPROP_TYPE"* %32, i32* getelementptr inbounds ([1000 x [19 x i32]], [1000 x [19 x i32]]* @"main_$MYK", i64 0, i64 0, i64 0))
  %33 = add nuw nsw i64 %20, 1
  %34 = icmp eq i64 %33, 101
  br i1 %34, label %35, label %19

35:                                               ; preds = %19
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(i32* nocapture readonly %0) local_unnamed_addr #2

; Function Attrs: nofree nosync nounwind readnone speculatable
declare %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 %0, i64 %1, i64 %2, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %3, i64 %4) #1

; Function Attrs: nofree nosync nounwind readnone willreturn mustprogress
declare i64 @llvm.ssa.copy.i64(i64 returned %0) #5

attributes #0 = { nofree noinline nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree nosync nounwind readnone speculatable }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { nofree noinline nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #4 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #5 = { nofree nosync nounwind readnone willreturn mustprogress }
attributes #6 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}

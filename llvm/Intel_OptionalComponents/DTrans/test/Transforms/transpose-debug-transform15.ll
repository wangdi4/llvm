; REQUIRES: asserts
; RUN: opt < %s -disable-output -dtrans-transpose -debug-only=dtrans-transpose-transform  2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -passes=dtrans-transpose -debug-only=dtrans-transpose-transform 2>&1 | FileCheck %s

; Check that physpropmod_mp_physprop_ field 0 is transposed even though it is assigned through
; a pointer assignment, propagated up the call chain to an AllocaInst and then
; propagated down the call chain.

; CHECK-LABEL: Transform candidate: physpropmod_mp_physprop_[0]
; CHECK: Before: get_hygro_rad_props_:  %[[N0:[0-9]+]]  = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %[[X0:[0-9]+]], float* %[[I0:[0-9]+]], i64 %[[J0:[0-9]+]])
; CHECK: After : get_hygro_rad_props_:  %[[N0]] = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* %[[I0]], i64 %[[J0]])
; CHECK: Before: get_hygro_rad_props_:  %[[N1:[0-9]+]] = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 %[[X1:[0-9]+]], float* %[[I1:[0-9]+]], i64 %[[J1:[0-9]+]])
; CHECK: After : get_hygro_rad_props_:  %[[N1]] = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 76, float* %[[I1]], i64 %[[J1]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" = type { %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"PHYSPROPMOD$.btPHYSPROP_TYPE" = type { %"QNCA_a0$float*$rank2$" }
%"QNCA_a0$float*$rank2$" = type { float*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@physpropmod_mp_physprop_ = internal global %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" { %"PHYSPROPMOD$.btPHYSPROP_TYPE"* null, i64 0, i64 0, i64 0, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@"main_$TAU" = internal global [19 x [26 x [4 x float]]] zeroinitializer, align 16
@"main_$KRH" = internal global [26 x [4 x i32]] zeroinitializer, align 16
@anon.5b3c5dff38f14be85a38ea3af0f7d558.0 = internal unnamed_addr constant i32 2
@anon.5b3c5dff38f14be85a38ea3af0f7d558.1 = internal unnamed_addr constant i32 10

; Function Attrs: nofree noinline nounwind uwtable
define internal void @physprop_init_(i32* noalias nocapture readonly dereferenceable(4) %0) #0 {
  %2 = alloca i64, align 8
  store i64 5, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 5), align 8
  store i64 96, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 1), align 8
  store i64 1, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 4), align 8
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 2), align 8
  %3 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, i64* %3, align 1
  %4 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 10, i64* %4, align 1
  %5 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 96, i64* %5, align 1
  %6 = call i32 (i64*, i32, ...) @for_check_mult_overflow64(i64* nonnull %2, i32 2, i64 10, i64 96) #9
  %7 = load i64, i64* %2, align 8
  %8 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  %9 = and i64 %8, -68451041281
  %10 = or i64 %9, 1073741824
  store i64 %10, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  %11 = shl i32 %6, 4
  %12 = and i32 %11, 16
  %13 = or i32 %12, 262144
  %14 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 5), align 8
  %15 = inttoptr i64 %14 to i8*
  %16 = tail call i32 @for_allocate_handle(i64 %7, i8** bitcast (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_ to i8**), i32 %13, i8* %15) #9
  br label %17

17:                                               ; preds = %1, %17
  %18 = phi float [ %29, %17 ], [ 1.000000e+00, %1 ]
  %19 = phi i64 [ %30, %17 ], [ 10, %1 ]
  %20 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %21 = load i64, i64* %5, align 1
  %22 = load i64, i64* %3, align 1
  %23 = fptosi float %18 to i32
  %24 = sext i32 %23 to i64
  %25 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %22, i64 %21, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %20, i64 %24)
  %26 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %25, i64 0, i32 0
  %27 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %26, i64 0, i32 0
  store float* null, float** %27, align 1
  %28 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %26, i64 0, i32 3
  store i64 0, i64* %28, align 1
  %29 = fadd reassoc ninf nsz arcp contract afn float %18, 1.000000e+00
  %30 = add nsw i64 %19, -1
  %31 = icmp sgt i64 %19, 1
  br i1 %31, label %17, label %32

32:                                               ; preds = %32, %17
  %33 = phi float [ %64, %32 ], [ 1.000000e+00, %17 ]
  %34 = phi i64 [ %65, %32 ], [ 10, %17 ]
  %35 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %36 = load i64, i64* %5, align 1
  %37 = load i64, i64* %3, align 1
  %38 = fptosi float %33 to i32
  %39 = sext i32 %38 to i64
  %40 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %37, i64 %36, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %35, i64 %39)
  %41 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %40, i64 0, i32 0
  %42 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %41, i64 0, i32 3
  store i64 5, i64* %42, align 1
  %43 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %41, i64 0, i32 5
  store i64 0, i64* %43, align 1
  %44 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %41, i64 0, i32 1
  store i64 4, i64* %44, align 1
  %45 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %41, i64 0, i32 4
  store i64 2, i64* %45, align 1
  %46 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %41, i64 0, i32 2
  store i64 0, i64* %46, align 1
  %47 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %41, i64 0, i32 6, i64 0
  %48 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %47, i64 0, i32 2
  %49 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %48, i32 0)
  store i64 1, i64* %49, align 1
  %50 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %47, i64 0, i32 0
  %51 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %50, i32 0)
  store i64 1000, i64* %51, align 1
  %52 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %48, i32 1)
  store i64 1, i64* %52, align 1
  %53 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %50, i32 1)
  store i64 19, i64* %53, align 1
  %54 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %47, i64 0, i32 1
  %55 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %54, i32 0)
  store i64 4, i64* %55, align 1
  %56 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %54, i32 1)
  store i64 4000, i64* %56, align 1
  %57 = load i64, i64* %42, align 1
  %58 = and i64 %57, -68451041281
  %59 = or i64 %58, 1073741824
  store i64 %59, i64* %42, align 1
  %60 = load i64, i64* %43, align 1
  %61 = inttoptr i64 %60 to i8*
  %62 = bitcast %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %40 to i8**
  %63 = tail call i32 @for_allocate_handle(i64 76000, i8** %62, i32 262144, i8* %61) #9
  %64 = fadd reassoc ninf nsz arcp contract afn float %33, 1.000000e+00
  %65 = add nsw i64 %34, -1
  %66 = icmp sgt i64 %34, 1
  br i1 %66, label %32, label %67

67:                                               ; preds = %32
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 %0, i64 %1, i32 %2, i64* %3, i32 %4) #1

; Function Attrs: nofree
declare dso_local i32 @for_check_mult_overflow64(i64* nocapture %0, i32 %1, ...) local_unnamed_addr #2

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64 %0, i8** nocapture %1, i32 %2, i8* %3) local_unnamed_addr #2

; Function Attrs: nofree nosync nounwind readnone speculatable
declare %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 %0, i64 %1, i64 %2, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %3, i64 %4) #1

; Function Attrs: nofree noinline nosync nounwind uwtable willreturn mustprogress
define internal void @physprop_get_(i32* noalias nocapture readonly dereferenceable(4) %0, %"QNCA_a0$float*$rank2$"* noalias nocapture dereferenceable(96) %1) #3 {
  %3 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %4 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  %5 = load i64, i64* %4, align 1
  %6 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %7 = load i64, i64* %6, align 1
  %8 = load i32, i32* %0, align 1
  %9 = sext i32 %8 to i64
  %10 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %7, i64 %5, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %3, i64 %9)
  %11 = bitcast %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %10 to %"QNCA_a0$float*$rank2$"*
  %12 = load %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %11, align 1
  store %"QNCA_a0$float*$rank2$" %12, %"QNCA_a0$float*$rank2$"* %1, align 1
  ret void
}

; Function Attrs: nofree noinline nosync nounwind uwtable
define internal void @get_hygro_rad_props_(i32* noalias nocapture readonly dereferenceable(4) %0, %"QNCA_a0$float*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %1, float* noalias nocapture readnone dereferenceable(4) %2) #4 {
  %4 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %1, i64 0, i32 0
  %5 = load float*, float** %4, align 1
  %6 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %1, i64 0, i32 6, i64 0
  %7 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %6, i64 0, i32 1
  %8 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %7, i32 0)
  %9 = load i64, i64* %8, align 1
  %10 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %7, i32 1)
  %11 = load i64, i64* %10, align 1
  br label %12

12:                                               ; preds = %35, %3
  %13 = phi i64 [ %36, %35 ], [ 1, %3 ]
  %14 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %11, float* %5, i64 %13)
  %15 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 416, float* nonnull getelementptr inbounds ([19 x [26 x [4 x float]]], [19 x [26 x [4 x float]]]* @"main_$TAU", i64 0, i64 0, i64 0, i64 0), i64 %13)
  br label %16

16:                                               ; preds = %32, %12
  %17 = phi i64 [ %33, %32 ], [ 1, %12 ]
  br label %18

18:                                               ; preds = %18, %16
  %19 = phi i64 [ %30, %18 ], [ 1, %16 ]
  %20 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 16, i32* nonnull getelementptr inbounds ([26 x [4 x i32]], [26 x [4 x i32]]* @"main_$KRH", i64 0, i64 0, i64 0), i64 %19)
  %21 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %20, i64 %17)
  %22 = load i32, i32* %21, align 1
  %23 = add nsw i32 %22, 1
  %24 = sext i32 %23 to i64
  %25 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 %9, float* %14, i64 %24)
  %26 = load float, float* %25, align 1
  %27 = fadd reassoc ninf nsz arcp contract afn float %26, %26
  %28 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 16, float* nonnull %15, i64 %19)
  %29 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* nonnull %28, i64 %17)
  store float %27, float* %29, align 1
  %30 = add nuw nsw i64 %19, 1
  %31 = icmp eq i64 %30, 27
  br i1 %31, label %32, label %18

32:                                               ; preds = %18
  %33 = add nuw nsw i64 %17, 1
  %34 = icmp eq i64 %33, 5
  br i1 %34, label %35, label %16

35:                                               ; preds = %32
  %36 = add nuw nsw i64 %13, 1
  %37 = icmp eq i64 %36, 20
  br i1 %37, label %38, label %12

38:                                               ; preds = %35
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 %0, i64 %1, i64 %2, float* %3, i64 %4) #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 %0, i64 %1, i64 %2, i32* %3, i64 %4) #1

; Function Attrs: nofree noinline nosync nounwind uwtable writeonly
define internal void @init_krh_(i32* noalias nocapture readnone dereferenceable(4) %0) #5 {
  br label %2

2:                                                ; preds = %12, %1
  %3 = phi i64 [ %13, %12 ], [ 1, %1 ]
  br label %4

4:                                                ; preds = %4, %2
  %5 = phi i64 [ %10, %4 ], [ 1, %2 ]
  %6 = add nuw nsw i64 %5, %3
  %7 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 16, i32* nonnull getelementptr inbounds ([26 x [4 x i32]], [26 x [4 x i32]]* @"main_$KRH", i64 0, i64 0, i64 0), i64 %5)
  %8 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull %7, i64 %3)
  %9 = trunc i64 %6 to i32
  store i32 %9, i32* %8, align 1
  %10 = add nuw nsw i64 %5, 1
  %11 = icmp eq i64 %10, 27
  br i1 %11, label %12, label %4

12:                                               ; preds = %4
  %13 = add nuw nsw i64 %3, 1
  %14 = icmp eq i64 %13, 5
  br i1 %14, label %15, label %2

15:                                               ; preds = %12
  ret void
}

; Function Attrs: nofree noinline noreturn nosync nounwind uwtable
define internal void @bigloop_(float* noalias nocapture readnone dereferenceable(4) %0, i32* noalias nocapture readonly dereferenceable(4) %1) #6 {
  %3 = alloca i32, align 8
  %4 = alloca %"QNCA_a0$float*$rank2$", align 8
  %5 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %4, i64 0, i32 0
  store float* null, float** %5, align 8
  %6 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %4, i64 0, i32 3
  store i64 0, i64* %6, align 8
  store i32 1, i32* %3, align 8
  br label %7

7:                                                ; preds = %7, %2
  call void @physprop_get_(i32* nonnull %3, %"QNCA_a0$float*$rank2$"* nonnull %4)
  call void @get_hygro_rad_props_(i32* nonnull getelementptr inbounds ([26 x [4 x i32]], [26 x [4 x i32]]* @"main_$KRH", i64 0, i64 0, i64 0), %"QNCA_a0$float*$rank2$"* nonnull %4, float* nonnull getelementptr inbounds ([19 x [26 x [4 x float]]], [19 x [26 x [4 x float]]]* @"main_$TAU", i64 0, i64 0, i64 0, i64 0))
  %8 = load i32, i32* %3, align 8
  %9 = add nsw i32 %8, 1
  store i32 %9, i32* %3, align 8
  br label %7
}

; Function Attrs: nofree noreturn nounwind uwtable
define dso_local void @MAIN__() #7 {
  %1 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.5b3c5dff38f14be85a38ea3af0f7d558.0) #9
  tail call void @init_krh_(i32* getelementptr inbounds ([26 x [4 x i32]], [26 x [4 x i32]]* @"main_$KRH", i64 0, i64 0, i64 0))
  tail call void @physprop_init_(i32* nonnull @anon.5b3c5dff38f14be85a38ea3af0f7d558.1)
  tail call void @bigloop_(float* getelementptr inbounds ([19 x [26 x [4 x float]]], [19 x [26 x [4 x float]]]* @"main_$TAU", i64 0, i64 0, i64 0, i64 0), i32* getelementptr inbounds ([26 x [4 x i32]], [26 x [4 x i32]]* @"main_$KRH", i64 0, i64 0, i64 0))
  unreachable
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(i32* nocapture readonly %0) local_unnamed_addr #2

; Function Attrs: nofree nosync nounwind readnone willreturn mustprogress
declare i64 @llvm.ssa.copy.i64(i64 returned %0) #8

attributes #0 = { nofree noinline nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree nosync nounwind readnone speculatable }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { nofree noinline nosync nounwind uwtable willreturn mustprogress "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #4 = { nofree noinline nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #5 = { nofree noinline nosync nounwind uwtable writeonly "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #6 = { nofree noinline noreturn nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #7 = { nofree noreturn nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #8 = { nofree nosync nounwind readnone willreturn mustprogress }
attributes #9 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}

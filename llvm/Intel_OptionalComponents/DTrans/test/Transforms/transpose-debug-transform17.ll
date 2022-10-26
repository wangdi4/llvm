; REQUIRES; asserts
; RUN: opt < %s -dva-check-dtrans-outofboundsok -dtrans-outofboundsok=false -disable-output -passes=dtrans-transpose  -debug-only=dtrans-transpose-transform 2>&1 | FileCheck %s

; Check that physpropmod_mp_physprop_ field 0 is transposed even though field 1
; is a variable that is of type character(len=32) and passed down the call
; chain, as long as -dtrans-outofboundsok=false.

; CHECK-LABEL: Transform candidate: physpropmod_mp_physprop_[0]
; CHECK: Before: get_hygro_rad_props_:  %[[N0:[0-9]+]]  = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %[[X0:[0-9]+]], float* elementtype(float) %[[I0:[0-9]+]], i64 %[[J0:[0-9]+]])
; CHECK: After : get_hygro_rad_props_:  %[[N0]] = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %[[I0]], i64 %[[J0]])
; CHECK: Before: get_hygro_rad_props_:  %[[N1:[0-9]+]] = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 %[[X1:[0-9]+]], float* elementtype(float) %[[I1:[0-9]+]], i64 %[[J1:[0-9]+]])
; CHECK: After : get_hygro_rad_props_:  %[[N1]] = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 76, float* elementtype(float) %[[I1]], i64 %[[J1]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" = type { %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"PHYSPROPMOD$.btPHYSPROP_TYPE" = type { %"QNCA_a0$float*$rank2$", [32 x i8] }
%"QNCA_a0$float*$rank2$" = type { float*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@strlit = internal unnamed_addr constant [5 x i8] c"hello"
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
  store i64 128, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 1), align 8
  store i64 1, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 4), align 8
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 2), align 8
  %3 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, i64* %3, align 1
  %4 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 10, i64* %4, align 1
  %5 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 128, i64* %5, align 1
  %6 = call i32 (i64*, i32, ...) @for_check_mult_overflow64(i64* nonnull %2, i32 2, i64 10, i64 128) #11
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
  %16 = tail call i32 @for_allocate_handle(i64 %7, i8** bitcast (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_ to i8**), i32 %13, i8* %15) #11
  br label %17

17:                                               ; preds = %1, %17
  %18 = phi float [ %29, %17 ], [ 1.000000e+00, %1 ]
  %19 = phi i64 [ %30, %17 ], [ 10, %1 ]
  %20 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %21 = load i64, i64* %5, align 1
  %22 = load i64, i64* %3, align 1
  %23 = fptosi float %18 to i32
  %24 = sext i32 %23 to i64
  %25 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %22, i64 %21, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %20, i64 %24)
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
  %33 = phi float [ %71, %32 ], [ 1.000000e+00, %17 ]
  %34 = phi i64 [ %72, %32 ], [ 10, %17 ]
  %35 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %36 = load i64, i64* %5, align 1
  %37 = load i64, i64* %3, align 1
  %38 = fptosi float %33 to i32
  %39 = sext i32 %38 to i64
  %40 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %37, i64 %36, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %35, i64 %39)
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
  %49 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %48, i32 0)
  store i64 1, i64* %49, align 1
  %50 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %47, i64 0, i32 0
  %51 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %50, i32 0)
  store i64 1000, i64* %51, align 1
  %52 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %48, i32 1)
  store i64 1, i64* %52, align 1
  %53 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %50, i32 1)
  store i64 19, i64* %53, align 1
  %54 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %47, i64 0, i32 1
  %55 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %54, i32 0)
  store i64 4, i64* %55, align 1
  %56 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %54, i32 1)
  store i64 4000, i64* %56, align 1
  %57 = load i64, i64* %42, align 1
  %58 = and i64 %57, -68451041281
  %59 = or i64 %58, 1073741824
  store i64 %59, i64* %42, align 1
  %60 = load i64, i64* %43, align 1
  %61 = inttoptr i64 %60 to i8*
  %62 = bitcast %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %40 to i8**
  %63 = tail call i32 @for_allocate_handle(i64 76000, i8** %62, i32 262144, i8* %61) #11
  %64 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %65 = load i64, i64* %5, align 1
  %66 = load i64, i64* %3, align 1
  %67 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %66, i64 %65, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %64, i64 %39)
  %68 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %67, i64 0, i32 1
  %69 = getelementptr [32 x i8], [32 x i8]* %68, i64 0, i64 0
  %70 = getelementptr i8, i8* %69, i64 5
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* noundef nonnull align 1 dereferenceable(5) %69, i8* noundef nonnull align 1 dereferenceable(5) getelementptr inbounds ([5 x i8], [5 x i8]* @strlit, i64 0, i64 0), i64 5, i1 false)
  tail call void @llvm.memset.p0i8.i64(i8* noundef nonnull align 1 dereferenceable(27) %70, i8 32, i64 27, i1 false)
  %71 = fadd reassoc ninf nsz arcp contract afn float %33, 1.000000e+00
  %72 = add nsw i64 %34, -1
  %73 = icmp sgt i64 %34, 1
  br i1 %73, label %32, label %74

74:                                               ; preds = %32
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 %0, i64 %1, i32 %2, i64* elementtype(i64) %3, i32 %4) #1

; Function Attrs: nofree
declare dso_local i32 @for_check_mult_overflow64(i64* nocapture %0, i32 %1, ...) local_unnamed_addr #2

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64 %0, i8** nocapture %1, i32 %2, i8* %3) local_unnamed_addr #2

; Function Attrs: nofree nosync nounwind readnone speculatable
declare %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 %0, i64 %1, i64 %2, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %3, i64 %4) #1

; Function Attrs: argmemonly mustprogress nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly %0, i8* noalias nocapture readonly %1, i64 %2, i1 immarg %3) #3

; Function Attrs: argmemonly mustprogress nofree nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly %0, i8 %1, i64 %2, i1 immarg %3) #4

; Function Attrs: mustprogress nofree noinline nosync nounwind uwtable willreturn
define internal void @physprop_get_(i32* noalias nocapture readonly dereferenceable(4) %0, %"QNCA_a0$float*$rank2$"* noalias nocapture dereferenceable(96) %1) #5 {
  %3 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %4 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  %5 = load i64, i64* %4, align 1
  %6 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %7 = load i64, i64* %6, align 1
  %8 = load i32, i32* %0, align 1
  %9 = sext i32 %8 to i64
  %10 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %7, i64 %5, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %3, i64 %9)
  %11 = bitcast %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %10 to %"QNCA_a0$float*$rank2$"*
  %12 = load %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %11, align 1
  store %"QNCA_a0$float*$rank2$" %12, %"QNCA_a0$float*$rank2$"* %1, align 1
  ret void
}

; Function Attrs: nofree noinline nosync nounwind uwtable
define internal void @get_hygro_rad_props_(i32* noalias nocapture readonly dereferenceable(4) %0, %"QNCA_a0$float*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %1, float* noalias nocapture readnone dereferenceable(4) %2, i8* noalias nocapture readonly %3, i64 %4) #6 {
  %6 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %1, i64 0, i32 0
  %7 = load float*, float** %6, align 1
  %8 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %1, i64 0, i32 6, i64 0
  %9 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %8, i64 0, i32 1
  %10 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %9, i32 0)
  %11 = load i64, i64* %10, align 1
  %12 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %9, i32 1)
  %13 = load i64, i64* %12, align 1
  br label %14

14:                                               ; preds = %37, %5
  %15 = phi i64 [ %38, %37 ], [ 1, %5 ]
  %16 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %13, float* elementtype(float) %7, i64 %15)
  %17 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 416, float* elementtype(float) nonnull getelementptr inbounds ([19 x [26 x [4 x float]]], [19 x [26 x [4 x float]]]* @"main_$TAU", i64 0, i64 0, i64 0, i64 0), i64 %15)
  br label %18

18:                                               ; preds = %34, %14
  %19 = phi i64 [ %35, %34 ], [ 1, %14 ]
  br label %20

20:                                               ; preds = %20, %18
  %21 = phi i64 [ %32, %20 ], [ 1, %18 ]
  %22 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 16, i32* elementtype(i32) nonnull getelementptr inbounds ([26 x [4 x i32]], [26 x [4 x i32]]* @"main_$KRH", i64 0, i64 0, i64 0), i64 %21)
  %23 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) nonnull %22, i64 %19)
  %24 = load i32, i32* %23, align 1
  %25 = add nsw i32 %24, 1
  %26 = sext i32 %25 to i64
  %27 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 %11, float* elementtype(float) %16, i64 %26)
  %28 = load float, float* %27, align 1
  %29 = fadd reassoc ninf nsz arcp contract afn float %28, %28
  %30 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 16, float* elementtype(float) nonnull %17, i64 %21)
  %31 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %30, i64 %19)
  store float %29, float* %31, align 1
  %32 = add nuw nsw i64 %21, 1
  %33 = icmp eq i64 %32, 27
  br i1 %33, label %34, label %20

34:                                               ; preds = %20
  %35 = add nuw nsw i64 %19, 1
  %36 = icmp eq i64 %35, 5
  br i1 %36, label %37, label %18

37:                                               ; preds = %34
  %38 = add nuw nsw i64 %15, 1
  %39 = icmp eq i64 %38, 20
  br i1 %39, label %40, label %14

40:                                               ; preds = %37
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 %0, i64 %1, i64 %2, float* elementtype(float) %3, i64 %4) #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 %0, i64 %1, i64 %2, i32* elementtype(i32) %3, i64 %4) #1

; Function Attrs: nofree noinline nosync nounwind uwtable writeonly
define internal void @init_krh_(i32* noalias nocapture readnone dereferenceable(4) %0) #7 {
  br label %2

2:                                                ; preds = %12, %1
  %3 = phi i64 [ %13, %12 ], [ 1, %1 ]
  br label %4

4:                                                ; preds = %4, %2
  %5 = phi i64 [ %10, %4 ], [ 1, %2 ]
  %6 = add nuw nsw i64 %5, %3
  %7 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 16, i32* elementtype(i32) nonnull getelementptr inbounds ([26 x [4 x i32]], [26 x [4 x i32]]* @"main_$KRH", i64 0, i64 0, i64 0), i64 %5)
  %8 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) nonnull %7, i64 %3)
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
define internal void @bigloop_(float* noalias nocapture readnone dereferenceable(4) %0, i32* noalias nocapture readonly dereferenceable(4) %1) #8 {
  %3 = alloca i32, align 8
  %4 = alloca %"QNCA_a0$float*$rank2$", align 8
  %5 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %4, i64 0, i32 0
  store float* null, float** %5, align 8
  %6 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %4, i64 0, i32 3
  store i64 0, i64* %6, align 8
  store i32 1, i32* %3, align 8
  br label %7

7:                                                ; preds = %7, %2
  %8 = phi i32 [ %9, %7 ], [ 1, %2 ]
  call void @physprop_get_(i32* nonnull %3, %"QNCA_a0$float*$rank2$"* nonnull %4)
  call void @get_hygro_rad_props_(i32* nonnull getelementptr inbounds ([26 x [4 x i32]], [26 x [4 x i32]]* @"main_$KRH", i64 0, i64 0, i64 0), %"QNCA_a0$float*$rank2$"* nonnull %4, float* nonnull getelementptr inbounds ([19 x [26 x [4 x float]]], [19 x [26 x [4 x float]]]* @"main_$TAU", i64 0, i64 0, i64 0, i64 0), i8* undef, i64 undef)
  %9 = add nuw nsw i32 %8, 1
  store i32 %9, i32* %3, align 8
  br label %7
}

; Function Attrs: nofree noreturn nounwind uwtable
define dso_local void @MAIN__() #9 {
  %1 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.5b3c5dff38f14be85a38ea3af0f7d558.0) #11
  tail call void @init_krh_(i32* getelementptr inbounds ([26 x [4 x i32]], [26 x [4 x i32]]* @"main_$KRH", i64 0, i64 0, i64 0))
  tail call void @physprop_init_(i32* nonnull @anon.5b3c5dff38f14be85a38ea3af0f7d558.1)
  tail call void @bigloop_(float* getelementptr inbounds ([19 x [26 x [4 x float]]], [19 x [26 x [4 x float]]]* @"main_$TAU", i64 0, i64 0, i64 0, i64 0), i32* getelementptr inbounds ([26 x [4 x i32]], [26 x [4 x i32]]* @"main_$KRH", i64 0, i64 0, i64 0))
  unreachable
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(i32* nocapture readonly %0) local_unnamed_addr #2

; Function Attrs: mustprogress nofree nosync nounwind readnone willreturn
declare i64 @llvm.ssa.copy.i64(i64 returned %0) #10

attributes #0 = { nofree noinline nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree nosync nounwind readnone speculatable }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { argmemonly mustprogress nofree nounwind willreturn }
attributes #4 = { argmemonly mustprogress nofree nounwind willreturn writeonly }
attributes #5 = { mustprogress nofree noinline nosync nounwind uwtable willreturn "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #6 = { nofree noinline nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #7 = { nofree noinline nosync nounwind uwtable writeonly "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #8 = { nofree noinline noreturn nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #9 = { nofree noreturn nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #10 = { mustprogress nofree nosync nounwind readnone willreturn }
attributes #11 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}

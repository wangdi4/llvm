; REQUIRES: asserts
; RUN: opt < %s -disable-output -passes=dtrans-transpose -debug-only=dtrans-transpose-transform 2>&1 | FileCheck %s

; Check that physpropmod_mp_physprop_ fields 0 and 2 are both transposed
; even though field 1 is not a nested dope vector field. Check
; that the reading and writing of that field with memcpy, memset, and
; for_trim does not invalidate the dope vector analysis.

; Check that the array represented by field 0 of physpropmod_mp_physprop_
; is not transposed because the indirectly subscripted index is not the
; fastest varying subscript.

; CHECK-LABEL: Transform candidate: physpropmod_mp_physprop_[0]
; CHECK-NOT: Before
; CHECK-NOT: After

; Check that the array represented by field 2 of physpropmod_mp_physprop_
; is transposed to ensure that the indirectly subscripted index is not the
; fastest varying subscript.

; CHECK-LABEL: Transform candidate: physpropmod_mp_physprop_[2]
; CHECK-NEXT: Before: MAIN__: %[[N0:[0-9]+]] = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %[[I0:[0-9]+]],
; CHECK-NEXT: After : MAIN__: %[[N0]] = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4,
; CHECK-NEXT: Before: MAIN__: %[[N1:[0-9]+]] = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %[[I1:[0-9]+]],
; CHECK-NEXT: After : MAIN__: %[[N1]] = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 4000,

; Check that the array through which the indirect subscripting is occurring is
; not transposed.

; CHECK-LABEL: Transform candidate: main_$MYK
; CHECK-NOT: Before
; CHECK-NOT: After

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" = type { %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"PHYSPROPMOD$.btPHYSPROP_TYPE" = type { %"QNCA_a0$float*$rank2$", [32 x i8], %"QNCA_a0$float*$rank2$" }
%"QNCA_a0$float*$rank2$" = type { float*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@strlit.1 = internal unnamed_addr constant [7 x i8] c"SULFATE"
@physpropmod_mp_physprop_ = internal global %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" { %"PHYSPROPMOD$.btPHYSPROP_TYPE"* null, i64 0, i64 0, i64 0, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@"main_$MYK" = internal unnamed_addr global [1000 x [19 x i32]] zeroinitializer, align 16
@anon.263b53731fe38c4199c4a10e662745ed.0 = internal unnamed_addr constant i32 2

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #0 {
  %1 = alloca [32 x i8], align 8
  %2 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.263b53731fe38c4199c4a10e662745ed.0) #5
  br label %3

3:                                                ; preds = %12, %0
  %4 = phi i64 [ %13, %12 ], [ 1, %0 ]
  %5 = trunc i64 %4 to i32
  br label %6

6:                                                ; preds = %6, %3
  %7 = phi i64 [ %10, %6 ], [ 1, %3 ]
  %8 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 76, i32* elementtype(i32) getelementptr inbounds ([1000 x [19 x i32]], [1000 x [19 x i32]]* @"main_$MYK", i64 0, i64 0, i64 0), i64 %7)
  %9 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %8, i64 %4)
  store i32 %5, i32* %9, align 1
  %10 = add nuw nsw i64 %7, 1
  %11 = icmp eq i64 %10, 1001
  br i1 %11, label %12, label %6

12:                                               ; preds = %6
  %13 = add nuw nsw i64 %4, 1
  %14 = icmp eq i64 %13, 20
  br i1 %14, label %15, label %3

15:                                               ; preds = %12
  %16 = getelementptr inbounds [32 x i8], [32 x i8]* %1, i64 0, i64 0
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 5), align 8
  store i64 224, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 1), align 8
  store i64 1, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 4), align 8
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 2), align 8
  %17 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, i64* %17, align 1
  %18 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 100, i64* %18, align 1
  %19 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 224, i64* %19, align 1
  store i64 1073741829, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  %20 = tail call i32 @for_allocate_handle(i64 22400, i8** bitcast (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_ to i8**), i32 262144, i8* null) #5
  br label %21

21:                                               ; preds = %21, %15
  %22 = phi i64 [ %84, %21 ], [ 1, %15 ]
  %23 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %24 = load i64, i64* %19, align 1
  %25 = load i64, i64* %17, align 1
  %26 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %25, i64 %24, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %23, i64 %22)
  %27 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %26, i64 0, i32 0
  %28 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %27, i64 0, i32 3
  store i64 5, i64* %28, align 1
  %29 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %27, i64 0, i32 5
  store i64 0, i64* %29, align 1
  %30 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %27, i64 0, i32 1
  store i64 4, i64* %30, align 1
  %31 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %27, i64 0, i32 4
  store i64 2, i64* %31, align 1
  %32 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %27, i64 0, i32 2
  store i64 0, i64* %32, align 1
  %33 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %27, i64 0, i32 6, i64 0
  %34 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %33, i64 0, i32 2
  %35 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %34, i32 0)
  store i64 1, i64* %35, align 1
  %36 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %33, i64 0, i32 0
  %37 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %36, i32 0)
  store i64 19, i64* %37, align 1
  %38 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %34, i32 1)
  store i64 1, i64* %38, align 1
  %39 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %36, i32 1)
  store i64 1000, i64* %39, align 1
  %40 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %33, i64 0, i32 1
  %41 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %40, i32 0)
  store i64 4, i64* %41, align 1
  %42 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %40, i32 1)
  store i64 76, i64* %42, align 1
  %43 = load i64, i64* %28, align 1
  %44 = and i64 %43, -68451041281
  %45 = or i64 %44, 1073741824
  store i64 %45, i64* %28, align 1
  %46 = load i64, i64* %29, align 1
  %47 = inttoptr i64 %46 to i8*
  %48 = bitcast %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %26 to i8**
  %49 = tail call i32 @for_allocate_handle(i64 76000, i8** %48, i32 262144, i8* %47) #5
  %50 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %51 = load i64, i64* %19, align 1
  %52 = load i64, i64* %17, align 1
  %53 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %52, i64 %51, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %50, i64 %22)
  %54 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %53, i64 0, i32 2
  %55 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %54, i64 0, i32 3
  store i64 5, i64* %55, align 1
  %56 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %54, i64 0, i32 5
  store i64 0, i64* %56, align 1
  %57 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %54, i64 0, i32 1
  store i64 4, i64* %57, align 1
  %58 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %54, i64 0, i32 4
  store i64 2, i64* %58, align 1
  %59 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %54, i64 0, i32 2
  store i64 0, i64* %59, align 1
  %60 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %54, i64 0, i32 6, i64 0
  %61 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %60, i64 0, i32 2
  %62 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %61, i32 0)
  store i64 1, i64* %62, align 1
  %63 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %60, i64 0, i32 0
  %64 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %63, i32 0)
  store i64 19, i64* %64, align 1
  %65 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %61, i32 1)
  store i64 1, i64* %65, align 1
  %66 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %63, i32 1)
  store i64 1000, i64* %66, align 1
  %67 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %60, i64 0, i32 1
  %68 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %67, i32 0)
  store i64 4, i64* %68, align 1
  %69 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %67, i32 1)
  store i64 76, i64* %69, align 1
  %70 = load i64, i64* %55, align 1
  %71 = and i64 %70, -68451041281
  %72 = or i64 %71, 1073741824
  store i64 %72, i64* %55, align 1
  %73 = load i64, i64* %56, align 1
  %74 = inttoptr i64 %73 to i8*
  %75 = bitcast %"QNCA_a0$float*$rank2$"* %54 to i8**
  %76 = tail call i32 @for_allocate_handle(i64 76000, i8** nonnull %75, i32 262144, i8* %74) #5
  %77 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %78 = load i64, i64* %19, align 1
  %79 = load i64, i64* %17, align 1
  %80 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %79, i64 %78, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %77, i64 %22)
  %81 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %80, i64 0, i32 1
  %82 = getelementptr [32 x i8], [32 x i8]* %81, i64 0, i64 0
  %83 = getelementptr i8, i8* %82, i64 7
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* noundef nonnull align 1 dereferenceable(7) %82, i8* noundef nonnull align 1 dereferenceable(7) getelementptr inbounds ([7 x i8], [7 x i8]* @strlit.1, i64 0, i64 0), i64 7, i1 false)
  tail call void @llvm.memset.p0i8.i64(i8* noundef nonnull align 1 dereferenceable(25) %83, i8 32, i64 25, i1 false)
  %84 = add nuw nsw i64 %22, 1
  %85 = icmp eq i64 %84, 101
  br i1 %85, label %86, label %21

86:                                               ; preds = %163, %21
  %87 = phi i64 [ %164, %163 ], [ 1, %21 ]
  br label %88

88:                                               ; preds = %160, %86
  %89 = phi i64 [ %161, %160 ], [ 1, %86 ]
  %90 = sub nsw i64 %87, %89
  %91 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 76, i32* elementtype(i32) getelementptr inbounds ([1000 x [19 x i32]], [1000 x [19 x i32]]* @"main_$MYK", i64 0, i64 0, i64 0), i64 %89)
  br label %92

92:                                               ; preds = %157, %88
  %93 = phi i64 [ %158, %157 ], [ 2, %88 ]
  %94 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %95 = load i64, i64* %19, align 1
  %96 = load i64, i64* %17, align 1
  %97 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %96, i64 %95, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %94, i64 101)
  %98 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %97, i64 0, i32 1
  %99 = getelementptr [32 x i8], [32 x i8]* %98, i64 0, i64 0
  %100 = call i64 @for_trim(i8* nonnull %16, i32 32, i8* nonnull %99, i32 32) #5
  %101 = shl i64 %100, 32
  %102 = ashr exact i64 %101, 32
  %103 = call i64 @for_cpstr(i8* nonnull %16, i64 %102, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @strlit.1, i64 0, i64 0), i64 7, i64 2) #5
  %104 = and i64 %103, 1
  %105 = icmp eq i64 %104, 0
  br i1 %105, label %157, label %106

106:                                              ; preds = %92
  %107 = add nsw i64 %90, %93
  %108 = trunc i64 %107 to i32
  %109 = sitofp i32 %108 to float
  %110 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %111 = load i64, i64* %19, align 1
  %112 = load i64, i64* %17, align 1
  %113 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %112, i64 %111, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %110, i64 101)
  %114 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %113, i64 0, i32 2
  %115 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %114, i64 0, i32 0
  %116 = load float*, float** %115, align 1
  %117 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %114, i64 0, i32 6, i64 0
  %118 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %117, i64 0, i32 1
  %119 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %118, i32 0)
  %120 = load i64, i64* %119, align 1
  %121 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %117, i64 0, i32 2
  %122 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %121, i32 0)
  %123 = load i64, i64* %122, align 1
  %124 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %91, i64 %93)
  %125 = load i32, i32* %124, align 1
  %126 = add nsw i32 %125, 1
  %127 = sext i32 %126 to i64
  %128 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %118, i32 1)
  %129 = load i64, i64* %128, align 1
  %130 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %121, i32 1)
  %131 = load i64, i64* %130, align 1
  %132 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %131, i64 %129, float* elementtype(float) %116, i64 %89)
  %133 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %123, i64 %120, float* elementtype(float) %132, i64 %127)
  store float %109, float* %133, align 1
  %134 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %135 = load i64, i64* %19, align 1
  %136 = load i64, i64* %17, align 1
  %137 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %136, i64 %135, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %134, i64 101)
  %138 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %137, i64 0, i32 0
  %139 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %138, i64 0, i32 0
  %140 = load float*, float** %139, align 1
  %141 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %138, i64 0, i32 6, i64 0
  %142 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %141, i64 0, i32 1
  %143 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %142, i32 0)
  %144 = load i64, i64* %143, align 1
  %145 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %141, i64 0, i32 2
  %146 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %145, i32 0)
  %147 = load i64, i64* %146, align 1
  %148 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %142, i32 1)
  %149 = load i64, i64* %148, align 1
  %150 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %145, i32 1)
  %151 = load i64, i64* %150, align 1
  %152 = load i32, i32* %124, align 1
  %153 = add nsw i32 %152, -1
  %154 = sext i32 %153 to i64
  %155 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %151, i64 %149, float* elementtype(float) %140, i64 %154)
  %156 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %147, i64 %144, float* elementtype(float) %155, i64 %89)
  store float %109, float* %156, align 1
  br label %157

157:                                              ; preds = %106, %92
  %158 = add nuw nsw i64 %93, 1
  %159 = icmp eq i64 %158, 19
  br i1 %159, label %160, label %92

160:                                              ; preds = %157
  %161 = add nuw nsw i64 %89, 1
  %162 = icmp eq i64 %161, 1001
  br i1 %162, label %163, label %88

163:                                              ; preds = %160
  %164 = add nuw nsw i64 %87, 1
  %165 = icmp eq i64 %164, 101
  br i1 %165, label %166, label %86

166:                                              ; preds = %163
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(i32* nocapture readonly %0) local_unnamed_addr #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 %0, i64 %1, i64 %2, i32* elementtype(i32) %3, i64 %4) #2

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 %0, i64 %1, i32 %2, i64* elementtype(i64) %3, i32 %4) #2

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64 %0, i8** nocapture %1, i32 %2, i8* %3) local_unnamed_addr #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 %0, i64 %1, i64 %2, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %3, i64 %4) #2

; Function Attrs: argmemonly nofree nounwind willreturn mustprogress
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly %0, i8* noalias nocapture readonly %1, i64 %2, i1 immarg %3) #3

; Function Attrs: argmemonly nofree nounwind willreturn writeonly mustprogress
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly %0, i8 %1, i64 %2, i1 immarg %3) #4

; Function Attrs: nofree
declare dso_local i64 @for_trim(i8* nocapture %0, i32 %1, i8* nocapture readonly %2, i32 %3) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i64 @for_cpstr(i8* nocapture readonly %0, i64 %1, i8* nocapture readonly %2, i64 %3, i64 %4) local_unnamed_addr #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 %0, i64 %1, i64 %2, float* elementtype(float) %3, i64 %4) #2

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nofree nosync nounwind readnone speculatable }
attributes #3 = { argmemonly nofree nounwind willreturn mustprogress }
attributes #4 = { argmemonly nofree nounwind willreturn writeonly mustprogress }
attributes #5 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}

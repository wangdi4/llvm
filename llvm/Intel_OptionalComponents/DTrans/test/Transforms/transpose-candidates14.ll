; REQUIRES: asserts
; RUN: opt < %s -dva-check-dtrans-outofboundsok -dtrans-outofboundsok=false -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck %s

; Check that physpropmod_mp_physprop_ fields 0 and 1 are both valid transpose
; candidates even though the field values were nullified before they were
; allocated. Neither will be profitable for transposing, as the subscripts
; are in the natural order.

; CHECK: Transpose candidate: physpropmod_mp_physprop_
; CHECK: Nested Field Number : 0
; CHECK: IsValid{{ *}}: true
; CHECK: IsProfitable{{ *}}: false

; CHECK: Transpose candidate: physpropmod_mp_physprop_
; CHECK: Nested Field Number : 1
; CHECK: IsValid{{ *}}: true
; CHECK: IsProfitable{{ *}}: false

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" = type { %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"PHYSPROPMOD$.btPHYSPROP_TYPE" = type { %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$", float* }
%"QNCA_a0$float*$rank2$" = type { float*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@physpropmod_mp_physprop_ = internal global %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" { %"PHYSPROPMOD$.btPHYSPROP_TYPE"* null, i64 0, i64 0, i64 0, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@anon.39d3b15ada3f55b3c535c1be4b71a62a.0 = internal unnamed_addr constant i32 2

; Function Attrs: nofree noinline nounwind uwtable
define internal void @physprop_init_(i32* noalias nocapture readonly dereferenceable(4) %0) #0 {
  %2 = alloca i64, align 8
  store i64 5, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 5), align 8
  store i64 200, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 1), align 8
  store i64 1, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 4), align 8
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 2), align 8
  %3 = load i32, i32* %0, align 1
  %4 = sext i32 %3 to i64
  %5 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, i64* %5, align 1
  %6 = icmp sgt i64 %4, 0
  %7 = select i1 %6, i64 %4, i64 0
  %8 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 %7, i64* %8, align 1
  %9 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 200, i64* %9, align 1
  %10 = call i32 (i64*, i32, ...) @for_check_mult_overflow64(i64* nonnull %2, i32 2, i64 %7, i64 200) #7
  %11 = load i64, i64* %2, align 8
  %12 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  %13 = and i64 %12, -68451041281
  %14 = or i64 %13, 1073741824
  store i64 %14, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  %15 = shl i32 %10, 4
  %16 = and i32 %15, 16
  %17 = or i32 %16, 262144
  %18 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 5), align 8
  %19 = inttoptr i64 %18 to i8*
  %20 = tail call i32 @for_allocate_handle(i64 %11, i8** bitcast (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_ to i8**), i32 %17, i8* %19) #7
  %21 = icmp slt i32 %3, 1
  br i1 %21, label %102, label %22

22:                                               ; preds = %1
  %23 = add nuw nsw i32 %3, 1
  %24 = zext i32 %23 to i64
  br label %25

25:                                               ; preds = %25, %22
  %26 = phi i64 [ 1, %22 ], [ %41, %25 ]
  %27 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %28 = load i64, i64* %9, align 1
  %29 = load i64, i64* %5, align 1
  %30 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %29, i64 %28, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %27, i64 %26)
  %31 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %30, i64 0, i32 0
  %32 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %31, i64 0, i32 0
  store float* null, float** %32, align 1
  %33 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %31, i64 0, i32 3
  store i64 0, i64* %33, align 1
  %34 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %35 = load i64, i64* %9, align 1
  %36 = load i64, i64* %5, align 1
  %37 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %36, i64 %35, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %34, i64 %26)
  %38 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %37, i64 0, i32 1
  %39 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %38, i64 0, i32 0
  store float* null, float** %39, align 1
  %40 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %38, i64 0, i32 3
  store i64 0, i64* %40, align 1
  %41 = add nuw nsw i64 %26, 1
  %42 = icmp eq i64 %41, %24
  br i1 %42, label %43, label %25

43:                                               ; preds = %43, %25
  %44 = phi i64 [ %99, %43 ], [ 1, %25 ]
  %45 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %46 = load i64, i64* %9, align 1
  %47 = load i64, i64* %5, align 1
  %48 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %47, i64 %46, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %45, i64 %44)
  %49 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %48, i64 0, i32 0
  %50 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %49, i64 0, i32 3
  store i64 5, i64* %50, align 1
  %51 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %49, i64 0, i32 5
  store i64 0, i64* %51, align 1
  %52 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %49, i64 0, i32 1
  store i64 4, i64* %52, align 1
  %53 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %49, i64 0, i32 4
  store i64 2, i64* %53, align 1
  %54 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %49, i64 0, i32 2
  store i64 0, i64* %54, align 1
  %55 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %49, i64 0, i32 6, i64 0
  %56 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %55, i64 0, i32 2
  %57 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %56, i32 0)
  store i64 1, i64* %57, align 1
  %58 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %55, i64 0, i32 0
  %59 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %58, i32 0)
  store i64 1000, i64* %59, align 1
  %60 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %56, i32 1)
  store i64 1, i64* %60, align 1
  %61 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %58, i32 1)
  store i64 19, i64* %61, align 1
  %62 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %55, i64 0, i32 1
  %63 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %62, i32 0)
  store i64 4, i64* %63, align 1
  %64 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %62, i32 1)
  store i64 4000, i64* %64, align 1
  %65 = load i64, i64* %50, align 1
  %66 = and i64 %65, -68451041281
  %67 = or i64 %66, 1073741824
  store i64 %67, i64* %50, align 1
  %68 = load i64, i64* %51, align 1
  %69 = inttoptr i64 %68 to i8*
  %70 = bitcast %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %48 to i8**
  %71 = tail call i32 @for_allocate_handle(i64 76000, i8** %70, i32 262144, i8* %69) #7
  %72 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %73 = load i64, i64* %9, align 1
  %74 = load i64, i64* %5, align 1
  %75 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %74, i64 %73, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %72, i64 %44)
  %76 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %75, i64 0, i32 1
  %77 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %76, i64 0, i32 3
  store i64 5, i64* %77, align 1
  %78 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %76, i64 0, i32 5
  store i64 0, i64* %78, align 1
  %79 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %76, i64 0, i32 1
  store i64 4, i64* %79, align 1
  %80 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %76, i64 0, i32 4
  store i64 2, i64* %80, align 1
  %81 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %76, i64 0, i32 2
  store i64 0, i64* %81, align 1
  %82 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %76, i64 0, i32 6, i64 0
  %83 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %82, i64 0, i32 2
  %84 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %83, i32 0)
  store i64 1, i64* %84, align 1
  %85 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %82, i64 0, i32 0
  %86 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %85, i32 0)
  store i64 1000, i64* %86, align 1
  %87 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %83, i32 1)
  store i64 1, i64* %87, align 1
  %88 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %85, i32 1)
  store i64 19, i64* %88, align 1
  %89 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %82, i64 0, i32 1
  %90 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %89, i32 0)
  store i64 4, i64* %90, align 1
  %91 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %89, i32 1)
  store i64 4000, i64* %91, align 1
  %92 = load i64, i64* %77, align 1
  %93 = and i64 %92, -68451041281
  %94 = or i64 %93, 1073741824
  store i64 %94, i64* %77, align 1
  %95 = load i64, i64* %78, align 1
  %96 = inttoptr i64 %95 to i8*
  %97 = bitcast %"QNCA_a0$float*$rank2$"* %76 to i8**
  %98 = tail call i32 @for_allocate_handle(i64 76000, i8** nonnull %97, i32 262144, i8* %96) #7
  %99 = add nuw i64 %44, 1
  %100 = trunc i64 %99 to i32
  %101 = icmp slt i32 %3, %100
  br i1 %101, label %102, label %43

102:                                              ; preds = %43, %1
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

; Function Attrs: nofree noinline norecurse nosync nounwind uwtable willreturn writeonly mustprogress
define internal void @physprop_pass_(float* noalias nocapture dereferenceable(4) %0) #3 {
  store float 1.500000e+01, float* %0, align 1
  ret void
}

; Function Attrs: nofree noinline nosync nounwind uwtable
define internal void @physprop_use_(i32* noalias nocapture readonly dereferenceable(4) %0) #4 {
  %2 = load i32, i32* %0, align 1
  %3 = sitofp i32 %2 to float
  %4 = fptosi float %3 to i64
  %5 = icmp slt i64 %4, 1
  br i1 %5, label %70, label %6

6:                                                ; preds = %1
  %7 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  %8 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  br label %9

9:                                                ; preds = %66, %6
  %10 = phi float [ %67, %66 ], [ 1.000000e+00, %6 ]
  %11 = phi i64 [ %68, %66 ], [ %4, %6 ]
  %12 = fptosi float %10 to i32
  %13 = sext i32 %12 to i64
  br label %14

14:                                               ; preds = %63, %9
  %15 = phi i64 [ %64, %63 ], [ 1, %9 ]
  br label %16

16:                                               ; preds = %16, %14
  %17 = phi i64 [ %61, %16 ], [ 1, %14 ]
  %18 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %19 = load i64, i64* %7, align 1
  %20 = load i64, i64* %8, align 1
  %21 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %20, i64 %19, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %18, i64 %13)
  %22 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %21, i64 0, i32 1
  %23 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %22, i64 0, i32 0
  %24 = load float*, float** %23, align 1
  %25 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %22, i64 0, i32 6, i64 0
  %26 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %25, i64 0, i32 1
  %27 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %26, i32 0)
  %28 = load i64, i64* %27, align 1
  %29 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %25, i64 0, i32 2
  %30 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %29, i32 0)
  %31 = load i64, i64* %30, align 1
  %32 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %26, i32 1)
  %33 = load i64, i64* %32, align 1
  %34 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %29, i32 1)
  %35 = load i64, i64* %34, align 1
  %36 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %35, i64 %33, float* elementtype(float) %24, i64 %17)
  %37 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %31, i64 %28, float* elementtype(float) %36, i64 %15)
  %38 = load float, float* %37, align 1
  %39 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %21, i64 0, i32 0
  %40 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %39, i64 0, i32 0
  %41 = load float*, float** %40, align 1
  %42 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %39, i64 0, i32 6, i64 0
  %43 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %42, i64 0, i32 1
  %44 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %43, i32 0)
  %45 = load i64, i64* %44, align 1
  %46 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %42, i64 0, i32 2
  %47 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %46, i32 0)
  %48 = load i64, i64* %47, align 1
  %49 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %43, i32 1)
  %50 = load i64, i64* %49, align 1
  %51 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %46, i32 1)
  %52 = load i64, i64* %51, align 1
  %53 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %52, i64 %50, float* elementtype(float) %41, i64 %17)
  %54 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %48, i64 %45, float* elementtype(float) %53, i64 %15)
  store float %38, float* %54, align 1
  %55 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %56 = load i64, i64* %7, align 1
  %57 = load i64, i64* %8, align 1
  %58 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %57, i64 %56, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %55, i64 %13)
  %59 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %58, i64 0, i32 2
  %60 = load float*, float** %59, align 1
  tail call void @physprop_pass_(float* %60)
  %61 = add nuw nsw i64 %17, 1
  %62 = icmp eq i64 %61, 20
  br i1 %62, label %63, label %16

63:                                               ; preds = %16
  %64 = add nuw nsw i64 %15, 1
  %65 = icmp eq i64 %64, 1001
  br i1 %65, label %66, label %14

66:                                               ; preds = %63
  %67 = fadd reassoc ninf nsz arcp contract afn float %10, 1.000000e+00
  %68 = add nsw i64 %11, -1
  %69 = icmp sgt i64 %11, 1
  br i1 %69, label %9, label %70

70:                                               ; preds = %66, %1
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 %0, i64 %1, i64 %2, float* elementtype(float) %3, i64 %4) #1

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #5 {
  %1 = alloca i32, align 8
  %2 = alloca i32, align 8
  %3 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.39d3b15ada3f55b3c535c1be4b71a62a.0) #7
  call void @physprop_init_(i32* nonnull %2)
  call void @physprop_use_(i32* nonnull %1)
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(i32* nocapture readonly %0) local_unnamed_addr #2

; Function Attrs: nofree nosync nounwind readnone willreturn mustprogress
declare i32 @llvm.ssa.copy.i32(i32 returned %0) #6

; Function Attrs: nofree nosync nounwind readnone willreturn mustprogress
declare i64 @llvm.ssa.copy.i64(i64 returned %0) #6

attributes #0 = { nofree noinline nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nofree nosync nounwind readnone speculatable }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { nofree noinline norecurse nosync nounwind uwtable willreturn writeonly mustprogress "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #4 = { nofree noinline nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #5 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #6 = { nofree nosync nounwind readnone willreturn mustprogress }
attributes #7 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}

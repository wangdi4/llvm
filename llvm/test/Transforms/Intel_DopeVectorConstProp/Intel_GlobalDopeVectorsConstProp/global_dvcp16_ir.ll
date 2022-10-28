; RUN: opt < %s -S -passes=dopevectorconstprop -debug-only=dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

; Check that dope vector constant propagation will happen for the loads
; indicated in the trace, but in particular within @myoutput, because it
; is not called with multiple arrays of different sizes.

; This test is similar to global_dvcp16.ll, but checks the IR in @myoutput.

; The IR below was generated from the following Fortran source code:
;
;        module physpropmod
;        type :: physprop_type
;        real, pointer :: sw_hygro_ext(:,:)
;        endtype physprop_type
;        type (physprop_type), pointer :: physprop(:)
;        integer, parameter :: physpropcount = 10
;        integer, parameter :: ndvsize = 10
;        end module
;
;        subroutine myinit(myinteger)
;        use physpropmod
;        integer, intent(in) :: myinteger
;        do i = 1, ndvsize
;          do j = 1, ndvsize
;            physprop(myinteger)%sw_hygro_ext(i,j) = i + j
;          end do
;        end do
;        print *, physprop(myinteger)%sw_hygro_ext
;        end subroutine
;
;        program main
;        use physpropmod
;        interface
;        subroutine myinit(myinteger)
;        integer, intent(in) :: myinteger
;        end subroutine
;        subroutine myoutput(myallarray)
;        real, intent(in) :: myallarray(:,:)
;        end subroutine
;        end interface
;        real, allocatable, target :: mylocalarray(:,:)
;        allocate(physprop(physpropcount))
;        allocate(mylocalarray(5,5))
;        mylocalarray = 5
;        do i = 1, phypropscount
;          allocate(physprop(i)%sw_hygro_ext(ndvsize,ndvsize))
;          call myinit(i)
;          call myoutput(physprop(i)%sw_hygro_ext)
;        end do
;        end program
;        subroutine myoutput(myallarray)
;        real, intent(in) :: myallarray(:,:)
;        integer lb1, lb2
;        lb1 = lbound(myallarray, 1)
;        lb1 = lbound(myallarray, 2)
;        print *, myallarray(lb1, lb2)
;        end subroutine

; CHECK: define internal void @myoutput
; CHECK: tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 40
; CHECK: tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" = type { %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE" = type { %"__DTRT_QNCA_a0$float*$rank2$" }
%"__DTRT_QNCA_a0$float*$rank2$" = type { float*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@anon.2bc5e417881711fb3ab09da8e681987f.0 = internal unnamed_addr constant i32 2
@physpropmod_mp_physprop_ = internal global %"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" { %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"* null, i64 0, i64 0, i64 0, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@"main_$MYLOCALARRAY" = internal global %"__DTRT_QNCA_a0$float*$rank2$" { float* null, i64 0, i64 0, i64 1073741952, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }

; Function Attrs: nofree noinline nounwind uwtable
define internal void @myinit_(i32* noalias nocapture readonly dereferenceable(4) %0) #0 {
  %2 = alloca [8 x i64], align 16
  %3 = alloca [4 x i8], align 1
  %4 = alloca { i64, i8* }, align 8
  %5 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i32 0, i32 6, i32 0, i32 1), i32 0)
  %6 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i32 0, i32 6, i32 0, i32 2), i32 0)
  %7 = load i32, i32* %0, align 1
  %8 = sext i32 %7 to i64
  br label %9

9:                                                ; preds = %38, %1
  %10 = phi i64 [ %39, %38 ], [ 1, %1 ]
  br label %11

11:                                               ; preds = %11, %9
  %12 = phi i64 [ %36, %11 ], [ 1, %9 ]
  %13 = add nuw nsw i64 %12, %10
  %14 = trunc i64 %13 to i32
  %15 = sitofp i32 %14 to float
  %16 = load %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i32 0, i32 0), align 1
  %17 = load i64, i64* %5, align 1
  %18 = load i64, i64* %6, align 1
  %19 = tail call %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s___DTRT_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s___DTRT_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %18, i64 %17, %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE") %16, i64 %8)
  %20 = getelementptr inbounds %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE", %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"* %19, i32 0, i32 0
  %21 = getelementptr inbounds %"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* %20, i32 0, i32 0
  %22 = load float*, float** %21, align 1
  %23 = getelementptr inbounds %"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* %20, i32 0, i32 6, i32 0
  %24 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %23, i32 0, i32 1
  %25 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %24, i32 0)
  %26 = load i64, i64* %25, align 1
  %27 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %23, i32 0, i32 2
  %28 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %27, i32 0)
  %29 = load i64, i64* %28, align 1
  %30 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %24, i32 1)
  %31 = load i64, i64* %30, align 1
  %32 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %27, i32 1)
  %33 = load i64, i64* %32, align 1
  %34 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %33, i64 %31, float* elementtype(float) %22, i64 %12)
  %35 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %29, i64 %26, float* elementtype(float) %34, i64 %10)
  store float %15, float* %35, align 1
  %36 = add nuw nsw i64 %12, 1
  %37 = icmp ne i64 %36, 11
  br i1 %37, label %11, label %38

38:                                               ; preds = %11
  %39 = add nuw nsw i64 %10, 1
  %40 = icmp ne i64 %39, 11
  br i1 %40, label %9, label %41

41:                                               ; preds = %38
  %42 = tail call i8* @llvm.stacksave()
  %43 = load %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i32 0, i32 0), align 1
  %44 = load i64, i64* %5, align 1
  %45 = load i64, i64* %6, align 1
  %46 = tail call %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s___DTRT_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s___DTRT_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %45, i64 %44, %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE") %43, i64 %8)
  %47 = getelementptr inbounds %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE", %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"* %46, i32 0, i32 0
  %48 = getelementptr inbounds %"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* %47, i32 0, i32 3
  %49 = load i64, i64* %48, align 1
  %50 = and i64 %49, 4
  %51 = icmp ne i64 %50, 0
  %52 = getelementptr inbounds %"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* %47, i32 0, i32 6, i32 0
  %53 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %52, i32 0, i32 1
  %54 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %53, i32 0)
  %55 = load i64, i64* %54, align 1, !range !3
  %56 = getelementptr inbounds %"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* %47, i32 0, i32 1
  %57 = load i64, i64* %56, align 1
  %58 = icmp eq i64 %55, %57
  %59 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %53, i32 1)
  %60 = load i64, i64* %59, align 1, !range !3
  %61 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %52, i32 0, i32 0
  %62 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %61, i32 0)
  %63 = load i64, i64* %62, align 1
  %64 = mul nsw i64 %63, %55
  %65 = icmp eq i64 %60, %64
  %66 = and i1 %58, %65
  %67 = or i1 %51, %66
  %68 = xor i1 %67, true
  %69 = getelementptr inbounds %"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* %47, i32 0, i32 0
  %70 = load float*, float** %69, align 1
  %71 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %52, i32 0, i32 2
  %72 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %71, i32 0)
  %73 = load i64, i64* %72, align 1
  %74 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %71, i32 1)
  %75 = load i64, i64* %74, align 1
  %76 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %61, i32 1)
  %77 = load i64, i64* %76, align 1
  %78 = mul nsw i64 %63, 4
  br i1 %68, label %79, label %111

79:                                               ; preds = %41
  %80 = mul nsw i64 %77, %78
  %81 = sdiv i64 %80, 4
  %82 = alloca float, i64 %81, align 4
  %83 = icmp sle i64 1, %77
  br i1 %83, label %84, label %109

84:                                               ; preds = %79
  %85 = icmp sle i64 1, %63
  %86 = add i64 %63, 1
  %87 = add nuw i64 %77, 1
  br label %102

88:                                               ; preds = %105, %88
  %89 = phi i64 [ %73, %105 ], [ %94, %88 ]
  %90 = phi i64 [ 1, %105 ], [ %95, %88 ]
  %91 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %73, i64 %55, float* elementtype(float) %106, i64 %89)
  %92 = load float, float* %91, align 1
  %93 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* nonnull elementtype(float) %107, i64 %90)
  store float %92, float* %93, align 1
  %94 = add nsw i64 %89, 1
  %95 = add nuw nsw i64 %90, 1
  %96 = icmp ne i64 %95, %86
  br i1 %96, label %88, label %97

97:                                               ; preds = %88
  br label %98

98:                                               ; preds = %102, %97
  %99 = add nsw i64 %103, 1
  %100 = add nuw nsw i64 %104, 1
  %101 = icmp ne i64 %100, %87
  br i1 %101, label %102, label %108

102:                                              ; preds = %98, %84
  %103 = phi i64 [ %75, %84 ], [ %99, %98 ]
  %104 = phi i64 [ 1, %84 ], [ %100, %98 ]
  br i1 %85, label %105, label %98

105:                                              ; preds = %102
  %106 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %75, i64 %60, float* elementtype(float) %70, i64 %103)
  %107 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %78, float* nonnull elementtype(float) %82, i64 %104)
  br label %88

108:                                              ; preds = %98
  br label %109

109:                                              ; preds = %108, %79
  %110 = bitcast float* %82 to i8*
  br label %114

111:                                              ; preds = %41
  %112 = bitcast float* %70 to i8*
  %113 = mul nsw i64 %77, %78
  br label %114

114:                                              ; preds = %111, %109
  %115 = phi i64 [ %113, %111 ], [ %80, %109 ]
  %116 = phi i8* [ %110, %109 ], [ %112, %111 ]
  %117 = getelementptr inbounds [4 x i8], [4 x i8]* %3, i32 0, i32 0
  store i8 26, i8* %117, align 1
  %118 = getelementptr inbounds [4 x i8], [4 x i8]* %3, i32 0, i32 1
  store i8 5, i8* %118, align 1
  %119 = getelementptr inbounds [4 x i8], [4 x i8]* %3, i32 0, i32 2
  store i8 1, i8* %119, align 1
  %120 = getelementptr inbounds [4 x i8], [4 x i8]* %3, i32 0, i32 3
  store i8 0, i8* %120, align 1
  %121 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %4, i32 0, i32 0
  store i64 %115, i64* %121, align 1
  %122 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %4, i32 0, i32 1
  store i8* %116, i8** %122, align 1
  %123 = bitcast [8 x i64]* %2 to i8*
  %124 = bitcast [4 x i8]* %3 to i8*
  %125 = bitcast { i64, i8* }* %4 to i8*
  %126 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %123, i32 -1, i64 1239157112576, i8* nonnull %124, i8* nonnull %125)
  call void @llvm.stackrestore(i8* %42)
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 %0, i64 %1, i32 %2, i64* elementtype(i64) %3, i32 %4) #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 %0, i64 %1, i64 %2, float* elementtype(float) %3, i64 %4) #1

; Function Attrs: mustprogress nofree nosync nounwind willreturn
declare i8* @llvm.stacksave() #2

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(i8* %0, i32 %1, i64 %2, i8* %3, i8* %4, ...) #3

; Function Attrs: mustprogress nofree nosync nounwind willreturn
declare void @llvm.stackrestore(i8* %0) #2

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #4 {
  %1 = alloca i32, align 8
  %2 = tail call i32 @for_set_reentrancy(i32* @anon.2bc5e417881711fb3ab09da8e681987f.0)
  store i64 0, i64* getelementptr inbounds (%"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i32 0, i32 5), align 1
  store i64 96, i64* getelementptr inbounds (%"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i32 0, i32 1), align 1
  store i64 1, i64* getelementptr inbounds (%"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i32 0, i32 4), align 1
  store i64 0, i64* getelementptr inbounds (%"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i32 0, i32 2), align 1
  %3 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i32 0, i32 6, i32 0, i32 2), i32 0)
  store i64 1, i64* %3, align 1
  %4 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i32 0, i32 6, i32 0, i32 0), i32 0)
  store i64 10, i64* %4, align 1
  %5 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i32 0, i32 6, i32 0, i32 1), i32 0)
  store i64 96, i64* %5, align 1
  store i64 1073741829, i64* getelementptr inbounds (%"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i32 0, i32 3), align 1
  %6 = tail call i32 @for_allocate_handle(i64 960, i8** bitcast (%"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_ to i8**), i32 262144, i8* null)
  %7 = load i64, i64* getelementptr inbounds (%"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* @"main_$MYLOCALARRAY", i32 0, i32 3), align 1
  %8 = and i64 %7, 256
  %9 = lshr i64 %8, 8
  %10 = shl nuw nsw i64 %9, 8
  %11 = add nuw nsw i64 %10, 133
  %12 = and i64 %7, 1030792151040
  %13 = lshr i64 %12, 36
  %14 = and i64 %11, -1030792151041
  %15 = shl nuw nsw i64 %13, 36
  %16 = add nuw nsw i64 %14, %15
  store i64 0, i64* getelementptr inbounds (%"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* @"main_$MYLOCALARRAY", i32 0, i32 5), align 1
  store i64 4, i64* getelementptr inbounds (%"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* @"main_$MYLOCALARRAY", i32 0, i32 1), align 1
  store i64 2, i64* getelementptr inbounds (%"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* @"main_$MYLOCALARRAY", i32 0, i32 4), align 1
  store i64 0, i64* getelementptr inbounds (%"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* @"main_$MYLOCALARRAY", i32 0, i32 2), align 1
  %17 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* @"main_$MYLOCALARRAY", i32 0, i32 6, i32 0, i32 2), i32 0)
  store i64 1, i64* %17, align 1
  %18 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* @"main_$MYLOCALARRAY", i32 0, i32 6, i32 0, i32 0), i32 0)
  store i64 5, i64* %18, align 1
  %19 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* @"main_$MYLOCALARRAY", i32 0, i32 6, i32 0, i32 2), i32 1)
  store i64 1, i64* %19, align 1
  %20 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* @"main_$MYLOCALARRAY", i32 0, i32 6, i32 0, i32 0), i32 1)
  store i64 5, i64* %20, align 1
  %21 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* @"main_$MYLOCALARRAY", i32 0, i32 6, i32 0, i32 1), i32 0)
  store i64 4, i64* %21, align 1
  %22 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* @"main_$MYLOCALARRAY", i32 0, i32 6, i32 0, i32 1), i32 1)
  store i64 20, i64* %22, align 1
  %23 = and i64 %16, -68451041281
  %24 = or i64 %23, 1073741824
  store i64 %24, i64* getelementptr inbounds (%"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* @"main_$MYLOCALARRAY", i32 0, i32 3), align 1
  %25 = and i64 %16, 1
  %26 = shl nuw nsw i64 %25, 1
  %27 = trunc i64 %26 to i32
  %28 = and i32 %27, -2097169
  %29 = and i64 %16, 1030792151040
  %30 = lshr i64 %29, 36
  %31 = and i32 %28, -31457281
  %32 = shl nuw nsw i64 %30, 21
  %33 = trunc i64 %32 to i32
  %34 = or i32 %31, %33
  %35 = and i64 %16, 1099511627776
  %36 = lshr i64 %35, 40
  %37 = and i32 %34, -33554433
  %38 = shl nuw nsw i64 %36, 25
  %39 = trunc i64 %38 to i32
  %40 = or i32 %37, %39
  %41 = and i32 %40, -2031617
  %42 = or i32 %41, 262144
  %43 = tail call i32 @for_alloc_allocatable_handle(i64 100, i8** bitcast (%"__DTRT_QNCA_a0$float*$rank2$"* @"main_$MYLOCALARRAY" to i8**), i32 %42, i8* null)
  %44 = load i64, i64* %17, align 1
  %45 = load i64, i64* %19, align 1
  %46 = load i64, i64* %22, align 1, !range !3
  %47 = load float*, float** getelementptr inbounds (%"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* @"main_$MYLOCALARRAY", i32 0, i32 0), align 1
  %48 = load i64, i64* %18, align 1
  %49 = load i64, i64* %20, align 1
  %50 = icmp sle i64 1, %49
  br i1 %50, label %51, label %73

51:                                               ; preds = %0
  %52 = icmp sle i64 1, %48
  %53 = add i64 %48, 1
  %54 = add nuw i64 %49, 1
  br label %67

55:                                               ; preds = %70, %55
  %56 = phi i64 [ 1, %70 ], [ %60, %55 ]
  %57 = phi i64 [ %44, %70 ], [ %59, %55 ]
  %58 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %44, i64 4, float* elementtype(float) %71, i64 %57)
  store float 5.000000e+00, float* %58, align 1
  %59 = add nsw i64 %57, 1
  %60 = add nuw nsw i64 %56, 1
  %61 = icmp ne i64 %60, %53
  br i1 %61, label %55, label %62

62:                                               ; preds = %55
  br label %63

63:                                               ; preds = %67, %62
  %64 = add nsw i64 %68, 1
  %65 = add nuw nsw i64 %69, 1
  %66 = icmp ne i64 %65, %54
  br i1 %66, label %67, label %72

67:                                               ; preds = %63, %51
  %68 = phi i64 [ %45, %51 ], [ %64, %63 ]
  %69 = phi i64 [ 1, %51 ], [ %65, %63 ]
  br i1 %52, label %70, label %63

70:                                               ; preds = %67
  %71 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %45, i64 %46, float* elementtype(float) %47, i64 %68)
  br label %55

72:                                               ; preds = %63
  br label %73

73:                                               ; preds = %72, %0
  store i32 1, i32* %1, align 1
  br i1 false, label %74, label %75

74:                                               ; preds = %73
  br label %117

75:                                               ; preds = %73
  br label %76

76:                                               ; preds = %76, %75
  %77 = phi i32 [ %115, %76 ], [ 1, %75 ]
  %78 = load %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i32 0, i32 0), align 1
  %79 = load i64, i64* %5, align 1
  %80 = load i64, i64* %3, align 1
  %81 = sext i32 %77 to i64
  %82 = call %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s___DTRT_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s___DTRT_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %80, i64 %79, %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE") %78, i64 %81)
  %83 = getelementptr inbounds %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE", %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"* %82, i32 0, i32 0
  %84 = getelementptr inbounds %"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* %83, i32 0, i32 3
  store i64 5, i64* %84, align 1
  %85 = getelementptr inbounds %"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* %83, i32 0, i32 5
  store i64 0, i64* %85, align 1
  %86 = getelementptr inbounds %"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* %83, i32 0, i32 1
  store i64 4, i64* %86, align 1
  %87 = getelementptr inbounds %"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* %83, i32 0, i32 4
  store i64 2, i64* %87, align 1
  %88 = getelementptr inbounds %"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* %83, i32 0, i32 2
  store i64 0, i64* %88, align 1
  %89 = getelementptr inbounds %"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* %83, i32 0, i32 6, i32 0
  %90 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %89, i32 0, i32 2
  %91 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %90, i32 0)
  store i64 1, i64* %91, align 1
  %92 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %89, i32 0, i32 0
  %93 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %92, i32 0)
  store i64 10, i64* %93, align 1
  %94 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %90, i32 1)
  store i64 1, i64* %94, align 1
  %95 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %92, i32 1)
  store i64 10, i64* %95, align 1
  %96 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %89, i32 0, i32 1
  %97 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %96, i32 0)
  store i64 4, i64* %97, align 1
  %98 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %96, i32 1)
  store i64 40, i64* %98, align 1
  %99 = getelementptr inbounds %"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* %83, i32 0, i32 0
  %100 = load i64, i64* %84, align 1
  %101 = and i64 %100, -68451041281
  %102 = or i64 %101, 1073741824
  store i64 %102, i64* %84, align 1
  %103 = load i64, i64* %85, align 1
  %104 = inttoptr i64 %103 to i8*
  %105 = bitcast float** %99 to i8**
  %106 = call i32 @for_allocate_handle(i64 400, i8** %105, i32 262144, i8* %104)
  call void @myinit_(i32* nonnull %1)
  %107 = load %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"__DTRT_QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i32 0, i32 0), align 1
  %108 = load i64, i64* %5, align 1
  %109 = load i64, i64* %3, align 1
  %110 = load i32, i32* %1, align 1
  %111 = sext i32 %110 to i64
  %112 = call %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s___DTRT_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s___DTRT_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %109, i64 %108, %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE") %107, i64 %111)
  %113 = getelementptr inbounds %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE", %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"* %112, i32 0, i32 0
  call void @myoutput_.1(%"__DTRT_QNCA_a0$float*$rank2$"* %113)
  %114 = load i32, i32* %1, align 1
  %115 = add nsw i32 %114, 1
  store i32 %115, i32* %1, align 1
  br i1 true, label %76, label %116

116:                                              ; preds = %76
  br label %117

117:                                              ; preds = %116, %74
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(i32* nocapture readonly %0) #3

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64 %0, i8** nocapture %1, i32 %2, i8* %3) #3

; Function Attrs: nofree
declare dso_local i32 @for_alloc_allocatable_handle(i64 %0, i8** nocapture %1, i32 %2, i8* %3) #3

; Function Attrs: nofree nosync nounwind readnone speculatable
declare %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s___DTRT_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s___DTRT_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 %0, i64 %1, i64 %2, %"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"__DTRT_PHYSPROPMOD$.btPHYSPROP_TYPE") %3, i64 %4) #1

; Function Attrs: nofree noinline nounwind uwtable
define internal void @myoutput_.1(%"__DTRT_QNCA_a0$float*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %0) #0 {
  %2 = alloca [8 x i64], align 16
  %3 = alloca [4 x i8], align 1
  %4 = alloca { float }, align 8
  %5 = getelementptr inbounds %"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* %0, i32 0, i32 6, i32 0
  br label %6

6:                                                ; preds = %1
  br label %8

7:                                                ; No predecessors!
  br label %8

8:                                                ; preds = %7, %6
  br label %9

9:                                                ; preds = %8
  br label %11

10:                                               ; No predecessors!
  br label %11

11:                                               ; preds = %10, %9
  %12 = getelementptr inbounds %"__DTRT_QNCA_a0$float*$rank2$", %"__DTRT_QNCA_a0$float*$rank2$"* %0, i32 0, i32 0
  %13 = load float*, float** %12, align 1
  %14 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %5, i32 0, i32 1
  %15 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %14, i32 0)
  %16 = load i64, i64* %15, align 1
  %17 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %14, i32 1)
  %18 = load i64, i64* %17, align 1
  %19 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %18, float* elementtype(float) %13, i64 0)
  %20 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 %16, float* nonnull elementtype(float) %19, i64 1)
  %21 = load float, float* %20, align 1
  %22 = getelementptr inbounds [4 x i8], [4 x i8]* %3, i32 0, i32 0
  store i8 26, i8* %22, align 1
  %23 = getelementptr inbounds [4 x i8], [4 x i8]* %3, i32 0, i32 1
  store i8 1, i8* %23, align 1
  %24 = getelementptr inbounds [4 x i8], [4 x i8]* %3, i32 0, i32 2
  store i8 1, i8* %24, align 1
  %25 = getelementptr inbounds [4 x i8], [4 x i8]* %3, i32 0, i32 3
  store i8 0, i8* %25, align 1
  %26 = getelementptr inbounds { float }, { float }* %4, i32 0, i32 0
  store float %21, float* %26, align 1
  %27 = bitcast [8 x i64]* %2 to i8*
  %28 = bitcast [4 x i8]* %3 to i8*
  %29 = bitcast { float }* %4 to i8*
  %30 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %27, i32 -1, i64 1239157112576, i8* nonnull %28, i8* nonnull %29)
  ret void
}

attributes #0 = { nofree noinline nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree nosync nounwind readnone speculatable }
attributes #2 = { mustprogress nofree nosync nounwind willreturn }
attributes #3 = { nofree "intel-lang"="fortran" }
attributes #4 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
!3 = !{i64 1, i64 -9223372036854775808}

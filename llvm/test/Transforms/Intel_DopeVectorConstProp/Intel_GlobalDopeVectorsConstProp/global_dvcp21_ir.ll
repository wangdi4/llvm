; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -passes=dopevectorconstprop -dope-vector-global-const-prop=true -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -dvcp-tile-mv-min-calls=3 -S 2>&1 | FileCheck %s

; This test case checks that DVCP was applied even if aggressive DVCP
; is disabled due to function @bar calls @foo 3 times, and the calls
; are marked as "prefer-inline-tile-choice". This test case was created
; from the following code, with functions @bar and @foo added.

;      MODULE ARR_MOD
;         REAL, POINTER :: A (:,:)
;
;         CONTAINS
;
;         SUBROUTINE ALLOCATE_ARR()
;           ALLOCATE(A(10, 10))
;
;           RETURN
;         END SUBROUTINE ALLOCATE_ARR
;
;         SUBROUTINE INITIALIZE_ARR(N, M)
;           INTEGER, INTENT(IN) :: N, M
;
;           DO i = 1, N
;             DO j = 1, M
;               A(j, i) = i
;             END DO
;           END DO
;
;           RETURN
;         END SUBROUTINE INITIALIZE_ARR
;
;         SUBROUTINE PRINT_ARR(N, M)
;           INTEGER, INTENT(IN) :: N, M
;
;           DO i = 1, N
;             DO j = 1, M
;               print *, A(j,i)
;             END DO
;           END DO
;           RETURN
;         END SUBROUTINE
;
;      END MODULE
;
;      PROGRAM main
;        USE ARR_MOD
;        IMPLICIT NONE
;
;        CALL ALLOCATE_ARR()
;        CALL INITIALIZE_ARR(10, 10)
;        CALL PRINT_ARR(10, 10)
;      END

; ifx -xCORE-AVX512 -Ofast -flto arr.f90 -mllvm -debug-only=dope-vector-global-const-prop

; The test case basically allocates the global array A in ALLOCATE_ARR, then
; initializes it in INITIALIZE_ARR and the use will be in PRINT_ARR. It is the
; same test case as global_dvcp21.ll, but it checks the IR.

; CHECK: define internal void @arr_mod_mp_initialize_arr_
; CHECK:   %18 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 40, float* elementtype(float) %13, i64 %8)
; CHECK:   %19 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %18, i64 %12)

; CHECK: define internal void @arr_mod_mp_print_arr_
; CHECK:  %26 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 40, float* elementtype(float) %21, i64 %18)
; CHECK:  %27 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %26, i64 %20)

; ModuleID = 'ld-temp.o'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$float*$rank2$" = type { float*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@arr_mod_mp_a_ = internal global %"QNCA_a0$float*$rank2$" { float* null, i64 0, i64 0, i64 0, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }
@anon.87529b4ebf98830a9107fed24e462e82.0 = internal unnamed_addr constant i32 2
@anon.87529b4ebf98830a9107fed24e462e82.1 = internal unnamed_addr constant i32 10

; Function Attrs: nofree noinline nounwind uwtable
define internal void @arr_mod_mp_allocate_arr_() #0 {
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @arr_mod_mp_a_, i64 0, i32 5), align 8
  store i64 4, i64* getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @arr_mod_mp_a_, i64 0, i32 1), align 8
  store i64 2, i64* getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @arr_mod_mp_a_, i64 0, i32 4), align 16
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @arr_mod_mp_a_, i64 0, i32 2), align 16
  %1 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, i64* %1, align 1
  %2 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 10, i64* %2, align 1
  %3 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 1)
  store i64 1, i64* %3, align 1
  %4 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 0), i32 1)
  store i64 10, i64* %4, align 1
  %5 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 4, i64* %5, align 1
  %6 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 1)
  store i64 40, i64* %6, align 1
  store i64 1073741829, i64* getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @arr_mod_mp_a_, i64 0, i32 3), align 8
  %7 = tail call i32 @for_allocate_handle(i64 400, i8** bitcast (%"QNCA_a0$float*$rank2$"* @arr_mod_mp_a_ to i8**), i32 262144, i8* null) #3
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #1

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, i8** nocapture, i32, i8*) local_unnamed_addr #2

; Function Attrs: nofree noinline nounwind uwtable
define internal void @arr_mod_mp_initialize_arr_(i32* noalias nocapture readonly dereferenceable(4) %0, i32* noalias nocapture readonly dereferenceable(4) %1) #0 {
  %3 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 0)
  %4 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %5 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 1)
  %6 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 1)
  br label %7

7:                                                ; preds = %22, %2
  %8 = phi i64 [ 1, %2 ], [ %23, %22 ]
  %9 = trunc i64 %8 to i32
  %10 = sitofp i32 %9 to float
  br label %11

11:                                               ; preds = %11, %7
  %12 = phi i64 [ 1, %7 ], [ %20, %11 ]
  %13 = load float*, float** getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %14 = load i64, i64* %3, align 1
  %15 = load i64, i64* %4, align 1
  %16 = load i64, i64* %5, align 1
  %17 = load i64, i64* %6, align 1
  %18 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %17, i64 %16, float* elementtype(float) %13, i64 %8)
  %19 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %15, i64 %14, float* elementtype(float) %18, i64 %12)
  store float %10, float* %19, align 1
  %20 = add nuw nsw i64 %12, 1
  %21 = icmp eq i64 %20, 11
  br i1 %21, label %22, label %11

22:                                               ; preds = %11
  %23 = add nuw nsw i64 %8, 1
  %24 = icmp eq i64 %23, 11
  br i1 %24, label %25, label %7

25:                                               ; preds = %22
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #1

; Function Attrs: nofree noinline nounwind uwtable
define internal void @arr_mod_mp_print_arr_(i32* noalias nocapture readonly dereferenceable(4) %0, i32* noalias nocapture readonly dereferenceable(4) %1) #0 {
  %3 = alloca [8 x i64], align 16
  %4 = alloca [4 x i8], align 1
  %5 = alloca { float }, align 8
  %6 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 0)
  %7 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %8 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 1)
  %9 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 1)
  %10 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 0
  %11 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 1
  %12 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 2
  %13 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 3
  %14 = getelementptr inbounds { float }, { float }* %5, i64 0, i32 0
  %15 = bitcast [8 x i64]* %3 to i8*
  %16 = bitcast { float }* %5 to i8*
  br label %17

17:                                               ; preds = %33, %2
  %18 = phi i64 [ 1, %2 ], [ %34, %33 ]
  br label %19

19:                                               ; preds = %17, %19
  %20 = phi i64 [ %30, %19 ], [ 1, %17 ]
  %21 = load float*, float** getelementptr inbounds (%"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %22 = load i64, i64* %6, align 1
  %23 = load i64, i64* %7, align 1
  %24 = load i64, i64* %8, align 1
  %25 = load i64, i64* %9, align 1
  %26 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %25, i64 %24, float* elementtype(float) %21, i64 %18)
  %27 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %23, i64 %22, float* elementtype(float) %26, i64 %20)
  %28 = load float, float* %27, align 1
  store i8 26, i8* %10, align 1
  store i8 1, i8* %11, align 1
  store i8 1, i8* %12, align 1
  store i8 0, i8* %13, align 1
  store float %28, float* %14, align 8
  %29 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %15, i32 -1, i64 1239157112576, i8* nonnull %10, i8* nonnull %16) #3
  %30 = add nuw i64 %20, 1
  %31 = trunc i64 %30 to i32
  %32 = icmp slt i32 10, %31
  br i1 %32, label %33, label %19

33:                                               ; preds = %19
  %34 = add nuw i64 %18, 1
  %35 = trunc i64 %34 to i32
  %36 = icmp slt i32 10, %35
  br i1 %36, label %37, label %17

37:                                               ; preds = %33
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr #2

; Function Attrs: nofree noinline nounwind uwtable
define dso_local void @MAIN__() #0 {
  %1 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.87529b4ebf98830a9107fed24e462e82.0) #3
  tail call void @arr_mod_mp_allocate_arr_()
  tail call void @arr_mod_mp_initialize_arr_(i32* nonnull @anon.87529b4ebf98830a9107fed24e462e82.1, i32* nonnull @anon.87529b4ebf98830a9107fed24e462e82.1)
  tail call void @arr_mod_mp_print_arr_(i32* nonnull @anon.87529b4ebf98830a9107fed24e462e82.1, i32* nonnull @anon.87529b4ebf98830a9107fed24e462e82.1)
  ret void
}

define dso_local void @foo() {
  ret void
}

define dso_local void @bar() {
  call void @foo() #4
  call void @foo() #4
  call void @foo() #4

  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(i32* nocapture readonly) local_unnamed_addr #2

attributes #0 = { nofree noinline nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { nounwind }
attributes #4 = { "prefer-inline-tile-choice" }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}

; end INTEL_FEATURE_SW_ADVANCED
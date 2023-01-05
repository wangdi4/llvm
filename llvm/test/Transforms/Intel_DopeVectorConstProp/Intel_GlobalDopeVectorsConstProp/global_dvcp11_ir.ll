; RUN: opt < %s -passes=dopevectorconstprop -dope-vector-global-const-prop=true -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s

; This test case checks that the analysis for the global dope vector
; fails since there is one nested dope vector that wasn't initialized.
; It is the same test case as glob_dvcp11.ll but it checks the IR.
; This test was created from the following source code:

;      MODULE ARR_MOD
;
;         TYPE T_TESTTYPE
;
;           REAL, POINTER :: inner_array_A(:,:)
;           REAL, POINTER :: inner_array_B(:,:,:)
;           REAL, POINTER :: inner_array_C(:)
;
;         END TYPE T_TESTTYPE
;
;         TYPE (T_TESTTYPE), ALLOCATABLE :: A (:)
;
;         CONTAINS
;
;         SUBROUTINE ALLOCATE_ARR(I)
;           INTEGER, INTENT(IN) :: I
;
;           IF(I.eq.1) ALLOCATE(A(I))
;
;           ALLOCATE(A(I) % inner_array_B(10, 10, 10))
;           ALLOCATE(A(I) % inner_array_C(10))
;
;           RETURN
;         END SUBROUTINE ALLOCATE_ARR
;
;         SUBROUTINE INITIALIZE_ARR(I, N, M, O)
;           INTEGER, INTENT(IN) :: I, N, M, O
;
;           DO j = 1, N
;             DO k = 1, M
;               A(I) % inner_array_A(k, j) = j
;               DO l = 1, O
;                 A(I) % inner_array_B(k, j, l) = j
;               END DO
;             END DO
;             A(I) % inner_array_C(k) = k
;           END DO
;
;           RETURN
;         END SUBROUTINE INITIALIZE_ARR
;
;         SUBROUTINE PRINT_ARR(I, N, M, O)
;           INTEGER, INTENT(IN) :: I, N, M, O
;
;           DO j = 1, N
;             DO k = 1, M
;               print *, A(I) % inner_array_A(k, j)
;               DO l = 1, O
;                 print *, A(I) % inner_array_B(k, j, l)
;               END DO
;             END DO
;             print *, A(I) % inner_array_C(k)
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
;        INTEGER :: I
;
;        DO I = 1, 10
;          CALL ALLOCATE_ARR(I)
;          CALL INITIALIZE_ARR(I, 10, 10, 10)
;          CALL PRINT_ARR(I, 10, 10, 10)
;        END DO
;      END

; ifx -xCORE-AVX512 -Ofast -flto arr.f90 -mllvm -debug-only=dope-vector-global-const-prop

; The test case basically allocates the global array A in ALLOCATE_ARR, then
; initializes it in INITIALIZE_ARR and the use will be in PRINT_ARR. The problem
; is that ALLOCATE_ARR won't allocate the field inner_array_A in T_TESTTYPE.

; CHECK: define internal void @arr_mod_mp_initialize_arr_
; CHECK:  %16 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %15, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %14, i64 %7)
; CHECK:  %29 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %28, i64 %26, float* elementtype(float) %18, i64 %9)
; CHECK:  %30 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %24, i64 %21, float* elementtype(float) %29, i64 %13)
; CHECK:  %35 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %34, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %33, i64 %7)
; CHECK:  %52 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 %51, i64 %49, float* elementtype(float) %37, i64 %32)
; CHECK:  %53 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %47, i64 %45, float* elementtype(float) %52, i64 %9)
; CHECK:  %54 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %43, i64 %40, float* elementtype(float) %53, i64 %13)

; ModuleID = 'ld-temp.o'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$" = type { %"ARR_MOD$.btT_TESTTYPE"*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"ARR_MOD$.btT_TESTTYPE" = type { %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank1$" }
%"QNCA_a0$float*$rank2$" = type { float*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank3$" = type { float*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank1$" = type { float*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@arr_mod_mp_a_ = internal global %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$" { %"ARR_MOD$.btT_TESTTYPE"* null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@anon.87529b4ebf98830a9107fed24e462e82.0 = internal unnamed_addr constant i32 2
@anon.87529b4ebf98830a9107fed24e462e82.1 = internal unnamed_addr constant i32 10

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_allocate_arr_(i32* noalias nocapture readonly dereferenceable(4) %0) #0 {
  %2 = alloca i64, align 8
  %3 = load i32, i32* %0, align 1
  %4 = icmp eq i32 %3, 1
  br i1 %4, label %7, label %5

5:                                                ; preds = %1
  %6 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  br label %35

7:                                                ; preds = %1
  %8 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 3), align 8
  %9 = and i64 %8, 1030792151296
  %10 = or i64 %9, 133
  store i64 %10, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 3), align 8
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 5), align 8
  store i64 288, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 1), align 8
  store i64 1, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 4), align 16
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 2), align 16
  %11 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, i64* %11, align 1
  %12 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 1, i64* %12, align 1
  %13 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 288, i64* %13, align 1
  %14 = call i32 (i64*, i32, ...) @for_check_mult_overflow64(i64* nonnull %2, i32 2, i64 1, i64 288) #5
  %15 = load i64, i64* %2, align 8
  %16 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 3), align 8
  %17 = and i64 %16, -68451041281
  %18 = or i64 %17, 1073741824
  store i64 %18, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 3), align 8
  %19 = trunc i64 %16 to i32
  %20 = shl i32 %19, 1
  %21 = and i32 %20, 2
  %22 = shl i32 %14, 4
  %23 = and i32 %22, 16
  %24 = lshr i64 %16, 15
  %25 = trunc i64 %24 to i32
  %26 = and i32 %25, 31457280
  %27 = and i32 %25, 33554432
  %28 = or i32 %23, %21
  %29 = or i32 %28, %26
  %30 = or i32 %29, %27
  %31 = or i32 %30, 262144
  %32 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 5), align 8
  %33 = inttoptr i64 %32 to i8*
  %34 = tail call i32 @for_alloc_allocatable_handle(i64 %15, i8** bitcast (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_ to i8**), i32 %31, i8* %33) #5
  br label %35

35:                                               ; preds = %7, %5
  %36 = phi i64* [ %6, %5 ], [ %11, %7 ]
  %37 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %38 = load i64, i64* %36, align 1
  %39 = sext i32 %3 to i64
  %40 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %38, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %37, i64 %39)
  %41 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 1, i32 3
  %42 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 1, i32 5
  store i64 0, i64* %42, align 1
  %43 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 1, i32 1
  store i64 4, i64* %43, align 1
  %44 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 1, i32 4
  store i64 3, i64* %44, align 1
  %45 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 1, i32 2
  store i64 0, i64* %45, align 1
  %46 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 1, i32 6, i64 0, i32 2
  %47 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %46, i32 0)
  store i64 1, i64* %47, align 1
  %48 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 1, i32 6, i64 0, i32 0
  %49 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %48, i32 0)
  store i64 10, i64* %49, align 1
  %50 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %46, i32 1)
  store i64 1, i64* %50, align 1
  %51 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %48, i32 1)
  store i64 10, i64* %51, align 1
  %52 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %46, i32 2)
  store i64 1, i64* %52, align 1
  %53 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %48, i32 2)
  store i64 10, i64* %53, align 1
  %54 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 1, i32 6, i64 0, i32 1
  %55 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %54, i32 0)
  store i64 4, i64* %55, align 1
  %56 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %54, i32 1)
  store i64 40, i64* %56, align 1
  %57 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %54, i32 2)
  store i64 400, i64* %57, align 1
  %58 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 1, i32 0
  store i64 1073741829, i64* %41, align 1
  %59 = bitcast float** %58 to i8**
  %60 = tail call i32 @for_allocate_handle(i64 4000, i8** nonnull %59, i32 262144, i8* null) #5
  %61 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %62 = load i64, i64* %36, align 1
  %63 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %62, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %61, i64 %39)
  %64 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %63, i64 0, i32 2, i32 3
  %65 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %63, i64 0, i32 2, i32 5
  store i64 0, i64* %65, align 1
  %66 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %63, i64 0, i32 2, i32 1
  store i64 4, i64* %66, align 1
  %67 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %63, i64 0, i32 2, i32 4
  store i64 1, i64* %67, align 1
  %68 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %63, i64 0, i32 2, i32 2
  store i64 0, i64* %68, align 1
  %69 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %63, i64 0, i32 2, i32 6, i64 0, i32 2
  %70 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %69, i32 0)
  store i64 1, i64* %70, align 1
  %71 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %63, i64 0, i32 2, i32 6, i64 0, i32 0
  %72 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %71, i32 0)
  store i64 10, i64* %72, align 1
  %73 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %63, i64 0, i32 2, i32 6, i64 0, i32 1
  %74 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %73, i32 0)
  store i64 4, i64* %74, align 1
  %75 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %63, i64 0, i32 2, i32 0
  store i64 1073741829, i64* %64, align 1
  %76 = bitcast float** %75 to i8**
  %77 = tail call i32 @for_allocate_handle(i64 40, i8** nonnull %76, i32 262144, i8* null) #5
  ret void
}

; Function Attrs: nosync nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #1

; Function Attrs: nofree
declare dso_local i32 @for_check_mult_overflow64(i64* nocapture, i32, ...) local_unnamed_addr #2

; Function Attrs: nofree
declare dso_local i32 @for_alloc_allocatable_handle(i64, i8** nocapture, i32, i8*) local_unnamed_addr #2

; Function Attrs: nosync nounwind readnone speculatable
declare %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8, i64, i64, %"ARR_MOD$.btT_TESTTYPE"*, i64) #1

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, i8** nocapture, i32, i8*) local_unnamed_addr #2

; Function Attrs: nofree nosync nounwind uwtable
define internal void @arr_mod_mp_initialize_arr_(i32* noalias nocapture readonly dereferenceable(4) %0, i32* noalias nocapture readonly dereferenceable(4) %1, i32* noalias nocapture readonly dereferenceable(4) %2, i32* noalias nocapture readonly dereferenceable(4) %3) #3 {
  %5 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %6 = load i32, i32* %0, align 1
  %7 = sext i32 %6 to i64
  br label %8

8:                                                ; preds = %60, %4
  %9 = phi i64 [ 1, %4 ], [ %73, %60 ]
  %10 = trunc i64 %9 to i32
  %11 = sitofp i32 %10 to float
  br label %12

12:                                               ; preds = %57, %8
  %13 = phi i64 [ 1, %8 ], [ %58, %57 ]
  %14 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %15 = load i64, i64* %5, align 1
  %16 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %15, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %14, i64 %7)
  %17 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %16, i64 0, i32 0, i32 0
  %18 = load float*, float** %17, align 1
  %19 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %16, i64 0, i32 0, i32 6, i64 0, i32 1
  %20 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %19, i32 0)
  %21 = load i64, i64* %20, align 1
  %22 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %16, i64 0, i32 0, i32 6, i64 0, i32 2
  %23 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %22, i32 0)
  %24 = load i64, i64* %23, align 1
  %25 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %19, i32 1)
  %26 = load i64, i64* %25, align 1
  %27 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %22, i32 1)
  %28 = load i64, i64* %27, align 1
  %29 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %28, i64 %26, float* elementtype(float) %18, i64 %9)
  %30 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %24, i64 %21, float* elementtype(float) %29, i64 %13)
  store float %11, float* %30, align 1
  br label %31

31:                                               ; preds = %12, %31
  %32 = phi i64 [ %55, %31 ], [ 1, %12 ]
  %33 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %34 = load i64, i64* %5, align 1
  %35 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %34, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %33, i64 %7)
  %36 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %35, i64 0, i32 1, i32 0
  %37 = load float*, float** %36, align 1
  %38 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %35, i64 0, i32 1, i32 6, i64 0, i32 1
  %39 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %38, i32 0)
  %40 = load i64, i64* %39, align 1
  %41 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %35, i64 0, i32 1, i32 6, i64 0, i32 2
  %42 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %41, i32 0)
  %43 = load i64, i64* %42, align 1
  %44 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %38, i32 1)
  %45 = load i64, i64* %44, align 1
  %46 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %41, i32 1)
  %47 = load i64, i64* %46, align 1
  %48 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %38, i32 2)
  %49 = load i64, i64* %48, align 1
  %50 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %41, i32 2)
  %51 = load i64, i64* %50, align 1
  %52 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 %51, i64 %49, float* elementtype(float) %37, i64 %32)
  %53 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %47, i64 %45, float* elementtype(float) %52, i64 %9)
  %54 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %43, i64 %40, float* elementtype(float) %53, i64 %13)
  store float %11, float* %54, align 1
  %55 = add nuw nsw i64 %32, 1
  %56 = icmp eq i64 %55, 11
  br i1 %56, label %57, label %31

57:                                               ; preds = %31
  %58 = add nuw nsw i64 %13, 1
  %59 = icmp eq i64 %58, 11
  br i1 %59, label %60, label %12

60:                                               ; preds = %57
  %61 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %62 = load i64, i64* %5, align 1
  %63 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %62, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %61, i64 %7)
  %64 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %63, i64 0, i32 2, i32 0
  %65 = load float*, float** %64, align 1
  %66 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %63, i64 0, i32 2, i32 6, i64 0, i32 1
  %67 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %66, i32 0)
  %68 = load i64, i64* %67, align 1
  %69 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %63, i64 0, i32 2, i32 6, i64 0, i32 2
  %70 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %69, i32 0)
  %71 = load i64, i64* %70, align 1
  %72 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %71, i64 %68, float* elementtype(float) %65, i64 11)
  store float 1.100000e+01, float* %72, align 1
  %73 = add nuw nsw i64 %9, 1
  %74 = icmp eq i64 %73, 11
  br i1 %74, label %75, label %8

75:                                               ; preds = %60
  ret void
}

; Function Attrs: nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #1

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_print_arr_(i32* noalias nocapture readonly dereferenceable(4) %0, i32* noalias nocapture readonly dereferenceable(4) %1, i32* noalias nocapture readonly dereferenceable(4) %2, i32* noalias nocapture readonly dereferenceable(4) %3) #0 {
  %5 = alloca [8 x i64], align 16
  %6 = alloca [4 x i8], align 1
  %7 = alloca { float }, align 8
  %8 = alloca [4 x i8], align 1
  %9 = alloca { float }, align 8
  %10 = alloca [4 x i8], align 1
  %11 = alloca { float }, align 8
  %12 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %13 = load i32, i32* %0, align 1
  %14 = sext i32 %13 to i64
  %15 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 0
  %16 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 1
  %17 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 2
  %18 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 3
  %19 = getelementptr inbounds { float }, { float }* %7, i64 0, i32 0
  %20 = bitcast [8 x i64]* %5 to i8*
  %21 = bitcast { float }* %7 to i8*
  %22 = getelementptr inbounds [4 x i8], [4 x i8]* %8, i64 0, i64 0
  %23 = getelementptr inbounds [4 x i8], [4 x i8]* %8, i64 0, i64 1
  %24 = getelementptr inbounds [4 x i8], [4 x i8]* %8, i64 0, i64 2
  %25 = getelementptr inbounds [4 x i8], [4 x i8]* %8, i64 0, i64 3
  %26 = getelementptr inbounds { float }, { float }* %9, i64 0, i32 0
  %27 = bitcast { float }* %9 to i8*
  %28 = getelementptr inbounds [4 x i8], [4 x i8]* %10, i64 0, i64 0
  %29 = getelementptr inbounds [4 x i8], [4 x i8]* %10, i64 0, i64 1
  %30 = getelementptr inbounds [4 x i8], [4 x i8]* %10, i64 0, i64 2
  %31 = getelementptr inbounds [4 x i8], [4 x i8]* %10, i64 0, i64 3
  %32 = getelementptr inbounds { float }, { float }* %11, i64 0, i32 0
  %33 = bitcast { float }* %11 to i8*
  br label %34

34:                                               ; preds = %90, %4
  %35 = phi i64 [ 1, %4 ], [ %107, %90 ]
  br label %36

36:                                               ; preds = %34, %86
  %37 = phi i64 [ %87, %86 ], [ 1, %34 ]
  %38 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %39 = load i64, i64* %12, align 1
  %40 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %39, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %38, i64 %14)
  %41 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 0
  %42 = load float*, float** %41, align 1
  %43 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 6, i64 0, i32 1
  %44 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %43, i32 0)
  %45 = load i64, i64* %44, align 1
  %46 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 6, i64 0, i32 2
  %47 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %46, i32 0)
  %48 = load i64, i64* %47, align 1
  %49 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %43, i32 1)
  %50 = load i64, i64* %49, align 1
  %51 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %46, i32 1)
  %52 = load i64, i64* %51, align 1
  %53 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %52, i64 %50, float* elementtype(float) %42, i64 %35)
  %54 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %48, i64 %45, float* elementtype(float) %53, i64 %37)
  %55 = load float, float* %54, align 1
  store i8 26, i8* %15, align 1
  store i8 1, i8* %16, align 1
  store i8 1, i8* %17, align 1
  store i8 0, i8* %18, align 1
  store float %55, float* %19, align 8
  %56 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %20, i32 -1, i64 1239157112576, i8* nonnull %15, i8* nonnull %21) #5
  br label %57

57:                                               ; preds = %36, %57
  %58 = phi i64 [ %83, %57 ], [ 1, %36 ]
  %59 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %60 = load i64, i64* %12, align 1
  %61 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %60, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %59, i64 %14)
  %62 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %61, i64 0, i32 1, i32 0
  %63 = load float*, float** %62, align 1
  %64 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %61, i64 0, i32 1, i32 6, i64 0, i32 1
  %65 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %64, i32 0)
  %66 = load i64, i64* %65, align 1
  %67 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %61, i64 0, i32 1, i32 6, i64 0, i32 2
  %68 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %67, i32 0)
  %69 = load i64, i64* %68, align 1
  %70 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %64, i32 1)
  %71 = load i64, i64* %70, align 1
  %72 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %67, i32 1)
  %73 = load i64, i64* %72, align 1
  %74 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %64, i32 2)
  %75 = load i64, i64* %74, align 1
  %76 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %67, i32 2)
  %77 = load i64, i64* %76, align 1
  %78 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 %77, i64 %75, float* elementtype(float) %63, i64 %58)
  %79 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %73, i64 %71, float* elementtype(float) %78, i64 %35)
  %80 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %69, i64 %66, float* elementtype(float) %79, i64 %37)
  %81 = load float, float* %80, align 1
  store i8 26, i8* %22, align 1
  store i8 1, i8* %23, align 1
  store i8 1, i8* %24, align 1
  store i8 0, i8* %25, align 1
  store float %81, float* %26, align 8
  %82 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %20, i32 -1, i64 1239157112576, i8* nonnull %22, i8* nonnull %27) #5
  %83 = add nuw i64 %58, 1
  %84 = trunc i64 %83 to i32
  %85 = icmp slt i32 10, %84
  br i1 %85, label %86, label %57

86:                                               ; preds = %57
  %87 = add nuw i64 %37, 1
  %88 = trunc i64 %87 to i32
  %89 = icmp slt i32 10, %88
  br i1 %89, label %90, label %36

90:                                               ; preds = %86
  %91 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %92 = load i64, i64* %12, align 1
  %93 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %92, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %91, i64 %14)
  %94 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %93, i64 0, i32 2, i32 0
  %95 = load float*, float** %94, align 1
  %96 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %93, i64 0, i32 2, i32 6, i64 0, i32 1
  %97 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %96, i32 0)
  %98 = load i64, i64* %97, align 1
  %99 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %93, i64 0, i32 2, i32 6, i64 0, i32 2
  %100 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %99, i32 0)
  %101 = load i64, i64* %100, align 1
  %102 = shl i64 %87, 32
  %103 = ashr exact i64 %102, 32
  %104 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %101, i64 %98, float* elementtype(float) %95, i64 %103)
  %105 = load float, float* %104, align 1
  store i8 26, i8* %28, align 1
  store i8 1, i8* %29, align 1
  store i8 1, i8* %30, align 1
  store i8 0, i8* %31, align 1
  store float %105, float* %32, align 8
  %106 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %20, i32 -1, i64 1239157112576, i8* nonnull %28, i8* nonnull %33) #5
  %107 = add nuw i64 %35, 1
  %108 = trunc i64 %107 to i32
  %109 = icmp slt i32 10, %108
  br i1 %109, label %110, label %34

110:                                              ; preds = %90
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr #2

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #0 {
  %1 = alloca i32, align 8
  %2 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.87529b4ebf98830a9107fed24e462e82.0) #5
  store i32 1, i32* %1, align 8
  br label %3

3:                                                ; preds = %3, %0
  %4 = phi i32 [ %5, %3 ], [ 1, %0 ]
  call void @arr_mod_mp_allocate_arr_(i32* nonnull %1)
  call void @arr_mod_mp_initialize_arr_(i32* nonnull %1, i32* nonnull @anon.87529b4ebf98830a9107fed24e462e82.1, i32* nonnull @anon.87529b4ebf98830a9107fed24e462e82.1, i32* nonnull @anon.87529b4ebf98830a9107fed24e462e82.1)
  call void @arr_mod_mp_print_arr_(i32* nonnull %1, i32* nonnull @anon.87529b4ebf98830a9107fed24e462e82.1, i32* nonnull @anon.87529b4ebf98830a9107fed24e462e82.1, i32* nonnull @anon.87529b4ebf98830a9107fed24e462e82.1)
  %5 = add nuw nsw i32 %4, 1
  store i32 %5, i32* %1, align 8
  %6 = icmp eq i32 %5, 11
  br i1 %6, label %7, label %3

7:                                                ; preds = %3
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(i32* nocapture readonly) local_unnamed_addr #2

; Function Attrs: nofree nosync nounwind readnone willreturn
declare i32 @llvm.ssa.copy.i32(i32 returned) #4

; Function Attrs: nofree nosync nounwind readnone willreturn
declare i64 @llvm.ssa.copy.i64(i64 returned) #4

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nosync nounwind readnone speculatable }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { nofree nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #4 = { nofree nosync nounwind readnone willreturn }
attributes #5 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
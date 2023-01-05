; RUN: opt < %s -passes=dopevectorconstprop -dope-vector-global-const-prop=true -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s

; This test case checks that the dope vector information was collected and
; propagated even though the data allocation comes from the user input
; (not constant). This is the same test case as global_dvcp04.ll, but it checks
; the IR. It was created from the following source code:

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
;         SUBROUTINE ALLOCATE_ARR(I, N)
;           INTEGER, INTENT(IN) :: I, N
;
;           IF(I.eq.1) ALLOCATE(A(I))
;
;           ALLOCATE(A(I) % inner_array_A(10, 10))
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
;        INTEGER :: I, N
;
;        READ (*,*) N
;
;        DO I = 1, 10
;          CALL ALLOCATE_ARR(I, N)
;          CALL INITIALIZE_ARR(I, 10, 10, 10)
;          CALL PRINT_ARR(I, 10, 10, 10)
;        END DO
;      END

; ifx -xCORE-AVX512 -Ofast -flto arr.f90 -mllvm -debug-only=dope-vector-global-const-prop

; This test case is the same as glob_dvcp02.ll but the allocation, initialization and
; use is controlled by the user input (READ).

; Check that the constants were propagated in function @arr_mod_mp_allocate_arr_
; CHECK: define internal void @arr_mod_mp_allocate_arr_
; CHECK:   %44 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %41, i64 %43)
; CHECK:   %78 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %76, i64 %43)
; CHECK:   %112 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %110, i64 %43)

; Check that the constants were propagated in function @arr_mod_mp_initialize_arr_
; CHECK: define internal void @arr_mod_mp_initialize_arr_
; CHECK:   %30 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %28, i64 %12)
; CHECK:   %43 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %40, float* elementtype(float) %32, i64 %22)
; CHECK:   %44 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %43, i64 %27)
; CHECK:   %49 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %47, i64 %12)
; CHECK:   %66 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 %63, float* elementtype(float) %51, i64 %46)
; CHECK:   %67 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %59, float* elementtype(float) %66, i64 %22)
; CHECK:   %68 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %67, i64 %27)
; CHECK:   %79 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %77, i64 %12)
; CHECK:   %89 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %81, i64 %88)

; Check that the constants were propagated in function @arr_mod_mp_print_arr_
; CHECK: define internal void @arr_mod_mp_print_arr_
; CHECK:   %47 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %45, i64 %19)
; CHECK:   %60 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %57, float* elementtype(float) %49, i64 %42)
; CHECK:   %61 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %60, i64 %44)
; CHECK:   %68 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %66, i64 %19)
; CHECK:   %85 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 %82, float* elementtype(float) %70, i64 %65)
; CHECK:   %86 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %78, float* elementtype(float) %85, i64 %42)
; CHECK:   %87 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %86, i64 %44)
; CHECK:   %101 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %99, i64 %19)
; CHECK:   %112 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %103, i64 %111)

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

; Function Attrs: nofree noinline nounwind uwtable
define internal void @arr_mod_mp_allocate_arr_(i32* noalias nocapture readonly dereferenceable(4) %0, i32* noalias nocapture readonly dereferenceable(4) %1) #0 {
  %3 = alloca i64, align 8
  %4 = alloca i64, align 8
  %5 = alloca i64, align 8
  %6 = alloca i64, align 8
  %7 = load i32, i32* %0, align 1
  %8 = icmp eq i32 %7, 1
  br i1 %8, label %11, label %9

9:                                                ; preds = %2
  %10 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  br label %39

11:                                               ; preds = %2
  %12 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 3), align 8
  %13 = and i64 %12, 1030792151296
  %14 = or i64 %13, 133
  store i64 %14, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 3), align 8
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 5), align 8
  store i64 288, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 1), align 8
  store i64 1, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 4), align 16
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 2), align 16
  %15 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, i64* %15, align 1
  %16 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 1, i64* %16, align 1
  %17 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 288, i64* %17, align 1
  %18 = call i32 (i64*, i32, ...) @for_check_mult_overflow64(i64* nonnull %3, i32 2, i64 1, i64 288) #3
  %19 = load i64, i64* %3, align 8
  %20 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 3), align 8
  %21 = and i64 %20, -68451041281
  %22 = or i64 %21, 1073741824
  store i64 %22, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 3), align 8
  %23 = trunc i64 %20 to i32
  %24 = shl i32 %23, 1
  %25 = and i32 %24, 2
  %26 = shl i32 %18, 4
  %27 = and i32 %26, 16
  %28 = lshr i64 %20, 15
  %29 = trunc i64 %28 to i32
  %30 = and i32 %29, 31457280
  %31 = and i32 %29, 33554432
  %32 = or i32 %27, %25
  %33 = or i32 %32, %30
  %34 = or i32 %33, %31
  %35 = or i32 %34, 262144
  %36 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 5), align 8
  %37 = inttoptr i64 %36 to i8*
  %38 = tail call i32 @for_alloc_allocatable_handle(i64 %19, i8** bitcast (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_ to i8**), i32 %35, i8* %37) #3
  br label %39

39:                                               ; preds = %11, %9
  %40 = phi i64* [ %10, %9 ], [ %15, %11 ]
  %41 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %42 = load i64, i64* %40, align 1
  %43 = sext i32 %7 to i64
  %44 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %42, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %41, i64 %43)
  %45 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %44, i64 0, i32 0, i32 3
  store i64 5, i64* %45, align 1
  %46 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %44, i64 0, i32 0, i32 5
  store i64 0, i64* %46, align 1
  %47 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %44, i64 0, i32 0, i32 1
  store i64 4, i64* %47, align 1
  %48 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %44, i64 0, i32 0, i32 4
  store i64 2, i64* %48, align 1
  %49 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %44, i64 0, i32 0, i32 2
  store i64 0, i64* %49, align 1
  %50 = load i32, i32* %1, align 1
  %51 = sext i32 %50 to i64
  %52 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %44, i64 0, i32 0, i32 6, i64 0, i32 2
  %53 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %52, i32 0)
  store i64 1, i64* %53, align 1
  %54 = icmp sgt i64 %51, 0
  %55 = select i1 %54, i64 %51, i64 0
  %56 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %44, i64 0, i32 0, i32 6, i64 0, i32 0
  %57 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %56, i32 0)
  store i64 %55, i64* %57, align 1
  %58 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %52, i32 1)
  store i64 1, i64* %58, align 1
  %59 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %56, i32 1)
  store i64 %55, i64* %59, align 1
  %60 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %44, i64 0, i32 0, i32 6, i64 0, i32 1
  %61 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %60, i32 0)
  store i64 4, i64* %61, align 1
  %62 = shl nuw nsw i64 %55, 2
  %63 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %60, i32 1)
  store i64 %62, i64* %63, align 1
  %64 = call i32 (i64*, i32, ...) @for_check_mult_overflow64(i64* nonnull %4, i32 3, i64 %55, i64 %55, i64 4) #3
  %65 = load i64, i64* %4, align 8
  %66 = load i64, i64* %45, align 1
  %67 = and i64 %66, -68451041281
  %68 = or i64 %67, 1073741824
  store i64 %68, i64* %45, align 1
  %69 = shl i32 %64, 4
  %70 = and i32 %69, 16
  %71 = or i32 %70, 262144
  %72 = load i64, i64* %46, align 1
  %73 = inttoptr i64 %72 to i8*
  %74 = bitcast %"ARR_MOD$.btT_TESTTYPE"* %44 to i8**
  %75 = tail call i32 @for_allocate_handle(i64 %65, i8** %74, i32 %71, i8* %73) #3
  %76 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %77 = load i64, i64* %40, align 1
  %78 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %77, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %76, i64 %43)
  %79 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %78, i64 0, i32 1, i32 3
  store i64 5, i64* %79, align 1
  %80 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %78, i64 0, i32 1, i32 5
  store i64 0, i64* %80, align 1
  %81 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %78, i64 0, i32 1, i32 1
  store i64 4, i64* %81, align 1
  %82 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %78, i64 0, i32 1, i32 4
  store i64 3, i64* %82, align 1
  %83 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %78, i64 0, i32 1, i32 2
  store i64 0, i64* %83, align 1
  %84 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %78, i64 0, i32 1, i32 6, i64 0, i32 2
  %85 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %84, i32 0)
  store i64 1, i64* %85, align 1
  %86 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %78, i64 0, i32 1, i32 6, i64 0, i32 0
  %87 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %86, i32 0)
  store i64 %55, i64* %87, align 1
  %88 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %84, i32 1)
  store i64 1, i64* %88, align 1
  %89 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %86, i32 1)
  store i64 %55, i64* %89, align 1
  %90 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %84, i32 2)
  store i64 1, i64* %90, align 1
  %91 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %86, i32 2)
  store i64 %55, i64* %91, align 1
  %92 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %78, i64 0, i32 1, i32 6, i64 0, i32 1
  %93 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %92, i32 0)
  store i64 4, i64* %93, align 1
  %94 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %92, i32 1)
  store i64 %62, i64* %94, align 1
  %95 = mul nuw nsw i64 %62, %55
  %96 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %92, i32 2)
  store i64 %95, i64* %96, align 1
  %97 = call i32 (i64*, i32, ...) @for_check_mult_overflow64(i64* nonnull %5, i32 4, i64 %55, i64 %55, i64 %55, i64 4) #3
  %98 = load i64, i64* %5, align 8
  %99 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %78, i64 0, i32 1, i32 0
  %100 = load i64, i64* %79, align 1
  %101 = and i64 %100, -68451041281
  %102 = or i64 %101, 1073741824
  store i64 %102, i64* %79, align 1
  %103 = shl i32 %97, 4
  %104 = and i32 %103, 16
  %105 = or i32 %104, 262144
  %106 = load i64, i64* %80, align 1
  %107 = inttoptr i64 %106 to i8*
  %108 = bitcast float** %99 to i8**
  %109 = tail call i32 @for_allocate_handle(i64 %98, i8** nonnull %108, i32 %105, i8* %107) #3
  %110 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %111 = load i64, i64* %40, align 1
  %112 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %111, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %110, i64 %43)
  %113 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %112, i64 0, i32 2, i32 3
  store i64 5, i64* %113, align 1
  %114 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %112, i64 0, i32 2, i32 5
  store i64 0, i64* %114, align 1
  %115 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %112, i64 0, i32 2, i32 1
  store i64 4, i64* %115, align 1
  %116 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %112, i64 0, i32 2, i32 4
  store i64 1, i64* %116, align 1
  %117 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %112, i64 0, i32 2, i32 2
  store i64 0, i64* %117, align 1
  %118 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %112, i64 0, i32 2, i32 6, i64 0, i32 2
  %119 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %118, i32 0)
  store i64 1, i64* %119, align 1
  %120 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %112, i64 0, i32 2, i32 6, i64 0, i32 0
  %121 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %120, i32 0)
  store i64 %55, i64* %121, align 1
  %122 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %112, i64 0, i32 2, i32 6, i64 0, i32 1
  %123 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %122, i32 0)
  store i64 4, i64* %123, align 1
  %124 = call i32 (i64*, i32, ...) @for_check_mult_overflow64(i64* nonnull %6, i32 2, i64 %55, i64 4) #3
  %125 = load i64, i64* %6, align 8
  %126 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %112, i64 0, i32 2, i32 0
  %127 = load i64, i64* %113, align 1
  %128 = and i64 %127, -68451041281
  %129 = or i64 %128, 1073741824
  store i64 %129, i64* %113, align 1
  %130 = shl i32 %124, 4
  %131 = and i32 %130, 16
  %132 = or i32 %131, 262144
  %133 = load i64, i64* %114, align 1
  %134 = inttoptr i64 %133 to i8*
  %135 = bitcast float** %126 to i8**
  %136 = tail call i32 @for_allocate_handle(i64 %125, i8** nonnull %135, i32 %132, i8* %134) #3
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #1

; Function Attrs: nofree
declare dso_local i32 @for_check_mult_overflow64(i64* nocapture, i32, ...) local_unnamed_addr #2

; Function Attrs: nofree
declare dso_local i32 @for_alloc_allocatable_handle(i64, i8** nocapture, i32, i8*) local_unnamed_addr #2

; Function Attrs: nounwind readnone speculatable
declare %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8, i64, i64, %"ARR_MOD$.btT_TESTTYPE"*, i64) #1

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, i8** nocapture, i32, i8*) local_unnamed_addr #2

; Function Attrs: nofree noinline nounwind uwtable
define internal void @arr_mod_mp_initialize_arr_(i32* noalias nocapture readonly dereferenceable(4) %0, i32* noalias nocapture readonly dereferenceable(4) %1, i32* noalias nocapture readonly dereferenceable(4) %2, i32* noalias nocapture readonly dereferenceable(4) %3) #0 {
  %5 = load i32, i32* %1, align 1
  %6 = icmp slt i32 %5, 1
  br i1 %6, label %92, label %7

7:                                                ; preds = %4
  %8 = load i32, i32* %2, align 1
  %9 = icmp slt i32 %8, 1
  %10 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %11 = load i32, i32* %0, align 1
  %12 = sext i32 %11 to i64
  %13 = load i32, i32* %3, align 1
  %14 = icmp slt i32 %13, 1
  %15 = add nuw nsw i32 %13, 1
  %16 = add i32 %8, 1
  %17 = add nuw nsw i32 %5, 1
  %18 = sext i32 %17 to i64
  %19 = sext i32 %16 to i64
  %20 = sext i32 %15 to i64
  br label %21

21:                                               ; preds = %74, %7
  %22 = phi i64 [ 1, %7 ], [ %90, %74 ]
  br i1 %9, label %74, label %23

23:                                               ; preds = %21
  %24 = trunc i64 %22 to i32
  %25 = sitofp i32 %24 to float
  br label %26

26:                                               ; preds = %71, %23
  %27 = phi i64 [ 1, %23 ], [ %72, %71 ]
  %28 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %29 = load i64, i64* %10, align 1
  %30 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %29, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %28, i64 %12)
  %31 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %30, i64 0, i32 0, i32 0
  %32 = load float*, float** %31, align 1
  %33 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %30, i64 0, i32 0, i32 6, i64 0, i32 1
  %34 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %33, i32 0)
  %35 = load i64, i64* %34, align 1
  %36 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %30, i64 0, i32 0, i32 6, i64 0, i32 2
  %37 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %36, i32 0)
  %38 = load i64, i64* %37, align 1
  %39 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %33, i32 1)
  %40 = load i64, i64* %39, align 1
  %41 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %36, i32 1)
  %42 = load i64, i64* %41, align 1
  %43 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %42, i64 %40, float* elementtype(float) %32, i64 %22)
  %44 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %38, i64 %35, float* elementtype(float) %43, i64 %27)
  store float %25, float* %44, align 1
  br i1 %14, label %71, label %45

45:                                               ; preds = %45, %26
  %46 = phi i64 [ %69, %45 ], [ 1, %26 ]
  %47 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %48 = load i64, i64* %10, align 1
  %49 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %48, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %47, i64 %12)
  %50 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %49, i64 0, i32 1, i32 0
  %51 = load float*, float** %50, align 1
  %52 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %49, i64 0, i32 1, i32 6, i64 0, i32 1
  %53 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %52, i32 0)
  %54 = load i64, i64* %53, align 1
  %55 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %49, i64 0, i32 1, i32 6, i64 0, i32 2
  %56 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %55, i32 0)
  %57 = load i64, i64* %56, align 1
  %58 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %52, i32 1)
  %59 = load i64, i64* %58, align 1
  %60 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %55, i32 1)
  %61 = load i64, i64* %60, align 1
  %62 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %52, i32 2)
  %63 = load i64, i64* %62, align 1
  %64 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %55, i32 2)
  %65 = load i64, i64* %64, align 1
  %66 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 %65, i64 %63, float* elementtype(float) %51, i64 %46)
  %67 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %61, i64 %59, float* elementtype(float) %66, i64 %22)
  %68 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %57, i64 %54, float* elementtype(float) %67, i64 %27)
  store float %25, float* %68, align 1
  %69 = add nuw nsw i64 %46, 1
  %70 = icmp eq i64 %69, %20
  br i1 %70, label %71, label %45

71:                                               ; preds = %45, %26
  %72 = add nuw nsw i64 %27, 1
  %73 = icmp eq i64 %72, %19
  br i1 %73, label %74, label %26

74:                                               ; preds = %71, %21
  %75 = phi i32 [ 1, %21 ], [ %16, %71 ]
  %76 = sitofp i32 %75 to float
  %77 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %78 = load i64, i64* %10, align 1
  %79 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %78, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %77, i64 %12)
  %80 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %79, i64 0, i32 2, i32 0
  %81 = load float*, float** %80, align 1
  %82 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %79, i64 0, i32 2, i32 6, i64 0, i32 1
  %83 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %82, i32 0)
  %84 = load i64, i64* %83, align 1
  %85 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %79, i64 0, i32 2, i32 6, i64 0, i32 2
  %86 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %85, i32 0)
  %87 = load i64, i64* %86, align 1
  %88 = sext i32 %75 to i64
  %89 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %87, i64 %84, float* elementtype(float) %81, i64 %88)
  store float %76, float* %89, align 1
  %90 = add nuw nsw i64 %22, 1
  %91 = icmp eq i64 %90, %18
  br i1 %91, label %92, label %21

92:                                               ; preds = %74, %4
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #1

; Function Attrs: nofree noinline nounwind uwtable
define internal void @arr_mod_mp_print_arr_(i32* noalias nocapture readonly dereferenceable(4) %0, i32* noalias nocapture readonly dereferenceable(4) %1, i32* noalias nocapture readonly dereferenceable(4) %2, i32* noalias nocapture readonly dereferenceable(4) %3) #0 {
  %5 = alloca [8 x i64], align 16
  %6 = alloca [4 x i8], align 1
  %7 = alloca { float }, align 8
  %8 = alloca [4 x i8], align 1
  %9 = alloca { float }, align 8
  %10 = alloca [4 x i8], align 1
  %11 = alloca { float }, align 8
  %12 = load i32, i32* %1, align 1
  %13 = icmp slt i32 %12, 1
  br i1 %13, label %118, label %14

14:                                               ; preds = %4
  %15 = load i32, i32* %2, align 1
  %16 = icmp slt i32 %15, 1
  %17 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %18 = load i32, i32* %0, align 1
  %19 = sext i32 %18 to i64
  %20 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 0
  %21 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 1
  %22 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 2
  %23 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 3
  %24 = getelementptr inbounds { float }, { float }* %7, i64 0, i32 0
  %25 = bitcast [8 x i64]* %5 to i8*
  %26 = bitcast { float }* %7 to i8*
  %27 = load i32, i32* %3, align 1
  %28 = icmp slt i32 %27, 1
  %29 = getelementptr inbounds [4 x i8], [4 x i8]* %8, i64 0, i64 0
  %30 = getelementptr inbounds [4 x i8], [4 x i8]* %8, i64 0, i64 1
  %31 = getelementptr inbounds [4 x i8], [4 x i8]* %8, i64 0, i64 2
  %32 = getelementptr inbounds [4 x i8], [4 x i8]* %8, i64 0, i64 3
  %33 = getelementptr inbounds { float }, { float }* %9, i64 0, i32 0
  %34 = bitcast { float }* %9 to i8*
  %35 = getelementptr inbounds [4 x i8], [4 x i8]* %10, i64 0, i64 0
  %36 = getelementptr inbounds [4 x i8], [4 x i8]* %10, i64 0, i64 1
  %37 = getelementptr inbounds [4 x i8], [4 x i8]* %10, i64 0, i64 2
  %38 = getelementptr inbounds [4 x i8], [4 x i8]* %10, i64 0, i64 3
  %39 = getelementptr inbounds { float }, { float }* %11, i64 0, i32 0
  %40 = bitcast { float }* %11 to i8*
  br label %41

41:                                               ; preds = %97, %14
  %42 = phi i64 [ 1, %14 ], [ %115, %97 ]
  br i1 %16, label %97, label %43

43:                                               ; preds = %93, %41
  %44 = phi i64 [ %94, %93 ], [ 1, %41 ]
  %45 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %46 = load i64, i64* %17, align 1
  %47 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %46, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %45, i64 %19)
  %48 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %47, i64 0, i32 0, i32 0
  %49 = load float*, float** %48, align 1
  %50 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %47, i64 0, i32 0, i32 6, i64 0, i32 1
  %51 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %50, i32 0)
  %52 = load i64, i64* %51, align 1
  %53 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %47, i64 0, i32 0, i32 6, i64 0, i32 2
  %54 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %53, i32 0)
  %55 = load i64, i64* %54, align 1
  %56 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %50, i32 1)
  %57 = load i64, i64* %56, align 1
  %58 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %53, i32 1)
  %59 = load i64, i64* %58, align 1
  %60 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %59, i64 %57, float* elementtype(float) %49, i64 %42)
  %61 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %55, i64 %52, float* elementtype(float) %60, i64 %44)
  %62 = load float, float* %61, align 1
  store i8 26, i8* %20, align 1
  store i8 1, i8* %21, align 1
  store i8 1, i8* %22, align 1
  store i8 0, i8* %23, align 1
  store float %62, float* %24, align 8
  %63 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %25, i32 -1, i64 1239157112576, i8* nonnull %20, i8* nonnull %26) #3
  br i1 %28, label %93, label %64

64:                                               ; preds = %64, %43
  %65 = phi i64 [ %90, %64 ], [ 1, %43 ]
  %66 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %67 = load i64, i64* %17, align 1
  %68 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %67, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %66, i64 %19)
  %69 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %68, i64 0, i32 1, i32 0
  %70 = load float*, float** %69, align 1
  %71 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %68, i64 0, i32 1, i32 6, i64 0, i32 1
  %72 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %71, i32 0)
  %73 = load i64, i64* %72, align 1
  %74 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %68, i64 0, i32 1, i32 6, i64 0, i32 2
  %75 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %74, i32 0)
  %76 = load i64, i64* %75, align 1
  %77 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %71, i32 1)
  %78 = load i64, i64* %77, align 1
  %79 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %74, i32 1)
  %80 = load i64, i64* %79, align 1
  %81 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %71, i32 2)
  %82 = load i64, i64* %81, align 1
  %83 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %74, i32 2)
  %84 = load i64, i64* %83, align 1
  %85 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 %84, i64 %82, float* elementtype(float) %70, i64 %65)
  %86 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %80, i64 %78, float* elementtype(float) %85, i64 %42)
  %87 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %76, i64 %73, float* elementtype(float) %86, i64 %44)
  %88 = load float, float* %87, align 1
  store i8 26, i8* %29, align 1
  store i8 1, i8* %30, align 1
  store i8 1, i8* %31, align 1
  store i8 0, i8* %32, align 1
  store float %88, float* %33, align 8
  %89 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %25, i32 -1, i64 1239157112576, i8* nonnull %29, i8* nonnull %34) #3
  %90 = add nuw i64 %65, 1
  %91 = trunc i64 %90 to i32
  %92 = icmp slt i32 %27, %91
  br i1 %92, label %93, label %64

93:                                               ; preds = %64, %43
  %94 = add nuw i64 %44, 1
  %95 = trunc i64 %94 to i32
  %96 = icmp slt i32 %15, %95
  br i1 %96, label %97, label %43

97:                                               ; preds = %93, %41
  %98 = phi i64 [ 1, %41 ], [ %94, %93 ]
  %99 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %100 = load i64, i64* %17, align 1
  %101 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %100, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %99, i64 %19)
  %102 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %101, i64 0, i32 2, i32 0
  %103 = load float*, float** %102, align 1
  %104 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %101, i64 0, i32 2, i32 6, i64 0, i32 1
  %105 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %104, i32 0)
  %106 = load i64, i64* %105, align 1
  %107 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %101, i64 0, i32 2, i32 6, i64 0, i32 2
  %108 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %107, i32 0)
  %109 = load i64, i64* %108, align 1
  %110 = shl i64 %98, 32
  %111 = ashr exact i64 %110, 32
  %112 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %109, i64 %106, float* elementtype(float) %103, i64 %111)
  %113 = load float, float* %112, align 1
  store i8 26, i8* %35, align 1
  store i8 1, i8* %36, align 1
  store i8 1, i8* %37, align 1
  store i8 0, i8* %38, align 1
  store float %113, float* %39, align 8
  %114 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %25, i32 -1, i64 1239157112576, i8* nonnull %35, i8* nonnull %40) #3
  %115 = add nuw i64 %42, 1
  %116 = trunc i64 %115 to i32
  %117 = icmp slt i32 %12, %116
  br i1 %117, label %118, label %41

118:                                              ; preds = %97, %4
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr #2

; Function Attrs: nofree noinline nounwind uwtable
define dso_local void @MAIN__() #0 {
  %1 = alloca [8 x i64], align 16
  %2 = alloca i32, align 8
  %3 = alloca i32, align 8
  %4 = alloca [4 x i8], align 1
  %5 = alloca { i8* }, align 8
  %6 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.87529b4ebf98830a9107fed24e462e82.0) #3
  %7 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 0
  store i8 9, i8* %7, align 1
  %8 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 1
  store i8 3, i8* %8, align 1
  %9 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 2
  store i8 1, i8* %9, align 1
  %10 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 3
  store i8 0, i8* %10, align 1
  %11 = bitcast { i8* }* %5 to i32**
  store i32* %2, i32** %11, align 8
  %12 = bitcast [8 x i64]* %1 to i8*
  %13 = bitcast { i8* }* %5 to i8*
  %14 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_read_seq_lis(i8* nonnull %12, i32 -4, i64 1239157112576, i8* nonnull %7, i8* nonnull %13) #3
  store i32 1, i32* %3, align 8
  br label %15

15:                                               ; preds = %15, %0
  %16 = phi i32 [ %17, %15 ], [ 1, %0 ]
  call void @arr_mod_mp_allocate_arr_(i32* nonnull %3, i32* nonnull %2)
  call void @arr_mod_mp_initialize_arr_(i32* nonnull %3, i32* nonnull %2, i32* nonnull %2, i32* nonnull %2)
  call void @arr_mod_mp_print_arr_(i32* nonnull %3, i32* nonnull %2, i32* nonnull %2, i32* nonnull %2)
  %17 = add nuw nsw i32 %16, 1
  store i32 %17, i32* %3, align 8
  %18 = icmp eq i32 %17, 11
  br i1 %18, label %19, label %15

19:                                               ; preds = %15
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(i32* nocapture readonly) local_unnamed_addr #2

; Function Attrs: nofree
declare dso_local i32 @for_read_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr #2

attributes #0 = { nofree noinline nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
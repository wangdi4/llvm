; RUN: opt < %s -passes=dopevectorconstprop -dope-vector-global-const-prop=true -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s

; This test case checks that the fields for multiple global dope vectors
; were collected and propagated correctly. Also, it identifies and collects
; the nested dope vectors. This is the same test case as glob_dvcp05.ll but
; it checks the IR. It was created from the following source code:

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
;         TYPE T_TESTTYPE_2
;
;           REAL, POINTER :: inner_array_A(:,:)
;           REAL, POINTER :: inner_array_B(:,:,:)
;           REAL, POINTER :: inner_array_C(:)
;
;         END TYPE T_TESTTYPE_2
;
;         TYPE (T_TESTTYPE_2), ALLOCATABLE :: B (:)
;
;         CONTAINS
;
;         SUBROUTINE ALLOCATE_ARR(I)
;           INTEGER, INTENT(IN) :: I
;
;           IF(I.eq.1) ALLOCATE(A(I))
;
;           ALLOCATE(A(I) % inner_array_A(10, 10))
;           ALLOCATE(A(I) % inner_array_B(10, 10, 10))
;           ALLOCATE(A(I) % inner_array_C(10))
;
;           IF(I.eq.1) ALLOCATE(B(I))
;
;           ALLOCATE(B(I) % inner_array_A(10, 10))
;           ALLOCATE(B(I) % inner_array_B(10, 10, 10))
;           ALLOCATE(B(I) % inner_array_C(10))
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
;           DO j = 1, N
;             DO k = 1, M
;               B(I) % inner_array_A(k, j) = j
;               DO l = 1, O
;                 B(I) % inner_array_B(k, j, l) = j
;               END DO
;             END DO
;             B(I) % inner_array_C(k) = k
;           END DO
;
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
;
;           DO j = 1, N
;             DO k = 1, M
;               print *, B(I) % inner_array_A(k, j)
;               DO l = 1, O
;                 print *, B(I) % inner_array_B(k, j, l)
;               END DO
;             END DO
;             print *, B(I) % inner_array_C(k)
;           END DO
;
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

; The test case basically allocates, initializes and uses the global arrays
; A and B.

; Check that the constants were propagated for function @arr_mod_mp_allocate_arr_
; CHECK: define internal void @arr_mod_mp_allocate_arr_
; CHECK:   %41 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %38, i64 %40)
; CHECK:   %60 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %58, i64 %40)
; CHECK:   %83 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %81, i64 %40)
; CHECK:   %132 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %130, i64 %40)
; CHECK:   %151 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %149, i64 %40)
; CHECK:   %174 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %172, i64 %40)

; Check that the constants were propagated for function @arr_mod_mp_initialize_arr_
; CHECK: define internal void @arr_mod_mp_initialize_arr_
; CHECK:   %16 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %14, i64 %7)
; CHECK:   %29 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 40, float* elementtype(float) %18, i64 %9)
; CHECK:   %30 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %29, i64 %13)
; CHECK:   %35 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %33, i64 %7)
; CHECK:   %52 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 400, float* elementtype(float) %37, i64 %32)
; CHECK:   %53 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 40, float* elementtype(float) %52, i64 %9)
; CHECK:   %54 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %53, i64 %13)
; CHECK:   %63 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %61, i64 %7)
; CHECK:   %72 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %65, i64 11)
; CHECK:   %85 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %83, i64 %7)
; CHECK:   %98 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 40, float* elementtype(float) %87, i64 %78)
; CHECK:   %99 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %98, i64 %82)
; CHECK:   %104 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %102, i64 %7)
; CHECK:   %121 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 400, float* elementtype(float) %106, i64 %101)
; CHECK:   %122 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 40, float* elementtype(float) %121, i64 %78)
; CHECK:   %123 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %122, i64 %82)
; CHECK:   %132 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %130, i64 %7)
; CHECK:   %141 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %134, i64 11)

; Check that the constants were propagated for function @arr_mod_mp_print_arr_
; CHECK: define internal void @arr_mod_mp_print_arr_
; CHECK:   %46 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %44, i64 %20)
; CHECK:   %59 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 40, float* elementtype(float) %48, i64 %41)
; CHECK:   %60 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %59, i64 %43)
; CHECK:   %67 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %65, i64 %20)
; CHECK:   %84 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 400, float* elementtype(float) %69, i64 %64)
; CHECK:   %85 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 40, float* elementtype(float) %84, i64 %41)
; CHECK:   %86 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %85, i64 %43)
; CHECK:   %99 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %97, i64 %20)
; CHECK:   %110 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %101, i64 %109)
; CHECK:   %142 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %140, i64 %20)
; CHECK:   %155 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 40, float* elementtype(float) %144, i64 %137)
; CHECK:   %156 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %155, i64 %139)
; CHECK:   %163 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %161, i64 %20)
; CHECK:   %180 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 400, float* elementtype(float) %165, i64 %160)
; CHECK:   %181 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 40, float* elementtype(float) %180, i64 %137)
; CHECK:   %182 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %181, i64 %139)
; CHECK:   %195 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %193, i64 %20)
; CHECK:   %206 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %197, i64 %205)

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
@arr_mod_mp_b_ = internal global %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$" { %"ARR_MOD$.btT_TESTTYPE"* null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@anon.5adb142a4af92269a23dd8f105f60717.0 = internal unnamed_addr constant i32 2
@anon.5adb142a4af92269a23dd8f105f60717.1 = internal unnamed_addr constant i32 10

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_allocate_arr_(i32* noalias nocapture readonly dereferenceable(4) %0) #0 {
  %2 = alloca i64, align 8
  %3 = alloca i64, align 8
  %4 = load i32, i32* %0, align 1
  %5 = icmp eq i32 %4, 1
  br i1 %5, label %8, label %6

6:                                                ; preds = %1
  %7 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  br label %36

8:                                                ; preds = %1
  %9 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 3), align 8
  %10 = and i64 %9, 1030792151296
  %11 = or i64 %10, 133
  store i64 %11, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 3), align 8
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 5), align 8
  store i64 288, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 1), align 8
  store i64 1, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 4), align 16
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 2), align 16
  %12 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, i64* %12, align 1
  %13 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 1, i64* %13, align 1
  %14 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 288, i64* %14, align 1
  %15 = call i32 (i64*, i32, ...) @for_check_mult_overflow64(i64* nonnull %2, i32 2, i64 1, i64 288) #3
  %16 = load i64, i64* %2, align 8
  %17 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 3), align 8
  %18 = and i64 %17, -68451041281
  %19 = or i64 %18, 1073741824
  store i64 %19, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 3), align 8
  %20 = trunc i64 %17 to i32
  %21 = shl i32 %20, 1
  %22 = and i32 %21, 2
  %23 = shl i32 %15, 4
  %24 = and i32 %23, 16
  %25 = lshr i64 %17, 15
  %26 = trunc i64 %25 to i32
  %27 = and i32 %26, 31457280
  %28 = and i32 %26, 33554432
  %29 = or i32 %24, %22
  %30 = or i32 %29, %27
  %31 = or i32 %30, %28
  %32 = or i32 %31, 262144
  %33 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 5), align 8
  %34 = inttoptr i64 %33 to i8*
  %35 = tail call i32 @for_alloc_allocatable_handle(i64 %16, i8** bitcast (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_ to i8**), i32 %32, i8* %34) #3
  br label %36

36:                                               ; preds = %8, %6
  %37 = phi i64* [ %7, %6 ], [ %12, %8 ]
  %38 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %39 = load i64, i64* %37, align 1
  %40 = sext i32 %4 to i64
  %41 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %39, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %38, i64 %40)
  %42 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %41, i64 0, i32 0, i32 3
  %43 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %41, i64 0, i32 0, i32 5
  store i64 0, i64* %43, align 1
  %44 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %41, i64 0, i32 0, i32 1
  store i64 4, i64* %44, align 1
  %45 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %41, i64 0, i32 0, i32 4
  store i64 2, i64* %45, align 1
  %46 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %41, i64 0, i32 0, i32 2
  store i64 0, i64* %46, align 1
  %47 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %41, i64 0, i32 0, i32 6, i64 0, i32 2
  %48 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %47, i32 0)
  store i64 1, i64* %48, align 1
  %49 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %41, i64 0, i32 0, i32 6, i64 0, i32 0
  %50 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %49, i32 0)
  store i64 10, i64* %50, align 1
  %51 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %47, i32 1)
  store i64 1, i64* %51, align 1
  %52 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %49, i32 1)
  store i64 10, i64* %52, align 1
  %53 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %41, i64 0, i32 0, i32 6, i64 0, i32 1
  %54 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %53, i32 0)
  store i64 4, i64* %54, align 1
  %55 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %53, i32 1)
  store i64 40, i64* %55, align 1
  store i64 1073741829, i64* %42, align 1
  %56 = bitcast %"ARR_MOD$.btT_TESTTYPE"* %41 to i8**
  %57 = tail call i32 @for_allocate_handle(i64 400, i8** %56, i32 262144, i8* null) #3
  %58 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %59 = load i64, i64* %37, align 1
  %60 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %59, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %58, i64 %40)
  %61 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %60, i64 0, i32 1, i32 3
  %62 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %60, i64 0, i32 1, i32 5
  store i64 0, i64* %62, align 1
  %63 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %60, i64 0, i32 1, i32 1
  store i64 4, i64* %63, align 1
  %64 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %60, i64 0, i32 1, i32 4
  store i64 3, i64* %64, align 1
  %65 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %60, i64 0, i32 1, i32 2
  store i64 0, i64* %65, align 1
  %66 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %60, i64 0, i32 1, i32 6, i64 0, i32 2
  %67 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %66, i32 0)
  store i64 1, i64* %67, align 1
  %68 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %60, i64 0, i32 1, i32 6, i64 0, i32 0
  %69 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %68, i32 0)
  store i64 10, i64* %69, align 1
  %70 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %66, i32 1)
  store i64 1, i64* %70, align 1
  %71 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %68, i32 1)
  store i64 10, i64* %71, align 1
  %72 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %66, i32 2)
  store i64 1, i64* %72, align 1
  %73 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %68, i32 2)
  store i64 10, i64* %73, align 1
  %74 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %60, i64 0, i32 1, i32 6, i64 0, i32 1
  %75 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %74, i32 0)
  store i64 4, i64* %75, align 1
  %76 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %74, i32 1)
  store i64 40, i64* %76, align 1
  %77 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %74, i32 2)
  store i64 400, i64* %77, align 1
  %78 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %60, i64 0, i32 1, i32 0
  store i64 1073741829, i64* %61, align 1
  %79 = bitcast float** %78 to i8**
  %80 = tail call i32 @for_allocate_handle(i64 4000, i8** nonnull %79, i32 262144, i8* null) #3
  %81 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %82 = load i64, i64* %37, align 1
  %83 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %82, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %81, i64 %40)
  %84 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %83, i64 0, i32 2, i32 3
  %85 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %83, i64 0, i32 2, i32 5
  store i64 0, i64* %85, align 1
  %86 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %83, i64 0, i32 2, i32 1
  store i64 4, i64* %86, align 1
  %87 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %83, i64 0, i32 2, i32 4
  store i64 1, i64* %87, align 1
  %88 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %83, i64 0, i32 2, i32 2
  store i64 0, i64* %88, align 1
  %89 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %83, i64 0, i32 2, i32 6, i64 0, i32 2
  %90 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %89, i32 0)
  store i64 1, i64* %90, align 1
  %91 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %83, i64 0, i32 2, i32 6, i64 0, i32 0
  %92 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %91, i32 0)
  store i64 10, i64* %92, align 1
  %93 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %83, i64 0, i32 2, i32 6, i64 0, i32 1
  %94 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %93, i32 0)
  store i64 4, i64* %94, align 1
  %95 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %83, i64 0, i32 2, i32 0
  store i64 1073741829, i64* %84, align 1
  %96 = bitcast float** %95 to i8**
  %97 = tail call i32 @for_allocate_handle(i64 40, i8** nonnull %96, i32 262144, i8* null) #3
  br i1 %5, label %100, label %98

98:                                               ; preds = %36
  %99 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 6, i64 0, i32 2), i32 0)
  br label %128

100:                                              ; preds = %36
  %101 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 3), align 8
  %102 = and i64 %101, 1030792151296
  %103 = or i64 %102, 133
  store i64 %103, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 3), align 8
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 5), align 8
  store i64 288, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 1), align 8
  store i64 1, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 4), align 16
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 2), align 16
  %104 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, i64* %104, align 1
  %105 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 %40, i64* %105, align 1
  %106 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 288, i64* %106, align 1
  %107 = call i32 (i64*, i32, ...) @for_check_mult_overflow64(i64* nonnull %3, i32 2, i64 %40, i64 288) #3
  %108 = load i64, i64* %3, align 8
  %109 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 3), align 8
  %110 = and i64 %109, -68451041281
  %111 = or i64 %110, 1073741824
  store i64 %111, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 3), align 8
  %112 = trunc i64 %109 to i32
  %113 = shl i32 %112, 1
  %114 = and i32 %113, 2
  %115 = shl i32 %107, 4
  %116 = and i32 %115, 16
  %117 = lshr i64 %109, 15
  %118 = trunc i64 %117 to i32
  %119 = and i32 %118, 31457280
  %120 = and i32 %118, 33554432
  %121 = or i32 %116, %114
  %122 = or i32 %121, %119
  %123 = or i32 %122, %120
  %124 = or i32 %123, 262144
  %125 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 5), align 8
  %126 = inttoptr i64 %125 to i8*
  %127 = tail call i32 @for_alloc_allocatable_handle(i64 %108, i8** bitcast (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_ to i8**), i32 %124, i8* %126) #3
  br label %128

128:                                              ; preds = %100, %98
  %129 = phi i64* [ %99, %98 ], [ %104, %100 ]
  %130 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 0), align 16
  %131 = load i64, i64* %129, align 1
  %132 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %131, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %130, i64 %40)
  %133 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %132, i64 0, i32 0, i32 3
  %134 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %132, i64 0, i32 0, i32 5
  store i64 0, i64* %134, align 1
  %135 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %132, i64 0, i32 0, i32 1
  store i64 4, i64* %135, align 1
  %136 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %132, i64 0, i32 0, i32 4
  store i64 2, i64* %136, align 1
  %137 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %132, i64 0, i32 0, i32 2
  store i64 0, i64* %137, align 1
  %138 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %132, i64 0, i32 0, i32 6, i64 0, i32 2
  %139 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %138, i32 0)
  store i64 1, i64* %139, align 1
  %140 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %132, i64 0, i32 0, i32 6, i64 0, i32 0
  %141 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %140, i32 0)
  store i64 10, i64* %141, align 1
  %142 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %138, i32 1)
  store i64 1, i64* %142, align 1
  %143 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %140, i32 1)
  store i64 10, i64* %143, align 1
  %144 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %132, i64 0, i32 0, i32 6, i64 0, i32 1
  %145 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %144, i32 0)
  store i64 4, i64* %145, align 1
  %146 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %144, i32 1)
  store i64 40, i64* %146, align 1
  store i64 1073741829, i64* %133, align 1
  %147 = bitcast %"ARR_MOD$.btT_TESTTYPE"* %132 to i8**
  %148 = tail call i32 @for_allocate_handle(i64 400, i8** %147, i32 262144, i8* null) #3
  %149 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 0), align 16
  %150 = load i64, i64* %129, align 1
  %151 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %150, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %149, i64 %40)
  %152 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %151, i64 0, i32 1, i32 3
  %153 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %151, i64 0, i32 1, i32 5
  store i64 0, i64* %153, align 1
  %154 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %151, i64 0, i32 1, i32 1
  store i64 4, i64* %154, align 1
  %155 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %151, i64 0, i32 1, i32 4
  store i64 3, i64* %155, align 1
  %156 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %151, i64 0, i32 1, i32 2
  store i64 0, i64* %156, align 1
  %157 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %151, i64 0, i32 1, i32 6, i64 0, i32 2
  %158 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %157, i32 0)
  store i64 1, i64* %158, align 1
  %159 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %151, i64 0, i32 1, i32 6, i64 0, i32 0
  %160 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %159, i32 0)
  store i64 10, i64* %160, align 1
  %161 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %157, i32 1)
  store i64 1, i64* %161, align 1
  %162 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %159, i32 1)
  store i64 10, i64* %162, align 1
  %163 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %157, i32 2)
  store i64 1, i64* %163, align 1
  %164 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %159, i32 2)
  store i64 10, i64* %164, align 1
  %165 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %151, i64 0, i32 1, i32 6, i64 0, i32 1
  %166 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %165, i32 0)
  store i64 4, i64* %166, align 1
  %167 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %165, i32 1)
  store i64 40, i64* %167, align 1
  %168 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %165, i32 2)
  store i64 400, i64* %168, align 1
  %169 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %151, i64 0, i32 1, i32 0
  store i64 1073741829, i64* %152, align 1
  %170 = bitcast float** %169 to i8**
  %171 = tail call i32 @for_allocate_handle(i64 4000, i8** nonnull %170, i32 262144, i8* null) #3
  %172 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 0), align 16
  %173 = load i64, i64* %129, align 1
  %174 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %173, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %172, i64 %40)
  %175 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %174, i64 0, i32 2, i32 3
  %176 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %174, i64 0, i32 2, i32 5
  store i64 0, i64* %176, align 1
  %177 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %174, i64 0, i32 2, i32 1
  store i64 4, i64* %177, align 1
  %178 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %174, i64 0, i32 2, i32 4
  store i64 1, i64* %178, align 1
  %179 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %174, i64 0, i32 2, i32 2
  store i64 0, i64* %179, align 1
  %180 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %174, i64 0, i32 2, i32 6, i64 0, i32 2
  %181 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %180, i32 0)
  store i64 1, i64* %181, align 1
  %182 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %174, i64 0, i32 2, i32 6, i64 0, i32 0
  %183 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %182, i32 0)
  store i64 10, i64* %183, align 1
  %184 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %174, i64 0, i32 2, i32 6, i64 0, i32 1
  %185 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %184, i32 0)
  store i64 4, i64* %185, align 1
  %186 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %174, i64 0, i32 2, i32 0
  store i64 1073741829, i64* %175, align 1
  %187 = bitcast float** %186 to i8**
  %188 = tail call i32 @for_allocate_handle(i64 40, i8** nonnull %187, i32 262144, i8* null) #3
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

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_initialize_arr_(i32* noalias nocapture readonly dereferenceable(4) %0, i32* noalias nocapture readonly dereferenceable(4) %1, i32* noalias nocapture readonly dereferenceable(4) %2, i32* noalias nocapture readonly dereferenceable(4) %3) #0 {
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
  %76 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 6, i64 0, i32 2), i32 0)
  br label %77

77:                                               ; preds = %129, %75
  %78 = phi i64 [ 1, %75 ], [ %142, %129 ]
  %79 = trunc i64 %78 to i32
  %80 = sitofp i32 %79 to float
  br label %81

81:                                               ; preds = %126, %77
  %82 = phi i64 [ 1, %77 ], [ %127, %126 ]
  %83 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 0), align 16
  %84 = load i64, i64* %76, align 1
  %85 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %84, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %83, i64 %7)
  %86 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %85, i64 0, i32 0, i32 0
  %87 = load float*, float** %86, align 1
  %88 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %85, i64 0, i32 0, i32 6, i64 0, i32 1
  %89 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %88, i32 0)
  %90 = load i64, i64* %89, align 1
  %91 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %85, i64 0, i32 0, i32 6, i64 0, i32 2
  %92 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %91, i32 0)
  %93 = load i64, i64* %92, align 1
  %94 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %88, i32 1)
  %95 = load i64, i64* %94, align 1
  %96 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %91, i32 1)
  %97 = load i64, i64* %96, align 1
  %98 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %97, i64 %95, float* elementtype(float) %87, i64 %78)
  %99 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %93, i64 %90, float* elementtype(float) %98, i64 %82)
  store float %80, float* %99, align 1
  br label %100

100:                                              ; preds = %81, %100
  %101 = phi i64 [ %124, %100 ], [ 1, %81 ]
  %102 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 0), align 16
  %103 = load i64, i64* %76, align 1
  %104 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %103, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %102, i64 %7)
  %105 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %104, i64 0, i32 1, i32 0
  %106 = load float*, float** %105, align 1
  %107 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %104, i64 0, i32 1, i32 6, i64 0, i32 1
  %108 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %107, i32 0)
  %109 = load i64, i64* %108, align 1
  %110 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %104, i64 0, i32 1, i32 6, i64 0, i32 2
  %111 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %110, i32 0)
  %112 = load i64, i64* %111, align 1
  %113 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %107, i32 1)
  %114 = load i64, i64* %113, align 1
  %115 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %110, i32 1)
  %116 = load i64, i64* %115, align 1
  %117 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %107, i32 2)
  %118 = load i64, i64* %117, align 1
  %119 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %110, i32 2)
  %120 = load i64, i64* %119, align 1
  %121 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 %120, i64 %118, float* elementtype(float) %106, i64 %101)
  %122 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %116, i64 %114, float* elementtype(float) %121, i64 %78)
  %123 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %112, i64 %109, float* elementtype(float) %122, i64 %82)
  store float %80, float* %123, align 1
  %124 = add nuw nsw i64 %101, 1
  %125 = icmp eq i64 %124, 11
  br i1 %125, label %126, label %100

126:                                              ; preds = %100
  %127 = add nuw nsw i64 %82, 1
  %128 = icmp eq i64 %127, 11
  br i1 %128, label %129, label %81

129:                                              ; preds = %126
  %130 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 0), align 16
  %131 = load i64, i64* %76, align 1
  %132 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %131, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %130, i64 %7)
  %133 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %132, i64 0, i32 2, i32 0
  %134 = load float*, float** %133, align 1
  %135 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %132, i64 0, i32 2, i32 6, i64 0, i32 1
  %136 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %135, i32 0)
  %137 = load i64, i64* %136, align 1
  %138 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %132, i64 0, i32 2, i32 6, i64 0, i32 2
  %139 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %138, i32 0)
  %140 = load i64, i64* %139, align 1
  %141 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %140, i64 %137, float* elementtype(float) %134, i64 11)
  store float 1.100000e+01, float* %141, align 1
  %142 = add nuw nsw i64 %78, 1
  %143 = icmp eq i64 %142, 11
  br i1 %143, label %144, label %77

144:                                              ; preds = %129
  ret void
}

; Function Attrs: nounwind readnone speculatable
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
  %12 = alloca [4 x i8], align 1
  %13 = alloca { float }, align 8
  %14 = alloca [4 x i8], align 1
  %15 = alloca { float }, align 8
  %16 = alloca [4 x i8], align 1
  %17 = alloca { float }, align 8
  %18 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %19 = load i32, i32* %0, align 1
  %20 = sext i32 %19 to i64
  %21 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 0
  %22 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 1
  %23 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 2
  %24 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 3
  %25 = getelementptr inbounds { float }, { float }* %7, i64 0, i32 0
  %26 = bitcast [8 x i64]* %5 to i8*
  %27 = bitcast { float }* %7 to i8*
  %28 = getelementptr inbounds [4 x i8], [4 x i8]* %8, i64 0, i64 0
  %29 = getelementptr inbounds [4 x i8], [4 x i8]* %8, i64 0, i64 1
  %30 = getelementptr inbounds [4 x i8], [4 x i8]* %8, i64 0, i64 2
  %31 = getelementptr inbounds [4 x i8], [4 x i8]* %8, i64 0, i64 3
  %32 = getelementptr inbounds { float }, { float }* %9, i64 0, i32 0
  %33 = bitcast { float }* %9 to i8*
  %34 = getelementptr inbounds [4 x i8], [4 x i8]* %10, i64 0, i64 0
  %35 = getelementptr inbounds [4 x i8], [4 x i8]* %10, i64 0, i64 1
  %36 = getelementptr inbounds [4 x i8], [4 x i8]* %10, i64 0, i64 2
  %37 = getelementptr inbounds [4 x i8], [4 x i8]* %10, i64 0, i64 3
  %38 = getelementptr inbounds { float }, { float }* %11, i64 0, i32 0
  %39 = bitcast { float }* %11 to i8*
  br label %40

40:                                               ; preds = %96, %4
  %41 = phi i64 [ 1, %4 ], [ %113, %96 ]
  br label %42

42:                                               ; preds = %40, %92
  %43 = phi i64 [ %93, %92 ], [ 1, %40 ]
  %44 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %45 = load i64, i64* %18, align 1
  %46 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %45, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %44, i64 %20)
  %47 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %46, i64 0, i32 0, i32 0
  %48 = load float*, float** %47, align 1
  %49 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %46, i64 0, i32 0, i32 6, i64 0, i32 1
  %50 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %49, i32 0)
  %51 = load i64, i64* %50, align 1
  %52 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %46, i64 0, i32 0, i32 6, i64 0, i32 2
  %53 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %52, i32 0)
  %54 = load i64, i64* %53, align 1
  %55 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %49, i32 1)
  %56 = load i64, i64* %55, align 1
  %57 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %52, i32 1)
  %58 = load i64, i64* %57, align 1
  %59 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %58, i64 %56, float* elementtype(float) %48, i64 %41)
  %60 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %54, i64 %51, float* elementtype(float) %59, i64 %43)
  %61 = load float, float* %60, align 1
  store i8 26, i8* %21, align 1
  store i8 1, i8* %22, align 1
  store i8 1, i8* %23, align 1
  store i8 0, i8* %24, align 1
  store float %61, float* %25, align 8
  %62 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %26, i32 -1, i64 1239157112576, i8* nonnull %21, i8* nonnull %27) #3
  br label %63

63:                                               ; preds = %42, %63
  %64 = phi i64 [ %89, %63 ], [ 1, %42 ]
  %65 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %66 = load i64, i64* %18, align 1
  %67 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %66, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %65, i64 %20)
  %68 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %67, i64 0, i32 1, i32 0
  %69 = load float*, float** %68, align 1
  %70 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %67, i64 0, i32 1, i32 6, i64 0, i32 1
  %71 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %70, i32 0)
  %72 = load i64, i64* %71, align 1
  %73 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %67, i64 0, i32 1, i32 6, i64 0, i32 2
  %74 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %73, i32 0)
  %75 = load i64, i64* %74, align 1
  %76 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %70, i32 1)
  %77 = load i64, i64* %76, align 1
  %78 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %73, i32 1)
  %79 = load i64, i64* %78, align 1
  %80 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %70, i32 2)
  %81 = load i64, i64* %80, align 1
  %82 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %73, i32 2)
  %83 = load i64, i64* %82, align 1
  %84 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 %83, i64 %81, float* elementtype(float) %69, i64 %64)
  %85 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %79, i64 %77, float* elementtype(float) %84, i64 %41)
  %86 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %75, i64 %72, float* elementtype(float) %85, i64 %43)
  %87 = load float, float* %86, align 1
  store i8 26, i8* %28, align 1
  store i8 1, i8* %29, align 1
  store i8 1, i8* %30, align 1
  store i8 0, i8* %31, align 1
  store float %87, float* %32, align 8
  %88 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %26, i32 -1, i64 1239157112576, i8* nonnull %28, i8* nonnull %33) #3
  %89 = add nuw i64 %64, 1
  %90 = trunc i64 %89 to i32
  %91 = icmp slt i32 10, %90
  br i1 %91, label %92, label %63

92:                                               ; preds = %63
  %93 = add nuw i64 %43, 1
  %94 = trunc i64 %93 to i32
  %95 = icmp slt i32 10, %94
  br i1 %95, label %96, label %42

96:                                               ; preds = %92
  %97 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %98 = load i64, i64* %18, align 1
  %99 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %98, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %97, i64 %20)
  %100 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %99, i64 0, i32 2, i32 0
  %101 = load float*, float** %100, align 1
  %102 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %99, i64 0, i32 2, i32 6, i64 0, i32 1
  %103 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %102, i32 0)
  %104 = load i64, i64* %103, align 1
  %105 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %99, i64 0, i32 2, i32 6, i64 0, i32 2
  %106 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %105, i32 0)
  %107 = load i64, i64* %106, align 1
  %108 = shl i64 %93, 32
  %109 = ashr exact i64 %108, 32
  %110 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %107, i64 %104, float* elementtype(float) %101, i64 %109)
  %111 = load float, float* %110, align 1
  store i8 26, i8* %34, align 1
  store i8 1, i8* %35, align 1
  store i8 1, i8* %36, align 1
  store i8 0, i8* %37, align 1
  store float %111, float* %38, align 8
  %112 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %26, i32 -1, i64 1239157112576, i8* nonnull %34, i8* nonnull %39) #3
  %113 = add nuw i64 %41, 1
  %114 = trunc i64 %113 to i32
  %115 = icmp slt i32 10, %114
  br i1 %115, label %116, label %40

116:                                              ; preds = %96
  %117 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %118 = getelementptr inbounds [4 x i8], [4 x i8]* %12, i64 0, i64 0
  %119 = getelementptr inbounds [4 x i8], [4 x i8]* %12, i64 0, i64 1
  %120 = getelementptr inbounds [4 x i8], [4 x i8]* %12, i64 0, i64 2
  %121 = getelementptr inbounds [4 x i8], [4 x i8]* %12, i64 0, i64 3
  %122 = getelementptr inbounds { float }, { float }* %13, i64 0, i32 0
  %123 = bitcast { float }* %13 to i8*
  %124 = getelementptr inbounds [4 x i8], [4 x i8]* %14, i64 0, i64 0
  %125 = getelementptr inbounds [4 x i8], [4 x i8]* %14, i64 0, i64 1
  %126 = getelementptr inbounds [4 x i8], [4 x i8]* %14, i64 0, i64 2
  %127 = getelementptr inbounds [4 x i8], [4 x i8]* %14, i64 0, i64 3
  %128 = getelementptr inbounds { float }, { float }* %15, i64 0, i32 0
  %129 = bitcast { float }* %15 to i8*
  %130 = getelementptr inbounds [4 x i8], [4 x i8]* %16, i64 0, i64 0
  %131 = getelementptr inbounds [4 x i8], [4 x i8]* %16, i64 0, i64 1
  %132 = getelementptr inbounds [4 x i8], [4 x i8]* %16, i64 0, i64 2
  %133 = getelementptr inbounds [4 x i8], [4 x i8]* %16, i64 0, i64 3
  %134 = getelementptr inbounds { float }, { float }* %17, i64 0, i32 0
  %135 = bitcast { float }* %17 to i8*
  br label %136

136:                                              ; preds = %192, %116
  %137 = phi i64 [ 1, %116 ], [ %209, %192 ]
  br label %138

138:                                              ; preds = %136, %188
  %139 = phi i64 [ %189, %188 ], [ 1, %136 ]
  %140 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 0), align 16
  %141 = load i64, i64* %117, align 1
  %142 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %141, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %140, i64 %20)
  %143 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %142, i64 0, i32 0, i32 0
  %144 = load float*, float** %143, align 1
  %145 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %142, i64 0, i32 0, i32 6, i64 0, i32 1
  %146 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %145, i32 0)
  %147 = load i64, i64* %146, align 1
  %148 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %142, i64 0, i32 0, i32 6, i64 0, i32 2
  %149 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %148, i32 0)
  %150 = load i64, i64* %149, align 1
  %151 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %145, i32 1)
  %152 = load i64, i64* %151, align 1
  %153 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %148, i32 1)
  %154 = load i64, i64* %153, align 1
  %155 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %154, i64 %152, float* elementtype(float) %144, i64 %137)
  %156 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %150, i64 %147, float* elementtype(float) %155, i64 %139)
  %157 = load float, float* %156, align 1
  store i8 26, i8* %118, align 1
  store i8 1, i8* %119, align 1
  store i8 1, i8* %120, align 1
  store i8 0, i8* %121, align 1
  store float %157, float* %122, align 8
  %158 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %26, i32 -1, i64 1239157112576, i8* nonnull %118, i8* nonnull %123) #3
  br label %159

159:                                              ; preds = %138, %159
  %160 = phi i64 [ %185, %159 ], [ 1, %138 ]
  %161 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 0), align 16
  %162 = load i64, i64* %117, align 1
  %163 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %162, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %161, i64 %20)
  %164 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %163, i64 0, i32 1, i32 0
  %165 = load float*, float** %164, align 1
  %166 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %163, i64 0, i32 1, i32 6, i64 0, i32 1
  %167 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %166, i32 0)
  %168 = load i64, i64* %167, align 1
  %169 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %163, i64 0, i32 1, i32 6, i64 0, i32 2
  %170 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %169, i32 0)
  %171 = load i64, i64* %170, align 1
  %172 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %166, i32 1)
  %173 = load i64, i64* %172, align 1
  %174 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %169, i32 1)
  %175 = load i64, i64* %174, align 1
  %176 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %166, i32 2)
  %177 = load i64, i64* %176, align 1
  %178 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %169, i32 2)
  %179 = load i64, i64* %178, align 1
  %180 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 %179, i64 %177, float* elementtype(float) %165, i64 %160)
  %181 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %175, i64 %173, float* elementtype(float) %180, i64 %137)
  %182 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %171, i64 %168, float* elementtype(float) %181, i64 %139)
  %183 = load float, float* %182, align 1
  store i8 26, i8* %124, align 1
  store i8 1, i8* %125, align 1
  store i8 1, i8* %126, align 1
  store i8 0, i8* %127, align 1
  store float %183, float* %128, align 8
  %184 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %26, i32 -1, i64 1239157112576, i8* nonnull %124, i8* nonnull %129) #3
  %185 = add nuw i64 %160, 1
  %186 = trunc i64 %185 to i32
  %187 = icmp slt i32 10, %186
  br i1 %187, label %188, label %159

188:                                              ; preds = %159
  %189 = add nuw i64 %139, 1
  %190 = trunc i64 %189 to i32
  %191 = icmp slt i32 10, %190
  br i1 %191, label %192, label %138

192:                                              ; preds = %188
  %193 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_b_, i64 0, i32 0), align 16
  %194 = load i64, i64* %117, align 1
  %195 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %194, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %193, i64 %20)
  %196 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %195, i64 0, i32 2, i32 0
  %197 = load float*, float** %196, align 1
  %198 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %195, i64 0, i32 2, i32 6, i64 0, i32 1
  %199 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %198, i32 0)
  %200 = load i64, i64* %199, align 1
  %201 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %195, i64 0, i32 2, i32 6, i64 0, i32 2
  %202 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %201, i32 0)
  %203 = load i64, i64* %202, align 1
  %204 = shl i64 %189, 32
  %205 = ashr exact i64 %204, 32
  %206 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %203, i64 %200, float* elementtype(float) %197, i64 %205)
  %207 = load float, float* %206, align 1
  store i8 26, i8* %130, align 1
  store i8 1, i8* %131, align 1
  store i8 1, i8* %132, align 1
  store i8 0, i8* %133, align 1
  store float %207, float* %134, align 8
  %208 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %26, i32 -1, i64 1239157112576, i8* nonnull %130, i8* nonnull %135) #3
  %209 = add nuw i64 %137, 1
  %210 = trunc i64 %209 to i32
  %211 = icmp slt i32 10, %210
  br i1 %211, label %212, label %136

212:                                              ; preds = %192
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr #2

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #0 {
  %1 = alloca i32, align 8
  %2 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.5adb142a4af92269a23dd8f105f60717.0) #3
  store i32 1, i32* %1, align 8
  br label %3

3:                                                ; preds = %3, %0
  %4 = phi i32 [ %5, %3 ], [ 1, %0 ]
  call void @arr_mod_mp_allocate_arr_(i32* nonnull %1)
  call void @arr_mod_mp_initialize_arr_(i32* nonnull %1, i32* nonnull @anon.5adb142a4af92269a23dd8f105f60717.1, i32* nonnull @anon.5adb142a4af92269a23dd8f105f60717.1, i32* nonnull @anon.5adb142a4af92269a23dd8f105f60717.1)
  call void @arr_mod_mp_print_arr_(i32* nonnull %1, i32* nonnull @anon.5adb142a4af92269a23dd8f105f60717.1, i32* nonnull @anon.5adb142a4af92269a23dd8f105f60717.1, i32* nonnull @anon.5adb142a4af92269a23dd8f105f60717.1)
  %5 = add nuw nsw i32 %4, 1
  store i32 %5, i32* %1, align 8
  %6 = icmp eq i32 %5, 11
  br i1 %6, label %7, label %3

7:                                                ; preds = %3
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(i32* nocapture readonly) local_unnamed_addr #2

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
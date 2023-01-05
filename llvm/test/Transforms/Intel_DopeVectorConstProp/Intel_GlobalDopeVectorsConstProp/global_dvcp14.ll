; REQUIRES: asserts
; RUN: opt < %s -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-GLOBDV

; RUN: opt < %s -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-FIELD0

; RUN: opt < %s -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-FIELD1

; RUN: opt < %s -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-FIELD2

; This test case checks that the constant propagation works even if one field
; wasn't used (dead nested dope vector). It was created from this test case:

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

; In this test case, inner_array_C was allocated but never initialized or used.
; This means that inner_array_C is dead. The debug output should present that
; the constants were NOT propagated for the dope vector (field 2), but they
; were propagated for the global dope vector, field 0 and fiel 1.

; CHECK-GLOBDV: Global variable: arr_mod_mp_a_
; CHECK-GLOBDV:   LLVM Type: QNCA_a0$%"ARR_MOD$.btT_TESTTYPE"*$rank1$
; CHECK-GLOBDV:   Global dope vector result: Pass
; CHECK-GLOBDV:   Dope vector analysis result: Pass
; CHECK-GLOBDV:   Constant propagation status: performed
; CHECK-GLOBDV:     [0] Array Pointer: Read
; CHECK-GLOBDV:     [1] Element size: Written | Constant = i64 288
; CHECK-GLOBDV:     [2] Co-Dimension: Written | Constant = i64 0
; CHECK-GLOBDV:     [3] Flags: Read | Written
; CHECK-GLOBDV:     [4] Dimensions: Written | Constant = i64 1
; CHECK-GLOBDV:     [6][0] Extent: Written | Constant = i64 1
; CHECK-GLOBDV:     [6][0] Stride: Written | Constant = i64 288
; CHECK-GLOBDV:     [6][0] Lower Bound: Read | Written | Constant = i64 1
; CHECK-GLOBDV:   Nested dope vectors: 3

; CHECK-FIELD1:    Field[1]: QNCA_a0$float*$rank3$
; CHECK-FIELD1:      Dope vector analysis result: Pass
; CHECK-FIELD1:      Constant propagation status: performed
; CHECK-FIELD1:        [0] Array Pointer: Read
; CHECK-FIELD1:        [1] Element size: Written | Constant = i64 4
; CHECK-FIELD1:        [2] Co-Dimension: Written | Constant = i64 0
; CHECK-FIELD1:        [3] Flags: Written
; CHECK-FIELD1:        [4] Dimensions: Written | Constant = i64 3
; CHECK-FIELD1:        [6][0] Extent: Written | Constant = i64 10
; CHECK-FIELD1:        [6][0] Stride: Read | Written | Constant = i64 4
; CHECK-FIELD1:        [6][0] Lower Bound: Read | Written | Constant = i64 1
; CHECK-FIELD1:        [6][1] Extent: Written | Constant = i64 10
; CHECK-FIELD1:        [6][1] Stride: Read | Written | Constant = i64 40
; CHECK-FIELD1:        [6][1] Lower Bound: Read | Written | Constant = i64 1
; CHECK-FIELD1:        [6][2] Extent: Written | Constant = i64 10
; CHECK-FIELD1:        [6][2] Stride: Read | Written | Constant = i64 400
; CHECK-FIELD1:        [6][2] Lower Bound: Read | Written | Constant = i64 1

; CHECK-FIELD0:    Field[0]: QNCA_a0$float*$rank2$
; CHECK-FIELD0:      Dope vector analysis result: Pass
; CHECK-FIELD0:      Constant propagation status: performed
; CHECK-FIELD0:        [0] Array Pointer: Read
; CHECK-FIELD0:        [1] Element size: Written | Constant = i64 4
; CHECK-FIELD0:        [2] Co-Dimension: Written | Constant = i64 0
; CHECK-FIELD0:        [3] Flags: Written
; CHECK-FIELD0:        [4] Dimensions: Written | Constant = i64 2
; CHECK-FIELD0:        [6][0] Extent: Written | Constant = i64 10
; CHECK-FIELD0:        [6][0] Stride: Read | Written | Constant = i64 4
; CHECK-FIELD0:        [6][0] Lower Bound: Read | Written | Constant = i64 1
; CHECK-FIELD0:        [6][1] Extent: Written | Constant = i64 10
; CHECK-FIELD0:        [6][1] Stride: Read | Written | Constant = i64 40
; CHECK-FIELD0:        [6][1] Lower Bound: Read | Written | Constant = i64 1

; CHECK-FIELD2:    Field[2]: QNCA_a0$float*$rank1$
; CHECK-FIELD2:      Dope vector analysis result: Pass
; CHECK-FIELD2:      Constant propagation status: NOT performed
; CHECK-FIELD2:        [0] Array Pointer:
; CHECK-FIELD2:        [1] Element size: Written | Constant = i64 4
; CHECK-FIELD2:        [2] Co-Dimension: Written | Constant = i64 0
; CHECK-FIELD2:        [3] Flags: Written
; CHECK-FIELD2:        [4] Dimensions: Written | Constant = i64 1
; CHECK-FIELD2:        [6][0] Extent: Written | Constant = i64 10
; CHECK-FIELD2:        [6][0] Stride: Written | Constant = i64 4
; CHECK-FIELD2:        [6][0] Lower Bound: Written | Constant = i64 1

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
  %14 = call i32 (i64*, i32, ...) @for_check_mult_overflow64(i64* nonnull %2, i32 2, i64 1, i64 288) #4
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
  %34 = tail call i32 @for_alloc_allocatable_handle(i64 %15, i8** bitcast (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_ to i8**), i32 %31, i8* %33) #4
  br label %35

35:                                               ; preds = %7, %5
  %36 = phi i64* [ %6, %5 ], [ %11, %7 ]
  %37 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %38 = load i64, i64* %36, align 1
  %39 = sext i32 %3 to i64
  %40 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %38, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %37, i64 %39)
  %41 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 3
  %42 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 5
  store i64 0, i64* %42, align 1
  %43 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 1
  store i64 4, i64* %43, align 1
  %44 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 4
  store i64 2, i64* %44, align 1
  %45 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 2
  store i64 0, i64* %45, align 1
  %46 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 6, i64 0, i32 2
  %47 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %46, i32 0)
  store i64 1, i64* %47, align 1
  %48 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 6, i64 0, i32 0
  %49 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %48, i32 0)
  store i64 10, i64* %49, align 1
  %50 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %46, i32 1)
  store i64 1, i64* %50, align 1
  %51 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %48, i32 1)
  store i64 10, i64* %51, align 1
  %52 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 6, i64 0, i32 1
  %53 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %52, i32 0)
  store i64 4, i64* %53, align 1
  %54 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %52, i32 1)
  store i64 40, i64* %54, align 1
  store i64 1073741829, i64* %41, align 1
  %55 = bitcast %"ARR_MOD$.btT_TESTTYPE"* %40 to i8**
  %56 = tail call i32 @for_allocate_handle(i64 400, i8** %55, i32 262144, i8* null) #4
  %57 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %58 = load i64, i64* %36, align 1
  %59 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %58, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %57, i64 %39)
  %60 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 3
  %61 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 5
  store i64 0, i64* %61, align 1
  %62 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 1
  store i64 4, i64* %62, align 1
  %63 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 4
  store i64 3, i64* %63, align 1
  %64 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 2
  store i64 0, i64* %64, align 1
  %65 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 6, i64 0, i32 2
  %66 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %65, i32 0)
  store i64 1, i64* %66, align 1
  %67 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 6, i64 0, i32 0
  %68 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %67, i32 0)
  store i64 10, i64* %68, align 1
  %69 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %65, i32 1)
  store i64 1, i64* %69, align 1
  %70 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %67, i32 1)
  store i64 10, i64* %70, align 1
  %71 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %65, i32 2)
  store i64 1, i64* %71, align 1
  %72 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %67, i32 2)
  store i64 10, i64* %72, align 1
  %73 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 6, i64 0, i32 1
  %74 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %73, i32 0)
  store i64 4, i64* %74, align 1
  %75 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %73, i32 1)
  store i64 40, i64* %75, align 1
  %76 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %73, i32 2)
  store i64 400, i64* %76, align 1
  %77 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 0
  store i64 1073741829, i64* %60, align 1
  %78 = bitcast float** %77 to i8**
  %79 = tail call i32 @for_allocate_handle(i64 4000, i8** nonnull %78, i32 262144, i8* null) #4
  %80 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %81 = load i64, i64* %36, align 1
  %82 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %81, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %80, i64 %39)
  %83 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %82, i64 0, i32 2, i32 3
  %84 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %82, i64 0, i32 2, i32 5
  store i64 0, i64* %84, align 1
  %85 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %82, i64 0, i32 2, i32 1
  store i64 4, i64* %85, align 1
  %86 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %82, i64 0, i32 2, i32 4
  store i64 1, i64* %86, align 1
  %87 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %82, i64 0, i32 2, i32 2
  store i64 0, i64* %87, align 1
  %88 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %82, i64 0, i32 2, i32 6, i64 0, i32 2
  %89 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %88, i32 0)
  store i64 1, i64* %89, align 1
  %90 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %82, i64 0, i32 2, i32 6, i64 0, i32 0
  %91 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %90, i32 0)
  store i64 10, i64* %91, align 1
  %92 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %82, i64 0, i32 2, i32 6, i64 0, i32 1
  %93 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %92, i32 0)
  store i64 4, i64* %93, align 1
  %94 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %82, i64 0, i32 2, i32 0
  store i64 1073741829, i64* %83, align 1
  %95 = bitcast float** %94 to i8**
  %96 = tail call i32 @for_allocate_handle(i64 40, i8** nonnull %95, i32 262144, i8* null) #4
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #1

; Function Attrs: nofree
declare dso_local i32 @for_check_mult_overflow64(i64* nocapture, i32, ...) local_unnamed_addr #2

; Function Attrs: nofree
declare dso_local i32 @for_alloc_allocatable_handle(i64, i8** nocapture, i32, i8*) local_unnamed_addr #2

; Function Attrs: nofree nosync nounwind readnone speculatable
declare %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8, i64, i64, %"ARR_MOD$.btT_TESTTYPE"*, i64) #1

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, i8** nocapture, i32, i8*) local_unnamed_addr #2

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_print_arr_(i32* noalias nocapture readonly dereferenceable(4) %0, i32* noalias nocapture readonly dereferenceable(4) %1, i32* noalias nocapture readonly dereferenceable(4) %2, i32* noalias nocapture readonly dereferenceable(4) %3) #0 {
  %5 = alloca [8 x i64], align 16
  %6 = alloca [4 x i8], align 1
  %7 = alloca { float }, align 8
  %8 = alloca [4 x i8], align 1
  %9 = alloca { float }, align 8
  %10 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %11 = load i32, i32* %0, align 1
  %12 = sext i32 %11 to i64
  %13 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 0
  %14 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 1
  %15 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 2
  %16 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 3
  %17 = getelementptr inbounds { float }, { float }* %7, i64 0, i32 0
  %18 = bitcast [8 x i64]* %5 to i8*
  %19 = bitcast { float }* %7 to i8*
  %20 = getelementptr inbounds [4 x i8], [4 x i8]* %8, i64 0, i64 0
  %21 = getelementptr inbounds [4 x i8], [4 x i8]* %8, i64 0, i64 1
  %22 = getelementptr inbounds [4 x i8], [4 x i8]* %8, i64 0, i64 2
  %23 = getelementptr inbounds [4 x i8], [4 x i8]* %8, i64 0, i64 3
  %24 = getelementptr inbounds { float }, { float }* %9, i64 0, i32 0
  %25 = bitcast { float }* %9 to i8*
  br label %26

26:                                               ; preds = %82, %4
  %27 = phi i64 [ 1, %4 ], [ %83, %82 ]
  br label %28

28:                                               ; preds = %26, %78
  %29 = phi i64 [ %79, %78 ], [ 1, %26 ]
  %30 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %31 = load i64, i64* %10, align 1
  %32 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %31, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %30, i64 %12)
  %33 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %32, i64 0, i32 0, i32 0
  %34 = load float*, float** %33, align 1
  %35 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %32, i64 0, i32 0, i32 6, i64 0, i32 1
  %36 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %35, i32 0)
  %37 = load i64, i64* %36, align 1
  %38 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %32, i64 0, i32 0, i32 6, i64 0, i32 2
  %39 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %38, i32 0)
  %40 = load i64, i64* %39, align 1
  %41 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %35, i32 1)
  %42 = load i64, i64* %41, align 1
  %43 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %38, i32 1)
  %44 = load i64, i64* %43, align 1
  %45 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %44, i64 %42, float* elementtype(float) %34, i64 %27)
  %46 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %40, i64 %37, float* elementtype(float) %45, i64 %29)
  %47 = load float, float* %46, align 1
  store i8 26, i8* %13, align 1
  store i8 1, i8* %14, align 1
  store i8 1, i8* %15, align 1
  store i8 0, i8* %16, align 1
  store float %47, float* %17, align 8
  %48 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %18, i32 -1, i64 1239157112576, i8* nonnull %13, i8* nonnull %19) #4
  br label %49

49:                                               ; preds = %28, %49
  %50 = phi i64 [ %75, %49 ], [ 1, %28 ]
  %51 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %52 = load i64, i64* %10, align 1
  %53 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %52, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %51, i64 %12)
  %54 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %53, i64 0, i32 1, i32 0
  %55 = load float*, float** %54, align 1
  %56 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %53, i64 0, i32 1, i32 6, i64 0, i32 1
  %57 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %56, i32 0)
  %58 = load i64, i64* %57, align 1
  %59 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %53, i64 0, i32 1, i32 6, i64 0, i32 2
  %60 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %59, i32 0)
  %61 = load i64, i64* %60, align 1
  %62 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %56, i32 1)
  %63 = load i64, i64* %62, align 1
  %64 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %59, i32 1)
  %65 = load i64, i64* %64, align 1
  %66 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %56, i32 2)
  %67 = load i64, i64* %66, align 1
  %68 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %59, i32 2)
  %69 = load i64, i64* %68, align 1
  %70 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 %69, i64 %67, float* elementtype(float) %55, i64 %50)
  %71 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %65, i64 %63, float* elementtype(float) %70, i64 %27)
  %72 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %61, i64 %58, float* elementtype(float) %71, i64 %29)
  %73 = load float, float* %72, align 1
  store i8 26, i8* %20, align 1
  store i8 1, i8* %21, align 1
  store i8 1, i8* %22, align 1
  store i8 0, i8* %23, align 1
  store float %73, float* %24, align 8
  %74 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %18, i32 -1, i64 1239157112576, i8* nonnull %20, i8* nonnull %25) #4
  %75 = add nuw i64 %50, 1
  %76 = trunc i64 %75 to i32
  %77 = icmp slt i32 10, %76
  br i1 %77, label %78, label %49

78:                                               ; preds = %49
  %79 = add nuw i64 %29, 1
  %80 = trunc i64 %79 to i32
  %81 = icmp slt i32 10, %80
  br i1 %81, label %82, label %28

82:                                               ; preds = %78
  %83 = add nuw i64 %27, 1
  %84 = trunc i64 %83 to i32
  %85 = icmp slt i32 10, %84
  br i1 %85, label %86, label %26

86:                                               ; preds = %82
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #1

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr #2

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #0 {
  %1 = alloca i32, align 8
  %2 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.87529b4ebf98830a9107fed24e462e82.0) #4
  store i32 1, i32* %1, align 8
  %3 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0) #4
  br label %4

4:                                                ; preds = %62, %0
  %5 = phi i32 [ %63, %62 ], [ 1, %0 ]
  call void @arr_mod_mp_allocate_arr_(i32* nonnull %1)
  %6 = zext i32 %5 to i64
  br label %7

7:                                                ; preds = %59, %4
  %8 = phi i64 [ %60, %59 ], [ 1, %4 ]
  %9 = trunc i64 %8 to i32
  %10 = sitofp i32 %9 to float
  br label %11

11:                                               ; preds = %56, %7
  %12 = phi i64 [ %57, %56 ], [ 1, %7 ]
  %13 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16, !noalias !3
  %14 = load i64, i64* %3, align 1, !noalias !3
  %15 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %14, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %13, i64 %6) #4
  %16 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %15, i64 0, i32 0, i32 0
  %17 = load float*, float** %16, align 1, !noalias !3
  %18 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %15, i64 0, i32 0, i32 6, i64 0, i32 1
  %19 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %18, i32 0) #4
  %20 = load i64, i64* %19, align 1, !noalias !3
  %21 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %15, i64 0, i32 0, i32 6, i64 0, i32 2
  %22 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %21, i32 0) #4
  %23 = load i64, i64* %22, align 1, !noalias !3
  %24 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %18, i32 1) #4
  %25 = load i64, i64* %24, align 1, !noalias !3
  %26 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %21, i32 1) #4
  %27 = load i64, i64* %26, align 1, !noalias !3
  %28 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %27, i64 %25, float* elementtype(float) %17, i64 %8) #4
  %29 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %23, i64 %20, float* elementtype(float) %28, i64 %12) #4
  store float %10, float* %29, align 1, !noalias !3
  br label %30

30:                                               ; preds = %30, %11
  %31 = phi i64 [ %54, %30 ], [ 1, %11 ]
  %32 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16, !noalias !3
  %33 = load i64, i64* %3, align 1, !noalias !3
  %34 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %33, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %32, i64 %6) #4
  %35 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %34, i64 0, i32 1, i32 0
  %36 = load float*, float** %35, align 1, !noalias !3
  %37 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %34, i64 0, i32 1, i32 6, i64 0, i32 1
  %38 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %37, i32 0) #4
  %39 = load i64, i64* %38, align 1, !noalias !3
  %40 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %34, i64 0, i32 1, i32 6, i64 0, i32 2
  %41 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %40, i32 0) #4
  %42 = load i64, i64* %41, align 1, !noalias !3
  %43 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %37, i32 1) #4
  %44 = load i64, i64* %43, align 1, !noalias !3
  %45 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %40, i32 1) #4
  %46 = load i64, i64* %45, align 1, !noalias !3
  %47 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %37, i32 2) #4
  %48 = load i64, i64* %47, align 1, !noalias !3
  %49 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %40, i32 2) #4
  %50 = load i64, i64* %49, align 1, !noalias !3
  %51 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 %50, i64 %48, float* elementtype(float) %36, i64 %31) #4
  %52 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %46, i64 %44, float* elementtype(float) %51, i64 %8) #4
  %53 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %42, i64 %39, float* elementtype(float) %52, i64 %12) #4
  store float %10, float* %53, align 1, !noalias !3
  %54 = add nuw nsw i64 %31, 1
  %55 = icmp eq i64 %54, 11
  br i1 %55, label %56, label %30

56:                                               ; preds = %30
  %57 = add nuw nsw i64 %12, 1
  %58 = icmp eq i64 %57, 11
  br i1 %58, label %59, label %11

59:                                               ; preds = %56
  %60 = add nuw nsw i64 %8, 1
  %61 = icmp eq i64 %60, 11
  br i1 %61, label %62, label %7

62:                                               ; preds = %59
  call void @arr_mod_mp_print_arr_(i32* nonnull %1, i32* nonnull @anon.87529b4ebf98830a9107fed24e462e82.1, i32* nonnull @anon.87529b4ebf98830a9107fed24e462e82.1, i32* nonnull @anon.87529b4ebf98830a9107fed24e462e82.1)
  %63 = add nuw nsw i32 %5, 1
  store i32 %63, i32* %1, align 8
  %64 = icmp ult i32 %5, 10
  br i1 %64, label %4, label %65

65:                                               ; preds = %62
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(i32* nocapture readonly) local_unnamed_addr #2

; Function Attrs: nofree nosync nounwind readnone willreturn mustprogress
declare i32 @llvm.ssa.copy.i32(i32 returned) #3

; Function Attrs: nofree nosync nounwind readnone willreturn mustprogress
declare i64 @llvm.ssa.copy.i64(i64 returned) #3

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nofree nosync nounwind readnone speculatable }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { nofree nosync nounwind readnone willreturn mustprogress }
attributes #4 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
!3 = !{!4, !6, !7, !8}
!4 = distinct !{!4, !5, !"arr_mod_mp_initialize_arr_: %I"}
!5 = distinct !{!5, !"arr_mod_mp_initialize_arr_"}
!6 = distinct !{!6, !5, !"arr_mod_mp_initialize_arr_: %N"}
!7 = distinct !{!7, !5, !"arr_mod_mp_initialize_arr_: %M"}
!8 = distinct !{!8, !5, !"arr_mod_mp_initialize_arr_: %O"}
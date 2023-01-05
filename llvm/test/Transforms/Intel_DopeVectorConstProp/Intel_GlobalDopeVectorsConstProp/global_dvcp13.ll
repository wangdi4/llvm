; REQUIRES: asserts
; RUN: opt < %s -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-GLOBDV

; RUN: opt < %s -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-FIELD0

; RUN: opt < %s -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-FIELD1

; RUN: opt < %s -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-FIELD2

; This test case checks that the analysis for the global dope vector
; @arr_mod_mp_a_ didn't pass since it couldn't guarantee that the store
; instruction for a nested dope vector will be executed if the call to alloc
; executes. It was created from the following source code by making a
; modification to the function ALLOCATE_ARR in the IR:

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

; The analysis process should fail because the allocation for inner_array_A is
; wrapped in a conditional. We can't guarantee if the store instructions for
; the dope vector fields will execute if the call to @for_allocate_handle
; executes.

; CHECK-GLOBDV: Global variable: arr_mod_mp_a_
; CHECK-GLOBDV-NEXT:   LLVM Type: QNCA_a0$%"ARR_MOD$.btT_TESTTYPE"*$rank1$
; CHECK-GLOBDV-NEXT:   Global dope vector result: Failed to collect nested dope vectors' data
; CHECK-GLOBDV:        Constant propagation status: NOT performed

; CHECK-FIELD0:    Field[0]: QNCA_a0$float*$rank2$
; CHECK-FIELD0-NEXT:      Dope vector analysis result: Incomplete data collection
; CHECK-FIELD0-NEXT:   Constant propagation status: NOT performed

; CHECK-FIELD1:    Field[1]: QNCA_a0$float*$rank3$
; CHECK-FIELD1-NEXT:      Dope vector analysis result: Incomplete data collection
; CHECK-FIELD1-NEXT:   Constant propagation status: NOT performed

; CHECK-FIELD2:    Field[2]: QNCA_a0$float*$rank1$
; CHECK-FIELD2-NEXT:      Dope vector analysis result: Incomplete data collection
; CHECK-FIELD2-NEXT:   Constant propagation status: NOT performed


; ModuleID = 'ld-temp.o'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$" = type { %"ARR_MOD$.btT_TESTTYPE"*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"ARR_MOD$.btT_TESTTYPE" = type { %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank1$" }
%"QNCA_a0$float*$rank2$" = type { float*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank3$" = type { float*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank1$" = type { float*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@arr_mod_mp_a_ = internal global %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$" { %"ARR_MOD$.btT_TESTTYPE"* null, i64 0, i64 0, i64 1342177408, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@anon.87529b4ebf98830a9107fed24e462e82.0 = internal unnamed_addr constant i32 2
@anon.87529b4ebf98830a9107fed24e462e82.1 = internal unnamed_addr constant i32 10

define internal i32 @arr_alloc_dv(i8** %0) {
  %2 = tail call i32 @for_allocate_handle(i64 400, i8** %0, i32 327680, i8* null) #3
  ret i32 %2
}

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_allocate_arr_(i32* noalias nocapture readonly dereferenceable(4) %0, i32 %temp) #0 {
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
  %14 = call i32 (i64*, i32, ...) @for_check_mult_overflow64(i64* nonnull %2, i32 2, i64 1, i64 288) #3
  %15 = load i64, i64* %2, align 8
  %16 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 3), align 8
  %17 = and i64 %16, -68451041281
  %18 = or i64 %17, 1342177280
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
  %31 = or i32 %30, 327680
  %32 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 5), align 8
  %33 = inttoptr i64 %32 to i8*
  %34 = tail call i32 @for_alloc_allocatable_handle(i64 %15, i8** bitcast (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_ to i8**), i32 %31, i8* %33) #3
  br label %35

35:                                               ; preds = %7, %5
  %36 = phi i64* [ %6, %5 ], [ %11, %7 ]
  %37 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %38 = load i64, i64* %36, align 1
  %39 = sext i32 %3 to i64
  %40 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %38, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %37, i64 %39)
  %41 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0
  %42 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %41, i64 0, i32 3
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
  %49 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %48, i32 0)
  store i64 1, i64* %49, align 1
  %50 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %47, i64 0, i32 0
  %51 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %50, i32 0)
  store i64 10, i64* %51, align 1
  %52 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %48, i32 1)
  store i64 1, i64* %52, align 1
  %53 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %50, i32 1)
  store i64 10, i64* %53, align 1
  %54 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %47, i64 0, i32 1
  %55 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %54, i32 0)
  store i64 4, i64* %55, align 1
  %56 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %54, i32 1)
  store i64 40, i64* %56, align 1
  store i64 1342177285, i64* %42, align 1
  %57 = bitcast %"ARR_MOD$.btT_TESTTYPE"* %40 to i8**

; This conditional prevents to guaranteed that call to @for_allocate_handle
; will execute if the store instructions executed.

  %cmp = icmp eq i32 %temp, 1
  br i1 %cmp, label %if.then, label %if.end

if.then:
  %58 = tail call i32 @arr_alloc_dv(i8** %57)
  br label %if.end

if.end:

  %59 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %60 = load i64, i64* %36, align 1
  %61 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %60, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %59, i64 %39)
  %62 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %61, i64 0, i32 1
  %63 = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %62, i64 0, i32 3
  %64 = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %62, i64 0, i32 5
  store i64 0, i64* %64, align 1
  %65 = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %62, i64 0, i32 1
  store i64 4, i64* %65, align 1
  %66 = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %62, i64 0, i32 4
  store i64 3, i64* %66, align 1
  %67 = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %62, i64 0, i32 2
  store i64 0, i64* %67, align 1
  %68 = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %62, i64 0, i32 6, i64 0
  %69 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %68, i64 0, i32 2
  %70 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %69, i32 0)
  store i64 1, i64* %70, align 1
  %71 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %68, i64 0, i32 0
  %72 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %71, i32 0)
  store i64 10, i64* %72, align 1
  %73 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %69, i32 1)
  store i64 1, i64* %73, align 1
  %74 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %71, i32 1)
  store i64 10, i64* %74, align 1
  %75 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %69, i32 2)
  store i64 1, i64* %75, align 1
  %76 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %71, i32 2)
  store i64 10, i64* %76, align 1
  %77 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %68, i64 0, i32 1
  %78 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %77, i32 0)
  store i64 4, i64* %78, align 1
  %79 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %77, i32 1)
  store i64 40, i64* %79, align 1
  %80 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %77, i32 2)
  store i64 400, i64* %80, align 1
  store i64 1342177285, i64* %63, align 1
  %81 = bitcast %"QNCA_a0$float*$rank3$"* %62 to i8**
  %82 = tail call i32 @for_allocate_handle(i64 4000, i8** nonnull %81, i32 327680, i8* null) #3
  %83 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %84 = load i64, i64* %36, align 1
  %85 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %84, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %83, i64 %39)
  %86 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %85, i64 0, i32 2
  %87 = getelementptr inbounds %"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* %86, i64 0, i32 3
  %88 = getelementptr inbounds %"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* %86, i64 0, i32 5
  store i64 0, i64* %88, align 1
  %89 = getelementptr inbounds %"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* %86, i64 0, i32 1
  store i64 4, i64* %89, align 1
  %90 = getelementptr inbounds %"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* %86, i64 0, i32 4
  store i64 1, i64* %90, align 1
  %91 = getelementptr inbounds %"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* %86, i64 0, i32 2
  store i64 0, i64* %91, align 1
  %92 = getelementptr inbounds %"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* %86, i64 0, i32 6, i64 0
  %93 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %92, i64 0, i32 2
  %94 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %93, i32 0)
  store i64 1, i64* %94, align 1
  %95 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %92, i64 0, i32 0
  %96 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %95, i32 0)
  store i64 10, i64* %96, align 1
  %97 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %92, i64 0, i32 1
  %98 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %97, i32 0)
  store i64 4, i64* %98, align 1
  store i64 1342177285, i64* %87, align 1
  %99 = bitcast %"QNCA_a0$float*$rank1$"* %86 to i8**
  %100 = tail call i32 @for_allocate_handle(i64 40, i8** nonnull %99, i32 327680, i8* null) #3
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

8:                                                ; preds = %64, %4
  %9 = phi i64 [ 1, %4 ], [ %79, %64 ]
  %10 = trunc i64 %9 to i32
  %11 = sitofp i32 %10 to float
  br label %12

12:                                               ; preds = %61, %8
  %13 = phi i64 [ 1, %8 ], [ %62, %61 ]
  %14 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %15 = load i64, i64* %5, align 1
  %16 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %15, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %14, i64 %7)
  %17 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %16, i64 0, i32 0
  %18 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %17, i64 0, i32 0
  %19 = load float*, float** %18, align 1
  %20 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %17, i64 0, i32 6, i64 0
  %21 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %20, i64 0, i32 1
  %22 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %21, i32 0)
  %23 = load i64, i64* %22, align 1
  %24 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %20, i64 0, i32 2
  %25 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %24, i32 0)
  %26 = load i64, i64* %25, align 1
  %27 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %21, i32 1)
  %28 = load i64, i64* %27, align 1
  %29 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %24, i32 1)
  %30 = load i64, i64* %29, align 1
  %31 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %30, i64 %28, float* elementtype(float) %19, i64 %9)
  %32 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %26, i64 %23, float* elementtype(float) %31, i64 %13)
  store float %11, float* %32, align 1
  br label %33

33:                                               ; preds = %12, %33
  %34 = phi i64 [ %59, %33 ], [ 1, %12 ]
  %35 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %36 = load i64, i64* %5, align 1
  %37 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %36, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %35, i64 %7)
  %38 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %37, i64 0, i32 1
  %39 = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %38, i64 0, i32 0
  %40 = load float*, float** %39, align 1
  %41 = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %38, i64 0, i32 6, i64 0
  %42 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %41, i64 0, i32 1
  %43 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %42, i32 0)
  %44 = load i64, i64* %43, align 1
  %45 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %41, i64 0, i32 2
  %46 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %45, i32 0)
  %47 = load i64, i64* %46, align 1
  %48 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %42, i32 1)
  %49 = load i64, i64* %48, align 1
  %50 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %45, i32 1)
  %51 = load i64, i64* %50, align 1
  %52 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %42, i32 2)
  %53 = load i64, i64* %52, align 1
  %54 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %45, i32 2)
  %55 = load i64, i64* %54, align 1
  %56 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 %55, i64 %53, float* elementtype(float) %40, i64 %34)
  %57 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %51, i64 %49, float* elementtype(float) %56, i64 %9)
  %58 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %47, i64 %44, float* elementtype(float) %57, i64 %13)
  store float %11, float* %58, align 1
  %59 = add nuw nsw i64 %34, 1
  %60 = icmp eq i64 %59, 11
  br i1 %60, label %61, label %33

61:                                               ; preds = %33
  %62 = add nuw nsw i64 %13, 1
  %63 = icmp eq i64 %62, 11
  br i1 %63, label %64, label %12

64:                                               ; preds = %61
  %65 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %66 = load i64, i64* %5, align 1
  %67 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %66, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %65, i64 %7)
  %68 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %67, i64 0, i32 2
  %69 = getelementptr inbounds %"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* %68, i64 0, i32 0
  %70 = load float*, float** %69, align 1
  %71 = getelementptr inbounds %"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* %68, i64 0, i32 6, i64 0
  %72 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %71, i64 0, i32 1
  %73 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %72, i32 0)
  %74 = load i64, i64* %73, align 1
  %75 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %71, i64 0, i32 2
  %76 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %75, i32 0)
  %77 = load i64, i64* %76, align 1
  %78 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %77, i64 %74, float* elementtype(float) %70, i64 11)
  store float 1.100000e+01, float* %78, align 1
  %79 = add nuw nsw i64 %9, 1
  %80 = icmp eq i64 %79, 11
  br i1 %80, label %81, label %8

81:                                               ; preds = %64
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #1

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_print_arr_(i32* noalias nocapture readonly dereferenceable(4) %0, i32* noalias nocapture readonly dereferenceable(4) %1, i32* noalias nocapture readonly dereferenceable(4) %2, i32* noalias nocapture readonly dereferenceable(4) %3) #0 {
  %5 = alloca [8 x i64], align 32
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

34:                                               ; preds = %94, %4
  %35 = phi i64 [ 1, %4 ], [ %113, %94 ]
  br label %36

36:                                               ; preds = %34, %90
  %37 = phi i64 [ %91, %90 ], [ 1, %34 ]
  %38 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %39 = load i64, i64* %12, align 1
  %40 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %39, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %38, i64 %14)
  %41 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0
  %42 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %41, i64 0, i32 0
  %43 = load float*, float** %42, align 1
  %44 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %41, i64 0, i32 6, i64 0
  %45 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %44, i64 0, i32 1
  %46 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %45, i32 0)
  %47 = load i64, i64* %46, align 1
  %48 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %44, i64 0, i32 2
  %49 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %48, i32 0)
  %50 = load i64, i64* %49, align 1
  %51 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %45, i32 1)
  %52 = load i64, i64* %51, align 1
  %53 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %48, i32 1)
  %54 = load i64, i64* %53, align 1
  %55 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %54, i64 %52, float* elementtype(float) %43, i64 %35)
  %56 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %50, i64 %47, float* elementtype(float) %55, i64 %37)
  %57 = load float, float* %56, align 1
  store i8 26, i8* %15, align 1
  store i8 1, i8* %16, align 1
  store i8 1, i8* %17, align 1
  store i8 0, i8* %18, align 1
  store float %57, float* %19, align 8
  %58 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %20, i32 -1, i64 1239157112576, i8* nonnull %15, i8* nonnull %21) #3
  br label %59

59:                                               ; preds = %36, %59
  %60 = phi i64 [ %87, %59 ], [ 1, %36 ]
  %61 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %62 = load i64, i64* %12, align 1
  %63 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %62, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %61, i64 %14)
  %64 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %63, i64 0, i32 1
  %65 = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %64, i64 0, i32 0
  %66 = load float*, float** %65, align 1
  %67 = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %64, i64 0, i32 6, i64 0
  %68 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %67, i64 0, i32 1
  %69 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %68, i32 0)
  %70 = load i64, i64* %69, align 1
  %71 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %67, i64 0, i32 2
  %72 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %71, i32 0)
  %73 = load i64, i64* %72, align 1
  %74 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %68, i32 1)
  %75 = load i64, i64* %74, align 1
  %76 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %71, i32 1)
  %77 = load i64, i64* %76, align 1
  %78 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %68, i32 2)
  %79 = load i64, i64* %78, align 1
  %80 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %71, i32 2)
  %81 = load i64, i64* %80, align 1
  %82 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 %81, i64 %79, float* elementtype(float) %66, i64 %60)
  %83 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %77, i64 %75, float* elementtype(float) %82, i64 %35)
  %84 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %73, i64 %70, float* elementtype(float) %83, i64 %37)
  %85 = load float, float* %84, align 1
  store i8 26, i8* %22, align 1
  store i8 1, i8* %23, align 1
  store i8 1, i8* %24, align 1
  store i8 0, i8* %25, align 1
  store float %85, float* %26, align 8
  %86 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %20, i32 -1, i64 1239157112576, i8* nonnull %22, i8* nonnull %27) #3
  %87 = add nuw i64 %60, 1
  %88 = trunc i64 %87 to i32
  %89 = icmp slt i32 10, %88
  br i1 %89, label %90, label %59

90:                                               ; preds = %59
  %91 = add nuw i64 %37, 1
  %92 = trunc i64 %91 to i32
  %93 = icmp slt i32 10, %92
  br i1 %93, label %94, label %36

94:                                               ; preds = %90
  %95 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16
  %96 = load i64, i64* %12, align 1
  %97 = call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %96, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %95, i64 %14)
  %98 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %97, i64 0, i32 2
  %99 = getelementptr inbounds %"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* %98, i64 0, i32 0
  %100 = load float*, float** %99, align 1
  %101 = getelementptr inbounds %"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* %98, i64 0, i32 6, i64 0
  %102 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %101, i64 0, i32 1
  %103 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %102, i32 0)
  %104 = load i64, i64* %103, align 1
  %105 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %101, i64 0, i32 2
  %106 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %105, i32 0)
  %107 = load i64, i64* %106, align 1
  %108 = shl i64 %91, 32
  %109 = ashr exact i64 %108, 32
  %110 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %107, i64 %104, float* elementtype(float) %100, i64 %109)
  %111 = load float, float* %110, align 1
  store i8 26, i8* %28, align 1
  store i8 1, i8* %29, align 1
  store i8 1, i8* %30, align 1
  store i8 0, i8* %31, align 1
  store float %111, float* %32, align 8
  %112 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %20, i32 -1, i64 1239157112576, i8* nonnull %28, i8* nonnull %33) #3
  %113 = add nuw i64 %35, 1
  %114 = trunc i64 %113 to i32
  %115 = icmp slt i32 10, %114
  br i1 %115, label %116, label %34

116:                                              ; preds = %94
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr #2

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #0 {
  %1 = alloca i32, align 8
  %2 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.87529b4ebf98830a9107fed24e462e82.0) #3
  store i32 1, i32* %1, align 8
  br label %3

3:                                                ; preds = %3, %0
  %4 = phi i32 [ %5, %3 ], [ 1, %0 ]
  call void @arr_mod_mp_allocate_arr_(i32* nonnull %1, i32 %2)
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

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
; REQUIRES: asserts
; RUN: opt < %s -opaque-pointers -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-GLOBDV

; RUN: opt < %s -opaque-pointers -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-FIELD0

; RUN: opt < %s -opaque-pointers -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-FIELD1

; RUN: opt < %s -opaque-pointers -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-FIELD2

; This test case checks that the information for the global dope vector
; and the nested dope vectors was collected and propagated correctly even
; if the nested dope vectors are parameters to a function. It was created
; from the following source code:

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
;           INTEGER, INTENT(in) :: I, N, M, O
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
;         SUBROUTINE PRINT_INFO(I, N, M, O, ARRAY_A, ARRAY_B, ARRAY_C)
;           INTEGER, INTENT(IN) :: I, N, M, O
;           REAL, INTENT(IN) :: ARRAY_A(:,:), ARRAY_B(:,:,:), ARRAY_C(:)
;
;           DO j = 1, N
;             DO k = 1, M
;               print *, ARRAY_A(k, j)
;               DO l = 1, O
;                 print *, ARRAY_B(k, j, l)
;               END DO
;             END DO
;             print *, ARRAY_C(k)
;           END DO
;           RETURN
;         END SUBROUTINE
;
;         SUBROUTINE PRINT_ARR(I, N, M, O)
;           INTEGER, INTENT(IN) :: I, N, M, O
;
;           CALL PRINT_INFO(I, N, M, O, A(I) % inner_array_A, &
;              & A(I) % inner_array_B, A(I) % inner_array_C)
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
;
;      END

; ifx -xCORE-AVX512 -Ofast -flto arr.f90 -mllvm -debug-only=dope-vector-global-const-prop

; The global array A is allocated in ALLOCATE_ARR and the initialized in
; INITIALIZE_ARR. Function PRINT_ARR will pass the fields of T_TESTTYPE to
; PRINT_INFO.

; CHECK-GLOBDV: Global variable: arr_mod_mp_a_
; CHECK-GLOBDV-NEXT:   LLVM Type: QNCA_a0$%"ARR_MOD$.btT_TESTTYPE"*$rank1$
; CHECK-GLOBDV-NEXT:   Global dope vector result: Pass
; CHECK-GLOBDV-NEXT:   Dope vector analysis result: Pass
; CHECK-GLOBDV-NEXT:   Constant propagation status: performed
; CHECK-GLOBDV-NEXT:     [0] Array Pointer: Read
; CHECK-GLOBDV-NEXT:     [1] Element size: Written | Constant = i64 288
; CHECK-GLOBDV-NEXT:     [2] Co-Dimension: Written | Constant = i64 0
; CHECK-GLOBDV-NEXT:     [3] Flags: Read | Written
; CHECK-GLOBDV-NEXT:     [4] Dimensions: Written | Constant = i64 1
; CHECK-GLOBDV-NEXT:     [6][0] Extent: Written | Constant = i64 1
; CHECK-GLOBDV-NEXT:     [6][0] Stride: Written | Constant = i64 288
; CHECK-GLOBDV-NEXT:     [6][0] Lower Bound: Read | Written | Constant = i64 1
; CHECK-GLOBDV-NEXT:   Nested dope vectors: 3

; CHECK-FIELD0:    Field[0]: QNCA_a0$float*$rank2$
; CHECK-FIELD0-NEXT:      Dope vector analysis result: Pass
; CHECK-FIELD0-NEXT:      Constant propagation status: performed
; CHECK-FIELD0-NEXT:        [0] Array Pointer: Read
; CHECK-FIELD0-NEXT:        [1] Element size: Written | Constant = i64 4
; CHECK-FIELD0-NEXT:        [2] Co-Dimension: Written | Constant = i64 0
; CHECK-FIELD0-NEXT:        [3] Flags: Written
; CHECK-FIELD0-NEXT:        [4] Dimensions: Written | Constant = i64 2
; CHECK-FIELD0-NEXT:        [6][0] Extent: Written | Constant = i64 10
; CHECK-FIELD0-NEXT:        [6][0] Stride: Read | Written | Constant = i64 4
; CHECK-FIELD0-NEXT:        [6][0] Lower Bound: Read | Written | Constant = i64 1
; CHECK-FIELD0-NEXT:        [6][1] Extent: Written | Constant = i64 10
; CHECK-FIELD0-NEXT:        [6][1] Stride: Read | Written | Constant = i64 40
; CHECK-FIELD0-NEXT:        [6][1] Lower Bound: Read | Written | Constant = i64 1

; CHECK-FIELD2:    Field[2]: QNCA_a0$float*$rank1$
; CHECK-FIELD2-NEXT:      Dope vector analysis result: Pass
; CHECK-FIELD2-NEXT:      Constant propagation status: performed
; CHECK-FIELD2-NEXT:        [0] Array Pointer: Read
; CHECK-FIELD2-NEXT:        [1] Element size: Written | Constant = i64 4
; CHECK-FIELD2-NEXT:        [2] Co-Dimension: Written | Constant = i64 0
; CHECK-FIELD2-NEXT:        [3] Flags: Written
; CHECK-FIELD2-NEXT:        [4] Dimensions: Written | Constant = i64 1
; CHECK-FIELD2-NEXT:        [6][0] Extent: Written | Constant = i64 10
; CHECK-FIELD2-NEXT:        [6][0] Stride: Read | Written | Constant = i64 4
; CHECK-FIELD2-NEXT:        [6][0] Lower Bound: Read | Written | Constant = i64 1

; CHECK-FIELD1:    Field[1]: QNCA_a0$float*$rank3$
; CHECK-FIELD1-NEXT:      Dope vector analysis result: Pass
; CHECK-FIELD1-NEXT:      Constant propagation status: performed
; CHECK-FIELD1-NEXT:        [0] Array Pointer: Read
; CHECK-FIELD1-NEXT:        [1] Element size: Written | Constant = i64 4
; CHECK-FIELD1-NEXT:        [2] Co-Dimension: Written | Constant = i64 0
; CHECK-FIELD1-NEXT:        [3] Flags: Written
; CHECK-FIELD1-NEXT:        [4] Dimensions: Written | Constant = i64 3
; CHECK-FIELD1-NEXT:        [6][0] Extent: Written | Constant = i64 10
; CHECK-FIELD1-NEXT:        [6][0] Stride: Read | Written | Constant = i64 4
; CHECK-FIELD1-NEXT:        [6][0] Lower Bound: Read | Written | Constant = i64 1
; CHECK-FIELD1-NEXT:        [6][1] Extent: Written | Constant = i64 10
; CHECK-FIELD1-NEXT:        [6][1] Stride: Read | Written | Constant = i64 40
; CHECK-FIELD1-NEXT:        [6][1] Lower Bound: Read | Written | Constant = i64 1
; CHECK-FIELD1-NEXT:        [6][2] Extent: Written | Constant = i64 10
; CHECK-FIELD1-NEXT:        [6][2] Stride: Read | Written | Constant = i64 400
; CHECK-FIELD1-NEXT:        [6][2] Lower Bound: Read | Written | Constant = i64 1


; ModuleID = 'ld-temp.o'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"ARR_MOD$.btT_TESTTYPE" = type { %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank1$" }
%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank3$" = type { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@arr_mod_mp_a_ = internal global %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$" { ptr null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@anon.11934500c7ed222a0a148892d04ea3ac.0 = internal unnamed_addr constant i32 2
@anon.11934500c7ed222a0a148892d04ea3ac.1 = internal unnamed_addr constant i32 10

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_allocate_arr_(ptr noalias nocapture readonly dereferenceable(4) %0) #0 {
  %2 = alloca i64, align 8
  %3 = load i32, ptr %0, align 1
  %4 = icmp eq i32 %3, 1
  br i1 %4, label %7, label %5

5:                                                ; preds = %1
  %6 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  br label %35

7:                                                ; preds = %1
  %8 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
  %9 = and i64 %8, 1030792151296
  %10 = or i64 %9, 133
  store i64 %10, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 5), align 8
  store i64 288, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 1), align 8
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 4), align 16
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 2), align 16
  %11 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %11, align 1
  %12 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 1, ptr %12, align 1
  %13 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 288, ptr %13, align 1
  %14 = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %2, i32 2, i64 1, i64 288) #5
  %15 = load i64, ptr %2, align 8
  %16 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
  %17 = and i64 %16, -68451041281
  %18 = or i64 %17, 1073741824
  store i64 %18, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
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
  %32 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 5), align 8
  %33 = inttoptr i64 %32 to ptr
  %34 = tail call i32 @for_alloc_allocatable_handle(i64 %15, ptr @arr_mod_mp_a_, i32 %31, ptr %33) #5
  br label %35

35:                                               ; preds = %7, %5
  %36 = phi ptr [ %6, %5 ], [ %11, %7 ]
  %37 = load ptr, ptr @arr_mod_mp_a_, align 16
  %38 = load i64, ptr %36, align 1
  %39 = sext i32 %3 to i64
  %40 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %38, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %37, i64 %39)
  %41 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %40, i64 0, i32 0, i32 3
  %42 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %40, i64 0, i32 0, i32 5
  store i64 0, ptr %42, align 1
  %43 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %40, i64 0, i32 0, i32 1
  store i64 4, ptr %43, align 1
  %44 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %40, i64 0, i32 0, i32 4
  store i64 2, ptr %44, align 1
  %45 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %40, i64 0, i32 0, i32 2
  store i64 0, ptr %45, align 1
  %46 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %40, i64 0, i32 0, i32 6, i64 0, i32 2
  %47 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %46, i32 0)
  store i64 1, ptr %47, align 1
  %48 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %40, i64 0, i32 0, i32 6, i64 0, i32 0
  %49 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %48, i32 0)
  store i64 10, ptr %49, align 1
  %50 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %46, i32 1)
  store i64 1, ptr %50, align 1
  %51 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %48, i32 1)
  store i64 10, ptr %51, align 1
  %52 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %40, i64 0, i32 0, i32 6, i64 0, i32 1
  %53 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %52, i32 0)
  store i64 4, ptr %53, align 1
  %54 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %52, i32 1)
  store i64 40, ptr %54, align 1
  store i64 1073741829, ptr %41, align 1
  %55 = bitcast ptr %40 to ptr
  %56 = tail call i32 @for_allocate_handle(i64 400, ptr %55, i32 262144, ptr null) #5
  %57 = load ptr, ptr @arr_mod_mp_a_, align 16
  %58 = load i64, ptr %36, align 1
  %59 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %58, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %57, i64 %39)
  %60 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %59, i64 0, i32 1, i32 3
  %61 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %59, i64 0, i32 1, i32 5
  store i64 0, ptr %61, align 1
  %62 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %59, i64 0, i32 1, i32 1
  store i64 4, ptr %62, align 1
  %63 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %59, i64 0, i32 1, i32 4
  store i64 3, ptr %63, align 1
  %64 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %59, i64 0, i32 1, i32 2
  store i64 0, ptr %64, align 1
  %65 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %59, i64 0, i32 1, i32 6, i64 0, i32 2
  %66 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %65, i32 0)
  store i64 1, ptr %66, align 1
  %67 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %59, i64 0, i32 1, i32 6, i64 0, i32 0
  %68 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %67, i32 0)
  store i64 10, ptr %68, align 1
  %69 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %65, i32 1)
  store i64 1, ptr %69, align 1
  %70 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %67, i32 1)
  store i64 10, ptr %70, align 1
  %71 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %65, i32 2)
  store i64 1, ptr %71, align 1
  %72 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %67, i32 2)
  store i64 10, ptr %72, align 1
  %73 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %59, i64 0, i32 1, i32 6, i64 0, i32 1
  %74 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %73, i32 0)
  store i64 4, ptr %74, align 1
  %75 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %73, i32 1)
  store i64 40, ptr %75, align 1
  %76 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %73, i32 2)
  store i64 400, ptr %76, align 1
  %77 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %59, i64 0, i32 1, i32 0
  store i64 1073741829, ptr %60, align 1
  %78 = bitcast ptr %77 to ptr
  %79 = tail call i32 @for_allocate_handle(i64 4000, ptr nonnull %78, i32 262144, ptr null) #5
  %80 = load ptr, ptr @arr_mod_mp_a_, align 16
  %81 = load i64, ptr %36, align 1
  %82 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %81, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %80, i64 %39)
  %83 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %82, i64 0, i32 2, i32 3
  %84 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %82, i64 0, i32 2, i32 5
  store i64 0, ptr %84, align 1
  %85 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %82, i64 0, i32 2, i32 1
  store i64 4, ptr %85, align 1
  %86 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %82, i64 0, i32 2, i32 4
  store i64 1, ptr %86, align 1
  %87 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %82, i64 0, i32 2, i32 2
  store i64 0, ptr %87, align 1
  %88 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %82, i64 0, i32 2, i32 6, i64 0, i32 2
  %89 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %88, i32 0)
  store i64 1, ptr %89, align 1
  %90 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %82, i64 0, i32 2, i32 6, i64 0, i32 0
  %91 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %90, i32 0)
  store i64 10, ptr %91, align 1
  %92 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %82, i64 0, i32 2, i32 6, i64 0, i32 1
  %93 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %92, i32 0)
  store i64 4, ptr %93, align 1
  %94 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %82, i64 0, i32 2, i32 0
  store i64 1073741829, ptr %83, align 1
  %95 = bitcast ptr %94 to ptr
  %96 = tail call i32 @for_allocate_handle(i64 40, ptr nonnull %95, i32 262144, ptr null) #5
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_check_mult_overflow64(ptr nocapture, i32, ...) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_alloc_allocatable_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: nofree nosync nounwind uwtable
define internal void @arr_mod_mp_initialize_arr_(ptr noalias nocapture readonly dereferenceable(4) %0, ptr noalias nocapture readonly dereferenceable(4) %1, ptr noalias nocapture readonly dereferenceable(4) %2, ptr noalias nocapture readonly dereferenceable(4) %3) #2 {
  %5 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %6 = load i32, ptr %0, align 1
  %7 = sext i32 %6 to i64
  br label %8

8:                                                ; preds = %60, %4
  %9 = phi i64 [ 1, %4 ], [ %73, %60 ]
  %10 = trunc i64 %9 to i32
  %11 = sitofp i32 %10 to float
  br label %12

12:                                               ; preds = %57, %8
  %13 = phi i64 [ 1, %8 ], [ %58, %57 ]
  %14 = load ptr, ptr @arr_mod_mp_a_, align 16
  %15 = load i64, ptr %5, align 1
  %16 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %15, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %14, i64 %7)
  %17 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %16, i64 0, i32 0, i32 0
  %18 = load ptr, ptr %17, align 1
  %19 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %16, i64 0, i32 0, i32 6, i64 0, i32 1
  %20 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %19, i32 0)
  %21 = load i64, ptr %20, align 1
  %22 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %16, i64 0, i32 0, i32 6, i64 0, i32 2
  %23 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %22, i32 0)
  %24 = load i64, ptr %23, align 1
  %25 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %19, i32 1)
  %26 = load i64, ptr %25, align 1
  %27 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %22, i32 1)
  %28 = load i64, ptr %27, align 1
  %29 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %28, i64 %26, ptr elementtype(float) %18, i64 %9)
  %30 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %24, i64 %21, ptr elementtype(float) %29, i64 %13)
  store float %11, ptr %30, align 1
  br label %31

31:                                               ; preds = %31, %12
  %32 = phi i64 [ %55, %31 ], [ 1, %12 ]
  %33 = load ptr, ptr @arr_mod_mp_a_, align 16
  %34 = load i64, ptr %5, align 1
  %35 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %34, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %33, i64 %7)
  %36 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %35, i64 0, i32 1, i32 0
  %37 = load ptr, ptr %36, align 1
  %38 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %35, i64 0, i32 1, i32 6, i64 0, i32 1
  %39 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %38, i32 0)
  %40 = load i64, ptr %39, align 1
  %41 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %35, i64 0, i32 1, i32 6, i64 0, i32 2
  %42 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %41, i32 0)
  %43 = load i64, ptr %42, align 1
  %44 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %38, i32 1)
  %45 = load i64, ptr %44, align 1
  %46 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %41, i32 1)
  %47 = load i64, ptr %46, align 1
  %48 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %38, i32 2)
  %49 = load i64, ptr %48, align 1
  %50 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %41, i32 2)
  %51 = load i64, ptr %50, align 1
  %52 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %51, i64 %49, ptr elementtype(float) %37, i64 %32)
  %53 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %47, i64 %45, ptr elementtype(float) %52, i64 %9)
  %54 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %43, i64 %40, ptr elementtype(float) %53, i64 %13)
  store float %11, ptr %54, align 1
  %55 = add nuw nsw i64 %32, 1
  %56 = icmp eq i64 %55, 11
  br i1 %56, label %57, label %31

57:                                               ; preds = %31
  %58 = add nuw nsw i64 %13, 1
  %59 = icmp eq i64 %58, 11
  br i1 %59, label %60, label %12

60:                                               ; preds = %57
  %61 = load ptr, ptr @arr_mod_mp_a_, align 16
  %62 = load i64, ptr %5, align 1
  %63 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %62, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %61, i64 %7)
  %64 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %63, i64 0, i32 2, i32 0
  %65 = load ptr, ptr %64, align 1
  %66 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %63, i64 0, i32 2, i32 6, i64 0, i32 1
  %67 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %66, i32 0)
  %68 = load i64, ptr %67, align 1
  %69 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %63, i64 0, i32 2, i32 6, i64 0, i32 2
  %70 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %69, i32 0)
  %71 = load i64, ptr %70, align 1
  %72 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %71, i64 %68, ptr elementtype(float) %65, i64 11)
  store float 1.100000e+01, ptr %72, align 1
  %73 = add nuw nsw i64 %9, 1
  %74 = icmp eq i64 %73, 11
  br i1 %74, label %75, label %8

75:                                               ; preds = %60
  ret void
}

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_print_info_(ptr noalias nocapture readonly %0, ptr noalias nocapture readonly dereferenceable(4) %1, ptr noalias nocapture readonly dereferenceable(4) %2, ptr noalias nocapture readonly dereferenceable(4) %3, ptr noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %4, ptr noalias nocapture readonly dereferenceable(120) "assumed_shape" "ptrnoalias" %5, ptr noalias nocapture readonly dereferenceable(72) "assumed_shape" "ptrnoalias" %6) #0 {
  %8 = alloca [8 x i64], align 16
  %9 = alloca [4 x i8], align 1
  %10 = alloca { float }, align 8
  %11 = alloca [4 x i8], align 1
  %12 = alloca { float }, align 8
  %13 = alloca [4 x i8], align 1
  %14 = alloca { float }, align 8
  %15 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %4, i64 0, i32 0
  %16 = load ptr, ptr %15, align 1
  %17 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %4, i64 0, i32 6, i64 0, i32 1
  %18 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %17, i32 0)
  %19 = load i64, ptr %18, align 1
  %20 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %17, i32 1)
  %21 = load i64, ptr %20, align 1
  %22 = getelementptr inbounds [4 x i8], ptr %9, i64 0, i64 0
  %23 = getelementptr inbounds [4 x i8], ptr %9, i64 0, i64 1
  %24 = getelementptr inbounds [4 x i8], ptr %9, i64 0, i64 2
  %25 = getelementptr inbounds [4 x i8], ptr %9, i64 0, i64 3
  %26 = getelementptr inbounds { float }, ptr %10, i64 0, i32 0
  %27 = bitcast ptr %8 to ptr
  %28 = bitcast ptr %10 to ptr
  %29 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %5, i64 0, i32 0
  %30 = load ptr, ptr %29, align 1
  %31 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %5, i64 0, i32 6, i64 0, i32 1
  %32 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %31, i32 0)
  %33 = load i64, ptr %32, align 1
  %34 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %31, i32 1)
  %35 = load i64, ptr %34, align 1
  %36 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %31, i32 2)
  %37 = load i64, ptr %36, align 1
  %38 = getelementptr inbounds [4 x i8], ptr %11, i64 0, i64 0
  %39 = getelementptr inbounds [4 x i8], ptr %11, i64 0, i64 1
  %40 = getelementptr inbounds [4 x i8], ptr %11, i64 0, i64 2
  %41 = getelementptr inbounds [4 x i8], ptr %11, i64 0, i64 3
  %42 = getelementptr inbounds { float }, ptr %12, i64 0, i32 0
  %43 = bitcast ptr %12 to ptr
  %44 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %6, i64 0, i32 0
  %45 = load ptr, ptr %44, align 1
  %46 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %6, i64 0, i32 6, i64 0, i32 1
  %47 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %46, i32 0)
  %48 = load i64, ptr %47, align 1
  %49 = getelementptr inbounds [4 x i8], ptr %13, i64 0, i64 0
  %50 = getelementptr inbounds [4 x i8], ptr %13, i64 0, i64 1
  %51 = getelementptr inbounds [4 x i8], ptr %13, i64 0, i64 2
  %52 = getelementptr inbounds [4 x i8], ptr %13, i64 0, i64 3
  %53 = getelementptr inbounds { float }, ptr %14, i64 0, i32 0
  %54 = bitcast ptr %14 to ptr
  br label %55

55:                                               ; preds = %77, %7
  %56 = phi i64 [ 1, %7 ], [ %83, %77 ]
  %57 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %21, ptr elementtype(float) %16, i64 %56)
  br label %58

58:                                               ; preds = %73, %55
  %59 = phi i64 [ 1, %55 ], [ %74, %73 ]
  %60 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %19, ptr elementtype(float) %57, i64 %59)
  %61 = load float, ptr %60, align 1
  store i8 26, ptr %22, align 1
  store i8 1, ptr %23, align 1
  store i8 1, ptr %24, align 1
  store i8 0, ptr %25, align 1
  store float %61, ptr %26, align 8
  %62 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %27, i32 -1, i64 1239157112576, ptr nonnull %22, ptr nonnull %28) #5
  br label %63

63:                                               ; preds = %63, %58
  %64 = phi i64 [ %70, %63 ], [ 1, %58 ]
  %65 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %37, ptr elementtype(float) %30, i64 %64)
  %66 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %35, ptr elementtype(float) %65, i64 %56)
  %67 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %33, ptr elementtype(float) %66, i64 %59)
  %68 = load float, ptr %67, align 1
  store i8 26, ptr %38, align 1
  store i8 1, ptr %39, align 1
  store i8 1, ptr %40, align 1
  store i8 0, ptr %41, align 1
  store float %68, ptr %42, align 8
  %69 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %27, i32 -1, i64 1239157112576, ptr nonnull %38, ptr nonnull %43) #5
  %70 = add nuw i64 %64, 1
  %71 = trunc i64 %70 to i32
  %72 = icmp slt i32 10, %71
  br i1 %72, label %73, label %63

73:                                               ; preds = %63
  %74 = add nuw i64 %59, 1
  %75 = trunc i64 %74 to i32
  %76 = icmp slt i32 10, %75
  br i1 %76, label %77, label %58

77:                                               ; preds = %73
  %78 = shl i64 %74, 32
  %79 = ashr exact i64 %78, 32
  %80 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %48, ptr elementtype(float) %45, i64 %79)
  %81 = load float, ptr %80, align 1
  store i8 26, ptr %49, align 1
  store i8 1, ptr %50, align 1
  store i8 1, ptr %51, align 1
  store i8 0, ptr %52, align 1
  store float %81, ptr %53, align 8
  %82 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %27, i32 -1, i64 1239157112576, ptr nonnull %49, ptr nonnull %54) #5
  %83 = add nuw i64 %56, 1
  %84 = trunc i64 %83 to i32
  %85 = icmp slt i32 10, %84
  br i1 %85, label %86, label %55

86:                                               ; preds = %77
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr #1

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #0 {
  %1 = alloca i32, align 8
  %2 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.0) #5
  store i32 1, ptr %1, align 8
  %3 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0) #5
  br label %4

4:                                                ; preds = %4, %0
  %5 = phi i32 [ %13, %4 ], [ 1, %0 ]
  call void @arr_mod_mp_allocate_arr_(ptr nonnull %1)
  call void @arr_mod_mp_initialize_arr_(ptr nonnull %1, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1)
  %6 = load ptr, ptr @arr_mod_mp_a_, align 16, !noalias !3
  %7 = load i64, ptr %3, align 1, !noalias !3
  %8 = zext i32 %5 to i64
  %9 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %7, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %6, i64 %8) #5
  %10 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %9, i64 0, i32 1
  %11 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %9, i64 0, i32 2
  %12 = bitcast ptr %9 to ptr
  tail call void @arr_mod_mp_print_info_(ptr undef, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, ptr %12, ptr nonnull %10, ptr nonnull %11) #5, !noalias !9
  %13 = add nuw nsw i32 %5, 1
  store i32 %13, ptr %1, align 8
  %14 = icmp ult i32 %5, 10
  br i1 %14, label %4, label %15

15:                                               ; preds = %4
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: nocallback nofree nosync nounwind readnone willreturn
declare i32 @llvm.ssa.copy.i32(i32 returned) #3

; Function Attrs: nocallback nofree nosync nounwind readnone willreturn
declare i64 @llvm.ssa.copy.i64(i64 returned) #3

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #4

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #4

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+mmx,+pclmul,+pku,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nofree nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+mmx,+pclmul,+pku,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { nocallback nofree nosync nounwind readnone willreturn }
attributes #4 = { nounwind readnone speculatable }
attributes #5 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
!3 = !{!4, !6, !7, !8}
!4 = distinct !{!4, !5, !"arr_mod_mp_print_arr_: %I"}
!5 = distinct !{!5, !"arr_mod_mp_print_arr_"}
!6 = distinct !{!6, !5, !"arr_mod_mp_print_arr_: %N"}
!7 = distinct !{!7, !5, !"arr_mod_mp_print_arr_: %M"}
!8 = distinct !{!8, !5, !"arr_mod_mp_print_arr_: %O"}
!9 = !{!4}

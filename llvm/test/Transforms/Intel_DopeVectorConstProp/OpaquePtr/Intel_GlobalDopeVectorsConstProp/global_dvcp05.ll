; REQUIRES: asserts
; Check for global dope vector A
; RUN: opt < %s -opaque-pointers -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-GLOBDV-A

; RUN: opt < %s -opaque-pointers -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-FIELD0-A

; RUN: opt < %s -opaque-pointers -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-FIELD1-A

; RUN: opt < %s -opaque-pointers -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-FIELD2-A

; Check for global dope vector B
; RUN: opt < %s -opaque-pointers -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-GLOBDV-B

; RUN: opt < %s -opaque-pointers -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-FIELD0-B

; RUN: opt < %s -opaque-pointers -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-FIELD1-B

; RUN: opt < %s -opaque-pointers -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-FIELD2-B

; This test case checks that the fields for multiple global dope vectors
; were collected and propagated correctly. Also, it identifies and collects
; the nested dope vectors. It was created from the following source code:

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

; CHECK-GLOBDV-A: Global variable: arr_mod_mp_a_
; CHECK-GLOBDV-A-NEXT:  LLVM Type: QNCA_a0$%"ARR_MOD$.btT_TESTTYPE"*$rank1$
; CHECK-GLOBDV-A-NEXT:  Global dope vector result: Pass
; CHECK-GLOBDV-A-NEXT:  Dope vector analysis result: Pass
; CHECK-GLOBDV-A-NEXT:  Constant propagation status: performed
; CHECK-GLOBDV-A-NEXT:    [0] Array Pointer: Read
; CHECK-GLOBDV-A-NEXT:    [1] Element size: Written | Constant = i64 288
; CHECK-GLOBDV-A-NEXT:    [2] Co-Dimension: Written | Constant = i64 0
; CHECK-GLOBDV-A-NEXT:    [3] Flags: Read | Written
; CHECK-GLOBDV-A-NEXT:    [4] Dimensions: Written | Constant = i64 1
; CHECK-GLOBDV-A-NEXT:    [6][0] Extent: Written | Constant = i64 1
; CHECK-GLOBDV-A-NEXT:    [6][0] Stride: Written | Constant = i64 288
; CHECK-GLOBDV-A-NEXT:    [6][0] Lower Bound: Read | Written | Constant = i64 1
; CHECK-GLOBDV-A-NEXT:  Nested dope vectors: 3

; CHECK-FIELD2-A:    Field[2]: QNCA_a0$float*$rank1$
; CHECK-FIELD2-A-NEXT:      Dope vector analysis result: Pass
; CHECK-FIELD2-A-NEXT:      Constant propagation status: performed
; CHECK-FIELD2-A-NEXT:        [0] Array Pointer: Read
; CHECK-FIELD2-A-NEXT:        [1] Element size: Written | Constant = i64 4
; CHECK-FIELD2-A-NEXT:        [2] Co-Dimension: Written | Constant = i64 0
; CHECK-FIELD2-A-NEXT:        [3] Flags: Written
; CHECK-FIELD2-A-NEXT:        [4] Dimensions: Written | Constant = i64 1
; CHECK-FIELD2-A-NEXT:        [6][0] Extent: Written | Constant = i64 10
; CHECK-FIELD2-A-NEXT:        [6][0] Stride: Read | Written | Constant = i64 4
; CHECK-FIELD2-A-NEXT:        [6][0] Lower Bound: Read | Written | Constant = i64 1

; CHECK-FIELD0-A:    Field[0]: QNCA_a0$float*$rank2$
; CHECK-FIELD0-A-NEXT:      Dope vector analysis result: Pass
; CHECK-FIELD0-A-NEXT:      Constant propagation status: performed
; CHECK-FIELD0-A-NEXT:        [0] Array Pointer: Read
; CHECK-FIELD0-A-NEXT:        [1] Element size: Written | Constant = i64 4
; CHECK-FIELD0-A-NEXT:        [2] Co-Dimension: Written | Constant = i64 0
; CHECK-FIELD0-A-NEXT:        [3] Flags: Written
; CHECK-FIELD0-A-NEXT:        [4] Dimensions: Written | Constant = i64 2
; CHECK-FIELD0-A-NEXT:        [6][0] Extent: Written | Constant = i64 10
; CHECK-FIELD0-A-NEXT:        [6][0] Stride: Read | Written | Constant = i64 4
; CHECK-FIELD0-A-NEXT:        [6][0] Lower Bound: Read | Written | Constant = i64 1
; CHECK-FIELD0-A-NEXT:        [6][1] Extent: Written | Constant = i64 10
; CHECK-FIELD0-A-NEXT:        [6][1] Stride: Read | Written | Constant = i64 40
; CHECK-FIELD0-A-NEXT:        [6][1] Lower Bound: Read | Written | Constant = i64 1

; CHECK-FIELD1-A:    Field[1]: QNCA_a0$float*$rank3$
; CHECK-FIELD1-A-NEXT:      Dope vector analysis result: Pass
; CHECK-FIELD1-A-NEXT:      Constant propagation status: performed
; CHECK-FIELD1-A-NEXT:        [0] Array Pointer: Read
; CHECK-FIELD1-A-NEXT:        [1] Element size: Written | Constant = i64 4
; CHECK-FIELD1-A-NEXT:        [2] Co-Dimension: Written | Constant = i64 0
; CHECK-FIELD1-A-NEXT:        [3] Flags: Written
; CHECK-FIELD1-A-NEXT:        [4] Dimensions: Written | Constant = i64 3
; CHECK-FIELD1-A-NEXT:        [6][0] Extent: Written | Constant = i64 10
; CHECK-FIELD1-A-NEXT:        [6][0] Stride: Read | Written | Constant = i64 4
; CHECK-FIELD1-A-NEXT:        [6][0] Lower Bound: Read | Written | Constant = i64 1
; CHECK-FIELD1-A-NEXT:        [6][1] Extent: Written | Constant = i64 10
; CHECK-FIELD1-A-NEXT:        [6][1] Stride: Read | Written | Constant = i64 40
; CHECK-FIELD1-A-NEXT:        [6][1] Lower Bound: Read | Written | Constant = i64 1
; CHECK-FIELD1-A-NEXT:        [6][2] Extent: Written | Constant = i64 10
; CHECK-FIELD1-A-NEXT:        [6][2] Stride: Read | Written | Constant = i64 400
; CHECK-FIELD1-A-NEXT:        [6][2] Lower Bound: Read | Written | Constant = i64 1

; CHECK-GLOBDV-B: Global variable: arr_mod_mp_b_
; CHECK-GLOBDV-B-NEXT:  LLVM Type: QNCA_a0$%"ARR_MOD$.btT_TESTTYPE"*$rank1$
; CHECK-GLOBDV-B-NEXT:  Global dope vector result: Pass
; CHECK-GLOBDV-B-NEXT:  Dope vector analysis result: Pass
; CHECK-GLOBDV-B-NEXT:  Constant propagation status: performed
; CHECK-GLOBDV-B-NEXT:    [0] Array Pointer: Read
; CHECK-GLOBDV-B-NEXT:    [1] Element size: Written | Constant = i64 288
; CHECK-GLOBDV-B-NEXT:    [2] Co-Dimension: Written | Constant = i64 0
; CHECK-GLOBDV-B-NEXT:    [3] Flags: Read | Written
; CHECK-GLOBDV-B-NEXT:    [4] Dimensions: Written | Constant = i64 1
; CHECK-GLOBDV-B-NEXT:    [6][0] Extent: Written
; CHECK-GLOBDV-B-NEXT:    [6][0] Stride: Written | Constant = i64 288
; CHECK-GLOBDV-B-NEXT:    [6][0] Lower Bound: Read | Written | Constant = i64 1
; CHECK-GLOBDV-B-NEXT:  Nested dope vectors: 3

; CHECK-FIELD2-B:    Field[2]: QNCA_a0$float*$rank1$
; CHECK-FIELD2-B-NEXT:      Dope vector analysis result: Pass
; CHECK-FIELD2-B-NEXT:      Constant propagation status: performed
; CHECK-FIELD2-B-NEXT:        [0] Array Pointer: Read
; CHECK-FIELD2-B-NEXT:        [1] Element size: Written | Constant = i64 4
; CHECK-FIELD2-B-NEXT:        [2] Co-Dimension: Written | Constant = i64 0
; CHECK-FIELD2-B-NEXT:        [3] Flags: Written
; CHECK-FIELD2-B-NEXT:        [4] Dimensions: Written | Constant = i64 1
; CHECK-FIELD2-B-NEXT:        [6][0] Extent: Written | Constant = i64 10
; CHECK-FIELD2-B-NEXT:        [6][0] Stride: Read | Written | Constant = i64 4
; CHECK-FIELD2-B-NEXT:        [6][0] Lower Bound: Read | Written | Constant = i64 1

; CHECK-FIELD1-B:    Field[1]: QNCA_a0$float*$rank3$
; CHECK-FIELD1-B-NEXT:      Dope vector analysis result: Pass
; CHECK-FIELD1-B-NEXT:      Constant propagation status: performed
; CHECK-FIELD1-B-NEXT:        [0] Array Pointer: Read
; CHECK-FIELD1-B-NEXT:        [1] Element size: Written | Constant = i64 4
; CHECK-FIELD1-B-NEXT:        [2] Co-Dimension: Written | Constant = i64 0
; CHECK-FIELD1-B-NEXT:        [3] Flags: Written
; CHECK-FIELD1-B-NEXT:        [4] Dimensions: Written | Constant = i64 3
; CHECK-FIELD1-B-NEXT:        [6][0] Extent: Written | Constant = i64 10
; CHECK-FIELD1-B-NEXT:        [6][0] Stride: Read | Written | Constant = i64 4
; CHECK-FIELD1-B-NEXT:        [6][0] Lower Bound: Read | Written | Constant = i64 1
; CHECK-FIELD1-B-NEXT:        [6][1] Extent: Written | Constant = i64 10
; CHECK-FIELD1-B-NEXT:        [6][1] Stride: Read | Written | Constant = i64 40
; CHECK-FIELD1-B-NEXT:        [6][1] Lower Bound: Read | Written | Constant = i64 1
; CHECK-FIELD1-B-NEXT:        [6][2] Extent: Written | Constant = i64 10
; CHECK-FIELD1-B-NEXT:        [6][2] Stride: Read | Written | Constant = i64 400
; CHECK-FIELD1-B-NEXT:        [6][2] Lower Bound: Read | Written | Constant = i64 1

; CHECK-FIELD0-B:    Field[0]: QNCA_a0$float*$rank2$
; CHECK-FIELD0-B-NEXT:      Dope vector analysis result: Pass
; CHECK-FIELD0-B-NEXT:      Constant propagation status: performed
; CHECK-FIELD0-B-NEXT:        [0] Array Pointer: Read
; CHECK-FIELD0-B-NEXT:        [1] Element size: Written | Constant = i64 4
; CHECK-FIELD0-B-NEXT:        [2] Co-Dimension: Written | Constant = i64 0
; CHECK-FIELD0-B-NEXT:        [3] Flags: Written
; CHECK-FIELD0-B-NEXT:        [4] Dimensions: Written | Constant = i64 2
; CHECK-FIELD0-B-NEXT:        [6][0] Extent: Written | Constant = i64 10
; CHECK-FIELD0-B-NEXT:        [6][0] Stride: Read | Written | Constant = i64 4
; CHECK-FIELD0-B-NEXT:        [6][0] Lower Bound: Read | Written | Constant = i64 1
; CHECK-FIELD0-B-NEXT:        [6][1] Extent: Written | Constant = i64 10
; CHECK-FIELD0-B-NEXT:        [6][1] Stride: Read | Written | Constant = i64 40
; CHECK-FIELD0-B-NEXT:        [6][1] Lower Bound: Read | Written | Constant = i64 1


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
@arr_mod_mp_b_ = internal global %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$" { ptr null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@anon.5adb142a4af92269a23dd8f105f60717.0 = internal unnamed_addr constant i32 2
@anon.5adb142a4af92269a23dd8f105f60717.1 = internal unnamed_addr constant i32 10

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_allocate_arr_(ptr noalias nocapture readonly dereferenceable(4) %0) #0 {
  %2 = alloca i64, align 8
  %3 = alloca i64, align 8
  %4 = load i32, ptr %0, align 1
  %5 = icmp eq i32 %4, 1
  br i1 %5, label %8, label %6

6:                                                ; preds = %1
  %7 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  br label %36

8:                                                ; preds = %1
  %9 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
  %10 = and i64 %9, 1030792151296
  %11 = or i64 %10, 133
  store i64 %11, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 5), align 8
  store i64 288, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 1), align 8
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 4), align 16
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 2), align 16
  %12 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %12, align 1
  %13 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 1, ptr %13, align 1
  %14 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 288, ptr %14, align 1
  %15 = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %2, i32 2, i64 1, i64 288) #3
  %16 = load i64, ptr %2, align 8
  %17 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
  %18 = and i64 %17, -68451041281
  %19 = or i64 %18, 1073741824
  store i64 %19, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
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
  %33 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 5), align 8
  %34 = inttoptr i64 %33 to ptr
  %35 = tail call i32 @for_alloc_allocatable_handle(i64 %16, ptr @arr_mod_mp_a_, i32 %32, ptr %34) #3
  br label %36

36:                                               ; preds = %8, %6
  %37 = phi ptr [ %7, %6 ], [ %12, %8 ]
  %38 = load ptr, ptr @arr_mod_mp_a_, align 16
  %39 = load i64, ptr %37, align 1
  %40 = sext i32 %4 to i64
  %41 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %39, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %38, i64 %40)
  %42 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %41, i64 0, i32 0, i32 3
  %43 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %41, i64 0, i32 0, i32 5
  store i64 0, ptr %43, align 1
  %44 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %41, i64 0, i32 0, i32 1
  store i64 4, ptr %44, align 1
  %45 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %41, i64 0, i32 0, i32 4
  store i64 2, ptr %45, align 1
  %46 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %41, i64 0, i32 0, i32 2
  store i64 0, ptr %46, align 1
  %47 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %41, i64 0, i32 0, i32 6, i64 0, i32 2
  %48 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %47, i32 0)
  store i64 1, ptr %48, align 1
  %49 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %41, i64 0, i32 0, i32 6, i64 0, i32 0
  %50 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %49, i32 0)
  store i64 10, ptr %50, align 1
  %51 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %47, i32 1)
  store i64 1, ptr %51, align 1
  %52 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %49, i32 1)
  store i64 10, ptr %52, align 1
  %53 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %41, i64 0, i32 0, i32 6, i64 0, i32 1
  %54 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %53, i32 0)
  store i64 4, ptr %54, align 1
  %55 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %53, i32 1)
  store i64 40, ptr %55, align 1
  store i64 1073741829, ptr %42, align 1
  %56 = bitcast ptr %41 to ptr
  %57 = tail call i32 @for_allocate_handle(i64 400, ptr %56, i32 262144, ptr null) #3
  %58 = load ptr, ptr @arr_mod_mp_a_, align 16
  %59 = load i64, ptr %37, align 1
  %60 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %59, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %58, i64 %40)
  %61 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %60, i64 0, i32 1, i32 3
  %62 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %60, i64 0, i32 1, i32 5
  store i64 0, ptr %62, align 1
  %63 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %60, i64 0, i32 1, i32 1
  store i64 4, ptr %63, align 1
  %64 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %60, i64 0, i32 1, i32 4
  store i64 3, ptr %64, align 1
  %65 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %60, i64 0, i32 1, i32 2
  store i64 0, ptr %65, align 1
  %66 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %60, i64 0, i32 1, i32 6, i64 0, i32 2
  %67 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %66, i32 0)
  store i64 1, ptr %67, align 1
  %68 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %60, i64 0, i32 1, i32 6, i64 0, i32 0
  %69 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %68, i32 0)
  store i64 10, ptr %69, align 1
  %70 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %66, i32 1)
  store i64 1, ptr %70, align 1
  %71 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %68, i32 1)
  store i64 10, ptr %71, align 1
  %72 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %66, i32 2)
  store i64 1, ptr %72, align 1
  %73 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %68, i32 2)
  store i64 10, ptr %73, align 1
  %74 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %60, i64 0, i32 1, i32 6, i64 0, i32 1
  %75 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %74, i32 0)
  store i64 4, ptr %75, align 1
  %76 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %74, i32 1)
  store i64 40, ptr %76, align 1
  %77 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %74, i32 2)
  store i64 400, ptr %77, align 1
  %78 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %60, i64 0, i32 1, i32 0
  store i64 1073741829, ptr %61, align 1
  %79 = bitcast ptr %78 to ptr
  %80 = tail call i32 @for_allocate_handle(i64 4000, ptr nonnull %79, i32 262144, ptr null) #3
  %81 = load ptr, ptr @arr_mod_mp_a_, align 16
  %82 = load i64, ptr %37, align 1
  %83 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %82, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %81, i64 %40)
  %84 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %83, i64 0, i32 2, i32 3
  %85 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %83, i64 0, i32 2, i32 5
  store i64 0, ptr %85, align 1
  %86 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %83, i64 0, i32 2, i32 1
  store i64 4, ptr %86, align 1
  %87 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %83, i64 0, i32 2, i32 4
  store i64 1, ptr %87, align 1
  %88 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %83, i64 0, i32 2, i32 2
  store i64 0, ptr %88, align 1
  %89 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %83, i64 0, i32 2, i32 6, i64 0, i32 2
  %90 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %89, i32 0)
  store i64 1, ptr %90, align 1
  %91 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %83, i64 0, i32 2, i32 6, i64 0, i32 0
  %92 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %91, i32 0)
  store i64 10, ptr %92, align 1
  %93 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %83, i64 0, i32 2, i32 6, i64 0, i32 1
  %94 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %93, i32 0)
  store i64 4, ptr %94, align 1
  %95 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %83, i64 0, i32 2, i32 0
  store i64 1073741829, ptr %84, align 1
  %96 = bitcast ptr %95 to ptr
  %97 = tail call i32 @for_allocate_handle(i64 40, ptr nonnull %96, i32 262144, ptr null) #3
  br i1 %5, label %100, label %98

98:                                               ; preds = %36
  %99 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 6, i64 0, i32 2), i32 0)
  br label %128

100:                                              ; preds = %36
  %101 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 3), align 8
  %102 = and i64 %101, 1030792151296
  %103 = or i64 %102, 133
  store i64 %103, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 3), align 8
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 5), align 8
  store i64 288, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 1), align 8
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 4), align 16
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 2), align 16
  %104 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %104, align 1
  %105 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 %40, ptr %105, align 1
  %106 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 288, ptr %106, align 1
  %107 = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %3, i32 2, i64 %40, i64 288) #3
  %108 = load i64, ptr %3, align 8
  %109 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 3), align 8
  %110 = and i64 %109, -68451041281
  %111 = or i64 %110, 1073741824
  store i64 %111, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 3), align 8
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
  %125 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 5), align 8
  %126 = inttoptr i64 %125 to ptr
  %127 = tail call i32 @for_alloc_allocatable_handle(i64 %108, ptr @arr_mod_mp_b_, i32 %124, ptr %126) #3
  br label %128

128:                                              ; preds = %100, %98
  %129 = phi ptr [ %99, %98 ], [ %104, %100 ]
  %130 = load ptr, ptr @arr_mod_mp_b_, align 16
  %131 = load i64, ptr %129, align 1
  %132 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %131, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %130, i64 %40)
  %133 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %132, i64 0, i32 0, i32 3
  %134 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %132, i64 0, i32 0, i32 5
  store i64 0, ptr %134, align 1
  %135 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %132, i64 0, i32 0, i32 1
  store i64 4, ptr %135, align 1
  %136 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %132, i64 0, i32 0, i32 4
  store i64 2, ptr %136, align 1
  %137 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %132, i64 0, i32 0, i32 2
  store i64 0, ptr %137, align 1
  %138 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %132, i64 0, i32 0, i32 6, i64 0, i32 2
  %139 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %138, i32 0)
  store i64 1, ptr %139, align 1
  %140 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %132, i64 0, i32 0, i32 6, i64 0, i32 0
  %141 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %140, i32 0)
  store i64 10, ptr %141, align 1
  %142 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %138, i32 1)
  store i64 1, ptr %142, align 1
  %143 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %140, i32 1)
  store i64 10, ptr %143, align 1
  %144 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %132, i64 0, i32 0, i32 6, i64 0, i32 1
  %145 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %144, i32 0)
  store i64 4, ptr %145, align 1
  %146 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %144, i32 1)
  store i64 40, ptr %146, align 1
  store i64 1073741829, ptr %133, align 1
  %147 = bitcast ptr %132 to ptr
  %148 = tail call i32 @for_allocate_handle(i64 400, ptr %147, i32 262144, ptr null) #3
  %149 = load ptr, ptr @arr_mod_mp_b_, align 16
  %150 = load i64, ptr %129, align 1
  %151 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %150, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %149, i64 %40)
  %152 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %151, i64 0, i32 1, i32 3
  %153 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %151, i64 0, i32 1, i32 5
  store i64 0, ptr %153, align 1
  %154 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %151, i64 0, i32 1, i32 1
  store i64 4, ptr %154, align 1
  %155 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %151, i64 0, i32 1, i32 4
  store i64 3, ptr %155, align 1
  %156 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %151, i64 0, i32 1, i32 2
  store i64 0, ptr %156, align 1
  %157 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %151, i64 0, i32 1, i32 6, i64 0, i32 2
  %158 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %157, i32 0)
  store i64 1, ptr %158, align 1
  %159 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %151, i64 0, i32 1, i32 6, i64 0, i32 0
  %160 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %159, i32 0)
  store i64 10, ptr %160, align 1
  %161 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %157, i32 1)
  store i64 1, ptr %161, align 1
  %162 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %159, i32 1)
  store i64 10, ptr %162, align 1
  %163 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %157, i32 2)
  store i64 1, ptr %163, align 1
  %164 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %159, i32 2)
  store i64 10, ptr %164, align 1
  %165 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %151, i64 0, i32 1, i32 6, i64 0, i32 1
  %166 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %165, i32 0)
  store i64 4, ptr %166, align 1
  %167 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %165, i32 1)
  store i64 40, ptr %167, align 1
  %168 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %165, i32 2)
  store i64 400, ptr %168, align 1
  %169 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %151, i64 0, i32 1, i32 0
  store i64 1073741829, ptr %152, align 1
  %170 = bitcast ptr %169 to ptr
  %171 = tail call i32 @for_allocate_handle(i64 4000, ptr nonnull %170, i32 262144, ptr null) #3
  %172 = load ptr, ptr @arr_mod_mp_b_, align 16
  %173 = load i64, ptr %129, align 1
  %174 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %173, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %172, i64 %40)
  %175 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %174, i64 0, i32 2, i32 3
  %176 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %174, i64 0, i32 2, i32 5
  store i64 0, ptr %176, align 1
  %177 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %174, i64 0, i32 2, i32 1
  store i64 4, ptr %177, align 1
  %178 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %174, i64 0, i32 2, i32 4
  store i64 1, ptr %178, align 1
  %179 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %174, i64 0, i32 2, i32 2
  store i64 0, ptr %179, align 1
  %180 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %174, i64 0, i32 2, i32 6, i64 0, i32 2
  %181 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %180, i32 0)
  store i64 1, ptr %181, align 1
  %182 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %174, i64 0, i32 2, i32 6, i64 0, i32 0
  %183 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %182, i32 0)
  store i64 10, ptr %183, align 1
  %184 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %174, i64 0, i32 2, i32 6, i64 0, i32 1
  %185 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %184, i32 0)
  store i64 4, ptr %185, align 1
  %186 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %174, i64 0, i32 2, i32 0
  store i64 1073741829, ptr %175, align 1
  %187 = bitcast ptr %186 to ptr
  %188 = tail call i32 @for_allocate_handle(i64 40, ptr nonnull %187, i32 262144, ptr null) #3
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_check_mult_overflow64(ptr nocapture, i32, ...) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_alloc_allocatable_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_initialize_arr_(ptr noalias nocapture readonly dereferenceable(4) %0, ptr noalias nocapture readonly dereferenceable(4) %1, ptr noalias nocapture readonly dereferenceable(4) %2, ptr noalias nocapture readonly dereferenceable(4) %3) #0 {
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
  %76 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 6, i64 0, i32 2), i32 0)
  br label %77

77:                                               ; preds = %129, %75
  %78 = phi i64 [ 1, %75 ], [ %142, %129 ]
  %79 = trunc i64 %78 to i32
  %80 = sitofp i32 %79 to float
  br label %81

81:                                               ; preds = %126, %77
  %82 = phi i64 [ 1, %77 ], [ %127, %126 ]
  %83 = load ptr, ptr @arr_mod_mp_b_, align 16
  %84 = load i64, ptr %76, align 1
  %85 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %84, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %83, i64 %7)
  %86 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %85, i64 0, i32 0, i32 0
  %87 = load ptr, ptr %86, align 1
  %88 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %85, i64 0, i32 0, i32 6, i64 0, i32 1
  %89 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %88, i32 0)
  %90 = load i64, ptr %89, align 1
  %91 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %85, i64 0, i32 0, i32 6, i64 0, i32 2
  %92 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %91, i32 0)
  %93 = load i64, ptr %92, align 1
  %94 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %88, i32 1)
  %95 = load i64, ptr %94, align 1
  %96 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %91, i32 1)
  %97 = load i64, ptr %96, align 1
  %98 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %97, i64 %95, ptr elementtype(float) %87, i64 %78)
  %99 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %93, i64 %90, ptr elementtype(float) %98, i64 %82)
  store float %80, ptr %99, align 1
  br label %100

100:                                              ; preds = %100, %81
  %101 = phi i64 [ %124, %100 ], [ 1, %81 ]
  %102 = load ptr, ptr @arr_mod_mp_b_, align 16
  %103 = load i64, ptr %76, align 1
  %104 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %103, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %102, i64 %7)
  %105 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %104, i64 0, i32 1, i32 0
  %106 = load ptr, ptr %105, align 1
  %107 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %104, i64 0, i32 1, i32 6, i64 0, i32 1
  %108 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %107, i32 0)
  %109 = load i64, ptr %108, align 1
  %110 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %104, i64 0, i32 1, i32 6, i64 0, i32 2
  %111 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %110, i32 0)
  %112 = load i64, ptr %111, align 1
  %113 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %107, i32 1)
  %114 = load i64, ptr %113, align 1
  %115 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %110, i32 1)
  %116 = load i64, ptr %115, align 1
  %117 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %107, i32 2)
  %118 = load i64, ptr %117, align 1
  %119 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %110, i32 2)
  %120 = load i64, ptr %119, align 1
  %121 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %120, i64 %118, ptr elementtype(float) %106, i64 %101)
  %122 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %116, i64 %114, ptr elementtype(float) %121, i64 %78)
  %123 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %112, i64 %109, ptr elementtype(float) %122, i64 %82)
  store float %80, ptr %123, align 1
  %124 = add nuw nsw i64 %101, 1
  %125 = icmp eq i64 %124, 11
  br i1 %125, label %126, label %100

126:                                              ; preds = %100
  %127 = add nuw nsw i64 %82, 1
  %128 = icmp eq i64 %127, 11
  br i1 %128, label %129, label %81

129:                                              ; preds = %126
  %130 = load ptr, ptr @arr_mod_mp_b_, align 16
  %131 = load i64, ptr %76, align 1
  %132 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %131, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %130, i64 %7)
  %133 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %132, i64 0, i32 2, i32 0
  %134 = load ptr, ptr %133, align 1
  %135 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %132, i64 0, i32 2, i32 6, i64 0, i32 1
  %136 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %135, i32 0)
  %137 = load i64, ptr %136, align 1
  %138 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %132, i64 0, i32 2, i32 6, i64 0, i32 2
  %139 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %138, i32 0)
  %140 = load i64, ptr %139, align 1
  %141 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %140, i64 %137, ptr elementtype(float) %134, i64 11)
  store float 1.100000e+01, ptr %141, align 1
  %142 = add nuw nsw i64 %78, 1
  %143 = icmp eq i64 %142, 11
  br i1 %143, label %144, label %77

144:                                              ; preds = %129
  ret void
}

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_print_arr_(ptr noalias nocapture readonly dereferenceable(4) %0, ptr noalias nocapture readonly dereferenceable(4) %1, ptr noalias nocapture readonly dereferenceable(4) %2, ptr noalias nocapture readonly dereferenceable(4) %3) #0 {
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
  %18 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %19 = load i32, ptr %0, align 1
  %20 = sext i32 %19 to i64
  %21 = getelementptr inbounds [4 x i8], ptr %6, i64 0, i64 0
  %22 = getelementptr inbounds [4 x i8], ptr %6, i64 0, i64 1
  %23 = getelementptr inbounds [4 x i8], ptr %6, i64 0, i64 2
  %24 = getelementptr inbounds [4 x i8], ptr %6, i64 0, i64 3
  %25 = getelementptr inbounds { float }, ptr %7, i64 0, i32 0
  %26 = bitcast ptr %5 to ptr
  %27 = bitcast ptr %7 to ptr
  %28 = getelementptr inbounds [4 x i8], ptr %8, i64 0, i64 0
  %29 = getelementptr inbounds [4 x i8], ptr %8, i64 0, i64 1
  %30 = getelementptr inbounds [4 x i8], ptr %8, i64 0, i64 2
  %31 = getelementptr inbounds [4 x i8], ptr %8, i64 0, i64 3
  %32 = getelementptr inbounds { float }, ptr %9, i64 0, i32 0
  %33 = bitcast ptr %9 to ptr
  %34 = getelementptr inbounds [4 x i8], ptr %10, i64 0, i64 0
  %35 = getelementptr inbounds [4 x i8], ptr %10, i64 0, i64 1
  %36 = getelementptr inbounds [4 x i8], ptr %10, i64 0, i64 2
  %37 = getelementptr inbounds [4 x i8], ptr %10, i64 0, i64 3
  %38 = getelementptr inbounds { float }, ptr %11, i64 0, i32 0
  %39 = bitcast ptr %11 to ptr
  br label %40

40:                                               ; preds = %96, %4
  %41 = phi i64 [ 1, %4 ], [ %113, %96 ]
  br label %42

42:                                               ; preds = %92, %40
  %43 = phi i64 [ %93, %92 ], [ 1, %40 ]
  %44 = load ptr, ptr @arr_mod_mp_a_, align 16
  %45 = load i64, ptr %18, align 1
  %46 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %45, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %44, i64 %20)
  %47 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %46, i64 0, i32 0, i32 0
  %48 = load ptr, ptr %47, align 1
  %49 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %46, i64 0, i32 0, i32 6, i64 0, i32 1
  %50 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %49, i32 0)
  %51 = load i64, ptr %50, align 1
  %52 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %46, i64 0, i32 0, i32 6, i64 0, i32 2
  %53 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %52, i32 0)
  %54 = load i64, ptr %53, align 1
  %55 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %49, i32 1)
  %56 = load i64, ptr %55, align 1
  %57 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %52, i32 1)
  %58 = load i64, ptr %57, align 1
  %59 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %58, i64 %56, ptr elementtype(float) %48, i64 %41)
  %60 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %54, i64 %51, ptr elementtype(float) %59, i64 %43)
  %61 = load float, ptr %60, align 1
  store i8 26, ptr %21, align 1
  store i8 1, ptr %22, align 1
  store i8 1, ptr %23, align 1
  store i8 0, ptr %24, align 1
  store float %61, ptr %25, align 8
  %62 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %26, i32 -1, i64 1239157112576, ptr nonnull %21, ptr nonnull %27) #3
  br label %63

63:                                               ; preds = %63, %42
  %64 = phi i64 [ %89, %63 ], [ 1, %42 ]
  %65 = load ptr, ptr @arr_mod_mp_a_, align 16
  %66 = load i64, ptr %18, align 1
  %67 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %66, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %65, i64 %20)
  %68 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %67, i64 0, i32 1, i32 0
  %69 = load ptr, ptr %68, align 1
  %70 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %67, i64 0, i32 1, i32 6, i64 0, i32 1
  %71 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %70, i32 0)
  %72 = load i64, ptr %71, align 1
  %73 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %67, i64 0, i32 1, i32 6, i64 0, i32 2
  %74 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %73, i32 0)
  %75 = load i64, ptr %74, align 1
  %76 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %70, i32 1)
  %77 = load i64, ptr %76, align 1
  %78 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %73, i32 1)
  %79 = load i64, ptr %78, align 1
  %80 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %70, i32 2)
  %81 = load i64, ptr %80, align 1
  %82 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %73, i32 2)
  %83 = load i64, ptr %82, align 1
  %84 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %83, i64 %81, ptr elementtype(float) %69, i64 %64)
  %85 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %79, i64 %77, ptr elementtype(float) %84, i64 %41)
  %86 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %75, i64 %72, ptr elementtype(float) %85, i64 %43)
  %87 = load float, ptr %86, align 1
  store i8 26, ptr %28, align 1
  store i8 1, ptr %29, align 1
  store i8 1, ptr %30, align 1
  store i8 0, ptr %31, align 1
  store float %87, ptr %32, align 8
  %88 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %26, i32 -1, i64 1239157112576, ptr nonnull %28, ptr nonnull %33) #3
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
  %97 = load ptr, ptr @arr_mod_mp_a_, align 16
  %98 = load i64, ptr %18, align 1
  %99 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %98, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %97, i64 %20)
  %100 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %99, i64 0, i32 2, i32 0
  %101 = load ptr, ptr %100, align 1
  %102 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %99, i64 0, i32 2, i32 6, i64 0, i32 1
  %103 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %102, i32 0)
  %104 = load i64, ptr %103, align 1
  %105 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %99, i64 0, i32 2, i32 6, i64 0, i32 2
  %106 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %105, i32 0)
  %107 = load i64, ptr %106, align 1
  %108 = shl i64 %93, 32
  %109 = ashr exact i64 %108, 32
  %110 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %107, i64 %104, ptr elementtype(float) %101, i64 %109)
  %111 = load float, ptr %110, align 1
  store i8 26, ptr %34, align 1
  store i8 1, ptr %35, align 1
  store i8 1, ptr %36, align 1
  store i8 0, ptr %37, align 1
  store float %111, ptr %38, align 8
  %112 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %26, i32 -1, i64 1239157112576, ptr nonnull %34, ptr nonnull %39) #3
  %113 = add nuw i64 %41, 1
  %114 = trunc i64 %113 to i32
  %115 = icmp slt i32 10, %114
  br i1 %115, label %116, label %40

116:                                              ; preds = %96
  %117 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %118 = getelementptr inbounds [4 x i8], ptr %12, i64 0, i64 0
  %119 = getelementptr inbounds [4 x i8], ptr %12, i64 0, i64 1
  %120 = getelementptr inbounds [4 x i8], ptr %12, i64 0, i64 2
  %121 = getelementptr inbounds [4 x i8], ptr %12, i64 0, i64 3
  %122 = getelementptr inbounds { float }, ptr %13, i64 0, i32 0
  %123 = bitcast ptr %13 to ptr
  %124 = getelementptr inbounds [4 x i8], ptr %14, i64 0, i64 0
  %125 = getelementptr inbounds [4 x i8], ptr %14, i64 0, i64 1
  %126 = getelementptr inbounds [4 x i8], ptr %14, i64 0, i64 2
  %127 = getelementptr inbounds [4 x i8], ptr %14, i64 0, i64 3
  %128 = getelementptr inbounds { float }, ptr %15, i64 0, i32 0
  %129 = bitcast ptr %15 to ptr
  %130 = getelementptr inbounds [4 x i8], ptr %16, i64 0, i64 0
  %131 = getelementptr inbounds [4 x i8], ptr %16, i64 0, i64 1
  %132 = getelementptr inbounds [4 x i8], ptr %16, i64 0, i64 2
  %133 = getelementptr inbounds [4 x i8], ptr %16, i64 0, i64 3
  %134 = getelementptr inbounds { float }, ptr %17, i64 0, i32 0
  %135 = bitcast ptr %17 to ptr
  br label %136

136:                                              ; preds = %192, %116
  %137 = phi i64 [ 1, %116 ], [ %209, %192 ]
  br label %138

138:                                              ; preds = %188, %136
  %139 = phi i64 [ %189, %188 ], [ 1, %136 ]
  %140 = load ptr, ptr @arr_mod_mp_b_, align 16
  %141 = load i64, ptr %117, align 1
  %142 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %141, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %140, i64 %20)
  %143 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %142, i64 0, i32 0, i32 0
  %144 = load ptr, ptr %143, align 1
  %145 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %142, i64 0, i32 0, i32 6, i64 0, i32 1
  %146 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %145, i32 0)
  %147 = load i64, ptr %146, align 1
  %148 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %142, i64 0, i32 0, i32 6, i64 0, i32 2
  %149 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %148, i32 0)
  %150 = load i64, ptr %149, align 1
  %151 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %145, i32 1)
  %152 = load i64, ptr %151, align 1
  %153 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %148, i32 1)
  %154 = load i64, ptr %153, align 1
  %155 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %154, i64 %152, ptr elementtype(float) %144, i64 %137)
  %156 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %150, i64 %147, ptr elementtype(float) %155, i64 %139)
  %157 = load float, ptr %156, align 1
  store i8 26, ptr %118, align 1
  store i8 1, ptr %119, align 1
  store i8 1, ptr %120, align 1
  store i8 0, ptr %121, align 1
  store float %157, ptr %122, align 8
  %158 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %26, i32 -1, i64 1239157112576, ptr nonnull %118, ptr nonnull %123) #3
  br label %159

159:                                              ; preds = %159, %138
  %160 = phi i64 [ %185, %159 ], [ 1, %138 ]
  %161 = load ptr, ptr @arr_mod_mp_b_, align 16
  %162 = load i64, ptr %117, align 1
  %163 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %162, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %161, i64 %20)
  %164 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %163, i64 0, i32 1, i32 0
  %165 = load ptr, ptr %164, align 1
  %166 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %163, i64 0, i32 1, i32 6, i64 0, i32 1
  %167 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %166, i32 0)
  %168 = load i64, ptr %167, align 1
  %169 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %163, i64 0, i32 1, i32 6, i64 0, i32 2
  %170 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %169, i32 0)
  %171 = load i64, ptr %170, align 1
  %172 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %166, i32 1)
  %173 = load i64, ptr %172, align 1
  %174 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %169, i32 1)
  %175 = load i64, ptr %174, align 1
  %176 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %166, i32 2)
  %177 = load i64, ptr %176, align 1
  %178 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %169, i32 2)
  %179 = load i64, ptr %178, align 1
  %180 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %179, i64 %177, ptr elementtype(float) %165, i64 %160)
  %181 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %175, i64 %173, ptr elementtype(float) %180, i64 %137)
  %182 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %171, i64 %168, ptr elementtype(float) %181, i64 %139)
  %183 = load float, ptr %182, align 1
  store i8 26, ptr %124, align 1
  store i8 1, ptr %125, align 1
  store i8 1, ptr %126, align 1
  store i8 0, ptr %127, align 1
  store float %183, ptr %128, align 8
  %184 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %26, i32 -1, i64 1239157112576, ptr nonnull %124, ptr nonnull %129) #3
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
  %193 = load ptr, ptr @arr_mod_mp_b_, align 16
  %194 = load i64, ptr %117, align 1
  %195 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %194, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %193, i64 %20)
  %196 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %195, i64 0, i32 2, i32 0
  %197 = load ptr, ptr %196, align 1
  %198 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %195, i64 0, i32 2, i32 6, i64 0, i32 1
  %199 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %198, i32 0)
  %200 = load i64, ptr %199, align 1
  %201 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %195, i64 0, i32 2, i32 6, i64 0, i32 2
  %202 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %201, i32 0)
  %203 = load i64, ptr %202, align 1
  %204 = shl i64 %189, 32
  %205 = ashr exact i64 %204, 32
  %206 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %203, i64 %200, ptr elementtype(float) %197, i64 %205)
  %207 = load float, ptr %206, align 1
  store i8 26, ptr %130, align 1
  store i8 1, ptr %131, align 1
  store i8 1, ptr %132, align 1
  store i8 0, ptr %133, align 1
  store float %207, ptr %134, align 8
  %208 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %26, i32 -1, i64 1239157112576, ptr nonnull %130, ptr nonnull %135) #3
  %209 = add nuw i64 %137, 1
  %210 = trunc i64 %209 to i32
  %211 = icmp slt i32 10, %210
  br i1 %211, label %212, label %136

212:                                              ; preds = %192
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr #1

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #0 {
  %1 = alloca i32, align 8
  %2 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.5adb142a4af92269a23dd8f105f60717.0) #3
  store i32 1, ptr %1, align 8
  br label %3

3:                                                ; preds = %3, %0
  %4 = phi i32 [ %5, %3 ], [ 1, %0 ]
  call void @arr_mod_mp_allocate_arr_(ptr nonnull %1)
  call void @arr_mod_mp_initialize_arr_(ptr nonnull %1, ptr nonnull @anon.5adb142a4af92269a23dd8f105f60717.1, ptr nonnull @anon.5adb142a4af92269a23dd8f105f60717.1, ptr nonnull @anon.5adb142a4af92269a23dd8f105f60717.1)
  call void @arr_mod_mp_print_arr_(ptr nonnull %1, ptr nonnull @anon.5adb142a4af92269a23dd8f105f60717.1, ptr nonnull @anon.5adb142a4af92269a23dd8f105f60717.1, ptr nonnull @anon.5adb142a4af92269a23dd8f105f60717.1)
  %5 = add nuw nsw i32 %4, 1
  store i32 %5, ptr %1, align 8
  %6 = icmp eq i32 %5, 11
  br i1 %6, label %7, label %3

7:                                                ; preds = %3
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #2

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+mmx,+pclmul,+pku,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nounwind readnone speculatable }
attributes #3 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}

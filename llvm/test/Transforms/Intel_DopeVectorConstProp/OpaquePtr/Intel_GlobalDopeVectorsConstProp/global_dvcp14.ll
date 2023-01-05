; REQUIRES: asserts
; RUN: opt -opaque-pointers < %s -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-GLOBDV

; RUN: opt -opaque-pointers < %s -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-FIELD0

; RUN: opt -opaque-pointers < %s -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-FIELD1

; RUN: opt -opaque-pointers < %s -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s -check-prefix=CHECK-FIELD2

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

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"ARR_MOD$.btT_TESTTYPE" = type { %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank1$" }
%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank3$" = type { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@arr_mod_mp_a_ = internal global %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$" { ptr null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@anon.87529b4ebf98830a9107fed24e462e82.0 = internal unnamed_addr constant i32 2
@anon.87529b4ebf98830a9107fed24e462e82.1 = internal unnamed_addr constant i32 10

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_allocate_arr_(ptr noalias nocapture readonly dereferenceable(4) %arg) #0 {
bb:
  %i = alloca i64, align 8
  %i1 = load i32, ptr %arg, align 1
  %i2 = icmp eq i32 %i1, 1
  br i1 %i2, label %bb5, label %bb3

bb3:                                              ; preds = %bb
  %i4 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  br label %bb33

bb5:                                              ; preds = %bb
  %i6 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
  %i7 = and i64 %i6, 1030792151296
  %i8 = or i64 %i7, 133
  store i64 %i8, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 5), align 8
  store i64 288, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 1), align 8
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 4), align 16
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 2), align 16
  %i9 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %i9, align 1
  %i10 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 1, ptr %i10, align 1
  %i11 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 288, ptr %i11, align 1
  %i12 = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %i, i32 2, i64 1, i64 288) #4
  %i13 = load i64, ptr %i, align 8
  %i14 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
  %i15 = and i64 %i14, -68451041281
  %i16 = or i64 %i15, 1073741824
  store i64 %i16, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
  %i17 = trunc i64 %i14 to i32
  %i18 = shl i32 %i17, 1
  %i19 = and i32 %i18, 2
  %i20 = shl i32 %i12, 4
  %i21 = and i32 %i20, 16
  %i22 = lshr i64 %i14, 15
  %i23 = trunc i64 %i22 to i32
  %i24 = and i32 %i23, 31457280
  %i25 = and i32 %i23, 33554432
  %i26 = or i32 %i21, %i19
  %i27 = or i32 %i26, %i24
  %i28 = or i32 %i27, %i25
  %i29 = or i32 %i28, 262144
  %i30 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 5), align 8
  %i31 = inttoptr i64 %i30 to ptr
  %i32 = tail call i32 @for_alloc_allocatable_handle(i64 %i13, ptr @arr_mod_mp_a_, i32 %i29, ptr %i31) #4
  br label %bb33

bb33:                                             ; preds = %bb5, %bb3
  %i34 = phi ptr [ %i4, %bb3 ], [ %i9, %bb5 ]
  %i35 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i36 = load i64, ptr %i34, align 1
  %i37 = sext i32 %i1 to i64
  %i38 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i36, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i35, i64 %i37)
  %i39 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 3
  %i40 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 5
  store i64 0, ptr %i40, align 1
  %i41 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 1
  store i64 4, ptr %i41, align 1
  %i42 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 4
  store i64 2, ptr %i42, align 1
  %i43 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 2
  store i64 0, ptr %i43, align 1
  %i44 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 6, i64 0, i32 2
  %i45 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i44, i32 0)
  store i64 1, ptr %i45, align 1
  %i46 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 6, i64 0, i32 0
  %i47 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i46, i32 0)
  store i64 10, ptr %i47, align 1
  %i48 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i44, i32 1)
  store i64 1, ptr %i48, align 1
  %i49 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i46, i32 1)
  store i64 10, ptr %i49, align 1
  %i50 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 6, i64 0, i32 1
  %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i50, i32 0)
  store i64 4, ptr %i51, align 1
  %i52 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i50, i32 1)
  store i64 40, ptr %i52, align 1
  store i64 1073741829, ptr %i39, align 1
  %i53 = bitcast ptr %i38 to ptr
  %i54 = tail call i32 @for_allocate_handle(i64 400, ptr %i53, i32 262144, ptr null) #4
  %i55 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i56 = load i64, ptr %i34, align 1
  %i57 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i56, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i55, i64 %i37)
  %i58 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 3
  %i59 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 5
  store i64 0, ptr %i59, align 1
  %i60 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 1
  store i64 4, ptr %i60, align 1
  %i61 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 4
  store i64 3, ptr %i61, align 1
  %i62 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 2
  store i64 0, ptr %i62, align 1
  %i63 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 6, i64 0, i32 2
  %i64 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i63, i32 0)
  store i64 1, ptr %i64, align 1
  %i65 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 6, i64 0, i32 0
  %i66 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i65, i32 0)
  store i64 10, ptr %i66, align 1
  %i67 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i63, i32 1)
  store i64 1, ptr %i67, align 1
  %i68 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i65, i32 1)
  store i64 10, ptr %i68, align 1
  %i69 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i63, i32 2)
  store i64 1, ptr %i69, align 1
  %i70 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i65, i32 2)
  store i64 10, ptr %i70, align 1
  %i71 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 6, i64 0, i32 1
  %i72 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i71, i32 0)
  store i64 4, ptr %i72, align 1
  %i73 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i71, i32 1)
  store i64 40, ptr %i73, align 1
  %i74 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i71, i32 2)
  store i64 400, ptr %i74, align 1
  %i75 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 0
  store i64 1073741829, ptr %i58, align 1
  %i76 = bitcast ptr %i75 to ptr
  %i77 = tail call i32 @for_allocate_handle(i64 4000, ptr nonnull %i76, i32 262144, ptr null) #4
  %i78 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i79 = load i64, ptr %i34, align 1
  %i80 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i79, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i78, i64 %i37)
  %i81 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i80, i64 0, i32 2, i32 3
  %i82 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i80, i64 0, i32 2, i32 5
  store i64 0, ptr %i82, align 1
  %i83 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i80, i64 0, i32 2, i32 1
  store i64 4, ptr %i83, align 1
  %i84 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i80, i64 0, i32 2, i32 4
  store i64 1, ptr %i84, align 1
  %i85 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i80, i64 0, i32 2, i32 2
  store i64 0, ptr %i85, align 1
  %i86 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i80, i64 0, i32 2, i32 6, i64 0, i32 2
  %i87 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i86, i32 0)
  store i64 1, ptr %i87, align 1
  %i88 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i80, i64 0, i32 2, i32 6, i64 0, i32 0
  %i89 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i88, i32 0)
  store i64 10, ptr %i89, align 1
  %i90 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i80, i64 0, i32 2, i32 6, i64 0, i32 1
  %i91 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i90, i32 0)
  store i64 4, ptr %i91, align 1
  %i92 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i80, i64 0, i32 2, i32 0
  store i64 1073741829, ptr %i81, align 1
  %i93 = bitcast ptr %i92 to ptr
  %i94 = tail call i32 @for_allocate_handle(i64 40, ptr nonnull %i93, i32 262144, ptr null) #4
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_check_mult_overflow64(ptr nocapture, i32, ...) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_alloc_allocatable_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_print_arr_(ptr noalias nocapture readonly dereferenceable(4) %arg, ptr noalias nocapture readonly dereferenceable(4) %arg1, ptr noalias nocapture readonly dereferenceable(4) %arg2, ptr noalias nocapture readonly dereferenceable(4) %arg3) #0 {
bb:
  %i = alloca [8 x i64], align 16
  %i4 = alloca [4 x i8], align 1
  %i5 = alloca { float }, align 8
  %i6 = alloca [4 x i8], align 1
  %i7 = alloca { float }, align 8
  %i8 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %i9 = load i32, ptr %arg, align 1
  %i10 = sext i32 %i9 to i64
  %i11 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 0
  %i12 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 1
  %i13 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 2
  %i14 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 3
  %i15 = getelementptr inbounds { float }, ptr %i5, i64 0, i32 0
  %i16 = bitcast ptr %i to ptr
  %i17 = bitcast ptr %i5 to ptr
  %i18 = getelementptr inbounds [4 x i8], ptr %i6, i64 0, i64 0
  %i19 = getelementptr inbounds [4 x i8], ptr %i6, i64 0, i64 1
  %i20 = getelementptr inbounds [4 x i8], ptr %i6, i64 0, i64 2
  %i21 = getelementptr inbounds [4 x i8], ptr %i6, i64 0, i64 3
  %i22 = getelementptr inbounds { float }, ptr %i7, i64 0, i32 0
  %i23 = bitcast ptr %i7 to ptr
  br label %bb24

bb24:                                             ; preds = %bb80, %bb
  %i25 = phi i64 [ 1, %bb ], [ %i81, %bb80 ]
  br label %bb26

bb26:                                             ; preds = %bb76, %bb24
  %i27 = phi i64 [ %i77, %bb76 ], [ 1, %bb24 ]
  %i28 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i29 = load i64, ptr %i8, align 1
  %i30 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i29, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i28, i64 %i10)
  %i31 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i30, i64 0, i32 0, i32 0
  %i32 = load ptr, ptr %i31, align 1
  %i33 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i30, i64 0, i32 0, i32 6, i64 0, i32 1
  %i34 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i33, i32 0)
  %i35 = load i64, ptr %i34, align 1
  %i36 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i30, i64 0, i32 0, i32 6, i64 0, i32 2
  %i37 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i36, i32 0)
  %i38 = load i64, ptr %i37, align 1
  %i39 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i33, i32 1)
  %i40 = load i64, ptr %i39, align 1
  %i41 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i36, i32 1)
  %i42 = load i64, ptr %i41, align 1
  %i43 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i42, i64 %i40, ptr elementtype(float) %i32, i64 %i25)
  %i44 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i38, i64 %i35, ptr elementtype(float) %i43, i64 %i27)
  %i45 = load float, ptr %i44, align 1
  store i8 26, ptr %i11, align 1
  store i8 1, ptr %i12, align 1
  store i8 1, ptr %i13, align 1
  store i8 0, ptr %i14, align 1
  store float %i45, ptr %i15, align 8
  %i46 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i16, i32 -1, i64 1239157112576, ptr nonnull %i11, ptr nonnull %i17) #4
  br label %bb47

bb47:                                             ; preds = %bb47, %bb26
  %i48 = phi i64 [ %i73, %bb47 ], [ 1, %bb26 ]
  %i49 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i50 = load i64, ptr %i8, align 1
  %i51 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i50, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i49, i64 %i10)
  %i52 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i51, i64 0, i32 1, i32 0
  %i53 = load ptr, ptr %i52, align 1
  %i54 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i51, i64 0, i32 1, i32 6, i64 0, i32 1
  %i55 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i54, i32 0)
  %i56 = load i64, ptr %i55, align 1
  %i57 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i51, i64 0, i32 1, i32 6, i64 0, i32 2
  %i58 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i57, i32 0)
  %i59 = load i64, ptr %i58, align 1
  %i60 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i54, i32 1)
  %i61 = load i64, ptr %i60, align 1
  %i62 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i57, i32 1)
  %i63 = load i64, ptr %i62, align 1
  %i64 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i54, i32 2)
  %i65 = load i64, ptr %i64, align 1
  %i66 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i57, i32 2)
  %i67 = load i64, ptr %i66, align 1
  %i68 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %i67, i64 %i65, ptr elementtype(float) %i53, i64 %i48)
  %i69 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i63, i64 %i61, ptr elementtype(float) %i68, i64 %i25)
  %i70 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i59, i64 %i56, ptr elementtype(float) %i69, i64 %i27)
  %i71 = load float, ptr %i70, align 1
  store i8 26, ptr %i18, align 1
  store i8 1, ptr %i19, align 1
  store i8 1, ptr %i20, align 1
  store i8 0, ptr %i21, align 1
  store float %i71, ptr %i22, align 8
  %i72 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i16, i32 -1, i64 1239157112576, ptr nonnull %i18, ptr nonnull %i23) #4
  %i73 = add nuw i64 %i48, 1
  %i74 = trunc i64 %i73 to i32
  %i75 = icmp slt i32 10, %i74
  br i1 %i75, label %bb76, label %bb47

bb76:                                             ; preds = %bb47
  %i77 = add nuw i64 %i27, 1
  %i78 = trunc i64 %i77 to i32
  %i79 = icmp slt i32 10, %i78
  br i1 %i79, label %bb80, label %bb26

bb80:                                             ; preds = %bb76
  %i81 = add nuw i64 %i25, 1
  %i82 = trunc i64 %i81 to i32
  %i83 = icmp slt i32 10, %i82
  br i1 %i83, label %bb84, label %bb24

bb84:                                             ; preds = %bb80
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr #1

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #0 {
bb:
  %i = alloca i32, align 8
  %i1 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.87529b4ebf98830a9107fed24e462e82.0) #4
  store i32 1, ptr %i, align 8
  %i2 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0) #4
  br label %bb3

bb3:                                              ; preds = %bb61, %bb
  %i4 = phi i32 [ %i62, %bb61 ], [ 1, %bb ]
  call void @arr_mod_mp_allocate_arr_(ptr nonnull %i)
  %i5 = zext i32 %i4 to i64
  br label %bb6

bb6:                                              ; preds = %bb58, %bb3
  %i7 = phi i64 [ %i59, %bb58 ], [ 1, %bb3 ]
  %i8 = trunc i64 %i7 to i32
  %i9 = sitofp i32 %i8 to float
  br label %bb10

bb10:                                             ; preds = %bb55, %bb6
  %i11 = phi i64 [ %i56, %bb55 ], [ 1, %bb6 ]
  %i12 = load ptr, ptr @arr_mod_mp_a_, align 16, !noalias !3
  %i13 = load i64, ptr %i2, align 1, !noalias !3
  %i14 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i13, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i12, i64 %i5) #4
  %i15 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i14, i64 0, i32 0, i32 0
  %i16 = load ptr, ptr %i15, align 1, !noalias !3
  %i17 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i14, i64 0, i32 0, i32 6, i64 0, i32 1
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i17, i32 0) #4
  %i19 = load i64, ptr %i18, align 1, !noalias !3
  %i20 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i14, i64 0, i32 0, i32 6, i64 0, i32 2
  %i21 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i20, i32 0) #4
  %i22 = load i64, ptr %i21, align 1, !noalias !3
  %i23 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i17, i32 1) #4
  %i24 = load i64, ptr %i23, align 1, !noalias !3
  %i25 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i20, i32 1) #4
  %i26 = load i64, ptr %i25, align 1, !noalias !3
  %i27 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i26, i64 %i24, ptr elementtype(float) %i16, i64 %i7) #4
  %i28 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i22, i64 %i19, ptr elementtype(float) %i27, i64 %i11) #4
  store float %i9, ptr %i28, align 1, !noalias !3
  br label %bb29

bb29:                                             ; preds = %bb29, %bb10
  %i30 = phi i64 [ %i53, %bb29 ], [ 1, %bb10 ]
  %i31 = load ptr, ptr @arr_mod_mp_a_, align 16, !noalias !3
  %i32 = load i64, ptr %i2, align 1, !noalias !3
  %i33 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i32, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i31, i64 %i5) #4
  %i34 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i33, i64 0, i32 1, i32 0
  %i35 = load ptr, ptr %i34, align 1, !noalias !3
  %i36 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i33, i64 0, i32 1, i32 6, i64 0, i32 1
  %i37 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i36, i32 0) #4
  %i38 = load i64, ptr %i37, align 1, !noalias !3
  %i39 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i33, i64 0, i32 1, i32 6, i64 0, i32 2
  %i40 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i39, i32 0) #4
  %i41 = load i64, ptr %i40, align 1, !noalias !3
  %i42 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i36, i32 1) #4
  %i43 = load i64, ptr %i42, align 1, !noalias !3
  %i44 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i39, i32 1) #4
  %i45 = load i64, ptr %i44, align 1, !noalias !3
  %i46 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i36, i32 2) #4
  %i47 = load i64, ptr %i46, align 1, !noalias !3
  %i48 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i39, i32 2) #4
  %i49 = load i64, ptr %i48, align 1, !noalias !3
  %i50 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %i49, i64 %i47, ptr elementtype(float) %i35, i64 %i30) #4
  %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i45, i64 %i43, ptr elementtype(float) %i50, i64 %i7) #4
  %i52 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i41, i64 %i38, ptr elementtype(float) %i51, i64 %i11) #4
  store float %i9, ptr %i52, align 1, !noalias !3
  %i53 = add nuw nsw i64 %i30, 1
  %i54 = icmp eq i64 %i53, 11
  br i1 %i54, label %bb55, label %bb29

bb55:                                             ; preds = %bb29
  %i56 = add nuw nsw i64 %i11, 1
  %i57 = icmp eq i64 %i56, 11
  br i1 %i57, label %bb58, label %bb10

bb58:                                             ; preds = %bb55
  %i59 = add nuw nsw i64 %i7, 1
  %i60 = icmp eq i64 %i59, 11
  br i1 %i60, label %bb61, label %bb6

bb61:                                             ; preds = %bb58
  call void @arr_mod_mp_print_arr_(ptr nonnull %i, ptr nonnull @anon.87529b4ebf98830a9107fed24e462e82.1, ptr nonnull @anon.87529b4ebf98830a9107fed24e462e82.1, ptr nonnull @anon.87529b4ebf98830a9107fed24e462e82.1)
  %i62 = add nuw nsw i32 %i4, 1
  store i32 %i62, ptr %i, align 8
  %i63 = icmp ult i32 %i4, 10
  br i1 %i63, label %bb3, label %bb64

bb64:                                             ; preds = %bb61
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: nocallback nofree nosync nounwind readnone willreturn
declare i32 @llvm.ssa.copy.i32(i32 returned) #2

; Function Attrs: nocallback nofree nosync nounwind readnone willreturn
declare i64 @llvm.ssa.copy.i64(i64 returned) #2

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #3

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #3

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nocallback nofree nosync nounwind readnone willreturn }
attributes #3 = { nounwind readnone speculatable }
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

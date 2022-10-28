; RUN: opt -opaque-pointers < %s -passes=dopevectorconstprop -dope-vector-global-const-prop=true -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s

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
; CHECK:   %i39 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i36, i64 %i38)
; CHECK:   %i58 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i56, i64 %i38)
; CHECK:   %i81 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i79, i64 %i38)
; CHECK:   %i130 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i128, i64 %i38)
; CHECK:   %i149 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i147, i64 %i38)
; CHECK:   %i172 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i170, i64 %i38)

; Check that the constants were propagated for function @arr_mod_mp_initialize_arr_
; CHECK: define internal void @arr_mod_mp_initialize_arr_
; CHECK:   %i14 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i12, i64 %i5)
; CHECK:   %i27 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i16, i64 %i7)
; CHECK:   %i28 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i27, i64 %i11)
; CHECK:   %i33 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i31, i64 %i5)
; CHECK:   %i50 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 400, ptr elementtype(float) %i35, i64 %i30)
; CHECK:   %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i50, i64 %i7)
; CHECK:   %i52 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i51, i64 %i11)
; CHECK:   %i61 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i59, i64 %i5)
; CHECK:   %i70 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i63, i64 11)
; CHECK:   %i83 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i81, i64 %i5)
; CHECK:   %i96 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i85, i64 %i76)
; CHECK:   %i97 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i96, i64 %i80)
; CHECK:   %i102 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i100, i64 %i5)
; CHECK:   %i119 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 400, ptr elementtype(float) %i104, i64 %i99)
; CHECK:   %i120 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i119, i64 %i76)
; CHECK:   %i121 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i120, i64 %i80)
; CHECK:   %i130 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i128, i64 %i5)
; CHECK:   %i139 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i132, i64 11)

; Check that the constants were propagated for function @arr_mod_mp_print_arr_
; CHECK: define internal void @arr_mod_mp_print_arr_
; CHECK:   %i44 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i42, i64 %i18)
; CHECK:   %i57 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i46, i64 %i39)
; CHECK:   %i58 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i57, i64 %i41)
; CHECK:   %i65 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i63, i64 %i18)
; CHECK:   %i82 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 400, ptr elementtype(float) %i67, i64 %i62)
; CHECK:   %i83 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i82, i64 %i39)
; CHECK:   %i84 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i83, i64 %i41)
; CHECK:   %i97 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i95, i64 %i18)
; CHECK:   %i108 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i99, i64 %i107)
; CHECK:   %i140 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i138, i64 %i18)
; CHECK:   %i153 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i142, i64 %i135)
; CHECK:   %i154 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i153, i64 %i137)
; CHECK:   %i161 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i159, i64 %i18)
; CHECK:   %i178 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 400, ptr elementtype(float) %i163, i64 %i158)
; CHECK:   %i179 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i178, i64 %i135)
; CHECK:   %i180 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i179, i64 %i137)
; CHECK:   %i193 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i191, i64 %i18)
; CHECK:   %i204 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i195, i64 %i203)

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
define internal void @arr_mod_mp_allocate_arr_(ptr noalias nocapture readonly dereferenceable(4) %arg) #0 {
bb:
  %i = alloca i64, align 8
  %i1 = alloca i64, align 8
  %i2 = load i32, ptr %arg, align 1
  %i3 = icmp eq i32 %i2, 1
  br i1 %i3, label %bb6, label %bb4

bb4:                                              ; preds = %bb
  %i5 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  br label %bb34

bb6:                                              ; preds = %bb
  %i7 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
  %i8 = and i64 %i7, 1030792151296
  %i9 = or i64 %i8, 133
  store i64 %i9, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 5), align 8
  store i64 288, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 1), align 8
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 4), align 16
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 2), align 16
  %i10 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %i10, align 1
  %i11 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 1, ptr %i11, align 1
  %i12 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 288, ptr %i12, align 1
  %i13 = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %i, i32 2, i64 1, i64 288) #3
  %i14 = load i64, ptr %i, align 8
  %i15 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
  %i16 = and i64 %i15, -68451041281
  %i17 = or i64 %i16, 1073741824
  store i64 %i17, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
  %i18 = trunc i64 %i15 to i32
  %i19 = shl i32 %i18, 1
  %i20 = and i32 %i19, 2
  %i21 = shl i32 %i13, 4
  %i22 = and i32 %i21, 16
  %i23 = lshr i64 %i15, 15
  %i24 = trunc i64 %i23 to i32
  %i25 = and i32 %i24, 31457280
  %i26 = and i32 %i24, 33554432
  %i27 = or i32 %i22, %i20
  %i28 = or i32 %i27, %i25
  %i29 = or i32 %i28, %i26
  %i30 = or i32 %i29, 262144
  %i31 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 5), align 8
  %i32 = inttoptr i64 %i31 to ptr
  %i33 = tail call i32 @for_alloc_allocatable_handle(i64 %i14, ptr @arr_mod_mp_a_, i32 %i30, ptr %i32) #3
  br label %bb34

bb34:                                             ; preds = %bb6, %bb4
  %i35 = phi ptr [ %i5, %bb4 ], [ %i10, %bb6 ]
  %i36 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i37 = load i64, ptr %i35, align 1
  %i38 = sext i32 %i2 to i64
  %i39 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i37, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i36, i64 %i38)
  %i40 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i39, i64 0, i32 0, i32 3
  %i41 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i39, i64 0, i32 0, i32 5
  store i64 0, ptr %i41, align 1
  %i42 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i39, i64 0, i32 0, i32 1
  store i64 4, ptr %i42, align 1
  %i43 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i39, i64 0, i32 0, i32 4
  store i64 2, ptr %i43, align 1
  %i44 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i39, i64 0, i32 0, i32 2
  store i64 0, ptr %i44, align 1
  %i45 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i39, i64 0, i32 0, i32 6, i64 0, i32 2
  %i46 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i45, i32 0)
  store i64 1, ptr %i46, align 1
  %i47 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i39, i64 0, i32 0, i32 6, i64 0, i32 0
  %i48 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i47, i32 0)
  store i64 10, ptr %i48, align 1
  %i49 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i45, i32 1)
  store i64 1, ptr %i49, align 1
  %i50 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i47, i32 1)
  store i64 10, ptr %i50, align 1
  %i51 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i39, i64 0, i32 0, i32 6, i64 0, i32 1
  %i52 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i51, i32 0)
  store i64 4, ptr %i52, align 1
  %i53 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i51, i32 1)
  store i64 40, ptr %i53, align 1
  store i64 1073741829, ptr %i40, align 1
  %i54 = bitcast ptr %i39 to ptr
  %i55 = tail call i32 @for_allocate_handle(i64 400, ptr %i54, i32 262144, ptr null) #3
  %i56 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i57 = load i64, ptr %i35, align 1
  %i58 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i57, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i56, i64 %i38)
  %i59 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i58, i64 0, i32 1, i32 3
  %i60 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i58, i64 0, i32 1, i32 5
  store i64 0, ptr %i60, align 1
  %i61 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i58, i64 0, i32 1, i32 1
  store i64 4, ptr %i61, align 1
  %i62 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i58, i64 0, i32 1, i32 4
  store i64 3, ptr %i62, align 1
  %i63 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i58, i64 0, i32 1, i32 2
  store i64 0, ptr %i63, align 1
  %i64 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i58, i64 0, i32 1, i32 6, i64 0, i32 2
  %i65 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i64, i32 0)
  store i64 1, ptr %i65, align 1
  %i66 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i58, i64 0, i32 1, i32 6, i64 0, i32 0
  %i67 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i66, i32 0)
  store i64 10, ptr %i67, align 1
  %i68 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i64, i32 1)
  store i64 1, ptr %i68, align 1
  %i69 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i66, i32 1)
  store i64 10, ptr %i69, align 1
  %i70 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i64, i32 2)
  store i64 1, ptr %i70, align 1
  %i71 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i66, i32 2)
  store i64 10, ptr %i71, align 1
  %i72 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i58, i64 0, i32 1, i32 6, i64 0, i32 1
  %i73 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i72, i32 0)
  store i64 4, ptr %i73, align 1
  %i74 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i72, i32 1)
  store i64 40, ptr %i74, align 1
  %i75 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i72, i32 2)
  store i64 400, ptr %i75, align 1
  %i76 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i58, i64 0, i32 1, i32 0
  store i64 1073741829, ptr %i59, align 1
  %i77 = bitcast ptr %i76 to ptr
  %i78 = tail call i32 @for_allocate_handle(i64 4000, ptr nonnull %i77, i32 262144, ptr null) #3
  %i79 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i80 = load i64, ptr %i35, align 1
  %i81 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i80, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i79, i64 %i38)
  %i82 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i81, i64 0, i32 2, i32 3
  %i83 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i81, i64 0, i32 2, i32 5
  store i64 0, ptr %i83, align 1
  %i84 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i81, i64 0, i32 2, i32 1
  store i64 4, ptr %i84, align 1
  %i85 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i81, i64 0, i32 2, i32 4
  store i64 1, ptr %i85, align 1
  %i86 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i81, i64 0, i32 2, i32 2
  store i64 0, ptr %i86, align 1
  %i87 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i81, i64 0, i32 2, i32 6, i64 0, i32 2
  %i88 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i87, i32 0)
  store i64 1, ptr %i88, align 1
  %i89 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i81, i64 0, i32 2, i32 6, i64 0, i32 0
  %i90 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i89, i32 0)
  store i64 10, ptr %i90, align 1
  %i91 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i81, i64 0, i32 2, i32 6, i64 0, i32 1
  %i92 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i91, i32 0)
  store i64 4, ptr %i92, align 1
  %i93 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i81, i64 0, i32 2, i32 0
  store i64 1073741829, ptr %i82, align 1
  %i94 = bitcast ptr %i93 to ptr
  %i95 = tail call i32 @for_allocate_handle(i64 40, ptr nonnull %i94, i32 262144, ptr null) #3
  br i1 %i3, label %bb98, label %bb96

bb96:                                             ; preds = %bb34
  %i97 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 6, i64 0, i32 2), i32 0)
  br label %bb126

bb98:                                             ; preds = %bb34
  %i99 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 3), align 8
  %i100 = and i64 %i99, 1030792151296
  %i101 = or i64 %i100, 133
  store i64 %i101, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 3), align 8
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 5), align 8
  store i64 288, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 1), align 8
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 4), align 16
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 2), align 16
  %i102 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %i102, align 1
  %i103 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 %i38, ptr %i103, align 1
  %i104 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 288, ptr %i104, align 1
  %i105 = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %i1, i32 2, i64 %i38, i64 288) #3
  %i106 = load i64, ptr %i1, align 8
  %i107 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 3), align 8
  %i108 = and i64 %i107, -68451041281
  %i109 = or i64 %i108, 1073741824
  store i64 %i109, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 3), align 8
  %i110 = trunc i64 %i107 to i32
  %i111 = shl i32 %i110, 1
  %i112 = and i32 %i111, 2
  %i113 = shl i32 %i105, 4
  %i114 = and i32 %i113, 16
  %i115 = lshr i64 %i107, 15
  %i116 = trunc i64 %i115 to i32
  %i117 = and i32 %i116, 31457280
  %i118 = and i32 %i116, 33554432
  %i119 = or i32 %i114, %i112
  %i120 = or i32 %i119, %i117
  %i121 = or i32 %i120, %i118
  %i122 = or i32 %i121, 262144
  %i123 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 5), align 8
  %i124 = inttoptr i64 %i123 to ptr
  %i125 = tail call i32 @for_alloc_allocatable_handle(i64 %i106, ptr @arr_mod_mp_b_, i32 %i122, ptr %i124) #3
  br label %bb126

bb126:                                            ; preds = %bb98, %bb96
  %i127 = phi ptr [ %i97, %bb96 ], [ %i102, %bb98 ]
  %i128 = load ptr, ptr @arr_mod_mp_b_, align 16
  %i129 = load i64, ptr %i127, align 1
  %i130 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i129, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i128, i64 %i38)
  %i131 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i130, i64 0, i32 0, i32 3
  %i132 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i130, i64 0, i32 0, i32 5
  store i64 0, ptr %i132, align 1
  %i133 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i130, i64 0, i32 0, i32 1
  store i64 4, ptr %i133, align 1
  %i134 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i130, i64 0, i32 0, i32 4
  store i64 2, ptr %i134, align 1
  %i135 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i130, i64 0, i32 0, i32 2
  store i64 0, ptr %i135, align 1
  %i136 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i130, i64 0, i32 0, i32 6, i64 0, i32 2
  %i137 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i136, i32 0)
  store i64 1, ptr %i137, align 1
  %i138 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i130, i64 0, i32 0, i32 6, i64 0, i32 0
  %i139 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i138, i32 0)
  store i64 10, ptr %i139, align 1
  %i140 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i136, i32 1)
  store i64 1, ptr %i140, align 1
  %i141 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i138, i32 1)
  store i64 10, ptr %i141, align 1
  %i142 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i130, i64 0, i32 0, i32 6, i64 0, i32 1
  %i143 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i142, i32 0)
  store i64 4, ptr %i143, align 1
  %i144 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i142, i32 1)
  store i64 40, ptr %i144, align 1
  store i64 1073741829, ptr %i131, align 1
  %i145 = bitcast ptr %i130 to ptr
  %i146 = tail call i32 @for_allocate_handle(i64 400, ptr %i145, i32 262144, ptr null) #3
  %i147 = load ptr, ptr @arr_mod_mp_b_, align 16
  %i148 = load i64, ptr %i127, align 1
  %i149 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i148, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i147, i64 %i38)
  %i150 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i149, i64 0, i32 1, i32 3
  %i151 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i149, i64 0, i32 1, i32 5
  store i64 0, ptr %i151, align 1
  %i152 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i149, i64 0, i32 1, i32 1
  store i64 4, ptr %i152, align 1
  %i153 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i149, i64 0, i32 1, i32 4
  store i64 3, ptr %i153, align 1
  %i154 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i149, i64 0, i32 1, i32 2
  store i64 0, ptr %i154, align 1
  %i155 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i149, i64 0, i32 1, i32 6, i64 0, i32 2
  %i156 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i155, i32 0)
  store i64 1, ptr %i156, align 1
  %i157 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i149, i64 0, i32 1, i32 6, i64 0, i32 0
  %i158 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i157, i32 0)
  store i64 10, ptr %i158, align 1
  %i159 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i155, i32 1)
  store i64 1, ptr %i159, align 1
  %i160 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i157, i32 1)
  store i64 10, ptr %i160, align 1
  %i161 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i155, i32 2)
  store i64 1, ptr %i161, align 1
  %i162 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i157, i32 2)
  store i64 10, ptr %i162, align 1
  %i163 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i149, i64 0, i32 1, i32 6, i64 0, i32 1
  %i164 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i163, i32 0)
  store i64 4, ptr %i164, align 1
  %i165 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i163, i32 1)
  store i64 40, ptr %i165, align 1
  %i166 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i163, i32 2)
  store i64 400, ptr %i166, align 1
  %i167 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i149, i64 0, i32 1, i32 0
  store i64 1073741829, ptr %i150, align 1
  %i168 = bitcast ptr %i167 to ptr
  %i169 = tail call i32 @for_allocate_handle(i64 4000, ptr nonnull %i168, i32 262144, ptr null) #3
  %i170 = load ptr, ptr @arr_mod_mp_b_, align 16
  %i171 = load i64, ptr %i127, align 1
  %i172 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i171, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i170, i64 %i38)
  %i173 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i172, i64 0, i32 2, i32 3
  %i174 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i172, i64 0, i32 2, i32 5
  store i64 0, ptr %i174, align 1
  %i175 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i172, i64 0, i32 2, i32 1
  store i64 4, ptr %i175, align 1
  %i176 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i172, i64 0, i32 2, i32 4
  store i64 1, ptr %i176, align 1
  %i177 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i172, i64 0, i32 2, i32 2
  store i64 0, ptr %i177, align 1
  %i178 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i172, i64 0, i32 2, i32 6, i64 0, i32 2
  %i179 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i178, i32 0)
  store i64 1, ptr %i179, align 1
  %i180 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i172, i64 0, i32 2, i32 6, i64 0, i32 0
  %i181 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i180, i32 0)
  store i64 10, ptr %i181, align 1
  %i182 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i172, i64 0, i32 2, i32 6, i64 0, i32 1
  %i183 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i182, i32 0)
  store i64 4, ptr %i183, align 1
  %i184 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i172, i64 0, i32 2, i32 0
  store i64 1073741829, ptr %i173, align 1
  %i185 = bitcast ptr %i184 to ptr
  %i186 = tail call i32 @for_allocate_handle(i64 40, ptr nonnull %i185, i32 262144, ptr null) #3
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_check_mult_overflow64(ptr nocapture, i32, ...) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_alloc_allocatable_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_initialize_arr_(ptr noalias nocapture readonly dereferenceable(4) %arg, ptr noalias nocapture readonly dereferenceable(4) %arg1, ptr noalias nocapture readonly dereferenceable(4) %arg2, ptr noalias nocapture readonly dereferenceable(4) %arg3) #0 {
bb:
  %i = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %i4 = load i32, ptr %arg, align 1
  %i5 = sext i32 %i4 to i64
  br label %bb6

bb6:                                              ; preds = %bb58, %bb
  %i7 = phi i64 [ 1, %bb ], [ %i71, %bb58 ]
  %i8 = trunc i64 %i7 to i32
  %i9 = sitofp i32 %i8 to float
  br label %bb10

bb10:                                             ; preds = %bb55, %bb6
  %i11 = phi i64 [ 1, %bb6 ], [ %i56, %bb55 ]
  %i12 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i13 = load i64, ptr %i, align 1
  %i14 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i13, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i12, i64 %i5)
  %i15 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i14, i64 0, i32 0, i32 0
  %i16 = load ptr, ptr %i15, align 1
  %i17 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i14, i64 0, i32 0, i32 6, i64 0, i32 1
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i17, i32 0)
  %i19 = load i64, ptr %i18, align 1
  %i20 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i14, i64 0, i32 0, i32 6, i64 0, i32 2
  %i21 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i20, i32 0)
  %i22 = load i64, ptr %i21, align 1
  %i23 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i17, i32 1)
  %i24 = load i64, ptr %i23, align 1
  %i25 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i20, i32 1)
  %i26 = load i64, ptr %i25, align 1
  %i27 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i26, i64 %i24, ptr elementtype(float) %i16, i64 %i7)
  %i28 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i22, i64 %i19, ptr elementtype(float) %i27, i64 %i11)
  store float %i9, ptr %i28, align 1
  br label %bb29

bb29:                                             ; preds = %bb29, %bb10
  %i30 = phi i64 [ %i53, %bb29 ], [ 1, %bb10 ]
  %i31 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i32 = load i64, ptr %i, align 1
  %i33 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i32, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i31, i64 %i5)
  %i34 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i33, i64 0, i32 1, i32 0
  %i35 = load ptr, ptr %i34, align 1
  %i36 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i33, i64 0, i32 1, i32 6, i64 0, i32 1
  %i37 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i36, i32 0)
  %i38 = load i64, ptr %i37, align 1
  %i39 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i33, i64 0, i32 1, i32 6, i64 0, i32 2
  %i40 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i39, i32 0)
  %i41 = load i64, ptr %i40, align 1
  %i42 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i36, i32 1)
  %i43 = load i64, ptr %i42, align 1
  %i44 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i39, i32 1)
  %i45 = load i64, ptr %i44, align 1
  %i46 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i36, i32 2)
  %i47 = load i64, ptr %i46, align 1
  %i48 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i39, i32 2)
  %i49 = load i64, ptr %i48, align 1
  %i50 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %i49, i64 %i47, ptr elementtype(float) %i35, i64 %i30)
  %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i45, i64 %i43, ptr elementtype(float) %i50, i64 %i7)
  %i52 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i41, i64 %i38, ptr elementtype(float) %i51, i64 %i11)
  store float %i9, ptr %i52, align 1
  %i53 = add nuw nsw i64 %i30, 1
  %i54 = icmp eq i64 %i53, 11
  br i1 %i54, label %bb55, label %bb29

bb55:                                             ; preds = %bb29
  %i56 = add nuw nsw i64 %i11, 1
  %i57 = icmp eq i64 %i56, 11
  br i1 %i57, label %bb58, label %bb10

bb58:                                             ; preds = %bb55
  %i59 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i60 = load i64, ptr %i, align 1
  %i61 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i60, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i59, i64 %i5)
  %i62 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i61, i64 0, i32 2, i32 0
  %i63 = load ptr, ptr %i62, align 1
  %i64 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i61, i64 0, i32 2, i32 6, i64 0, i32 1
  %i65 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i64, i32 0)
  %i66 = load i64, ptr %i65, align 1
  %i67 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i61, i64 0, i32 2, i32 6, i64 0, i32 2
  %i68 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i67, i32 0)
  %i69 = load i64, ptr %i68, align 1
  %i70 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i69, i64 %i66, ptr elementtype(float) %i63, i64 11)
  store float 1.100000e+01, ptr %i70, align 1
  %i71 = add nuw nsw i64 %i7, 1
  %i72 = icmp eq i64 %i71, 11
  br i1 %i72, label %bb73, label %bb6

bb73:                                             ; preds = %bb58
  %i74 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 6, i64 0, i32 2), i32 0)
  br label %bb75

bb75:                                             ; preds = %bb127, %bb73
  %i76 = phi i64 [ 1, %bb73 ], [ %i140, %bb127 ]
  %i77 = trunc i64 %i76 to i32
  %i78 = sitofp i32 %i77 to float
  br label %bb79

bb79:                                             ; preds = %bb124, %bb75
  %i80 = phi i64 [ 1, %bb75 ], [ %i125, %bb124 ]
  %i81 = load ptr, ptr @arr_mod_mp_b_, align 16
  %i82 = load i64, ptr %i74, align 1
  %i83 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i82, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i81, i64 %i5)
  %i84 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i83, i64 0, i32 0, i32 0
  %i85 = load ptr, ptr %i84, align 1
  %i86 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i83, i64 0, i32 0, i32 6, i64 0, i32 1
  %i87 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i86, i32 0)
  %i88 = load i64, ptr %i87, align 1
  %i89 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i83, i64 0, i32 0, i32 6, i64 0, i32 2
  %i90 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i89, i32 0)
  %i91 = load i64, ptr %i90, align 1
  %i92 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i86, i32 1)
  %i93 = load i64, ptr %i92, align 1
  %i94 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i89, i32 1)
  %i95 = load i64, ptr %i94, align 1
  %i96 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i95, i64 %i93, ptr elementtype(float) %i85, i64 %i76)
  %i97 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i91, i64 %i88, ptr elementtype(float) %i96, i64 %i80)
  store float %i78, ptr %i97, align 1
  br label %bb98

bb98:                                             ; preds = %bb98, %bb79
  %i99 = phi i64 [ %i122, %bb98 ], [ 1, %bb79 ]
  %i100 = load ptr, ptr @arr_mod_mp_b_, align 16
  %i101 = load i64, ptr %i74, align 1
  %i102 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i101, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i100, i64 %i5)
  %i103 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i102, i64 0, i32 1, i32 0
  %i104 = load ptr, ptr %i103, align 1
  %i105 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i102, i64 0, i32 1, i32 6, i64 0, i32 1
  %i106 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i105, i32 0)
  %i107 = load i64, ptr %i106, align 1
  %i108 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i102, i64 0, i32 1, i32 6, i64 0, i32 2
  %i109 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i108, i32 0)
  %i110 = load i64, ptr %i109, align 1
  %i111 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i105, i32 1)
  %i112 = load i64, ptr %i111, align 1
  %i113 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i108, i32 1)
  %i114 = load i64, ptr %i113, align 1
  %i115 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i105, i32 2)
  %i116 = load i64, ptr %i115, align 1
  %i117 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i108, i32 2)
  %i118 = load i64, ptr %i117, align 1
  %i119 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %i118, i64 %i116, ptr elementtype(float) %i104, i64 %i99)
  %i120 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i114, i64 %i112, ptr elementtype(float) %i119, i64 %i76)
  %i121 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i110, i64 %i107, ptr elementtype(float) %i120, i64 %i80)
  store float %i78, ptr %i121, align 1
  %i122 = add nuw nsw i64 %i99, 1
  %i123 = icmp eq i64 %i122, 11
  br i1 %i123, label %bb124, label %bb98

bb124:                                            ; preds = %bb98
  %i125 = add nuw nsw i64 %i80, 1
  %i126 = icmp eq i64 %i125, 11
  br i1 %i126, label %bb127, label %bb79

bb127:                                            ; preds = %bb124
  %i128 = load ptr, ptr @arr_mod_mp_b_, align 16
  %i129 = load i64, ptr %i74, align 1
  %i130 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i129, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i128, i64 %i5)
  %i131 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i130, i64 0, i32 2, i32 0
  %i132 = load ptr, ptr %i131, align 1
  %i133 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i130, i64 0, i32 2, i32 6, i64 0, i32 1
  %i134 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i133, i32 0)
  %i135 = load i64, ptr %i134, align 1
  %i136 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i130, i64 0, i32 2, i32 6, i64 0, i32 2
  %i137 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i136, i32 0)
  %i138 = load i64, ptr %i137, align 1
  %i139 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i138, i64 %i135, ptr elementtype(float) %i132, i64 11)
  store float 1.100000e+01, ptr %i139, align 1
  %i140 = add nuw nsw i64 %i76, 1
  %i141 = icmp eq i64 %i140, 11
  br i1 %i141, label %bb142, label %bb75

bb142:                                            ; preds = %bb127
  ret void
}

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_print_arr_(ptr noalias nocapture readonly dereferenceable(4) %arg, ptr noalias nocapture readonly dereferenceable(4) %arg1, ptr noalias nocapture readonly dereferenceable(4) %arg2, ptr noalias nocapture readonly dereferenceable(4) %arg3) #0 {
bb:
  %i = alloca [8 x i64], align 16
  %i4 = alloca [4 x i8], align 1
  %i5 = alloca { float }, align 8
  %i6 = alloca [4 x i8], align 1
  %i7 = alloca { float }, align 8
  %i8 = alloca [4 x i8], align 1
  %i9 = alloca { float }, align 8
  %i10 = alloca [4 x i8], align 1
  %i11 = alloca { float }, align 8
  %i12 = alloca [4 x i8], align 1
  %i13 = alloca { float }, align 8
  %i14 = alloca [4 x i8], align 1
  %i15 = alloca { float }, align 8
  %i16 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %i17 = load i32, ptr %arg, align 1
  %i18 = sext i32 %i17 to i64
  %i19 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 0
  %i20 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 1
  %i21 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 2
  %i22 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 3
  %i23 = getelementptr inbounds { float }, ptr %i5, i64 0, i32 0
  %i24 = bitcast ptr %i to ptr
  %i25 = bitcast ptr %i5 to ptr
  %i26 = getelementptr inbounds [4 x i8], ptr %i6, i64 0, i64 0
  %i27 = getelementptr inbounds [4 x i8], ptr %i6, i64 0, i64 1
  %i28 = getelementptr inbounds [4 x i8], ptr %i6, i64 0, i64 2
  %i29 = getelementptr inbounds [4 x i8], ptr %i6, i64 0, i64 3
  %i30 = getelementptr inbounds { float }, ptr %i7, i64 0, i32 0
  %i31 = bitcast ptr %i7 to ptr
  %i32 = getelementptr inbounds [4 x i8], ptr %i8, i64 0, i64 0
  %i33 = getelementptr inbounds [4 x i8], ptr %i8, i64 0, i64 1
  %i34 = getelementptr inbounds [4 x i8], ptr %i8, i64 0, i64 2
  %i35 = getelementptr inbounds [4 x i8], ptr %i8, i64 0, i64 3
  %i36 = getelementptr inbounds { float }, ptr %i9, i64 0, i32 0
  %i37 = bitcast ptr %i9 to ptr
  br label %bb38

bb38:                                             ; preds = %bb94, %bb
  %i39 = phi i64 [ 1, %bb ], [ %i111, %bb94 ]
  br label %bb40

bb40:                                             ; preds = %bb90, %bb38
  %i41 = phi i64 [ %i91, %bb90 ], [ 1, %bb38 ]
  %i42 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i43 = load i64, ptr %i16, align 1
  %i44 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i43, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i42, i64 %i18)
  %i45 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i44, i64 0, i32 0, i32 0
  %i46 = load ptr, ptr %i45, align 1
  %i47 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i44, i64 0, i32 0, i32 6, i64 0, i32 1
  %i48 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i47, i32 0)
  %i49 = load i64, ptr %i48, align 1
  %i50 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i44, i64 0, i32 0, i32 6, i64 0, i32 2
  %i51 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i50, i32 0)
  %i52 = load i64, ptr %i51, align 1
  %i53 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i47, i32 1)
  %i54 = load i64, ptr %i53, align 1
  %i55 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i50, i32 1)
  %i56 = load i64, ptr %i55, align 1
  %i57 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i56, i64 %i54, ptr elementtype(float) %i46, i64 %i39)
  %i58 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i52, i64 %i49, ptr elementtype(float) %i57, i64 %i41)
  %i59 = load float, ptr %i58, align 1
  store i8 26, ptr %i19, align 1
  store i8 1, ptr %i20, align 1
  store i8 1, ptr %i21, align 1
  store i8 0, ptr %i22, align 1
  store float %i59, ptr %i23, align 8
  %i60 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i24, i32 -1, i64 1239157112576, ptr nonnull %i19, ptr nonnull %i25) #3
  br label %bb61

bb61:                                             ; preds = %bb61, %bb40
  %i62 = phi i64 [ %i87, %bb61 ], [ 1, %bb40 ]
  %i63 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i64 = load i64, ptr %i16, align 1
  %i65 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i64, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i63, i64 %i18)
  %i66 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i65, i64 0, i32 1, i32 0
  %i67 = load ptr, ptr %i66, align 1
  %i68 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i65, i64 0, i32 1, i32 6, i64 0, i32 1
  %i69 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i68, i32 0)
  %i70 = load i64, ptr %i69, align 1
  %i71 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i65, i64 0, i32 1, i32 6, i64 0, i32 2
  %i72 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i71, i32 0)
  %i73 = load i64, ptr %i72, align 1
  %i74 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i68, i32 1)
  %i75 = load i64, ptr %i74, align 1
  %i76 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i71, i32 1)
  %i77 = load i64, ptr %i76, align 1
  %i78 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i68, i32 2)
  %i79 = load i64, ptr %i78, align 1
  %i80 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i71, i32 2)
  %i81 = load i64, ptr %i80, align 1
  %i82 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %i81, i64 %i79, ptr elementtype(float) %i67, i64 %i62)
  %i83 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i77, i64 %i75, ptr elementtype(float) %i82, i64 %i39)
  %i84 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i73, i64 %i70, ptr elementtype(float) %i83, i64 %i41)
  %i85 = load float, ptr %i84, align 1
  store i8 26, ptr %i26, align 1
  store i8 1, ptr %i27, align 1
  store i8 1, ptr %i28, align 1
  store i8 0, ptr %i29, align 1
  store float %i85, ptr %i30, align 8
  %i86 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i24, i32 -1, i64 1239157112576, ptr nonnull %i26, ptr nonnull %i31) #3
  %i87 = add nuw i64 %i62, 1
  %i88 = trunc i64 %i87 to i32
  %i89 = icmp slt i32 10, %i88
  br i1 %i89, label %bb90, label %bb61

bb90:                                             ; preds = %bb61
  %i91 = add nuw i64 %i41, 1
  %i92 = trunc i64 %i91 to i32
  %i93 = icmp slt i32 10, %i92
  br i1 %i93, label %bb94, label %bb40

bb94:                                             ; preds = %bb90
  %i95 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i96 = load i64, ptr %i16, align 1
  %i97 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i96, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i95, i64 %i18)
  %i98 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i97, i64 0, i32 2, i32 0
  %i99 = load ptr, ptr %i98, align 1
  %i100 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i97, i64 0, i32 2, i32 6, i64 0, i32 1
  %i101 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i100, i32 0)
  %i102 = load i64, ptr %i101, align 1
  %i103 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i97, i64 0, i32 2, i32 6, i64 0, i32 2
  %i104 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i103, i32 0)
  %i105 = load i64, ptr %i104, align 1
  %i106 = shl i64 %i91, 32
  %i107 = ashr exact i64 %i106, 32
  %i108 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i105, i64 %i102, ptr elementtype(float) %i99, i64 %i107)
  %i109 = load float, ptr %i108, align 1
  store i8 26, ptr %i32, align 1
  store i8 1, ptr %i33, align 1
  store i8 1, ptr %i34, align 1
  store i8 0, ptr %i35, align 1
  store float %i109, ptr %i36, align 8
  %i110 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i24, i32 -1, i64 1239157112576, ptr nonnull %i32, ptr nonnull %i37) #3
  %i111 = add nuw i64 %i39, 1
  %i112 = trunc i64 %i111 to i32
  %i113 = icmp slt i32 10, %i112
  br i1 %i113, label %bb114, label %bb38

bb114:                                            ; preds = %bb94
  %i115 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %i116 = getelementptr inbounds [4 x i8], ptr %i10, i64 0, i64 0
  %i117 = getelementptr inbounds [4 x i8], ptr %i10, i64 0, i64 1
  %i118 = getelementptr inbounds [4 x i8], ptr %i10, i64 0, i64 2
  %i119 = getelementptr inbounds [4 x i8], ptr %i10, i64 0, i64 3
  %i120 = getelementptr inbounds { float }, ptr %i11, i64 0, i32 0
  %i121 = bitcast ptr %i11 to ptr
  %i122 = getelementptr inbounds [4 x i8], ptr %i12, i64 0, i64 0
  %i123 = getelementptr inbounds [4 x i8], ptr %i12, i64 0, i64 1
  %i124 = getelementptr inbounds [4 x i8], ptr %i12, i64 0, i64 2
  %i125 = getelementptr inbounds [4 x i8], ptr %i12, i64 0, i64 3
  %i126 = getelementptr inbounds { float }, ptr %i13, i64 0, i32 0
  %i127 = bitcast ptr %i13 to ptr
  %i128 = getelementptr inbounds [4 x i8], ptr %i14, i64 0, i64 0
  %i129 = getelementptr inbounds [4 x i8], ptr %i14, i64 0, i64 1
  %i130 = getelementptr inbounds [4 x i8], ptr %i14, i64 0, i64 2
  %i131 = getelementptr inbounds [4 x i8], ptr %i14, i64 0, i64 3
  %i132 = getelementptr inbounds { float }, ptr %i15, i64 0, i32 0
  %i133 = bitcast ptr %i15 to ptr
  br label %bb134

bb134:                                            ; preds = %bb190, %bb114
  %i135 = phi i64 [ 1, %bb114 ], [ %i207, %bb190 ]
  br label %bb136

bb136:                                            ; preds = %bb186, %bb134
  %i137 = phi i64 [ %i187, %bb186 ], [ 1, %bb134 ]
  %i138 = load ptr, ptr @arr_mod_mp_b_, align 16
  %i139 = load i64, ptr %i115, align 1
  %i140 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i139, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i138, i64 %i18)
  %i141 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i140, i64 0, i32 0, i32 0
  %i142 = load ptr, ptr %i141, align 1
  %i143 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i140, i64 0, i32 0, i32 6, i64 0, i32 1
  %i144 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i143, i32 0)
  %i145 = load i64, ptr %i144, align 1
  %i146 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i140, i64 0, i32 0, i32 6, i64 0, i32 2
  %i147 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i146, i32 0)
  %i148 = load i64, ptr %i147, align 1
  %i149 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i143, i32 1)
  %i150 = load i64, ptr %i149, align 1
  %i151 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i146, i32 1)
  %i152 = load i64, ptr %i151, align 1
  %i153 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i152, i64 %i150, ptr elementtype(float) %i142, i64 %i135)
  %i154 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i148, i64 %i145, ptr elementtype(float) %i153, i64 %i137)
  %i155 = load float, ptr %i154, align 1
  store i8 26, ptr %i116, align 1
  store i8 1, ptr %i117, align 1
  store i8 1, ptr %i118, align 1
  store i8 0, ptr %i119, align 1
  store float %i155, ptr %i120, align 8
  %i156 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i24, i32 -1, i64 1239157112576, ptr nonnull %i116, ptr nonnull %i121) #3
  br label %bb157

bb157:                                            ; preds = %bb157, %bb136
  %i158 = phi i64 [ %i183, %bb157 ], [ 1, %bb136 ]
  %i159 = load ptr, ptr @arr_mod_mp_b_, align 16
  %i160 = load i64, ptr %i115, align 1
  %i161 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i160, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i159, i64 %i18)
  %i162 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i161, i64 0, i32 1, i32 0
  %i163 = load ptr, ptr %i162, align 1
  %i164 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i161, i64 0, i32 1, i32 6, i64 0, i32 1
  %i165 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i164, i32 0)
  %i166 = load i64, ptr %i165, align 1
  %i167 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i161, i64 0, i32 1, i32 6, i64 0, i32 2
  %i168 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i167, i32 0)
  %i169 = load i64, ptr %i168, align 1
  %i170 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i164, i32 1)
  %i171 = load i64, ptr %i170, align 1
  %i172 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i167, i32 1)
  %i173 = load i64, ptr %i172, align 1
  %i174 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i164, i32 2)
  %i175 = load i64, ptr %i174, align 1
  %i176 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i167, i32 2)
  %i177 = load i64, ptr %i176, align 1
  %i178 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %i177, i64 %i175, ptr elementtype(float) %i163, i64 %i158)
  %i179 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i173, i64 %i171, ptr elementtype(float) %i178, i64 %i135)
  %i180 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i169, i64 %i166, ptr elementtype(float) %i179, i64 %i137)
  %i181 = load float, ptr %i180, align 1
  store i8 26, ptr %i122, align 1
  store i8 1, ptr %i123, align 1
  store i8 1, ptr %i124, align 1
  store i8 0, ptr %i125, align 1
  store float %i181, ptr %i126, align 8
  %i182 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i24, i32 -1, i64 1239157112576, ptr nonnull %i122, ptr nonnull %i127) #3
  %i183 = add nuw i64 %i158, 1
  %i184 = trunc i64 %i183 to i32
  %i185 = icmp slt i32 10, %i184
  br i1 %i185, label %bb186, label %bb157

bb186:                                            ; preds = %bb157
  %i187 = add nuw i64 %i137, 1
  %i188 = trunc i64 %i187 to i32
  %i189 = icmp slt i32 10, %i188
  br i1 %i189, label %bb190, label %bb136

bb190:                                            ; preds = %bb186
  %i191 = load ptr, ptr @arr_mod_mp_b_, align 16
  %i192 = load i64, ptr %i115, align 1
  %i193 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i192, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i191, i64 %i18)
  %i194 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i193, i64 0, i32 2, i32 0
  %i195 = load ptr, ptr %i194, align 1
  %i196 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i193, i64 0, i32 2, i32 6, i64 0, i32 1
  %i197 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i196, i32 0)
  %i198 = load i64, ptr %i197, align 1
  %i199 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i193, i64 0, i32 2, i32 6, i64 0, i32 2
  %i200 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i199, i32 0)
  %i201 = load i64, ptr %i200, align 1
  %i202 = shl i64 %i187, 32
  %i203 = ashr exact i64 %i202, 32
  %i204 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i201, i64 %i198, ptr elementtype(float) %i195, i64 %i203)
  %i205 = load float, ptr %i204, align 1
  store i8 26, ptr %i128, align 1
  store i8 1, ptr %i129, align 1
  store i8 1, ptr %i130, align 1
  store i8 0, ptr %i131, align 1
  store float %i205, ptr %i132, align 8
  %i206 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i24, i32 -1, i64 1239157112576, ptr nonnull %i128, ptr nonnull %i133) #3
  %i207 = add nuw i64 %i135, 1
  %i208 = trunc i64 %i207 to i32
  %i209 = icmp slt i32 10, %i208
  br i1 %i209, label %bb210, label %bb134

bb210:                                            ; preds = %bb190
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr #1

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #0 {
bb:
  %i = alloca i32, align 8
  %i1 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.5adb142a4af92269a23dd8f105f60717.0) #3
  store i32 1, ptr %i, align 8
  br label %bb2

bb2:                                              ; preds = %bb2, %bb
  %i3 = phi i32 [ %i4, %bb2 ], [ 1, %bb ]
  call void @arr_mod_mp_allocate_arr_(ptr nonnull %i)
  call void @arr_mod_mp_initialize_arr_(ptr nonnull %i, ptr nonnull @anon.5adb142a4af92269a23dd8f105f60717.1, ptr nonnull @anon.5adb142a4af92269a23dd8f105f60717.1, ptr nonnull @anon.5adb142a4af92269a23dd8f105f60717.1)
  call void @arr_mod_mp_print_arr_(ptr nonnull %i, ptr nonnull @anon.5adb142a4af92269a23dd8f105f60717.1, ptr nonnull @anon.5adb142a4af92269a23dd8f105f60717.1, ptr nonnull @anon.5adb142a4af92269a23dd8f105f60717.1)
  %i4 = add nuw nsw i32 %i3, 1
  store i32 %i4, ptr %i, align 8
  %i5 = icmp eq i32 %i4, 11
  br i1 %i5, label %bb6, label %bb2

bb6:                                              ; preds = %bb2
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #2

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nounwind readnone speculatable }
attributes #3 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}

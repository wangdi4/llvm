; RUN: opt -opaque-pointers < %s -passes=dopevectorconstprop -dope-vector-global-const-prop=true -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s

; This test case checks that the fields for the global dope vector
; @arr_mod_mp_a_ were collected and propagated correctly. Also, it
; identifies, collects and propagates the nested dope vectors. This
; is the same test case as global_dvcp24.ll, but it checks the IR.
; It was created from the following source code:

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

; The test case basically allocates the global array A in ALLOCATE_ARR, then
; initializes it in INITIALIZE_ARR and the use will be in PRINT_ARR. The
; function that allocates the array A should also allocate the information
; for inner_array_A, inner_array_B, inner_array_C.

; Check that the constants were propagated in function @arr_mod_mp_allocate_arr_
; CHECK: define internal void @arr_mod_mp_allocate_arr_
; CHECK: %i38 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i35, i64 %i37)
; CHECK: %i59 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i57, i64 %i37)
; CHECK: %i83 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i81, i64 %i37)

; Check that the constants were propagated in function @arr_mod_mp_initialize_arr_
; CHECK: define internal void @arr_mod_mp_initialize_arr_
; CHECK: %i14 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i12, i64 %i5)
; CHECK: %i29 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i17, i64 %i7)
; CHECK: %i30 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i29, i64 %i11)
; CHECK: %i35 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i33, i64 %i5)
; CHECK: %i54 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 400, ptr elementtype(float) %i38, i64 %i32)
; CHECK: %i55 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i54, i64 %i7)
; CHECK: %i56 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i55, i64 %i11)
; CHECK: %i65 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i63, i64 %i5)
; CHECK: %i76 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i68, i64 11)

; Check that the constants were propagated in function @arr_mod_mp_print_arr_
; CHECK: define internal void @arr_mod_mp_print_arr_
; CHECK: %i38 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i36, i64 %i12)
; CHECK: %i53 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i41, i64 %i33)
; CHECK: %i54 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i53, i64 %i35)
; CHECK: %i61 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i59, i64 %i12)
; CHECK: %i80 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 400, ptr elementtype(float) %i64, i64 %i58)
; CHECK: %i81 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i80, i64 %i33)
; CHECK: %i82 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i81, i64 %i35)
; CHECK: %i95 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i93, i64 %i12)
; CHECK: %i108 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i98, i64 %i107)



target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"ARR_MOD$.btT_TESTTYPE" = type { %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank1$" }
%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank3$" = type { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@arr_mod_mp_a_ = internal global %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$" { ptr null, i64 0, i64 0, i64 1342177408, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
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
  %i12 = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %i, i32 2, i64 1, i64 288) #3
  %i13 = load i64, ptr %i, align 8
  %i14 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
  %i15 = and i64 %i14, -68451041281
  %i16 = or i64 %i15, 1342177280
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
  %i29 = or i32 %i28, 327680
  %i30 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 5), align 8
  %i31 = inttoptr i64 %i30 to ptr
  %i32 = tail call i32 @for_alloc_allocatable_handle(i64 %i13, ptr @arr_mod_mp_a_, i32 %i29, ptr %i31) #3
  br label %bb33

bb33:                                             ; preds = %bb5, %bb3
  %i34 = phi ptr [ %i4, %bb3 ], [ %i9, %bb5 ]
  %i35 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i36 = load i64, ptr %i34, align 1
  %i37 = sext i32 %i1 to i64
  %i38 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i36, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i35, i64 %i37)
  %i39 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0
  %i40 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i39, i64 0, i32 3
  %i41 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i39, i64 0, i32 5
  store i64 0, ptr %i41, align 1
  %i42 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i39, i64 0, i32 1
  store i64 4, ptr %i42, align 1
  %i43 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i39, i64 0, i32 4
  store i64 2, ptr %i43, align 1
  %i44 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i39, i64 0, i32 2
  store i64 0, ptr %i44, align 1
  %i45 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i39, i64 0, i32 6, i64 0
  %i46 = getelementptr inbounds { i64, i64, i64 }, ptr %i45, i64 0, i32 2
  %i47 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i46, i32 0)
  store i64 1, ptr %i47, align 1
  %i48 = getelementptr inbounds { i64, i64, i64 }, ptr %i45, i64 0, i32 0
  %i49 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i48, i32 0)
  store i64 10, ptr %i49, align 1
  %i50 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i46, i32 1)
  store i64 1, ptr %i50, align 1
  %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i48, i32 1)
  store i64 10, ptr %i51, align 1
  %i52 = getelementptr inbounds { i64, i64, i64 }, ptr %i45, i64 0, i32 1
  %i53 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i52, i32 0)
  store i64 4, ptr %i53, align 1
  %i54 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i52, i32 1)
  store i64 40, ptr %i54, align 1
  store i64 1342177285, ptr %i40, align 1
  %i56 = tail call i32 @for_allocate_handle(i64 400, ptr %i39, i32 327680, ptr null) #3
  %i57 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i58 = load i64, ptr %i34, align 1
  %i59 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i58, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i57, i64 %i37)
  %i60 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i59, i64 0, i32 1
  %i61 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %i60, i64 0, i32 3
  %i62 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %i60, i64 0, i32 5
  store i64 0, ptr %i62, align 1
  %i63 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %i60, i64 0, i32 1
  store i64 4, ptr %i63, align 1
  %i64 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %i60, i64 0, i32 4
  store i64 3, ptr %i64, align 1
  %i65 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %i60, i64 0, i32 2
  store i64 0, ptr %i65, align 1
  %i66 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %i60, i64 0, i32 6, i64 0
  %i67 = getelementptr inbounds { i64, i64, i64 }, ptr %i66, i64 0, i32 2
  %i68 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i67, i32 0)
  store i64 1, ptr %i68, align 1
  %i69 = getelementptr inbounds { i64, i64, i64 }, ptr %i66, i64 0, i32 0
  %i70 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i69, i32 0)
  store i64 10, ptr %i70, align 1
  %i71 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i67, i32 1)
  store i64 1, ptr %i71, align 1
  %i72 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i69, i32 1)
  store i64 10, ptr %i72, align 1
  %i73 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i67, i32 2)
  store i64 1, ptr %i73, align 1
  %i74 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i69, i32 2)
  store i64 10, ptr %i74, align 1
  %i75 = getelementptr inbounds { i64, i64, i64 }, ptr %i66, i64 0, i32 1
  %i76 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i75, i32 0)
  store i64 4, ptr %i76, align 1
  %i77 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i75, i32 1)
  store i64 40, ptr %i77, align 1
  %i78 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i75, i32 2)
  store i64 400, ptr %i78, align 1
  store i64 1342177285, ptr %i61, align 1
  %i79 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i60, i64 0, i32 0
  %i80 = tail call i32 @for_allocate_handle(i64 4000, ptr nonnull %i79, i32 327680, ptr null) #3
  %i81 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i82 = load i64, ptr %i34, align 1
  %i83 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i82, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i81, i64 %i37)
  %i84 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i83, i64 0, i32 2
  %i85 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %i84, i64 0, i32 3
  %i86 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %i84, i64 0, i32 5
  store i64 0, ptr %i86, align 1
  %i87 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %i84, i64 0, i32 1
  store i64 4, ptr %i87, align 1
  %i88 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %i84, i64 0, i32 4
  store i64 1, ptr %i88, align 1
  %i89 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %i84, i64 0, i32 2
  store i64 0, ptr %i89, align 1
  %i90 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %i84, i64 0, i32 6, i64 0
  %i91 = getelementptr inbounds { i64, i64, i64 }, ptr %i90, i64 0, i32 2
  %i92 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i91, i32 0)
  store i64 1, ptr %i92, align 1
  %i93 = getelementptr inbounds { i64, i64, i64 }, ptr %i90, i64 0, i32 0
  %i94 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i93, i32 0)
  store i64 10, ptr %i94, align 1
  %i95 = getelementptr inbounds { i64, i64, i64 }, ptr %i90, i64 0, i32 1
  %i96 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i95, i32 0)
  store i64 4, ptr %i96, align 1
  store i64 1342177285, ptr %i85, align 1
  %i97 = bitcast ptr %i84 to ptr
  %i98 = tail call i32 @for_allocate_handle(i64 40, ptr nonnull %i97, i32 327680, ptr null) #3
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

bb6:                                              ; preds = %bb62, %bb
  %i7 = phi i64 [ 1, %bb ], [ %i77, %bb62 ]
  %i8 = trunc i64 %i7 to i32
  %i9 = sitofp i32 %i8 to float
  br label %bb10

bb10:                                             ; preds = %bb59, %bb6
  %i11 = phi i64 [ 1, %bb6 ], [ %i60, %bb59 ]
  %i12 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i13 = load i64, ptr %i, align 1
  %i14 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i13, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i12, i64 %i5)
  %i15 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i14, i64 0, i32 0
  %i16 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i15, i64 0, i32 0
  %i17 = load ptr, ptr %i16, align 1
  %i18 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i15, i64 0, i32 6, i64 0
  %i19 = getelementptr inbounds { i64, i64, i64 }, ptr %i18, i64 0, i32 1
  %i20 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i19, i32 0)
  %i21 = load i64, ptr %i20, align 1
  %i22 = getelementptr inbounds { i64, i64, i64 }, ptr %i18, i64 0, i32 2
  %i23 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i22, i32 0)
  %i24 = load i64, ptr %i23, align 1
  %i25 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i19, i32 1)
  %i26 = load i64, ptr %i25, align 1
  %i27 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i22, i32 1)
  %i28 = load i64, ptr %i27, align 1
  %i29 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i28, i64 %i26, ptr elementtype(float) %i17, i64 %i7)
  %i30 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i24, i64 %i21, ptr elementtype(float) %i29, i64 %i11)
  store float %i9, ptr %i30, align 1
  br label %bb31

bb31:                                             ; preds = %bb31, %bb10
  %i32 = phi i64 [ %i57, %bb31 ], [ 1, %bb10 ]
  %i33 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i34 = load i64, ptr %i, align 1
  %i35 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i34, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i33, i64 %i5)
  %i36 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i35, i64 0, i32 1
  %i37 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %i36, i64 0, i32 0
  %i38 = load ptr, ptr %i37, align 1
  %i39 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %i36, i64 0, i32 6, i64 0
  %i40 = getelementptr inbounds { i64, i64, i64 }, ptr %i39, i64 0, i32 1
  %i41 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i40, i32 0)
  %i42 = load i64, ptr %i41, align 1
  %i43 = getelementptr inbounds { i64, i64, i64 }, ptr %i39, i64 0, i32 2
  %i44 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i43, i32 0)
  %i45 = load i64, ptr %i44, align 1
  %i46 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i40, i32 1)
  %i47 = load i64, ptr %i46, align 1
  %i48 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i43, i32 1)
  %i49 = load i64, ptr %i48, align 1
  %i50 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i40, i32 2)
  %i51 = load i64, ptr %i50, align 1
  %i52 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i43, i32 2)
  %i53 = load i64, ptr %i52, align 1
  %i54 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %i53, i64 %i51, ptr elementtype(float) %i38, i64 %i32)
  %i55 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i49, i64 %i47, ptr elementtype(float) %i54, i64 %i7)
  %i56 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i45, i64 %i42, ptr elementtype(float) %i55, i64 %i11)
  store float %i9, ptr %i56, align 1
  %i57 = add nuw nsw i64 %i32, 1
  %i58 = icmp eq i64 %i57, 11
  br i1 %i58, label %bb59, label %bb31

bb59:                                             ; preds = %bb31
  %i60 = add nuw nsw i64 %i11, 1
  %i61 = icmp eq i64 %i60, 11
  br i1 %i61, label %bb62, label %bb10

bb62:                                             ; preds = %bb59
  %i63 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i64 = load i64, ptr %i, align 1
  %i65 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i64, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i63, i64 %i5)
  %i66 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i65, i64 0, i32 2
  %i67 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %i66, i64 0, i32 0
  %i68 = load ptr, ptr %i67, align 1
  %i69 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %i66, i64 0, i32 6, i64 0
  %i70 = getelementptr inbounds { i64, i64, i64 }, ptr %i69, i64 0, i32 1
  %i71 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i70, i32 0)
  %i72 = load i64, ptr %i71, align 1
  %i73 = getelementptr inbounds { i64, i64, i64 }, ptr %i69, i64 0, i32 2
  %i74 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i73, i32 0)
  %i75 = load i64, ptr %i74, align 1
  %i76 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i75, i64 %i72, ptr elementtype(float) %i68, i64 11)
  store float 1.100000e+01, ptr %i76, align 1
  %i77 = add nuw nsw i64 %i7, 1
  %i78 = icmp eq i64 %i77, 11
  br i1 %i78, label %bb79, label %bb6

bb79:                                             ; preds = %bb62
  ret void
}

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_print_arr_(ptr noalias nocapture readonly dereferenceable(4) %arg, ptr noalias nocapture readonly dereferenceable(4) %arg1, ptr noalias nocapture readonly dereferenceable(4) %arg2, ptr noalias nocapture readonly dereferenceable(4) %arg3) #0 {
bb:
  %i = alloca [8 x i64], align 32
  %i4 = alloca [4 x i8], align 1
  %i5 = alloca { float }, align 8
  %i6 = alloca [4 x i8], align 1
  %i7 = alloca { float }, align 8
  %i8 = alloca [4 x i8], align 1
  %i9 = alloca { float }, align 8
  %i10 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %i11 = load i32, ptr %arg, align 1
  %i12 = sext i32 %i11 to i64
  %i13 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 0
  %i14 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 1
  %i15 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 2
  %i16 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 3
  %i17 = getelementptr inbounds { float }, ptr %i5, i64 0, i32 0
  %i18 = bitcast ptr %i to ptr
  %i19 = bitcast ptr %i5 to ptr
  %i20 = getelementptr inbounds [4 x i8], ptr %i6, i64 0, i64 0
  %i21 = getelementptr inbounds [4 x i8], ptr %i6, i64 0, i64 1
  %i22 = getelementptr inbounds [4 x i8], ptr %i6, i64 0, i64 2
  %i23 = getelementptr inbounds [4 x i8], ptr %i6, i64 0, i64 3
  %i24 = getelementptr inbounds { float }, ptr %i7, i64 0, i32 0
  %i25 = bitcast ptr %i7 to ptr
  %i26 = getelementptr inbounds [4 x i8], ptr %i8, i64 0, i64 0
  %i27 = getelementptr inbounds [4 x i8], ptr %i8, i64 0, i64 1
  %i28 = getelementptr inbounds [4 x i8], ptr %i8, i64 0, i64 2
  %i29 = getelementptr inbounds [4 x i8], ptr %i8, i64 0, i64 3
  %i30 = getelementptr inbounds { float }, ptr %i9, i64 0, i32 0
  %i31 = bitcast ptr %i9 to ptr
  br label %bb32

bb32:                                             ; preds = %bb92, %bb
  %i33 = phi i64 [ 1, %bb ], [ %i111, %bb92 ]
  br label %bb34

bb34:                                             ; preds = %bb88, %bb32
  %i35 = phi i64 [ %i89, %bb88 ], [ 1, %bb32 ]
  %i36 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i37 = load i64, ptr %i10, align 1
  %i38 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i37, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i36, i64 %i12)
  %i39 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0
  %i40 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i39, i64 0, i32 0
  %i41 = load ptr, ptr %i40, align 1
  %i42 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i39, i64 0, i32 6, i64 0
  %i43 = getelementptr inbounds { i64, i64, i64 }, ptr %i42, i64 0, i32 1
  %i44 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i43, i32 0)
  %i45 = load i64, ptr %i44, align 1
  %i46 = getelementptr inbounds { i64, i64, i64 }, ptr %i42, i64 0, i32 2
  %i47 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i46, i32 0)
  %i48 = load i64, ptr %i47, align 1
  %i49 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i43, i32 1)
  %i50 = load i64, ptr %i49, align 1
  %i51 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i46, i32 1)
  %i52 = load i64, ptr %i51, align 1
  %i53 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i52, i64 %i50, ptr elementtype(float) %i41, i64 %i33)
  %i54 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i48, i64 %i45, ptr elementtype(float) %i53, i64 %i35)
  %i55 = load float, ptr %i54, align 1
  store i8 26, ptr %i13, align 1
  store i8 1, ptr %i14, align 1
  store i8 1, ptr %i15, align 1
  store i8 0, ptr %i16, align 1
  store float %i55, ptr %i17, align 8
  %i56 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i18, i32 -1, i64 1239157112576, ptr nonnull %i13, ptr nonnull %i19) #3
  br label %bb57

bb57:                                             ; preds = %bb57, %bb34
  %i58 = phi i64 [ %i85, %bb57 ], [ 1, %bb34 ]
  %i59 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i60 = load i64, ptr %i10, align 1
  %i61 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i60, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i59, i64 %i12)
  %i62 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i61, i64 0, i32 1
  %i63 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %i62, i64 0, i32 0
  %i64 = load ptr, ptr %i63, align 1
  %i65 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %i62, i64 0, i32 6, i64 0
  %i66 = getelementptr inbounds { i64, i64, i64 }, ptr %i65, i64 0, i32 1
  %i67 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i66, i32 0)
  %i68 = load i64, ptr %i67, align 1
  %i69 = getelementptr inbounds { i64, i64, i64 }, ptr %i65, i64 0, i32 2
  %i70 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i69, i32 0)
  %i71 = load i64, ptr %i70, align 1
  %i72 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i66, i32 1)
  %i73 = load i64, ptr %i72, align 1
  %i74 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i69, i32 1)
  %i75 = load i64, ptr %i74, align 1
  %i76 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i66, i32 2)
  %i77 = load i64, ptr %i76, align 1
  %i78 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i69, i32 2)
  %i79 = load i64, ptr %i78, align 1
  %i80 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %i79, i64 %i77, ptr elementtype(float) %i64, i64 %i58)
  %i81 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i75, i64 %i73, ptr elementtype(float) %i80, i64 %i33)
  %i82 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i71, i64 %i68, ptr elementtype(float) %i81, i64 %i35)
  %i83 = load float, ptr %i82, align 1
  store i8 26, ptr %i20, align 1
  store i8 1, ptr %i21, align 1
  store i8 1, ptr %i22, align 1
  store i8 0, ptr %i23, align 1
  store float %i83, ptr %i24, align 8
  %i84 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i18, i32 -1, i64 1239157112576, ptr nonnull %i20, ptr nonnull %i25) #3
  %i85 = add nuw i64 %i58, 1
  %i86 = trunc i64 %i85 to i32
  %i87 = icmp slt i32 10, %i86
  br i1 %i87, label %bb88, label %bb57

bb88:                                             ; preds = %bb57
  %i89 = add nuw i64 %i35, 1
  %i90 = trunc i64 %i89 to i32
  %i91 = icmp slt i32 10, %i90
  br i1 %i91, label %bb92, label %bb34

bb92:                                             ; preds = %bb88
  %i93 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i94 = load i64, ptr %i10, align 1
  %i95 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i94, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i93, i64 %i12)
  %i96 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i95, i64 0, i32 2
  %i97 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %i96, i64 0, i32 0
  %i98 = load ptr, ptr %i97, align 1
  %i99 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %i96, i64 0, i32 6, i64 0
  %i100 = getelementptr inbounds { i64, i64, i64 }, ptr %i99, i64 0, i32 1
  %i101 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i100, i32 0)
  %i102 = load i64, ptr %i101, align 1
  %i103 = getelementptr inbounds { i64, i64, i64 }, ptr %i99, i64 0, i32 2
  %i104 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i103, i32 0)
  %i105 = load i64, ptr %i104, align 1
  %i106 = shl i64 %i89, 32
  %i107 = ashr exact i64 %i106, 32
  %i108 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i105, i64 %i102, ptr elementtype(float) %i98, i64 %i107)
  %i109 = load float, ptr %i108, align 1
  store i8 26, ptr %i26, align 1
  store i8 1, ptr %i27, align 1
  store i8 1, ptr %i28, align 1
  store i8 0, ptr %i29, align 1
  store float %i109, ptr %i30, align 8
  %i110 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i18, i32 -1, i64 1239157112576, ptr nonnull %i26, ptr nonnull %i31) #3
  %i111 = add nuw i64 %i33, 1
  %i112 = trunc i64 %i111 to i32
  %i113 = icmp slt i32 10, %i112
  br i1 %i113, label %bb114, label %bb32

bb114:                                            ; preds = %bb92
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr #1

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #0 {
bb:
  %i = alloca i32, align 8
  %i1 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.87529b4ebf98830a9107fed24e462e82.0) #3
  store i32 1, ptr %i, align 8
  br label %bb2

bb2:                                              ; preds = %bb2, %bb
  %i3 = phi i32 [ %i4, %bb2 ], [ 1, %bb ]
  call void @arr_mod_mp_allocate_arr_(ptr nonnull %i)
  call void @arr_mod_mp_initialize_arr_(ptr nonnull %i, ptr nonnull @anon.87529b4ebf98830a9107fed24e462e82.1, ptr nonnull @anon.87529b4ebf98830a9107fed24e462e82.1, ptr nonnull @anon.87529b4ebf98830a9107fed24e462e82.1)
  call void @arr_mod_mp_print_arr_(ptr nonnull %i, ptr nonnull @anon.87529b4ebf98830a9107fed24e462e82.1, ptr nonnull @anon.87529b4ebf98830a9107fed24e462e82.1, ptr nonnull @anon.87529b4ebf98830a9107fed24e462e82.1)
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

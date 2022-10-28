; RUN: opt -opaque-pointers < %s -passes=dopevectorconstprop -dope-vector-global-const-prop=true -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s

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
; CHECK:   %i42 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i39, i64 %i41)
; CHECK:   %i76 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i74, i64 %i41)
; CHECK:   %i110 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i108, i64 %i41)

; Check that the constants were propagated in function @arr_mod_mp_initialize_arr_
; CHECK: define internal void @arr_mod_mp_initialize_arr_
; CHECK:   %i28 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i26, i64 %i10)
; CHECK:   %i41 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i38, ptr elementtype(float) %i30, i64 %i20)
; CHECK:   %i42 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i41, i64 %i25)
; CHECK:   %i47 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i45, i64 %i10)
; CHECK:   %i64 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %i61, ptr elementtype(float) %i49, i64 %i44)
; CHECK:   %i65 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i57, ptr elementtype(float) %i64, i64 %i20)
; CHECK:   %i66 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i65, i64 %i25)
; CHECK:   %i77 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i75, i64 %i10)
; CHECK:   %i87 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i79, i64 %i86)

; Check that the constants were propagated in function @arr_mod_mp_print_arr_
; CHECK: define internal void @arr_mod_mp_print_arr_
; CHECK:   %i45 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i43, i64 %i17)
; CHECK:   %i58 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i55, ptr elementtype(float) %i47, i64 %i40)
; CHECK:   %i59 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i58, i64 %i42)
; CHECK:   %i66 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i64, i64 %i17)
; CHECK:   %i83 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %i80, ptr elementtype(float) %i68, i64 %i63)
; CHECK:   %i84 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i76, ptr elementtype(float) %i83, i64 %i40)
; CHECK:   %i85 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i84, i64 %i42)
; CHECK:   %i99 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i97, i64 %i17)
; CHECK:   %i110 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i101, i64 %i109)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"ARR_MOD$.btT_TESTTYPE" = type { %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank1$" }
%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank3$" = type { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@arr_mod_mp_a_ = internal global %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$" { ptr null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@anon.87529b4ebf98830a9107fed24e462e82.0 = internal unnamed_addr constant i32 2

; Function Attrs: nofree noinline nounwind uwtable
define internal void @arr_mod_mp_allocate_arr_(ptr noalias nocapture readonly dereferenceable(4) %arg, ptr noalias nocapture readonly dereferenceable(4) %arg1) #0 {
bb:
  %i = alloca i64, align 8
  %i2 = alloca i64, align 8
  %i3 = alloca i64, align 8
  %i4 = alloca i64, align 8
  %i5 = load i32, ptr %arg, align 1
  %i6 = icmp eq i32 %i5, 1
  br i1 %i6, label %bb9, label %bb7

bb7:                                              ; preds = %bb
  %i8 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  br label %bb37

bb9:                                              ; preds = %bb
  %i10 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
  %i11 = and i64 %i10, 1030792151296
  %i12 = or i64 %i11, 133
  store i64 %i12, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 5), align 8
  store i64 288, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 1), align 8
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 4), align 16
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 2), align 16
  %i13 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %i13, align 1
  %i14 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 1, ptr %i14, align 1
  %i15 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 288, ptr %i15, align 1
  %i16 = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %i, i32 2, i64 1, i64 288) #3
  %i17 = load i64, ptr %i, align 8
  %i18 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
  %i19 = and i64 %i18, -68451041281
  %i20 = or i64 %i19, 1073741824
  store i64 %i20, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
  %i21 = trunc i64 %i18 to i32
  %i22 = shl i32 %i21, 1
  %i23 = and i32 %i22, 2
  %i24 = shl i32 %i16, 4
  %i25 = and i32 %i24, 16
  %i26 = lshr i64 %i18, 15
  %i27 = trunc i64 %i26 to i32
  %i28 = and i32 %i27, 31457280
  %i29 = and i32 %i27, 33554432
  %i30 = or i32 %i25, %i23
  %i31 = or i32 %i30, %i28
  %i32 = or i32 %i31, %i29
  %i33 = or i32 %i32, 262144
  %i34 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 5), align 8
  %i35 = inttoptr i64 %i34 to ptr
  %i36 = tail call i32 @for_alloc_allocatable_handle(i64 %i17, ptr @arr_mod_mp_a_, i32 %i33, ptr %i35) #3
  br label %bb37

bb37:                                             ; preds = %bb9, %bb7
  %i38 = phi ptr [ %i8, %bb7 ], [ %i13, %bb9 ]
  %i39 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i40 = load i64, ptr %i38, align 1
  %i41 = sext i32 %i5 to i64
  %i42 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i40, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i39, i64 %i41)
  %i43 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i42, i64 0, i32 0, i32 3
  store i64 5, ptr %i43, align 1
  %i44 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i42, i64 0, i32 0, i32 5
  store i64 0, ptr %i44, align 1
  %i45 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i42, i64 0, i32 0, i32 1
  store i64 4, ptr %i45, align 1
  %i46 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i42, i64 0, i32 0, i32 4
  store i64 2, ptr %i46, align 1
  %i47 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i42, i64 0, i32 0, i32 2
  store i64 0, ptr %i47, align 1
  %i48 = load i32, ptr %arg1, align 1
  %i49 = sext i32 %i48 to i64
  %i50 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i42, i64 0, i32 0, i32 6, i64 0, i32 2
  %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i50, i32 0)
  store i64 1, ptr %i51, align 1
  %i52 = icmp sgt i64 %i49, 0
  %i53 = select i1 %i52, i64 %i49, i64 0
  %i54 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i42, i64 0, i32 0, i32 6, i64 0, i32 0
  %i55 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i54, i32 0)
  store i64 %i53, ptr %i55, align 1
  %i56 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i50, i32 1)
  store i64 1, ptr %i56, align 1
  %i57 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i54, i32 1)
  store i64 %i53, ptr %i57, align 1
  %i58 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i42, i64 0, i32 0, i32 6, i64 0, i32 1
  %i59 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i58, i32 0)
  store i64 4, ptr %i59, align 1
  %i60 = shl nuw nsw i64 %i53, 2
  %i61 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i58, i32 1)
  store i64 %i60, ptr %i61, align 1
  %i62 = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %i2, i32 3, i64 %i53, i64 %i53, i64 4) #3
  %i63 = load i64, ptr %i2, align 8
  %i64 = load i64, ptr %i43, align 1
  %i65 = and i64 %i64, -68451041281
  %i66 = or i64 %i65, 1073741824
  store i64 %i66, ptr %i43, align 1
  %i67 = shl i32 %i62, 4
  %i68 = and i32 %i67, 16
  %i69 = or i32 %i68, 262144
  %i70 = load i64, ptr %i44, align 1
  %i71 = inttoptr i64 %i70 to ptr
  %i72 = bitcast ptr %i42 to ptr
  %i73 = tail call i32 @for_allocate_handle(i64 %i63, ptr %i72, i32 %i69, ptr %i71) #3
  %i74 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i75 = load i64, ptr %i38, align 1
  %i76 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i75, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i74, i64 %i41)
  %i77 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i76, i64 0, i32 1, i32 3
  store i64 5, ptr %i77, align 1
  %i78 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i76, i64 0, i32 1, i32 5
  store i64 0, ptr %i78, align 1
  %i79 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i76, i64 0, i32 1, i32 1
  store i64 4, ptr %i79, align 1
  %i80 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i76, i64 0, i32 1, i32 4
  store i64 3, ptr %i80, align 1
  %i81 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i76, i64 0, i32 1, i32 2
  store i64 0, ptr %i81, align 1
  %i82 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i76, i64 0, i32 1, i32 6, i64 0, i32 2
  %i83 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i82, i32 0)
  store i64 1, ptr %i83, align 1
  %i84 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i76, i64 0, i32 1, i32 6, i64 0, i32 0
  %i85 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i84, i32 0)
  store i64 %i53, ptr %i85, align 1
  %i86 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i82, i32 1)
  store i64 1, ptr %i86, align 1
  %i87 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i84, i32 1)
  store i64 %i53, ptr %i87, align 1
  %i88 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i82, i32 2)
  store i64 1, ptr %i88, align 1
  %i89 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i84, i32 2)
  store i64 %i53, ptr %i89, align 1
  %i90 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i76, i64 0, i32 1, i32 6, i64 0, i32 1
  %i91 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i90, i32 0)
  store i64 4, ptr %i91, align 1
  %i92 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i90, i32 1)
  store i64 %i60, ptr %i92, align 1
  %i93 = mul nuw nsw i64 %i60, %i53
  %i94 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i90, i32 2)
  store i64 %i93, ptr %i94, align 1
  %i95 = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %i3, i32 4, i64 %i53, i64 %i53, i64 %i53, i64 4) #3
  %i96 = load i64, ptr %i3, align 8
  %i97 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i76, i64 0, i32 1, i32 0
  %i98 = load i64, ptr %i77, align 1
  %i99 = and i64 %i98, -68451041281
  %i100 = or i64 %i99, 1073741824
  store i64 %i100, ptr %i77, align 1
  %i101 = shl i32 %i95, 4
  %i102 = and i32 %i101, 16
  %i103 = or i32 %i102, 262144
  %i104 = load i64, ptr %i78, align 1
  %i105 = inttoptr i64 %i104 to ptr
  %i106 = bitcast ptr %i97 to ptr
  %i107 = tail call i32 @for_allocate_handle(i64 %i96, ptr nonnull %i106, i32 %i103, ptr %i105) #3
  %i108 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i109 = load i64, ptr %i38, align 1
  %i110 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i109, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i108, i64 %i41)
  %i111 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i110, i64 0, i32 2, i32 3
  store i64 5, ptr %i111, align 1
  %i112 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i110, i64 0, i32 2, i32 5
  store i64 0, ptr %i112, align 1
  %i113 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i110, i64 0, i32 2, i32 1
  store i64 4, ptr %i113, align 1
  %i114 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i110, i64 0, i32 2, i32 4
  store i64 1, ptr %i114, align 1
  %i115 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i110, i64 0, i32 2, i32 2
  store i64 0, ptr %i115, align 1
  %i116 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i110, i64 0, i32 2, i32 6, i64 0, i32 2
  %i117 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i116, i32 0)
  store i64 1, ptr %i117, align 1
  %i118 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i110, i64 0, i32 2, i32 6, i64 0, i32 0
  %i119 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i118, i32 0)
  store i64 %i53, ptr %i119, align 1
  %i120 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i110, i64 0, i32 2, i32 6, i64 0, i32 1
  %i121 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i120, i32 0)
  store i64 4, ptr %i121, align 1
  %i122 = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %i4, i32 2, i64 %i53, i64 4) #3
  %i123 = load i64, ptr %i4, align 8
  %i124 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i110, i64 0, i32 2, i32 0
  %i125 = load i64, ptr %i111, align 1
  %i126 = and i64 %i125, -68451041281
  %i127 = or i64 %i126, 1073741824
  store i64 %i127, ptr %i111, align 1
  %i128 = shl i32 %i122, 4
  %i129 = and i32 %i128, 16
  %i130 = or i32 %i129, 262144
  %i131 = load i64, ptr %i112, align 1
  %i132 = inttoptr i64 %i131 to ptr
  %i133 = bitcast ptr %i124 to ptr
  %i134 = tail call i32 @for_allocate_handle(i64 %i123, ptr nonnull %i133, i32 %i130, ptr %i132) #3
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_check_mult_overflow64(ptr nocapture, i32, ...) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_alloc_allocatable_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: nofree noinline nounwind uwtable
define internal void @arr_mod_mp_initialize_arr_(ptr noalias nocapture readonly dereferenceable(4) %arg, ptr noalias nocapture readonly dereferenceable(4) %arg1, ptr noalias nocapture readonly dereferenceable(4) %arg2, ptr noalias nocapture readonly dereferenceable(4) %arg3) #0 {
bb:
  %i = load i32, ptr %arg1, align 1
  %i4 = icmp slt i32 %i, 1
  br i1 %i4, label %bb90, label %bb5

bb5:                                              ; preds = %bb
  %i6 = load i32, ptr %arg2, align 1
  %i7 = icmp slt i32 %i6, 1
  %i8 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %i9 = load i32, ptr %arg, align 1
  %i10 = sext i32 %i9 to i64
  %i11 = load i32, ptr %arg3, align 1
  %i12 = icmp slt i32 %i11, 1
  %i13 = add nuw nsw i32 %i11, 1
  %i14 = add i32 %i6, 1
  %i15 = add nuw nsw i32 %i, 1
  %i16 = sext i32 %i15 to i64
  %i17 = sext i32 %i14 to i64
  %i18 = sext i32 %i13 to i64
  br label %bb19

bb19:                                             ; preds = %bb72, %bb5
  %i20 = phi i64 [ 1, %bb5 ], [ %i88, %bb72 ]
  br i1 %i7, label %bb72, label %bb21

bb21:                                             ; preds = %bb19
  %i22 = trunc i64 %i20 to i32
  %i23 = sitofp i32 %i22 to float
  br label %bb24

bb24:                                             ; preds = %bb69, %bb21
  %i25 = phi i64 [ 1, %bb21 ], [ %i70, %bb69 ]
  %i26 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i27 = load i64, ptr %i8, align 1
  %i28 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i27, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i26, i64 %i10)
  %i29 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i28, i64 0, i32 0, i32 0
  %i30 = load ptr, ptr %i29, align 1
  %i31 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i28, i64 0, i32 0, i32 6, i64 0, i32 1
  %i32 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i31, i32 0)
  %i33 = load i64, ptr %i32, align 1
  %i34 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i28, i64 0, i32 0, i32 6, i64 0, i32 2
  %i35 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i34, i32 0)
  %i36 = load i64, ptr %i35, align 1
  %i37 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i31, i32 1)
  %i38 = load i64, ptr %i37, align 1
  %i39 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i34, i32 1)
  %i40 = load i64, ptr %i39, align 1
  %i41 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i40, i64 %i38, ptr elementtype(float) %i30, i64 %i20)
  %i42 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i36, i64 %i33, ptr elementtype(float) %i41, i64 %i25)
  store float %i23, ptr %i42, align 1
  br i1 %i12, label %bb69, label %bb43

bb43:                                             ; preds = %bb43, %bb24
  %i44 = phi i64 [ %i67, %bb43 ], [ 1, %bb24 ]
  %i45 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i46 = load i64, ptr %i8, align 1
  %i47 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i46, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i45, i64 %i10)
  %i48 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i47, i64 0, i32 1, i32 0
  %i49 = load ptr, ptr %i48, align 1
  %i50 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i47, i64 0, i32 1, i32 6, i64 0, i32 1
  %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i50, i32 0)
  %i52 = load i64, ptr %i51, align 1
  %i53 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i47, i64 0, i32 1, i32 6, i64 0, i32 2
  %i54 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i53, i32 0)
  %i55 = load i64, ptr %i54, align 1
  %i56 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i50, i32 1)
  %i57 = load i64, ptr %i56, align 1
  %i58 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i53, i32 1)
  %i59 = load i64, ptr %i58, align 1
  %i60 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i50, i32 2)
  %i61 = load i64, ptr %i60, align 1
  %i62 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i53, i32 2)
  %i63 = load i64, ptr %i62, align 1
  %i64 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %i63, i64 %i61, ptr elementtype(float) %i49, i64 %i44)
  %i65 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i59, i64 %i57, ptr elementtype(float) %i64, i64 %i20)
  %i66 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i55, i64 %i52, ptr elementtype(float) %i65, i64 %i25)
  store float %i23, ptr %i66, align 1
  %i67 = add nuw nsw i64 %i44, 1
  %i68 = icmp eq i64 %i67, %i18
  br i1 %i68, label %bb69, label %bb43

bb69:                                             ; preds = %bb43, %bb24
  %i70 = add nuw nsw i64 %i25, 1
  %i71 = icmp eq i64 %i70, %i17
  br i1 %i71, label %bb72, label %bb24

bb72:                                             ; preds = %bb69, %bb19
  %i73 = phi i32 [ 1, %bb19 ], [ %i14, %bb69 ]
  %i74 = sitofp i32 %i73 to float
  %i75 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i76 = load i64, ptr %i8, align 1
  %i77 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i76, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i75, i64 %i10)
  %i78 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i77, i64 0, i32 2, i32 0
  %i79 = load ptr, ptr %i78, align 1
  %i80 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i77, i64 0, i32 2, i32 6, i64 0, i32 1
  %i81 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i80, i32 0)
  %i82 = load i64, ptr %i81, align 1
  %i83 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i77, i64 0, i32 2, i32 6, i64 0, i32 2
  %i84 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i83, i32 0)
  %i85 = load i64, ptr %i84, align 1
  %i86 = sext i32 %i73 to i64
  %i87 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i85, i64 %i82, ptr elementtype(float) %i79, i64 %i86)
  store float %i74, ptr %i87, align 1
  %i88 = add nuw nsw i64 %i20, 1
  %i89 = icmp eq i64 %i88, %i16
  br i1 %i89, label %bb90, label %bb19

bb90:                                             ; preds = %bb72, %bb
  ret void
}

; Function Attrs: nofree noinline nounwind uwtable
define internal void @arr_mod_mp_print_arr_(ptr noalias nocapture readonly dereferenceable(4) %arg, ptr noalias nocapture readonly dereferenceable(4) %arg1, ptr noalias nocapture readonly dereferenceable(4) %arg2, ptr noalias nocapture readonly dereferenceable(4) %arg3) #0 {
bb:
  %i = alloca [8 x i64], align 16
  %i4 = alloca [4 x i8], align 1
  %i5 = alloca { float }, align 8
  %i6 = alloca [4 x i8], align 1
  %i7 = alloca { float }, align 8
  %i8 = alloca [4 x i8], align 1
  %i9 = alloca { float }, align 8
  %i10 = load i32, ptr %arg1, align 1
  %i11 = icmp slt i32 %i10, 1
  br i1 %i11, label %bb116, label %bb12

bb12:                                             ; preds = %bb
  %i13 = load i32, ptr %arg2, align 1
  %i14 = icmp slt i32 %i13, 1
  %i15 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %i16 = load i32, ptr %arg, align 1
  %i17 = sext i32 %i16 to i64
  %i18 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 0
  %i19 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 1
  %i20 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 2
  %i21 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 3
  %i22 = getelementptr inbounds { float }, ptr %i5, i64 0, i32 0
  %i23 = bitcast ptr %i to ptr
  %i24 = bitcast ptr %i5 to ptr
  %i25 = load i32, ptr %arg3, align 1
  %i26 = icmp slt i32 %i25, 1
  %i27 = getelementptr inbounds [4 x i8], ptr %i6, i64 0, i64 0
  %i28 = getelementptr inbounds [4 x i8], ptr %i6, i64 0, i64 1
  %i29 = getelementptr inbounds [4 x i8], ptr %i6, i64 0, i64 2
  %i30 = getelementptr inbounds [4 x i8], ptr %i6, i64 0, i64 3
  %i31 = getelementptr inbounds { float }, ptr %i7, i64 0, i32 0
  %i32 = bitcast ptr %i7 to ptr
  %i33 = getelementptr inbounds [4 x i8], ptr %i8, i64 0, i64 0
  %i34 = getelementptr inbounds [4 x i8], ptr %i8, i64 0, i64 1
  %i35 = getelementptr inbounds [4 x i8], ptr %i8, i64 0, i64 2
  %i36 = getelementptr inbounds [4 x i8], ptr %i8, i64 0, i64 3
  %i37 = getelementptr inbounds { float }, ptr %i9, i64 0, i32 0
  %i38 = bitcast ptr %i9 to ptr
  br label %bb39

bb39:                                             ; preds = %bb95, %bb12
  %i40 = phi i64 [ 1, %bb12 ], [ %i113, %bb95 ]
  br i1 %i14, label %bb95, label %bb41

bb41:                                             ; preds = %bb91, %bb39
  %i42 = phi i64 [ %i92, %bb91 ], [ 1, %bb39 ]
  %i43 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i44 = load i64, ptr %i15, align 1
  %i45 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i44, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i43, i64 %i17)
  %i46 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i45, i64 0, i32 0, i32 0
  %i47 = load ptr, ptr %i46, align 1
  %i48 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i45, i64 0, i32 0, i32 6, i64 0, i32 1
  %i49 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i48, i32 0)
  %i50 = load i64, ptr %i49, align 1
  %i51 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i45, i64 0, i32 0, i32 6, i64 0, i32 2
  %i52 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i51, i32 0)
  %i53 = load i64, ptr %i52, align 1
  %i54 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i48, i32 1)
  %i55 = load i64, ptr %i54, align 1
  %i56 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i51, i32 1)
  %i57 = load i64, ptr %i56, align 1
  %i58 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i57, i64 %i55, ptr elementtype(float) %i47, i64 %i40)
  %i59 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i53, i64 %i50, ptr elementtype(float) %i58, i64 %i42)
  %i60 = load float, ptr %i59, align 1
  store i8 26, ptr %i18, align 1
  store i8 1, ptr %i19, align 1
  store i8 1, ptr %i20, align 1
  store i8 0, ptr %i21, align 1
  store float %i60, ptr %i22, align 8
  %i61 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i23, i32 -1, i64 1239157112576, ptr nonnull %i18, ptr nonnull %i24) #3
  br i1 %i26, label %bb91, label %bb62

bb62:                                             ; preds = %bb62, %bb41
  %i63 = phi i64 [ %i88, %bb62 ], [ 1, %bb41 ]
  %i64 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i65 = load i64, ptr %i15, align 1
  %i66 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i65, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i64, i64 %i17)
  %i67 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i66, i64 0, i32 1, i32 0
  %i68 = load ptr, ptr %i67, align 1
  %i69 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i66, i64 0, i32 1, i32 6, i64 0, i32 1
  %i70 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i69, i32 0)
  %i71 = load i64, ptr %i70, align 1
  %i72 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i66, i64 0, i32 1, i32 6, i64 0, i32 2
  %i73 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i72, i32 0)
  %i74 = load i64, ptr %i73, align 1
  %i75 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i69, i32 1)
  %i76 = load i64, ptr %i75, align 1
  %i77 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i72, i32 1)
  %i78 = load i64, ptr %i77, align 1
  %i79 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i69, i32 2)
  %i80 = load i64, ptr %i79, align 1
  %i81 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i72, i32 2)
  %i82 = load i64, ptr %i81, align 1
  %i83 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %i82, i64 %i80, ptr elementtype(float) %i68, i64 %i63)
  %i84 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i78, i64 %i76, ptr elementtype(float) %i83, i64 %i40)
  %i85 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i74, i64 %i71, ptr elementtype(float) %i84, i64 %i42)
  %i86 = load float, ptr %i85, align 1
  store i8 26, ptr %i27, align 1
  store i8 1, ptr %i28, align 1
  store i8 1, ptr %i29, align 1
  store i8 0, ptr %i30, align 1
  store float %i86, ptr %i31, align 8
  %i87 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i23, i32 -1, i64 1239157112576, ptr nonnull %i27, ptr nonnull %i32) #3
  %i88 = add nuw i64 %i63, 1
  %i89 = trunc i64 %i88 to i32
  %i90 = icmp slt i32 %i25, %i89
  br i1 %i90, label %bb91, label %bb62

bb91:                                             ; preds = %bb62, %bb41
  %i92 = add nuw i64 %i42, 1
  %i93 = trunc i64 %i92 to i32
  %i94 = icmp slt i32 %i13, %i93
  br i1 %i94, label %bb95, label %bb41

bb95:                                             ; preds = %bb91, %bb39
  %i96 = phi i64 [ 1, %bb39 ], [ %i92, %bb91 ]
  %i97 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i98 = load i64, ptr %i15, align 1
  %i99 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i98, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i97, i64 %i17)
  %i100 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i99, i64 0, i32 2, i32 0
  %i101 = load ptr, ptr %i100, align 1
  %i102 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i99, i64 0, i32 2, i32 6, i64 0, i32 1
  %i103 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i102, i32 0)
  %i104 = load i64, ptr %i103, align 1
  %i105 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i99, i64 0, i32 2, i32 6, i64 0, i32 2
  %i106 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i105, i32 0)
  %i107 = load i64, ptr %i106, align 1
  %i108 = shl i64 %i96, 32
  %i109 = ashr exact i64 %i108, 32
  %i110 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i107, i64 %i104, ptr elementtype(float) %i101, i64 %i109)
  %i111 = load float, ptr %i110, align 1
  store i8 26, ptr %i33, align 1
  store i8 1, ptr %i34, align 1
  store i8 1, ptr %i35, align 1
  store i8 0, ptr %i36, align 1
  store float %i111, ptr %i37, align 8
  %i112 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i23, i32 -1, i64 1239157112576, ptr nonnull %i33, ptr nonnull %i38) #3
  %i113 = add nuw i64 %i40, 1
  %i114 = trunc i64 %i113 to i32
  %i115 = icmp slt i32 %i10, %i114
  br i1 %i115, label %bb116, label %bb39

bb116:                                            ; preds = %bb95, %bb
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr #1

; Function Attrs: nofree noinline nounwind uwtable
define dso_local void @MAIN__() #0 {
bb:
  %i = alloca [8 x i64], align 16
  %i1 = alloca i32, align 8
  %i2 = alloca i32, align 8
  %i3 = alloca [4 x i8], align 1
  %i4 = alloca { ptr }, align 8
  %i5 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.87529b4ebf98830a9107fed24e462e82.0) #3
  %i6 = getelementptr inbounds [4 x i8], ptr %i3, i64 0, i64 0
  store i8 9, ptr %i6, align 1
  %i7 = getelementptr inbounds [4 x i8], ptr %i3, i64 0, i64 1
  store i8 3, ptr %i7, align 1
  %i8 = getelementptr inbounds [4 x i8], ptr %i3, i64 0, i64 2
  store i8 1, ptr %i8, align 1
  %i9 = getelementptr inbounds [4 x i8], ptr %i3, i64 0, i64 3
  store i8 0, ptr %i9, align 1
  %i10 = bitcast ptr %i4 to ptr
  store ptr %i1, ptr %i10, align 8
  %i11 = bitcast ptr %i to ptr
  %i12 = bitcast ptr %i4 to ptr
  %i13 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_read_seq_lis(ptr nonnull %i11, i32 -4, i64 1239157112576, ptr nonnull %i6, ptr nonnull %i12) #3
  store i32 1, ptr %i2, align 8
  br label %bb14

bb14:                                             ; preds = %bb14, %bb
  %i15 = phi i32 [ %i16, %bb14 ], [ 1, %bb ]
  call void @arr_mod_mp_allocate_arr_(ptr nonnull %i2, ptr nonnull %i1)
  call void @arr_mod_mp_initialize_arr_(ptr nonnull %i2, ptr nonnull %i1, ptr nonnull %i1, ptr nonnull %i1)
  call void @arr_mod_mp_print_arr_(ptr nonnull %i2, ptr nonnull %i1, ptr nonnull %i1, ptr nonnull %i1)
  %i16 = add nuw nsw i32 %i15, 1
  store i32 %i16, ptr %i2, align 8
  %i17 = icmp eq i32 %i16, 11
  br i1 %i17, label %bb18, label %bb14

bb18:                                             ; preds = %bb14
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_read_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr #1

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #2

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

attributes #0 = { nofree noinline nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nounwind readnone speculatable }
attributes #3 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}

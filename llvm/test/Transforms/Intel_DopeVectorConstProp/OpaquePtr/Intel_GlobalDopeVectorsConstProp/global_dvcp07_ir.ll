; RUN: opt -opaque-pointers < %s -passes=dopevectorconstprop -dope-vector-global-const-prop=true -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s

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

; Check function @arr_mod_mp_allocate_arr_
; CHECK: define internal void @arr_mod_mp_allocate_arr_
; CHECK:   %i38 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i35, i64 %i37)
; CHECK:   %i57 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i55, i64 %i37)
; CHECK:   %i80 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i78, i64 %i37)

; Check function @arr_mod_mp_initialize_arr_
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

; Check function @arr_mod_mp_print_info_
; CHECK: define internal void @arr_mod_mp_print_info_
; CHECK:   %i55 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i14, i64 %i54)
; CHECK:   %i58 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i55, i64 %i57)
; CHECK:   %i63 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 400, ptr elementtype(float) %i28, i64 %i62)
; CHECK:   %i64 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i63, i64 %i54)
; CHECK:   %i65 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i64, i64 %i57)
; CHECK:   %i78 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i43, i64 %i77)

; Check that the pointers to the constant propagation happens in @MAIN__ when
; accessing the nested dope vectors, and the pointers to the nested dope
; vectors are passed to @arr_mod_mp_print_info_
; NOTE: The call to @arr_mod_mp_print_info_ happens in main due to inlining
; when building the IR for the test case, yet the concept is the same (nested
; dope vectors passed to a function).

; CHECK: define dso_local void @MAIN__()
; CHECK:   %i8 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i5, i64 %i7) #5
; CHECK:   %i9 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i8, i64 0, i32 1
; CHECK:   %i10 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i8, i64 0, i32 2
; CHECK:   %i11 = bitcast ptr %i8 to ptr
; CHECK:   tail call void @arr_mod_mp_print_info_(ptr undef, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, ptr %i11, ptr nonnull %i9, ptr nonnull %i10) #5, !noalias !9

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
  %i12 = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %i, i32 2, i64 1, i64 288) #5
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
  %i32 = tail call i32 @for_alloc_allocatable_handle(i64 %i13, ptr @arr_mod_mp_a_, i32 %i29, ptr %i31) #5
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
  %i54 = tail call i32 @for_allocate_handle(i64 400, ptr %i53, i32 262144, ptr null) #5
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
  %i77 = tail call i32 @for_allocate_handle(i64 4000, ptr nonnull %i76, i32 262144, ptr null) #5
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
  %i94 = tail call i32 @for_allocate_handle(i64 40, ptr nonnull %i93, i32 262144, ptr null) #5
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_check_mult_overflow64(ptr nocapture, i32, ...) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_alloc_allocatable_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: nofree nosync nounwind uwtable
define internal void @arr_mod_mp_initialize_arr_(ptr noalias nocapture readonly dereferenceable(4) %arg, ptr noalias nocapture readonly dereferenceable(4) %arg1, ptr noalias nocapture readonly dereferenceable(4) %arg2, ptr noalias nocapture readonly dereferenceable(4) %arg3) #2 {
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
  ret void
}

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_print_info_(ptr noalias nocapture readonly %arg, ptr noalias nocapture readonly dereferenceable(4) %arg1, ptr noalias nocapture readonly dereferenceable(4) %arg2, ptr noalias nocapture readonly dereferenceable(4) %arg3, ptr noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %arg4, ptr noalias nocapture readonly dereferenceable(120) "assumed_shape" "ptrnoalias" %arg5, ptr noalias nocapture readonly dereferenceable(72) "assumed_shape" "ptrnoalias" %arg6) #0 {
bb:
  %i = alloca [8 x i64], align 16
  %i7 = alloca [4 x i8], align 1
  %i8 = alloca { float }, align 8
  %i9 = alloca [4 x i8], align 1
  %i10 = alloca { float }, align 8
  %i11 = alloca [4 x i8], align 1
  %i12 = alloca { float }, align 8
  %i13 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %arg4, i64 0, i32 0
  %i14 = load ptr, ptr %i13, align 1
  %i15 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %arg4, i64 0, i32 6, i64 0, i32 1
  %i16 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i15, i32 0)
  %i17 = load i64, ptr %i16, align 1
  %i18 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i15, i32 1)
  %i19 = load i64, ptr %i18, align 1
  %i20 = getelementptr inbounds [4 x i8], ptr %i7, i64 0, i64 0
  %i21 = getelementptr inbounds [4 x i8], ptr %i7, i64 0, i64 1
  %i22 = getelementptr inbounds [4 x i8], ptr %i7, i64 0, i64 2
  %i23 = getelementptr inbounds [4 x i8], ptr %i7, i64 0, i64 3
  %i24 = getelementptr inbounds { float }, ptr %i8, i64 0, i32 0
  %i25 = bitcast ptr %i to ptr
  %i26 = bitcast ptr %i8 to ptr
  %i27 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %arg5, i64 0, i32 0
  %i28 = load ptr, ptr %i27, align 1
  %i29 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %arg5, i64 0, i32 6, i64 0, i32 1
  %i30 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i29, i32 0)
  %i31 = load i64, ptr %i30, align 1
  %i32 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i29, i32 1)
  %i33 = load i64, ptr %i32, align 1
  %i34 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i29, i32 2)
  %i35 = load i64, ptr %i34, align 1
  %i36 = getelementptr inbounds [4 x i8], ptr %i9, i64 0, i64 0
  %i37 = getelementptr inbounds [4 x i8], ptr %i9, i64 0, i64 1
  %i38 = getelementptr inbounds [4 x i8], ptr %i9, i64 0, i64 2
  %i39 = getelementptr inbounds [4 x i8], ptr %i9, i64 0, i64 3
  %i40 = getelementptr inbounds { float }, ptr %i10, i64 0, i32 0
  %i41 = bitcast ptr %i10 to ptr
  %i42 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %arg6, i64 0, i32 0
  %i43 = load ptr, ptr %i42, align 1
  %i44 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %arg6, i64 0, i32 6, i64 0, i32 1
  %i45 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i44, i32 0)
  %i46 = load i64, ptr %i45, align 1
  %i47 = getelementptr inbounds [4 x i8], ptr %i11, i64 0, i64 0
  %i48 = getelementptr inbounds [4 x i8], ptr %i11, i64 0, i64 1
  %i49 = getelementptr inbounds [4 x i8], ptr %i11, i64 0, i64 2
  %i50 = getelementptr inbounds [4 x i8], ptr %i11, i64 0, i64 3
  %i51 = getelementptr inbounds { float }, ptr %i12, i64 0, i32 0
  %i52 = bitcast ptr %i12 to ptr
  br label %bb53

bb53:                                             ; preds = %bb75, %bb
  %i54 = phi i64 [ 1, %bb ], [ %i81, %bb75 ]
  %i55 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i19, ptr elementtype(float) %i14, i64 %i54)
  br label %bb56

bb56:                                             ; preds = %bb71, %bb53
  %i57 = phi i64 [ 1, %bb53 ], [ %i72, %bb71 ]
  %i58 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i17, ptr elementtype(float) %i55, i64 %i57)
  %i59 = load float, ptr %i58, align 1
  store i8 26, ptr %i20, align 1
  store i8 1, ptr %i21, align 1
  store i8 1, ptr %i22, align 1
  store i8 0, ptr %i23, align 1
  store float %i59, ptr %i24, align 8
  %i60 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i25, i32 -1, i64 1239157112576, ptr nonnull %i20, ptr nonnull %i26) #5
  br label %bb61

bb61:                                             ; preds = %bb61, %bb56
  %i62 = phi i64 [ %i68, %bb61 ], [ 1, %bb56 ]
  %i63 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %i35, ptr elementtype(float) %i28, i64 %i62)
  %i64 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i33, ptr elementtype(float) %i63, i64 %i54)
  %i65 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i31, ptr elementtype(float) %i64, i64 %i57)
  %i66 = load float, ptr %i65, align 1
  store i8 26, ptr %i36, align 1
  store i8 1, ptr %i37, align 1
  store i8 1, ptr %i38, align 1
  store i8 0, ptr %i39, align 1
  store float %i66, ptr %i40, align 8
  %i67 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i25, i32 -1, i64 1239157112576, ptr nonnull %i36, ptr nonnull %i41) #5
  %i68 = add nuw i64 %i62, 1
  %i69 = trunc i64 %i68 to i32
  %i70 = icmp slt i32 10, %i69
  br i1 %i70, label %bb71, label %bb61

bb71:                                             ; preds = %bb61
  %i72 = add nuw i64 %i57, 1
  %i73 = trunc i64 %i72 to i32
  %i74 = icmp slt i32 10, %i73
  br i1 %i74, label %bb75, label %bb56

bb75:                                             ; preds = %bb71
  %i76 = shl i64 %i72, 32
  %i77 = ashr exact i64 %i76, 32
  %i78 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i46, ptr elementtype(float) %i43, i64 %i77)
  %i79 = load float, ptr %i78, align 1
  store i8 26, ptr %i47, align 1
  store i8 1, ptr %i48, align 1
  store i8 1, ptr %i49, align 1
  store i8 0, ptr %i50, align 1
  store float %i79, ptr %i51, align 8
  %i80 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i25, i32 -1, i64 1239157112576, ptr nonnull %i47, ptr nonnull %i52) #5
  %i81 = add nuw i64 %i54, 1
  %i82 = trunc i64 %i81 to i32
  %i83 = icmp slt i32 10, %i82
  br i1 %i83, label %bb84, label %bb53

bb84:                                             ; preds = %bb75
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr #1

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #0 {
bb:
  %i = alloca i32, align 8
  %i1 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.0) #5
  store i32 1, ptr %i, align 8
  %i2 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0) #5
  br label %bb3

bb3:                                              ; preds = %bb3, %bb
  %i4 = phi i32 [ %i12, %bb3 ], [ 1, %bb ]
  call void @arr_mod_mp_allocate_arr_(ptr nonnull %i)
  call void @arr_mod_mp_initialize_arr_(ptr nonnull %i, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1)
  %i5 = load ptr, ptr @arr_mod_mp_a_, align 16, !noalias !3
  %i6 = load i64, ptr %i2, align 1, !noalias !3
  %i7 = zext i32 %i4 to i64
  %i8 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i6, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i5, i64 %i7) #5
  %i9 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i8, i64 0, i32 1
  %i10 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i8, i64 0, i32 2
  %i11 = bitcast ptr %i8 to ptr
  tail call void @arr_mod_mp_print_info_(ptr undef, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, ptr %i11, ptr nonnull %i9, ptr nonnull %i10) #5, !noalias !9
  %i12 = add nuw nsw i32 %i4, 1
  store i32 %i12, ptr %i, align 8
  %i13 = icmp ult i32 %i4, 10
  br i1 %i13, label %bb3, label %bb14

bb14:                                             ; preds = %bb3
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

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nofree nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
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

; RUN: opt -opaque-pointers < %s -passes=dopevectorconstprop -dope-vector-global-const-prop=true -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s

; This test case checks that the fields for the global dope vector
; are invalid for constant collection since the global array is being copied.
; Also, due to the copy, the pointer to the global dope vector will be used
; in other functions that aren't related to data allocation. The
; transformation shouldn't happen. It was created from the following source
; code:

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
;         TYPE (T_TESTTYPE), ALLOCATABLE :: B (:)
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
;
;        B = A
;
;      END

; ifx -xCORE-AVX512 -Ofast -flto arr.f90 -mllvm -debug-only=dope-vector-global-const-prop

; The test case allocates, initializes and uses array global array A but then
; the data is copied to array B. It should invalidate the constant propagation.

; Check that constants weren't propagated
; CHECK: define internal void @arr_mod_mp_initialize_arr_
; CHECK: %i33 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i32, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i31, i64 %i5)
; CHECK: %i50 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %i49, i64 %i47, ptr elementtype(float) %i35, i64 %i30)
; CHECK: %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i45, i64 %i43, ptr elementtype(float) %i50, i64 %i7)
; CHECK: %i52 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i41, i64 %i38, ptr elementtype(float) %i51, i64 %i11)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"ARR_MOD$.btT_TESTTYPE" = type { %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank1$" }
%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank3$" = type { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@arr_mod_mp_a_ = internal global %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$" { ptr null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@anon.5adb142a4af92269a23dd8f105f60717.0 = internal unnamed_addr constant i32 2
@anon.5adb142a4af92269a23dd8f105f60717.1 = internal unnamed_addr constant i32 10
@arr_mod_mp_b_ = internal global %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$" { ptr null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }

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

bb32:                                             ; preds = %bb88, %bb
  %i33 = phi i64 [ 1, %bb ], [ %i105, %bb88 ]
  br label %bb34

bb34:                                             ; preds = %bb84, %bb32
  %i35 = phi i64 [ %i85, %bb84 ], [ 1, %bb32 ]
  %i36 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i37 = load i64, ptr %i10, align 1
  %i38 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i37, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i36, i64 %i12)
  %i39 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 0
  %i40 = load ptr, ptr %i39, align 1
  %i41 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 6, i64 0, i32 1
  %i42 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i41, i32 0)
  %i43 = load i64, ptr %i42, align 1
  %i44 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 6, i64 0, i32 2
  %i45 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i44, i32 0)
  %i46 = load i64, ptr %i45, align 1
  %i47 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i41, i32 1)
  %i48 = load i64, ptr %i47, align 1
  %i49 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i44, i32 1)
  %i50 = load i64, ptr %i49, align 1
  %i51 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i50, i64 %i48, ptr elementtype(float) %i40, i64 %i33)
  %i52 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i46, i64 %i43, ptr elementtype(float) %i51, i64 %i35)
  %i53 = load float, ptr %i52, align 1
  store i8 26, ptr %i13, align 1
  store i8 1, ptr %i14, align 1
  store i8 1, ptr %i15, align 1
  store i8 0, ptr %i16, align 1
  store float %i53, ptr %i17, align 8
  %i54 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i18, i32 -1, i64 1239157112576, ptr nonnull %i13, ptr nonnull %i19) #5
  br label %bb55

bb55:                                             ; preds = %bb55, %bb34
  %i56 = phi i64 [ %i81, %bb55 ], [ 1, %bb34 ]
  %i57 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i58 = load i64, ptr %i10, align 1
  %i59 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i58, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i57, i64 %i12)
  %i60 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i59, i64 0, i32 1, i32 0
  %i61 = load ptr, ptr %i60, align 1
  %i62 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i59, i64 0, i32 1, i32 6, i64 0, i32 1
  %i63 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i62, i32 0)
  %i64 = load i64, ptr %i63, align 1
  %i65 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i59, i64 0, i32 1, i32 6, i64 0, i32 2
  %i66 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i65, i32 0)
  %i67 = load i64, ptr %i66, align 1
  %i68 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i62, i32 1)
  %i69 = load i64, ptr %i68, align 1
  %i70 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i65, i32 1)
  %i71 = load i64, ptr %i70, align 1
  %i72 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i62, i32 2)
  %i73 = load i64, ptr %i72, align 1
  %i74 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i65, i32 2)
  %i75 = load i64, ptr %i74, align 1
  %i76 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %i75, i64 %i73, ptr elementtype(float) %i61, i64 %i56)
  %i77 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i71, i64 %i69, ptr elementtype(float) %i76, i64 %i33)
  %i78 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i67, i64 %i64, ptr elementtype(float) %i77, i64 %i35)
  %i79 = load float, ptr %i78, align 1
  store i8 26, ptr %i20, align 1
  store i8 1, ptr %i21, align 1
  store i8 1, ptr %i22, align 1
  store i8 0, ptr %i23, align 1
  store float %i79, ptr %i24, align 8
  %i80 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i18, i32 -1, i64 1239157112576, ptr nonnull %i20, ptr nonnull %i25) #5
  %i81 = add nuw i64 %i56, 1
  %i82 = trunc i64 %i81 to i32
  %i83 = icmp slt i32 10, %i82
  br i1 %i83, label %bb84, label %bb55

bb84:                                             ; preds = %bb55
  %i85 = add nuw i64 %i35, 1
  %i86 = trunc i64 %i85 to i32
  %i87 = icmp slt i32 10, %i86
  br i1 %i87, label %bb88, label %bb34

bb88:                                             ; preds = %bb84
  %i89 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i90 = load i64, ptr %i10, align 1
  %i91 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i90, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i89, i64 %i12)
  %i92 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i91, i64 0, i32 2, i32 0
  %i93 = load ptr, ptr %i92, align 1
  %i94 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i91, i64 0, i32 2, i32 6, i64 0, i32 1
  %i95 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i94, i32 0)
  %i96 = load i64, ptr %i95, align 1
  %i97 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i91, i64 0, i32 2, i32 6, i64 0, i32 2
  %i98 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i97, i32 0)
  %i99 = load i64, ptr %i98, align 1
  %i100 = shl i64 %i85, 32
  %i101 = ashr exact i64 %i100, 32
  %i102 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i99, i64 %i96, ptr elementtype(float) %i93, i64 %i101)
  %i103 = load float, ptr %i102, align 1
  store i8 26, ptr %i26, align 1
  store i8 1, ptr %i27, align 1
  store i8 1, ptr %i28, align 1
  store i8 0, ptr %i29, align 1
  store float %i103, ptr %i30, align 8
  %i104 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i18, i32 -1, i64 1239157112576, ptr nonnull %i26, ptr nonnull %i31) #5
  %i105 = add nuw i64 %i33, 1
  %i106 = trunc i64 %i105 to i32
  %i107 = icmp slt i32 10, %i106
  br i1 %i107, label %bb108, label %bb32

bb108:                                            ; preds = %bb88
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define dso_local void @MAIN__() #2 {
bb:
  %i = alloca i32, align 8
  %i1 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.5adb142a4af92269a23dd8f105f60717.0) #5
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
  %i7 = load ptr, ptr @arr_mod_mp_b_, align 16
  %i8 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
  %i9 = and i64 %i8, 1
  %i10 = icmp eq i64 %i9, 0
  %i11 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 3), align 8
  br i1 %i10, label %bb12, label %bb35

bb12:                                             ; preds = %bb6
  %i13 = and i64 %i11, 1
  %i14 = icmp eq i64 %i13, 0
  br i1 %i14, label %bb62, label %bb15

bb15:                                             ; preds = %bb12
  %i16 = trunc i64 %i11 to i32
  %i17 = shl i32 %i16, 1
  %i18 = and i32 %i17, 4
  %i19 = lshr i64 %i11, 3
  %i20 = trunc i64 %i19 to i32
  %i21 = and i32 %i20, 256
  %i22 = lshr i64 %i11, 15
  %i23 = trunc i64 %i22 to i32
  %i24 = and i32 %i23, 31457280
  %i25 = and i32 %i23, 33554432
  %i26 = or i32 %i21, %i18
  %i27 = or i32 %i26, %i24
  %i28 = or i32 %i27, %i25
  %i29 = or i32 %i28, 262146
  %i30 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 5), align 8
  %i31 = inttoptr i64 %i30 to ptr
  %i32 = tail call i32 @for_dealloc_allocatable_handle(ptr %i7, i32 %i29, ptr %i31) #5
  store ptr null, ptr @arr_mod_mp_b_, align 16
  %i33 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 3), align 8
  %i34 = and i64 %i33, -1030792153090
  store i64 %i34, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 3), align 8
  br label %bb62

bb35:                                             ; preds = %bb6
  %i36 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_b_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %i37 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i38 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %i39 = load i64, ptr %i38, align 1
  %i40 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 0), i32 0)
  %i41 = load i64, ptr %i40, align 1
  %i42 = lshr i64 %i11, 15
  %i43 = trunc i64 %i42 to i32
  %i44 = and i32 %i43, 65011712
  %i45 = or i32 %i44, 262144
  %i46 = tail call ptr @for_realloc_lhs(ptr @arr_mod_mp_b_, ptr @arr_mod_mp_a_, i32 %i45) #5
  %i47 = load ptr, ptr @arr_mod_mp_b_, align 16
  %i48 = load i64, ptr %i36, align 1
  %i49 = icmp slt i64 %i41, 1
  br i1 %i49, label %bb62, label %bb50

bb50:                                             ; preds = %bb35
  %i51 = add nuw nsw i64 %i41, 1
  br label %bb52

bb52:                                             ; preds = %bb52, %bb50
  %i53 = phi i64 [ %i58, %bb52 ], [ %i48, %bb50 ]
  %i54 = phi i64 [ %i59, %bb52 ], [ %i39, %bb50 ]
  %i55 = phi i64 [ %i60, %bb52 ], [ 1, %bb50 ]
  %i56 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i39, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i37, i64 %i54)
  %i57 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i48, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i47, i64 %i53)
  tail call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 1 dereferenceable(288) %i57, ptr noundef nonnull align 1 dereferenceable(288) %i56, i64 288, i1 false)
  %i58 = add nsw i64 %i53, 1
  %i59 = add nsw i64 %i54, 1
  %i60 = add nuw nsw i64 %i55, 1
  %i61 = icmp eq i64 %i60, %i51
  br i1 %i61, label %bb62, label %bb52

bb62:                                             ; preds = %bb52, %bb35, %bb15, %bb12
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_dealloc_allocatable_handle(ptr nocapture readonly, i32, ptr) local_unnamed_addr #1

declare dso_local ptr @for_realloc_lhs(ptr nocapture, ptr nocapture readonly, i32) local_unnamed_addr

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #3

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #3

; Function Attrs: argmemonly nocallback nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #4

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { nounwind readnone speculatable }
attributes #4 = { argmemonly nocallback nofree nounwind willreturn }
attributes #5 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}

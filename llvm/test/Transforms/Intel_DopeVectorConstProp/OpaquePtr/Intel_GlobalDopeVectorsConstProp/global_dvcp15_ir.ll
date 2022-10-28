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

; ifx -xCORE-AVX512 -Ofast -flto arr.f90 -mllvm -hir-details-dims -g -mllvm -print-debug-loc

; The global array A is allocated in ALLOCATE_ARR and the initialized in
; INITIALIZE_ARR. Function PRINT_ARR will pass the fields of T_TESTTYPE to
; PRINT_INFO. This test case is the same as global_dvcp07_ir.ll but it checks
; for debug information. When debug information is added, the nested dope
; vectors are copied to a local dope vector and then it is passed to the
; function. The goal of this test case is to make sure that the information
; was collected from these copies and the constants were propagated, too.

; We are interested in the function @arr_mod_mp_print_info_ since it has
; the copy dope vector as parameter.

; CHECK: define internal void @arr_mod_mp_print_info_
; CHECK:   %i46 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i11, i64 %i45), !dbg !106
; CHECK:   %i49 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i46, i64 %i48), !dbg !106
; CHECK:   %i57 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 400, ptr elementtype(float) %i23, i64 %i56), !dbg !108
; CHECK:   %i58 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i57, i64 %i45), !dbg !108
; CHECK:   %i59 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i58, i64 %i48), !dbg !108
; CHECK:   %i75 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i36, i64 %i74), !dbg !111

source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"ARR_MOD$.btT_TESTTYPE" = type { %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank1$" }
%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank3$" = type { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@arr_mod_mp_a_ = internal global %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$" { ptr null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }, !dbg !0
@anon.11934500c7ed222a0a148892d04ea3ac.0 = internal unnamed_addr constant i32 2
@anon.11934500c7ed222a0a148892d04ea3ac.1 = internal unnamed_addr constant i32 10

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_allocate_arr_(ptr noalias nocapture readonly dereferenceable(4) %arg) #0 !dbg !36 {
bb:
  %i = alloca i64, align 8, !dbg !39
  call void @llvm.dbg.declare(metadata ptr %arg, metadata !38, metadata !DIExpression()), !dbg !39
  %i1 = load i32, ptr %arg, align 1, !dbg !40
  %i2 = icmp eq i32 %i1, 1, !dbg !41
  br i1 %i2, label %bb5, label %bb3, !dbg !41

bb3:                                              ; preds = %bb
  %i4 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0), !dbg !42
  br label %bb33, !dbg !41

bb5:                                              ; preds = %bb
  %i6 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8, !dbg !43
  %i7 = and i64 %i6, 1030792151296, !dbg !43
  %i8 = or i64 %i7, 133, !dbg !43
  store i64 %i8, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8, !dbg !43
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 5), align 8, !dbg !43
  store i64 288, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 1), align 8, !dbg !43
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 4), align 16, !dbg !43
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 2), align 16, !dbg !43
  %i9 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0), !dbg !43
  store i64 1, ptr %i9, align 1, !dbg !43
  %i10 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 0), i32 0), !dbg !43
  store i64 1, ptr %i10, align 1, !dbg !43
  %i11 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 0), !dbg !43
  store i64 288, ptr %i11, align 1, !dbg !43
  %i12 = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %i, i32 2, i64 1, i64 288) #5, !dbg !43
  %i13 = load i64, ptr %i, align 8, !dbg !43
  %i14 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8, !dbg !43
  %i15 = and i64 %i14, -68451041281, !dbg !43
  %i16 = or i64 %i15, 1073741824, !dbg !43
  store i64 %i16, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8, !dbg !43
  %i17 = trunc i64 %i14 to i32, !dbg !43
  %i18 = shl i32 %i17, 1, !dbg !43
  %i19 = and i32 %i18, 2, !dbg !43
  %i20 = shl i32 %i12, 4, !dbg !43
  %i21 = and i32 %i20, 16, !dbg !43
  %i22 = lshr i64 %i14, 15, !dbg !43
  %i23 = trunc i64 %i22 to i32, !dbg !43
  %i24 = and i32 %i23, 31457280, !dbg !43
  %i25 = and i32 %i23, 33554432, !dbg !43
  %i26 = or i32 %i21, %i19, !dbg !43
  %i27 = or i32 %i26, %i24, !dbg !43
  %i28 = or i32 %i27, %i25, !dbg !43
  %i29 = or i32 %i28, 262144, !dbg !43
  %i30 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 5), align 8, !dbg !43
  %i31 = inttoptr i64 %i30 to ptr, !dbg !43
  %i32 = tail call i32 @for_alloc_allocatable_handle(i64 %i13, ptr @arr_mod_mp_a_, i32 %i29, ptr %i31) #5, !dbg !43
  br label %bb33, !dbg !41

bb33:                                             ; preds = %bb5, %bb3
  %i34 = phi ptr [ %i4, %bb3 ], [ %i9, %bb5 ], !dbg !42
  %i35 = load ptr, ptr @arr_mod_mp_a_, align 16, !dbg !42
  %i36 = load i64, ptr %i34, align 1, !dbg !42
  %i37 = sext i32 %i1 to i64, !dbg !42
  %i38 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i36, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i35, i64 %i37), !dbg !44
  %i39 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 3, !dbg !44
  %i40 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 5, !dbg !42
  store i64 0, ptr %i40, align 1, !dbg !42
  %i41 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 1, !dbg !42
  store i64 4, ptr %i41, align 1, !dbg !42
  %i42 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 4, !dbg !42
  store i64 2, ptr %i42, align 1, !dbg !42
  %i43 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 2, !dbg !42
  store i64 0, ptr %i43, align 1, !dbg !42
  %i44 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 6, i64 0, i32 2, !dbg !42
  %i45 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i44, i32 0), !dbg !42
  store i64 1, ptr %i45, align 1, !dbg !42
  %i46 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 6, i64 0, i32 0, !dbg !42
  %i47 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i46, i32 0), !dbg !42
  store i64 10, ptr %i47, align 1, !dbg !42
  %i48 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i44, i32 1), !dbg !42
  store i64 1, ptr %i48, align 1, !dbg !42
  %i49 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i46, i32 1), !dbg !42
  store i64 10, ptr %i49, align 1, !dbg !42
  %i50 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 6, i64 0, i32 1, !dbg !42
  %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i50, i32 0), !dbg !42
  store i64 4, ptr %i51, align 1, !dbg !42
  %i52 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i50, i32 1), !dbg !42
  store i64 40, ptr %i52, align 1, !dbg !42
  store i64 1073741829, ptr %i39, align 1, !dbg !42
  %i53 = bitcast ptr %i38 to ptr, !dbg !42
  %i54 = tail call i32 @for_allocate_handle(i64 400, ptr %i53, i32 262144, ptr null) #5, !dbg !42
  %i55 = load ptr, ptr @arr_mod_mp_a_, align 16, !dbg !45
  %i56 = load i64, ptr %i34, align 1, !dbg !45
  %i57 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i56, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i55, i64 %i37), !dbg !46
  %i58 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 3, !dbg !46
  %i59 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 5, !dbg !45
  store i64 0, ptr %i59, align 1, !dbg !45
  %i60 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 1, !dbg !45
  store i64 4, ptr %i60, align 1, !dbg !45
  %i61 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 4, !dbg !45
  store i64 3, ptr %i61, align 1, !dbg !45
  %i62 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 2, !dbg !45
  store i64 0, ptr %i62, align 1, !dbg !45
  %i63 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 6, i64 0, i32 2, !dbg !45
  %i64 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i63, i32 0), !dbg !45
  store i64 1, ptr %i64, align 1, !dbg !45
  %i65 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 6, i64 0, i32 0, !dbg !45
  %i66 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i65, i32 0), !dbg !45
  store i64 10, ptr %i66, align 1, !dbg !45
  %i67 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i63, i32 1), !dbg !45
  store i64 1, ptr %i67, align 1, !dbg !45
  %i68 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i65, i32 1), !dbg !45
  store i64 10, ptr %i68, align 1, !dbg !45
  %i69 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i63, i32 2), !dbg !45
  store i64 1, ptr %i69, align 1, !dbg !45
  %i70 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i65, i32 2), !dbg !45
  store i64 10, ptr %i70, align 1, !dbg !45
  %i71 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 6, i64 0, i32 1, !dbg !45
  %i72 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i71, i32 0), !dbg !45
  store i64 4, ptr %i72, align 1, !dbg !45
  %i73 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i71, i32 1), !dbg !45
  store i64 40, ptr %i73, align 1, !dbg !45
  %i74 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i71, i32 2), !dbg !45
  store i64 400, ptr %i74, align 1, !dbg !45
  %i75 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 0, !dbg !45
  store i64 1073741829, ptr %i58, align 1, !dbg !45
  %i76 = bitcast ptr %i75 to ptr, !dbg !45
  %i77 = tail call i32 @for_allocate_handle(i64 4000, ptr nonnull %i76, i32 262144, ptr null) #5, !dbg !45
  %i78 = load ptr, ptr @arr_mod_mp_a_, align 16, !dbg !47
  %i79 = load i64, ptr %i34, align 1, !dbg !47
  %i80 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i79, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i78, i64 %i37), !dbg !48
  %i81 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i80, i64 0, i32 2, i32 3, !dbg !48
  %i82 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i80, i64 0, i32 2, i32 5, !dbg !47
  store i64 0, ptr %i82, align 1, !dbg !47
  %i83 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i80, i64 0, i32 2, i32 1, !dbg !47
  store i64 4, ptr %i83, align 1, !dbg !47
  %i84 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i80, i64 0, i32 2, i32 4, !dbg !47
  store i64 1, ptr %i84, align 1, !dbg !47
  %i85 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i80, i64 0, i32 2, i32 2, !dbg !47
  store i64 0, ptr %i85, align 1, !dbg !47
  %i86 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i80, i64 0, i32 2, i32 6, i64 0, i32 2, !dbg !47
  %i87 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i86, i32 0), !dbg !47
  store i64 1, ptr %i87, align 1, !dbg !47
  %i88 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i80, i64 0, i32 2, i32 6, i64 0, i32 0, !dbg !47
  %i89 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i88, i32 0), !dbg !47
  store i64 10, ptr %i89, align 1, !dbg !47
  %i90 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i80, i64 0, i32 2, i32 6, i64 0, i32 1, !dbg !47
  %i91 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i90, i32 0), !dbg !47
  store i64 4, ptr %i91, align 1, !dbg !47
  %i92 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i80, i64 0, i32 2, i32 0, !dbg !47
  store i64 1073741829, ptr %i81, align 1, !dbg !47
  %i93 = bitcast ptr %i92 to ptr, !dbg !47
  %i94 = tail call i32 @for_allocate_handle(i64 40, ptr nonnull %i93, i32 262144, ptr null) #5, !dbg !47
  ret void, !dbg !49
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nofree
declare dso_local i32 @for_check_mult_overflow64(ptr nocapture, i32, ...) local_unnamed_addr #2

; Function Attrs: nofree
declare dso_local i32 @for_alloc_allocatable_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #2

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #2

; Function Attrs: nofree nosync nounwind uwtable
define internal void @arr_mod_mp_initialize_arr_(ptr noalias nocapture readonly dereferenceable(4) %arg, ptr noalias nocapture readonly dereferenceable(4) %arg1, ptr noalias nocapture readonly dereferenceable(4) %arg2, ptr noalias nocapture readonly dereferenceable(4) %arg3) #3 !dbg !50 {
bb:
  call void @llvm.dbg.declare(metadata ptr %arg, metadata !52, metadata !DIExpression()), !dbg !59
  call void @llvm.dbg.declare(metadata ptr @anon.11934500c7ed222a0a148892d04ea3ac.1, metadata !53, metadata !DIExpression()), !dbg !60
  call void @llvm.dbg.declare(metadata ptr @anon.11934500c7ed222a0a148892d04ea3ac.1, metadata !54, metadata !DIExpression()), !dbg !61
  call void @llvm.dbg.declare(metadata ptr @anon.11934500c7ed222a0a148892d04ea3ac.1, metadata !55, metadata !DIExpression()), !dbg !62
  call void @llvm.dbg.value(metadata i32 1, metadata !58, metadata !DIExpression()), !dbg !63
  %i = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0), !dbg !64
  %i4 = load i32, ptr %arg, align 1, !dbg !64
  %i5 = sext i32 %i4 to i64, !dbg !64
  br label %bb6, !dbg !65

bb6:                                              ; preds = %bb58, %bb
  %i7 = phi i64 [ 1, %bb ], [ %i71, %bb58 ]
  call void @llvm.dbg.value(metadata i64 %i7, metadata !58, metadata !DIExpression()), !dbg !63
  call void @llvm.dbg.value(metadata i32 1, metadata !57, metadata !DIExpression()), !dbg !63
  %i8 = trunc i64 %i7 to i32, !dbg !64
  %i9 = sitofp i32 %i8 to float, !dbg !64
  br label %bb10, !dbg !66

bb10:                                             ; preds = %bb55, %bb6
  %i11 = phi i64 [ 1, %bb6 ], [ %i56, %bb55 ]
  call void @llvm.dbg.value(metadata i64 %i11, metadata !57, metadata !DIExpression()), !dbg !63
  %i12 = load ptr, ptr @arr_mod_mp_a_, align 16, !dbg !64
  %i13 = load i64, ptr %i, align 1, !dbg !64
  %i14 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i13, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i12, i64 %i5), !dbg !67
  %i15 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i14, i64 0, i32 0, i32 0, !dbg !68
  %i16 = load ptr, ptr %i15, align 1, !dbg !68
  %i17 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i14, i64 0, i32 0, i32 6, i64 0, i32 1, !dbg !68
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i17, i32 0), !dbg !68
  %i19 = load i64, ptr %i18, align 1, !dbg !68
  %i20 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i14, i64 0, i32 0, i32 6, i64 0, i32 2, !dbg !68
  %i21 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i20, i32 0), !dbg !68
  %i22 = load i64, ptr %i21, align 1, !dbg !68
  %i23 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i17, i32 1), !dbg !68
  %i24 = load i64, ptr %i23, align 1, !dbg !68
  %i25 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i20, i32 1), !dbg !68
  %i26 = load i64, ptr %i25, align 1, !dbg !68
  %i27 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i26, i64 %i24, ptr elementtype(float) %i16, i64 %i7), !dbg !67
  %i28 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i22, i64 %i19, ptr elementtype(float) %i27, i64 %i11), !dbg !67
  store float %i9, ptr %i28, align 1, !dbg !67
  call void @llvm.dbg.value(metadata i32 1, metadata !56, metadata !DIExpression()), !dbg !63
  br label %bb29

bb29:                                             ; preds = %bb29, %bb10
  %i30 = phi i64 [ %i53, %bb29 ], [ 1, %bb10 ]
  call void @llvm.dbg.value(metadata i64 %i30, metadata !56, metadata !DIExpression()), !dbg !63
  %i31 = load ptr, ptr @arr_mod_mp_a_, align 16, !dbg !69
  %i32 = load i64, ptr %i, align 1, !dbg !69
  %i33 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i32, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i31, i64 %i5), !dbg !70
  %i34 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i33, i64 0, i32 1, i32 0, !dbg !71
  %i35 = load ptr, ptr %i34, align 1, !dbg !71
  %i36 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i33, i64 0, i32 1, i32 6, i64 0, i32 1, !dbg !71
  %i37 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i36, i32 0), !dbg !71
  %i38 = load i64, ptr %i37, align 1, !dbg !71
  %i39 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i33, i64 0, i32 1, i32 6, i64 0, i32 2, !dbg !71
  %i40 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i39, i32 0), !dbg !71
  %i41 = load i64, ptr %i40, align 1, !dbg !71
  %i42 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i36, i32 1), !dbg !71
  %i43 = load i64, ptr %i42, align 1, !dbg !71
  %i44 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i39, i32 1), !dbg !71
  %i45 = load i64, ptr %i44, align 1, !dbg !71
  %i46 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i36, i32 2), !dbg !71
  %i47 = load i64, ptr %i46, align 1, !dbg !71
  %i48 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i39, i32 2), !dbg !71
  %i49 = load i64, ptr %i48, align 1, !dbg !71
  %i50 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %i49, i64 %i47, ptr elementtype(float) %i35, i64 %i30), !dbg !70
  %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i45, i64 %i43, ptr elementtype(float) %i50, i64 %i7), !dbg !70
  %i52 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i41, i64 %i38, ptr elementtype(float) %i51, i64 %i11), !dbg !70
  store float %i9, ptr %i52, align 1, !dbg !70
  %i53 = add nuw nsw i64 %i30, 1, !dbg !72
  call void @llvm.dbg.value(metadata i64 %i53, metadata !56, metadata !DIExpression()), !dbg !63
  %i54 = icmp eq i64 %i53, 11, !dbg !72
  br i1 %i54, label %bb55, label %bb29, !dbg !72

bb55:                                             ; preds = %bb29
  %i56 = add nuw nsw i64 %i11, 1, !dbg !73
  call void @llvm.dbg.value(metadata i64 %i56, metadata !57, metadata !DIExpression()), !dbg !63
  %i57 = icmp eq i64 %i56, 11, !dbg !73
  br i1 %i57, label %bb58, label %bb10, !dbg !73

bb58:                                             ; preds = %bb55
  call void @llvm.dbg.value(metadata i32 11, metadata !57, metadata !DIExpression()), !dbg !63
  %i59 = load ptr, ptr @arr_mod_mp_a_, align 16, !dbg !74
  %i60 = load i64, ptr %i, align 1, !dbg !74
  %i61 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i60, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i59, i64 %i5), !dbg !75
  %i62 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i61, i64 0, i32 2, i32 0, !dbg !76
  %i63 = load ptr, ptr %i62, align 1, !dbg !76
  %i64 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i61, i64 0, i32 2, i32 6, i64 0, i32 1, !dbg !76
  %i65 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i64, i32 0), !dbg !76
  %i66 = load i64, ptr %i65, align 1, !dbg !76
  %i67 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i61, i64 0, i32 2, i32 6, i64 0, i32 2, !dbg !76
  %i68 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i67, i32 0), !dbg !76
  %i69 = load i64, ptr %i68, align 1, !dbg !76
  %i70 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i69, i64 %i66, ptr elementtype(float) %i63, i64 11), !dbg !75
  store float 1.100000e+01, ptr %i70, align 1, !dbg !75
  %i71 = add nuw nsw i64 %i7, 1, !dbg !77
  call void @llvm.dbg.value(metadata i64 %i71, metadata !58, metadata !DIExpression()), !dbg !63
  %i72 = icmp eq i64 %i71, 11, !dbg !77
  br i1 %i72, label %bb73, label %bb6, !dbg !77

bb73:                                             ; preds = %bb58
  ret void, !dbg !78
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_print_info_(ptr noalias nocapture readonly %arg, ptr noalias nocapture readonly dereferenceable(4) %arg1, ptr noalias nocapture readonly dereferenceable(4) %arg2, ptr noalias nocapture readonly dereferenceable(4) %arg3, ptr noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %arg4, ptr noalias nocapture readonly dereferenceable(120) "assumed_shape" "ptrnoalias" %arg5, ptr noalias nocapture readonly dereferenceable(72) "assumed_shape" "ptrnoalias" %arg6) #0 !dbg !79 {
bb:
  %i = alloca [8 x i64], align 16
  %i7 = alloca [4 x i8], align 1, !dbg !94
  %i8 = alloca [4 x i8], align 1, !dbg !94
  %i9 = alloca [4 x i8], align 1, !dbg !94
  call void @llvm.dbg.declare(metadata ptr undef, metadata !81, metadata !DIExpression()), !dbg !94
  call void @llvm.dbg.declare(metadata ptr @anon.11934500c7ed222a0a148892d04ea3ac.1, metadata !82, metadata !DIExpression()), !dbg !95
  call void @llvm.dbg.declare(metadata ptr @anon.11934500c7ed222a0a148892d04ea3ac.1, metadata !83, metadata !DIExpression()), !dbg !96
  call void @llvm.dbg.declare(metadata ptr @anon.11934500c7ed222a0a148892d04ea3ac.1, metadata !84, metadata !DIExpression()), !dbg !97
  call void @llvm.dbg.declare(metadata ptr %arg4, metadata !85, metadata !DIExpression()), !dbg !98
  call void @llvm.dbg.declare(metadata ptr %arg5, metadata !87, metadata !DIExpression()), !dbg !99
  call void @llvm.dbg.declare(metadata ptr %arg6, metadata !89, metadata !DIExpression()), !dbg !100
  call void @llvm.dbg.value(metadata i32 1, metadata !93, metadata !DIExpression()), !dbg !101
  %i10 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %arg4, i64 0, i32 0, !dbg !102
  %i11 = load ptr, ptr %i10, align 1, !dbg !102
  %i12 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %arg4, i64 0, i32 6, i64 0, i32 1, !dbg !102
  %i13 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i12, i32 0), !dbg !102
  %i14 = load i64, ptr %i13, align 1, !dbg !102
  %i15 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i12, i32 1), !dbg !102
  %i16 = load i64, ptr %i15, align 1, !dbg !102
  %i17 = getelementptr inbounds [4 x i8], ptr %i7, i64 0, i64 0, !dbg !102
  %i18 = getelementptr inbounds [4 x i8], ptr %i7, i64 0, i64 1, !dbg !102
  %i19 = getelementptr inbounds [4 x i8], ptr %i7, i64 0, i64 2, !dbg !102
  %i20 = getelementptr inbounds [4 x i8], ptr %i7, i64 0, i64 3, !dbg !102
  %i21 = bitcast ptr %i to ptr, !dbg !102
  %i22 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %arg5, i64 0, i32 0, !dbg !103
  %i23 = load ptr, ptr %i22, align 1, !dbg !103
  %i24 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %arg5, i64 0, i32 6, i64 0, i32 1, !dbg !103
  %i25 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i24, i32 0), !dbg !103
  %i26 = load i64, ptr %i25, align 1, !dbg !103
  %i27 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i24, i32 1), !dbg !103
  %i28 = load i64, ptr %i27, align 1, !dbg !103
  %i29 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i24, i32 2), !dbg !103
  %i30 = load i64, ptr %i29, align 1, !dbg !103
  %i31 = getelementptr inbounds [4 x i8], ptr %i8, i64 0, i64 0, !dbg !103
  %i32 = getelementptr inbounds [4 x i8], ptr %i8, i64 0, i64 1, !dbg !103
  %i33 = getelementptr inbounds [4 x i8], ptr %i8, i64 0, i64 2, !dbg !103
  %i34 = getelementptr inbounds [4 x i8], ptr %i8, i64 0, i64 3, !dbg !103
  %i35 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %arg6, i64 0, i32 0, !dbg !104
  %i36 = load ptr, ptr %i35, align 1, !dbg !104
  %i37 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %arg6, i64 0, i32 6, i64 0, i32 1, !dbg !104
  %i38 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i37, i32 0), !dbg !104
  %i39 = load i64, ptr %i38, align 1, !dbg !104
  %i40 = getelementptr inbounds [4 x i8], ptr %i9, i64 0, i64 0, !dbg !104
  %i41 = getelementptr inbounds [4 x i8], ptr %i9, i64 0, i64 1, !dbg !104
  %i42 = getelementptr inbounds [4 x i8], ptr %i9, i64 0, i64 2, !dbg !104
  %i43 = getelementptr inbounds [4 x i8], ptr %i9, i64 0, i64 3, !dbg !104
  br label %bb44, !dbg !105

bb44:                                             ; preds = %bb72, %bb
  %i45 = phi i64 [ 1, %bb ], [ %i81, %bb72 ]
  call void @llvm.dbg.value(metadata i64 %i45, metadata !93, metadata !DIExpression()), !dbg !101
  call void @llvm.dbg.value(metadata i32 1, metadata !92, metadata !DIExpression()), !dbg !101
  %i46 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i16, ptr elementtype(float) %i11, i64 %i45), !dbg !106
  br label %bb47, !dbg !107

bb47:                                             ; preds = %bb68, %bb44
  %i48 = phi i64 [ 1, %bb44 ], [ %i69, %bb68 ]
  call void @llvm.dbg.value(metadata i64 %i48, metadata !92, metadata !DIExpression()), !dbg !101
  %i49 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i14, ptr elementtype(float) %i46, i64 %i48), !dbg !106
  %i50 = load float, ptr %i49, align 1, !dbg !106
  store i8 26, ptr %i17, align 1, !dbg !102
  store i8 1, ptr %i18, align 1, !dbg !102
  store i8 1, ptr %i19, align 1, !dbg !102
  store i8 0, ptr %i20, align 1, !dbg !102
  %i51 = alloca { float }, align 8, !dbg !102
  %i52 = getelementptr inbounds { float }, ptr %i51, i64 0, i32 0, !dbg !102
  store float %i50, ptr %i52, align 8, !dbg !102
  %i53 = bitcast ptr %i51 to ptr, !dbg !102
  %i54 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i21, i32 -1, i64 1239157112576, ptr nonnull %i17, ptr nonnull %i53) #5, !dbg !102
  call void @llvm.dbg.value(metadata i32 1, metadata !91, metadata !DIExpression()), !dbg !101
  br label %bb55

bb55:                                             ; preds = %bb55, %bb47
  %i56 = phi i64 [ %i65, %bb55 ], [ 1, %bb47 ]
  call void @llvm.dbg.value(metadata i64 %i56, metadata !91, metadata !DIExpression()), !dbg !101
  %i57 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %i30, ptr elementtype(float) %i23, i64 %i56), !dbg !108
  %i58 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i28, ptr elementtype(float) %i57, i64 %i45), !dbg !108
  %i59 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i26, ptr elementtype(float) %i58, i64 %i48), !dbg !108
  %i60 = load float, ptr %i59, align 1, !dbg !108
  store i8 26, ptr %i31, align 1, !dbg !103
  store i8 1, ptr %i32, align 1, !dbg !103
  store i8 1, ptr %i33, align 1, !dbg !103
  store i8 0, ptr %i34, align 1, !dbg !103
  %i61 = alloca { float }, align 8, !dbg !103
  %i62 = getelementptr inbounds { float }, ptr %i61, i64 0, i32 0, !dbg !103
  store float %i60, ptr %i62, align 8, !dbg !103
  %i63 = bitcast ptr %i61 to ptr, !dbg !103
  %i64 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i21, i32 -1, i64 1239157112576, ptr nonnull %i31, ptr nonnull %i63) #5, !dbg !103
  %i65 = add nuw i64 %i56, 1, !dbg !109
  call void @llvm.dbg.value(metadata i64 %i65, metadata !91, metadata !DIExpression()), !dbg !101
  %i66 = trunc i64 %i65 to i32, !dbg !109
  %i67 = icmp slt i32 10, %i66, !dbg !109
  br i1 %i67, label %bb68, label %bb55, !dbg !109

bb68:                                             ; preds = %bb55
  %i69 = add nuw i64 %i48, 1, !dbg !110
  call void @llvm.dbg.value(metadata i64 %i69, metadata !92, metadata !DIExpression()), !dbg !101
  %i70 = trunc i64 %i69 to i32, !dbg !110
  %i71 = icmp slt i32 10, %i70, !dbg !110
  br i1 %i71, label %bb72, label %bb47, !dbg !110

bb72:                                             ; preds = %bb68
  call void @llvm.dbg.value(metadata i32 undef, metadata !92, metadata !DIExpression()), !dbg !101
  %i73 = shl i64 %i69, 32, !dbg !104
  %i74 = ashr exact i64 %i73, 32, !dbg !104
  %i75 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i39, ptr elementtype(float) %i36, i64 %i74), !dbg !111
  %i76 = load float, ptr %i75, align 1, !dbg !111
  store i8 26, ptr %i40, align 1, !dbg !104
  store i8 1, ptr %i41, align 1, !dbg !104
  store i8 1, ptr %i42, align 1, !dbg !104
  store i8 0, ptr %i43, align 1, !dbg !104
  %i77 = alloca { float }, align 8, !dbg !104
  %i78 = getelementptr inbounds { float }, ptr %i77, i64 0, i32 0, !dbg !104
  store float %i76, ptr %i78, align 8, !dbg !104
  %i79 = bitcast ptr %i77 to ptr, !dbg !104
  %i80 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i21, i32 -1, i64 1239157112576, ptr nonnull %i40, ptr nonnull %i79) #5, !dbg !104
  %i81 = add nuw i64 %i45, 1, !dbg !112
  call void @llvm.dbg.value(metadata i64 %i81, metadata !93, metadata !DIExpression()), !dbg !101
  %i82 = trunc i64 %i81 to i32, !dbg !112
  %i83 = icmp slt i32 10, %i82, !dbg !112
  br i1 %i83, label %bb84, label %bb44, !dbg !112

bb84:                                             ; preds = %bb72
  ret void, !dbg !113
}

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr #2

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_print_arr_(ptr noalias nocapture readonly dereferenceable(4) %arg, ptr noalias nocapture readonly dereferenceable(4) %arg1, ptr noalias nocapture readonly dereferenceable(4) %arg2, ptr noalias nocapture readonly dereferenceable(4) %arg3) #0 !dbg !114 {
bb:
  %i = alloca %"QNCA_a0$float*$rank2$", align 8, !dbg !120
  %i4 = alloca %"QNCA_a0$float*$rank3$", align 8, !dbg !120
  %i5 = alloca %"QNCA_a0$float*$rank1$", align 8, !dbg !120
  call void @llvm.dbg.declare(metadata ptr %arg, metadata !116, metadata !DIExpression()), !dbg !120
  call void @llvm.dbg.declare(metadata ptr @anon.11934500c7ed222a0a148892d04ea3ac.1, metadata !117, metadata !DIExpression()), !dbg !121
  call void @llvm.dbg.declare(metadata ptr @anon.11934500c7ed222a0a148892d04ea3ac.1, metadata !118, metadata !DIExpression()), !dbg !122
  call void @llvm.dbg.declare(metadata ptr @anon.11934500c7ed222a0a148892d04ea3ac.1, metadata !119, metadata !DIExpression()), !dbg !123
  %i6 = load ptr, ptr @arr_mod_mp_a_, align 16, !dbg !124
  %i7 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0), !dbg !124
  %i8 = load i64, ptr %i7, align 1, !dbg !124
  %i9 = load i32, ptr %arg, align 1, !dbg !124
  %i10 = sext i32 %i9 to i64, !dbg !124
  %i11 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i8, i64 288, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i6, i64 %i10), !dbg !124
  %i12 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i11, i64 0, i32 0, i32 0, !dbg !124
  %i13 = load ptr, ptr %i12, align 1, !dbg !124
  %i14 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i11, i64 0, i32 0, i32 6, i64 0, i32 1, !dbg !124
  %i15 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i14, i32 0), !dbg !124
  %i16 = load i64, ptr %i15, align 1, !dbg !124
  %i17 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i11, i64 0, i32 0, i32 6, i64 0, i32 0, !dbg !124
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i17, i32 0), !dbg !124
  %i19 = load i64, ptr %i18, align 1, !dbg !124
  %i20 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i14, i32 1), !dbg !124
  %i21 = load i64, ptr %i20, align 1, !dbg !124
  %i22 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i17, i32 1), !dbg !124
  %i23 = load i64, ptr %i22, align 1, !dbg !124
  %i24 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 3, !dbg !124
  %i25 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 1, !dbg !124
  store i64 4, ptr %i25, align 8, !dbg !124
  %i26 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 4, !dbg !124
  store i64 2, ptr %i26, align 8, !dbg !124
  %i27 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 2, !dbg !124
  store i64 0, ptr %i27, align 8, !dbg !124
  %i28 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 6, i64 0, i32 1, !dbg !124
  %i29 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i28, i32 0), !dbg !124
  store i64 %i16, ptr %i29, align 1, !dbg !124
  %i30 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 6, i64 0, i32 2, !dbg !124
  %i31 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i30, i32 0), !dbg !124
  store i64 1, ptr %i31, align 1, !dbg !124
  %i32 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 6, i64 0, i32 0, !dbg !124
  %i33 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i32, i32 0), !dbg !124
  store i64 %i19, ptr %i33, align 1, !dbg !124
  %i34 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i28, i32 1), !dbg !124
  store i64 %i21, ptr %i34, align 1, !dbg !124
  %i35 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i30, i32 1), !dbg !124
  store i64 1, ptr %i35, align 1, !dbg !124
  %i36 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i32, i32 1), !dbg !124
  store i64 %i23, ptr %i36, align 1, !dbg !124
  %i37 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 0, !dbg !124
  store ptr %i13, ptr %i37, align 8, !dbg !124
  store i64 1, ptr %i24, align 8, !dbg !124
  %i38 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i11, i64 0, i32 1, i32 0, !dbg !124
  %i39 = load ptr, ptr %i38, align 1, !dbg !124
  %i40 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i11, i64 0, i32 1, i32 6, i64 0, i32 1, !dbg !124
  %i41 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i40, i32 0), !dbg !124
  %i42 = load i64, ptr %i41, align 1, !dbg !124
  %i43 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i11, i64 0, i32 1, i32 6, i64 0, i32 0, !dbg !124
  %i44 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i43, i32 0), !dbg !124
  %i45 = load i64, ptr %i44, align 1, !dbg !124
  %i46 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i40, i32 1), !dbg !124
  %i47 = load i64, ptr %i46, align 1, !dbg !124
  %i48 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i43, i32 1), !dbg !124
  %i49 = load i64, ptr %i48, align 1, !dbg !124
  %i50 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i40, i32 2), !dbg !124
  %i51 = load i64, ptr %i50, align 1, !dbg !124
  %i52 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i43, i32 2), !dbg !124
  %i53 = load i64, ptr %i52, align 1, !dbg !124
  %i54 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %i4, i64 0, i32 3, !dbg !124
  %i55 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %i4, i64 0, i32 1, !dbg !124
  store i64 4, ptr %i55, align 8, !dbg !124
  %i56 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %i4, i64 0, i32 4, !dbg !124
  store i64 3, ptr %i56, align 8, !dbg !124
  %i57 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %i4, i64 0, i32 2, !dbg !124
  store i64 0, ptr %i57, align 8, !dbg !124
  %i58 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %i4, i64 0, i32 6, i64 0, i32 1, !dbg !124
  %i59 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i58, i32 0), !dbg !124
  store i64 %i42, ptr %i59, align 1, !dbg !124
  %i60 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %i4, i64 0, i32 6, i64 0, i32 2, !dbg !124
  %i61 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i60, i32 0), !dbg !124
  store i64 1, ptr %i61, align 1, !dbg !124
  %i62 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %i4, i64 0, i32 6, i64 0, i32 0, !dbg !124
  %i63 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i62, i32 0), !dbg !124
  store i64 %i45, ptr %i63, align 1, !dbg !124
  %i64 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i58, i32 1), !dbg !124
  store i64 %i47, ptr %i64, align 1, !dbg !124
  %i65 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i60, i32 1), !dbg !124
  store i64 1, ptr %i65, align 1, !dbg !124
  %i66 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i62, i32 1), !dbg !124
  store i64 %i49, ptr %i66, align 1, !dbg !124
  %i67 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i58, i32 2), !dbg !124
  store i64 %i51, ptr %i67, align 1, !dbg !124
  %i68 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i60, i32 2), !dbg !124
  store i64 1, ptr %i68, align 1, !dbg !124
  %i69 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i62, i32 2), !dbg !124
  store i64 %i53, ptr %i69, align 1, !dbg !124
  %i70 = getelementptr inbounds %"QNCA_a0$float*$rank3$", ptr %i4, i64 0, i32 0, !dbg !124
  store ptr %i39, ptr %i70, align 8, !dbg !124
  store i64 1, ptr %i54, align 8, !dbg !124
  %i71 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i11, i64 0, i32 2, i32 0, !dbg !124
  %i72 = load ptr, ptr %i71, align 1, !dbg !124
  %i73 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i11, i64 0, i32 2, i32 6, i64 0, i32 1, !dbg !124
  %i74 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i73, i32 0), !dbg !124
  %i75 = load i64, ptr %i74, align 1, !dbg !124
  %i76 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i11, i64 0, i32 2, i32 6, i64 0, i32 0, !dbg !124
  %i77 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i76, i32 0), !dbg !124
  %i78 = load i64, ptr %i77, align 1, !dbg !124
  %i79 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %i5, i64 0, i32 3, !dbg !124
  %i80 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %i5, i64 0, i32 1, !dbg !124
  store i64 4, ptr %i80, align 8, !dbg !124
  %i81 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %i5, i64 0, i32 4, !dbg !124
  store i64 1, ptr %i81, align 8, !dbg !124
  %i82 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %i5, i64 0, i32 2, !dbg !124
  store i64 0, ptr %i82, align 8, !dbg !124
  %i83 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %i5, i64 0, i32 6, i64 0, i32 1, !dbg !124
  %i84 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i83, i32 0), !dbg !124
  store i64 %i75, ptr %i84, align 1, !dbg !124
  %i85 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %i5, i64 0, i32 6, i64 0, i32 2, !dbg !124
  %i86 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i85, i32 0), !dbg !124
  store i64 1, ptr %i86, align 1, !dbg !124
  %i87 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %i5, i64 0, i32 6, i64 0, i32 0, !dbg !124
  %i88 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i87, i32 0), !dbg !124
  store i64 %i78, ptr %i88, align 1, !dbg !124
  %i89 = getelementptr inbounds %"QNCA_a0$float*$rank1$", ptr %i5, i64 0, i32 0, !dbg !124
  store ptr %i72, ptr %i89, align 8, !dbg !124
  store i64 1, ptr %i79, align 8, !dbg !124
  call void @arr_mod_mp_print_info_(ptr undef, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, ptr nonnull %i, ptr nonnull %i4, ptr nonnull %i5), !dbg !125
  ret void, !dbg !126
}

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #0 !dbg !25 {
bb:
  %i = alloca i32, align 8
  %i1 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.0) #5, !dbg !127
  call void @llvm.dbg.value(metadata i32 1, metadata !29, metadata !DIExpression()), !dbg !128
  store i32 1, ptr %i, align 8, !dbg !129
  br label %bb2, !dbg !129

bb2:                                              ; preds = %bb2, %bb
  %i3 = phi i32 [ %i4, %bb2 ], [ 1, %bb ], !dbg !130
  call void @llvm.dbg.value(metadata ptr %i, metadata !29, metadata !DIExpression(DW_OP_deref)), !dbg !128
  call void @arr_mod_mp_allocate_arr_(ptr nonnull %i), !dbg !131
  call void @arr_mod_mp_initialize_arr_(ptr nonnull %i, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1), !dbg !132
  call void @arr_mod_mp_print_arr_(ptr nonnull %i, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1), !dbg !133
  call void @llvm.dbg.value(metadata i32 %i3, metadata !29, metadata !DIExpression()), !dbg !128
  %i4 = add nuw nsw i32 %i3, 1, !dbg !130
  call void @llvm.dbg.value(metadata i32 %i4, metadata !29, metadata !DIExpression()), !dbg !128
  store i32 %i4, ptr %i, align 8, !dbg !130
  %i5 = icmp eq i32 %i4, 11, !dbg !130
  br i1 %i5, label %bb6, label %bb2, !dbg !130

bb6:                                              ; preds = %bb2
  ret void, !dbg !134
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #2

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #4

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #4

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { nofree nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #4 = { nounwind readnone speculatable }
attributes #5 = { nounwind }

!llvm.dbg.cu = !{!20}
!omp_offload.info = !{}
!llvm.module.flags = !{!31, !32, !33, !34, !35}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "a", linkageName: "arr_mod_mp_a_", scope: !2, file: !3, line: 11, type: !4, isLocal: false, isDefinition: true)
!2 = !DIModule(scope: null, name: "arr_mod", file: !3, line: 1)
!3 = !DIFile(filename: "simple.f90", directory: "/localdisk2/ayrivera/dev-tbaa/llvm/llvm/test/Transforms/Intel_DopeVectorConstProp/Intel_GlobalDopeVectorsConstProp")
!4 = !DICompositeType(tag: DW_TAG_array_type, baseType: !5, elements: !19, dataLocation: !DIExpression(DW_OP_push_object_address, DW_OP_deref), allocated: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 24, DW_OP_deref, DW_OP_constu, 1, DW_OP_and))
!5 = !DICompositeType(tag: DW_TAG_structure_type, name: "t_testtype", scope: !2, file: !3, line: 3, size: 2304, elements: !6)
!6 = !{!7, !13, !17}
!7 = !DIDerivedType(tag: DW_TAG_member, name: "inner_array_a", scope: !5, file: !3, line: 3, baseType: !8, flags: DIFlagPublic)
!8 = !DICompositeType(tag: DW_TAG_array_type, baseType: !9, elements: !10, dataLocation: !DIExpression(DW_OP_push_object_address, DW_OP_deref), associated: !DIExpression(DW_OP_push_object_address, DW_OP_deref, DW_OP_constu, 0, DW_OP_or))
!9 = !DIBasicType(name: "REAL*4", size: 32, encoding: DW_ATE_float)
!10 = !{!11, !12}
!11 = !DISubrange(lowerBound: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 64, DW_OP_deref), upperBound: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 64, DW_OP_deref, DW_OP_push_object_address, DW_OP_plus_uconst, 48, DW_OP_deref, DW_OP_plus, DW_OP_constu, 1, DW_OP_minus), stride: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 56, DW_OP_deref))
!12 = !DISubrange(lowerBound: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 88, DW_OP_deref), upperBound: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 88, DW_OP_deref, DW_OP_push_object_address, DW_OP_plus_uconst, 72, DW_OP_deref, DW_OP_plus, DW_OP_constu, 1, DW_OP_minus), stride: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 80, DW_OP_deref))
!13 = !DIDerivedType(tag: DW_TAG_member, name: "inner_array_b", scope: !5, file: !3, line: 3, baseType: !14, offset: 768, flags: DIFlagPublic)
!14 = !DICompositeType(tag: DW_TAG_array_type, baseType: !9, elements: !15, dataLocation: !DIExpression(DW_OP_push_object_address, DW_OP_deref), associated: !DIExpression(DW_OP_push_object_address, DW_OP_deref, DW_OP_constu, 0, DW_OP_or))
!15 = !{!11, !12, !16}
!16 = !DISubrange(lowerBound: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 112, DW_OP_deref), upperBound: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 112, DW_OP_deref, DW_OP_push_object_address, DW_OP_plus_uconst, 96, DW_OP_deref, DW_OP_plus, DW_OP_constu, 1, DW_OP_minus), stride: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 104, DW_OP_deref))
!17 = !DIDerivedType(tag: DW_TAG_member, name: "inner_array_c", scope: !5, file: !3, line: 3, baseType: !18, offset: 1728, flags: DIFlagPublic)
!18 = !DICompositeType(tag: DW_TAG_array_type, baseType: !9, elements: !19, dataLocation: !DIExpression(DW_OP_push_object_address, DW_OP_deref), associated: !DIExpression(DW_OP_push_object_address, DW_OP_deref, DW_OP_constu, 0, DW_OP_or))
!19 = !{!11}
!20 = distinct !DICompileUnit(language: DW_LANG_Fortran95, file: !3, producer: "Intel(R) Fortran 21.0-2698", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !21, globals: !22, imports: !23, splitDebugInlining: false, nameTableKind: None)
!21 = !{}
!22 = !{!0}
!23 = !{!24}
!24 = !DIImportedEntity(tag: DW_TAG_imported_module, scope: !25, entity: !2, file: !3, line: 71)
!25 = distinct !DISubprogram(name: "main", linkageName: "MAIN__", scope: !3, file: !3, line: 70, type: !26, scopeLine: 70, spFlags: DISPFlagDefinition | DISPFlagMainSubprogram, unit: !20, retainedNodes: !28)
!26 = !DISubroutineType(types: !27)
!27 = !{null}
!28 = !{!29}
!29 = !DILocalVariable(name: "i", scope: !25, file: !3, line: 74, type: !30)
!30 = !DIBasicType(name: "INTEGER*4", size: 32, encoding: DW_ATE_signed)
!31 = !{i32 2, !"Debug Info Version", i32 3}
!32 = !{i32 2, !"Dwarf Version", i32 4}
!33 = !{i32 1, !"ThinLTO", i32 0}
!34 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!35 = !{i32 1, !"LTOPostLink", i32 1}
!36 = distinct !DISubprogram(name: "allocate_arr", linkageName: "arr_mod_mp_allocate_arr_", scope: !2, file: !3, line: 15, type: !26, scopeLine: 15, spFlags: DISPFlagDefinition, unit: !20, retainedNodes: !37)
!37 = !{!38}
!38 = !DILocalVariable(name: "i", arg: 1, scope: !36, file: !3, line: 15, type: !30)
!39 = !DILocation(line: 15, column: 34, scope: !36)
!40 = !DILocation(line: 18, column: 12, scope: !36)
!41 = !DILocation(line: 18, column: 16, scope: !36)
!42 = !DILocation(line: 20, column: 12, scope: !36)
!43 = !DILocation(line: 18, column: 23, scope: !36)
!44 = !DILocation(line: 20, column: 21, scope: !36)
!45 = !DILocation(line: 21, column: 12, scope: !36)
!46 = !DILocation(line: 21, column: 21, scope: !36)
!47 = !DILocation(line: 22, column: 12, scope: !36)
!48 = !DILocation(line: 22, column: 21, scope: !36)
!49 = !DILocation(line: 25, column: 10, scope: !36)
!50 = distinct !DISubprogram(name: "initialize_arr", linkageName: "arr_mod_mp_initialize_arr_", scope: !2, file: !3, line: 27, type: !26, scopeLine: 27, spFlags: DISPFlagDefinition, unit: !20, retainedNodes: !51)
!51 = !{!52, !53, !54, !55, !56, !57, !58}
!52 = !DILocalVariable(name: "i", arg: 1, scope: !50, file: !3, line: 27, type: !30)
!53 = !DILocalVariable(name: "n", arg: 2, scope: !50, file: !3, line: 27, type: !30)
!54 = !DILocalVariable(name: "m", arg: 3, scope: !50, file: !3, line: 27, type: !30)
!55 = !DILocalVariable(name: "o", arg: 4, scope: !50, file: !3, line: 27, type: !30)
!56 = !DILocalVariable(name: "l", scope: !50, file: !3, line: 33, type: !30)
!57 = !DILocalVariable(name: "k", scope: !50, file: !3, line: 31, type: !30)
!58 = !DILocalVariable(name: "j", scope: !50, file: !3, line: 30, type: !30)
!59 = !DILocation(line: 27, column: 36, scope: !50)
!60 = !DILocation(line: 27, column: 39, scope: !50)
!61 = !DILocation(line: 27, column: 42, scope: !50)
!62 = !DILocation(line: 27, column: 45, scope: !50)
!63 = !DILocation(line: 0, scope: !50)
!64 = !DILocation(line: 32, column: 45, scope: !50)
!65 = !DILocation(line: 31, column: 14, scope: !50)
!66 = !DILocation(line: 33, column: 16, scope: !50)
!67 = !DILocation(line: 32, column: 16, scope: !50)
!68 = !DILocation(line: 32, column: 23, scope: !50)
!69 = !DILocation(line: 34, column: 50, scope: !50)
!70 = !DILocation(line: 34, column: 18, scope: !50)
!71 = !DILocation(line: 34, column: 25, scope: !50)
!72 = !DILocation(line: 35, column: 16, scope: !50)
!73 = !DILocation(line: 36, column: 14, scope: !50)
!74 = !DILocation(line: 37, column: 40, scope: !50)
!75 = !DILocation(line: 37, column: 14, scope: !50)
!76 = !DILocation(line: 37, column: 21, scope: !50)
!77 = !DILocation(line: 38, column: 12, scope: !50)
!78 = !DILocation(line: 41, column: 10, scope: !50)
!79 = distinct !DISubprogram(name: "print_info", linkageName: "arr_mod_mp_print_info_", scope: !2, file: !3, line: 43, type: !26, scopeLine: 43, spFlags: DISPFlagDefinition, unit: !20, retainedNodes: !80)
!80 = !{!81, !82, !83, !84, !85, !87, !89, !91, !92, !93}
!81 = !DILocalVariable(name: "i", arg: 1, scope: !79, file: !3, line: 43, type: !30)
!82 = !DILocalVariable(name: "n", arg: 2, scope: !79, file: !3, line: 43, type: !30)
!83 = !DILocalVariable(name: "m", arg: 3, scope: !79, file: !3, line: 43, type: !30)
!84 = !DILocalVariable(name: "o", arg: 4, scope: !79, file: !3, line: 43, type: !30)
!85 = !DILocalVariable(name: "array_a", arg: 5, scope: !79, file: !3, line: 43, type: !86)
!86 = !DICompositeType(tag: DW_TAG_array_type, baseType: !9, elements: !10, dataLocation: !DIExpression(DW_OP_push_object_address, DW_OP_deref))
!87 = !DILocalVariable(name: "array_b", arg: 6, scope: !79, file: !3, line: 43, type: !88)
!88 = !DICompositeType(tag: DW_TAG_array_type, baseType: !9, elements: !15, dataLocation: !DIExpression(DW_OP_push_object_address, DW_OP_deref))
!89 = !DILocalVariable(name: "array_c", arg: 7, scope: !79, file: !3, line: 43, type: !90)
!90 = !DICompositeType(tag: DW_TAG_array_type, baseType: !9, elements: !19, dataLocation: !DIExpression(DW_OP_push_object_address, DW_OP_deref))
!91 = !DILocalVariable(name: "l", scope: !79, file: !3, line: 50, type: !30)
!92 = !DILocalVariable(name: "k", scope: !79, file: !3, line: 48, type: !30)
!93 = !DILocalVariable(name: "j", scope: !79, file: !3, line: 47, type: !30)
!94 = !DILocation(line: 43, column: 32, scope: !79)
!95 = !DILocation(line: 43, column: 35, scope: !79)
!96 = !DILocation(line: 43, column: 38, scope: !79)
!97 = !DILocation(line: 43, column: 41, scope: !79)
!98 = !DILocation(line: 43, column: 44, scope: !79)
!99 = !DILocation(line: 43, column: 53, scope: !79)
!100 = !DILocation(line: 43, column: 62, scope: !79)
!101 = !DILocation(line: 0, scope: !79)
!102 = !DILocation(line: 49, column: 16, scope: !79)
!103 = !DILocation(line: 51, column: 18, scope: !79)
!104 = !DILocation(line: 54, column: 14, scope: !79)
!105 = !DILocation(line: 48, column: 14, scope: !79)
!106 = !DILocation(line: 49, column: 25, scope: !79)
!107 = !DILocation(line: 50, column: 16, scope: !79)
!108 = !DILocation(line: 51, column: 27, scope: !79)
!109 = !DILocation(line: 52, column: 16, scope: !79)
!110 = !DILocation(line: 53, column: 14, scope: !79)
!111 = !DILocation(line: 54, column: 23, scope: !79)
!112 = !DILocation(line: 55, column: 12, scope: !79)
!113 = !DILocation(line: 57, column: 10, scope: !79)
!114 = distinct !DISubprogram(name: "print_arr", linkageName: "arr_mod_mp_print_arr_", scope: !2, file: !3, line: 59, type: !26, scopeLine: 59, spFlags: DISPFlagDefinition, unit: !20, retainedNodes: !115)
!115 = !{!116, !117, !118, !119}
!116 = !DILocalVariable(name: "i", arg: 1, scope: !114, file: !3, line: 59, type: !30)
!117 = !DILocalVariable(name: "n", arg: 2, scope: !114, file: !3, line: 59, type: !30)
!118 = !DILocalVariable(name: "m", arg: 3, scope: !114, file: !3, line: 59, type: !30)
!119 = !DILocalVariable(name: "o", arg: 4, scope: !114, file: !3, line: 59, type: !30)
!120 = !DILocation(line: 59, column: 31, scope: !114)
!121 = !DILocation(line: 59, column: 34, scope: !114)
!122 = !DILocation(line: 59, column: 37, scope: !114)
!123 = !DILocation(line: 59, column: 40, scope: !114)
!124 = !DILocation(line: 62, column: 12, scope: !114)
!125 = !DILocation(line: 62, column: 17, scope: !114)
!126 = !DILocation(line: 66, column: 10, scope: !114)
!127 = !DILocation(line: 70, column: 15, scope: !25)
!128 = !DILocation(line: 0, scope: !25)
!129 = !DILocation(line: 76, column: 9, scope: !25)
!130 = !DILocation(line: 80, column: 9, scope: !25)
!131 = !DILocation(line: 77, column: 16, scope: !25)
!132 = !DILocation(line: 78, column: 16, scope: !25)
!133 = !DILocation(line: 79, column: 16, scope: !25)
!134 = !DILocation(line: 82, column: 7, scope: !25)

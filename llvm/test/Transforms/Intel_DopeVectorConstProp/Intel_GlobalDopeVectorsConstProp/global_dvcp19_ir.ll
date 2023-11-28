; RUN: opt  < %s -passes=dopevectorconstprop -dope-vector-global-const-prop=true -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s

; This test case checks that the information for the global dope vector
; and the nested dope vectors were collected and propagated correctly even
; if the nested dope vectors were parameters to a function. It was created
; from the following source code:

;      MODULE ARR_MOD
;
;         TYPE T_TESTTYPE
;
;           REAL, POINTER :: inner_array_A(:,:)
;           REAL, POINTER :: inner_array_B(:,:)
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
;           ALLOCATE(A(I) % inner_array_B(10, 10))
;
;           RETURN
;         END SUBROUTINE ALLOCATE_ARR
;
;         SUBROUTINE INITIALIZE_ARR(I, N, M)
;           INTEGER, INTENT(in) :: I, N, M
;
;           DO j = 1, N
;             DO k = 1, M
;               A(I) % inner_array_A(k, j) = j
;               A(I) % inner_array_B(k, j) = j
;             END DO
;           END DO
;
;           RETURN
;         END SUBROUTINE INITIALIZE_ARR
;
;         SUBROUTINE PRINT_INFO(N, M, ARRAY_IN)
;           INTEGER, INTENT(IN) :: N, M
;           REAL, INTENT(IN) :: ARRAY_IN(:,:)
;
;           DO j = 1, N
;             DO k = 1, M
;               print *, ARRAY_IN(k, j)
;             END DO
;           END DO
;           RETURN
;         END SUBROUTINE
;
;         SUBROUTINE PRINT_ARR(I, N, M)
;           INTEGER, INTENT(IN) :: I, N, M
;
;           CALL PRINT_INFO(N, M, A(I) % inner_array_A)
;           CALL PRINT_INFO(N, M, A(I) % inner_array_B)
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
;          CALL INITIALIZE_ARR(I, 10, 10)
;          CALL PRINT_ARR(I, 10, 10)
;        END DO
;
;      END

; ifx -xCORE-AVX512 -Ofast -flto arr.f90 -mllvm -hir-details-dims -g -mllvm -print-debug-loc

; The global array A is allocated in ALLOCATE_ARR and the initialized in
; INITIALIZE_ARR. Function PRINT_ARR will call PRINT_INFO twice. The first call
; passes field inner_array_A, and the second call passes inner_array_B. The
; goal of this test case is to make sure that the load instructions inside
; PRINT_INFO weren't modified since they are loading two different dope
; vectors, but it doesn't affect the constant propagation of inner_array_A
; and inner_array_B in the function ALLOCATE_ARR, INITIALIZE_ARR, and PRINT_ARR.
; This test case includes debug information because the FE will copy the dope
; vectors that are arguments into local dope vectors and these local dope
; vectors will be used as arguments for the inner calls.

; We are interested in the function @arr_mod_mp_print_info_ since it has
; the copy dope vector as parameter.

; Check that constant propagation was applied to function
; @arr_mod_mp_allocate_arr_, @arr_mod_mp_initialize_arr_ and
; @arr_mod_mp_print_arr_, but not for function @arr_mod_mp_print_info_.

; CHECK: define internal void @arr_mod_mp_allocate_arr_
; CHECK:   %i38 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 192, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i35, i64 %i37)
; CHECK:   %i57 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 192, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i55, i64 %i37)

; CHECK: define internal void @arr_mod_mp_initialize_arr_
; CHECK:   %i7 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 192, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i, i64 %i6)
; CHECK:   %i37 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i36, i64 %i23)
; CHECK:   %i38 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i37, i64 %i35)
; CHECK:   %i40 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i39, i64 %i23)
; CHECK:   %i41 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i40, i64 %i35)

; Check that constant propagation wasn't applied to function @arr_mod_mp_print_info_
; CHECK: define internal void @arr_mod_mp_print_info_
; CHECK-NOT: %i21 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i6, i64 %i20)
; CHECK-NOT: %i24 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i21, i64 %i23)

; CHECK: define internal void @arr_mod_mp_print_arr_
; CHECK:   %i9 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 192, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i4, i64 %i8)
; CHECK:   store i64 10, ptr %i31, align 1
; CHECK:   store i64 40, ptr %i32, align 1
; CHECK:   store i64 10, ptr %i34, align 1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"ARR_MOD$.btT_TESTTYPE" = type <{ %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$" }>
%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@arr_mod_mp_a_ = internal global %"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$" { ptr null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }, !dbg !0
@anon.11934500c7ed222a0a148892d04ea3ac.0 = internal unnamed_addr constant i32 65536
@anon.11934500c7ed222a0a148892d04ea3ac.1 = internal unnamed_addr constant i32 2
@anon.11934500c7ed222a0a148892d04ea3ac.2 = internal unnamed_addr constant i32 10

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_allocate_arr_(ptr noalias nocapture readonly dereferenceable(4) %arg) #0 !dbg !30 {
bb:
  %i = alloca i64, align 8, !dbg !33
  call void @llvm.dbg.declare(metadata ptr %arg, metadata !32, metadata !DIExpression()), !dbg !33
  %i1 = load i32, ptr %arg, align 1, !dbg !34, !tbaa !35
  %i2 = icmp eq i32 %i1, 1, !dbg !39
  br i1 %i2, label %bb5, label %bb3, !dbg !39

bb3:                                              ; preds = %bb
  %i4 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0), !dbg !40
  br label %bb33, !dbg !39

bb5:                                              ; preds = %bb
  %i6 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8, !dbg !41, !tbaa !42
  %i7 = and i64 %i6, 1030792151296, !dbg !41
  %i8 = or i64 %i7, 133, !dbg !41
  store i64 %i8, ptr getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8, !dbg !41, !tbaa !42
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 5), align 8, !dbg !41, !tbaa !45
  store i64 192, ptr getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 1), align 8, !dbg !41, !tbaa !46
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 4), align 16, !dbg !41, !tbaa !47
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 2), align 16, !dbg !41, !tbaa !48
  %i9 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0), !dbg !41
  store i64 1, ptr %i9, align 1, !dbg !41, !tbaa !49
  %i10 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 0), i32 0), !dbg !41
  store i64 1, ptr %i10, align 1, !dbg !41, !tbaa !50
  %i11 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 0), !dbg !41
  store i64 192, ptr %i11, align 1, !dbg !41, !tbaa !51
  %i12 = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %i, i32 2, i64 1, i64 192) #6, !dbg !41
  %i13 = load i64, ptr %i, align 8, !dbg !41, !tbaa !52
  %i14 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8, !dbg !41, !tbaa !42
  %i15 = and i64 %i14, -68451041281, !dbg !41
  %i16 = or i64 %i15, 1073741824, !dbg !41
  store i64 %i16, ptr getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8, !dbg !41, !tbaa !42
  %i17 = trunc i64 %i14 to i32, !dbg !41
  %i18 = shl i32 %i17, 1, !dbg !41
  %i19 = and i32 %i18, 2, !dbg !41
  %i20 = shl i32 %i12, 4, !dbg !41
  %i21 = and i32 %i20, 16, !dbg !41
  %i22 = lshr i64 %i14, 15, !dbg !41
  %i23 = trunc i64 %i22 to i32, !dbg !41
  %i24 = and i32 %i23, 31457280, !dbg !41
  %i25 = and i32 %i23, 33554432, !dbg !41
  %i26 = or i32 %i21, %i19, !dbg !41
  %i27 = or i32 %i26, %i24, !dbg !41
  %i28 = or i32 %i27, %i25, !dbg !41
  %i29 = or i32 %i28, 262144, !dbg !41
  %i30 = load i64, ptr getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 5), align 8, !dbg !41, !tbaa !45
  %i31 = inttoptr i64 %i30 to ptr, !dbg !41
  %i32 = tail call i32 @for_alloc_allocatable_handle(i64 %i13, ptr @arr_mod_mp_a_, i32 %i29, ptr %i31) #6, !dbg !41
  br label %bb33, !dbg !39

bb33:                                             ; preds = %bb5, %bb3
  %i34 = phi ptr [ %i4, %bb3 ], [ %i9, %bb5 ], !dbg !40
  %i35 = load ptr, ptr @arr_mod_mp_a_, align 16, !dbg !40, !tbaa !53
  %i36 = load i64, ptr %i34, align 1, !dbg !40, !tbaa !49
  %i37 = sext i32 %i1 to i64, !dbg !40
  %i38 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i36, i64 192, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i35, i64 %i37), !dbg !40
  %i39 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 3, !dbg !54
  %i40 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 5, !dbg !40
  store i64 0, ptr %i40, align 1, !dbg !40, !tbaa !55
  %i41 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 1, !dbg !40
  store i64 4, ptr %i41, align 1, !dbg !40, !tbaa !57
  %i42 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 4, !dbg !40
  store i64 2, ptr %i42, align 1, !dbg !40, !tbaa !58
  %i43 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 2, !dbg !40
  store i64 0, ptr %i43, align 1, !dbg !40, !tbaa !59
  %i44 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 6, i64 0, i32 2, !dbg !40
  %i45 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i44, i32 0), !dbg !40
  store i64 1, ptr %i45, align 1, !dbg !40, !tbaa !60
  %i46 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 6, i64 0, i32 0, !dbg !40
  %i47 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i46, i32 0), !dbg !40
  store i64 10, ptr %i47, align 1, !dbg !40, !tbaa !61
  %i48 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i44, i32 1), !dbg !40
  store i64 1, ptr %i48, align 1, !dbg !40, !tbaa !60
  %i49 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i46, i32 1), !dbg !40
  store i64 10, ptr %i49, align 1, !dbg !40, !tbaa !61
  %i50 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 0, i32 6, i64 0, i32 1, !dbg !40
  %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i50, i32 0), !dbg !40
  store i64 4, ptr %i51, align 1, !dbg !40, !tbaa !62
  %i52 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i50, i32 1), !dbg !40
  store i64 40, ptr %i52, align 1, !dbg !40, !tbaa !62
  store i64 1073741829, ptr %i39, align 1, !dbg !40, !tbaa !63
  %i53 = bitcast ptr %i38 to ptr, !dbg !40
  %i54 = tail call i32 @for_allocate_handle(i64 400, ptr %i53, i32 262144, ptr null) #6, !dbg !40
  %i55 = load ptr, ptr @arr_mod_mp_a_, align 16, !dbg !64, !tbaa !53
  %i56 = load i64, ptr %i34, align 1, !dbg !64, !tbaa !49
  %i57 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i56, i64 192, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i55, i64 %i37), !dbg !64
  %i58 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 3, !dbg !65
  %i59 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 5, !dbg !64
  store i64 0, ptr %i59, align 1, !dbg !64, !tbaa !66
  %i60 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 1, !dbg !64
  store i64 4, ptr %i60, align 1, !dbg !64, !tbaa !68
  %i61 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 4, !dbg !64
  store i64 2, ptr %i61, align 1, !dbg !64, !tbaa !69
  %i62 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 2, !dbg !64
  store i64 0, ptr %i62, align 1, !dbg !64, !tbaa !70
  %i63 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 6, i64 0, i32 2, !dbg !64
  %i64 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i63, i32 0), !dbg !64
  store i64 1, ptr %i64, align 1, !dbg !64, !tbaa !71
  %i65 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 6, i64 0, i32 0, !dbg !64
  %i66 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i65, i32 0), !dbg !64
  store i64 10, ptr %i66, align 1, !dbg !64, !tbaa !72
  %i67 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i63, i32 1), !dbg !64
  store i64 1, ptr %i67, align 1, !dbg !64, !tbaa !71
  %i68 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i65, i32 1), !dbg !64
  store i64 10, ptr %i68, align 1, !dbg !64, !tbaa !72
  %i69 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 6, i64 0, i32 1, !dbg !64
  %i70 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i69, i32 0), !dbg !64
  store i64 4, ptr %i70, align 1, !dbg !64, !tbaa !73
  %i71 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i69, i32 1), !dbg !64
  store i64 40, ptr %i71, align 1, !dbg !64, !tbaa !73
  %i72 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i57, i64 0, i32 1, i32 0, !dbg !64
  store i64 1073741829, ptr %i58, align 1, !dbg !64, !tbaa !74
  %i73 = bitcast ptr %i72 to ptr, !dbg !64
  %i74 = tail call i32 @for_allocate_handle(i64 400, ptr nonnull %i73, i32 262144, ptr null) #6, !dbg !64
  ret void, !dbg !75
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
define internal void @arr_mod_mp_initialize_arr_(ptr noalias nocapture readonly dereferenceable(4) %arg, ptr noalias nocapture readonly dereferenceable(4) %arg1, ptr noalias nocapture readonly dereferenceable(4) %arg2) #3 !dbg !76 {
bb:
  call void @llvm.dbg.declare(metadata ptr %arg, metadata !78, metadata !DIExpression()), !dbg !83
  call void @llvm.dbg.declare(metadata ptr @anon.11934500c7ed222a0a148892d04ea3ac.2, metadata !79, metadata !DIExpression()), !dbg !84
  call void @llvm.dbg.declare(metadata ptr @anon.11934500c7ed222a0a148892d04ea3ac.2, metadata !80, metadata !DIExpression()), !dbg !85
  call void @llvm.dbg.value(metadata i32 1, metadata !82, metadata !DIExpression()), !dbg !86
  %i = load ptr, ptr @arr_mod_mp_a_, align 16, !dbg !87
  %i3 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0), !dbg !87
  %i4 = load i64, ptr %i3, align 1, !dbg !87
  %i5 = load i32, ptr %arg, align 1, !dbg !87
  %i6 = sext i32 %i5 to i64, !dbg !87
  %i7 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i4, i64 192, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i, i64 %i6), !dbg !87
  %i8 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i7, i64 0, i32 0, i32 0, !dbg !88
  %i9 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i7, i64 0, i32 0, i32 6, i64 0, i32 1, !dbg !88
  %i10 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i9, i32 0), !dbg !88
  %i11 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i7, i64 0, i32 0, i32 6, i64 0, i32 2, !dbg !88
  %i12 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i11, i32 0), !dbg !88
  %i13 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i9, i32 1), !dbg !88
  %i14 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i11, i32 1), !dbg !88
  %i15 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i7, i64 0, i32 1, i32 0, !dbg !89
  %i16 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i7, i64 0, i32 1, i32 6, i64 0, i32 1, !dbg !89
  %i17 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i16, i32 0), !dbg !89
  %i18 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i7, i64 0, i32 1, i32 6, i64 0, i32 2, !dbg !89
  %i19 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i18, i32 0), !dbg !89
  %i20 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i16, i32 1), !dbg !89
  %i21 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i18, i32 1), !dbg !89
  br label %bb22, !dbg !90

bb22:                                             ; preds = %bb44, %bb
  %i23 = phi i64 [ 1, %bb ], [ %i45, %bb44 ]
  call void @llvm.dbg.value(metadata i64 %i23, metadata !82, metadata !DIExpression()), !dbg !86
  call void @llvm.dbg.value(metadata i32 1, metadata !81, metadata !DIExpression()), !dbg !86
  %i24 = trunc i64 %i23 to i32, !dbg !87
  %i25 = sitofp i32 %i24 to float, !dbg !87
  %i26 = load i64, ptr %i10, align 1, !dbg !88, !tbaa !91
  %i27 = load i64, ptr %i12, align 1, !dbg !88, !tbaa !96
  %i28 = load i64, ptr %i13, align 1, !dbg !88, !tbaa !91
  %i29 = load i64, ptr %i14, align 1, !dbg !88, !tbaa !96
  %i30 = load i64, ptr %i17, align 1, !dbg !89, !tbaa !97
  %i31 = load i64, ptr %i19, align 1, !dbg !89, !tbaa !99
  %i32 = load i64, ptr %i20, align 1, !dbg !89, !tbaa !97
  %i33 = load i64, ptr %i21, align 1, !dbg !89, !tbaa !99
  br label %bb34, !dbg !100

bb34:                                             ; preds = %bb34, %bb22
  %i35 = phi i64 [ 1, %bb22 ], [ %i42, %bb34 ]
  call void @llvm.dbg.value(metadata i64 %i35, metadata !81, metadata !DIExpression()), !dbg !86
  %i36 = load ptr, ptr %i8, align 1, !dbg !88, !tbaa !101
  %i37 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i29, i64 %i28, ptr elementtype(float) %i36, i64 %i23), !dbg !88
  %i38 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i27, i64 %i26, ptr elementtype(float) %i37, i64 %i35), !dbg !88
  store float %i25, ptr %i38, align 1, !dbg !102, !tbaa !101
  %i39 = load ptr, ptr %i15, align 1, !dbg !89, !tbaa !103
  %i40 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i33, i64 %i32, ptr elementtype(float) %i39, i64 %i23), !dbg !89
  %i41 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i31, i64 %i30, ptr elementtype(float) %i40, i64 %i35), !dbg !89
  store float %i25, ptr %i41, align 1, !dbg !104, !tbaa !103
  %i42 = add nuw nsw i64 %i35, 1, !dbg !100
  call void @llvm.dbg.value(metadata i64 %i42, metadata !81, metadata !DIExpression()), !dbg !86
  %i43 = icmp eq i64 %i42, 11, !dbg !100
  br i1 %i43, label %bb44, label %bb34, !dbg !100

bb44:                                             ; preds = %bb34
  %i45 = add nuw nsw i64 %i23, 1, !dbg !105
  call void @llvm.dbg.value(metadata i64 %i45, metadata !82, metadata !DIExpression()), !dbg !86
  %i46 = icmp eq i64 %i45, 11, !dbg !105
  br i1 %i46, label %bb47, label %bb22, !dbg !105

bb47:                                             ; preds = %bb44
  ret void, !dbg !106
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_print_info_(ptr noalias nocapture readonly dereferenceable(4) %arg, ptr noalias nocapture readonly dereferenceable(4) %arg1, ptr noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %arg2) #0 !dbg !107 {
bb:
  %i = alloca [8 x i64], align 16
  %i3 = alloca [4 x i8], align 1, !dbg !115
  %i4 = alloca <{ i64 }>, align 8, !dbg !115
  call void @llvm.dbg.declare(metadata ptr @anon.11934500c7ed222a0a148892d04ea3ac.2, metadata !109, metadata !DIExpression()), !dbg !115
  call void @llvm.dbg.declare(metadata ptr @anon.11934500c7ed222a0a148892d04ea3ac.2, metadata !110, metadata !DIExpression()), !dbg !116
  call void @llvm.dbg.declare(metadata ptr %arg2, metadata !111, metadata !DIExpression()), !dbg !117
  call void @llvm.dbg.value(metadata i32 1, metadata !114, metadata !DIExpression()), !dbg !118
  %i5 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %arg2, i64 0, i32 0, !dbg !119
  %i6 = load ptr, ptr %i5, align 1, !dbg !119
  %i7 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %arg2, i64 0, i32 6, i64 0, i32 1, !dbg !119
  %i8 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i7, i32 0), !dbg !119
  %i9 = load i64, ptr %i8, align 1, !dbg !119
  %i10 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i7, i32 1), !dbg !119
  %i11 = load i64, ptr %i10, align 1, !dbg !119
  %i12 = getelementptr inbounds [4 x i8], ptr %i3, i64 0, i64 0, !dbg !119
  %i13 = getelementptr inbounds [4 x i8], ptr %i3, i64 0, i64 1, !dbg !119
  %i14 = getelementptr inbounds [4 x i8], ptr %i3, i64 0, i64 2, !dbg !119
  %i15 = getelementptr inbounds [4 x i8], ptr %i3, i64 0, i64 3, !dbg !119
  %i16 = bitcast ptr %i4 to ptr, !dbg !119
  %i17 = bitcast ptr %i to ptr, !dbg !119
  %i18 = bitcast ptr %i4 to ptr, !dbg !119
  br label %bb19, !dbg !120

bb19:                                             ; preds = %bb30, %bb
  %i20 = phi i64 [ 1, %bb ], [ %i31, %bb30 ]
  call void @llvm.dbg.value(metadata i64 %i20, metadata !114, metadata !DIExpression()), !dbg !118
  call void @llvm.dbg.value(metadata i32 1, metadata !113, metadata !DIExpression()), !dbg !118
  %i21 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i11, ptr elementtype(float) %i6, i64 %i20), !dbg !119
  br label %bb22, !dbg !121

bb22:                                             ; preds = %bb22, %bb19
  %i23 = phi i64 [ 1, %bb19 ], [ %i27, %bb22 ]
  call void @llvm.dbg.value(metadata i64 %i23, metadata !113, metadata !DIExpression()), !dbg !118
  %i24 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i9, ptr elementtype(float) %i21, i64 %i23), !dbg !119
  %i25 = load float, ptr %i24, align 1, !dbg !119, !tbaa !122
  store i8 26, ptr %i12, align 1, !dbg !119, !tbaa !127
  store i8 1, ptr %i13, align 1, !dbg !119, !tbaa !127
  store i8 1, ptr %i14, align 1, !dbg !119, !tbaa !127
  store i8 0, ptr %i15, align 1, !dbg !119, !tbaa !127
  store float %i25, ptr %i16, align 8, !dbg !119, !tbaa !128
  %i26 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i17, i32 -1, i64 1239157112576, ptr nonnull %i12, ptr nonnull %i18) #6, !dbg !119
  %i27 = add nuw i64 %i23, 1, !dbg !121
  call void @llvm.dbg.value(metadata i64 %i27, metadata !113, metadata !DIExpression()), !dbg !118
  %i28 = trunc i64 %i27 to i32, !dbg !121
  %i29 = icmp slt i32 10, %i28, !dbg !121
  br i1 %i29, label %bb30, label %bb22, !dbg !121

bb30:                                             ; preds = %bb22
  %i31 = add nuw i64 %i20, 1, !dbg !130
  call void @llvm.dbg.value(metadata i64 %i31, metadata !114, metadata !DIExpression()), !dbg !118
  %i32 = trunc i64 %i31 to i32, !dbg !130
  %i33 = icmp slt i32 10, %i32, !dbg !130
  br i1 %i33, label %bb34, label %bb19, !dbg !130

bb34:                                             ; preds = %bb30
  ret void, !dbg !131
}

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr #2

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_print_arr_(ptr noalias nocapture readonly dereferenceable(4) %arg, ptr noalias nocapture readonly dereferenceable(4) %arg1, ptr noalias nocapture readonly dereferenceable(4) %arg2) #0 !dbg !132 {
bb:
  %i = alloca %"QNCA_a0$float*$rank2$", align 8, !dbg !137
  %i3 = alloca %"QNCA_a0$float*$rank2$", align 8, !dbg !137
  call void @llvm.dbg.declare(metadata ptr %arg, metadata !134, metadata !DIExpression()), !dbg !137
  call void @llvm.dbg.declare(metadata ptr @anon.11934500c7ed222a0a148892d04ea3ac.2, metadata !135, metadata !DIExpression()), !dbg !138
  call void @llvm.dbg.declare(metadata ptr @anon.11934500c7ed222a0a148892d04ea3ac.2, metadata !136, metadata !DIExpression()), !dbg !139
  %i4 = load ptr, ptr @arr_mod_mp_a_, align 16, !dbg !140, !tbaa !141
  %i5 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0), !dbg !140
  %i6 = load i64, ptr %i5, align 1, !dbg !140, !tbaa !146
  %i7 = load i32, ptr %arg, align 1, !dbg !140, !tbaa !147
  %i8 = sext i32 %i7 to i64, !dbg !140
  %i9 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i6, i64 192, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i4, i64 %i8), !dbg !140
  %i10 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i9, i64 0, i32 0, i32 0, !dbg !140
  %i11 = load ptr, ptr %i10, align 1, !dbg !140, !tbaa !149
  %i12 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i9, i64 0, i32 0, i32 6, i64 0, i32 1, !dbg !140
  %i13 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i12, i32 0), !dbg !140
  %i14 = load i64, ptr %i13, align 1, !dbg !140, !tbaa !151
  %i15 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i9, i64 0, i32 0, i32 6, i64 0, i32 0, !dbg !140
  %i16 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i15, i32 0), !dbg !140
  %i17 = load i64, ptr %i16, align 1, !dbg !140, !tbaa !152
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i12, i32 1), !dbg !140
  %i19 = load i64, ptr %i18, align 1, !dbg !140, !tbaa !151
  %i20 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i15, i32 1), !dbg !140
  %i21 = load i64, ptr %i20, align 1, !dbg !140, !tbaa !152
  %i22 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 3, !dbg !140
  %i23 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 1, !dbg !140
  store i64 4, ptr %i23, align 8, !dbg !140, !tbaa !153
  %i24 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 4, !dbg !140
  store i64 2, ptr %i24, align 8, !dbg !140, !tbaa !155
  %i25 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 2, !dbg !140
  store i64 0, ptr %i25, align 8, !dbg !140, !tbaa !156
  %i26 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 6, i64 0, i32 1, !dbg !140
  %i27 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i26, i32 0), !dbg !140
  store i64 %i14, ptr %i27, align 1, !dbg !140, !tbaa !157
  %i28 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 6, i64 0, i32 2, !dbg !140
  %i29 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i28, i32 0), !dbg !140
  store i64 1, ptr %i29, align 1, !dbg !140, !tbaa !158
  %i30 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 6, i64 0, i32 0, !dbg !140
  %i31 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i30, i32 0), !dbg !140
  store i64 %i17, ptr %i31, align 1, !dbg !140, !tbaa !159
  %i32 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i26, i32 1), !dbg !140
  store i64 %i19, ptr %i32, align 1, !dbg !140, !tbaa !157
  %i33 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i28, i32 1), !dbg !140
  store i64 1, ptr %i33, align 1, !dbg !140, !tbaa !158
  %i34 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i30, i32 1), !dbg !140
  store i64 %i21, ptr %i34, align 1, !dbg !140, !tbaa !159
  %i35 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 0, !dbg !140
  store ptr %i11, ptr %i35, align 8, !dbg !140, !tbaa !160
  store i64 1, ptr %i22, align 8, !dbg !140, !tbaa !161
  call void @arr_mod_mp_print_info_(ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.2, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.2, ptr nonnull %i) #7, !dbg !162
  %i36 = load ptr, ptr @arr_mod_mp_a_, align 16, !dbg !163, !tbaa !141
  %i37 = load i64, ptr %i5, align 1, !dbg !163, !tbaa !146
  %i38 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i37, i64 192, ptr elementtype(%"ARR_MOD$.btT_TESTTYPE") %i36, i64 %i8), !dbg !163
  %i39 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 1, i32 0, !dbg !163
  %i40 = load ptr, ptr %i39, align 1, !dbg !163, !tbaa !164
  %i41 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 1, i32 6, i64 0, i32 1, !dbg !163
  %i42 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i41, i32 0), !dbg !163
  %i43 = load i64, ptr %i42, align 1, !dbg !163, !tbaa !166
  %i44 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", ptr %i38, i64 0, i32 1, i32 6, i64 0, i32 0, !dbg !163
  %i45 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i44, i32 0), !dbg !163
  %i46 = load i64, ptr %i45, align 1, !dbg !163, !tbaa !167
  %i47 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i41, i32 1), !dbg !163
  %i48 = load i64, ptr %i47, align 1, !dbg !163, !tbaa !166
  %i49 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i44, i32 1), !dbg !163
  %i50 = load i64, ptr %i49, align 1, !dbg !163, !tbaa !167
  %i51 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i3, i64 0, i32 3, !dbg !163
  %i52 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i3, i64 0, i32 1, !dbg !163
  store i64 4, ptr %i52, align 8, !dbg !163, !tbaa !168
  %i53 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i3, i64 0, i32 4, !dbg !163
  store i64 2, ptr %i53, align 8, !dbg !163, !tbaa !170
  %i54 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i3, i64 0, i32 2, !dbg !163
  store i64 0, ptr %i54, align 8, !dbg !163, !tbaa !171
  %i55 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i3, i64 0, i32 6, i64 0, i32 1, !dbg !163
  %i56 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i55, i32 0), !dbg !163
  store i64 %i43, ptr %i56, align 1, !dbg !163, !tbaa !172
  %i57 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i3, i64 0, i32 6, i64 0, i32 2, !dbg !163
  %i58 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i57, i32 0), !dbg !163
  store i64 1, ptr %i58, align 1, !dbg !163, !tbaa !173
  %i59 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i3, i64 0, i32 6, i64 0, i32 0, !dbg !163
  %i60 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i59, i32 0), !dbg !163
  store i64 %i46, ptr %i60, align 1, !dbg !163, !tbaa !174
  %i61 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i55, i32 1), !dbg !163
  store i64 %i48, ptr %i61, align 1, !dbg !163, !tbaa !172
  %i62 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i57, i32 1), !dbg !163
  store i64 1, ptr %i62, align 1, !dbg !163, !tbaa !173
  %i63 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i59, i32 1), !dbg !163
  store i64 %i50, ptr %i63, align 1, !dbg !163, !tbaa !174
  %i64 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i3, i64 0, i32 0, !dbg !163
  store ptr %i40, ptr %i64, align 8, !dbg !163, !tbaa !175
  store i64 1, ptr %i51, align 8, !dbg !163, !tbaa !176
  call void @arr_mod_mp_print_info_(ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.2, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.2, ptr nonnull %i3) #7, !dbg !177
  ret void, !dbg !178
}

; Function Attrs: nounwind uwtable
define dso_local void @MAIN__() #4 !dbg !19 {
bb:
  %i = alloca i32, align 8
  %i1 = tail call i32 @for_set_fpe_(ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.0) #6, !dbg !179
  %i2 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1) #6, !dbg !179
  call void @llvm.dbg.value(metadata i32 1, metadata !23, metadata !DIExpression()), !dbg !180
  store i32 1, ptr %i, align 8, !dbg !181, !tbaa !182
  br label %bb3, !dbg !181

bb3:                                              ; preds = %bb3, %bb
  %i4 = phi i32 [ %i5, %bb3 ], [ 1, %bb ], !dbg !186
  call void @llvm.dbg.value(metadata ptr %i, metadata !23, metadata !DIExpression(DW_OP_deref)), !dbg !180
  call void @arr_mod_mp_allocate_arr_(ptr nonnull %i) #7, !dbg !187
  call void @arr_mod_mp_initialize_arr_(ptr nonnull %i, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.2, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.2) #7, !dbg !188
  call void @arr_mod_mp_print_arr_(ptr nonnull %i, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.2, ptr nonnull @anon.11934500c7ed222a0a148892d04ea3ac.2) #7, !dbg !189
  call void @llvm.dbg.value(metadata i32 %i4, metadata !23, metadata !DIExpression()), !dbg !180
  %i5 = add nuw nsw i32 %i4, 1, !dbg !186
  call void @llvm.dbg.value(metadata i32 %i5, metadata !23, metadata !DIExpression()), !dbg !180
  store i32 %i5, ptr %i, align 8, !dbg !186, !tbaa !182
  %i6 = icmp eq i32 %i5, 11, !dbg !186
  br i1 %i6, label %bb7, label %bb3, !dbg !186

bb7:                                              ; preds = %bb3
  ret void, !dbg !190
}

declare dso_local i32 @for_set_fpe_(ptr nocapture readonly) local_unnamed_addr

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #2

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #5

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #5

attributes #0 = { nofree nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { nofree nosync nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #4 = { nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #5 = { nounwind readnone speculatable }
attributes #6 = { nounwind }
attributes #7 = { noinline }

!llvm.dbg.cu = !{!15}
!omp_offload.info = !{}
!llvm.module.flags = !{!25, !26, !27, !28, !29}
!ifx.types.dv = !{!191, !192}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "a", linkageName: "arr_mod_mp_a_", scope: !2, file: !3, line: 10, type: !4, isLocal: false, isDefinition: true)
!2 = !DIModule(scope: null, name: "arr_mod", file: !3, line: 1)
!3 = !DIFile(filename: "test.f90", directory: "/localdisk2/ayrivera/dev-debug-issue/llvm/llvm/test/Transforms/Intel_DopeVectorConstProp/Intel_GlobalDopeVectorsConstProp")
!4 = !DICompositeType(tag: DW_TAG_array_type, baseType: !5, elements: !14, dataLocation: !DIExpression(DW_OP_push_object_address, DW_OP_deref), allocated: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 24, DW_OP_deref, DW_OP_constu, 1, DW_OP_and))
!5 = !DICompositeType(tag: DW_TAG_structure_type, name: "t_testtype", scope: !2, file: !3, line: 3, size: 1536, elements: !6)
!6 = !{!7, !13}
!7 = !DIDerivedType(tag: DW_TAG_member, name: "inner_array_a", scope: !5, file: !3, line: 3, baseType: !8, flags: DIFlagPublic)
!8 = !DICompositeType(tag: DW_TAG_array_type, baseType: !9, elements: !10, dataLocation: !DIExpression(DW_OP_push_object_address, DW_OP_deref), associated: !DIExpression(DW_OP_push_object_address, DW_OP_deref, DW_OP_constu, 0, DW_OP_or))
!9 = !DIBasicType(name: "REAL*4", size: 32, encoding: DW_ATE_float)
!10 = !{!11, !12}
!11 = !DISubrange(lowerBound: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 64, DW_OP_deref), upperBound: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 64, DW_OP_deref, DW_OP_push_object_address, DW_OP_plus_uconst, 48, DW_OP_deref, DW_OP_plus, DW_OP_constu, 1, DW_OP_minus), stride: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 56, DW_OP_deref))
!12 = !DISubrange(lowerBound: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 88, DW_OP_deref), upperBound: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 88, DW_OP_deref, DW_OP_push_object_address, DW_OP_plus_uconst, 72, DW_OP_deref, DW_OP_plus, DW_OP_constu, 1, DW_OP_minus), stride: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 80, DW_OP_deref))
!13 = !DIDerivedType(tag: DW_TAG_member, name: "inner_array_b", scope: !5, file: !3, line: 3, baseType: !8, offset: 768, flags: DIFlagPublic)
!14 = !{!11}
!15 = distinct !DICompileUnit(language: DW_LANG_Fortran95, file: !3, producer: "Intel(R) Fortran 22.0-1245", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !16, imports: !17, splitDebugInlining: false, nameTableKind: None)
!16 = !{!0}
!17 = !{!18}
!18 = !DIImportedEntity(tag: DW_TAG_imported_module, scope: !19, entity: !2, file: !3, line: 65)
!19 = distinct !DISubprogram(name: "main", linkageName: "MAIN__", scope: !3, file: !3, line: 64, type: !20, scopeLine: 64, spFlags: DISPFlagDefinition | DISPFlagMainSubprogram, unit: !15, retainedNodes: !22)
!20 = !DISubroutineType(types: !21)
!21 = !{null}
!22 = !{!23}
!23 = !DILocalVariable(name: "i", scope: !19, file: !3, line: 68, type: !24)
!24 = !DIBasicType(name: "INTEGER*4", size: 32, encoding: DW_ATE_signed)
!25 = !{i32 2, !"Debug Info Version", i32 3}
!26 = !{i32 2, !"Dwarf Version", i32 4}
!27 = !{i32 1, !"ThinLTO", i32 0}
!28 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!29 = !{i32 1, !"LTOPostLink", i32 1}
!30 = distinct !DISubprogram(name: "allocate_arr", linkageName: "arr_mod_mp_allocate_arr_", scope: !2, file: !3, line: 14, type: !20, scopeLine: 14, spFlags: DISPFlagDefinition, unit: !15, retainedNodes: !31)
!31 = !{!32}
!32 = !DILocalVariable(name: "i", arg: 1, scope: !30, file: !3, line: 14, type: !24)
!33 = !DILocation(line: 14, column: 34, scope: !30)
!34 = !DILocation(line: 17, column: 12, scope: !30)
!35 = !{!36, !36, i64 0}
!36 = !{!"ifx$unique_sym$1", !37, i64 0}
!37 = !{!"Generic Fortran Symbol", !38, i64 0}
!38 = !{!"ifx$root$1$arr_mod_mp_allocate_arr_"}
!39 = !DILocation(line: 17, column: 16, scope: !30)
!40 = !DILocation(line: 19, column: 12, scope: !30)
!41 = !DILocation(line: 17, column: 23, scope: !30)
!42 = !{!43, !44, i64 24}
!43 = !{!"ifx$descr$1", !44, i64 0, !44, i64 8, !44, i64 16, !44, i64 24, !44, i64 32, !44, i64 40, !44, i64 48, !44, i64 56, !44, i64 64}
!44 = !{!"ifx$descr$field", !37, i64 0}
!45 = !{!43, !44, i64 40}
!46 = !{!43, !44, i64 8}
!47 = !{!43, !44, i64 32}
!48 = !{!43, !44, i64 16}
!49 = !{!43, !44, i64 64}
!50 = !{!43, !44, i64 48}
!51 = !{!43, !44, i64 56}
!52 = !{!37, !37, i64 0}
!53 = !{!43, !44, i64 0}
!54 = !DILocation(line: 19, column: 21, scope: !30)
!55 = !{!56, !44, i64 40}
!56 = !{!"ifx$descr$2", !44, i64 0, !44, i64 8, !44, i64 16, !44, i64 24, !44, i64 32, !44, i64 40, !44, i64 48, !44, i64 56, !44, i64 64, !44, i64 72, !44, i64 80, !44, i64 88}
!57 = !{!56, !44, i64 8}
!58 = !{!56, !44, i64 32}
!59 = !{!56, !44, i64 16}
!60 = !{!56, !44, i64 64}
!61 = !{!56, !44, i64 48}
!62 = !{!56, !44, i64 56}
!63 = !{!56, !44, i64 24}
!64 = !DILocation(line: 20, column: 12, scope: !30)
!65 = !DILocation(line: 20, column: 21, scope: !30)
!66 = !{!67, !44, i64 40}
!67 = !{!"ifx$descr$3", !44, i64 0, !44, i64 8, !44, i64 16, !44, i64 24, !44, i64 32, !44, i64 40, !44, i64 48, !44, i64 56, !44, i64 64, !44, i64 72, !44, i64 80, !44, i64 88}
!68 = !{!67, !44, i64 8}
!69 = !{!67, !44, i64 32}
!70 = !{!67, !44, i64 16}
!71 = !{!67, !44, i64 64}
!72 = !{!67, !44, i64 48}
!73 = !{!67, !44, i64 56}
!74 = !{!67, !44, i64 24}
!75 = !DILocation(line: 23, column: 10, scope: !30)
!76 = distinct !DISubprogram(name: "initialize_arr", linkageName: "arr_mod_mp_initialize_arr_", scope: !2, file: !3, line: 25, type: !20, scopeLine: 25, spFlags: DISPFlagDefinition, unit: !15, retainedNodes: !77)
!77 = !{!78, !79, !80, !81, !82}
!78 = !DILocalVariable(name: "i", arg: 1, scope: !76, file: !3, line: 25, type: !24)
!79 = !DILocalVariable(name: "n", arg: 2, scope: !76, file: !3, line: 25, type: !24)
!80 = !DILocalVariable(name: "m", arg: 3, scope: !76, file: !3, line: 25, type: !24)
!81 = !DILocalVariable(name: "k", scope: !76, file: !3, line: 29, type: !24)
!82 = !DILocalVariable(name: "j", scope: !76, file: !3, line: 28, type: !24)
!83 = !DILocation(line: 25, column: 36, scope: !76)
!84 = !DILocation(line: 25, column: 39, scope: !76)
!85 = !DILocation(line: 25, column: 42, scope: !76)
!86 = !DILocation(line: 0, scope: !76)
!87 = !DILocation(line: 30, column: 45, scope: !76)
!88 = !DILocation(line: 30, column: 23, scope: !76)
!89 = !DILocation(line: 31, column: 23, scope: !76)
!90 = !DILocation(line: 29, column: 14, scope: !76)
!91 = !{!92, !93, i64 56}
!92 = !{!"ifx$descr$5", !93, i64 0, !93, i64 8, !93, i64 16, !93, i64 24, !93, i64 32, !93, i64 40, !93, i64 48, !93, i64 56, !93, i64 64, !93, i64 72, !93, i64 80, !93, i64 88}
!93 = !{!"ifx$descr$field", !94, i64 0}
!94 = !{!"Generic Fortran Symbol", !95, i64 0}
!95 = !{!"ifx$root$2$arr_mod_mp_initialize_arr_"}
!96 = !{!92, !93, i64 64}
!97 = !{!98, !93, i64 56}
!98 = !{!"ifx$descr$6", !93, i64 0, !93, i64 8, !93, i64 16, !93, i64 24, !93, i64 32, !93, i64 40, !93, i64 48, !93, i64 56, !93, i64 64, !93, i64 72, !93, i64 80, !93, i64 88}
!99 = !{!98, !93, i64 64}
!100 = !DILocation(line: 32, column: 14, scope: !76)
!101 = !{!92, !93, i64 0}
!102 = !DILocation(line: 30, column: 16, scope: !76)
!103 = !{!98, !93, i64 0}
!104 = !DILocation(line: 31, column: 16, scope: !76)
!105 = !DILocation(line: 33, column: 12, scope: !76)
!106 = !DILocation(line: 36, column: 10, scope: !76)
!107 = distinct !DISubprogram(name: "print_info", linkageName: "arr_mod_mp_print_info_", scope: !2, file: !3, line: 38, type: !20, scopeLine: 38, spFlags: DISPFlagDefinition, unit: !15, retainedNodes: !108)
!108 = !{!109, !110, !111, !113, !114}
!109 = !DILocalVariable(name: "n", arg: 1, scope: !107, file: !3, line: 38, type: !24)
!110 = !DILocalVariable(name: "m", arg: 2, scope: !107, file: !3, line: 38, type: !24)
!111 = !DILocalVariable(name: "array_in", arg: 3, scope: !107, file: !3, line: 38, type: !112)
!112 = !DICompositeType(tag: DW_TAG_array_type, baseType: !9, elements: !10, dataLocation: !DIExpression(DW_OP_push_object_address, DW_OP_deref))
!113 = !DILocalVariable(name: "k", scope: !107, file: !3, line: 43, type: !24)
!114 = !DILocalVariable(name: "j", scope: !107, file: !3, line: 42, type: !24)
!115 = !DILocation(line: 38, column: 32, scope: !107)
!116 = !DILocation(line: 38, column: 35, scope: !107)
!117 = !DILocation(line: 38, column: 38, scope: !107)
!118 = !DILocation(line: 0, scope: !107)
!119 = !DILocation(line: 44, column: 16, scope: !107)
!120 = !DILocation(line: 43, column: 14, scope: !107)
!121 = !DILocation(line: 45, column: 14, scope: !107)
!122 = !{!123, !124, i64 0}
!123 = !{!"ifx$descr$7", !124, i64 0, !124, i64 8, !124, i64 16, !124, i64 24, !124, i64 32, !124, i64 40, !124, i64 48, !124, i64 56, !124, i64 64, !124, i64 72, !124, i64 80, !124, i64 88}
!124 = !{!"ifx$descr$field", !125, i64 0}
!125 = !{!"Generic Fortran Symbol", !126, i64 0}
!126 = !{!"ifx$root$3$arr_mod_mp_print_info_"}
!127 = !{!125, !125, i64 0}
!128 = !{!129, !129, i64 0}
!129 = !{!"ifx$unique_sym$11", !125, i64 0}
!130 = !DILocation(line: 46, column: 12, scope: !107)
!131 = !DILocation(line: 48, column: 10, scope: !107)
!132 = distinct !DISubprogram(name: "print_arr", linkageName: "arr_mod_mp_print_arr_", scope: !2, file: !3, line: 50, type: !20, scopeLine: 50, spFlags: DISPFlagDefinition, unit: !15, retainedNodes: !133)
!133 = !{!134, !135, !136}
!134 = !DILocalVariable(name: "i", arg: 1, scope: !132, file: !3, line: 50, type: !24)
!135 = !DILocalVariable(name: "n", arg: 2, scope: !132, file: !3, line: 50, type: !24)
!136 = !DILocalVariable(name: "m", arg: 3, scope: !132, file: !3, line: 50, type: !24)
!137 = !DILocation(line: 50, column: 31, scope: !132)
!138 = !DILocation(line: 50, column: 34, scope: !132)
!139 = !DILocation(line: 50, column: 37, scope: !132)
!140 = !DILocation(line: 54, column: 12, scope: !132)
!141 = !{!142, !143, i64 0}
!142 = !{!"ifx$descr$8", !143, i64 0, !143, i64 8, !143, i64 16, !143, i64 24, !143, i64 32, !143, i64 40, !143, i64 48, !143, i64 56, !143, i64 64}
!143 = !{!"ifx$descr$field", !144, i64 0}
!144 = !{!"Generic Fortran Symbol", !145, i64 0}
!145 = !{!"ifx$root$4$arr_mod_mp_print_arr_"}
!146 = !{!142, !143, i64 64}
!147 = !{!148, !148, i64 0}
!148 = !{!"ifx$unique_sym$12", !144, i64 0}
!149 = !{!150, !143, i64 0}
!150 = !{!"ifx$descr$9", !143, i64 0, !143, i64 8, !143, i64 16, !143, i64 24, !143, i64 32, !143, i64 40, !143, i64 48, !143, i64 56, !143, i64 64, !143, i64 72, !143, i64 80, !143, i64 88}
!151 = !{!150, !143, i64 56}
!152 = !{!150, !143, i64 48}
!153 = !{!154, !143, i64 8}
!154 = !{!"ifx$descr$10", !143, i64 0, !143, i64 8, !143, i64 16, !143, i64 24, !143, i64 32, !143, i64 40, !143, i64 48, !143, i64 56, !143, i64 64, !143, i64 72, !143, i64 80, !143, i64 88}
!155 = !{!154, !143, i64 32}
!156 = !{!154, !143, i64 16}
!157 = !{!154, !143, i64 56}
!158 = !{!154, !143, i64 64}
!159 = !{!154, !143, i64 48}
!160 = !{!154, !143, i64 0}
!161 = !{!154, !143, i64 24}
!162 = !DILocation(line: 54, column: 17, scope: !132)
!163 = !DILocation(line: 57, column: 12, scope: !132)
!164 = !{!165, !143, i64 0}
!165 = !{!"ifx$descr$11", !143, i64 0, !143, i64 8, !143, i64 16, !143, i64 24, !143, i64 32, !143, i64 40, !143, i64 48, !143, i64 56, !143, i64 64, !143, i64 72, !143, i64 80, !143, i64 88}
!166 = !{!165, !143, i64 56}
!167 = !{!165, !143, i64 48}
!168 = !{!169, !143, i64 8}
!169 = !{!"ifx$descr$12", !143, i64 0, !143, i64 8, !143, i64 16, !143, i64 24, !143, i64 32, !143, i64 40, !143, i64 48, !143, i64 56, !143, i64 64, !143, i64 72, !143, i64 80, !143, i64 88}
!170 = !{!169, !143, i64 32}
!171 = !{!169, !143, i64 16}
!172 = !{!169, !143, i64 56}
!173 = !{!169, !143, i64 64}
!174 = !{!169, !143, i64 48}
!175 = !{!169, !143, i64 0}
!176 = !{!169, !143, i64 24}
!177 = !DILocation(line: 57, column: 17, scope: !132)
!178 = !DILocation(line: 60, column: 10, scope: !132)
!179 = !DILocation(line: 64, column: 15, scope: !19)
!180 = !DILocation(line: 0, scope: !19)
!181 = !DILocation(line: 70, column: 9, scope: !19)
!182 = !{!183, !183, i64 0}
!183 = !{!"ifx$unique_sym$13", !184, i64 0}
!184 = !{!"Generic Fortran Symbol", !185, i64 0}
!185 = !{!"ifx$root$5$MAIN__"}
!186 = !DILocation(line: 77, column: 9, scope: !19)
!187 = !DILocation(line: 72, column: 16, scope: !19)
!188 = !DILocation(line: 74, column: 16, scope: !19)
!189 = !DILocation(line: 76, column: 16, scope: !19)
!190 = !DILocation(line: 79, column: 7, scope: !19)
!191 = !{%"QNCA_a0$float*$rank2$" zeroinitializer, float 0.000000e+00}
!192 = !{%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$" zeroinitializer, %"ARR_MOD$.btT_TESTTYPE" zeroinitializer}

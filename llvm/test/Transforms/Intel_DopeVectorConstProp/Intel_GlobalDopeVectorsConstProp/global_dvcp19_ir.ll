; RUN: opt < %s -passes=dopevectorconstprop -dope-vector-global-const-prop=true -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s

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
; CHECK:   %40 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 192, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %37, i64 %39)
; CHECK:   %59 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 192, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %57, i64 %39)

; CHECK: define internal void @arr_mod_mp_initialize_arr_
; CHECK:   %9 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 192, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %4, i64 %8)
; CHECK:   %39 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 40, float* elementtype(float) %38, i64 %25)
; CHECK:   %40 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %39, i64 %37)
; CHECK:   %42 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 40, float* elementtype(float) %41, i64 %25)
; CHECK:   %43 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %42, i64 %37)

; Check that constant propagation wasn't applied to function @arr_mod_mp_print_info_
; CHECK: define internal void @arr_mod_mp_print_info_
; CHECK-NOT: %23 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 40, float* elementtype(float) %8, i64 %22)
; CHECK-NOT: %26 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %23, i64 %25)

; CHECK: define internal void @arr_mod_mp_print_arr_
; CHECK:   %11 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 1, i64 192, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %6, i64 %10)
; CHECK:   store i64 4, i64* %29, align 1
; CHECK:   store i64 10, i64* %33, align 1
; CHECK:   store i64 40, i64* %34, align 1
; CHECK:   store i64 10, i64* %36, align 1

; ModuleID = 'ld-temp.o'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$" = type { %"ARR_MOD$.btT_TESTTYPE"*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"ARR_MOD$.btT_TESTTYPE" = type <{ %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$" }>
%"QNCA_a0$float*$rank2$" = type { float*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@arr_mod_mp_a_ = internal global %"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$" { %"ARR_MOD$.btT_TESTTYPE"* null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }, !dbg !0
@anon.11934500c7ed222a0a148892d04ea3ac.0 = internal unnamed_addr constant i32 65536
@anon.11934500c7ed222a0a148892d04ea3ac.1 = internal unnamed_addr constant i32 2
@anon.11934500c7ed222a0a148892d04ea3ac.2 = internal unnamed_addr constant i32 10

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_allocate_arr_(i32* noalias nocapture readonly dereferenceable(4) %0) #0 !dbg !30 {
  %2 = alloca i64, align 8, !dbg !33              ; test.f90:14:34
  call void @llvm.dbg.declare(metadata i32* %0, metadata !32, metadata !DIExpression()), !dbg !33  ; test.f90:14:34
  %3 = load i32, i32* %0, align 1, !dbg !34, !tbaa !35  ; test.f90:17:12
  %4 = icmp eq i32 %3, 1, !dbg !39                ; test.f90:17:16
  br i1 %4, label %7, label %5, !dbg !39          ; test.f90:17:16

5:                                                ; preds = %1
  %6 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", %"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0), !dbg !40  ; test.f90:19:12
  br label %35, !dbg !39                          ; test.f90:17:16

7:                                                ; preds = %1
  %8 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", %"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$"* @arr_mod_mp_a_, i64 0, i32 3), align 8, !dbg !41, !tbaa !42  ; test.f90:17:23
  %9 = and i64 %8, 1030792151296, !dbg !41        ; test.f90:17:23
  %10 = or i64 %9, 133, !dbg !41                  ; test.f90:17:23
  store i64 %10, i64* getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", %"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$"* @arr_mod_mp_a_, i64 0, i32 3), align 8, !dbg !41, !tbaa !42  ; test.f90:17:23
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", %"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$"* @arr_mod_mp_a_, i64 0, i32 5), align 8, !dbg !41, !tbaa !45  ; test.f90:17:23
  store i64 192, i64* getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", %"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$"* @arr_mod_mp_a_, i64 0, i32 1), align 8, !dbg !41, !tbaa !46  ; test.f90:17:23
  store i64 1, i64* getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", %"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$"* @arr_mod_mp_a_, i64 0, i32 4), align 16, !dbg !41, !tbaa !47  ; test.f90:17:23
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", %"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$"* @arr_mod_mp_a_, i64 0, i32 2), align 16, !dbg !41, !tbaa !48  ; test.f90:17:23
  %11 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", %"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0), !dbg !41  ; test.f90:17:23
  store i64 1, i64* %11, align 1, !dbg !41, !tbaa !49  ; test.f90:17:23
  %12 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", %"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 0), i32 0), !dbg !41  ; test.f90:17:23
  store i64 1, i64* %12, align 1, !dbg !41, !tbaa !50  ; test.f90:17:23
  %13 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", %"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 0), !dbg !41  ; test.f90:17:23
  store i64 192, i64* %13, align 1, !dbg !41, !tbaa !51  ; test.f90:17:23
  %14 = call i32 (i64*, i32, ...) @for_check_mult_overflow64(i64* nonnull %2, i32 2, i64 1, i64 192) #6, !dbg !41  ; test.f90:17:23
  %15 = load i64, i64* %2, align 8, !dbg !41, !tbaa !52  ; test.f90:17:23
  %16 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", %"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$"* @arr_mod_mp_a_, i64 0, i32 3), align 8, !dbg !41, !tbaa !42  ; test.f90:17:23
  %17 = and i64 %16, -68451041281, !dbg !41       ; test.f90:17:23
  %18 = or i64 %17, 1073741824, !dbg !41          ; test.f90:17:23
  store i64 %18, i64* getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", %"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$"* @arr_mod_mp_a_, i64 0, i32 3), align 8, !dbg !41, !tbaa !42  ; test.f90:17:23
  %19 = trunc i64 %16 to i32, !dbg !41            ; test.f90:17:23
  %20 = shl i32 %19, 1, !dbg !41                  ; test.f90:17:23
  %21 = and i32 %20, 2, !dbg !41                  ; test.f90:17:23
  %22 = shl i32 %14, 4, !dbg !41                  ; test.f90:17:23
  %23 = and i32 %22, 16, !dbg !41                 ; test.f90:17:23
  %24 = lshr i64 %16, 15, !dbg !41                ; test.f90:17:23
  %25 = trunc i64 %24 to i32, !dbg !41            ; test.f90:17:23
  %26 = and i32 %25, 31457280, !dbg !41           ; test.f90:17:23
  %27 = and i32 %25, 33554432, !dbg !41           ; test.f90:17:23
  %28 = or i32 %23, %21, !dbg !41                 ; test.f90:17:23
  %29 = or i32 %28, %26, !dbg !41                 ; test.f90:17:23
  %30 = or i32 %29, %27, !dbg !41                 ; test.f90:17:23
  %31 = or i32 %30, 262144, !dbg !41              ; test.f90:17:23
  %32 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", %"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$"* @arr_mod_mp_a_, i64 0, i32 5), align 8, !dbg !41, !tbaa !45  ; test.f90:17:23
  %33 = inttoptr i64 %32 to i8*, !dbg !41         ; test.f90:17:23
  %34 = tail call i32 @for_alloc_allocatable_handle(i64 %15, i8** bitcast (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$"* @arr_mod_mp_a_ to i8**), i32 %31, i8* %33) #6, !dbg !41  ; test.f90:17:23
  br label %35, !dbg !39                          ; test.f90:17:16

35:                                               ; preds = %7, %5
  %36 = phi i64* [ %6, %5 ], [ %11, %7 ], !dbg !40  ; test.f90:19:12
  %37 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", %"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16, !dbg !40, !tbaa !53  ; test.f90:19:12
  %38 = load i64, i64* %36, align 1, !dbg !40, !tbaa !49  ; test.f90:19:12
  %39 = sext i32 %3 to i64, !dbg !40              ; test.f90:19:12
  %40 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %38, i64 192, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %37, i64 %39), !dbg !40  ; test.f90:19:12
  %41 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 3, !dbg !54  ; test.f90:19:21
  %42 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 5, !dbg !40  ; test.f90:19:12
  store i64 0, i64* %42, align 1, !dbg !40, !tbaa !55  ; test.f90:19:12
  %43 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 1, !dbg !40  ; test.f90:19:12
  store i64 4, i64* %43, align 1, !dbg !40, !tbaa !57  ; test.f90:19:12
  %44 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 4, !dbg !40  ; test.f90:19:12
  store i64 2, i64* %44, align 1, !dbg !40, !tbaa !58  ; test.f90:19:12
  %45 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 2, !dbg !40  ; test.f90:19:12
  store i64 0, i64* %45, align 1, !dbg !40, !tbaa !59  ; test.f90:19:12
  %46 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 6, i64 0, i32 2, !dbg !40  ; test.f90:19:12
  %47 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %46, i32 0), !dbg !40  ; test.f90:19:12
  store i64 1, i64* %47, align 1, !dbg !40, !tbaa !60  ; test.f90:19:12
  %48 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 6, i64 0, i32 0, !dbg !40  ; test.f90:19:12
  %49 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %48, i32 0), !dbg !40  ; test.f90:19:12
  store i64 10, i64* %49, align 1, !dbg !40, !tbaa !61  ; test.f90:19:12
  %50 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %46, i32 1), !dbg !40  ; test.f90:19:12
  store i64 1, i64* %50, align 1, !dbg !40, !tbaa !60  ; test.f90:19:12
  %51 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %48, i32 1), !dbg !40  ; test.f90:19:12
  store i64 10, i64* %51, align 1, !dbg !40, !tbaa !61  ; test.f90:19:12
  %52 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 6, i64 0, i32 1, !dbg !40  ; test.f90:19:12
  %53 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %52, i32 0), !dbg !40  ; test.f90:19:12
  store i64 4, i64* %53, align 1, !dbg !40, !tbaa !62  ; test.f90:19:12
  %54 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %52, i32 1), !dbg !40  ; test.f90:19:12
  store i64 40, i64* %54, align 1, !dbg !40, !tbaa !62  ; test.f90:19:12
  store i64 1073741829, i64* %41, align 1, !dbg !40, !tbaa !63  ; test.f90:19:12
  %55 = bitcast %"ARR_MOD$.btT_TESTTYPE"* %40 to i8**, !dbg !40  ; test.f90:19:12
  %56 = tail call i32 @for_allocate_handle(i64 400, i8** %55, i32 262144, i8* null) #6, !dbg !40  ; test.f90:19:12
  %57 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", %"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16, !dbg !64, !tbaa !53  ; test.f90:20:12
  %58 = load i64, i64* %36, align 1, !dbg !64, !tbaa !49  ; test.f90:20:12
  %59 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %58, i64 192, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %57, i64 %39), !dbg !64  ; test.f90:20:12
  %60 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 3, !dbg !65  ; test.f90:20:21
  %61 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 5, !dbg !64  ; test.f90:20:12
  store i64 0, i64* %61, align 1, !dbg !64, !tbaa !66  ; test.f90:20:12
  %62 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 1, !dbg !64  ; test.f90:20:12
  store i64 4, i64* %62, align 1, !dbg !64, !tbaa !68  ; test.f90:20:12
  %63 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 4, !dbg !64  ; test.f90:20:12
  store i64 2, i64* %63, align 1, !dbg !64, !tbaa !69  ; test.f90:20:12
  %64 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 2, !dbg !64  ; test.f90:20:12
  store i64 0, i64* %64, align 1, !dbg !64, !tbaa !70  ; test.f90:20:12
  %65 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 6, i64 0, i32 2, !dbg !64  ; test.f90:20:12
  %66 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %65, i32 0), !dbg !64  ; test.f90:20:12
  store i64 1, i64* %66, align 1, !dbg !64, !tbaa !71  ; test.f90:20:12
  %67 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 6, i64 0, i32 0, !dbg !64  ; test.f90:20:12
  %68 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %67, i32 0), !dbg !64  ; test.f90:20:12
  store i64 10, i64* %68, align 1, !dbg !64, !tbaa !72  ; test.f90:20:12
  %69 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %65, i32 1), !dbg !64  ; test.f90:20:12
  store i64 1, i64* %69, align 1, !dbg !64, !tbaa !71  ; test.f90:20:12
  %70 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %67, i32 1), !dbg !64  ; test.f90:20:12
  store i64 10, i64* %70, align 1, !dbg !64, !tbaa !72  ; test.f90:20:12
  %71 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 6, i64 0, i32 1, !dbg !64  ; test.f90:20:12
  %72 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %71, i32 0), !dbg !64  ; test.f90:20:12
  store i64 4, i64* %72, align 1, !dbg !64, !tbaa !73  ; test.f90:20:12
  %73 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %71, i32 1), !dbg !64  ; test.f90:20:12
  store i64 40, i64* %73, align 1, !dbg !64, !tbaa !73  ; test.f90:20:12
  %74 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 0, !dbg !64  ; test.f90:20:12
  store i64 1073741829, i64* %60, align 1, !dbg !64, !tbaa !74  ; test.f90:20:12
  %75 = bitcast float** %74 to i8**, !dbg !64     ; test.f90:20:12
  %76 = tail call i32 @for_allocate_handle(i64 400, i8** nonnull %75, i32 262144, i8* null) #6, !dbg !64  ; test.f90:20:12
  ret void, !dbg !75                              ; test.f90:23:10
}

; Function Attrs: mustprogress nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #2

; Function Attrs: nofree
declare dso_local i32 @for_check_mult_overflow64(i64* nocapture, i32, ...) local_unnamed_addr #3

; Function Attrs: nofree
declare dso_local i32 @for_alloc_allocatable_handle(i64, i8** nocapture, i32, i8*) local_unnamed_addr #3

; Function Attrs: nofree nosync nounwind readnone speculatable
declare %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8, i64, i64, %"ARR_MOD$.btT_TESTTYPE"*, i64) #2

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, i8** nocapture, i32, i8*) local_unnamed_addr #3

; Function Attrs: nofree nosync nounwind uwtable
define internal void @arr_mod_mp_initialize_arr_(i32* noalias nocapture readonly dereferenceable(4) %0, i32* noalias nocapture readonly dereferenceable(4) %1, i32* noalias nocapture readonly dereferenceable(4) %2) #4 !dbg !76 {
  call void @llvm.dbg.declare(metadata i32* %0, metadata !78, metadata !DIExpression()), !dbg !83  ; test.f90:25:36
  call void @llvm.dbg.declare(metadata i32* @anon.11934500c7ed222a0a148892d04ea3ac.2, metadata !79, metadata !DIExpression()), !dbg !84  ; test.f90:25:39
  call void @llvm.dbg.declare(metadata i32* @anon.11934500c7ed222a0a148892d04ea3ac.2, metadata !80, metadata !DIExpression()), !dbg !85  ; test.f90:25:42
  call void @llvm.dbg.value(metadata i32 1, metadata !82, metadata !DIExpression()), !dbg !86  ; test.f90:0:0
  %4 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", %"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16, !dbg !87  ; test.f90:30:45
  %5 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", %"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0), !dbg !87  ; test.f90:30:45
  %6 = load i64, i64* %5, align 1, !dbg !87       ; test.f90:30:45
  %7 = load i32, i32* %0, align 1, !dbg !87       ; test.f90:30:45
  %8 = sext i32 %7 to i64, !dbg !87               ; test.f90:30:45
  %9 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %6, i64 192, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %4, i64 %8), !dbg !87  ; test.f90:30:45
  %10 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %9, i64 0, i32 0, i32 0, !dbg !88  ; test.f90:30:23
  %11 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %9, i64 0, i32 0, i32 6, i64 0, i32 1, !dbg !88  ; test.f90:30:23
  %12 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %11, i32 0), !dbg !88  ; test.f90:30:23
  %13 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %9, i64 0, i32 0, i32 6, i64 0, i32 2, !dbg !88  ; test.f90:30:23
  %14 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %13, i32 0), !dbg !88  ; test.f90:30:23
  %15 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %11, i32 1), !dbg !88  ; test.f90:30:23
  %16 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %13, i32 1), !dbg !88  ; test.f90:30:23
  %17 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %9, i64 0, i32 1, i32 0, !dbg !89  ; test.f90:31:23
  %18 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %9, i64 0, i32 1, i32 6, i64 0, i32 1, !dbg !89  ; test.f90:31:23
  %19 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %18, i32 0), !dbg !89  ; test.f90:31:23
  %20 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %9, i64 0, i32 1, i32 6, i64 0, i32 2, !dbg !89  ; test.f90:31:23
  %21 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %20, i32 0), !dbg !89  ; test.f90:31:23
  %22 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %18, i32 1), !dbg !89  ; test.f90:31:23
  %23 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %20, i32 1), !dbg !89  ; test.f90:31:23
  br label %24, !dbg !90                          ; test.f90:29:14

24:                                               ; preds = %46, %3
  %25 = phi i64 [ 1, %3 ], [ %47, %46 ]
  call void @llvm.dbg.value(metadata i64 %25, metadata !82, metadata !DIExpression()), !dbg !86  ; test.f90:0:0
  call void @llvm.dbg.value(metadata i32 1, metadata !81, metadata !DIExpression()), !dbg !86  ; test.f90:0:0
  %26 = trunc i64 %25 to i32, !dbg !87            ; test.f90:30:45
  %27 = sitofp i32 %26 to float, !dbg !87         ; test.f90:30:45
  %28 = load i64, i64* %12, align 1, !dbg !88, !tbaa !91  ; test.f90:30:23
  %29 = load i64, i64* %14, align 1, !dbg !88, !tbaa !96  ; test.f90:30:23
  %30 = load i64, i64* %15, align 1, !dbg !88, !tbaa !91  ; test.f90:30:23
  %31 = load i64, i64* %16, align 1, !dbg !88, !tbaa !96  ; test.f90:30:23
  %32 = load i64, i64* %19, align 1, !dbg !89, !tbaa !97  ; test.f90:31:23
  %33 = load i64, i64* %21, align 1, !dbg !89, !tbaa !99  ; test.f90:31:23
  %34 = load i64, i64* %22, align 1, !dbg !89, !tbaa !97  ; test.f90:31:23
  %35 = load i64, i64* %23, align 1, !dbg !89, !tbaa !99  ; test.f90:31:23
  br label %36, !dbg !100                         ; test.f90:32:14

36:                                               ; preds = %36, %24
  %37 = phi i64 [ 1, %24 ], [ %44, %36 ]
  call void @llvm.dbg.value(metadata i64 %37, metadata !81, metadata !DIExpression()), !dbg !86  ; test.f90:0:0
  %38 = load float*, float** %10, align 1, !dbg !88, !tbaa !101  ; test.f90:30:23
  %39 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %31, i64 %30, float* elementtype(float) %38, i64 %25), !dbg !88  ; test.f90:30:23
  %40 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %29, i64 %28, float* elementtype(float) %39, i64 %37), !dbg !88  ; test.f90:30:23
  store float %27, float* %40, align 1, !dbg !102, !tbaa !101  ; test.f90:30:16
  %41 = load float*, float** %17, align 1, !dbg !89, !tbaa !103  ; test.f90:31:23
  %42 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %35, i64 %34, float* elementtype(float) %41, i64 %25), !dbg !89  ; test.f90:31:23
  %43 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %33, i64 %32, float* elementtype(float) %42, i64 %37), !dbg !89  ; test.f90:31:23
  store float %27, float* %43, align 1, !dbg !104, !tbaa !103  ; test.f90:31:16
  %44 = add nuw nsw i64 %37, 1, !dbg !100         ; test.f90:32:14
  call void @llvm.dbg.value(metadata i64 %44, metadata !81, metadata !DIExpression()), !dbg !86  ; test.f90:0:0
  %45 = icmp eq i64 %44, 11, !dbg !100            ; test.f90:32:14
  br i1 %45, label %46, label %36, !dbg !100      ; test.f90:32:14

46:                                               ; preds = %36
  %47 = add nuw nsw i64 %25, 1, !dbg !105         ; test.f90:33:12
  call void @llvm.dbg.value(metadata i64 %47, metadata !82, metadata !DIExpression()), !dbg !86  ; test.f90:0:0
  %48 = icmp eq i64 %47, 11, !dbg !105            ; test.f90:33:12
  br i1 %48, label %49, label %24, !dbg !105      ; test.f90:33:12

49:                                               ; preds = %46
  ret void, !dbg !106                             ; test.f90:36:10
}

; Function Attrs: mustprogress nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #2

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_print_info_(i32* noalias nocapture readonly dereferenceable(4) %0, i32* noalias nocapture readonly dereferenceable(4) %1, %"QNCA_a0$float*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %2) #0 !dbg !107 {
  %4 = alloca [8 x i64], align 16
  %5 = alloca [4 x i8], align 1, !dbg !115        ; test.f90:38:32
  %6 = alloca <{ i64 }>, align 8, !dbg !115       ; test.f90:38:32
  call void @llvm.dbg.declare(metadata i32* @anon.11934500c7ed222a0a148892d04ea3ac.2, metadata !109, metadata !DIExpression()), !dbg !115  ; test.f90:38:32
  call void @llvm.dbg.declare(metadata i32* @anon.11934500c7ed222a0a148892d04ea3ac.2, metadata !110, metadata !DIExpression()), !dbg !116  ; test.f90:38:35
  call void @llvm.dbg.declare(metadata %"QNCA_a0$float*$rank2$"* %2, metadata !111, metadata !DIExpression()), !dbg !117  ; test.f90:38:38
  call void @llvm.dbg.value(metadata i32 1, metadata !114, metadata !DIExpression()), !dbg !118  ; test.f90:0:0
  %7 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 0, !dbg !119  ; test.f90:44:16
  %8 = load float*, float** %7, align 1, !dbg !119  ; test.f90:44:16
  %9 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 6, i64 0, i32 1, !dbg !119  ; test.f90:44:16
  %10 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %9, i32 0), !dbg !119  ; test.f90:44:16
  %11 = load i64, i64* %10, align 1, !dbg !119    ; test.f90:44:16
  %12 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %9, i32 1), !dbg !119  ; test.f90:44:16
  %13 = load i64, i64* %12, align 1, !dbg !119    ; test.f90:44:16
  %14 = getelementptr inbounds [4 x i8], [4 x i8]* %5, i64 0, i64 0, !dbg !119  ; test.f90:44:16
  %15 = getelementptr inbounds [4 x i8], [4 x i8]* %5, i64 0, i64 1, !dbg !119  ; test.f90:44:16
  %16 = getelementptr inbounds [4 x i8], [4 x i8]* %5, i64 0, i64 2, !dbg !119  ; test.f90:44:16
  %17 = getelementptr inbounds [4 x i8], [4 x i8]* %5, i64 0, i64 3, !dbg !119  ; test.f90:44:16
  %18 = bitcast <{ i64 }>* %6 to float*, !dbg !119  ; test.f90:44:16
  %19 = bitcast [8 x i64]* %4 to i8*, !dbg !119   ; test.f90:44:16
  %20 = bitcast <{ i64 }>* %6 to i8*, !dbg !119   ; test.f90:44:16
  br label %21, !dbg !120                         ; test.f90:43:14

21:                                               ; preds = %32, %3
  %22 = phi i64 [ 1, %3 ], [ %33, %32 ]
  call void @llvm.dbg.value(metadata i64 %22, metadata !114, metadata !DIExpression()), !dbg !118  ; test.f90:0:0
  call void @llvm.dbg.value(metadata i32 1, metadata !113, metadata !DIExpression()), !dbg !118  ; test.f90:0:0
  %23 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %13, float* elementtype(float) %8, i64 %22), !dbg !119  ; test.f90:44:16
  br label %24, !dbg !121                         ; test.f90:45:14

24:                                               ; preds = %24, %21
  %25 = phi i64 [ 1, %21 ], [ %29, %24 ]
  call void @llvm.dbg.value(metadata i64 %25, metadata !113, metadata !DIExpression()), !dbg !118  ; test.f90:0:0
  %26 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 %11, float* elementtype(float) %23, i64 %25), !dbg !119  ; test.f90:44:16
  %27 = load float, float* %26, align 1, !dbg !119, !tbaa !122  ; test.f90:44:16
  store i8 26, i8* %14, align 1, !dbg !119, !tbaa !127  ; test.f90:44:16
  store i8 1, i8* %15, align 1, !dbg !119, !tbaa !127  ; test.f90:44:16
  store i8 1, i8* %16, align 1, !dbg !119, !tbaa !127  ; test.f90:44:16
  store i8 0, i8* %17, align 1, !dbg !119, !tbaa !127  ; test.f90:44:16
  store float %27, float* %18, align 8, !dbg !119, !tbaa !128  ; test.f90:44:16
  %28 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %19, i32 -1, i64 1239157112576, i8* nonnull %14, i8* nonnull %20) #6, !dbg !119  ; test.f90:44:16
  %29 = add nuw i64 %25, 1, !dbg !121             ; test.f90:45:14
  call void @llvm.dbg.value(metadata i64 %29, metadata !113, metadata !DIExpression()), !dbg !118  ; test.f90:0:0
  %30 = trunc i64 %29 to i32, !dbg !121           ; test.f90:45:14
  %31 = icmp slt i32 10, %30, !dbg !121           ; test.f90:45:14
  br i1 %31, label %32, label %24, !dbg !121      ; test.f90:45:14

32:                                               ; preds = %24
  %33 = add nuw i64 %22, 1, !dbg !130             ; test.f90:46:12
  call void @llvm.dbg.value(metadata i64 %33, metadata !114, metadata !DIExpression()), !dbg !118  ; test.f90:0:0
  %34 = trunc i64 %33 to i32, !dbg !130           ; test.f90:46:12
  %35 = icmp slt i32 10, %34, !dbg !130           ; test.f90:46:12
  br i1 %35, label %36, label %21, !dbg !130      ; test.f90:46:12

36:                                               ; preds = %32
  ret void, !dbg !131                             ; test.f90:48:10
}

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr #3

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_print_arr_(i32* noalias nocapture readonly dereferenceable(4) %0, i32* noalias nocapture readonly dereferenceable(4) %1, i32* noalias nocapture readonly dereferenceable(4) %2) #0 !dbg !132 {
  %4 = alloca %"QNCA_a0$float*$rank2$", align 8, !dbg !137  ; test.f90:50:31
  %5 = alloca %"QNCA_a0$float*$rank2$", align 8, !dbg !137  ; test.f90:50:31
  call void @llvm.dbg.declare(metadata i32* %0, metadata !134, metadata !DIExpression()), !dbg !137  ; test.f90:50:31
  call void @llvm.dbg.declare(metadata i32* @anon.11934500c7ed222a0a148892d04ea3ac.2, metadata !135, metadata !DIExpression()), !dbg !138  ; test.f90:50:34
  call void @llvm.dbg.declare(metadata i32* @anon.11934500c7ed222a0a148892d04ea3ac.2, metadata !136, metadata !DIExpression()), !dbg !139  ; test.f90:50:37
  %6 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", %"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16, !dbg !140, !tbaa !141  ; test.f90:54:12
  %7 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", %"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0), !dbg !140  ; test.f90:54:12
  %8 = load i64, i64* %7, align 1, !dbg !140, !tbaa !146  ; test.f90:54:12
  %9 = load i32, i32* %0, align 1, !dbg !140, !tbaa !147  ; test.f90:54:12
  %10 = sext i32 %9 to i64, !dbg !140             ; test.f90:54:12
  %11 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %8, i64 192, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %6, i64 %10), !dbg !140  ; test.f90:54:12
  %12 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %11, i64 0, i32 0, i32 0, !dbg !140  ; test.f90:54:12
  %13 = load float*, float** %12, align 1, !dbg !140, !tbaa !149  ; test.f90:54:12
  %14 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %11, i64 0, i32 0, i32 6, i64 0, i32 1, !dbg !140  ; test.f90:54:12
  %15 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %14, i32 0), !dbg !140  ; test.f90:54:12
  %16 = load i64, i64* %15, align 1, !dbg !140, !tbaa !151  ; test.f90:54:12
  %17 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %11, i64 0, i32 0, i32 6, i64 0, i32 0, !dbg !140  ; test.f90:54:12
  %18 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %17, i32 0), !dbg !140  ; test.f90:54:12
  %19 = load i64, i64* %18, align 1, !dbg !140, !tbaa !152  ; test.f90:54:12
  %20 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %14, i32 1), !dbg !140  ; test.f90:54:12
  %21 = load i64, i64* %20, align 1, !dbg !140, !tbaa !151  ; test.f90:54:12
  %22 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %17, i32 1), !dbg !140  ; test.f90:54:12
  %23 = load i64, i64* %22, align 1, !dbg !140, !tbaa !152  ; test.f90:54:12
  %24 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %4, i64 0, i32 3, !dbg !140  ; test.f90:54:12
  %25 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %4, i64 0, i32 1, !dbg !140  ; test.f90:54:12
  store i64 4, i64* %25, align 8, !dbg !140, !tbaa !153  ; test.f90:54:12
  %26 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %4, i64 0, i32 4, !dbg !140  ; test.f90:54:12
  store i64 2, i64* %26, align 8, !dbg !140, !tbaa !155  ; test.f90:54:12
  %27 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %4, i64 0, i32 2, !dbg !140  ; test.f90:54:12
  store i64 0, i64* %27, align 8, !dbg !140, !tbaa !156  ; test.f90:54:12
  %28 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %4, i64 0, i32 6, i64 0, i32 1, !dbg !140  ; test.f90:54:12
  %29 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %28, i32 0), !dbg !140  ; test.f90:54:12
  store i64 %16, i64* %29, align 1, !dbg !140, !tbaa !157  ; test.f90:54:12
  %30 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %4, i64 0, i32 6, i64 0, i32 2, !dbg !140  ; test.f90:54:12
  %31 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %30, i32 0), !dbg !140  ; test.f90:54:12
  store i64 1, i64* %31, align 1, !dbg !140, !tbaa !158  ; test.f90:54:12
  %32 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %4, i64 0, i32 6, i64 0, i32 0, !dbg !140  ; test.f90:54:12
  %33 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %32, i32 0), !dbg !140  ; test.f90:54:12
  store i64 %19, i64* %33, align 1, !dbg !140, !tbaa !159  ; test.f90:54:12
  %34 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %28, i32 1), !dbg !140  ; test.f90:54:12
  store i64 %21, i64* %34, align 1, !dbg !140, !tbaa !157  ; test.f90:54:12
  %35 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %30, i32 1), !dbg !140  ; test.f90:54:12
  store i64 1, i64* %35, align 1, !dbg !140, !tbaa !158  ; test.f90:54:12
  %36 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %32, i32 1), !dbg !140  ; test.f90:54:12
  store i64 %23, i64* %36, align 1, !dbg !140, !tbaa !159  ; test.f90:54:12
  %37 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %4, i64 0, i32 0, !dbg !140  ; test.f90:54:12
  store float* %13, float** %37, align 8, !dbg !140, !tbaa !160  ; test.f90:54:12
  store i64 1, i64* %24, align 8, !dbg !140, !tbaa !161  ; test.f90:54:12
  call void @arr_mod_mp_print_info_(i32* nonnull @anon.11934500c7ed222a0a148892d04ea3ac.2, i32* nonnull @anon.11934500c7ed222a0a148892d04ea3ac.2, %"QNCA_a0$float*$rank2$"* nonnull %4) #7, !dbg !162  ; test.f90:54:17
  %38 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$", %"QNCA_a0$%ARR_MOD$.btT_TESTTYPE*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16, !dbg !163, !tbaa !141  ; test.f90:57:12
  %39 = load i64, i64* %7, align 1, !dbg !163, !tbaa !146  ; test.f90:57:12
  %40 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %39, i64 192, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %38, i64 %10), !dbg !163  ; test.f90:57:12
  %41 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 1, i32 0, !dbg !163  ; test.f90:57:12
  %42 = load float*, float** %41, align 1, !dbg !163, !tbaa !164  ; test.f90:57:12
  %43 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 1, i32 6, i64 0, i32 1, !dbg !163  ; test.f90:57:12
  %44 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %43, i32 0), !dbg !163  ; test.f90:57:12
  %45 = load i64, i64* %44, align 1, !dbg !163, !tbaa !166  ; test.f90:57:12
  %46 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 1, i32 6, i64 0, i32 0, !dbg !163  ; test.f90:57:12
  %47 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %46, i32 0), !dbg !163  ; test.f90:57:12
  %48 = load i64, i64* %47, align 1, !dbg !163, !tbaa !167  ; test.f90:57:12
  %49 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %43, i32 1), !dbg !163  ; test.f90:57:12
  %50 = load i64, i64* %49, align 1, !dbg !163, !tbaa !166  ; test.f90:57:12
  %51 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %46, i32 1), !dbg !163  ; test.f90:57:12
  %52 = load i64, i64* %51, align 1, !dbg !163, !tbaa !167  ; test.f90:57:12
  %53 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %5, i64 0, i32 3, !dbg !163  ; test.f90:57:12
  %54 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %5, i64 0, i32 1, !dbg !163  ; test.f90:57:12
  store i64 4, i64* %54, align 8, !dbg !163, !tbaa !168  ; test.f90:57:12
  %55 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %5, i64 0, i32 4, !dbg !163  ; test.f90:57:12
  store i64 2, i64* %55, align 8, !dbg !163, !tbaa !170  ; test.f90:57:12
  %56 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %5, i64 0, i32 2, !dbg !163  ; test.f90:57:12
  store i64 0, i64* %56, align 8, !dbg !163, !tbaa !171  ; test.f90:57:12
  %57 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %5, i64 0, i32 6, i64 0, i32 1, !dbg !163  ; test.f90:57:12
  %58 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %57, i32 0), !dbg !163  ; test.f90:57:12
  store i64 %45, i64* %58, align 1, !dbg !163, !tbaa !172  ; test.f90:57:12
  %59 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %5, i64 0, i32 6, i64 0, i32 2, !dbg !163  ; test.f90:57:12
  %60 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %59, i32 0), !dbg !163  ; test.f90:57:12
  store i64 1, i64* %60, align 1, !dbg !163, !tbaa !173  ; test.f90:57:12
  %61 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %5, i64 0, i32 6, i64 0, i32 0, !dbg !163  ; test.f90:57:12
  %62 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %61, i32 0), !dbg !163  ; test.f90:57:12
  store i64 %48, i64* %62, align 1, !dbg !163, !tbaa !174  ; test.f90:57:12
  %63 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %57, i32 1), !dbg !163  ; test.f90:57:12
  store i64 %50, i64* %63, align 1, !dbg !163, !tbaa !172  ; test.f90:57:12
  %64 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %59, i32 1), !dbg !163  ; test.f90:57:12
  store i64 1, i64* %64, align 1, !dbg !163, !tbaa !173  ; test.f90:57:12
  %65 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %61, i32 1), !dbg !163  ; test.f90:57:12
  store i64 %52, i64* %65, align 1, !dbg !163, !tbaa !174  ; test.f90:57:12
  %66 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %5, i64 0, i32 0, !dbg !163  ; test.f90:57:12
  store float* %42, float** %66, align 8, !dbg !163, !tbaa !175  ; test.f90:57:12
  store i64 1, i64* %53, align 8, !dbg !163, !tbaa !176  ; test.f90:57:12
  call void @arr_mod_mp_print_info_(i32* nonnull @anon.11934500c7ed222a0a148892d04ea3ac.2, i32* nonnull @anon.11934500c7ed222a0a148892d04ea3ac.2, %"QNCA_a0$float*$rank2$"* nonnull %5) #7, !dbg !177  ; test.f90:57:17
  ret void, !dbg !178                             ; test.f90:60:10
}

; Function Attrs: nounwind uwtable
define dso_local void @MAIN__() #5 !dbg !19 {
  %1 = alloca i32, align 8
  %2 = tail call i32 @for_set_fpe_(i32* nonnull @anon.11934500c7ed222a0a148892d04ea3ac.0) #6, !dbg !179  ; test.f90:64:15
  %3 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1) #6, !dbg !179  ; test.f90:64:15
  call void @llvm.dbg.value(metadata i32 1, metadata !23, metadata !DIExpression()), !dbg !180  ; test.f90:0:0
  store i32 1, i32* %1, align 8, !dbg !181, !tbaa !182  ; test.f90:70:9
  br label %4, !dbg !181                          ; test.f90:70:9

4:                                                ; preds = %4, %0
  %5 = phi i32 [ %6, %4 ], [ 1, %0 ], !dbg !186   ; test.f90:77:9
  call void @llvm.dbg.value(metadata i32* %1, metadata !23, metadata !DIExpression(DW_OP_deref)), !dbg !180  ; test.f90:0:0
  call void @arr_mod_mp_allocate_arr_(i32* nonnull %1) #7, !dbg !187  ; test.f90:72:16
  call void @arr_mod_mp_initialize_arr_(i32* nonnull %1, i32* nonnull @anon.11934500c7ed222a0a148892d04ea3ac.2, i32* nonnull @anon.11934500c7ed222a0a148892d04ea3ac.2) #7, !dbg !188  ; test.f90:74:16
  call void @arr_mod_mp_print_arr_(i32* nonnull %1, i32* nonnull @anon.11934500c7ed222a0a148892d04ea3ac.2, i32* nonnull @anon.11934500c7ed222a0a148892d04ea3ac.2) #7, !dbg !189  ; test.f90:76:16
  call void @llvm.dbg.value(metadata i32 %5, metadata !23, metadata !DIExpression()), !dbg !180  ; test.f90:0:0
  %6 = add nuw nsw i32 %5, 1, !dbg !186           ; test.f90:77:9
  call void @llvm.dbg.value(metadata i32 %6, metadata !23, metadata !DIExpression()), !dbg !180  ; test.f90:0:0
  store i32 %6, i32* %1, align 8, !dbg !186, !tbaa !182  ; test.f90:77:9
  %7 = icmp eq i32 %6, 11, !dbg !186              ; test.f90:77:9
  br i1 %7, label %8, label %4, !dbg !186         ; test.f90:77:9

8:                                                ; preds = %4
  ret void, !dbg !190                             ; test.f90:79:7
}

declare dso_local i32 @for_set_fpe_(i32* nocapture readonly) local_unnamed_addr

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(i32* nocapture readonly) local_unnamed_addr #3

attributes #0 = { nofree nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { nofree nosync nounwind readnone speculatable }
attributes #3 = { nofree "intel-lang"="fortran" }
attributes #4 = { nofree nosync nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #5 = { nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #6 = { nounwind }
attributes #7 = { noinline }

!llvm.dbg.cu = !{!15}
!omp_offload.info = !{}
!llvm.module.flags = !{!25, !26, !27, !28, !29}

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
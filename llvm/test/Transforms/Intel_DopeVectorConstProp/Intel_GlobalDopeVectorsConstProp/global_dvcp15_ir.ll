; RUN: opt < %s -passes=dopevectorconstprop -dope-vector-global-const-prop=true -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s

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
; CHECK:   %48 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 40, float* elementtype(float) %13, i64 %47), !dbg !106
; CHECK:   %51 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %48, i64 %50), !dbg !106
; CHECK:   %59 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 400, float* elementtype(float) %25, i64 %58), !dbg !108
; CHECK:   %60 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 40, float* elementtype(float) %59, i64 %47), !dbg !108
; CHECK:   %61 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %60, i64 %50), !dbg !108
; CHECK:   %77 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %38, i64 %76), !dbg !111

source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$" = type { %"ARR_MOD$.btT_TESTTYPE"*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"ARR_MOD$.btT_TESTTYPE" = type { %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank1$" }
%"QNCA_a0$float*$rank2$" = type { float*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank3$" = type { float*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
%"QNCA_a0$float*$rank1$" = type { float*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@arr_mod_mp_a_ = internal global %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$" { %"ARR_MOD$.btT_TESTTYPE"* null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }, !dbg !0
@anon.11934500c7ed222a0a148892d04ea3ac.0 = internal unnamed_addr constant i32 2
@anon.11934500c7ed222a0a148892d04ea3ac.1 = internal unnamed_addr constant i32 10

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_allocate_arr_(i32* noalias nocapture readonly dereferenceable(4) %0) #0 !dbg !36 {
  %2 = alloca i64, align 8, !dbg !39              ; simple.f90:15:34
  call void @llvm.dbg.declare(metadata i32* %0, metadata !38, metadata !DIExpression()), !dbg !39  ; simple.f90:15:34
  %3 = load i32, i32* %0, align 1, !dbg !40       ; simple.f90:18:12
  %4 = icmp eq i32 %3, 1, !dbg !41                ; simple.f90:18:16
  br i1 %4, label %7, label %5, !dbg !41          ; simple.f90:18:16

5:                                                ; preds = %1
  %6 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0), !dbg !42  ; simple.f90:20:12
  br label %35, !dbg !41                          ; simple.f90:18:16

7:                                                ; preds = %1
  %8 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 3), align 8, !dbg !43  ; simple.f90:18:23
  %9 = and i64 %8, 1030792151296, !dbg !43        ; simple.f90:18:23
  %10 = or i64 %9, 133, !dbg !43                  ; simple.f90:18:23
  store i64 %10, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 3), align 8, !dbg !43  ; simple.f90:18:23
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 5), align 8, !dbg !43  ; simple.f90:18:23
  store i64 288, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 1), align 8, !dbg !43  ; simple.f90:18:23
  store i64 1, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 4), align 16, !dbg !43  ; simple.f90:18:23
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 2), align 16, !dbg !43  ; simple.f90:18:23
  %11 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0), !dbg !43  ; simple.f90:18:23
  store i64 1, i64* %11, align 1, !dbg !43        ; simple.f90:18:23
  %12 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 0), i32 0), !dbg !43  ; simple.f90:18:23
  store i64 1, i64* %12, align 1, !dbg !43        ; simple.f90:18:23
  %13 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 0), !dbg !43  ; simple.f90:18:23
  store i64 288, i64* %13, align 1, !dbg !43      ; simple.f90:18:23
  %14 = call i32 (i64*, i32, ...) @for_check_mult_overflow64(i64* nonnull %2, i32 2, i64 1, i64 288) #5, !dbg !43  ; simple.f90:18:23
  %15 = load i64, i64* %2, align 8, !dbg !43      ; simple.f90:18:23
  %16 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 3), align 8, !dbg !43  ; simple.f90:18:23
  %17 = and i64 %16, -68451041281, !dbg !43       ; simple.f90:18:23
  %18 = or i64 %17, 1073741824, !dbg !43          ; simple.f90:18:23
  store i64 %18, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 3), align 8, !dbg !43  ; simple.f90:18:23
  %19 = trunc i64 %16 to i32, !dbg !43            ; simple.f90:18:23
  %20 = shl i32 %19, 1, !dbg !43                  ; simple.f90:18:23
  %21 = and i32 %20, 2, !dbg !43                  ; simple.f90:18:23
  %22 = shl i32 %14, 4, !dbg !43                  ; simple.f90:18:23
  %23 = and i32 %22, 16, !dbg !43                 ; simple.f90:18:23
  %24 = lshr i64 %16, 15, !dbg !43                ; simple.f90:18:23
  %25 = trunc i64 %24 to i32, !dbg !43            ; simple.f90:18:23
  %26 = and i32 %25, 31457280, !dbg !43           ; simple.f90:18:23
  %27 = and i32 %25, 33554432, !dbg !43           ; simple.f90:18:23
  %28 = or i32 %23, %21, !dbg !43                 ; simple.f90:18:23
  %29 = or i32 %28, %26, !dbg !43                 ; simple.f90:18:23
  %30 = or i32 %29, %27, !dbg !43                 ; simple.f90:18:23
  %31 = or i32 %30, 262144, !dbg !43              ; simple.f90:18:23
  %32 = load i64, i64* getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 5), align 8, !dbg !43  ; simple.f90:18:23
  %33 = inttoptr i64 %32 to i8*, !dbg !43         ; simple.f90:18:23
  %34 = tail call i32 @for_alloc_allocatable_handle(i64 %15, i8** bitcast (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_ to i8**), i32 %31, i8* %33) #5, !dbg !43  ; simple.f90:18:23
  br label %35, !dbg !41                          ; simple.f90:18:16

35:                                               ; preds = %7, %5
  %36 = phi i64* [ %6, %5 ], [ %11, %7 ], !dbg !42  ; simple.f90:20:12
  %37 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16, !dbg !42  ; simple.f90:20:12
  %38 = load i64, i64* %36, align 1, !dbg !42     ; simple.f90:20:12
  %39 = sext i32 %3 to i64, !dbg !42              ; simple.f90:20:12
  %40 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %38, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %37, i64 %39), !dbg !44  ; simple.f90:20:21
  %41 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 3, !dbg !44  ; simple.f90:20:21
  %42 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 5, !dbg !42  ; simple.f90:20:12
  store i64 0, i64* %42, align 1, !dbg !42        ; simple.f90:20:12
  %43 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 1, !dbg !42  ; simple.f90:20:12
  store i64 4, i64* %43, align 1, !dbg !42        ; simple.f90:20:12
  %44 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 4, !dbg !42  ; simple.f90:20:12
  store i64 2, i64* %44, align 1, !dbg !42        ; simple.f90:20:12
  %45 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 2, !dbg !42  ; simple.f90:20:12
  store i64 0, i64* %45, align 1, !dbg !42        ; simple.f90:20:12
  %46 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 6, i64 0, i32 2, !dbg !42  ; simple.f90:20:12
  %47 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %46, i32 0), !dbg !42  ; simple.f90:20:12
  store i64 1, i64* %47, align 1, !dbg !42        ; simple.f90:20:12
  %48 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 6, i64 0, i32 0, !dbg !42  ; simple.f90:20:12
  %49 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %48, i32 0), !dbg !42  ; simple.f90:20:12
  store i64 10, i64* %49, align 1, !dbg !42       ; simple.f90:20:12
  %50 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %46, i32 1), !dbg !42  ; simple.f90:20:12
  store i64 1, i64* %50, align 1, !dbg !42        ; simple.f90:20:12
  %51 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %48, i32 1), !dbg !42  ; simple.f90:20:12
  store i64 10, i64* %51, align 1, !dbg !42       ; simple.f90:20:12
  %52 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %40, i64 0, i32 0, i32 6, i64 0, i32 1, !dbg !42  ; simple.f90:20:12
  %53 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %52, i32 0), !dbg !42  ; simple.f90:20:12
  store i64 4, i64* %53, align 1, !dbg !42        ; simple.f90:20:12
  %54 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %52, i32 1), !dbg !42  ; simple.f90:20:12
  store i64 40, i64* %54, align 1, !dbg !42       ; simple.f90:20:12
  store i64 1073741829, i64* %41, align 1, !dbg !42  ; simple.f90:20:12
  %55 = bitcast %"ARR_MOD$.btT_TESTTYPE"* %40 to i8**, !dbg !42  ; simple.f90:20:12
  %56 = tail call i32 @for_allocate_handle(i64 400, i8** %55, i32 262144, i8* null) #5, !dbg !42  ; simple.f90:20:12
  %57 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16, !dbg !45  ; simple.f90:21:12
  %58 = load i64, i64* %36, align 1, !dbg !45     ; simple.f90:21:12
  %59 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %58, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %57, i64 %39), !dbg !46  ; simple.f90:21:21
  %60 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 3, !dbg !46  ; simple.f90:21:21
  %61 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 5, !dbg !45  ; simple.f90:21:12
  store i64 0, i64* %61, align 1, !dbg !45        ; simple.f90:21:12
  %62 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 1, !dbg !45  ; simple.f90:21:12
  store i64 4, i64* %62, align 1, !dbg !45        ; simple.f90:21:12
  %63 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 4, !dbg !45  ; simple.f90:21:12
  store i64 3, i64* %63, align 1, !dbg !45        ; simple.f90:21:12
  %64 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 2, !dbg !45  ; simple.f90:21:12
  store i64 0, i64* %64, align 1, !dbg !45        ; simple.f90:21:12
  %65 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 6, i64 0, i32 2, !dbg !45  ; simple.f90:21:12
  %66 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %65, i32 0), !dbg !45  ; simple.f90:21:12
  store i64 1, i64* %66, align 1, !dbg !45        ; simple.f90:21:12
  %67 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 6, i64 0, i32 0, !dbg !45  ; simple.f90:21:12
  %68 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %67, i32 0), !dbg !45  ; simple.f90:21:12
  store i64 10, i64* %68, align 1, !dbg !45       ; simple.f90:21:12
  %69 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %65, i32 1), !dbg !45  ; simple.f90:21:12
  store i64 1, i64* %69, align 1, !dbg !45        ; simple.f90:21:12
  %70 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %67, i32 1), !dbg !45  ; simple.f90:21:12
  store i64 10, i64* %70, align 1, !dbg !45       ; simple.f90:21:12
  %71 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %65, i32 2), !dbg !45  ; simple.f90:21:12
  store i64 1, i64* %71, align 1, !dbg !45        ; simple.f90:21:12
  %72 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %67, i32 2), !dbg !45  ; simple.f90:21:12
  store i64 10, i64* %72, align 1, !dbg !45       ; simple.f90:21:12
  %73 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 6, i64 0, i32 1, !dbg !45  ; simple.f90:21:12
  %74 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %73, i32 0), !dbg !45  ; simple.f90:21:12
  store i64 4, i64* %74, align 1, !dbg !45        ; simple.f90:21:12
  %75 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %73, i32 1), !dbg !45  ; simple.f90:21:12
  store i64 40, i64* %75, align 1, !dbg !45       ; simple.f90:21:12
  %76 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %73, i32 2), !dbg !45  ; simple.f90:21:12
  store i64 400, i64* %76, align 1, !dbg !45      ; simple.f90:21:12
  %77 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %59, i64 0, i32 1, i32 0, !dbg !45  ; simple.f90:21:12
  store i64 1073741829, i64* %60, align 1, !dbg !45  ; simple.f90:21:12
  %78 = bitcast float** %77 to i8**, !dbg !45     ; simple.f90:21:12
  %79 = tail call i32 @for_allocate_handle(i64 4000, i8** nonnull %78, i32 262144, i8* null) #5, !dbg !45  ; simple.f90:21:12
  %80 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16, !dbg !47  ; simple.f90:22:12
  %81 = load i64, i64* %36, align 1, !dbg !47     ; simple.f90:22:12
  %82 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %81, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %80, i64 %39), !dbg !48  ; simple.f90:22:21
  %83 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %82, i64 0, i32 2, i32 3, !dbg !48  ; simple.f90:22:21
  %84 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %82, i64 0, i32 2, i32 5, !dbg !47  ; simple.f90:22:12
  store i64 0, i64* %84, align 1, !dbg !47        ; simple.f90:22:12
  %85 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %82, i64 0, i32 2, i32 1, !dbg !47  ; simple.f90:22:12
  store i64 4, i64* %85, align 1, !dbg !47        ; simple.f90:22:12
  %86 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %82, i64 0, i32 2, i32 4, !dbg !47  ; simple.f90:22:12
  store i64 1, i64* %86, align 1, !dbg !47        ; simple.f90:22:12
  %87 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %82, i64 0, i32 2, i32 2, !dbg !47  ; simple.f90:22:12
  store i64 0, i64* %87, align 1, !dbg !47        ; simple.f90:22:12
  %88 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %82, i64 0, i32 2, i32 6, i64 0, i32 2, !dbg !47  ; simple.f90:22:12
  %89 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %88, i32 0), !dbg !47  ; simple.f90:22:12
  store i64 1, i64* %89, align 1, !dbg !47        ; simple.f90:22:12
  %90 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %82, i64 0, i32 2, i32 6, i64 0, i32 0, !dbg !47  ; simple.f90:22:12
  %91 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %90, i32 0), !dbg !47  ; simple.f90:22:12
  store i64 10, i64* %91, align 1, !dbg !47       ; simple.f90:22:12
  %92 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %82, i64 0, i32 2, i32 6, i64 0, i32 1, !dbg !47  ; simple.f90:22:12
  %93 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %92, i32 0), !dbg !47  ; simple.f90:22:12
  store i64 4, i64* %93, align 1, !dbg !47        ; simple.f90:22:12
  %94 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %82, i64 0, i32 2, i32 0, !dbg !47  ; simple.f90:22:12
  store i64 1073741829, i64* %83, align 1, !dbg !47  ; simple.f90:22:12
  %95 = bitcast float** %94 to i8**, !dbg !47     ; simple.f90:22:12
  %96 = tail call i32 @for_allocate_handle(i64 40, i8** nonnull %95, i32 262144, i8* null) #5, !dbg !47  ; simple.f90:22:12
  ret void, !dbg !49                              ; simple.f90:25:10
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
define internal void @arr_mod_mp_initialize_arr_(i32* noalias nocapture readonly dereferenceable(4) %0, i32* noalias nocapture readonly dereferenceable(4) %1, i32* noalias nocapture readonly dereferenceable(4) %2, i32* noalias nocapture readonly dereferenceable(4) %3) #4 !dbg !50 {
  call void @llvm.dbg.declare(metadata i32* %0, metadata !52, metadata !DIExpression()), !dbg !59  ; simple.f90:27:36
  call void @llvm.dbg.declare(metadata i32* @anon.11934500c7ed222a0a148892d04ea3ac.1, metadata !53, metadata !DIExpression()), !dbg !60  ; simple.f90:27:39
  call void @llvm.dbg.declare(metadata i32* @anon.11934500c7ed222a0a148892d04ea3ac.1, metadata !54, metadata !DIExpression()), !dbg !61  ; simple.f90:27:42
  call void @llvm.dbg.declare(metadata i32* @anon.11934500c7ed222a0a148892d04ea3ac.1, metadata !55, metadata !DIExpression()), !dbg !62  ; simple.f90:27:45
  call void @llvm.dbg.value(metadata i32 1, metadata !58, metadata !DIExpression()), !dbg !63  ; simple.f90:0:0
  %5 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0), !dbg !64  ; simple.f90:32:45
  %6 = load i32, i32* %0, align 1, !dbg !64       ; simple.f90:32:45
  %7 = sext i32 %6 to i64, !dbg !64               ; simple.f90:32:45
  br label %8, !dbg !65                           ; simple.f90:31:14

8:                                                ; preds = %60, %4
  %9 = phi i64 [ 1, %4 ], [ %73, %60 ]
  call void @llvm.dbg.value(metadata i64 %9, metadata !58, metadata !DIExpression()), !dbg !63  ; simple.f90:0:0
  call void @llvm.dbg.value(metadata i32 1, metadata !57, metadata !DIExpression()), !dbg !63  ; simple.f90:0:0
  %10 = trunc i64 %9 to i32, !dbg !64             ; simple.f90:32:45
  %11 = sitofp i32 %10 to float, !dbg !64         ; simple.f90:32:45
  br label %12, !dbg !66                          ; simple.f90:33:16

12:                                               ; preds = %57, %8
  %13 = phi i64 [ 1, %8 ], [ %58, %57 ]
  call void @llvm.dbg.value(metadata i64 %13, metadata !57, metadata !DIExpression()), !dbg !63  ; simple.f90:0:0
  %14 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16, !dbg !64  ; simple.f90:32:45
  %15 = load i64, i64* %5, align 1, !dbg !64      ; simple.f90:32:45
  %16 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %15, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %14, i64 %7), !dbg !67  ; simple.f90:32:16
  %17 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %16, i64 0, i32 0, i32 0, !dbg !68  ; simple.f90:32:23
  %18 = load float*, float** %17, align 1, !dbg !68  ; simple.f90:32:23
  %19 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %16, i64 0, i32 0, i32 6, i64 0, i32 1, !dbg !68  ; simple.f90:32:23
  %20 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %19, i32 0), !dbg !68  ; simple.f90:32:23
  %21 = load i64, i64* %20, align 1, !dbg !68     ; simple.f90:32:23
  %22 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %16, i64 0, i32 0, i32 6, i64 0, i32 2, !dbg !68  ; simple.f90:32:23
  %23 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %22, i32 0), !dbg !68  ; simple.f90:32:23
  %24 = load i64, i64* %23, align 1, !dbg !68     ; simple.f90:32:23
  %25 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %19, i32 1), !dbg !68  ; simple.f90:32:23
  %26 = load i64, i64* %25, align 1, !dbg !68     ; simple.f90:32:23
  %27 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %22, i32 1), !dbg !68  ; simple.f90:32:23
  %28 = load i64, i64* %27, align 1, !dbg !68     ; simple.f90:32:23
  %29 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %28, i64 %26, float* elementtype(float) %18, i64 %9), !dbg !67  ; simple.f90:32:16
  %30 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %24, i64 %21, float* elementtype(float) %29, i64 %13), !dbg !67  ; simple.f90:32:16
  store float %11, float* %30, align 1, !dbg !67  ; simple.f90:32:16
  call void @llvm.dbg.value(metadata i32 1, metadata !56, metadata !DIExpression()), !dbg !63  ; simple.f90:0:0
  br label %31

31:                                               ; preds = %12, %31
  %32 = phi i64 [ %55, %31 ], [ 1, %12 ]
  call void @llvm.dbg.value(metadata i64 %32, metadata !56, metadata !DIExpression()), !dbg !63  ; simple.f90:0:0
  %33 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16, !dbg !69  ; simple.f90:34:50
  %34 = load i64, i64* %5, align 1, !dbg !69      ; simple.f90:34:50
  %35 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %34, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %33, i64 %7), !dbg !70  ; simple.f90:34:18
  %36 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %35, i64 0, i32 1, i32 0, !dbg !71  ; simple.f90:34:25
  %37 = load float*, float** %36, align 1, !dbg !71  ; simple.f90:34:25
  %38 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %35, i64 0, i32 1, i32 6, i64 0, i32 1, !dbg !71  ; simple.f90:34:25
  %39 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %38, i32 0), !dbg !71  ; simple.f90:34:25
  %40 = load i64, i64* %39, align 1, !dbg !71     ; simple.f90:34:25
  %41 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %35, i64 0, i32 1, i32 6, i64 0, i32 2, !dbg !71  ; simple.f90:34:25
  %42 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %41, i32 0), !dbg !71  ; simple.f90:34:25
  %43 = load i64, i64* %42, align 1, !dbg !71     ; simple.f90:34:25
  %44 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %38, i32 1), !dbg !71  ; simple.f90:34:25
  %45 = load i64, i64* %44, align 1, !dbg !71     ; simple.f90:34:25
  %46 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %41, i32 1), !dbg !71  ; simple.f90:34:25
  %47 = load i64, i64* %46, align 1, !dbg !71     ; simple.f90:34:25
  %48 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %38, i32 2), !dbg !71  ; simple.f90:34:25
  %49 = load i64, i64* %48, align 1, !dbg !71     ; simple.f90:34:25
  %50 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %41, i32 2), !dbg !71  ; simple.f90:34:25
  %51 = load i64, i64* %50, align 1, !dbg !71     ; simple.f90:34:25
  %52 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 %51, i64 %49, float* elementtype(float) %37, i64 %32), !dbg !70  ; simple.f90:34:18
  %53 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %47, i64 %45, float* elementtype(float) %52, i64 %9), !dbg !70  ; simple.f90:34:18
  %54 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %43, i64 %40, float* elementtype(float) %53, i64 %13), !dbg !70  ; simple.f90:34:18
  store float %11, float* %54, align 1, !dbg !70  ; simple.f90:34:18
  %55 = add nuw nsw i64 %32, 1, !dbg !72          ; simple.f90:35:16
  call void @llvm.dbg.value(metadata i64 %55, metadata !56, metadata !DIExpression()), !dbg !63  ; simple.f90:0:0
  %56 = icmp eq i64 %55, 11, !dbg !72             ; simple.f90:35:16
  br i1 %56, label %57, label %31, !dbg !72       ; simple.f90:35:16

57:                                               ; preds = %31
  %58 = add nuw nsw i64 %13, 1, !dbg !73          ; simple.f90:36:14
  call void @llvm.dbg.value(metadata i64 %58, metadata !57, metadata !DIExpression()), !dbg !63  ; simple.f90:0:0
  %59 = icmp eq i64 %58, 11, !dbg !73             ; simple.f90:36:14
  br i1 %59, label %60, label %12, !dbg !73       ; simple.f90:36:14

60:                                               ; preds = %57
  call void @llvm.dbg.value(metadata i32 11, metadata !57, metadata !DIExpression()), !dbg !63  ; simple.f90:0:0
  %61 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16, !dbg !74  ; simple.f90:37:40
  %62 = load i64, i64* %5, align 1, !dbg !74      ; simple.f90:37:40
  %63 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %62, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %61, i64 %7), !dbg !75  ; simple.f90:37:14
  %64 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %63, i64 0, i32 2, i32 0, !dbg !76  ; simple.f90:37:21
  %65 = load float*, float** %64, align 1, !dbg !76  ; simple.f90:37:21
  %66 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %63, i64 0, i32 2, i32 6, i64 0, i32 1, !dbg !76  ; simple.f90:37:21
  %67 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %66, i32 0), !dbg !76  ; simple.f90:37:21
  %68 = load i64, i64* %67, align 1, !dbg !76     ; simple.f90:37:21
  %69 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %63, i64 0, i32 2, i32 6, i64 0, i32 2, !dbg !76  ; simple.f90:37:21
  %70 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %69, i32 0), !dbg !76  ; simple.f90:37:21
  %71 = load i64, i64* %70, align 1, !dbg !76     ; simple.f90:37:21
  %72 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %71, i64 %68, float* elementtype(float) %65, i64 11), !dbg !75  ; simple.f90:37:14
  store float 1.100000e+01, float* %72, align 1, !dbg !75  ; simple.f90:37:14
  %73 = add nuw nsw i64 %9, 1, !dbg !77           ; simple.f90:38:12
  call void @llvm.dbg.value(metadata i64 %73, metadata !58, metadata !DIExpression()), !dbg !63  ; simple.f90:0:0
  %74 = icmp eq i64 %73, 11, !dbg !77             ; simple.f90:38:12
  br i1 %74, label %75, label %8, !dbg !77        ; simple.f90:38:12

75:                                               ; preds = %60
  ret void, !dbg !78                              ; simple.f90:41:10
}

; Function Attrs: mustprogress nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #2

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_print_info_(i32* noalias nocapture readonly %0, i32* noalias nocapture readonly dereferenceable(4) %1, i32* noalias nocapture readonly dereferenceable(4) %2, i32* noalias nocapture readonly dereferenceable(4) %3, %"QNCA_a0$float*$rank2$"* noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %4, %"QNCA_a0$float*$rank3$"* noalias nocapture readonly dereferenceable(120) "assumed_shape" "ptrnoalias" %5, %"QNCA_a0$float*$rank1$"* noalias nocapture readonly dereferenceable(72) "assumed_shape" "ptrnoalias" %6) #0 !dbg !79 {
  %8 = alloca [8 x i64], align 16
  %9 = alloca [4 x i8], align 1, !dbg !94         ; simple.f90:43:32
  %10 = alloca [4 x i8], align 1, !dbg !94        ; simple.f90:43:32
  %11 = alloca [4 x i8], align 1, !dbg !94        ; simple.f90:43:32
  call void @llvm.dbg.declare(metadata i32* undef, metadata !81, metadata !DIExpression()), !dbg !94  ; simple.f90:43:32
  call void @llvm.dbg.declare(metadata i32* @anon.11934500c7ed222a0a148892d04ea3ac.1, metadata !82, metadata !DIExpression()), !dbg !95  ; simple.f90:43:35
  call void @llvm.dbg.declare(metadata i32* @anon.11934500c7ed222a0a148892d04ea3ac.1, metadata !83, metadata !DIExpression()), !dbg !96  ; simple.f90:43:38
  call void @llvm.dbg.declare(metadata i32* @anon.11934500c7ed222a0a148892d04ea3ac.1, metadata !84, metadata !DIExpression()), !dbg !97  ; simple.f90:43:41
  call void @llvm.dbg.declare(metadata %"QNCA_a0$float*$rank2$"* %4, metadata !85, metadata !DIExpression()), !dbg !98  ; simple.f90:43:44
  call void @llvm.dbg.declare(metadata %"QNCA_a0$float*$rank3$"* %5, metadata !87, metadata !DIExpression()), !dbg !99  ; simple.f90:43:53
  call void @llvm.dbg.declare(metadata %"QNCA_a0$float*$rank1$"* %6, metadata !89, metadata !DIExpression()), !dbg !100  ; simple.f90:43:62
  call void @llvm.dbg.value(metadata i32 1, metadata !93, metadata !DIExpression()), !dbg !101  ; simple.f90:0:0
  %12 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %4, i64 0, i32 0, !dbg !102  ; simple.f90:49:16
  %13 = load float*, float** %12, align 1, !dbg !102  ; simple.f90:49:16
  %14 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %4, i64 0, i32 6, i64 0, i32 1, !dbg !102  ; simple.f90:49:16
  %15 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %14, i32 0), !dbg !102  ; simple.f90:49:16
  %16 = load i64, i64* %15, align 1, !dbg !102    ; simple.f90:49:16
  %17 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %14, i32 1), !dbg !102  ; simple.f90:49:16
  %18 = load i64, i64* %17, align 1, !dbg !102    ; simple.f90:49:16
  %19 = getelementptr inbounds [4 x i8], [4 x i8]* %9, i64 0, i64 0, !dbg !102  ; simple.f90:49:16
  %20 = getelementptr inbounds [4 x i8], [4 x i8]* %9, i64 0, i64 1, !dbg !102  ; simple.f90:49:16
  %21 = getelementptr inbounds [4 x i8], [4 x i8]* %9, i64 0, i64 2, !dbg !102  ; simple.f90:49:16
  %22 = getelementptr inbounds [4 x i8], [4 x i8]* %9, i64 0, i64 3, !dbg !102  ; simple.f90:49:16
  %23 = bitcast [8 x i64]* %8 to i8*, !dbg !102   ; simple.f90:49:16
  %24 = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %5, i64 0, i32 0, !dbg !103  ; simple.f90:51:18
  %25 = load float*, float** %24, align 1, !dbg !103  ; simple.f90:51:18
  %26 = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %5, i64 0, i32 6, i64 0, i32 1, !dbg !103  ; simple.f90:51:18
  %27 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %26, i32 0), !dbg !103  ; simple.f90:51:18
  %28 = load i64, i64* %27, align 1, !dbg !103    ; simple.f90:51:18
  %29 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %26, i32 1), !dbg !103  ; simple.f90:51:18
  %30 = load i64, i64* %29, align 1, !dbg !103    ; simple.f90:51:18
  %31 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %26, i32 2), !dbg !103  ; simple.f90:51:18
  %32 = load i64, i64* %31, align 1, !dbg !103    ; simple.f90:51:18
  %33 = getelementptr inbounds [4 x i8], [4 x i8]* %10, i64 0, i64 0, !dbg !103  ; simple.f90:51:18
  %34 = getelementptr inbounds [4 x i8], [4 x i8]* %10, i64 0, i64 1, !dbg !103  ; simple.f90:51:18
  %35 = getelementptr inbounds [4 x i8], [4 x i8]* %10, i64 0, i64 2, !dbg !103  ; simple.f90:51:18
  %36 = getelementptr inbounds [4 x i8], [4 x i8]* %10, i64 0, i64 3, !dbg !103  ; simple.f90:51:18
  %37 = getelementptr inbounds %"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* %6, i64 0, i32 0, !dbg !104  ; simple.f90:54:14
  %38 = load float*, float** %37, align 1, !dbg !104  ; simple.f90:54:14
  %39 = getelementptr inbounds %"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* %6, i64 0, i32 6, i64 0, i32 1, !dbg !104  ; simple.f90:54:14
  %40 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %39, i32 0), !dbg !104  ; simple.f90:54:14
  %41 = load i64, i64* %40, align 1, !dbg !104    ; simple.f90:54:14
  %42 = getelementptr inbounds [4 x i8], [4 x i8]* %11, i64 0, i64 0, !dbg !104  ; simple.f90:54:14
  %43 = getelementptr inbounds [4 x i8], [4 x i8]* %11, i64 0, i64 1, !dbg !104  ; simple.f90:54:14
  %44 = getelementptr inbounds [4 x i8], [4 x i8]* %11, i64 0, i64 2, !dbg !104  ; simple.f90:54:14
  %45 = getelementptr inbounds [4 x i8], [4 x i8]* %11, i64 0, i64 3, !dbg !104  ; simple.f90:54:14
  br label %46, !dbg !105                         ; simple.f90:48:14

46:                                               ; preds = %74, %7
  %47 = phi i64 [ 1, %7 ], [ %83, %74 ]
  call void @llvm.dbg.value(metadata i64 %47, metadata !93, metadata !DIExpression()), !dbg !101  ; simple.f90:0:0
  call void @llvm.dbg.value(metadata i32 1, metadata !92, metadata !DIExpression()), !dbg !101  ; simple.f90:0:0
  %48 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %18, float* elementtype(float) %13, i64 %47), !dbg !106  ; simple.f90:49:25
  br label %49, !dbg !107                         ; simple.f90:50:16

49:                                               ; preds = %70, %46
  %50 = phi i64 [ 1, %46 ], [ %71, %70 ]
  call void @llvm.dbg.value(metadata i64 %50, metadata !92, metadata !DIExpression()), !dbg !101  ; simple.f90:0:0
  %51 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 %16, float* elementtype(float) %48, i64 %50), !dbg !106  ; simple.f90:49:25
  %52 = load float, float* %51, align 1, !dbg !106  ; simple.f90:49:25
  store i8 26, i8* %19, align 1, !dbg !102        ; simple.f90:49:16
  store i8 1, i8* %20, align 1, !dbg !102         ; simple.f90:49:16
  store i8 1, i8* %21, align 1, !dbg !102         ; simple.f90:49:16
  store i8 0, i8* %22, align 1, !dbg !102         ; simple.f90:49:16
  %53 = alloca { float }, align 8, !dbg !102      ; simple.f90:49:16
  %54 = getelementptr inbounds { float }, { float }* %53, i64 0, i32 0, !dbg !102  ; simple.f90:49:16
  store float %52, float* %54, align 8, !dbg !102  ; simple.f90:49:16
  %55 = bitcast { float }* %53 to i8*, !dbg !102  ; simple.f90:49:16
  %56 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %23, i32 -1, i64 1239157112576, i8* nonnull %19, i8* nonnull %55) #5, !dbg !102  ; simple.f90:49:16
  call void @llvm.dbg.value(metadata i32 1, metadata !91, metadata !DIExpression()), !dbg !101  ; simple.f90:0:0
  br label %57

57:                                               ; preds = %49, %57
  %58 = phi i64 [ %67, %57 ], [ 1, %49 ]
  call void @llvm.dbg.value(metadata i64 %58, metadata !91, metadata !DIExpression()), !dbg !101  ; simple.f90:0:0
  %59 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 %32, float* elementtype(float) %25, i64 %58), !dbg !108  ; simple.f90:51:27
  %60 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %30, float* elementtype(float) %59, i64 %47), !dbg !108  ; simple.f90:51:27
  %61 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 %28, float* elementtype(float) %60, i64 %50), !dbg !108  ; simple.f90:51:27
  %62 = load float, float* %61, align 1, !dbg !108  ; simple.f90:51:27
  store i8 26, i8* %33, align 1, !dbg !103        ; simple.f90:51:18
  store i8 1, i8* %34, align 1, !dbg !103         ; simple.f90:51:18
  store i8 1, i8* %35, align 1, !dbg !103         ; simple.f90:51:18
  store i8 0, i8* %36, align 1, !dbg !103         ; simple.f90:51:18
  %63 = alloca { float }, align 8, !dbg !103      ; simple.f90:51:18
  %64 = getelementptr inbounds { float }, { float }* %63, i64 0, i32 0, !dbg !103  ; simple.f90:51:18
  store float %62, float* %64, align 8, !dbg !103  ; simple.f90:51:18
  %65 = bitcast { float }* %63 to i8*, !dbg !103  ; simple.f90:51:18
  %66 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %23, i32 -1, i64 1239157112576, i8* nonnull %33, i8* nonnull %65) #5, !dbg !103  ; simple.f90:51:18
  %67 = add nuw i64 %58, 1, !dbg !109             ; simple.f90:52:16
  call void @llvm.dbg.value(metadata i64 %67, metadata !91, metadata !DIExpression()), !dbg !101  ; simple.f90:0:0
  %68 = trunc i64 %67 to i32, !dbg !109           ; simple.f90:52:16
  %69 = icmp slt i32 10, %68, !dbg !109           ; simple.f90:52:16
  br i1 %69, label %70, label %57, !dbg !109      ; simple.f90:52:16

70:                                               ; preds = %57
  %71 = add nuw i64 %50, 1, !dbg !110             ; simple.f90:53:14
  call void @llvm.dbg.value(metadata i64 %71, metadata !92, metadata !DIExpression()), !dbg !101  ; simple.f90:0:0
  %72 = trunc i64 %71 to i32, !dbg !110           ; simple.f90:53:14
  %73 = icmp slt i32 10, %72, !dbg !110           ; simple.f90:53:14
  br i1 %73, label %74, label %49, !dbg !110      ; simple.f90:53:14

74:                                               ; preds = %70
  call void @llvm.dbg.value(metadata i32 undef, metadata !92, metadata !DIExpression()), !dbg !101  ; simple.f90:0:0
  %75 = shl i64 %71, 32, !dbg !104                ; simple.f90:54:14
  %76 = ashr exact i64 %75, 32, !dbg !104         ; simple.f90:54:14
  %77 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 %41, float* elementtype(float) %38, i64 %76), !dbg !111  ; simple.f90:54:23
  %78 = load float, float* %77, align 1, !dbg !111  ; simple.f90:54:23
  store i8 26, i8* %42, align 1, !dbg !104        ; simple.f90:54:14
  store i8 1, i8* %43, align 1, !dbg !104         ; simple.f90:54:14
  store i8 1, i8* %44, align 1, !dbg !104         ; simple.f90:54:14
  store i8 0, i8* %45, align 1, !dbg !104         ; simple.f90:54:14
  %79 = alloca { float }, align 8, !dbg !104      ; simple.f90:54:14
  %80 = getelementptr inbounds { float }, { float }* %79, i64 0, i32 0, !dbg !104  ; simple.f90:54:14
  store float %78, float* %80, align 8, !dbg !104  ; simple.f90:54:14
  %81 = bitcast { float }* %79 to i8*, !dbg !104  ; simple.f90:54:14
  %82 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %23, i32 -1, i64 1239157112576, i8* nonnull %42, i8* nonnull %81) #5, !dbg !104  ; simple.f90:54:14
  %83 = add nuw i64 %47, 1, !dbg !112             ; simple.f90:55:12
  call void @llvm.dbg.value(metadata i64 %83, metadata !93, metadata !DIExpression()), !dbg !101  ; simple.f90:0:0
  %84 = trunc i64 %83 to i32, !dbg !112           ; simple.f90:55:12
  %85 = icmp slt i32 10, %84, !dbg !112           ; simple.f90:55:12
  br i1 %85, label %86, label %46, !dbg !112      ; simple.f90:55:12

86:                                               ; preds = %74
  ret void, !dbg !113                             ; simple.f90:57:10
}

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr #3

; Function Attrs: nofree nounwind uwtable
define internal void @arr_mod_mp_print_arr_(i32* noalias nocapture readonly dereferenceable(4) %0, i32* noalias nocapture readonly dereferenceable(4) %1, i32* noalias nocapture readonly dereferenceable(4) %2, i32* noalias nocapture readonly dereferenceable(4) %3) #0 !dbg !114 {
  %5 = alloca %"QNCA_a0$float*$rank2$", align 8, !dbg !120  ; simple.f90:59:31
  %6 = alloca %"QNCA_a0$float*$rank3$", align 8, !dbg !120  ; simple.f90:59:31
  %7 = alloca %"QNCA_a0$float*$rank1$", align 8, !dbg !120  ; simple.f90:59:31
  call void @llvm.dbg.declare(metadata i32* %0, metadata !116, metadata !DIExpression()), !dbg !120  ; simple.f90:59:31
  call void @llvm.dbg.declare(metadata i32* @anon.11934500c7ed222a0a148892d04ea3ac.1, metadata !117, metadata !DIExpression()), !dbg !121  ; simple.f90:59:34
  call void @llvm.dbg.declare(metadata i32* @anon.11934500c7ed222a0a148892d04ea3ac.1, metadata !118, metadata !DIExpression()), !dbg !122  ; simple.f90:59:37
  call void @llvm.dbg.declare(metadata i32* @anon.11934500c7ed222a0a148892d04ea3ac.1, metadata !119, metadata !DIExpression()), !dbg !123  ; simple.f90:59:40
  %8 = load %"ARR_MOD$.btT_TESTTYPE"*, %"ARR_MOD$.btT_TESTTYPE"** getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 0), align 16, !dbg !124  ; simple.f90:62:12
  %9 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$", %"QNCA_a0$%\22ARR_MOD$.btT_TESTTYPE\22*$rank1$"* @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0), !dbg !124  ; simple.f90:62:12
  %10 = load i64, i64* %9, align 1, !dbg !124     ; simple.f90:62:12
  %11 = load i32, i32* %0, align 1, !dbg !124     ; simple.f90:62:12
  %12 = sext i32 %11 to i64, !dbg !124            ; simple.f90:62:12
  %13 = tail call %"ARR_MOD$.btT_TESTTYPE"* @"llvm.intel.subscript.p0s_ARR_MOD$.btT_TESTTYPEs.i64.i64.p0s_ARR_MOD$.btT_TESTTYPEs.i64"(i8 0, i64 %10, i64 288, %"ARR_MOD$.btT_TESTTYPE"* elementtype(%"ARR_MOD$.btT_TESTTYPE") %8, i64 %12), !dbg !124  ; simple.f90:62:12
  %14 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %13, i64 0, i32 0, i32 0, !dbg !124  ; simple.f90:62:12
  %15 = load float*, float** %14, align 1, !dbg !124  ; simple.f90:62:12
  %16 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %13, i64 0, i32 0, i32 6, i64 0, i32 1, !dbg !124  ; simple.f90:62:12
  %17 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %16, i32 0), !dbg !124  ; simple.f90:62:12
  %18 = load i64, i64* %17, align 1, !dbg !124    ; simple.f90:62:12
  %19 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %13, i64 0, i32 0, i32 6, i64 0, i32 0, !dbg !124  ; simple.f90:62:12
  %20 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %19, i32 0), !dbg !124  ; simple.f90:62:12
  %21 = load i64, i64* %20, align 1, !dbg !124    ; simple.f90:62:12
  %22 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %16, i32 1), !dbg !124  ; simple.f90:62:12
  %23 = load i64, i64* %22, align 1, !dbg !124    ; simple.f90:62:12
  %24 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %19, i32 1), !dbg !124  ; simple.f90:62:12
  %25 = load i64, i64* %24, align 1, !dbg !124    ; simple.f90:62:12
  %26 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %5, i64 0, i32 3, !dbg !124  ; simple.f90:62:12
  %27 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %5, i64 0, i32 1, !dbg !124  ; simple.f90:62:12
  store i64 4, i64* %27, align 8, !dbg !124       ; simple.f90:62:12
  %28 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %5, i64 0, i32 4, !dbg !124  ; simple.f90:62:12
  store i64 2, i64* %28, align 8, !dbg !124       ; simple.f90:62:12
  %29 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %5, i64 0, i32 2, !dbg !124  ; simple.f90:62:12
  store i64 0, i64* %29, align 8, !dbg !124       ; simple.f90:62:12
  %30 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %5, i64 0, i32 6, i64 0, i32 1, !dbg !124  ; simple.f90:62:12
  %31 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %30, i32 0), !dbg !124  ; simple.f90:62:12
  store i64 %18, i64* %31, align 1, !dbg !124     ; simple.f90:62:12
  %32 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %5, i64 0, i32 6, i64 0, i32 2, !dbg !124  ; simple.f90:62:12
  %33 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %32, i32 0), !dbg !124  ; simple.f90:62:12
  store i64 1, i64* %33, align 1, !dbg !124       ; simple.f90:62:12
  %34 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %5, i64 0, i32 6, i64 0, i32 0, !dbg !124  ; simple.f90:62:12
  %35 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %34, i32 0), !dbg !124  ; simple.f90:62:12
  store i64 %21, i64* %35, align 1, !dbg !124     ; simple.f90:62:12
  %36 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %30, i32 1), !dbg !124  ; simple.f90:62:12
  store i64 %23, i64* %36, align 1, !dbg !124     ; simple.f90:62:12
  %37 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %32, i32 1), !dbg !124  ; simple.f90:62:12
  store i64 1, i64* %37, align 1, !dbg !124       ; simple.f90:62:12
  %38 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %34, i32 1), !dbg !124  ; simple.f90:62:12
  store i64 %25, i64* %38, align 1, !dbg !124     ; simple.f90:62:12
  %39 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %5, i64 0, i32 0, !dbg !124  ; simple.f90:62:12
  store float* %15, float** %39, align 8, !dbg !124  ; simple.f90:62:12
  store i64 1, i64* %26, align 8, !dbg !124       ; simple.f90:62:12
  %40 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %13, i64 0, i32 1, i32 0, !dbg !124  ; simple.f90:62:12
  %41 = load float*, float** %40, align 1, !dbg !124  ; simple.f90:62:12
  %42 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %13, i64 0, i32 1, i32 6, i64 0, i32 1, !dbg !124  ; simple.f90:62:12
  %43 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %42, i32 0), !dbg !124  ; simple.f90:62:12
  %44 = load i64, i64* %43, align 1, !dbg !124    ; simple.f90:62:12
  %45 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %13, i64 0, i32 1, i32 6, i64 0, i32 0, !dbg !124  ; simple.f90:62:12
  %46 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %45, i32 0), !dbg !124  ; simple.f90:62:12
  %47 = load i64, i64* %46, align 1, !dbg !124    ; simple.f90:62:12
  %48 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %42, i32 1), !dbg !124  ; simple.f90:62:12
  %49 = load i64, i64* %48, align 1, !dbg !124    ; simple.f90:62:12
  %50 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %45, i32 1), !dbg !124  ; simple.f90:62:12
  %51 = load i64, i64* %50, align 1, !dbg !124    ; simple.f90:62:12
  %52 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %42, i32 2), !dbg !124  ; simple.f90:62:12
  %53 = load i64, i64* %52, align 1, !dbg !124    ; simple.f90:62:12
  %54 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %45, i32 2), !dbg !124  ; simple.f90:62:12
  %55 = load i64, i64* %54, align 1, !dbg !124    ; simple.f90:62:12
  %56 = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %6, i64 0, i32 3, !dbg !124  ; simple.f90:62:12
  %57 = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %6, i64 0, i32 1, !dbg !124  ; simple.f90:62:12
  store i64 4, i64* %57, align 8, !dbg !124       ; simple.f90:62:12
  %58 = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %6, i64 0, i32 4, !dbg !124  ; simple.f90:62:12
  store i64 3, i64* %58, align 8, !dbg !124       ; simple.f90:62:12
  %59 = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %6, i64 0, i32 2, !dbg !124  ; simple.f90:62:12
  store i64 0, i64* %59, align 8, !dbg !124       ; simple.f90:62:12
  %60 = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %6, i64 0, i32 6, i64 0, i32 1, !dbg !124  ; simple.f90:62:12
  %61 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %60, i32 0), !dbg !124  ; simple.f90:62:12
  store i64 %44, i64* %61, align 1, !dbg !124     ; simple.f90:62:12
  %62 = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %6, i64 0, i32 6, i64 0, i32 2, !dbg !124  ; simple.f90:62:12
  %63 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %62, i32 0), !dbg !124  ; simple.f90:62:12
  store i64 1, i64* %63, align 1, !dbg !124       ; simple.f90:62:12
  %64 = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %6, i64 0, i32 6, i64 0, i32 0, !dbg !124  ; simple.f90:62:12
  %65 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %64, i32 0), !dbg !124  ; simple.f90:62:12
  store i64 %47, i64* %65, align 1, !dbg !124     ; simple.f90:62:12
  %66 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %60, i32 1), !dbg !124  ; simple.f90:62:12
  store i64 %49, i64* %66, align 1, !dbg !124     ; simple.f90:62:12
  %67 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %62, i32 1), !dbg !124  ; simple.f90:62:12
  store i64 1, i64* %67, align 1, !dbg !124       ; simple.f90:62:12
  %68 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %64, i32 1), !dbg !124  ; simple.f90:62:12
  store i64 %51, i64* %68, align 1, !dbg !124     ; simple.f90:62:12
  %69 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %60, i32 2), !dbg !124  ; simple.f90:62:12
  store i64 %53, i64* %69, align 1, !dbg !124     ; simple.f90:62:12
  %70 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %62, i32 2), !dbg !124  ; simple.f90:62:12
  store i64 1, i64* %70, align 1, !dbg !124       ; simple.f90:62:12
  %71 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %64, i32 2), !dbg !124  ; simple.f90:62:12
  store i64 %55, i64* %71, align 1, !dbg !124     ; simple.f90:62:12
  %72 = getelementptr inbounds %"QNCA_a0$float*$rank3$", %"QNCA_a0$float*$rank3$"* %6, i64 0, i32 0, !dbg !124  ; simple.f90:62:12
  store float* %41, float** %72, align 8, !dbg !124  ; simple.f90:62:12
  store i64 1, i64* %56, align 8, !dbg !124       ; simple.f90:62:12
  %73 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %13, i64 0, i32 2, i32 0, !dbg !124  ; simple.f90:62:12
  %74 = load float*, float** %73, align 1, !dbg !124  ; simple.f90:62:12
  %75 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %13, i64 0, i32 2, i32 6, i64 0, i32 1, !dbg !124  ; simple.f90:62:12
  %76 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %75, i32 0), !dbg !124  ; simple.f90:62:12
  %77 = load i64, i64* %76, align 1, !dbg !124    ; simple.f90:62:12
  %78 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE", %"ARR_MOD$.btT_TESTTYPE"* %13, i64 0, i32 2, i32 6, i64 0, i32 0, !dbg !124  ; simple.f90:62:12
  %79 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %78, i32 0), !dbg !124  ; simple.f90:62:12
  %80 = load i64, i64* %79, align 1, !dbg !124    ; simple.f90:62:12
  %81 = getelementptr inbounds %"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* %7, i64 0, i32 3, !dbg !124  ; simple.f90:62:12
  %82 = getelementptr inbounds %"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* %7, i64 0, i32 1, !dbg !124  ; simple.f90:62:12
  store i64 4, i64* %82, align 8, !dbg !124       ; simple.f90:62:12
  %83 = getelementptr inbounds %"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* %7, i64 0, i32 4, !dbg !124  ; simple.f90:62:12
  store i64 1, i64* %83, align 8, !dbg !124       ; simple.f90:62:12
  %84 = getelementptr inbounds %"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* %7, i64 0, i32 2, !dbg !124  ; simple.f90:62:12
  store i64 0, i64* %84, align 8, !dbg !124       ; simple.f90:62:12
  %85 = getelementptr inbounds %"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* %7, i64 0, i32 6, i64 0, i32 1, !dbg !124  ; simple.f90:62:12
  %86 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %85, i32 0), !dbg !124  ; simple.f90:62:12
  store i64 %77, i64* %86, align 1, !dbg !124     ; simple.f90:62:12
  %87 = getelementptr inbounds %"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* %7, i64 0, i32 6, i64 0, i32 2, !dbg !124  ; simple.f90:62:12
  %88 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %87, i32 0), !dbg !124  ; simple.f90:62:12
  store i64 1, i64* %88, align 1, !dbg !124       ; simple.f90:62:12
  %89 = getelementptr inbounds %"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* %7, i64 0, i32 6, i64 0, i32 0, !dbg !124  ; simple.f90:62:12
  %90 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %89, i32 0), !dbg !124  ; simple.f90:62:12
  store i64 %80, i64* %90, align 1, !dbg !124     ; simple.f90:62:12
  %91 = getelementptr inbounds %"QNCA_a0$float*$rank1$", %"QNCA_a0$float*$rank1$"* %7, i64 0, i32 0, !dbg !124  ; simple.f90:62:12
  store float* %74, float** %91, align 8, !dbg !124  ; simple.f90:62:12
  store i64 1, i64* %81, align 8, !dbg !124       ; simple.f90:62:12
  call void @arr_mod_mp_print_info_(i32* undef, i32* nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, i32* nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, i32* nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, %"QNCA_a0$float*$rank2$"* nonnull %5, %"QNCA_a0$float*$rank3$"* nonnull %6, %"QNCA_a0$float*$rank1$"* nonnull %7), !dbg !125  ; simple.f90:62:17
  ret void, !dbg !126                             ; simple.f90:66:10
}

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #0 !dbg !25 {
  %1 = alloca i32, align 8
  %2 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.11934500c7ed222a0a148892d04ea3ac.0) #5, !dbg !127  ; simple.f90:70:15
  call void @llvm.dbg.value(metadata i32 1, metadata !29, metadata !DIExpression()), !dbg !128  ; simple.f90:0:0
  store i32 1, i32* %1, align 8, !dbg !129        ; simple.f90:76:9
  br label %3, !dbg !129                          ; simple.f90:76:9

3:                                                ; preds = %3, %0
  %4 = phi i32 [ %5, %3 ], [ 1, %0 ], !dbg !130   ; simple.f90:80:9
  call void @llvm.dbg.value(metadata i32* %1, metadata !29, metadata !DIExpression(DW_OP_deref)), !dbg !128  ; simple.f90:0:0
  call void @arr_mod_mp_allocate_arr_(i32* nonnull %1), !dbg !131  ; simple.f90:77:16
  call void @arr_mod_mp_initialize_arr_(i32* nonnull %1, i32* nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, i32* nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, i32* nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1), !dbg !132  ; simple.f90:78:16
  call void @arr_mod_mp_print_arr_(i32* nonnull %1, i32* nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, i32* nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1, i32* nonnull @anon.11934500c7ed222a0a148892d04ea3ac.1), !dbg !133  ; simple.f90:79:16
  call void @llvm.dbg.value(metadata i32 %4, metadata !29, metadata !DIExpression()), !dbg !128  ; simple.f90:0:0
  %5 = add nuw nsw i32 %4, 1, !dbg !130           ; simple.f90:80:9
  call void @llvm.dbg.value(metadata i32 %5, metadata !29, metadata !DIExpression()), !dbg !128  ; simple.f90:0:0
  store i32 %5, i32* %1, align 8, !dbg !130       ; simple.f90:80:9
  %6 = icmp eq i32 %5, 11, !dbg !130              ; simple.f90:80:9
  br i1 %6, label %7, label %3, !dbg !130         ; simple.f90:80:9

7:                                                ; preds = %3
  ret void, !dbg !134                             ; simple.f90:82:7
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(i32* nocapture readonly) local_unnamed_addr #3

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { nofree nosync nounwind readnone speculatable }
attributes #3 = { nofree "intel-lang"="fortran" }
attributes #4 = { nofree nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
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


; RUN: opt < %s -opaque-pointers -disable-output -dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -disable-output -passes=dopevectorconstprop -dope-vector-global-const-prop=true -debug-only=dope-vector-global-const-prop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

; This test case checks that the fields for the global dope vector
; @arr_mod_mp_a_ were collected and propagated correctly. It was
; created from the following source code:

;      MODULE ARR_MOD
;         REAL, POINTER :: A (:,:)
;
;         CONTAINS
;
;         SUBROUTINE ALLOCATE_ARR()
;           ALLOCATE(A(10, 10))
;
;           RETURN
;         END SUBROUTINE ALLOCATE_ARR
;
;         SUBROUTINE INITIALIZE_ARR(N, M)
;           INTEGER, INTENT(IN) :: N, M
;
;           DO i = 1, N
;             DO j = 1, M
;               A(j, i) = i
;             END DO
;           END DO
;
;           RETURN
;         END SUBROUTINE INITIALIZE_ARR
;
;         SUBROUTINE PRINT_ARR(N, M)
;           INTEGER, INTENT(IN) :: N, M
;
;           DO i = 1, N
;             DO j = 1, M
;               print *, A(j,i)
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
;        CALL ALLOCATE_ARR()
;        CALL INITIALIZE_ARR(10, 10)
;        CALL PRINT_ARR(10, 10)
;      END

; ifx -xCORE-AVX512 -Ofast -flto arr.f90 -mllvm -debug-only=dope-vector-global-const-prop

; The test case basically allocates the global array A in ALLOCATE_ARR, then
; initializes it in INITIALIZE_ARR and the use will be in PRINT_ARR.

; CHECK: Global variable: arr_mod_mp_a_
; CHECK-NEXT:   LLVM Type: QNCA_a0$float*$rank2$
; CHECK-NEXT:   Global dope vector result: Pass
; CHECK-NEXT:   Dope vector analysis result: Pass
; CHECK-NEXT:   Constant propagation status: performed
; CHECK-NEXT:     [0] Array Pointer: Read
; CHECK-NEXT:     [1] Element size: Written | Constant = i64 4
; CHECK-NEXT:     [2] Co-Dimension: Written | Constant = i64 0
; CHECK-NEXT:     [3] Flags: Written
; CHECK-NEXT:     [4] Dimensions: Written | Constant = i64 2
; CHECK-NEXT:     [6][0] Extent: Written | Constant = i64 10
; CHECK-NEXT:     [6][0] Stride: Read | Written | Constant = i64 4
; CHECK-NEXT:     [6][0] Lower Bound: Read | Written | Constant = i64 1
; CHECK-NEXT:     [6][1] Extent: Written | Constant = i64 10
; CHECK-NEXT:     [6][1] Stride: Read | Written | Constant = i64 40
; CHECK-NEXT:     [6][1] Lower Bound: Read | Written | Constant = i64 1

; ModuleID = 'ld-temp.o'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@arr_mod_mp_a_ = internal global %"QNCA_a0$float*$rank2$" { ptr null, i64 0, i64 0, i64 0, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }
@anon.87529b4ebf98830a9107fed24e462e82.0 = internal unnamed_addr constant i32 2
@anon.87529b4ebf98830a9107fed24e462e82.1 = internal unnamed_addr constant i32 10

; Function Attrs: nofree noinline nounwind uwtable
define internal void @arr_mod_mp_allocate_arr_() #0 {
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 5), align 8
  store i64 4, ptr getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 1), align 8
  store i64 2, ptr getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 4), align 16
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 2), align 16
  %1 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %1, align 1
  %2 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 10, ptr %2, align 1
  %3 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 1)
  store i64 1, ptr %3, align 1
  %4 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 0), i32 1)
  store i64 10, ptr %4, align 1
  %5 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 4, ptr %5, align 1
  %6 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 1)
  store i64 40, ptr %6, align 1
  store i64 1073741829, ptr getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
  %7 = tail call i32 @for_allocate_handle(i64 400, ptr @arr_mod_mp_a_, i32 262144, ptr null) #3
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: nofree noinline nounwind uwtable
define internal void @arr_mod_mp_initialize_arr_(ptr noalias nocapture readonly dereferenceable(4) %0, ptr noalias nocapture readonly dereferenceable(4) %1) #0 {
  %3 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 0)
  %4 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %5 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 1)
  %6 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 1)
  br label %7

7:                                                ; preds = %22, %2
  %8 = phi i64 [ 1, %2 ], [ %23, %22 ]
  %9 = trunc i64 %8 to i32
  %10 = sitofp i32 %9 to float
  br label %11

11:                                               ; preds = %11, %7
  %12 = phi i64 [ 1, %7 ], [ %20, %11 ]
  %13 = load ptr, ptr @arr_mod_mp_a_, align 16
  %14 = load i64, ptr %3, align 1
  %15 = load i64, ptr %4, align 1
  %16 = load i64, ptr %5, align 1
  %17 = load i64, ptr %6, align 1
  %18 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %17, i64 %16, ptr elementtype(float) %13, i64 %8)
  %19 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %15, i64 %14, ptr elementtype(float) %18, i64 %12)
  store float %10, ptr %19, align 1
  %20 = add nuw nsw i64 %12, 1
  %21 = icmp eq i64 %20, 11
  br i1 %21, label %22, label %11

22:                                               ; preds = %11
  %23 = add nuw nsw i64 %8, 1
  %24 = icmp eq i64 %23, 11
  br i1 %24, label %25, label %7

25:                                               ; preds = %22
  ret void
}

; Function Attrs: nofree noinline nounwind uwtable
define internal void @arr_mod_mp_print_arr_(ptr noalias nocapture readonly dereferenceable(4) %0, ptr noalias nocapture readonly dereferenceable(4) %1) #0 {
  %3 = alloca [8 x i64], align 16
  %4 = alloca [4 x i8], align 1
  %5 = alloca { float }, align 8
  %6 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 0)
  %7 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %8 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 1)
  %9 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 1)
  %10 = getelementptr inbounds [4 x i8], ptr %4, i64 0, i64 0
  %11 = getelementptr inbounds [4 x i8], ptr %4, i64 0, i64 1
  %12 = getelementptr inbounds [4 x i8], ptr %4, i64 0, i64 2
  %13 = getelementptr inbounds [4 x i8], ptr %4, i64 0, i64 3
  %14 = getelementptr inbounds { float }, ptr %5, i64 0, i32 0
  %15 = bitcast ptr %3 to ptr
  %16 = bitcast ptr %5 to ptr
  br label %17

17:                                               ; preds = %33, %2
  %18 = phi i64 [ 1, %2 ], [ %34, %33 ]
  br label %19

19:                                               ; preds = %19, %17
  %20 = phi i64 [ %30, %19 ], [ 1, %17 ]
  %21 = load ptr, ptr @arr_mod_mp_a_, align 16
  %22 = load i64, ptr %6, align 1
  %23 = load i64, ptr %7, align 1
  %24 = load i64, ptr %8, align 1
  %25 = load i64, ptr %9, align 1
  %26 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %25, i64 %24, ptr elementtype(float) %21, i64 %18)
  %27 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %23, i64 %22, ptr elementtype(float) %26, i64 %20)
  %28 = load float, ptr %27, align 1
  store i8 26, ptr %10, align 1
  store i8 1, ptr %11, align 1
  store i8 1, ptr %12, align 1
  store i8 0, ptr %13, align 1
  store float %28, ptr %14, align 8
  %29 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %15, i32 -1, i64 1239157112576, ptr nonnull %10, ptr nonnull %16) #3
  %30 = add nuw i64 %20, 1
  %31 = trunc i64 %30 to i32
  %32 = icmp slt i32 10, %31
  br i1 %32, label %33, label %19

33:                                               ; preds = %19
  %34 = add nuw i64 %18, 1
  %35 = trunc i64 %34 to i32
  %36 = icmp slt i32 10, %35
  br i1 %36, label %37, label %17

37:                                               ; preds = %33
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr #1

; Function Attrs: nofree noinline nounwind uwtable
define dso_local void @MAIN__() #0 {
  %1 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.87529b4ebf98830a9107fed24e462e82.0) #3
  tail call void @arr_mod_mp_allocate_arr_()
  tail call void @arr_mod_mp_initialize_arr_(ptr nonnull @anon.87529b4ebf98830a9107fed24e462e82.1, ptr nonnull @anon.87529b4ebf98830a9107fed24e462e82.1)
  tail call void @arr_mod_mp_print_arr_(ptr nonnull @anon.87529b4ebf98830a9107fed24e462e82.1, ptr nonnull @anon.87529b4ebf98830a9107fed24e462e82.1)
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

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
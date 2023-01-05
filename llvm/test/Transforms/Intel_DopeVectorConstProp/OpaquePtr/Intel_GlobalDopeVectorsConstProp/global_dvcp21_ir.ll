; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers < %s -passes=dopevectorconstprop -dope-vector-global-const-prop=true -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -dvcp-tile-mv-min-calls=3 -S 2>&1 | FileCheck %s

; This test case checks that DVCP was applied even if aggressive DVCP
; is disabled due to function @bar calls @foo 3 times, and the calls
; are marked as "prefer-inline-tile-choice". This test case was created
; from the following code, with functions @bar and @foo added.

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
; initializes it in INITIALIZE_ARR and the use will be in PRINT_ARR. It is the
; same test case as global_dvcp21.ll, but it checks the IR.

; CHECK: define internal void @arr_mod_mp_initialize_arr_
; CHECK:   %i16 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i11, i64 %i6)
; CHECK:   %i17 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i16, i64 %i10)

; CHECK: define internal void @arr_mod_mp_print_arr_
; CHECK:  %i24 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(float) %i19, i64 %i16)
; CHECK:  %i25 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %i24, i64 %i18)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@arr_mod_mp_a_ = internal global %"QNCA_a0$float*$rank2$" { ptr null, i64 0, i64 0, i64 0, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }
@anon.87529b4ebf98830a9107fed24e462e82.0 = internal unnamed_addr constant i32 2
@anon.87529b4ebf98830a9107fed24e462e82.1 = internal unnamed_addr constant i32 10

; Function Attrs: nofree noinline nounwind uwtable
define internal void @arr_mod_mp_allocate_arr_() #0 {
bb:
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 5), align 8
  store i64 4, ptr getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 1), align 8
  store i64 2, ptr getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 4), align 16
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 2), align 16
  %i = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %i, align 1
  %i1 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 10, ptr %i1, align 1
  %i2 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 1)
  store i64 1, ptr %i2, align 1
  %i3 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 0), i32 1)
  store i64 10, ptr %i3, align 1
  %i4 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 4, ptr %i4, align 1
  %i5 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 1)
  store i64 40, ptr %i5, align 1
  store i64 1073741829, ptr getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 3), align 8
  %i6 = tail call i32 @for_allocate_handle(i64 400, ptr @arr_mod_mp_a_, i32 262144, ptr null) #3
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: nofree noinline nounwind uwtable
define internal void @arr_mod_mp_initialize_arr_(ptr noalias nocapture readonly dereferenceable(4) %arg, ptr noalias nocapture readonly dereferenceable(4) %arg1) #0 {
bb:
  %i = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 0)
  %i2 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %i3 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 1)
  %i4 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 1)
  br label %bb5

bb5:                                              ; preds = %bb20, %bb
  %i6 = phi i64 [ 1, %bb ], [ %i21, %bb20 ]
  %i7 = trunc i64 %i6 to i32
  %i8 = sitofp i32 %i7 to float
  br label %bb9

bb9:                                              ; preds = %bb9, %bb5
  %i10 = phi i64 [ 1, %bb5 ], [ %i18, %bb9 ]
  %i11 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i12 = load i64, ptr %i, align 1
  %i13 = load i64, ptr %i2, align 1
  %i14 = load i64, ptr %i3, align 1
  %i15 = load i64, ptr %i4, align 1
  %i16 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i15, i64 %i14, ptr elementtype(float) %i11, i64 %i6)
  %i17 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i13, i64 %i12, ptr elementtype(float) %i16, i64 %i10)
  store float %i8, ptr %i17, align 1
  %i18 = add nuw nsw i64 %i10, 1
  %i19 = icmp eq i64 %i18, 11
  br i1 %i19, label %bb20, label %bb9

bb20:                                             ; preds = %bb9
  %i21 = add nuw nsw i64 %i6, 1
  %i22 = icmp eq i64 %i21, 11
  br i1 %i22, label %bb23, label %bb5

bb23:                                             ; preds = %bb20
  ret void
}

; Function Attrs: nofree noinline nounwind uwtable
define internal void @arr_mod_mp_print_arr_(ptr noalias nocapture readonly dereferenceable(4) %arg, ptr noalias nocapture readonly dereferenceable(4) %arg1) #0 {
bb:
  %i = alloca [8 x i64], align 16
  %i2 = alloca [4 x i8], align 1
  %i3 = alloca { float }, align 8
  %i4 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 0)
  %i5 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %i6 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 1), i32 1)
  %i7 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank2$", ptr @arr_mod_mp_a_, i64 0, i32 6, i64 0, i32 2), i32 1)
  %i8 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 0
  %i9 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 1
  %i10 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 2
  %i11 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 3
  %i12 = getelementptr inbounds { float }, ptr %i3, i64 0, i32 0
  %i13 = bitcast ptr %i to ptr
  %i14 = bitcast ptr %i3 to ptr
  br label %bb15

bb15:                                             ; preds = %bb31, %bb
  %i16 = phi i64 [ 1, %bb ], [ %i32, %bb31 ]
  br label %bb17

bb17:                                             ; preds = %bb17, %bb15
  %i18 = phi i64 [ %i28, %bb17 ], [ 1, %bb15 ]
  %i19 = load ptr, ptr @arr_mod_mp_a_, align 16
  %i20 = load i64, ptr %i4, align 1
  %i21 = load i64, ptr %i5, align 1
  %i22 = load i64, ptr %i6, align 1
  %i23 = load i64, ptr %i7, align 1
  %i24 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i23, i64 %i22, ptr elementtype(float) %i19, i64 %i16)
  %i25 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i21, i64 %i20, ptr elementtype(float) %i24, i64 %i18)
  %i26 = load float, ptr %i25, align 1
  store i8 26, ptr %i8, align 1
  store i8 1, ptr %i9, align 1
  store i8 1, ptr %i10, align 1
  store i8 0, ptr %i11, align 1
  store float %i26, ptr %i12, align 8
  %i27 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %i13, i32 -1, i64 1239157112576, ptr nonnull %i8, ptr nonnull %i14) #3
  %i28 = add nuw i64 %i18, 1
  %i29 = trunc i64 %i28 to i32
  %i30 = icmp slt i32 10, %i29
  br i1 %i30, label %bb31, label %bb17

bb31:                                             ; preds = %bb17
  %i32 = add nuw i64 %i16, 1
  %i33 = trunc i64 %i32 to i32
  %i34 = icmp slt i32 10, %i33
  br i1 %i34, label %bb35, label %bb15

bb35:                                             ; preds = %bb31
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr #1

; Function Attrs: nofree noinline nounwind uwtable
define dso_local void @MAIN__() #0 {
bb:
  %i = tail call i32 @for_set_reentrancy(ptr nonnull @anon.87529b4ebf98830a9107fed24e462e82.0) #3
  tail call void @arr_mod_mp_allocate_arr_()
  tail call void @arr_mod_mp_initialize_arr_(ptr nonnull @anon.87529b4ebf98830a9107fed24e462e82.1, ptr nonnull @anon.87529b4ebf98830a9107fed24e462e82.1)
  tail call void @arr_mod_mp_print_arr_(ptr nonnull @anon.87529b4ebf98830a9107fed24e462e82.1, ptr nonnull @anon.87529b4ebf98830a9107fed24e462e82.1)
  ret void
}

define dso_local void @foo() {
bb:
  ret void
}

define dso_local void @bar() {
bb:
  call void @foo() #4
  call void @foo() #4
  call void @foo() #4
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
attributes #4 = { "prefer-inline-tile-choice" }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
; end INTEL_FEATURE_SW_ADVANCED

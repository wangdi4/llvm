; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -S -passes='cgscc(inline)' -inline-report=0xe807 -pre-lto-inline-cost -min-callee-args-double-external=4 -min-callee-ivdep-loops-double-external=2 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-OLD %s
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -S -passes='cgscc(inline)' -inline-report=0xe886 -pre-lto-inline-cost -min-callee-args-double-external=4 -min-callee-ivdep-loops-double-external=2 | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-NEW %s

; Check that rpassm_ is inlined by the double callsite external function
; inline heuristic because it is a Fortran function which is called twice
; in the same caller during PRE-LTO inlining, it has enough arguments, and
; has enough loops with IVDEP directives.

; CHECK-OLD-NOT: call void @rpassm_
; CHECK-LABEL: COMPILE FUNC: rpassm_
; CHECK-LABEL: COMPILE FUNC: MAIN__
; CHECK: INLINE: rpassm_{{.*}}Callee has double callsite without local linkage
; CHECK: INLINE: rpassm_{{.*}}Callee has double callsite without local linkage
; CHECK-NEW-NOT: call void @rpassm_

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"main_$BCOX" = internal unnamed_addr global [100 x float] zeroinitializer, align 16
@"main_$ACOX" = internal unnamed_addr global [100 x float] zeroinitializer, align 16
@"main_$INDARR" = internal unnamed_addr global [100 x i32] zeroinitializer, align 16
@0 = internal unnamed_addr constant i32 2
@1 = internal unnamed_addr constant i32 100
@2 = internal unnamed_addr constant i32 100

define void @rpassm_(float* noalias dereferenceable(4) %"rpassm_$A", float* noalias dereferenceable(4) %"rpassm_$B", i32* noalias dereferenceable(4) %"rpassm_$INDARR", i32* noalias dereferenceable(4) %"rpassm_$N") local_unnamed_addr #0 {
alloca_0:
  %"rpassm_$N_fetch.1" = load i32, i32* %"rpassm_$N", align 1
  %rel.1 = icmp slt i32 %"rpassm_$N_fetch.1", 1
  br i1 %rel.1, label %bb7, label %bb2

bb2:                                              ; preds = %alloca_0, %bb2
  %"rpassm_$I.0" = phi i32 [ 1, %alloca_0 ], [ %add.2, %bb2 ]
  %int_sext = zext i32 %"rpassm_$I.0" to i64
  %0 = add nsw i64 %int_sext, -1
  %1 = getelementptr inbounds i32, i32* %"rpassm_$INDARR", i64 %0
  %"rpassm_$INDARR[]_fetch.4" = load i32, i32* %1, align 1
  %int_sext1 = sext i32 %"rpassm_$INDARR[]_fetch.4" to i64
  %2 = add nsw i64 %int_sext1, -1
  %3 = getelementptr inbounds float, float* %"rpassm_$A", i64 %2
  %"rpassm_$A[]_fetch.5" = load float, float* %3, align 1
  %4 = getelementptr inbounds float, float* %"rpassm_$B", i64 %0
  %"rpassm_$B[]_fetch.7" = load float, float* %4, align 1
  %add.1 = fadd reassoc ninf nsz arcp contract afn float %"rpassm_$A[]_fetch.5", %"rpassm_$B[]_fetch.7"
  store float %add.1, float* %3, align 1
  %add.2 = add nuw nsw i32 %"rpassm_$I.0", 1
  %rel.2.not.not = icmp slt i32 %"rpassm_$I.0", %"rpassm_$N_fetch.1"
  br i1 %rel.2.not.not, label %bb2, label %bb3, !llvm.loop !2

bb3:                                              ; preds = %bb2
  %"rpassm_$N_fetch.13.pr" = load i32, i32* %"rpassm_$N", align 1
  %rel.3 = icmp slt i32 %"rpassm_$N_fetch.13.pr", 1
  br i1 %rel.3, label %bb7, label %bb6

bb6:                                              ; preds = %bb3, %bb6
  %"rpassm_$I.1" = phi i32 [ 1, %bb3 ], [ %add.4, %bb6 ]
  %int_sext7 = zext i32 %"rpassm_$I.1" to i64
  %5 = add nsw i64 %int_sext7, -1
  %6 = getelementptr inbounds i32, i32* %"rpassm_$INDARR", i64 %5
  %"rpassm_$INDARR[]_fetch.16" = load i32, i32* %6, align 1
  %int_sext9 = sext i32 %"rpassm_$INDARR[]_fetch.16" to i64
  %7 = add nsw i64 %int_sext9, -1
  %8 = getelementptr inbounds float, float* %"rpassm_$B", i64 %7
  %"rpassm_$B[]_fetch.17" = load float, float* %8, align 1
  %9 = getelementptr inbounds float, float* %"rpassm_$A", i64 %5
  %"rpassm_$A[]_fetch.19" = load float, float* %9, align 1
  %add.3 = fadd reassoc ninf nsz arcp contract afn float %"rpassm_$B[]_fetch.17", %"rpassm_$A[]_fetch.19"
  store float %add.3, float* %8, align 1
  %add.4 = add nuw nsw i32 %"rpassm_$I.1", 1
  %rel.4.not.not = icmp slt i32 %"rpassm_$I.1", %"rpassm_$N_fetch.13.pr"
  br i1 %rel.4.not.not, label %bb6, label %bb7, !llvm.loop !4

bb7:                                              ; preds = %alloca_0, %bb6, %bb3
  ret void
}

define void @MAIN__() local_unnamed_addr #0 {
alloca_1:
  %"main_$CCOX" = alloca float, align 8
  %func_result = call i32 @for_set_reentrancy(i32* nonnull @0) #2
  br label %bb10

bb10:                                             ; preds = %bb10, %alloca_1
  %"main_$I.0" = phi i32 [ 1, %alloca_1 ], [ %add.6, %bb10 ]
  %int_sext = zext i32 %"main_$I.0" to i64
  %0 = add nsw i64 %int_sext, -1
  %1 = getelementptr inbounds [100 x i32], [100 x i32]* @"main_$INDARR", i64 0, i64 %0
  store i32 %"main_$I.0", i32* %1, align 4
  %2 = getelementptr inbounds [100 x float], [100 x float]* @"main_$ACOX", i64 0, i64 %0
  store float 0.000000e+00, float* %2, align 4
  %3 = getelementptr inbounds [100 x float], [100 x float]* @"main_$BCOX", i64 0, i64 %0
  store float 0.000000e+00, float* %3, align 4
  %add.6 = add nuw nsw i32 %"main_$I.0", 1
  %rel.5 = icmp ult i32 %"main_$I.0", 100
  br i1 %rel.5, label %bb10, label %bb13

bb13:                                             ; preds = %bb10
  call void @rpassm_(float* getelementptr inbounds ([100 x float], [100 x float]* @"main_$ACOX", i64 0, i64 0), float* getelementptr inbounds ([100 x float], [100 x float]* @"main_$BCOX", i64 0, i64 0), i32* getelementptr inbounds ([100 x i32], [100 x i32]* @"main_$INDARR", i64 0, i64 0), i32* nonnull @1)
  call void @rpassm_(float* getelementptr inbounds ([100 x float], [100 x float]* @"main_$BCOX", i64 0, i64 0), float* nonnull %"main_$CCOX", i32* getelementptr inbounds ([100 x i32], [100 x i32]* @"main_$INDARR", i64 0, i64 0), i32* nonnull @2)
  ret void
}

declare i32 @for_set_reentrancy(i32* nocapture readonly)

attributes #0 = { nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.vectorize.ivdep_back"}
!4 = distinct !{!4, !3}
; end INTEL_FEATURE_SW_ADVANCED

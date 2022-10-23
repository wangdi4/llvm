; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers < %s -S -passes='cgscc(inline)' -inline-report=0xe807 -pre-lto-inline-cost -min-callee-args-double-external=4 -min-callee-ivdep-loops-double-external=2 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-OLD %s
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -S -passes='cgscc(inline)' -inline-report=0xe886 -pre-lto-inline-cost -min-callee-args-double-external=4 -min-callee-ivdep-loops-double-external=2 | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-NEW %s

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

; Function Attrs: nounwind uwtable
define void @rpassm_(ptr noalias dereferenceable(4) %"rpassm_$A", ptr noalias dereferenceable(4) %"rpassm_$B", ptr noalias dereferenceable(4) %"rpassm_$INDARR", ptr noalias dereferenceable(4) %"rpassm_$N") local_unnamed_addr #0 {
alloca_0:
  %"rpassm_$N_fetch.1" = load i32, ptr %"rpassm_$N", align 1
  %rel.1 = icmp slt i32 %"rpassm_$N_fetch.1", 1
  br i1 %rel.1, label %bb7, label %bb2

bb2:                                              ; preds = %bb2, %alloca_0
  %"rpassm_$I.0" = phi i32 [ 1, %alloca_0 ], [ %add.2, %bb2 ]
  %int_sext = zext i32 %"rpassm_$I.0" to i64
  %i = add nsw i64 %int_sext, -1
  %i1 = getelementptr inbounds i32, ptr %"rpassm_$INDARR", i64 %i
  %"rpassm_$INDARR[]_fetch.4" = load i32, ptr %i1, align 1
  %int_sext1 = sext i32 %"rpassm_$INDARR[]_fetch.4" to i64
  %i2 = add nsw i64 %int_sext1, -1
  %i3 = getelementptr inbounds float, ptr %"rpassm_$A", i64 %i2
  %"rpassm_$A[]_fetch.5" = load float, ptr %i3, align 1
  %i4 = getelementptr inbounds float, ptr %"rpassm_$B", i64 %i
  %"rpassm_$B[]_fetch.7" = load float, ptr %i4, align 1
  %add.1 = fadd reassoc ninf nsz arcp contract afn float %"rpassm_$A[]_fetch.5", %"rpassm_$B[]_fetch.7"
  store float %add.1, ptr %i3, align 1
  %add.2 = add nuw nsw i32 %"rpassm_$I.0", 1
  %rel.2.not.not = icmp slt i32 %"rpassm_$I.0", %"rpassm_$N_fetch.1"
  br i1 %rel.2.not.not, label %bb2, label %bb3, !llvm.loop !2

bb3:                                              ; preds = %bb2
  %"rpassm_$N_fetch.13.pr" = load i32, ptr %"rpassm_$N", align 1
  %rel.3 = icmp slt i32 %"rpassm_$N_fetch.13.pr", 1
  br i1 %rel.3, label %bb7, label %bb6

bb6:                                              ; preds = %bb6, %bb3
  %"rpassm_$I.1" = phi i32 [ 1, %bb3 ], [ %add.4, %bb6 ]
  %int_sext7 = zext i32 %"rpassm_$I.1" to i64
  %i5 = add nsw i64 %int_sext7, -1
  %i6 = getelementptr inbounds i32, ptr %"rpassm_$INDARR", i64 %i5
  %"rpassm_$INDARR[]_fetch.16" = load i32, ptr %i6, align 1
  %int_sext9 = sext i32 %"rpassm_$INDARR[]_fetch.16" to i64
  %i7 = add nsw i64 %int_sext9, -1
  %i8 = getelementptr inbounds float, ptr %"rpassm_$B", i64 %i7
  %"rpassm_$B[]_fetch.17" = load float, ptr %i8, align 1
  %i9 = getelementptr inbounds float, ptr %"rpassm_$A", i64 %i5
  %"rpassm_$A[]_fetch.19" = load float, ptr %i9, align 1
  %add.3 = fadd reassoc ninf nsz arcp contract afn float %"rpassm_$B[]_fetch.17", %"rpassm_$A[]_fetch.19"
  store float %add.3, ptr %i8, align 1
  %add.4 = add nuw nsw i32 %"rpassm_$I.1", 1
  %rel.4.not.not = icmp slt i32 %"rpassm_$I.1", %"rpassm_$N_fetch.13.pr"
  br i1 %rel.4.not.not, label %bb6, label %bb7, !llvm.loop !4

bb7:                                              ; preds = %bb6, %bb3, %alloca_0
  ret void
}

; Function Attrs: nounwind uwtable
define void @MAIN__() local_unnamed_addr #0 {
alloca_1:
  %"main_$CCOX" = alloca float, align 8
  %func_result = call i32 @for_set_reentrancy(ptr nonnull @0)
  br label %bb10

bb10:                                             ; preds = %bb10, %alloca_1
  %"main_$I.0" = phi i32 [ 1, %alloca_1 ], [ %add.6, %bb10 ]
  %int_sext = zext i32 %"main_$I.0" to i64
  %i = add nsw i64 %int_sext, -1
  %i1 = getelementptr inbounds [100 x i32], ptr @"main_$INDARR", i64 0, i64 %i
  store i32 %"main_$I.0", ptr %i1, align 4
  %i2 = getelementptr inbounds [100 x float], ptr @"main_$ACOX", i64 0, i64 %i
  store float 0.000000e+00, ptr %i2, align 4
  %i3 = getelementptr inbounds [100 x float], ptr @"main_$BCOX", i64 0, i64 %i
  store float 0.000000e+00, ptr %i3, align 4
  %add.6 = add nuw nsw i32 %"main_$I.0", 1
  %rel.5 = icmp ult i32 %"main_$I.0", 100
  br i1 %rel.5, label %bb10, label %bb13

bb13:                                             ; preds = %bb10
  call void @rpassm_(ptr @"main_$ACOX", ptr @"main_$BCOX", ptr @"main_$INDARR", ptr nonnull @1)
  call void @rpassm_(ptr @"main_$BCOX", ptr nonnull %"main_$CCOX", ptr @"main_$INDARR", ptr nonnull @2)
  ret void
}

declare i32 @for_set_reentrancy(ptr nocapture readonly)

attributes #0 = { nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.vectorize.ivdep_back"}
!4 = distinct !{!4, !3}
; end INTEL_FEATURE_SW_ADVANCED

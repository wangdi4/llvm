; [notes]
; - different pattern, will revisit it in a later JIRA.

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; *** Source Code ***
;subroutine sub(A, lb, ub)
;  real A(lb:ub, lb:ub, lb:ub)
;  A  = 1
;end
;
; [Note]
; - variable LB, variable UB

;*** IR Dump Before HIR Loop Collapse (hir-loop-collapse) ***

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, sext.i32.i64(%"(i32)var$2_fetch.3$") + -1 * sext.i32.i64(%"sub_$LB_fetch.1"), 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, sext.i32.i64(%"(i32)var$2_fetch.3$") + -1 * sext.i32.i64(%"sub_$LB_fetch.1"), 1   <DO_LOOP>
; CHECK:           |   |   + DO i3 = 0, sext.i32.i64(%"(i32)var$2_fetch.3$") + -1 * sext.i32.i64(%"sub_$LB_fetch.1"), 1   <DO_LOOP>
; CHECK:           |   |   |   (%"sub_$A")[i1 + sext.i32.i64(%"sub_$LB_fetch.1")][i2 + sext.i32.i64(%"sub_$LB_fetch.1")][i3 + sext.i32.i64(%"sub_$LB_fetch.1")] = 1.000000e+00;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION

;HIRLoopCollapse on Function : sub_()
;HIRLoopCollapse::doPreliminaryChecks() failed

;*** IR Dump After HIR Loop Collapse (hir-loop-collapse) ***

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, sext.i32.i64(%"(i32)var$2_fetch.3$") + -1 * sext.i32.i64(%"sub_$LB_fetch.1"), 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, sext.i32.i64(%"(i32)var$2_fetch.3$") + -1 * sext.i32.i64(%"sub_$LB_fetch.1"), 1   <DO_LOOP>
; CHECK:           |   |   + DO i3 = 0, sext.i32.i64(%"(i32)var$2_fetch.3$") + -1 * sext.i32.i64(%"sub_$LB_fetch.1"), 1   <DO_LOOP>
; CHECK:           |   |   |   (%"sub_$A")[i1 + sext.i32.i64(%"sub_$LB_fetch.1")][i2 + sext.i32.i64(%"sub_$LB_fetch.1")][i3 + sext.i32.i64(%"sub_$LB_fetch.1")] = 1.000000e+00;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION


; [Note]
; Below are the detailed dump (-hir-details-dims) for both before and after collapsing.
;

;*** IR Dump Before HIR Loop Collapse (hir-loop-collapse) ***

;<0>          BEGIN REGION { }
;<40>               + DO i1 = 0, sext.i32.i64(%"(i32)var$2_fetch.3$") + -1 * sext.i32.i64(%"sub_$LB_fetch.1"), 1   <DO_LOOP>
;<41>               |   + DO i2 = 0, sext.i32.i64(%"(i32)var$2_fetch.3$") + -1 * sext.i32.i64(%"sub_$LB_fetch.1"), 1   <DO_LOOP>
;<42>               |   |   + DO i3 = 0, sext.i32.i64(%"(i32)var$2_fetch.3$") + -1 * sext.i32.i64(%"sub_$LB_fetch.1"), 1   <DO_LOOP>
;<13>               |   |   |   (%"sub_$A")[%"sub_$LB_fetch.1":i1 + sext.i32.i64(%"sub_$LB_fetch.1"):4 * (sext.i32.i64((1 + (-1 * %"sub_$LB_fetch.1") + %"(i32)var$2_fetch.3$")) * sext.i32.i64((1 + (-1 * %"sub_$LB_fetch.1") + %"(i32)var$2_fetch.3$")))(float*:0)][%"sub_$LB_fetch.1":i2 + sext.i32.i64(%"sub_$LB_fetch.1"):4 * sext.i32.i64((1 + (-1 * %"sub_$LB_fetch.1") + %"(i32)var$2_fetch.3$"))(float*:0)][%"sub_$LB_fetch.1":i3 + sext.i32.i64(%"sub_$LB_fetch.1"):4(float*:0)] = 1.000000e+00;
;<42>               |   |   + END LOOP
;<41>               |   + END LOOP
;<40>               + END LOOP
;<0>          END REGION

;*** IR Dump After HIR Loop Collapse (hir-loop-collapse) ***

;<0>          BEGIN REGION { }
;<40>               + DO i1 = 0, sext.i32.i64(%"(i32)var$2_fetch.3$") + -1 * sext.i32.i64(%"sub_$LB_fetch.1"), 1   <DO_LOOP>
;<41>               |   + DO i2 = 0, sext.i32.i64(%"(i32)var$2_fetch.3$") + -1 * sext.i32.i64(%"sub_$LB_fetch.1"), 1   <DO_LOOP>
;<42>               |   |   + DO i3 = 0, sext.i32.i64(%"(i32)var$2_fetch.3$") + -1 * sext.i32.i64(%"sub_$LB_fetch.1"), 1   <DO_LOOP>
;<13>               |   |   |   (%"sub_$A")[%"sub_$LB_fetch.1":i1 + sext.i32.i64(%"sub_$LB_fetch.1"):4 * (sext.i32.i64((1 + (-1 * %"sub_$LB_fetch.1") + %"(i32)var$2_fetch.3$")) * sext.i32.i64((1 + (-1 * %"sub_$LB_fetch.1") + %"(i32)var$2_fetch.3$")))(float*:0)][%"sub_$LB_fetch.1":i2 + sext.i32.i64(%"sub_$LB_fetch.1"):4 * sext.i32.i64((1 + (-1 * %"sub_$LB_fetch.1") + %"(i32)var$2_fetch.3$"))(float*:0)][%"sub_$LB_fetch.1":i3 + sext.i32.i64(%"sub_$LB_fetch.1"):4(float*:0)] = 1.000000e+00;
;<42>               |   |   + END LOOP
;<41>               |   + END LOOP
;<40>               + END LOOP
;<0>          END REGION



;Module Before HIR
; ModuleID = 'collapse5.f90'
source_filename = "collapse5.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind uwtable
define void @sub_(ptr noalias nocapture dereferenceable(4) %"sub_$A", ptr noalias nocapture readonly dereferenceable(4) %"sub_$LB", ptr noalias nocapture readonly dereferenceable(4) %"sub_$UB") local_unnamed_addr #0 {
alloca_0:
  %"sub_$LB_fetch.1" = load i32, ptr %"sub_$LB", align 1
  %"sub_$UB_fetch.2" = load float, ptr %"sub_$UB", align 1
  %"(i32)var$2_fetch.3$" = fptosi float %"sub_$UB_fetch.2" to i32
  %sub.1 = sub i32 1, %"sub_$LB_fetch.1"
  %add.1 = add i32 %sub.1, %"(i32)var$2_fetch.3$"
  %int_sext = sext i32 %add.1 to i64
  %mul.1 = shl nsw i64 %int_sext, 2
  %mul.2 = mul nsw i64 %mul.1, %int_sext
  %int_sext5 = sext i32 %"sub_$LB_fetch.1" to i64
  %int_sext7 = sext i32 %"(i32)var$2_fetch.3$" to i64
  %sub.3 = sub nsw i64 1, %int_sext5
  %add.3 = add nsw i64 %sub.3, %int_sext7
  %rel.4.not33 = icmp slt i64 %add.3, 1
  br i1 %rel.4.not33, label %loop_exit17, label %loop_test11.preheader.preheader

loop_test11.preheader.preheader:                  ; preds = %alloca_0
  %0 = add nsw i64 %int_sext7, 2
  %1 = sub nsw i64 %0, %int_sext5
  br label %loop_test7.preheader.preheader

loop_body8:                                       ; preds = %loop_body8.preheader, %loop_body8
  %"$loop_ctr.035" = phi i64 [ %add.17, %loop_body8 ], [ 1, %loop_body8.preheader ]
  %"var$9.034" = phi i64 [ %add.16, %loop_body8 ], [ %int_sext5, %loop_body8.preheader ]
  %"sub_$A[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext5, i64 4, ptr elementtype(float) nonnull %"sub_$A[][]", i64 %"var$9.034")
  store float 1.000000e+00, ptr %"sub_$A[][][]", align 1
  %add.16 = add nsw i64 %"var$9.034", 1
  %add.17 = add nuw nsw i64 %"$loop_ctr.035", 1
  %exitcond = icmp eq i64 %add.17, %1
  br i1 %exitcond, label %loop_exit9, label %loop_body8

loop_exit9:                                       ; preds = %loop_body8
  %add.18 = add nsw i64 %"var$10.038", 1
  %add.19 = add nuw nsw i64 %"$loop_ctr1.037", 1
  %exitcond42 = icmp eq i64 %add.19, %1
  br i1 %exitcond42, label %loop_exit13, label %loop_body8.preheader

loop_body8.preheader:                             ; preds = %loop_exit9, %loop_test7.preheader.preheader
  %"var$10.038" = phi i64 [ %add.18, %loop_exit9 ], [ %int_sext5, %loop_test7.preheader.preheader ]
  %"$loop_ctr1.037" = phi i64 [ %add.19, %loop_exit9 ], [ 1, %loop_test7.preheader.preheader ]
  %"sub_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %int_sext5, i64 %mul.1, ptr elementtype(float) nonnull %"sub_$A[]", i64 %"var$10.038")
  br label %loop_body8

loop_exit13:                                      ; preds = %loop_exit9
  %add.20 = add nsw i64 %"var$11.041", 1
  %add.21 = add nuw nsw i64 %"$loop_ctr2.040", 1
  %exitcond43 = icmp eq i64 %add.21, %1
  br i1 %exitcond43, label %loop_exit17.loopexit, label %loop_test7.preheader.preheader

loop_test7.preheader.preheader:                   ; preds = %loop_exit13, %loop_test11.preheader.preheader
  %"var$11.041" = phi i64 [ %add.20, %loop_exit13 ], [ %int_sext5, %loop_test11.preheader.preheader ]
  %"$loop_ctr2.040" = phi i64 [ %add.21, %loop_exit13 ], [ 1, %loop_test11.preheader.preheader ]
  %"sub_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %int_sext5, i64 %mul.2, ptr elementtype(float) nonnull %"sub_$A", i64 %"var$11.041")
  br label %loop_body8.preheader

loop_exit17.loopexit:                             ; preds = %loop_exit13
  br label %loop_exit17

loop_exit17:                                      ; preds = %loop_exit17.loopexit, %alloca_0
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nofree nosync nounwind readnone speculatable }

!omp_offload.info = !{}

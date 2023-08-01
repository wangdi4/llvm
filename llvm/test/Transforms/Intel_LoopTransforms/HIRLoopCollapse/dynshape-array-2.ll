; [notes]
; - different pattern, will revisit it in a later JIRA.

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -hir-create-function-level-region  -disable-output < %s 2>&1 | FileCheck %s
;
; *** Source Code ***
;subroutine sub(A, lb)
;  real A(lb:100, lb:100, lb:100)
;  A  = 1
;end

; [Note]
; - variable LB, fixed UB


;*** IR Dump Before HIR Loop Collapse (hir-loop-collapse) ***
;Function: sub_

; CHECK:    BEGIN REGION { }
; CHECK:           + DO i1 = 0, -1 * sext.i32.i64(%"sub_$LB_fetch.1") + 100, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, -1 * sext.i32.i64(%"sub_$LB_fetch.1") + 100, 1   <DO_LOOP>
; CHECK:           |   |   + DO i3 = 0, -1 * sext.i32.i64(%"sub_$LB_fetch.1") + 100, 1   <DO_LOOP>
; CHECK:           |   |   |   (%"sub_$A")[i1 + sext.i32.i64(%"sub_$LB_fetch.1")][i2 + sext.i32.i64(%"sub_$LB_fetch.1")][i3 + sext.i32.i64(%"sub_$LB_fetch.1")] = 1.000000e+00;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION

;HIRLoopCollapse on Function : sub_()
;HIRLoopCollapse::doPreliminaryChecks() failed

;*** IR Dump After HIR Loop Collapse (hir-loop-collapse) ***
;Function: sub_

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, -1 * sext.i32.i64(%"sub_$LB_fetch.1") + 100, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, -1 * sext.i32.i64(%"sub_$LB_fetch.1") + 100, 1   <DO_LOOP>
; CHECK:           |   |   + DO i3 = 0, -1 * sext.i32.i64(%"sub_$LB_fetch.1") + 100, 1   <DO_LOOP>
; CHECK:           |   |   |   (%"sub_$A")[i1 + sext.i32.i64(%"sub_$LB_fetch.1")][i2 + sext.i32.i64(%"sub_$LB_fetch.1")][i3 + sext.i32.i64(%"sub_$LB_fetch.1")] = 1.000000e+00;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION


; [Note]
; Below are the detailed dump (-hir-details-dims) for both before and after collapsing.
;

;*** IR Dump After HIR Loop Collapse (hir-loop-collapse) ***
; [Note]
; This is a different pattern than those matched in the current pass.
; Pattern is abstracted as:
; -
; -
; -

;<0>          BEGIN REGION { }
;<40>               + DO i1 = 0, -1 * sext.i32.i64(%"sub_$LB_fetch.1") + 100, 1   <DO_LOOP>
;<41>               |   + DO i2 = 0, -1 * sext.i32.i64(%"sub_$LB_fetch.1") + 100, 1   <DO_LOOP>
;<42>               |   |   + DO i3 = 0, -1 * sext.i32.i64(%"sub_$LB_fetch.1") + 100, 1   <DO_LOOP>
;<13>               |   |   |   (%"sub_$A")[%"sub_$LB_fetch.1":i1 + sext.i32.i64(%"sub_$LB_fetch.1"):4 * (sext.i32.i64((101 + (-1 * %"sub_$LB_fetch.1"))) * sext.i32.i64((101 + (-1 * %"sub_$LB_fetch.1"))))(float*:0)][%"sub_$LB_fetch.1":i2 + sext.i32.i64(%"sub_$LB_fetch.1"):4 * sext.i32.i64((101 + (-1 * %"sub_$LB_fetch.1")))(float*:0)][%"sub_$LB_fetch.1":i3 + sext.i32.i64(%"sub_$LB_fetch.1"):4(float*:0)] = 1.000000e+00;
;<42>               |   |   + END LOOP
;<41>               |   + END LOOP
;<40>               + END LOOP
;<0>          END REGION


;Module Before HIR
; ModuleID = 'collapse3.f90'
source_filename = "collapse3.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind uwtable
define void @sub_(ptr noalias nocapture dereferenceable(4) %"sub_$A", ptr noalias nocapture readonly dereferenceable(4) %"sub_$LB") local_unnamed_addr #0 {
alloca_0:
  %"sub_$LB_fetch.1" = load i32, ptr %"sub_$LB", align 1
  %add.1 = sub i32 101, %"sub_$LB_fetch.1"
  %int_sext = sext i32 %add.1 to i64
  %mul.1 = shl nsw i64 %int_sext, 2
  %mul.2 = mul nsw i64 %mul.1, %int_sext
  %int_sext5 = sext i32 %"sub_$LB_fetch.1" to i64
  %rel.4.not30 = icmp sgt i32 %"sub_$LB_fetch.1", 100
  br i1 %rel.4.not30, label %loop_exit16, label %loop_test10.preheader.preheader

loop_test10.preheader.preheader:                  ; preds = %alloca_0
  %0 = sub nsw i64 102, %int_sext5
  br label %loop_test6.preheader.preheader

loop_body7:                                       ; preds = %loop_body7.preheader, %loop_body7
  %"$loop_ctr.032" = phi i64 [ %add.17, %loop_body7 ], [ 1, %loop_body7.preheader ]
  %"var$8.031" = phi i64 [ %add.16, %loop_body7 ], [ %int_sext5, %loop_body7.preheader ]
  %"sub_$A_entry[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext5, i64 4, ptr elementtype(float) nonnull %"sub_$A_entry[][]", i64 %"var$8.031")
  store float 1.000000e+00, ptr %"sub_$A_entry[][][]", align 1
  %add.16 = add nsw i64 %"var$8.031", 1
  %add.17 = add nuw nsw i64 %"$loop_ctr.032", 1
  %exitcond = icmp eq i64 %add.17, %0
  br i1 %exitcond, label %loop_exit8, label %loop_body7

loop_exit8:                                       ; preds = %loop_body7
  %add.18 = add nsw i64 %"var$9.035", 1
  %add.19 = add nuw nsw i64 %"$loop_ctr1.034", 1
  %exitcond39 = icmp eq i64 %add.19, %0
  br i1 %exitcond39, label %loop_exit12, label %loop_body7.preheader

loop_body7.preheader:                             ; preds = %loop_exit8, %loop_test6.preheader.preheader
  %"var$9.035" = phi i64 [ %add.18, %loop_exit8 ], [ %int_sext5, %loop_test6.preheader.preheader ]
  %"$loop_ctr1.034" = phi i64 [ %add.19, %loop_exit8 ], [ 1, %loop_test6.preheader.preheader ]
  %"sub_$A_entry[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %int_sext5, i64 %mul.1, ptr elementtype(float) nonnull %"sub_$A_entry[]", i64 %"var$9.035")
  br label %loop_body7

loop_exit12:                                      ; preds = %loop_exit8
  %add.20 = add nsw i64 %"var$10.038", 1
  %add.21 = add nuw nsw i64 %"$loop_ctr2.037", 1
  %exitcond40 = icmp eq i64 %add.21, %0
  br i1 %exitcond40, label %loop_exit16.loopexit, label %loop_test6.preheader.preheader

loop_test6.preheader.preheader:                   ; preds = %loop_exit12, %loop_test10.preheader.preheader
  %"var$10.038" = phi i64 [ %add.20, %loop_exit12 ], [ %int_sext5, %loop_test10.preheader.preheader ]
  %"$loop_ctr2.037" = phi i64 [ %add.21, %loop_exit12 ], [ 1, %loop_test10.preheader.preheader ]
  %"sub_$A_entry[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %int_sext5, i64 %mul.2, ptr elementtype(float) nonnull %"sub_$A", i64 %"var$10.038")
  br label %loop_body7.preheader

loop_exit16.loopexit:                             ; preds = %loop_exit12
  br label %loop_exit16

loop_exit16:                                      ; preds = %loop_exit16.loopexit, %alloca_0
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nofree nosync nounwind readnone speculatable }

!omp_offload.info = !{}

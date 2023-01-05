; [notes]
; - different pattern, will revisit it in a later JIRA.

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; *** Source Code ***
;subroutine sub(A, ub)
;  real A(3:ub, -3:ub, -3:ub)
;  A  = 1
;end
;
; [Note]
; - variable (different) LB, variable UB

;*** IR Dump Before HIR Loop Collapse (hir-loop-collapse) ***

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, sext.i32.i64(%"(i32)var$2_fetch.2$") + 3, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, sext.i32.i64(%"(i32)var$2_fetch.2$") + 3, 1   <DO_LOOP>
; CHECK:           |   |   + DO i3 = 0, sext.i32.i64(%"(i32)var$2_fetch.2$") + -3, 1   <DO_LOOP>
; CHECK:           |   |   |   (%"sub_$A")[i1][i2][i3] = 1.000000e+00;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION

;HIRLoopCollapse on Function : sub_()
;HIRLoopCollapse::doPreliminaryChecks() failed

;*** IR Dump After HIR Loop Collapse (hir-loop-collapse) ***

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, sext.i32.i64(%"(i32)var$2_fetch.2$") + 3, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, sext.i32.i64(%"(i32)var$2_fetch.2$") + 3, 1   <DO_LOOP>
; CHECK:           |   |   + DO i3 = 0, sext.i32.i64(%"(i32)var$2_fetch.2$") + -3, 1   <DO_LOOP>
; CHECK:           |   |   |   (%"sub_$A")[i1][i2][i3] = 1.000000e+00;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION


; [Note]
; Below are the detailed dump (-hir-details-dims) for both before and after collapsing.
;
;*** IR Dump Before HIR Loop Collapse (hir-loop-collapse) ***

;<0>          BEGIN REGION { }
;<38>               + DO i1 = 0, sext.i32.i64(%"(i32)var$2_fetch.2$") + 3, 1   <DO_LOOP>
;<39>               |   + DO i2 = 0, sext.i32.i64(%"(i32)var$2_fetch.2$") + 3, 1   <DO_LOOP>
;<40>               |   |   + DO i3 = 0, sext.i32.i64(%"(i32)var$2_fetch.2$") + -3, 1   <DO_LOOP>
;<15>               |   |   |   (%"sub_$A")[0:i1:4 * (sext.i32.i64((4 + %"(i32)var$2_fetch.2$")) * sext.i32.i64((-2 + %"(i32)var$2_fetch.2$")))(float*:0)][0:i2:4 * sext.i32.i64((-2 + %"(i32)var$2_fetch.2$"))(float*:0)][0:i3:4(float*:0)] = 1.000000e+00;
;<40>               |   |   + END LOOP
;<39>               |   + END LOOP
;<38>               + END LOOP
;<0>          END REGION

;*** IR Dump After HIR Loop Collapse (hir-loop-collapse) ***

;<0>          BEGIN REGION { }
;<38>               + DO i1 = 0, sext.i32.i64(%"(i32)var$2_fetch.2$") + 3, 1   <DO_LOOP>
;<39>               |   + DO i2 = 0, sext.i32.i64(%"(i32)var$2_fetch.2$") + 3, 1   <DO_LOOP>
;<40>               |   |   + DO i3 = 0, sext.i32.i64(%"(i32)var$2_fetch.2$") + -3, 1   <DO_LOOP>
;<15>               |   |   |   (%"sub_$A")[0:i1:4 * (sext.i32.i64((4 + %"(i32)var$2_fetch.2$")) * sext.i32.i64((-2 + %"(i32)var$2_fetch.2$")))(float*:0)][0:i2:4 * sext.i32.i64((-2 + %"(i32)var$2_fetch.2$"))(float*:0)][0:i3:4(float*:0)] = 1.000000e+00;
;<40>               |   |   + END LOOP
;<39>               |   + END LOOP
;<38>               + END LOOP
;<0>          END REGION



;Module Before HIR
; ModuleID = 'collapse4.f90'
source_filename = "collapse4.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind uwtable
define void @sub_(float* noalias nocapture dereferenceable(4) %"sub_$A", float* noalias nocapture readonly dereferenceable(4) %"sub_$UB") local_unnamed_addr #0 {
alloca_0:
  %"sub_$UB_fetch.1" = load float, float* %"sub_$UB", align 1
  %"(i32)var$2_fetch.2$" = fptosi float %"sub_$UB_fetch.1" to i32
  %add.1 = add nsw i32 %"(i32)var$2_fetch.2$", -2
  %int_sext = sext i32 %add.1 to i64
  %mul.1 = shl nsw i64 %int_sext, 2
  %add.2 = add nsw i32 %"(i32)var$2_fetch.2$", 4
  %int_sext3 = sext i32 %add.2 to i64
  %mul.2 = mul nsw i64 %mul.1, %int_sext3
  %rel.4.not15 = icmp slt i32 %"(i32)var$2_fetch.2$", 3
  %rel.5.not18 = icmp slt i32 %"(i32)var$2_fetch.2$", -3
  br i1 %rel.5.not18, label %loop_exit16, label %loop_test10.preheader.preheader

loop_test10.preheader.preheader:                  ; preds = %alloca_0
  %int_sext5 = sext i32 %"(i32)var$2_fetch.2$" to i64
  %0 = add nsw i64 %int_sext5, 1
  br label %loop_test6.preheader.preheader

loop_body7:                                       ; preds = %loop_body7.preheader, %loop_body7
  %"var$8.016" = phi i64 [ %add.16, %loop_body7 ], [ 3, %loop_body7.preheader ]
  %"sub_$A[][][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 3, i64 4, float* elementtype(float) nonnull %"sub_$A[][]", i64 %"var$8.016")
  store float 1.000000e+00, float* %"sub_$A[][][]", align 1
  %add.16 = add nuw nsw i64 %"var$8.016", 1
  %exitcond = icmp eq i64 %add.16, %0
  br i1 %exitcond, label %loop_exit8.loopexit, label %loop_body7

loop_exit8.loopexit:                              ; preds = %loop_body7
  br label %loop_exit8

loop_exit8:                                       ; preds = %loop_exit8.loopexit, %loop_test6.preheader
  %add.18 = add nsw i64 %"var$9.020", 1
  %exitcond24 = icmp eq i64 %add.18, %0
  br i1 %exitcond24, label %loop_exit12, label %loop_test6.preheader

loop_test6.preheader:                             ; preds = %loop_test6.preheader.preheader, %loop_exit8
  %"var$9.020" = phi i64 [ %add.18, %loop_exit8 ], [ -3, %loop_test6.preheader.preheader ]
  %"sub_$A[][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 -3, i64 %mul.1, float* elementtype(float) nonnull %"sub_$A[]", i64 %"var$9.020")
  br i1 %rel.4.not15, label %loop_exit8, label %loop_body7.preheader

loop_body7.preheader:                             ; preds = %loop_test6.preheader
  br label %loop_body7

loop_exit12:                                      ; preds = %loop_exit8
  %add.20 = add nsw i64 %"var$10.023", 1
  %exitcond25 = icmp eq i64 %add.20, %0
  br i1 %exitcond25, label %loop_exit16.loopexit, label %loop_test6.preheader.preheader

loop_test6.preheader.preheader:                   ; preds = %loop_exit12, %loop_test10.preheader.preheader
  %"var$10.023" = phi i64 [ %add.20, %loop_exit12 ], [ -3, %loop_test10.preheader.preheader ]
  %"sub_$A[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 -3, i64 %mul.2, float* elementtype(float) nonnull %"sub_$A", i64 %"var$10.023")
  br label %loop_test6.preheader

loop_exit16.loopexit:                             ; preds = %loop_exit12
  br label %loop_exit16

loop_exit16:                                      ; preds = %loop_exit16.loopexit, %alloca_0
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #1

attributes #0 = { nofree nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nofree nosync nounwind readnone speculatable }

!omp_offload.info = !{}

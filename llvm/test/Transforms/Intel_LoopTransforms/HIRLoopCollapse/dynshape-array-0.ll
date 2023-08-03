; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -hir-details -disable-output < %s 2>&1 | FileCheck %s
;
; *** Source Code ***
;subroutine sub (A, N1,N2,N3)
;  real*8 A(N1,N2,N3)
;  A = 0.0
;  end subroutine sub
;
;
; [Note]
; use -hir-details-dims flag to show the dimensional details when debugging.

;*** IR Dump Before HIR Loop Collapse ***
;Function: sub_

;      BEGIN REGION { }
;            + Ztt: No
;            + DO i1 = 0, zext.i32.i64(%"sub_$N3_fetch") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;            |
;            |   + Ztt: if (%"sub_$N2_fetch" >= 1)
;            |   + DO i2 = 0, sext.i32.i64(%"sub_$N2_fetch") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;            |   |
;            |   |   + Ztt: if (%"sub_$N1_fetch" >= 1)
;            |   |   + DO i3 = 0, sext.i32.i64(%"sub_$N1_fetch") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;            |   |   |   (%"sub_$A")[i1][i2][i3] = 0.000000e+00;
;            |   |   + END LOOP
;            |   + END LOOP
;            + END LOOP
;      END REGION


;*** IR Dump After HIR Loop Collapse ***
;Function: sub_

;CHECK:      BEGIN REGION { modified }
;
;CHECK:            + Ztt: if (%"sub_$N1_fetch" >= 1 & %"sub_$N2_fetch" >= 1)
;CHECK:            + DO i64 i1 = 0, (zext.i32.i64(%"sub_$N3_fetch") * sext.i32.i64(%"sub_$N1_fetch") * sext.i32.i64(%"sub_$N2_fetch")) + -1, 1   <DO_LOOP> 
;CHECK:            |   (%"sub_$A")[0][0][i1] = 0.000000e+00;
;CHECK:            + END LOOP
;CHECK:      END REGION


; [Note]
; Below are the detailed dump (-hir-details-dims) for both before and after collapsing.
;
; *** BEFORE ***
;
;       BEGIN REGION { }
;             + DO i1 = 0, zext.i32.i64(%"sub_$N3_fetch") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;             |   + DO i2 = 0, sext.i32.i64(%"sub_$N2_fetch") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;             |   |   + DO i3 = 0, sext.i32.i64(%"sub_$N1_fetch") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;             |   |   |   (%"sub_$A")[0:i1:8 * (sext.i32.i64(%"sub_$N2_fetch") * sext.i32.i64(%"sub_$N1_fetch"))(double*:0)][0:i2:8 * sext.i32.i64(%"sub_$N1_fetch")(double*:0)][0:i3:8(double*:0)] = 0.000000e+00;
;             |   |   + END LOOP
;             |   + END LOOP
;             + END LOOP
;       END REGION

; *** AFTER ***
;      BEGIN REGION { modified }
;            + DO i1 = 0, (zext.i32.i64(%"sub_$N3_fetch") * sext.i32.i64(%"sub_$N1_fetch") * sext.i32.i64(%"sub_$N2_fetch")) + -1, 1   <DO_LOOP>
;            |   (%"sub_$A")[0][0][0:i1:8(double*:0)] = 0.000000e+00;
;            + END LOOP
;      END REGION


;Module Before HIR
; ModuleID = 'collapse.f90'
source_filename = "collapse.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @sub_(ptr noalias nocapture dereferenceable(8) %"sub_$A", ptr noalias nocapture readonly dereferenceable(4) %"sub_$N1", ptr noalias nocapture readonly dereferenceable(4) %"sub_$N2", ptr noalias nocapture readonly dereferenceable(4) %"sub_$N3") local_unnamed_addr #0 {
alloca_0:
  %"sub_$N1_fetch" = load i32, ptr %"sub_$N1", align 1
  %"sub_$N2_fetch" = load i32, ptr %"sub_$N2", align 1
  %"sub_$N3_fetch" = load i32, ptr %"sub_$N3", align 1
  %int_sext = sext i32 %"sub_$N1_fetch" to i64
  %mul = shl nsw i64 %int_sext, 3
  %int_sext1 = sext i32 %"sub_$N2_fetch" to i64
  %mul2 = mul nsw i64 %mul, %int_sext1; [NOTES] %mul2 = n1 * 8 * n2
  %rel2658 = icmp slt i32 %"sub_$N3_fetch", 1
  br i1 %rel2658, label %bb30, label %bb24.preheader.lr.ph ; [NOTES] skip function if N3 < 1

bb24.preheader.lr.ph:                             ; preds = %alloca_0
  %int_sext3963 = zext i32 %"sub_$N3_fetch" to i64
  %rel1855 = icmp slt i32 %"sub_$N2_fetch", 1
  %rel53 = icmp slt i32 %"sub_$N1_fetch", 1
  %0 = add nsw i64 %int_sext, 1                   ; %0 = N1 + 1
  %1 = add nsw i64 %int_sext1, 1                  ; %1 = N2 + 1
  %2 = add nuw nsw i64 %int_sext3963, 1           ; %2 = N3 + 1
  br label %bb24.preheader

bb21:                                             ; preds = %bb21.lr.ph, %bb21
  %"var$5.054" = phi i64 [ 1, %bb21.lr.ph ], [ %add14, %bb21 ]
  %"sub_$A[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub_$A[][]", i64 %"var$5.054")
  store double 0.000000e+00, ptr %"sub_$A[][][]", align 1
  %add14 = add nuw nsw i64 %"var$5.054", 1
  %exitcond = icmp eq i64 %add14, %0
  br i1 %exitcond, label %bb22.loopexit, label %bb21

bb22.loopexit:                                    ; preds = %bb21
  br label %bb22

bb22:                                             ; preds = %bb22.loopexit, %bb20.preheader
  %add22 = add nuw nsw i64 %"var$6.056", 1
  %exitcond61 = icmp eq i64 %add22, %1
  br i1 %exitcond61, label %bb26.loopexit, label %bb20.preheader

bb20.preheader:                                   ; preds = %bb20.preheader.lr.ph, %bb22
  %"var$6.056" = phi i64 [ 1, %bb20.preheader.lr.ph ], [ %add22, %bb22 ]
  br i1 %rel53, label %bb22, label %bb21.lr.ph

bb21.lr.ph:                                       ; preds = %bb20.preheader
  %"sub_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) nonnull %"sub_$A[]", i64 %"var$6.056")
  br label %bb21

bb26.loopexit:                                    ; preds = %bb22
  br label %bb26

bb26:                                             ; preds = %bb26.loopexit, %bb24.preheader
  %add30 = add nuw nsw i64 %"var$7.059", 1
  %exitcond62 = icmp eq i64 %add30, %2
  br i1 %exitcond62, label %bb30.loopexit, label %bb24.preheader

bb24.preheader:                                   ; preds = %bb24.preheader.lr.ph, %bb26
  %"var$7.059" = phi i64 [ 1, %bb24.preheader.lr.ph ], [ %add30, %bb26 ]
  br i1 %rel1855, label %bb26, label %bb20.preheader.lr.ph

bb20.preheader.lr.ph:                             ; preds = %bb24.preheader
  %"sub_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul2, ptr elementtype(double) nonnull %"sub_$A", i64 %"var$7.059")
  br label %bb20.preheader

bb30.loopexit:                                    ; preds = %bb26
  br label %bb30

bb30:                                             ; preds = %bb30.loopexit, %alloca_0
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -hir-create-function-level-region  -disable-output < %s 2>&1 | FileCheck %s
;
; *** Source Code ***
;subroutine sub (A, B, N1,N2,N3)
;  real*8 A(N1,20,N3)
;  A = 1.0
;  end subroutine sub

; [Note]
; use -hir-details-dims  flag to show the dimensional details when debugging.



;*** IR Dump Before HIR Loop Collapse (hir-loop-collapse) ***
;Function: sub_
;
; CHECK:       BEGIN REGION { }
; CHECK:           + DO i1 = 0, zext.i32.i64(%"sub_$N3_fetch.2") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:           |   + DO i2 = 0, 19, 1   <DO_LOOP>
; CHECK:           |   |   + DO i3 = 0, sext.i32.i64(%"sub_$N1_fetch.1") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:           |   |   |   (%"sub_$A")[i1][i2][i3] = 1.000000e+00;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
;
;                  ret ;
;             END REGION

;*** IR Dump After HIR Loop Collapse (hir-loop-collapse) ***
;Function: sub_

; CHECK:      BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 20 * (zext.i32.i64(%"sub_$N3_fetch.2") * sext.i32.i64(%"sub_$N1_fetch.1")) + -1, 1   <DO_LOOP>
; CHECK:           |   (%"sub_$A")[0][0][i1] = 1.000000e+00;
; CHECK:           + END LOOP
;
;                  ret ;
; CHECK:     END REGION

; [Note]
; Below are the detailed dump (-hir-details-dims) for both before and after collapsing.
;

; *** BEFORE ***
;
;<0>          BEGIN REGION { }
;<52>               + DO i1 = 0, zext.i32.i64(%"sub_$N3_fetch.2") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;<53>               |   + DO i2 = 0, 19, 1   <DO_LOOP>
;<54>               |   |   + DO i3 = 0, sext.i32.i64(%"sub_$N1_fetch.1") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;<25>               |   |   |   (%"sub_$A")[0:i1:160 * sext.i32.i64(%"sub_$N1_fetch.1")(double*:0)][0:i2:8 * sext.i32.i64(%"sub_$N1_fetch.1")(double*:0)][0:i3:8(double*:0)] = 1.000000e+00;
;<54>               |   |   + END LOOP
;<53>               |   + END LOOP
;<52>               + END LOOP
;<52>
;<51>               ret ;
;<0>          END REGION

;*** IR Dump After HIR Loop Collapse (hir-loop-collapse) ***
;Function: sub_

;<0>          BEGIN REGION { modified }
;<54>               + DO i1 = 0, 20 * (zext.i32.i64(%"sub_$N3_fetch.2") * sext.i32.i64(%"sub_$N1_fetch.1")) + -1, 1   <DO_LOOP>
;<25>               |   (%"sub_$A")[0:0:160 * sext.i32.i64(%"sub_$N1_fetch.1")(double*:0)][0:0:8 * sext.i32.i64(%"sub_$N1_fetch.1")(double*:0)][0:i1:8(double*:0)] = 1.000000e+00;
;<54>               + END LOOP
;<54>
;<51>               ret ;
;<0>          END REGION


;Module Before HIR
; ModuleID = 'collapse2.f90'
source_filename = "collapse2.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind uwtable
define void @sub_(ptr noalias nocapture dereferenceable(8) %"sub_$A", ptr noalias nocapture readnone dereferenceable(4) %"sub_$B", ptr noalias nocapture readonly dereferenceable(4) %"sub_$N1", ptr noalias nocapture readnone dereferenceable(4) %"sub_$N2", ptr noalias nocapture readonly dereferenceable(4) %"sub_$N3") local_unnamed_addr #0 {
alloca_0:
  %"sub_$N1_fetch.1" = load i32, ptr %"sub_$N1", align 1
  %"sub_$N3_fetch.2" = load i32, ptr %"sub_$N3", align 1
  %int_sext = sext i32 %"sub_$N1_fetch.1" to i64
  %mul.1 = shl nsw i64 %int_sext, 3
  %mul.2 = mul nsw i64 %int_sext, 160
  %rel.3.not15 = icmp slt i32 %"sub_$N1_fetch.1", 1
  %rel.5.not18 = icmp slt i32 %"sub_$N3_fetch.2", 1
  br i1 %rel.5.not18, label %loop_exit17, label %loop_test11.preheader.preheader

loop_test11.preheader.preheader:                  ; preds = %alloca_0
  %int_sext522 = zext i32 %"sub_$N3_fetch.2" to i64
  %0 = add nsw i64 %int_sext, 1
  %1 = add nuw nsw i64 %int_sext522, 1
  br label %loop_test11.preheader

loop_body8:                                       ; preds = %loop_body8.preheader, %loop_body8
  %"$loop_ctr.016" = phi i64 [ %add.3, %loop_body8 ], [ 1, %loop_body8.preheader ]
  %"sub_$A[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub_$A[][]", i64 %"$loop_ctr.016")
  store double 1.000000e+00, ptr %"sub_$A[][][]", align 1
  %add.3 = add nuw nsw i64 %"$loop_ctr.016", 1
  %exitcond = icmp eq i64 %add.3, %0
  br i1 %exitcond, label %loop_exit9.loopexit, label %loop_body8

loop_exit9.loopexit:                              ; preds = %loop_body8
  br label %loop_exit9

loop_exit9:                                       ; preds = %loop_exit9.loopexit, %loop_test7.preheader
  %add.4 = add nuw nsw i64 %"$loop_ctr1.017", 1
  %exitcond20.not = icmp eq i64 %add.4, 21
  br i1 %exitcond20.not, label %loop_exit13, label %loop_test7.preheader

loop_test7.preheader:                             ; preds = %loop_test11.preheader, %loop_exit9
  %"$loop_ctr1.017" = phi i64 [ 1, %loop_test11.preheader ], [ %add.4, %loop_exit9 ]
  %"sub_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr elementtype(double) nonnull %"sub_$A[]", i64 %"$loop_ctr1.017")
  br i1 %rel.3.not15, label %loop_exit9, label %loop_body8.preheader

loop_body8.preheader:                             ; preds = %loop_test7.preheader
  br label %loop_body8

loop_exit13:                                      ; preds = %loop_exit9
  %add.5 = add nuw nsw i64 %"$loop_ctr2.019", 1
  %exitcond21 = icmp eq i64 %add.5, %1
  br i1 %exitcond21, label %loop_exit17.loopexit, label %loop_test11.preheader

loop_test11.preheader:                            ; preds = %loop_test11.preheader.preheader, %loop_exit13
  %"$loop_ctr2.019" = phi i64 [ %add.5, %loop_exit13 ], [ 1, %loop_test11.preheader.preheader ]
  %"sub_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul.2, ptr elementtype(double) nonnull %"sub_$A", i64 %"$loop_ctr2.019")
  br label %loop_test7.preheader

loop_exit17.loopexit:                             ; preds = %loop_exit13
  br label %loop_exit17

loop_exit17:                                      ; preds = %loop_exit17.loopexit, %alloca_0
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" } attributes #1 = { nofree nosync nounwind readnone speculatable }

!omp_offload.info = !{}

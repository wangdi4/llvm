; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;

;[Note]
;This LIT test illustrates a case of incorrect dynamic shape partial match in (%B)[i1][i2][i1][i2].
;The dynamic-shape matching logic recognizes the lowest 2 dimensions are good without looking at those un-matched dimensions.
;If it returns 2, it is actually the wrong result because i1 and i2 will make the highest 2 dimensions illegal to collapse.
;With the collapser fix, such scenarios are captured by examining those unmatched dimensions and return 0 as no partial
;match for dynamic-shape array is available.
;This prevents the collapser from incorrectly trigger in such cases.


;*** IR Dump Before HIR Loop Collapse (hir-loop-collapse) ***
;Function: sub_

;CHECK:      BEGIN REGION { }
;CHECK:            + DO i1 = 0, zext.i32.i64(%"(i32)var$2_fetch.2$") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;CHECK:            |   + DO i2 = 0, sext.i32.i64(%"(i32)var$1_fetch.1$") + -1, 1   <DO_LOOP>
;CHECK:            |   |   (%"sub_$A14")[i1][i2] = (%"sub_$B8")[i1][i2][i1][i2];
;CHECK:            |   + END LOOP
;CHECK:            + END LOOP
;CHECK:      END REGION

;*** IR Dump After HIR Loop Collapse (hir-loop-collapse) ***
;Function: sub_

;CHECK:      BEGIN REGION { }
;CHECK:            + DO i1 = 0, zext.i32.i64(%"(i32)var$2_fetch.2$") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;CHECK:            |   + DO i2 = 0, sext.i32.i64(%"(i32)var$1_fetch.1$") + -1, 1   <DO_LOOP>
;CHECK:            |   |   (%"sub_$A14")[i1][i2] = (%"sub_$B8")[i1][i2][i1][i2];
;CHECK:            |   + END LOOP
;CHECK:            + END LOOP
;CHECK:      END REGION


;Module Before HIR
; ModuleID = 'collapse.f90'
source_filename = "collapse.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind readonly uwtable
define void @sub_(ptr noalias nocapture dereferenceable(4) %"sub_$X", ptr noalias nocapture dereferenceable(4) %"sub_$Y") local_unnamed_addr #0 {
alloca_0:
  %"sub_$Y_fetch.7" = load float, ptr %"sub_$Y", align 1
  %"sub_$X_fetch.8" = load float, ptr %"sub_$X", align 1
  %"(i32)var$1_fetch.1$" = fptosi float %"sub_$Y_fetch.7" to i32
  %int_sext = sext i32 %"(i32)var$1_fetch.1$" to i64
  %rel.1 = icmp sgt i64 %int_sext, 0
  %slct.1 = select i1 %rel.1, i64 %int_sext, i64 0
  %mul.1 = shl nuw nsw i64 %slct.1, 2
  %"(i32)var$2_fetch.2$" = fptosi float %"sub_$X_fetch.8" to i32
  %int_sext2 = sext i32 %"(i32)var$2_fetch.2$" to i64
  %rel.2 = icmp sgt i64 %int_sext2, 0
  %slct.2 = select i1 %rel.2, i64 %int_sext2, i64 0
  %mul.2 = mul nuw nsw i64 %mul.1, %slct.2
  %mul.3 = mul nuw nsw i64 %slct.2, %slct.1
  %mul.4 = mul i64 %mul.3, %mul.2
  %div.1 = lshr exact i64 %mul.4, 2
  %"sub_$B8" = alloca float, i64 %div.1, align 4
  %div.2 = lshr exact i64 %mul.2, 2
  %"sub_$A14" = alloca float, i64 %div.2, align 4
  %mul.7 = shl nsw i64 %int_sext, 2
  %mul.8 = mul nsw i64 %mul.7, %int_sext2
  %mul.9 = mul nsw i64 %mul.8, %int_sext
  %rel.7 = icmp slt i32 %"(i32)var$2_fetch.2$", 1
  br i1 %rel.7, label %bb2, label %bb1.preheader

bb1.preheader:                                    ; preds = %alloca_0
  %rel.8 = icmp slt i32 %"(i32)var$1_fetch.1$", 1
  %0 = add nuw nsw i32 %"(i32)var$1_fetch.1$", 1
  %1 = add nuw nsw i32 %"(i32)var$2_fetch.2$", 1
  %wide.trip.count4143 = zext i32 %1 to i64
  %wide.trip.count = sext i32 %0 to i64
  br label %bb1

bb1:                                              ; preds = %bb1.preheader, %bb6
  %indvars.iv39 = phi i64 [ 1, %bb1.preheader ], [ %indvars.iv.next40, %bb6 ]
  br i1 %rel.8, label %bb6, label %bb5.preheader

bb5.preheader:                                    ; preds = %bb1
  %"sub_$B8[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul.9, ptr nonnull elementtype(float) %"sub_$B8", i64 %indvars.iv39)
  %"sub_$A14[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.7, ptr nonnull elementtype(float) %"sub_$A14", i64 %indvars.iv39)
  br label %bb5

bb5:                                              ; preds = %bb5.preheader, %bb5
  %indvars.iv = phi i64 [ 1, %bb5.preheader ], [ %indvars.iv.next, %bb5 ]
  %"sub_$B8[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul.8, ptr nonnull elementtype(float) %"sub_$B8[]", i64 %indvars.iv)
  %"sub_$B8[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.7, ptr nonnull elementtype(float) %"sub_$B8[][]", i64 %indvars.iv39)
  %"sub_$B8[][][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sub_$B8[][][]", i64 %indvars.iv)
  %"sub_$B8[][][][]_fetch.29" = load float, ptr %"sub_$B8[][][][]", align 1
  %"sub_$A14[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sub_$A14[]", i64 %indvars.iv)
  store float %"sub_$B8[][][][]_fetch.29", ptr %"sub_$A14[][]", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %bb6.loopexit, label %bb5

bb6.loopexit:                                     ; preds = %bb5
  br label %bb6

bb6:                                              ; preds = %bb6.loopexit, %bb1
  %indvars.iv.next40 = add nuw nsw i64 %indvars.iv39, 1
  %exitcond42 = icmp eq i64 %indvars.iv.next40, %wide.trip.count4143
  br i1 %exitcond42, label %bb2.loopexit, label %bb1

bb2.loopexit:                                     ; preds = %bb6
  br label %bb2

bb2:                                              ; preds = %bb2.loopexit, %alloca_0
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree nosync nounwind readonly uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nofree nosync nounwind readnone speculatable }

!omp_offload.info = !{}

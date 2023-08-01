; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; [Note]
; Loop collapse cannot trigger in this loopnest, because (%"sub_$B2")[i1][i3] is not contiguous access pattern in dim2.


;*** IR Dump Before HIR Loop Collapse (hir-loop-collapse) ***
;Function: sub_

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, sext.i32.i64(%"(i32)var$2_fetch.3$") + -1, 1   <DO_LOOP>
; CHECK:           |   |   + DO i3 = 0, sext.i32.i64(%"(i32)var$1_fetch.1$") + -1, 1   <DO_LOOP>
; CHECK:           |   |   |   %add.4 = (%"sub_$A8")[i1][i2][i3]  +  (%"sub_$B2")[i1][i3];
; CHECK:           |   |   |   (%"sub_$B2")[i1][i3] = %add.4;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION

;*** IR Dump After HIR Loop Collapse (hir-loop-collapse) ***
;Function: sub_

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, sext.i32.i64(%"(i32)var$2_fetch.3$") + -1, 1   <DO_LOOP>
; CHECK:           |   |   + DO i3 = 0, sext.i32.i64(%"(i32)var$1_fetch.1$") + -1, 1   <DO_LOOP>
; CHECK:           |   |   |   %add.4 = (%"sub_$A8")[i1][i2][i3]  +  (%"sub_$B2")[i1][i3];
; CHECK:           |   |   |   (%"sub_$B2")[i1][i3] = %add.4;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind readonly uwtable
define void @sub_(ptr noalias nocapture dereferenceable(4) %"sub_$X", ptr noalias nocapture dereferenceable(4) %"sub_$Y") local_unnamed_addr #0 {
alloca_0:
  %"sub_$X_fetch.4" = load float, ptr %"sub_$X", align 1
  %"sub_$Y_fetch.5" = load float, ptr %"sub_$Y", align 1
  %"(i32)var$1_fetch.1$" = fptosi float %"sub_$X_fetch.4" to i32
  %int_sext = sext i32 %"(i32)var$1_fetch.1$" to i64
  %rel.1 = icmp sgt i64 %int_sext, 0
  %slct.1 = select i1 %rel.1, i64 %int_sext, i64 0
  %div.1 = mul nuw nsw i64 %slct.1, 5
  %"sub_$B2" = alloca float, i64 %div.1, align 4
  %"(i32)var$2_fetch.3$" = fptosi float %"sub_$Y_fetch.5" to i32
  %int_sext6 = sext i32 %"(i32)var$2_fetch.3$" to i64
  %rel.3 = icmp sgt i64 %int_sext6, 0
  %slct.3 = select i1 %rel.3, i64 %int_sext6, i64 0
  %mul.4 = mul nuw nsw i64 %slct.1, 20
  %mul.5 = mul i64 %mul.4, %slct.3
  %div.2 = lshr exact i64 %mul.5, 2
  %"sub_$A8" = alloca float, i64 %div.2, align 4
  %mul.6 = shl nsw i64 %int_sext, 2
  %mul.7 = mul nsw i64 %mul.6, %int_sext6
  %rel.4 = icmp slt i32 %"(i32)var$2_fetch.3$", 1
  %rel.5 = icmp slt i32 %"(i32)var$1_fetch.1$", 1
  %0 = add nuw nsw i32 %"(i32)var$1_fetch.1$", 1
  %1 = add nuw nsw i32 %"(i32)var$2_fetch.3$", 1
  %wide.trip.count40 = sext i32 %1 to i64
  %wide.trip.count = sext i32 %0 to i64
  br label %bb1

bb1:                                              ; preds = %bb6, %alloca_0
  %indvars.iv42 = phi i64 [ %indvars.iv.next43, %bb6 ], [ 1, %alloca_0 ]
  br i1 %rel.4, label %bb6, label %bb5.preheader

bb5.preheader:                                    ; preds = %bb1
  %"sub_$A8[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul.7, ptr elementtype(float) nonnull %"sub_$A8", i64 %indvars.iv42)
  %"sub_$B2[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.6, ptr elementtype(float) nonnull %"sub_$B2", i64 %indvars.iv42)
  br label %bb5

bb5:                                              ; preds = %bb5.preheader, %bb10
  %indvars.iv38 = phi i64 [ 1, %bb5.preheader ], [ %indvars.iv.next39, %bb10 ]
  br i1 %rel.5, label %bb10, label %bb9.preheader

bb9.preheader:                                    ; preds = %bb5
  %"sub_$A8[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.6, ptr elementtype(float) nonnull %"sub_$A8[]", i64 %indvars.iv38)
  br label %bb9

bb9:                                              ; preds = %bb9.preheader, %bb9
  %indvars.iv = phi i64 [ 1, %bb9.preheader ], [ %indvars.iv.next, %bb9 ]
  %"sub_$A8[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"sub_$A8[][]", i64 %indvars.iv)
  %"sub_$A8[][][]_fetch.21" = load float, ptr %"sub_$A8[][][]", align 1
  %"sub_$B2[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"sub_$B2[]", i64 %indvars.iv)
  %"sub_$B2[][]_fetch.28" = load float, ptr %"sub_$B2[][]", align 1
  %add.4 = fadd reassoc ninf nsz arcp contract afn float %"sub_$A8[][][]_fetch.21", %"sub_$B2[][]_fetch.28"
  store float %add.4, ptr %"sub_$B2[][]", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %bb10.loopexit, label %bb9

bb10.loopexit:                                    ; preds = %bb9
  br label %bb10

bb10:                                             ; preds = %bb10.loopexit, %bb5
  %indvars.iv.next39 = add nuw nsw i64 %indvars.iv38, 1
  %exitcond41 = icmp eq i64 %indvars.iv.next39, %wide.trip.count40
  br i1 %exitcond41, label %bb6.loopexit, label %bb5

bb6.loopexit:                                     ; preds = %bb10
  br label %bb6

bb6:                                              ; preds = %bb6.loopexit, %bb1
  %indvars.iv.next43 = add nuw nsw i64 %indvars.iv42, 1
  %exitcond44.not = icmp eq i64 %indvars.iv.next43, 6
  br i1 %exitcond44.not, label %bb4, label %bb1

bb4:                                              ; preds = %bb6
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree nosync nounwind readonly uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nofree nosync nounwind readnone speculatable }

!omp_offload.info = !{}

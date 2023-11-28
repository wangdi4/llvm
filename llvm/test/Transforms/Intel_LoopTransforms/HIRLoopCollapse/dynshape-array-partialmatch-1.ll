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
;CHECK:            + DO i1 = 0, 99, 1   <DO_LOOP>
;CHECK:            |   + DO i2 = 0, 99, 1   <DO_LOOP>
;CHECK:            |   |   (@"sub_$A")[0][i1][i2] = (@"sub_$B")[0][i1][i2][i1][i2];
;CHECK:            |   + END LOOP
;CHECK:            + END LOOP
;CHECK:      END REGION

;*** IR Dump After HIR Loop Collapse (hir-loop-collapse) ***
;Function: sub_

;CHECK:     BEGIN REGION { }
;CHECK:            + DO i1 = 0, 99, 1   <DO_LOOP>
;CHECK:            |   + DO i2 = 0, 99, 1   <DO_LOOP>
;CHECK:            |   |   (@"sub_$A")[0][i1][i2] = (@"sub_$B")[0][i1][i2][i1][i2];
;CHECK:            |   + END LOOP
;CHECK:            + END LOOP
;CHECK:      END REGION


;Module Before HIR
; ModuleID = 'collapse2.f90'
source_filename = "collapse2.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"sub_$B" = internal unnamed_addr constant [100 x [100 x [100 x [100 x float]]]] zeroinitializer, align 16
@"sub_$A" = internal unnamed_addr global [100 x [100 x float]] zeroinitializer, align 16

; Function Attrs: nofree nosync nounwind uwtable writeonly
define void @sub_() local_unnamed_addr #0 {
alloca_0:
  br label %bb2

bb2:                                              ; preds = %bb9, %alloca_0
  %indvars.iv14 = phi i64 [ %indvars.iv.next15, %bb9 ], [ 1, %alloca_0 ]
  %"sub_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 4000000, ptr elementtype(float) @"sub_$B", i64 %indvars.iv14)
  %"sub_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 400, ptr elementtype(float) @"sub_$A", i64 %indvars.iv14)
  br label %bb6

bb6:                                              ; preds = %bb6, %bb2
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb6 ], [ 1, %bb2 ]
  %"sub_$B[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 40000, ptr elementtype(float) %"sub_$B[]", i64 %indvars.iv)
  %"sub_$B[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 400, ptr elementtype(float) %"sub_$B[][]", i64 %indvars.iv14)
  %"sub_$B[][][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %"sub_$B[][][]", i64 %indvars.iv)
  %"sub_$B[][][][]_fetch.5" = load float, ptr %"sub_$B[][][][]", align 1
  %"sub_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %"sub_$A[]", i64 %indvars.iv)
  store float %"sub_$B[][][][]_fetch.5", ptr %"sub_$A[][]", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 101
  br i1 %exitcond.not, label %bb9, label %bb6

bb9:                                              ; preds = %bb6
  %indvars.iv.next15 = add nuw nsw i64 %indvars.iv14, 1
  %exitcond16.not = icmp eq i64 %indvars.iv.next15, 101
  br i1 %exitcond16.not, label %bb5, label %bb2

bb5:                                              ; preds = %bb9
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree nosync nounwind uwtable writeonly "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nofree nosync nounwind readnone speculatable }

!omp_offload.info = !{}

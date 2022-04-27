; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-pre-vec-complete-unroll -print-after=hir-pre-vec-complete-unroll 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
; Verify that complete unroll is not triggered for perfect loop nest with deep loop depth, because
; this can be handled by loop collapsing
;
;*** IR Dump Before HIR PreVec Complete Unroll (hir-pre-vec-complete-unroll) ***
;Function: sub_
;
;<0>          BEGIN REGION { }
;<76>               + DO i1 = 0, 2, 1   <DO_LOOP>
;<77>               |   + DO i2 = 0, 2, 1   <DO_LOOP>
;<78>               |   |   + DO i3 = 0, 2, 1   <DO_LOOP>
;<79>               |   |   |   + DO i4 = 0, 2, 1   <DO_LOOP>
;<80>               |   |   |   |   + DO i5 = 0, 2, 1   <DO_LOOP>
;<81>               |   |   |   |   |   + DO i6 = 0, 2, 1   <DO_LOOP>
;<82>               |   |   |   |   |   |   + DO i7 = 0, 2, 1   <DO_LOOP>
;<27>               |   |   |   |   |   |   |   (%"sub_$RDA")[i1][i2][i3][i4][i5][i6][i7] = 1.000000e+00;
;<82>               |   |   |   |   |   |   + END LOOP
;<81>               |   |   |   |   |   + END LOOP
;<80>               |   |   |   |   + END LOOP
;<79>               |   |   |   + END LOOP
;<78>               |   |   + END LOOP
;<77>               |   + END LOOP
;<76>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR PreVec Complete Unroll (hir-pre-vec-complete-unroll) ***
;Function: sub_
;
; CHECK-NOT: modified
;
;Module Before HIR
; ModuleID = 'x.f'
source_filename = "x.f"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind writeonly uwtable
define void @sub_(float* noalias nocapture writeonly dereferenceable(4) %"sub_$RDA") local_unnamed_addr #0 {
alloca_0:
  br label %loop_test23.preheader

loop_body4:                                       ; preds = %loop_test3.preheader, %loop_body4
  %"$loop_ctr.021" = phi i64 [ 1, %loop_test3.preheader ], [ %add.1, %loop_body4 ]
  %"sub_$RDA_entry[][][][][][][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %"sub_$RDA_entry[][][][][][]", i64 %"$loop_ctr.021")
  store float 1.000000e+00, float* %"sub_$RDA_entry[][][][][][][]", align 1, !tbaa !0
  %add.1 = add nuw nsw i64 %"$loop_ctr.021", 1
  %exitcond.not = icmp eq i64 %add.1, 4
  br i1 %exitcond.not, label %loop_exit5, label %loop_body4

loop_exit5:                                       ; preds = %loop_body4
  %add.2 = add nuw nsw i64 %"$loop_ctr1.022", 1
  %exitcond28.not = icmp eq i64 %add.2, 4
  br i1 %exitcond28.not, label %loop_exit9, label %loop_test3.preheader

loop_test3.preheader:                             ; preds = %loop_test7.preheader, %loop_exit5
  %"$loop_ctr1.022" = phi i64 [ 1, %loop_test7.preheader ], [ %add.2, %loop_exit5 ]
  %"sub_$RDA_entry[][][][][][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 12, float* nonnull elementtype(float) %"sub_$RDA_entry[][][][][]", i64 %"$loop_ctr1.022")
  br label %loop_body4

loop_exit9:                                       ; preds = %loop_exit5
  %add.3 = add nuw nsw i64 %"$loop_ctr2.023", 1
  %exitcond29.not = icmp eq i64 %add.3, 4
  br i1 %exitcond29.not, label %loop_exit13, label %loop_test7.preheader

loop_test7.preheader:                             ; preds = %loop_test11.preheader, %loop_exit9
  %"$loop_ctr2.023" = phi i64 [ 1, %loop_test11.preheader ], [ %add.3, %loop_exit9 ]
  %"sub_$RDA_entry[][][][][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 36, float* nonnull elementtype(float) %"sub_$RDA_entry[][][][]", i64 %"$loop_ctr2.023")
  br label %loop_test3.preheader

loop_exit13:                                      ; preds = %loop_exit9
  %add.4 = add nuw nsw i64 %"$loop_ctr3.024", 1
  %exitcond30.not = icmp eq i64 %add.4, 4
  br i1 %exitcond30.not, label %loop_exit17, label %loop_test11.preheader

loop_test11.preheader:                            ; preds = %loop_test15.preheader, %loop_exit13
  %"$loop_ctr3.024" = phi i64 [ 1, %loop_test15.preheader ], [ %add.4, %loop_exit13 ]
  %"sub_$RDA_entry[][][][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 3, i64 1, i64 108, float* nonnull elementtype(float) %"sub_$RDA_entry[][][]", i64 %"$loop_ctr3.024")
  br label %loop_test7.preheader

loop_exit17:                                      ; preds = %loop_exit13
  %add.5 = add nuw nsw i64 %"$loop_ctr4.025", 1
  %exitcond31.not = icmp eq i64 %add.5, 4
  br i1 %exitcond31.not, label %loop_exit21, label %loop_test15.preheader

loop_test15.preheader:                            ; preds = %loop_test19.preheader, %loop_exit17
  %"$loop_ctr4.025" = phi i64 [ 1, %loop_test19.preheader ], [ %add.5, %loop_exit17 ]
  %"sub_$RDA_entry[][][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 4, i64 1, i64 324, float* nonnull elementtype(float) %"sub_$RDA_entry[][]", i64 %"$loop_ctr4.025")
  br label %loop_test11.preheader

loop_exit21:                                      ; preds = %loop_exit17
  %add.6 = add nuw nsw i64 %"$loop_ctr5.026", 1
  %exitcond32.not = icmp eq i64 %add.6, 4
  br i1 %exitcond32.not, label %loop_exit25, label %loop_test19.preheader

loop_test19.preheader:                            ; preds = %loop_test23.preheader, %loop_exit21
  %"$loop_ctr5.026" = phi i64 [ 1, %loop_test23.preheader ], [ %add.6, %loop_exit21 ]
  %"sub_$RDA_entry[][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 5, i64 1, i64 972, float* nonnull elementtype(float) %"sub_$RDA_entry[]", i64 %"$loop_ctr5.026")
  br label %loop_test15.preheader

loop_exit25:                                      ; preds = %loop_exit21
  %add.7 = add nuw nsw i64 %"$loop_ctr6.027", 1
  %exitcond33.not = icmp eq i64 %add.7, 4
  br i1 %exitcond33.not, label %loop_exit29, label %loop_test23.preheader

loop_test23.preheader:                            ; preds = %alloca_0, %loop_exit25
  %"$loop_ctr6.027" = phi i64 [ 1, %alloca_0 ], [ %add.7, %loop_exit25 ]
  %"sub_$RDA_entry[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 6, i64 1, i64 2916, float* nonnull elementtype(float) %"sub_$RDA", i64 %"$loop_ctr6.027")
  br label %loop_test19.preheader

loop_exit29:                                      ; preds = %loop_exit25
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #1

attributes #0 = { nofree nosync nounwind writeonly uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nofree nosync nounwind readnone speculatable }

!omp_offload.info = !{}

!0 = !{!1, !1, i64 0}
!1 = !{!"ifx$unique_sym$1", !2, i64 0}
!2 = !{!"Generic Fortran Symbol", !3, i64 0}
!3 = !{!"ifx$root$1$sub_"}


; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-collapse -print-after=hir-loop-collapse  -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa"  -disable-output < %s 2>&1 | FileCheck %s
; 
; Testing for max loop nest collapsing. There was a meomory overlay 
; subroutine sub(A) 
;   real  A(2,2,2,2,2,2,2,2,2,2,2,2)
;   A = 1.0
; end subroutine sub
;
;*** IR Dump Before HIR Loop Collapse (hir-loop-collapse) ***
;     BEGIN REGION { }
;      + DO i1 = 0, 1, 1   <DO_LOOP>
;      |   + DO i2 = 0, 1, 1   <DO_LOOP>
;      |   |   + DO i3 = 0, 1, 1   <DO_LOOP>
;      |   |   |   + DO i4 = 0, 1, 1   <DO_LOOP>
;      |   |   |   |   + DO i5 = 0, 1, 1   <DO_LOOP>
;      |   |   |   |   |   + DO i6 = 0, 1, 1   <DO_LOOP>
;      |   |   |   |   |   |   + DO i7 = 0, 1, 1   <DO_LOOP>
;      |   |   |   |   |   |   |   + DO i8 = 0, 1, 1   <DO_LOOP>
;      |   |   |   |   |   |   |   |   + DO i9 = 0, 1, 1   <DO_LOOP>
;      |   |   |   |   |   |   |   |   |   (%"sub_$A")[%"$loop_ctr11.047" + -1][%"$loop_ctr10.046" + -1][%"$loop_ctr9.045" + -1][i1][i2][i3][i4][i5][i6][i7][i8][i9] = 1.000000e+00;
;      |   |   |   |   |   |   |   |   + END LOOP
;      |   |   |   |   |   |   |   + END LOOP
;      |   |   |   |   |   |   + END LOOP
;      |   |   |   |   |   + END LOOP
;      |   |   |   |   + END LOOP
;      |   |   |   + END LOOP
;      |   |   + END LOOP
;      |   + END LOOP
;      + END LOOP
;     END REGION
;
;*** IR Dump After HIR Loop Collapse (hir-loop-collapse) ***

; CHECK:   BEGIN REGION 
; CHECK:     + DO i1 = 0, 511, 1   <DO_LOOP>
; CHECK:     |   (%"sub_$A")[%"$loop_ctr11.047" + -1][%"$loop_ctr10.046" + -1][%"$loop_ctr9.045" + -1][0][0][0][0][0][0][0][0][i1] = 1.000000e+00;
; CHECK:     + END LOOP
; CHECK:   END REGION

;Module Before HIR
; ModuleID = 'maxloopnest.f90'
source_filename = "maxloopnest.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind writeonly uwtable
define void @sub_(float* noalias nocapture writeonly dereferenceable(4) %"sub_$A") local_unnamed_addr #0 {
alloca_0:
  br label %loop_test43.preheader

loop_body4:                                       ; preds = %loop_test3.preheader, %loop_body4
  %"$loop_ctr.036" = phi i64 [ 1, %loop_test3.preheader ], [ %add.1, %loop_body4 ]
  %"sub_$A_entry[][][][][][][][][][][][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %"sub_$A_entry[][][][][][][][][][][]", i64 %"$loop_ctr.036")
  store float 1.000000e+00, float* %"sub_$A_entry[][][][][][][][][][][][]", align 1, !tbaa !0
  %add.1 = add nuw nsw i64 %"$loop_ctr.036", 1
  %exitcond.not = icmp eq i64 %add.1, 3
  br i1 %exitcond.not, label %loop_exit5, label %loop_body4

loop_exit5:                                       ; preds = %loop_body4
  %add.2 = add nuw nsw i64 %"$loop_ctr1.037", 1
  %exitcond48.not = icmp eq i64 %add.2, 3
  br i1 %exitcond48.not, label %loop_exit9, label %loop_test3.preheader

loop_test3.preheader:                             ; preds = %loop_test7.preheader, %loop_exit5
  %"$loop_ctr1.037" = phi i64 [ 1, %loop_test7.preheader ], [ 2, %loop_exit5 ]
  %"sub_$A_entry[][][][][][][][][][][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 8, float* elementtype(float) %"sub_$A_entry[][][][][][][][][][]", i64 %"$loop_ctr1.037")
  br label %loop_body4

loop_exit9:                                       ; preds = %loop_exit5
  %add.3 = add nuw nsw i64 %"$loop_ctr2.038", 1
  %exitcond49.not = icmp eq i64 %add.3, 3
  br i1 %exitcond49.not, label %loop_exit13, label %loop_test7.preheader

loop_test7.preheader:                             ; preds = %loop_test11.preheader, %loop_exit9
  %"$loop_ctr2.038" = phi i64 [ 1, %loop_test11.preheader ], [ 2, %loop_exit9 ]
  %"sub_$A_entry[][][][][][][][][][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 16, float* elementtype(float) %"sub_$A_entry[][][][][][][][][]", i64 %"$loop_ctr2.038")
  br label %loop_test3.preheader

loop_exit13:                                      ; preds = %loop_exit9
  %add.4 = add nuw nsw i64 %"$loop_ctr3.039", 1
  %exitcond50.not = icmp eq i64 %add.4, 3
  br i1 %exitcond50.not, label %loop_exit17, label %loop_test11.preheader

loop_test11.preheader:                            ; preds = %loop_test15.preheader, %loop_exit13
  %"$loop_ctr3.039" = phi i64 [ 1, %loop_test15.preheader ], [ 2, %loop_exit13 ]
  %"sub_$A_entry[][][][][][][][][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 3, i64 1, i64 32, float* elementtype(float) %"sub_$A_entry[][][][][][][][]", i64 %"$loop_ctr3.039")
  br label %loop_test7.preheader

loop_exit17:                                      ; preds = %loop_exit13
  %add.5 = add nuw nsw i64 %"$loop_ctr4.040", 1
  %exitcond51.not = icmp eq i64 %add.5, 3
  br i1 %exitcond51.not, label %loop_exit21, label %loop_test15.preheader

loop_test15.preheader:                            ; preds = %loop_test19.preheader, %loop_exit17
  %"$loop_ctr4.040" = phi i64 [ 1, %loop_test19.preheader ], [ 2, %loop_exit17 ]
  %"sub_$A_entry[][][][][][][][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 4, i64 1, i64 64, float* elementtype(float) %"sub_$A_entry[][][][][][][]", i64 %"$loop_ctr4.040")
  br label %loop_test11.preheader

loop_exit21:                                      ; preds = %loop_exit17
  %add.6 = add nuw nsw i64 %"$loop_ctr5.041", 1
  %exitcond52.not = icmp eq i64 %add.6, 3
  br i1 %exitcond52.not, label %loop_exit25, label %loop_test19.preheader

loop_test19.preheader:                            ; preds = %loop_test23.preheader, %loop_exit21
  %"$loop_ctr5.041" = phi i64 [ 1, %loop_test23.preheader ], [ 2, %loop_exit21 ]
  %"sub_$A_entry[][][][][][][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 5, i64 1, i64 128, float* elementtype(float) %"sub_$A_entry[][][][][][]", i64 %"$loop_ctr5.041")
  br label %loop_test15.preheader

loop_exit25:                                      ; preds = %loop_exit21
  %add.7 = add nuw nsw i64 %"$loop_ctr6.042", 1
  %exitcond53.not = icmp eq i64 %add.7, 3
  br i1 %exitcond53.not, label %loop_exit29, label %loop_test23.preheader

loop_test23.preheader:                            ; preds = %loop_test27.preheader, %loop_exit25
  %"$loop_ctr6.042" = phi i64 [ 1, %loop_test27.preheader ], [ 2, %loop_exit25 ]
  %"sub_$A_entry[][][][][][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 6, i64 1, i64 256, float* nonnull elementtype(float) %"sub_$A_entry[][][][][]", i64 %"$loop_ctr6.042")
  br label %loop_test19.preheader

loop_exit29:                                      ; preds = %loop_exit25
  %add.8 = add nuw nsw i64 %"$loop_ctr7.043", 1
  %exitcond54.not = icmp eq i64 %add.8, 3
  br i1 %exitcond54.not, label %loop_exit33, label %loop_test27.preheader

loop_test27.preheader:                            ; preds = %loop_test31.preheader, %loop_exit29
  %"$loop_ctr7.043" = phi i64 [ 1, %loop_test31.preheader ], [ 2, %loop_exit29 ]
  %"sub_$A_entry[][][][][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 7, i64 1, i64 512, float* nonnull elementtype(float) %"sub_$A_entry[][][][]", i64 %"$loop_ctr7.043")
  br label %loop_test23.preheader

loop_exit33:                                      ; preds = %loop_exit29
  %add.9 = add nuw nsw i64 %"$loop_ctr8.044", 1
  %exitcond55.not = icmp eq i64 %add.9, 3
  br i1 %exitcond55.not, label %loop_exit37, label %loop_test31.preheader

loop_test31.preheader:                            ; preds = %loop_test35.preheader, %loop_exit33
  %"$loop_ctr8.044" = phi i64 [ 1, %loop_test35.preheader ], [ 2, %loop_exit33 ]
  %"sub_$A_entry[][][][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 8, i64 1, i64 1024, float* nonnull elementtype(float) %"sub_$A_entry[][][]", i64 %"$loop_ctr8.044")
  br label %loop_test27.preheader

loop_exit37:                                      ; preds = %loop_exit33
  %add.10 = add nuw nsw i64 %"$loop_ctr9.045", 1
  %exitcond56.not = icmp eq i64 %add.10, 3
  br i1 %exitcond56.not, label %loop_exit41, label %loop_test35.preheader

loop_test35.preheader:                            ; preds = %loop_test39.preheader, %loop_exit37
  %"$loop_ctr9.045" = phi i64 [ 1, %loop_test39.preheader ], [ 2, %loop_exit37 ]
  %"sub_$A_entry[][][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 9, i64 1, i64 2048, float* nonnull elementtype(float) %"sub_$A_entry[][]", i64 %"$loop_ctr9.045")
  br label %loop_test31.preheader

loop_exit41:                                      ; preds = %loop_exit37
  %add.11 = add nuw nsw i64 %"$loop_ctr10.046", 1
  %exitcond57.not = icmp eq i64 %add.11, 3
  br i1 %exitcond57.not, label %loop_exit45, label %loop_test39.preheader

loop_test39.preheader:                            ; preds = %loop_test43.preheader, %loop_exit41
  %"$loop_ctr10.046" = phi i64 [ 1, %loop_test43.preheader ], [ 2, %loop_exit41 ]
  %"sub_$A_entry[][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 10, i64 1, i64 4096, float* nonnull elementtype(float) %"sub_$A_entry[]", i64 %"$loop_ctr10.046")
  br label %loop_test35.preheader

loop_exit45:                                      ; preds = %loop_exit41
  %add.12 = add nuw nsw i64 %"$loop_ctr11.047", 1
  %exitcond58.not = icmp eq i64 %add.12, 3
  br i1 %exitcond58.not, label %loop_exit49, label %loop_test43.preheader

loop_test43.preheader:                            ; preds = %alloca_0, %loop_exit45
  %"$loop_ctr11.047" = phi i64 [ 1, %alloca_0 ], [ 2, %loop_exit45 ]
  %"sub_$A_entry[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 11, i64 1, i64 8192, float* nonnull elementtype(float) %"sub_$A", i64 %"$loop_ctr11.047")
  br label %loop_test39.preheader

loop_exit49:                                      ; preds = %loop_exit45
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #1

attributes #0 = { nofree nosync nounwind writeonly uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" }
attributes #1 = { nofree nosync nounwind readnone speculatable }

!omp_offload.info = !{}

!0 = !{!1, !1, i64 0}
!1 = !{!"ifx$unique_sym$1", !2, i64 0}
!2 = !{!"Generic Fortran Symbol", !3, i64 0}
!3 = !{!"ifx$root$1$sub_"}

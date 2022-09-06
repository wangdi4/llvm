; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" < %s -disable-output -xmain-opt-level=3 < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate" -aa-pipeline="basic-aa" -debug-only=hir-opt-predicate < %s -disable-output -xmain-opt-level=3 < %s 2>&1 | FileCheck -check-prefix=CHECK-DBG %s

; This test case checks that the select instruction "%add.5" is converted as
; an if/else, which will be unswitched and the loop will be multiversioned.

; Original test is in Fortran, the MERGE function will produce a select
; instruction:

;  subroutine loop_merge_compute(nproma, nlev, nblk, &
;          i_startblk, i_endblk, &
;          i_startlev, i_endlev, &
;          i_startidx, i_endidx, &
;          blkidx, array)
;    use iso_fortran_env, only : real64
;    implicit none
;    integer, intent(in) :: nproma, nlev, nblk
;    integer, intent(in) :: i_startblk, i_endblk
;    integer, intent(in) :: i_startlev, i_endlev
;    integer, intent(in) :: i_startidx, i_endidx
;    integer, intent(in) :: blkidx(nlev,nblk)
;    real(kind=real64) :: array(nproma, nlev, nblk)
;    logical :: blkidx_pos
;    integer :: jb, jk, jc, jbind
;    real(kind=real64) :: add1, add2, add3
;    do jb = i_startblk, i_endblk
;      do jk = i_startlev, i_endlev
;        do jc = i_startidx, i_endidx
;          blkidx_pos = blkidx(jk,jb) > 0
;          jbind = MERGE(blkidx(jk,jb), 1, blkidx_pos)
;          add1 = MERGE(array(jc,jk,jb), real(0,real64), blkidx_pos)
;          add2 = MERGE(-array(jc,jk,jb), real(0,real64), blkidx_pos)
;          add3 = MERGE(real(1.0,real64)*array(jc,jk,jb), real(0,real64), blkidx_pos)
;          array(jc,jk,jbind) = add1 + add2 + add3
;        end do
;      end do
;    end do
; end subroutine loop_merge_compute

; HIR before transformation:

; Function: loop_merge_compute_
; BEGIN REGION { }
;       + DO i1 = 0, sext.i32.i64(%"loop_merge_compute_$I_ENDBLK_fetch.5") + -1 * sext.i32.i64(%"loop_merge_compute_$I_STARTBLK_fetch.4"), 1   <DO_LOOP>
;       |   + DO i2 = 0, sext.i32.i64(%"loop_merge_compute_$I_ENDLEV_fetch.8") + -1 * sext.i32.i64(%"loop_merge_compute_$I_STARTLEV_fetch.7"), 1   <DO_LOOP>
;       |   |      %"loop_merge_compute_$BLKIDX[][]_fetch.19" = (%"loop_merge_compute_$BLKIDX")[i1 + sext.i32.i64(%"loop_merge_compute_$I_STARTBLK_fetch.4") + -1][i2 + sext.i32.i64(%"loop_merge_compute_$I_STARTLEV_fetch.7") + -1];
;       |   |      %slct.2 = (%"loop_merge_compute_$BLKIDX[][]_fetch.19" > 0) ? %"loop_merge_compute_$BLKIDX[][]_fetch.19" : 1;
;       |   |   + DO i3 = 0, sext.i32.i64(%"loop_merge_compute_$I_ENDIDX_fetch.11") + -1 * sext.i32.i64(%"loop_merge_compute_$I_STARTIDX_fetch.10"), 1   <DO_LOOP>
;       |   |   |   %"loop_merge_compute_$ARRAY[][][]_fetch.38" = (%"loop_merge_compute_$ARRAY")[i1 + sext.i32.i64(%"loop_merge_compute_$I_STARTBLK_fetch.4")][i2 + sext.i32.i64(%"loop_merge_compute_$I_STARTLEV_fetch.7") + -1][i3 + sext.i32.i64(%"loop_merge_compute_$I_STARTIDX_fetch.10") + -1];
;       |   |   |   %add.5 = (%"loop_merge_compute_$BLKIDX[][]_fetch.19" > 0) ? %"loop_merge_compute_$ARRAY[][][]_fetch.38" : 0.000000e+00;
;       |   |   |   (%"loop_merge_compute_$ARRAY")[%slct.2][i2 + sext.i32.i64(%"loop_merge_compute_$I_STARTLEV_fetch.7") + -1][i3 + sext.i32.i64(%"loop_merge_compute_$I_STARTIDX_fetch.10") + -1] = %add.5;
;       |   |   + END LOOP
;       |   + END LOOP
;       + END LOOP
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, sext.i32.i64(%"loop_merge_compute_$I_ENDBLK_fetch.5") + -1 * sext.i32.i64(%"loop_merge_compute_$I_STARTBLK_fetch.4"), 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, sext.i32.i64(%"loop_merge_compute_$I_ENDLEV_fetch.8") + -1 * sext.i32.i64(%"loop_merge_compute_$I_STARTLEV_fetch.7"), 1   <DO_LOOP>
; CHECK:       |   |   if (%"loop_merge_compute_$I_ENDIDX_fetch.11" >= %"loop_merge_compute_$I_STARTIDX_fetch.10")
; CHECK:       |   |   {
; CHECK:       |   |      %"loop_merge_compute_$BLKIDX[][]_fetch.19" = (%"loop_merge_compute_$BLKIDX")[i1 + sext.i32.i64(%"loop_merge_compute_$I_STARTBLK_fetch.4") + -1][i2 + sext.i32.i64(%"loop_merge_compute_$I_STARTLEV_fetch.7") + -1];
; CHECK:       |   |      %slct.2 = (%"loop_merge_compute_$BLKIDX[][]_fetch.19" > 0) ? %"loop_merge_compute_$BLKIDX[][]_fetch.19" : 1;
; CHECK:       |   |      if (%"loop_merge_compute_$BLKIDX[][]_fetch.19" > 0)
; CHECK:       |   |      {
; CHECK:       |   |         + DO i3 = 0, sext.i32.i64(%"loop_merge_compute_$I_ENDIDX_fetch.11") + -1 * sext.i32.i64(%"loop_merge_compute_$I_STARTIDX_fetch.10"), 1   <DO_LOOP>
; CHECK:       |   |         |   %"loop_merge_compute_$ARRAY[][][]_fetch.38" = (%"loop_merge_compute_$ARRAY")[i1 + sext.i32.i64(%"loop_merge_compute_$I_STARTBLK_fetch.4")][i2 + sext.i32.i64(%"loop_merge_compute_$I_STARTLEV_fetch.7") + -1][i3 + sext.i32.i64(%"loop_merge_compute_$I_STARTIDX_fetch.10") + -1];
; CHECK:       |   |         |   %add.5 = %"loop_merge_compute_$ARRAY[][][]_fetch.38";
; CHECK:       |   |         |   (%"loop_merge_compute_$ARRAY")[%slct.2][i2 + sext.i32.i64(%"loop_merge_compute_$I_STARTLEV_fetch.7") + -1][i3 + sext.i32.i64(%"loop_merge_compute_$I_STARTIDX_fetch.10") + -1] = %add.5;
; CHECK:       |   |         + END LOOP
; CHECK:       |   |      }
; CHECK:       |   |      else
; CHECK:       |   |      {
; CHECK:       |   |         + DO i3 = 0, sext.i32.i64(%"loop_merge_compute_$I_ENDIDX_fetch.11") + -1 * sext.i32.i64(%"loop_merge_compute_$I_STARTIDX_fetch.10"), 1   <DO_LOOP>
; CHECK:       |   |         |   %"loop_merge_compute_$ARRAY[][][]_fetch.38" = (%"loop_merge_compute_$ARRAY")[i1 + sext.i32.i64(%"loop_merge_compute_$I_STARTBLK_fetch.4")][i2 + sext.i32.i64(%"loop_merge_compute_$I_STARTLEV_fetch.7") + -1][i3 + sext.i32.i64(%"loop_merge_compute_$I_STARTIDX_fetch.10") + -1];
; CHECK:       |   |         |   %add.5 = 0.000000e+00;
; CHECK:       |   |         |   (%"loop_merge_compute_$ARRAY")[%slct.2][i2 + sext.i32.i64(%"loop_merge_compute_$I_STARTLEV_fetch.7") + -1][i3 + sext.i32.i64(%"loop_merge_compute_$I_STARTIDX_fetch.10") + -1] = %add.5;
; CHECK:       |   |         + END LOOP
; CHECK:       |   |      }
; CHECK:       |   |   }
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION

; Check debug print

; CHECK-DBG: Opt Predicate for Function: loop_merge_compute_
; CHECK-DBG: Region: 0:
; CHECK-DBG: Candidates, count: 1
; CHECK-DBG:          %add.5 = (%"loop_merge_compute_$BLKIDX[][]_fetch.19" > 0) ? %"loop_merge_compute_$ARRAY[][][]_fetch.38" : 0.000000e+00;
; CHECK-DBG: Unswitching loop
; CHECK-DBG: Candidate for converting Select:
; CHECK-DBG:          %add.5 = (%"loop_merge_compute_$BLKIDX[][]_fetch.19" > 0) ? %"loop_merge_compute_$ARRAY[][][]_fetch.38" : 0.000000e+00;
; CHECK-DBG: Into If/Else:
; CHECK-DBG:      if (%"loop_merge_compute_$BLKIDX[][]_fetch.19" > 0)
; CHECK-DBG:      {
; CHECK-DBG:         %add.5 = %"loop_merge_compute_$ARRAY[][][]_fetch.38";
; CHECK-DBG:      }
; CHECK-DBG:      else
; CHECK-DBG:      {
; CHECK-DBG:         %add.5 = 0.000000e+00;
; CHECK-DBG:      }
; CHECK-DBG:          if (%"loop_merge_compute_$BLKIDX[][]_fetch.19" > 0), L: 2, PU: [ F/F ], S}


; ModuleID = 'm.f90'
source_filename = "m.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nofree nosync nounwind uwtable
define void @loop_merge_compute_(i32* noalias nocapture readonly dereferenceable(4) %"loop_merge_compute_$NPROMA", i32* noalias nocapture readonly dereferenceable(4) %"loop_merge_compute_$NLEV", i32* noalias nocapture readonly dereferenceable(4) %"loop_merge_compute_$NBLK", i32* noalias nocapture readonly dereferenceable(4) %"loop_merge_compute_$I_STARTBLK", i32* noalias nocapture readonly dereferenceable(4) %"loop_merge_compute_$I_ENDBLK", i32* noalias nocapture readonly dereferenceable(4) %"loop_merge_compute_$I_STARTLEV", i32* noalias nocapture readonly dereferenceable(4) %"loop_merge_compute_$I_ENDLEV", i32* noalias nocapture readonly dereferenceable(4) %"loop_merge_compute_$I_STARTIDX", i32* noalias nocapture readonly dereferenceable(4) %"loop_merge_compute_$I_ENDIDX", i32* noalias nocapture readonly dereferenceable(4) %"loop_merge_compute_$BLKIDX", double* noalias nocapture dereferenceable(8) %"loop_merge_compute_$ARRAY") local_unnamed_addr #0 !llfort.type_idx !0 {
alloca_0:
  %"loop_merge_compute_$NLEV_fetch.1" = load i32, i32* %"loop_merge_compute_$NLEV", align 1, !tbaa !1, !llfort.type_idx !6
  %"loop_merge_compute_$NPROMA_fetch.3" = load i32, i32* %"loop_merge_compute_$NPROMA", align 1, !tbaa !7, !llfort.type_idx !9
  %int_sext = sext i32 %"loop_merge_compute_$NLEV_fetch.1" to i64, !llfort.type_idx !10
  %mul.1 = shl nsw i64 %int_sext, 2
  %int_sext8 = sext i32 %"loop_merge_compute_$NPROMA_fetch.3" to i64, !llfort.type_idx !10
  %mul.3 = shl nsw i64 %int_sext8, 3
  %mul.4 = mul nsw i64 %mul.3, %int_sext
  %"loop_merge_compute_$I_STARTBLK_fetch.4" = load i32, i32* %"loop_merge_compute_$I_STARTBLK", align 1, !tbaa !11, !llfort.type_idx !13
  %"loop_merge_compute_$I_ENDBLK_fetch.5" = load i32, i32* %"loop_merge_compute_$I_ENDBLK", align 1, !tbaa !14, !llfort.type_idx !16
  %rel.1 = icmp slt i32 %"loop_merge_compute_$I_ENDBLK_fetch.5", %"loop_merge_compute_$I_STARTBLK_fetch.4"
  br i1 %rel.1, label %bb3, label %bb2.preheader

bb2.preheader:                                    ; preds = %alloca_0
  %"loop_merge_compute_$I_STARTLEV_fetch.7" = load i32, i32* %"loop_merge_compute_$I_STARTLEV", align 1, !tbaa !17, !llfort.type_idx !19
  %"loop_merge_compute_$I_ENDLEV_fetch.8" = load i32, i32* %"loop_merge_compute_$I_ENDLEV", align 1, !tbaa !20, !llfort.type_idx !22
  %rel.2 = icmp slt i32 %"loop_merge_compute_$I_ENDLEV_fetch.8", %"loop_merge_compute_$I_STARTLEV_fetch.7"
  %"loop_merge_compute_$I_STARTIDX_fetch.10" = load i32, i32* %"loop_merge_compute_$I_STARTIDX", align 1
  %"loop_merge_compute_$I_ENDIDX_fetch.11" = load i32, i32* %"loop_merge_compute_$I_ENDIDX", align 1
  %rel.3 = icmp slt i32 %"loop_merge_compute_$I_ENDIDX_fetch.11", %"loop_merge_compute_$I_STARTIDX_fetch.10"
  %0 = sext i32 %"loop_merge_compute_$I_STARTIDX_fetch.10" to i64
  %1 = add nsw i32 %"loop_merge_compute_$I_ENDIDX_fetch.11", 1
  %2 = sext i32 %"loop_merge_compute_$I_STARTLEV_fetch.7" to i64
  %3 = add nsw i32 %"loop_merge_compute_$I_ENDLEV_fetch.8", 1
  %4 = sext i32 %"loop_merge_compute_$I_STARTBLK_fetch.4" to i64
  %5 = add nsw i32 %"loop_merge_compute_$I_ENDBLK_fetch.5", 1
  %wide.trip.count78 = sext i32 %5 to i64
  %wide.trip.count74 = sext i32 %3 to i64
  %wide.trip.count = sext i32 %1 to i64
  br label %bb2

bb2:                                              ; preds = %bb2.preheader, %bb7
  %indvars.iv76 = phi i64 [ %4, %bb2.preheader ], [ %indvars.iv.next77, %bb7 ]
  br i1 %rel.2, label %bb7, label %bb6.preheader

bb6.preheader:                                    ; preds = %bb2
  %"loop_merge_compute_$BLKIDX[]" = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %mul.1, i32* nonnull elementtype(i32) %"loop_merge_compute_$BLKIDX", i64 %indvars.iv76)
  %"loop_merge_compute_$ARRAY[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %mul.4, double* nonnull elementtype(double) %"loop_merge_compute_$ARRAY", i64 %indvars.iv76)
  br label %bb6

bb6:                                              ; preds = %bb6.preheader, %bb11
  %indvars.iv72 = phi i64 [ %2, %bb6.preheader ], [ %indvars.iv.next73, %bb11 ]
  br i1 %rel.3, label %bb11, label %bb10.preheader

bb10.preheader:                                   ; preds = %bb6
  %"loop_merge_compute_$BLKIDX[][]" = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %"loop_merge_compute_$BLKIDX[]", i64 %indvars.iv72), !llfort.type_idx !23
  %"loop_merge_compute_$BLKIDX[][]_fetch.19" = load i32, i32* %"loop_merge_compute_$BLKIDX[][]", align 1, !tbaa !24, !llfort.type_idx !23
  %rel.4 = icmp sgt i32 %"loop_merge_compute_$BLKIDX[][]_fetch.19", 0
  %slct.2 = select i1 %rel.4, i32 %"loop_merge_compute_$BLKIDX[][]_fetch.19", i32 1
  %"loop_merge_compute_$ARRAY[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.3, double* nonnull elementtype(double) %"loop_merge_compute_$ARRAY[]", i64 %indvars.iv72), !llfort.type_idx !26
  %int_sext28 = zext i32 %slct.2 to i64
  %"loop_merge_compute_$ARRAY[]29" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %mul.4, double* nonnull elementtype(double) %"loop_merge_compute_$ARRAY", i64 %int_sext28), !llfort.type_idx !27
  %"loop_merge_compute_$ARRAY[][]30" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.3, double* nonnull elementtype(double) %"loop_merge_compute_$ARRAY[]29", i64 %indvars.iv72), !llfort.type_idx !28
  br label %bb10

bb10:                                             ; preds = %bb10.preheader, %bb10
  %indvars.iv = phi i64 [ %0, %bb10.preheader ], [ %indvars.iv.next, %bb10 ]
  %"loop_merge_compute_$ARRAY[][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"loop_merge_compute_$ARRAY[][]", i64 %indvars.iv), !llfort.type_idx !29
  %"loop_merge_compute_$ARRAY[][][]_fetch.38" = load double, double* %"loop_merge_compute_$ARRAY[][][]", align 1, !tbaa !30, !llfort.type_idx !29
  %add.5 = select reassoc ninf nsz arcp contract afn i1 %rel.4, double %"loop_merge_compute_$ARRAY[][][]_fetch.38", double 0.000000e+00
  %"loop_merge_compute_$ARRAY[][][]31" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"loop_merge_compute_$ARRAY[][]30", i64 %indvars.iv), !llfort.type_idx !32
  store double %add.5, double* %"loop_merge_compute_$ARRAY[][][]31", align 1, !tbaa !30
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %bb11.loopexit, label %bb10

bb11.loopexit:                                    ; preds = %bb10
  br label %bb11

bb11:                                             ; preds = %bb11.loopexit, %bb6
  %indvars.iv.next73 = add nsw i64 %indvars.iv72, 1
  %exitcond75 = icmp eq i64 %indvars.iv.next73, %wide.trip.count74
  br i1 %exitcond75, label %bb7.loopexit, label %bb6

bb7.loopexit:                                     ; preds = %bb11
  br label %bb7

bb7:                                              ; preds = %bb7.loopexit, %bb2
  %indvars.iv.next77 = add nsw i64 %indvars.iv76, 1
  %exitcond79 = icmp eq i64 %indvars.iv.next77, %wide.trip.count78
  br i1 %exitcond79, label %bb3.loopexit, label %bb2

bb3.loopexit:                                     ; preds = %bb7
  br label %bb3

bb3:                                              ; preds = %bb3.loopexit, %alloca_0
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

attributes #0 = { argmemonly nofree nosync nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nofree nosync nounwind readnone speculatable }

!omp_offload.info = !{}

!0 = !{i64 40}
!1 = !{!2, !2, i64 0}
!2 = !{!"ifx$unique_sym$1", !3, i64 0}
!3 = !{!"Fortran Data Symbol", !4, i64 0}
!4 = !{!"Generic Fortran Symbol", !5, i64 0}
!5 = !{!"ifx$root$1$loop_merge_compute_"}
!6 = !{i64 30}
!7 = !{!8, !8, i64 0}
!8 = !{!"ifx$unique_sym$3", !3, i64 0}
!9 = !{i64 29}
!10 = !{i64 3}
!11 = !{!12, !12, i64 0}
!12 = !{!"ifx$unique_sym$4", !3, i64 0}
!13 = !{i64 32}
!14 = !{!15, !15, i64 0}
!15 = !{!"ifx$unique_sym$5", !3, i64 0}
!16 = !{i64 33}
!17 = !{!18, !18, i64 0}
!18 = !{!"ifx$unique_sym$7", !3, i64 0}
!19 = !{i64 34}
!20 = !{!21, !21, i64 0}
!21 = !{!"ifx$unique_sym$8", !3, i64 0}
!22 = !{i64 35}
!23 = !{i64 51}
!24 = !{!25, !25, i64 0}
!25 = !{!"ifx$unique_sym$13", !3, i64 0}
!26 = !{i64 55}
!27 = !{i64 63}
!28 = !{i64 64}
!29 = !{i64 56}
!30 = !{!31, !31, i64 0}
!31 = !{!"ifx$unique_sym$16", !3, i64 0}
!32 = !{i64 65}
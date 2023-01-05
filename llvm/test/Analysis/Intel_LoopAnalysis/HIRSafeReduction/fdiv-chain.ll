; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-safe-reduction-analysis>" -disable-output 2>&1 | FileCheck %s

; This test case checks that safe reduction analysis was applied when there
; is a chain of divisions. It was created from the following source code:

;       subroutine sub (a,b,s,t,n,u,v)
;         real a(1000), b(1000), s, v, t
;         s = v
;         t = u
;         !dir$ vector always assert
;         do i=1, n
;           t = s / a(i)
;           s  =  t / b(i)
;         enddo
;       end subroutine sub

; HIR representation

;  BEGIN REGION { }
;        + DO i1 = 0, zext.i32.i64(%"sub_$N_fetch.3") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647> <vectorize>
;        |   %div.1 = %div.26  /  (%"sub_$A")[i1];
;        |   %div.26 = %div.1  /  (%"sub_$B")[i1];
;        + END LOOP
;  END REGION

; Safe reduction analysis print

; CHECK:    + DO i1 = 0, zext.i32.i64(%"sub_$N_fetch.3") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647> <vectorize>
; CHECK:    |   <Safe Reduction> Red Op: fdiv <Has Unsafe Algebra- No> <Conditional- No>
; CHECK:    |   %div.1 = %div.26  /  (%"sub_$A")[i1]; <Safe Reduction>
; CHECK:    |   %div.26 = %div.1  /  (%"sub_$B")[i1]; <Safe Reduction>
; CHECK:    + END LOOP


source_filename = "x6.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nofree nosync nounwind uwtable
define void @sub_(float* noalias nocapture readonly dereferenceable(4) %"sub_$A", float* noalias nocapture readonly dereferenceable(4) %"sub_$B", float* noalias nocapture dereferenceable(4) %"sub_$S", float* noalias nocapture writeonly dereferenceable(4) %"sub_$T", i32* noalias nocapture readonly dereferenceable(4) %"sub_$N", float* noalias nocapture readonly dereferenceable(4) %"sub_$U", float* noalias nocapture readonly dereferenceable(4) %"sub_$V") local_unnamed_addr #0 !llfort.type_idx !0 {
alloca_0:
  %"sub_$V_fetch.1" = load float, float* %"sub_$V", align 1, !tbaa !1, !llfort.type_idx !6
  store float %"sub_$V_fetch.1", float* %"sub_$S", align 1, !tbaa !7
  %"sub_$U_fetch.2" = load float, float* %"sub_$U", align 1, !tbaa !9, !llfort.type_idx !11
  store float %"sub_$U_fetch.2", float* %"sub_$T", align 1, !tbaa !12
  %"sub_$N_fetch.3" = load i32, i32* %"sub_$N", align 1, !tbaa !14, !llfort.type_idx !16
  %rel.1 = icmp slt i32 %"sub_$N_fetch.3", 1
  br i1 %rel.1, label %bb3, label %bb2.preheader

bb2.preheader:                                    ; preds = %alloca_0
  %0 = add nuw nsw i32 %"sub_$N_fetch.3", 1
  %wide.trip.count = zext i32 %0 to i64
  br label %bb2

bb2:                                              ; preds = %bb2.preheader, %bb2
  %indvars.iv = phi i64 [ 1, %bb2.preheader ], [ %indvars.iv.next, %bb2 ]
  %div.26 = phi float [ %"sub_$V_fetch.1", %bb2.preheader ], [ %div.2, %bb2 ]
  %"sub_$A_entry[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* nonnull elementtype(float) %"sub_$A", i64 %indvars.iv), !llfort.type_idx !17
  %"sub_$A_entry[]_fetch.7" = load float, float* %"sub_$A_entry[]", align 1, !tbaa !18, !llfort.type_idx !20
  %div.1 = fdiv reassoc ninf nsz arcp contract afn float %div.26, %"sub_$A_entry[]_fetch.7"
  %"sub_$B_entry[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* nonnull elementtype(float) %"sub_$B", i64 %indvars.iv), !llfort.type_idx !21
  %"sub_$B_entry[]_fetch.10" = load float, float* %"sub_$B_entry[]", align 1, !tbaa !22, !llfort.type_idx !24
  %div.2 = fdiv reassoc ninf nsz arcp contract afn float %div.1, %"sub_$B_entry[]_fetch.10"
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %bb3.loopexit, label %bb2, !llvm.loop !25

bb3.loopexit:                                     ; preds = %bb2
  %div.1.lcssa = phi float [ %div.1, %bb2 ]
  %div.2.lcssa = phi float [ %div.2, %bb2 ]
  store float %div.2.lcssa, float* %"sub_$S", align 1, !tbaa !7
  store float %div.1.lcssa, float* %"sub_$T", align 1, !tbaa !12
  br label %bb3

bb3:                                              ; preds = %bb3.loopexit, %alloca_0
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #1

attributes #0 = { argmemonly nofree nosync nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nofree nosync nounwind readnone speculatable }

!omp_offload.info = !{}

!0 = !{i64 44}
!1 = !{!2, !2, i64 0}
!2 = !{!"ifx$unique_sym$1", !3, i64 0}
!3 = !{!"Fortran Data Symbol", !4, i64 0}
!4 = !{!"Generic Fortran Symbol", !5, i64 0}
!5 = !{!"ifx$root$1$sub_"}
!6 = !{i64 29}
!7 = !{!8, !8, i64 0}
!8 = !{!"ifx$unique_sym$2", !3, i64 0}
!9 = !{!10, !10, i64 0}
!10 = !{!"ifx$unique_sym$3", !3, i64 0}
!11 = !{i64 28}
!12 = !{!13, !13, i64 0}
!13 = !{!"ifx$unique_sym$4", !3, i64 0}
!14 = !{!15, !15, i64 0}
!15 = !{!"ifx$unique_sym$5", !3, i64 0}
!16 = !{i64 27}
!17 = !{i64 55}
!18 = !{!19, !19, i64 0}
!19 = !{!"ifx$unique_sym$7", !3, i64 0}
!20 = !{i64 56}
!21 = !{i64 58}
!22 = !{!23, !23, i64 0}
!23 = !{!"ifx$unique_sym$8", !3, i64 0}
!24 = !{i64 59}
!25 = distinct !{!25, !26, !27, !28}
!26 = !{!"llvm.loop.vectorize.ignore_profitability"}
!27 = !{!"llvm.loop.vectorize.enable", i1 true}
!28 = !{!"llvm.loop.intel.vector.assert"}

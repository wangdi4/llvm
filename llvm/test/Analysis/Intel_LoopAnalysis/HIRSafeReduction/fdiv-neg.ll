; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-safe-reduction-analysis>" -disable-output 2>&1 | FileCheck %s

; This test case checks that the division wasn't vectorized because the
; expression is not in form of x = y / x. It was created from the following
; test case:

;       subroutine sub (a,s,n,v)
;         real a(1000), s, v
;         s = v
;         !dir$ vector always assert
;         do i=1, n
;           s  =  a(i) / s
;         enddo
;       end subroutine sub

; HIR representation

;   BEGIN REGION { }
;         + DO i1 = 0, zext.i32.i64(%"sub_$N_fetch.2") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647> <vectorize>
;         |   %div.14 = (%"sub_$A")[i1]  /  %div.14;
;         + END LOOP
;   END REGION

; Safe reduction print

; CHECK:             + DO i1 = 0, zext.i32.i64(%"sub_$N_fetch.2") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647> <vectorize>
; CHECK: No Safe Reduction
; CHECK:             + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nofree nosync nounwind uwtable
define void @sub_(float* noalias nocapture readonly dereferenceable(4) %"sub_$A", float* noalias nocapture dereferenceable(4) %"sub_$S", i32* noalias nocapture readonly dereferenceable(4) %"sub_$N", float* noalias nocapture readonly dereferenceable(4) %"sub_$V") local_unnamed_addr #0 !llfort.type_idx !0 {
alloca_0:
  %"sub_$V_fetch.1" = load float, float* %"sub_$V", align 1, !tbaa !1, !llfort.type_idx !6
  store float %"sub_$V_fetch.1", float* %"sub_$S", align 1, !tbaa !7
  %"sub_$N_fetch.2" = load i32, i32* %"sub_$N", align 1, !tbaa !9, !llfort.type_idx !11
  %rel.1 = icmp slt i32 %"sub_$N_fetch.2", 1
  br i1 %rel.1, label %bb3, label %bb2.preheader

bb2.preheader:                                    ; preds = %alloca_0
  %0 = add nuw nsw i32 %"sub_$N_fetch.2", 1
  %wide.trip.count = zext i32 %0 to i64
  br label %bb2

bb2:                                              ; preds = %bb2.preheader, %bb2
  %indvars.iv = phi i64 [ 1, %bb2.preheader ], [ %indvars.iv.next, %bb2 ]
  %div.14 = phi float [ %"sub_$V_fetch.1", %bb2.preheader ], [ %div.1, %bb2 ]
  %"sub_$A_entry[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* nonnull elementtype(float) %"sub_$A", i64 %indvars.iv), !llfort.type_idx !12
  %"sub_$A_entry[]_fetch.5" = load float, float* %"sub_$A_entry[]", align 1, !tbaa !13, !llfort.type_idx !15
  %div.1 = fdiv reassoc ninf nsz arcp contract afn float %"sub_$A_entry[]_fetch.5", %div.14
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %bb3.loopexit, label %bb2, !llvm.loop !16

bb3.loopexit:                                     ; preds = %bb2
  %div.1.lcssa = phi float [ %div.1, %bb2 ]
  store float %div.1.lcssa, float* %"sub_$S", align 1, !tbaa !7
  br label %bb3

bb3:                                              ; preds = %bb3.loopexit, %alloca_0
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #1

attributes #0 = { argmemonly nofree nosync nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nofree nosync nounwind readnone speculatable }

!omp_offload.info = !{}

!0 = !{i64 36}
!1 = !{!2, !2, i64 0}
!2 = !{!"ifx$unique_sym$1", !3, i64 0}
!3 = !{!"Fortran Data Symbol", !4, i64 0}
!4 = !{!"Generic Fortran Symbol", !5, i64 0}
!5 = !{!"ifx$root$1$sub_"}
!6 = !{i64 27}
!7 = !{!8, !8, i64 0}
!8 = !{!"ifx$unique_sym$2", !3, i64 0}
!9 = !{!10, !10, i64 0}
!10 = !{!"ifx$unique_sym$3", !3, i64 0}
!11 = !{i64 26}
!12 = !{i64 44}
!13 = !{!14, !14, i64 0}
!14 = !{!"ifx$unique_sym$5", !3, i64 0}
!15 = !{i64 45}
!16 = distinct !{!16, !17, !18, !19}
!17 = !{!"llvm.loop.vectorize.ignore_profitability"}
!18 = !{!"llvm.loop.vectorize.enable", i1 true}
!19 = !{!"llvm.loop.intel.vector.assert"}

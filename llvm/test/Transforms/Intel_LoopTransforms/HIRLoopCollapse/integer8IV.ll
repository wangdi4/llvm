
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;

;  Problem in compfail with -i8 option (all integers promoted to integer*8)
;    ifx -c -i8 -xCORE-AVX2 
;   SUBROUTINE BINOM8(IFA,N,M)
;   IMPLICIT DOUBLE PRECISION(A-H,O-Z)
;   INTEGER IFA(0:N,0:M)
;   DO  JJ=0,M
;     DO  II=0,N
;        IFA(II,JJ) = 0
;     enddo
;   enddo
;   end


;*** IR Dump before HIRLoopCollapsePass ***

; CHECK:       + DO i1 = 0, %"binom8_$M_fetch.3", 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, %"binom8_$N_fetch.1", 1   <DO_LOOP>
; CHECK:       |   |   (%"binom8_$IFA")[i1][i2] = 0;
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
     
;*** IR Dump After HIRLoopCollapsePass ***

; CHECK:       + DO i1 = 0, (1 + %"binom8_$M_fetch.3") + ((1 + %"binom8_$M_fetch.3") * %"binom8_$N_fetch.1") + -1, 1   <DO_LOOP>
; CHECK:       |   (%"binom8_$IFA")[0][i1] = 0;
; CHECK:       + END LOOP

;Module Before HIR
; ModuleID = '/tmp/ifxivoZs9.i'
source_filename = "/tmp/ifxivoZs9.i"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nofree nosync nounwind uwtable
define void @binom8_(ptr noalias nocapture writeonly dereferenceable(8) %"binom8_$IFA", ptr noalias nocapture readonly dereferenceable(8) %"binom8_$N", ptr noalias nocapture readonly dereferenceable(8) %"binom8_$M") local_unnamed_addr #0 {
alloca_0:
  %"binom8_$N_fetch.1" = load i64, ptr %"binom8_$N", align 1, !tbaa !0
  %add.1 = shl i64 %"binom8_$N_fetch.1", 3
  %mul.1 = add i64 %add.1, 8
  %"binom8_$M_fetch.3" = load i64, ptr %"binom8_$M", align 1, !tbaa !4
  %rel.1 = icmp slt i64 %"binom8_$M_fetch.3", 0
  br i1 %rel.1, label %bb3, label %bb2.preheader

bb2.preheader:                                    ; preds = %alloca_0
  %rel.2 = icmp slt i64 %"binom8_$N_fetch.1", 0
  %0 = add nsw i64 %"binom8_$N_fetch.1", 1
  %1 = add nuw nsw i64 %"binom8_$M_fetch.3", 1
  br label %bb2

bb2:                                              ; preds = %bb2.preheader, %bb7
  %"binom8_$JJ.0" = phi i64 [ %add.4, %bb7 ], [ 0, %bb2.preheader ]
  br i1 %rel.2, label %bb7, label %bb6.preheader

bb6.preheader:                                    ; preds = %bb2
  %"binom8_$IFA[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 %mul.1, ptr nonnull elementtype(i64) %"binom8_$IFA", i64 %"binom8_$JJ.0")
  br label %bb6

bb6:                                              ; preds = %bb6.preheader, %bb6
  %"binom8_$II.0" = phi i64 [ %add.3, %bb6 ], [ 0, %bb6.preheader ]
  %"binom8_$IFA[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(i64) %"binom8_$IFA[]", i64 %"binom8_$II.0")
  store i64 0, ptr %"binom8_$IFA[][]", align 1, !tbaa !6
  %add.3 = add nuw nsw i64 %"binom8_$II.0", 1
  %exitcond = icmp eq i64 %add.3, %0
  br i1 %exitcond, label %bb7.loopexit, label %bb6

bb7.loopexit:                                     ; preds = %bb6
  br label %bb7

bb7:                                              ; preds = %bb7.loopexit, %bb2
  %add.4 = add nuw nsw i64 %"binom8_$JJ.0", 1
  %exitcond8 = icmp eq i64 %add.4, %1
  br i1 %exitcond8, label %bb3.loopexit, label %bb2

bb3.loopexit:                                     ; preds = %bb7
  br label %bb3

bb3:                                              ; preds = %bb3.loopexit, %alloca_0
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { argmemonly nofree nosync nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nofree nosync nounwind readnone speculatable }

!omp_offload.info = !{}

!0 = !{!1, !1, i64 0}
!1 = !{!"ifx$unique_sym$1", !2, i64 0}
!2 = !{!"Generic Fortran Symbol", !3, i64 0}
!3 = !{!"ifx$root$1$binom8_"}
!4 = !{!5, !5, i64 0}
!5 = !{!"ifx$unique_sym$2", !2, i64 0}
!6 = !{!7, !7, i64 0}
!7 = !{!"ifx$unique_sym$5", !2, i64 0}


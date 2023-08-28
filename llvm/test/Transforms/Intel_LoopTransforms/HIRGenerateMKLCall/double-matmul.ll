; Test for generating mkl call for matrix multiplication with double and variable UB

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-interchange,hir-generate-mkl-call,hir-dead-store-elimination,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s
;
; subroutine sub(a,b,c,n) 
; real*8  a(n,n)
; real*8  b(n,n)
; real*8  c(n,n) 
; do k=1, n / 2
;  do j=1,n / 4
;     do i=1,n
;        c(i,j) = c(i,j) +  a(i,k) * b(k,j)
;     enddo
;  enddo
;  enddo
;
; HIR after HIRTempCleanup 
;
; + DO i1 = 0, sext.i32.i64(%div.1) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1073741823>  <LEGAL_MAX_TC = 1073; 
; |   + DO i2 = 0, sext.i32.i64(%div.2) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 536870911>  <LEGAL_MAX_TC = 536870911>
; |   |      %"sub_$B[][]_fetch.28" = (%"sub_$B")[i2][i1];
; |   |   + DO i3 = 0, sext.i32.i64(%"sub_$N_fetch.1") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; |   |   |   %mul.7 = %"sub_$B[][]_fetch.28"  *  (%"sub_$A")[i1][i3];
; |   |   |   %add.4 = %mul.7  +  (%"sub_$C")[i2][i3];
; |   |   |   (%"sub_$C")[i2][i3] = %add.4;
; |   |   + END LOOP
; |   + END LOOP
; + END LOOP
;
; HIR before this pass
;
; + DO i1 = 0, sext.i32.i64(%div.2) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 536870911>  <LEGAL_MAX_TC = 536870911>
; |   + DO i2 = 0, sext.i32.i64(%div.1) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1073741823>  <LEGAL_MAX_TC = 1073741823>
; |   + DO i3 = 0, sext.i32.i64(%"sub_$N_fetch.1") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>   LEGAL_MAX_TC = 2147483647>
; |   |   |   %"sub_$B[][]_fetch.28" = (%"sub_$B")[i1][i2];
; |   |   |   %mul.7 = %"sub_$B[][]_fetch.28"  *  (%"sub_$A")[i2][i3];
; |   |   |   %add.4 = %mul.7  +  (%"sub_$C")[i1][i3];
; |   |   |   (%"sub_$C")[i1][i3] = %add.4;
; |   |   + END LOOP
; |   + END LOOP
; + END LOOP
;
;
; After HIR SinkingForPerfectLoopnest, GenerateMKLCall, DeadStoreElimination
;
; CHECK:   BEGIN REGION { modified }
; CHECK:      (%.DopeVector)[0].0 = &((i8*)(%"sub_$C")[0][0]);
; CHECK:      (%.DopeVector)[0].1 = 8;
; CHECK:      (%.DopeVector)[0].2 = 0;
; CHECK:      (%.DopeVector)[0].3 = 0;
; CHECK:      (%.DopeVector)[0].4 = 2;
; CHECK:      (%.DopeVector)[0].5 = 0;
; CHECK:      (%.DopeVector)[0].6 = sext.i32.i64(%"sub_$N_fetch.1");
; CHECK:      (%.DopeVector)[0].7 = 8;
; CHECK:      (%.DopeVector)[0].8 = 1;
; CHECK:      (%.DopeVector)[0].9 = sext.i32.i64(%div.2);
; CHECK:      (%.DopeVector)[0].10 = (8 * sext.i32.i64(%"sub_$N_fetch.1"));
; CHECK:      (%.DopeVector)[0].11 = 1;
; CHECK:      (%.DopeVector3)[0].0 = &((i8*)(%"sub_$A")[0][0]);
; CHECK:      (%.DopeVector3)[0].1 = 8;
; CHECK:      (%.DopeVector3)[0].2 = 0;
; CHECK:      (%.DopeVector3)[0].3 = 0;
; CHECK:      (%.DopeVector3)[0].4 = 2;
; CHECK:      (%.DopeVector3)[0].5 = 0;
; CHECK:      (%.DopeVector3)[0].6 = sext.i32.i64(%"sub_$N_fetch.1");
; CHECK:      (%.DopeVector3)[0].7 = 8;
; CHECK:      (%.DopeVector3)[0].8 = 1;
; CHECK:      (%.DopeVector3)[0].9 = sext.i32.i64(%div.1);
; CHECK:      (%.DopeVector3)[0].10 = (8 * sext.i32.i64(%"sub_$N_fetch.1"));
; CHECK:      (%.DopeVector3)[0].11 = 1;
; CHECK:      (%.DopeVector4)[0].0 = &((i8*)(%"sub_$B")[0][0]);
; CHECK:      (%.DopeVector4)[0].1 = 8;
; CHECK:      (%.DopeVector4)[0].2 = 0;
; CHECK:      (%.DopeVector4)[0].3 = 0;
; CHECK:      (%.DopeVector4)[0].4 = 2;
; CHECK:      (%.DopeVector4)[0].5 = 0;
; CHECK:      (%.DopeVector4)[0].6 = sext.i32.i64(%div.1);
; CHECK:      (%.DopeVector4)[0].7 = 8;
; CHECK:      (%.DopeVector4)[0].8 = 1;
; CHECK:      (%.DopeVector4)[0].9 = sext.i32.i64(%div.2);
; CHECK:      (%.DopeVector4)[0].10 = (8 * sext.i32.i64(%"sub_$N_fetch.1"));
; CHECK:      (%.DopeVector4)[0].11 = 1;
; CHECK:      @matmul_mkl_f64_(&((%.DopeVector)[0]),  &((%.DopeVector3)[0]),  &((%.DopeVector4)[0]),  10,  1);
; CHECK:    END REGION

;Module Before HIR
; ModuleID = 'float-matmul.f90'
source_filename = "float-matmul.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define void @sub_(ptr noalias nocapture readonly dereferenceable(8) %"sub_$A", ptr noalias nocapture readonly dereferenceable(8) %"sub_$B", ptr noalias nocapture dereferenceable(8) %"sub_$C", ptr noalias nocapture readonly dereferenceable(4) %"sub_$N") local_unnamed_addr #0 {
alloca_0:
  %"sub_$N_fetch.1" = load i32, ptr %"sub_$N", align 1, !tbaa !0
  %int_sext = sext i32 %"sub_$N_fetch.1" to i64, !llfort.type_idx !5
  %mul.1 = shl nsw i64 %int_sext, 3
  %div.1 = sdiv i32 %"sub_$N_fetch.1", 2
  %rel.1 = icmp slt i32 %div.1, 1
  br i1 %rel.1, label %do.end_do4, label %do.body3.preheader

do.body3.preheader:                               ; preds = %alloca_0
  %div.2 = sdiv i32 %"sub_$N_fetch.1", 4
  %rel.2 = icmp slt i32 %div.2, 1
  %rel.3 = icmp slt i32 %"sub_$N_fetch.1", 1
  %0 = add nuw nsw i32 %"sub_$N_fetch.1", 1
  %1 = add nuw nsw i32 %div.2, 1
  %2 = add nuw nsw i32 %div.1, 1
  %wide.trip.count45 = zext i32 %2 to i64
  %wide.trip.count41 = sext i32 %1 to i64
  %wide.trip.count = sext i32 %0 to i64
  br label %do.body3

do.body3:                                         ; preds = %do.body3.preheader, %do.end_do8
  %indvars.iv43 = phi i64 [ 1, %do.body3.preheader ], [ %indvars.iv.next44, %do.end_do8 ]
  br i1 %rel.2, label %do.end_do8, label %do.body7.preheader

do.body7.preheader:                               ; preds = %do.body3
  %"sub_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr nonnull elementtype(double) %"sub_$A", i64 %indvars.iv43)
  br label %do.body7

do.body7:                                         ; preds = %do.body7.preheader, %do.end_do12
  %indvars.iv39 = phi i64 [ 1, %do.body7.preheader ], [ %indvars.iv.next40, %do.end_do12 ]
  br i1 %rel.3, label %do.end_do12, label %do.body11.preheader

do.body11.preheader:                              ; preds = %do.body7
  %"sub_$C[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr nonnull elementtype(double) %"sub_$C", i64 %indvars.iv39), !llfort.type_idx !6
  %"sub_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr nonnull elementtype(double) %"sub_$B", i64 %indvars.iv39), !llfort.type_idx !7
  %"sub_$B[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"sub_$B[]", i64 %indvars.iv43), !llfort.type_idx !7
  %"sub_$B[][]_fetch.28" = load double, ptr %"sub_$B[][]", align 1, !tbaa !8, !llfort.type_idx !10
  br label %do.body11

do.body11:                                        ; preds = %do.body11.preheader, %do.body11
  %indvars.iv = phi i64 [ 1, %do.body11.preheader ], [ %indvars.iv.next, %do.body11 ]
  %"sub_$C[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"sub_$C[]", i64 %indvars.iv), !llfort.type_idx !6
  %"sub_$C[][]_fetch.14" = load double, ptr %"sub_$C[][]", align 1, !tbaa !11, !llfort.type_idx !13
  %"sub_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"sub_$A[]", i64 %indvars.iv), !llfort.type_idx !14
  %"sub_$A[][]_fetch.21" = load double, ptr %"sub_$A[][]", align 1, !tbaa !15, !llfort.type_idx !17
  %mul.7 = fmul reassoc ninf nsz arcp contract afn double %"sub_$B[][]_fetch.28", %"sub_$A[][]_fetch.21"
  %add.4 = fadd reassoc ninf nsz arcp contract afn double %mul.7, %"sub_$C[][]_fetch.14"
  store double %add.4, ptr %"sub_$C[][]", align 1, !tbaa !11
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %do.end_do12.loopexit, label %do.body11

do.end_do12.loopexit:                             ; preds = %do.body11
  br label %do.end_do12

do.end_do12:                                      ; preds = %do.end_do12.loopexit, %do.body7
  %indvars.iv.next40 = add nuw nsw i64 %indvars.iv39, 1
  %exitcond42 = icmp eq i64 %indvars.iv.next40, %wide.trip.count41
  br i1 %exitcond42, label %do.end_do8.loopexit, label %do.body7

do.end_do8.loopexit:                              ; preds = %do.end_do12
  br label %do.end_do8

do.end_do8:                                       ; preds = %do.end_do8.loopexit, %do.body3
  %indvars.iv.next44 = add nuw nsw i64 %indvars.iv43, 1
  %exitcond46 = icmp eq i64 %indvars.iv.next44, %wide.trip.count45
  br i1 %exitcond46, label %do.end_do4.loopexit, label %do.body3

do.end_do4.loopexit:                              ; preds = %do.end_do8
  br label %do.end_do4

do.end_do4:                                       ; preds = %do.end_do4.loopexit, %alloca_0
  ret void
}

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cmov,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }

!omp_offload.info = !{}

!0 = !{!1, !1, i64 0}
!1 = !{!"ifx$unique_sym$1", !2, i64 0}
!2 = !{!"Fortran Data Symbol", !3, i64 0}
!3 = !{!"Generic Fortran Symbol", !4, i64 0}
!4 = !{!"ifx$root$1$sub_"}
!5 = !{i64 3}
!6 = !{i64 29}
!7 = !{i64 27}
!8 = !{!9, !9, i64 0}
!9 = !{!"ifx$unique_sym$7", !2, i64 0}
!10 = !{i64 55}
!11 = !{!12, !12, i64 0}
!12 = !{!"ifx$unique_sym$5", !2, i64 0}
!13 = !{i64 53}
!14 = !{i64 25}
!15 = !{!16, !16, i64 0}
!16 = !{!"ifx$unique_sym$6", !2, i64 0}
!17 = !{i64 54}

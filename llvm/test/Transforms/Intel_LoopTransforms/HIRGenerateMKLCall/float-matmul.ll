; Test for generating mkl call for matrix multiplication with float type and variable UB

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-interchange,hir-generate-mkl-call,hir-dead-store-elimination,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s
;
; subroutine sub(a,b,c,n) 
; real*4  a(n,n)
; real*4  b(n,n)
; real*4  c(n,n) 
; do k=1,1000
;   do j=1, n
;     do i=1,n
;       c(i,j) = c(i,j) +  a(i,k) * b(k,j)
;     enddo
;  enddo
;  enddo
; end subroutine 
;
;
; HIR before this pass 
;  + DO i1 = 0, sext.i32.i64(%div.2) + -1, 1   <DO_LOOP>  
;  |   + DO i2 = 0, sext.i32.i64(%div.1) + -1, 1   <DO_LOOP>  
;  |   |   + DO i3 = 0, sext.i32.i64(%"sub_$N_fetch.1") + -1, 1   <DO_LOOP> 
;  |   |   |   %"sub_$B[][]_fetch.28" = (%"sub_$B")[i1][i2];
;  |   |   |   %mul.7 = %"sub_$B[][]_fetch.28"  *  (%"sub_$A")[i2][i3];
;  |   |   |   %add.4 = %mul.7  +  (%"sub_$C")[i1][i3];
;  |   |   |   (%"sub_$C")[i1][i3] = %add.4;
;  |   |   + END LOOP
;  |   + END LOOP
;  + END LOOP
;
; After HIR SinkingForPerfectLoopnest, GenerateMKLCall, DeadStoreElimination
;
; CHECK:  BEGIN REGION { modified }
; CHECK:     (%.DopeVector)[0].0 = &((i8*)(%"sub_$C")[0][0]);
; CHECK:     (%.DopeVector)[0].1 = 4;
; CHECK:     (%.DopeVector)[0].2 = 0;
; CHECK:     (%.DopeVector)[0].3 = 0;
; CHECK:     (%.DopeVector)[0].4 = 2;
; CHECK:     (%.DopeVector)[0].5 = 0;
; CHECK:     (%.DopeVector)[0].6 = sext.i32.i64(%"sub_$N_fetch.1");
; CHECK:     (%.DopeVector)[0].7 = 4;
; CHECK:     (%.DopeVector)[0].8 = 1;
; CHECK:     (%.DopeVector)[0].9 = sext.i32.i64(%"sub_$N_fetch.1");
; CHECK:     (%.DopeVector)[0].10 = (4 * sext.i32.i64(%"sub_$N_fetch.1"));
; CHECK:     (%.DopeVector)[0].11 = 1;
; CHECK:     (%.DopeVector3)[0].0 = &((i8*)(%"sub_$A")[0][0]);
; CHECK:     (%.DopeVector3)[0].1 = 4;
; CHECK:     (%.DopeVector3)[0].2 = 0;
; CHECK:     (%.DopeVector3)[0].3 = 0;
; CHECK:     (%.DopeVector3)[0].4 = 2;
; CHECK:     (%.DopeVector3)[0].5 = 0;
; CHECK:     (%.DopeVector3)[0].6 = sext.i32.i64(%"sub_$N_fetch.1");
; CHECK:     (%.DopeVector3)[0].7 = 4;
; CHECK:     (%.DopeVector3)[0].8 = 1;
; CHECK:     (%.DopeVector3)[0].9 = 1000;
; CHECK:     (%.DopeVector3)[0].10 = (4 * sext.i32.i64(%"sub_$N_fetch.1"));
; CHECK:     (%.DopeVector3)[0].11 = 1;
; CHECK:     (%.DopeVector4)[0].0 = &((i8*)(%"sub_$B")[0][0]);
; CHECK:     (%.DopeVector4)[0].1 = 4;
; CHECK:     (%.DopeVector4)[0].2 = 0;
; CHECK:     (%.DopeVector4)[0].3 = 0;
; CHECK:     (%.DopeVector4)[0].4 = 2;
; CHECK:     (%.DopeVector4)[0].5 = 0;
; CHECK:     (%.DopeVector4)[0].6 = 1000;
; CHECK:     (%.DopeVector4)[0].7 = 4;
; CHECK:     (%.DopeVector4)[0].8 = 1;
; CHECK:     (%.DopeVector4)[0].9 = sext.i32.i64(%"sub_$N_fetch.1");
; CHECK:     (%.DopeVector4)[0].10 = (4 * sext.i32.i64(%"sub_$N_fetch.1"));
; CHECK:     (%.DopeVector4)[0].11 = 1;
; CHECK:     @matmul_mkl_f32_(&((%.DopeVector)[0]),  &((%.DopeVector3)[0]),  &((%.DopeVector4)[0]),  9,  1);
; CHECK:  END REGION
;
;Module Before HIR
; ModuleID = 'float-matmul.f90'
source_filename = "float-matmul.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define void @sub_(ptr noalias nocapture readonly dereferenceable(4) %"sub_$A", ptr noalias nocapture readonly dereferenceable(4) %"sub_$B", ptr noalias nocapture dereferenceable(4) %"sub_$C", ptr noalias nocapture readonly dereferenceable(4) %"sub_$N") local_unnamed_addr #0 {
alloca_0:
  %"sub_$N_fetch.1" = load i32, ptr %"sub_$N", align 1, !tbaa !0
  %int_sext = sext i32 %"sub_$N_fetch.1" to i64, !llfort.type_idx !5
  %mul.1 = shl nsw i64 %int_sext, 2
  %rel.1 = icmp slt i32 %"sub_$N_fetch.1", 1
  %0 = add nuw nsw i32 %"sub_$N_fetch.1", 1
  %wide.trip.count39 = sext i32 %0 to i64
  br label %do.body3

do.body3:                                         ; preds = %do.end_do8, %alloca_0
  %indvars.iv41 = phi i64 [ %indvars.iv.next42, %do.end_do8 ], [ 1, %alloca_0 ]
  br i1 %rel.1, label %do.end_do8, label %do.body7.preheader

do.body7.preheader:                               ; preds = %do.body3
  %"sub_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr nonnull elementtype(float) %"sub_$A", i64 %indvars.iv41), !llfort.type_idx !6
  br label %do.body7

do.body7:                                         ; preds = %do.body7.preheader, %do.end_do12
  %indvars.iv37 = phi i64 [ 1, %do.body7.preheader ], [ %indvars.iv.next38, %do.end_do12 ]
  %"sub_$C[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr nonnull elementtype(float) %"sub_$C", i64 %indvars.iv37), !llfort.type_idx !7
  %"sub_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr nonnull elementtype(float) %"sub_$B", i64 %indvars.iv37), !llfort.type_idx !8
  %"sub_$B[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sub_$B[]", i64 %indvars.iv41), !llfort.type_idx !8
  %"sub_$B[][]_fetch.26" = load float, ptr %"sub_$B[][]", align 1, !tbaa !9, !llfort.type_idx !11
  br label %do.body11

do.body11:                                        ; preds = %do.body7, %do.body11
  %indvars.iv = phi i64 [ 1, %do.body7 ], [ %indvars.iv.next, %do.body11 ]
  %"sub_$C[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sub_$C[]", i64 %indvars.iv), !llfort.type_idx !7
  %"sub_$C[][]_fetch.12" = load float, ptr %"sub_$C[][]", align 1, !tbaa !12, !llfort.type_idx !14
  %"sub_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sub_$A[]", i64 %indvars.iv), !llfort.type_idx !6
  %"sub_$A[][]_fetch.19" = load float, ptr %"sub_$A[][]", align 1, !tbaa !15, !llfort.type_idx !17
  %mul.7 = fmul reassoc ninf nsz arcp contract afn float %"sub_$B[][]_fetch.26", %"sub_$A[][]_fetch.19"
  %add.4 = fadd reassoc ninf nsz arcp contract afn float %mul.7, %"sub_$C[][]_fetch.12"
  store float %add.4, ptr %"sub_$C[][]", align 1, !tbaa !12
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count39
  br i1 %exitcond, label %do.end_do12, label %do.body11

do.end_do12:                                      ; preds = %do.body11
  %indvars.iv.next38 = add nuw nsw i64 %indvars.iv37, 1
  %exitcond40 = icmp eq i64 %indvars.iv.next38, %wide.trip.count39
  br i1 %exitcond40, label %do.end_do8.loopexit, label %do.body7

do.end_do8.loopexit:                              ; preds = %do.end_do12
  br label %do.end_do8

do.end_do8:                                       ; preds = %do.end_do8.loopexit, %do.body3
  %indvars.iv.next42 = add nuw nsw i64 %indvars.iv41, 1
  %exitcond43.not = icmp eq i64 %indvars.iv.next42, 1001
  br i1 %exitcond43.not, label %do.epilog6, label %do.body3

do.epilog6:                                       ; preds = %do.end_do8
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
!6 = !{i64 25}
!7 = !{i64 29}
!8 = !{i64 27}
!9 = !{!10, !10, i64 0}
!10 = !{!"ifx$unique_sym$7", !2, i64 0}
!11 = !{i64 54}
!12 = !{!13, !13, i64 0}
!13 = !{!"ifx$unique_sym$5", !2, i64 0}
!14 = !{i64 52}
!15 = !{!16, !16, i64 0}
!16 = !{!"ifx$unique_sym$6", !2, i64 0}
!17 = !{i64 53}

; Test for generating mkl call for float matrix vector multiply

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-interchange,hir-generate-mkl-call,hir-dead-store-elimination,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s
;
;  real*4 a(1000,1000), b(1000), c(1000)
;  
;  do i=1, 1000
;     do j=1,1000
;        c(i) = c(i) + a(j,i) * b(j) 
;     enddo
; enddo
;
;
; HIR before this pass

;  + DO i1 = 0, 999, 1   <DO_LOOP>
;  |   + DO i2 = 0, 999, 1   <DO_LOOP>
;  |   |   %add.113 = (%"matvec_$C")[i1];
;  |   |   %mul.2 = (%"matvec_$B")[i2]  *  (%"matvec_$A")[i1][i2];
;  |   |   %add.113 = %mul.2  +  %add.113;
;  |   |   (%"matvec_$C")[i1] = %add.113;
;  |   + END LOOP
;  + END LOOP
;
; After HIR SinkingForPerfectLoopnest, GenerateMKLCall, DeadStoreElimination
;
; CHECK:   BEGIN REGION { modified }
; CHECK:    (%.DopeVector)[0].0 = &((i8*)(%"matvec_$C")[0]);
; CHECK:    (%.DopeVector)[0].1 = 4;
; CHECK:    (%.DopeVector)[0].2 = 0;
; CHECK:    (%.DopeVector)[0].3 = 0;
; CHECK:    (%.DopeVector)[0].4 = 1;
; CHECK:    (%.DopeVector)[0].5 = 0;
; CHECK:    (%.DopeVector)[0].6 = 1000;
; CHECK:    (%.DopeVector)[0].7 = 4;
; CHECK:    (%.DopeVector)[0].8 = 1;
; CHECK:    (%.DopeVector2)[0].0 = &((i8*)(%"matvec_$B")[0]);
; CHECK:    (%.DopeVector2)[0].1 = 4;
; CHECK:    (%.DopeVector2)[0].2 = 0;
; CHECK:    (%.DopeVector2)[0].3 = 0;
; CHECK:    (%.DopeVector2)[0].4 = 1;
; CHECK:    (%.DopeVector2)[0].5 = 0;
; CHECK:    (%.DopeVector2)[0].6 = 1000;
; CHECK:    (%.DopeVector2)[0].7 = 4;
; CHECK:    (%.DopeVector2)[0].8 = 1;
; CHECK:    (%.DopeVector3)[0].0 = &((i8*)(%"matvec_$A")[0][0]);
; CHECK:    (%.DopeVector3)[0].1 = 4;
; CHECK:    (%.DopeVector3)[0].2 = 0;
; CHECK:    (%.DopeVector3)[0].3 = 0;
; CHECK:    (%.DopeVector3)[0].4 = 2;
; CHECK:    (%.DopeVector3)[0].5 = 0;
; CHECK:    (%.DopeVector3)[0].6 = 1000;
; CHECK:    (%.DopeVector3)[0].7 = 4;
; CHECK:    (%.DopeVector3)[0].8 = 1;
; CHECK:    (%.DopeVector3)[0].9 = 1000;
; CHECK:    (%.DopeVector3)[0].10 = 4000;
; CHECK:    (%.DopeVector3)[0].11 = 1;
; CHECK:    @matmul_mkl_f32_(&((%.DopeVector)[0]),  &((%.DopeVector2)[0]),  &((%.DopeVector3)[0]),  9,  1);
; CHECK:  END REGION

;Module Before HIR
; ModuleID = 'float-matrix-vector-mul.f90'
source_filename = "float-matrix-vector-mul.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define void @matvec_(ptr noalias nocapture readonly dereferenceable(4) %"matvec_$A", ptr noalias nocapture readonly dereferenceable(4) %"matvec_$B", ptr noalias nocapture dereferenceable(4) %"matvec_$C") local_unnamed_addr #0 {
alloca_0:
  br label %do.body2

do.body2:                                         ; preds = %do.epilog9, %alloca_0
  %indvars.iv14 = phi i64 [ %indvars.iv.next15, %do.epilog9 ], [ 1, %alloca_0 ]
  %"matvec_$C[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"matvec_$C", i64 %indvars.iv14), !llfort.type_idx !0
  %"matvec_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 4000, ptr nonnull elementtype(float) %"matvec_$A", i64 %indvars.iv14), !llfort.type_idx !0
  %"matvec_$C[].promoted" = load float, ptr %"matvec_$C[]", align 1, !tbaa !1
  br label %do.body6

do.body6:                                         ; preds = %do.body6, %do.body2
  %indvars.iv = phi i64 [ %indvars.iv.next, %do.body6 ], [ 1, %do.body2 ]
  %add.113 = phi float [ %add.1, %do.body6 ], [ %"matvec_$C[].promoted", %do.body2 ]
  %"matvec_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"matvec_$A[]", i64 %indvars.iv), !llfort.type_idx !0
  %"matvec_$A[][]_fetch.5" = load float, ptr %"matvec_$A[][]", align 1, !tbaa !6, !llfort.type_idx !8
  %"matvec_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"matvec_$B", i64 %indvars.iv), !llfort.type_idx !0
  %"matvec_$B[]_fetch.7" = load float, ptr %"matvec_$B[]", align 1, !tbaa !9, !llfort.type_idx !11
  %mul.2 = fmul reassoc ninf nsz arcp contract afn float %"matvec_$B[]_fetch.7", %"matvec_$A[][]_fetch.5"
  %add.1 = fadd reassoc ninf nsz arcp contract afn float %mul.2, %add.113
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1001
  br i1 %exitcond.not, label %do.epilog9, label %do.body6

do.epilog9:                                       ; preds = %do.body6
  %add.1.lcssa = phi float [ %add.1, %do.body6 ]
  store float %add.1.lcssa, ptr %"matvec_$C[]", align 1, !tbaa !1
  %indvars.iv.next15 = add nuw nsw i64 %indvars.iv14, 1
  %exitcond16.not = icmp eq i64 %indvars.iv.next15, 1001
  br i1 %exitcond16.not, label %do.epilog5, label %do.body2

do.epilog5:                                       ; preds = %do.epilog9
  ret void
}

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cmov,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }

!omp_offload.info = !{}

!0 = !{i64 5}
!1 = !{!2, !2, i64 0}
!2 = !{!"ifx$unique_sym$3", !3, i64 0}
!3 = !{!"Fortran Data Symbol", !4, i64 0}
!4 = !{!"Generic Fortran Symbol", !5, i64 0}
!5 = !{!"ifx$root$1$matvec_"}
!6 = !{!7, !7, i64 0}
!7 = !{!"ifx$unique_sym$4", !3, i64 0}
!8 = !{i64 43}
!9 = !{!10, !10, i64 0}
!10 = !{!"ifx$unique_sym$5", !3, i64 0}
!11 = !{i64 44}

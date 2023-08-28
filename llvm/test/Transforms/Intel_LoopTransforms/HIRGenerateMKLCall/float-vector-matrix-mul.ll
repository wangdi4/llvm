; Test for generating mkl call for float  vector matrix multiply

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-interchange,hir-generate-mkl-call,hir-dead-store-elimination,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s
;
;  real*4 a(1000,1000), b(1000), c(1000), sum
;  integer i1,i2,i3
;  DO i1 = 1,1000
;     sum = c(i1)
;     DO i2 = 1,1000
;        sum = sum + b(i2) * a(i2,i1)
;     enddo
;     c(i1) = sum
;  enddo
;
; HIR before this pass
;  + DO i1 = 0, 999, 1   <DO_LOOP>
;  |   + DO i2 = 0, 999, 1   <DO_LOOP>
;  |   |   %"vecmat_$SUM.0" = (%"vecmat_$C")[i1];
;  |   |   %mul.2 = (%"vecmat_$A")[i1][i2]  *  (%"vecmat_$B")[i2];
;  |   |   %"vecmat_$SUM.0" = %mul.2  +  %"vecmat_$SUM.0";
;  |   |   (%"vecmat_$C")[i1] = %"vecmat_$SUM.0";
;  |   + END LOOP
;  + END LOOP

; After HIR SinkingForPerfectLoopnest, GenerateMKLCall, DeadStoreElimination
;
; CHECK:  BEGIN REGION { modified }
; CHECK:   (%.DopeVector)[0].0 = &((i8*)(%"vecmat_$C")[0]);
; CHECK:   (%.DopeVector)[0].1 = 4;
; CHECK:   (%.DopeVector)[0].2 = 0;
; CHECK:   (%.DopeVector)[0].3 = 0;
; CHECK:   (%.DopeVector)[0].4 = 1;
; CHECK:   (%.DopeVector)[0].5 = 0;
; CHECK:   (%.DopeVector)[0].6 = 1000;
; CHECK:   (%.DopeVector)[0].7 = 4;
; CHECK:   (%.DopeVector)[0].8 = 1;
; CHECK:   (%.DopeVector2)[0].0 = &((i8*)(%"vecmat_$B")[0]);
; CHECK:   (%.DopeVector2)[0].1 = 4;
; CHECK:   (%.DopeVector2)[0].2 = 0;
; CHECK:   (%.DopeVector2)[0].3 = 0;
; CHECK:   (%.DopeVector2)[0].4 = 1;
; CHECK:   (%.DopeVector2)[0].5 = 0;
; CHECK:   (%.DopeVector2)[0].6 = 1000;
; CHECK:   (%.DopeVector2)[0].7 = 4;
; CHECK:   (%.DopeVector2)[0].8 = 1;
; CHECK:   (%.DopeVector3)[0].0 = &((i8*)(%"vecmat_$A")[0][0]);
; CHECK:   (%.DopeVector3)[0].1 = 4;
; CHECK:   (%.DopeVector3)[0].2 = 0;
; CHECK:   (%.DopeVector3)[0].3 = 0;
; CHECK:   (%.DopeVector3)[0].4 = 2;
; CHECK:   (%.DopeVector3)[0].5 = 0;
; CHECK:   (%.DopeVector3)[0].6 = 1000;
; CHECK:   (%.DopeVector3)[0].7 = 4;
; CHECK:   (%.DopeVector3)[0].8 = 1;
; CHECK:   (%.DopeVector3)[0].9 = 1000;
; CHECK:   (%.DopeVector3)[0].10 = 4000;
; CHECK:   (%.DopeVector3)[0].11 = 1;
; CHECK:   @matmul_mkl_f32_(&((%.DopeVector)[0]),  &((%.DopeVector2)[0]),  &((%.DopeVector3)[0]),  9,  1);
; CHECK:  END REGION

;Module Before HIR
; ModuleID = 'float-vector-matrix-mul.f90'
source_filename = "float-vector-matrix-mul.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define void @vecmat_(ptr noalias nocapture readonly dereferenceable(4) %"vecmat_$A", ptr noalias nocapture readonly dereferenceable(4) %"vecmat_$B", ptr noalias nocapture dereferenceable(4) %"vecmat_$C") local_unnamed_addr #0 {
alloca_0:
  br label %do.body2

do.body2:                                         ; preds = %do.epilog10, %alloca_0
  %indvars.iv13 = phi i64 [ %indvars.iv.next14, %do.epilog10 ], [ 1, %alloca_0 ]
  %"vecmat_$C[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"vecmat_$C", i64 %indvars.iv13), !llfort.type_idx !0
  %"vecmat_$C[]_fetch.2" = load float, ptr %"vecmat_$C[]", align 1, !tbaa !1, !llfort.type_idx !6
  %"vecmat_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 4000, ptr nonnull elementtype(float) %"vecmat_$A", i64 %indvars.iv13), !llfort.type_idx !0
  br label %do.body7

do.body7:                                         ; preds = %do.body7, %do.body2
  %indvars.iv = phi i64 [ %indvars.iv.next, %do.body7 ], [ 1, %do.body2 ]
  %"vecmat_$SUM.0" = phi float [ %add.1, %do.body7 ], [ %"vecmat_$C[]_fetch.2", %do.body2 ]
  %"vecmat_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"vecmat_$B", i64 %indvars.iv), !llfort.type_idx !0
  %"vecmat_$B[]_fetch.5" = load float, ptr %"vecmat_$B[]", align 1, !tbaa !7, !llfort.type_idx !9
  %"vecmat_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"vecmat_$A[]", i64 %indvars.iv), !llfort.type_idx !0
  %"vecmat_$A[][]_fetch.8" = load float, ptr %"vecmat_$A[][]", align 1, !tbaa !10, !llfort.type_idx !12
  %mul.2 = fmul reassoc ninf nsz arcp contract afn float %"vecmat_$A[][]_fetch.8", %"vecmat_$B[]_fetch.5"
  %add.1 = fadd reassoc ninf nsz arcp contract afn float %mul.2, %"vecmat_$SUM.0"
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1001
  br i1 %exitcond.not, label %do.epilog10, label %do.body7

do.epilog10:                                      ; preds = %do.body7
  %add.1.lcssa = phi float [ %add.1, %do.body7 ]
  store float %add.1.lcssa, ptr %"vecmat_$C[]", align 1, !tbaa !1
  %indvars.iv.next14 = add nuw nsw i64 %indvars.iv13, 1
  %exitcond15.not = icmp eq i64 %indvars.iv.next14, 1001
  br i1 %exitcond15.not, label %do.epilog5, label %do.body2

do.epilog5:                                       ; preds = %do.epilog10
  ret void
}

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cmov,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }

!omp_offload.info = !{}

!0 = !{i64 5}
!1 = !{!2, !2, i64 0}
!2 = !{!"ifx$unique_sym$2", !3, i64 0}
!3 = !{!"Fortran Data Symbol", !4, i64 0}
!4 = !{!"Generic Fortran Symbol", !5, i64 0}
!5 = !{!"ifx$root$1$vecmat_"}
!6 = !{i64 44}
!7 = !{!8, !8, i64 0}
!8 = !{!"ifx$unique_sym$5", !3, i64 0}
!9 = !{i64 45}
!10 = !{!11, !11, i64 0}
!11 = !{!"ifx$unique_sym$6", !3, i64 0}
!12 = !{i64 46}

; Test for generating mkl call for  matmul done with initialization to 0 for the result 

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-interchange,hir-generate-mkl-call,hir-dead-store-elimination,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s
;
;   subroutine matmul(a,b,c) 
;   implicit none
;   real*4 A(500,500), b(500, 500), c(500,500), sum
;   integer i1,i2,i3
;   DO i1 = 1, 500
;      DO i2=1,500
;         sum   = 0
;         DO i3 = 1, 500
;            sum = sum + b(i2,i3) *  A(i3,i1)
;         enddo
;         c(i2,i1) = sum
;      enddo
;   enddo
; end subroutine matmul
;
; MKL library has an argument to indidate if the result needs to be initialized as zero.
; But when compiling from source, sinking, distribution have happened and there is not a case 
; that the library needs to initialize the result (C array)
; 
;   TODO: The input matmul comes in as a different patten, with 2 memrefs in the multiply statement
;   New code is needed to handle this


; XFAIL: * 

; HIR before this pass
;
; After HIR SinkingForPerfectLoopnest, GenerateMKLCall, DeadStoreElimination
;
;  
;    BEGIN REGION { modified }
;    + DO i1 = 0, 499, 1   <DO_LOOP>
;    |   + DO i2 = 0, 499, 1   <DO_LOOP>
;    |   |   (%"matmul_$C")[i1][i2] = 0.000000e+00;
;    |   + END LOOP
;    + END LOOP
;    + DO i1 = 0, 499, 1   <DO_LOOP>
;    |   + DO i2 = 0, 499, 1   <DO_LOOP>
;    |   |   + DO i3 = 0, 499, 1   <DO_LOOP>
;    |   |   |   %"matmul_$SUM.0" = (%"matmul_$C")[i1][i3];
;    |   |   |   %mul.3 = (%"matmul_$A")[i1][i2]  *  (%"matmul_$B")[i2][i3];
;    |   |   |   %"matmul_$SUM.0" = %mul.3  +  %"matmul_$SUM.0";
;    |   |   |   (%"matmul_$C")[i1][i3] = %"matmul_$SUM.0";
;    |   |   + END LOOP
;    |   + END LOOP
;    + END LOOP
;    END REGION

;Module Before HIR
; ModuleID = 'float-matmul-with-reset.f90'
source_filename = "float-matmul-with-reset.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define void @matmul_(ptr noalias nocapture readonly dereferenceable(4) %"matmul_$A", ptr noalias nocapture readonly dereferenceable(4) %"matmul_$B", ptr noalias nocapture writeonly dereferenceable(4) %"matmul_$C") local_unnamed_addr #0 {
alloca_0:
  br label %do.body2

do.body2:                                         ; preds = %do.epilog9, %alloca_0
  %indvars.iv18 = phi i64 [ %indvars.iv.next19, %do.epilog9 ], [ 1, %alloca_0 ]
  %"matmul_$C[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 2000, ptr nonnull elementtype(float) %"matmul_$C", i64 %indvars.iv18)
  %"matmul_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 2000, ptr nonnull elementtype(float) %"matmul_$A", i64 %indvars.iv18), !llfort.type_idx !0
  br label %do.body6

do.body6:                                         ; preds = %do.epilog14, %do.body2
  %indvars.iv15 = phi i64 [ %indvars.iv.next16, %do.epilog14 ], [ 1, %do.body2 ]
  br label %do.body11

do.body11:                                        ; preds = %do.body11, %do.body6
  %indvars.iv = phi i64 [ %indvars.iv.next, %do.body11 ], [ 1, %do.body6 ]
  %"matmul_$SUM.0" = phi float [ %add.1, %do.body11 ], [ 0.000000e+00, %do.body6 ]
  %"matmul_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 2000, ptr nonnull elementtype(float) %"matmul_$B", i64 %indvars.iv), !llfort.type_idx !0
  %"matmul_$B[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"matmul_$B[]", i64 %indvars.iv15), !llfort.type_idx !0
  %"matmul_$B[][]_fetch.4" = load float, ptr %"matmul_$B[][]", align 1, !tbaa !1, !llfort.type_idx !6
  %"matmul_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"matmul_$A[]", i64 %indvars.iv), !llfort.type_idx !0
  %"matmul_$A[][]_fetch.7" = load float, ptr %"matmul_$A[][]", align 1, !tbaa !7, !llfort.type_idx !9
  %mul.3 = fmul reassoc ninf nsz arcp contract afn float %"matmul_$A[][]_fetch.7", %"matmul_$B[][]_fetch.4"
  %add.1 = fadd reassoc ninf nsz arcp contract afn float %mul.3, %"matmul_$SUM.0"
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 501
  br i1 %exitcond.not, label %do.epilog14, label %do.body11

do.epilog14:                                      ; preds = %do.body11
  %add.1.lcssa = phi float [ %add.1, %do.body11 ]
  %"matmul_$C[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"matmul_$C[]", i64 %indvars.iv15), !llfort.type_idx !0
  store float %add.1.lcssa, ptr %"matmul_$C[][]", align 1, !tbaa !10
  %indvars.iv.next16 = add nuw nsw i64 %indvars.iv15, 1
  %exitcond17.not = icmp eq i64 %indvars.iv.next16, 501
  br i1 %exitcond17.not, label %do.epilog9, label %do.body6

do.epilog9:                                       ; preds = %do.epilog14
  %indvars.iv.next19 = add nuw nsw i64 %indvars.iv18, 1
  %exitcond20.not = icmp eq i64 %indvars.iv.next19, 501
  br i1 %exitcond20.not, label %do.epilog5, label %do.body2

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
!2 = !{!"ifx$unique_sym$5", !3, i64 0}
!3 = !{!"Fortran Data Symbol", !4, i64 0}
!4 = !{!"Generic Fortran Symbol", !5, i64 0}
!5 = !{!"ifx$root$1$matmul_"}
!6 = !{i64 43}
!7 = !{!8, !8, i64 0}
!8 = !{!"ifx$unique_sym$6", !3, i64 0}
!9 = !{i64 44}
!10 = !{!11, !11, i64 0}
!11 = !{!"ifx$unique_sym$7", !3, i64 0}


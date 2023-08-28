; Test for generating mkl call for matrix multiplication with complex data type

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-interchange,hir-generate-mkl-call,hir-dead-store-elimination,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s
;

; Fortran complex matmul source code-
; program main
; complex*8 a3(50,50),b3(50,50), c3(50,50)
; n = 200
; do j=1,n
;    do k = 1,n
;       do i = 1,n
;          c3(i,j) = c3(i,j) + a3(i,k) * b3(k,j)
;       enddo
;    enddo
; enddo
; end

; HIR after HIRTempCleanup 

;     BEGIN REGION { }
;        + DO i1 = 0, 199, 1   <DO_LOOP>
;        |   + DO i2 = 0, 199, 1   <DO_LOOP>
;        |   |   %fetch.11 = (%"cmatmul_$B3")[i1][i2].0;
;        |   |   %fetch.12 = (%"cmatmul_$B3")[i1][i2].1;
;        |   |   
;        |   |   + DO i3 = 0, 199, 1   <DO_LOOP>
;        |   |   |   %fetch.7 = (%"cmatmul_$A3")[i2][i3].0;
;        |   |   |   %fetch.8 = (%"cmatmul_$A3")[i2][i3].1;
;        |   |   |   %mul.4 = %fetch.11  *  %fetch.7;
;        |   |   |   %mul.6 = %fetch.12  *  %fetch.7;
;        |   |   |   %mul.7 = %fetch.11  *  %fetch.8;
;        |   |   |   %sub.1 = %mul.4  +  (%"cmatmul_$C3")[i1][i3].0;
;        |   |   |   %6 = %fetch.12  *  %fetch.8;
;        |   |   |   %add.2 = %sub.1  -  %6;
;        |   |   |   %add.1 = %mul.7  +  (%"cmatmul_$C3")[i1][i3].1;
;        |   |   |   %add.3 = %add.1  +  %mul.6;
;        |   |   |   (%"cmatmul_$C3")[i1][i3].0 = %add.2;
;        |   |   |   (%"cmatmul_$C3")[i1][i3].1 = %add.3;
;        |   |   + END LOOP
;        |   + END LOOP
;        + END LOOP
;        END REGION


; After HIR SinkingForPerfectLoopnest, GenerateMKLCall, DeadStoreElimination
;
; CHECK:      BEGIN REGION { modified }
; CHECK:           (%.DopeVector)[0].0 = &((i8*)(%"cmatmul_$C3")[0][0].0);
; CHECK:           (%.DopeVector)[0].1 = 8;
; CHECK:           (%.DopeVector)[0].2 = 0;
; CHECK:           (%.DopeVector)[0].3 = 0;
; CHECK:           (%.DopeVector)[0].4 = 2;
; CHECK:           (%.DopeVector)[0].5 = 0;
; CHECK:           (%.DopeVector)[0].6 = 200;
; CHECK:           (%.DopeVector)[0].7 = 8;
; CHECK:           (%.DopeVector)[0].8 = 1;
; CHECK:           (%.DopeVector)[0].9 = 200;
; CHECK:           (%.DopeVector)[0].10 = 1600;
; CHECK:           (%.DopeVector)[0].11 = 1;
; CHECK:           (%.DopeVector3)[0].0 = &((i8*)(%"cmatmul_$A3")[0][0].0);
; CHECK:           (%.DopeVector3)[0].1 = 8;
; CHECK:           (%.DopeVector3)[0].2 = 0;
; CHECK:           (%.DopeVector3)[0].3 = 0;
; CHECK:           (%.DopeVector3)[0].4 = 2;
; CHECK:           (%.DopeVector3)[0].5 = 0;
; CHECK:           (%.DopeVector3)[0].6 = 200;
; CHECK:           (%.DopeVector3)[0].7 = 8;
; CHECK:           (%.DopeVector3)[0].8 = 1;
; CHECK:           (%.DopeVector3)[0].9 = 200;
; CHECK:           (%.DopeVector3)[0].10 = 1600;
; CHECK:           (%.DopeVector3)[0].11 = 1;
; CHECK:           (%.DopeVector4)[0].0 = &((i8*)(%"cmatmul_$B3")[0][0].0);
; CHECK:           (%.DopeVector4)[0].1 = 8;
; CHECK:           (%.DopeVector4)[0].2 = 0;
; CHECK:           (%.DopeVector4)[0].3 = 0;
; CHECK:           (%.DopeVector4)[0].4 = 2;
; CHECK:           (%.DopeVector4)[0].5 = 0;
; CHECK:           (%.DopeVector4)[0].6 = 200;
; CHECK:           (%.DopeVector4)[0].7 = 8;
; CHECK:           (%.DopeVector4)[0].8 = 1;
; CHECK:           (%.DopeVector4)[0].9 = 200;
; CHECK:           (%.DopeVector4)[0].10 = 1600;
; CHECK:           (%.DopeVector4)[0].11 = 1;
; CHECK:           @matmul_mkl_c64_(&((%.DopeVector)[0]),  &((%.DopeVector3)[0]),  &((%.DopeVector4)[0]),  28,  1);
; CHECK:      END REGION


;Module Before HIR
; ModuleID = 'complex.f90'
source_filename = "complex.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%complex_64bit = type { float, float }

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define void @cmatmul_(ptr noalias nocapture readonly dereferenceable(8) %"cmatmul_$A3", ptr noalias nocapture readonly dereferenceable(8) %"cmatmul_$B3", ptr noalias nocapture dereferenceable(8) %"cmatmul_$C3") local_unnamed_addr #0 {
alloca_0:
  br label %do.body2

do.body2:                                         ; preds = %do.epilog9, %alloca_0
  %indvars.iv41 = phi i64 [ %indvars.iv.next42, %do.epilog9 ], [ 1, %alloca_0 ]
  %"cmatmul_$C3[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 1600, ptr nonnull elementtype(%complex_64bit) %"cmatmul_$C3", i64 %indvars.iv41), !llfort.type_idx !0
  %"cmatmul_$B3[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 1600, ptr nonnull elementtype(%complex_64bit) %"cmatmul_$B3", i64 %indvars.iv41), !llfort.type_idx !0
  br label %do.body6

do.body6:                                         ; preds = %do.epilog13, %do.body2
  %indvars.iv38 = phi i64 [ %indvars.iv.next39, %do.epilog13 ], [ 1, %do.body2 ]
  %"cmatmul_$A3[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 1600, ptr nonnull elementtype(%complex_64bit) %"cmatmul_$A3", i64 %indvars.iv38), !llfort.type_idx !0
  %"cmatmul_$B3[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(%complex_64bit) %"cmatmul_$B3[]", i64 %indvars.iv38), !llfort.type_idx !0
  %0 = getelementptr %complex_64bit, ptr %"cmatmul_$B3[][]", i64 0, i32 0, !llfort.type_idx !1
  %1 = getelementptr %complex_64bit, ptr %"cmatmul_$B3[][]", i64 0, i32 1, !llfort.type_idx !2
  %fetch.11 = load float, ptr %0, align 1, !tbaa !3, !llfort.type_idx !1
  %fetch.12 = load float, ptr %1, align 1, !tbaa !8, !llfort.type_idx !2
  br label %do.body10

do.body10:                                        ; preds = %do.body10, %do.body6
  %indvars.iv = phi i64 [ %indvars.iv.next, %do.body10 ], [ 1, %do.body6 ]
  %"cmatmul_$C3[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(%complex_64bit) %"cmatmul_$C3[]", i64 %indvars.iv), !llfort.type_idx !0
  %2 = getelementptr %complex_64bit, ptr %"cmatmul_$C3[][]", i64 0, i32 0, !llfort.type_idx !10
  %3 = getelementptr %complex_64bit, ptr %"cmatmul_$C3[][]", i64 0, i32 1, !llfort.type_idx !11
  %fetch.3 = load float, ptr %2, align 1, !tbaa !12, !llfort.type_idx !10
  %fetch.4 = load float, ptr %3, align 1, !tbaa !14, !llfort.type_idx !11
  %"cmatmul_$A3[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(%complex_64bit) %"cmatmul_$A3[]", i64 %indvars.iv), !llfort.type_idx !0
  %4 = getelementptr %complex_64bit, ptr %"cmatmul_$A3[][]", i64 0, i32 0, !llfort.type_idx !16
  %5 = getelementptr %complex_64bit, ptr %"cmatmul_$A3[][]", i64 0, i32 1, !llfort.type_idx !17
  %fetch.7 = load float, ptr %4, align 1, !tbaa !18, !llfort.type_idx !16
  %fetch.8 = load float, ptr %5, align 1, !tbaa !20, !llfort.type_idx !17
  %mul.4 = fmul reassoc ninf nsz arcp contract afn float %fetch.11, %fetch.7
  %mul.6 = fmul reassoc ninf nsz arcp contract afn float %fetch.12, %fetch.7
  %mul.7 = fmul reassoc ninf nsz arcp contract afn float %fetch.11, %fetch.8
  %sub.1 = fadd reassoc ninf nsz arcp contract afn float %mul.4, %fetch.3
  %6 = fmul reassoc ninf nsz arcp contract afn float %fetch.12, %fetch.8
  %add.2 = fsub reassoc ninf nsz arcp contract afn float %sub.1, %6
  %add.1 = fadd reassoc ninf nsz arcp contract afn float %mul.7, %fetch.4
  %add.3 = fadd reassoc ninf nsz arcp contract afn float %add.1, %mul.6
  store float %add.2, ptr %2, align 1, !tbaa !12
  store float %add.3, ptr %3, align 1, !tbaa !14
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 201
  br i1 %exitcond.not, label %do.epilog13, label %do.body10

do.epilog13:                                      ; preds = %do.body10
  %indvars.iv.next39 = add nuw nsw i64 %indvars.iv38, 1
  %exitcond40.not = icmp eq i64 %indvars.iv.next39, 201
  br i1 %exitcond40.not, label %do.epilog9, label %do.body6

do.epilog9:                                       ; preds = %do.epilog13
  %indvars.iv.next42 = add nuw nsw i64 %indvars.iv41, 1
  %exitcond43.not = icmp eq i64 %indvars.iv.next42, 201
  br i1 %exitcond43.not, label %do.epilog5, label %do.body2

do.epilog5:                                       ; preds = %do.epilog9
  ret void
}

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }

!omp_offload.info = !{}

!0 = !{i64 8}
!1 = !{i64 53}
!2 = !{i64 54}
!3 = !{!4, !4, i64 0}
!4 = !{!"ifx$unique_sym$8", !5, i64 0}
!5 = !{!"Fortran Data Symbol", !6, i64 0}
!6 = !{!"Generic Fortran Symbol", !7, i64 0}
!7 = !{!"ifx$root$1$cmatmul_"}
!8 = !{!9, !9, i64 0}
!9 = !{!"ifx$unique_sym$9", !5, i64 0}
!10 = !{i64 43}
!11 = !{i64 44}
!12 = !{!13, !13, i64 0}
!13 = !{!"ifx$unique_sym$4", !5, i64 0}
!14 = !{!15, !15, i64 0}
!15 = !{!"ifx$unique_sym$5", !5, i64 0}
!16 = !{i64 48}
!17 = !{i64 49}
!18 = !{!19, !19, i64 0}
!19 = !{!"ifx$unique_sym$6", !5, i64 0}
!20 = !{!21, !21, i64 0}
!21 = !{!"ifx$unique_sym$7", !5, i64 0}


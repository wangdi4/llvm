; Test for generating mkl call for double  matrix vector multiply

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-generate-mkl-call,hir-dead-store-elimination,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s
;
; double c[1000], a[1000][1000], b[1000];
; void  matrixvector( ) { 
;  int l,i,j;
;  for (int l = 0; l < 100; l++) {
;    for (int i = 0; i < 1000; i++) {
;      for (int j = 0; j < 1000; j++) {
;  	c[i] += a[i][j] * b[j];
;      }
;    }
;  }
; }
;
; HIR before this pass
;
;   + DO i1 = 0, 99, 1   <DO_LOOP>
;   |   + DO i2 = 0, 999, 1   <DO_LOOP>
;   |   |   + DO i3 = 0, 999, 1   <DO_LOOP>
;   |   |   |   %add3132 = (@c)[0][i2];
;   |   |   |   %mul = (@b)[0][i3]  *  (@a)[0][i2][i3];
;   |   |   |   %add3132 = %add3132  +  %mul;
;   |   |   |   (@c)[0][i2] = %add3132;
;   |   |   + END LOOP
;   |   + END LOOP
;   + END LOOP

; After HIR SinkingForPerfectLoopnest, GenerateMKLCall, DeadStoreElimination
;

; CHECK:   BEGIN REGION { modified }
; CHECK:     + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:     |   (%.DopeVector)[0].0 = &((i8*)(@c)[0][0]);
; CHECK:     |   (%.DopeVector)[0].1 = 8;
; CHECK:     |   (%.DopeVector)[0].2 = 0;
; CHECK:     |   (%.DopeVector)[0].3 = 0;
; CHECK:     |   (%.DopeVector)[0].4 = 1;
; CHECK:     |   (%.DopeVector)[0].5 = 0;
; CHECK:     |   (%.DopeVector)[0].6 = 1000;
; CHECK:     |   (%.DopeVector)[0].7 = 8;
; CHECK:     |   (%.DopeVector)[0].8 = 1;
; CHECK:     |   (%.DopeVector3)[0].0 = &((i8*)(@b)[0][0]);
; CHECK:     |   (%.DopeVector3)[0].1 = 8;
; CHECK:     |   (%.DopeVector3)[0].2 = 0;
; CHECK:     |   (%.DopeVector3)[0].3 = 0;
; CHECK:     |   (%.DopeVector3)[0].4 = 1;
; CHECK:     |   (%.DopeVector3)[0].5 = 0;
; CHECK:     |   (%.DopeVector3)[0].6 = 1000;
; CHECK:     |   (%.DopeVector3)[0].7 = 8;
; CHECK:     |   (%.DopeVector3)[0].8 = 1;
; CHECK:     |   (%.DopeVector4)[0].0 = &((i8*)(@a)[0][0][0]);
; CHECK:     |   (%.DopeVector4)[0].1 = 8;
; CHECK:     |   (%.DopeVector4)[0].2 = 0;
; CHECK:     |   (%.DopeVector4)[0].3 = 0;
; CHECK:     |   (%.DopeVector4)[0].4 = 2;
; CHECK:     |   (%.DopeVector4)[0].5 = 0;
; CHECK:     |   (%.DopeVector4)[0].6 = 1000;
; CHECK:     |   (%.DopeVector4)[0].7 = 8;
; CHECK:     |   (%.DopeVector4)[0].8 = 1;
; CHECK:     |   (%.DopeVector4)[0].9 = 1000;
; CHECK:     |   (%.DopeVector4)[0].10 = 8000;
; CHECK:     |   (%.DopeVector4)[0].11 = 1;
; CHECK:     |   @matmul_mkl_f64_(&((%.DopeVector)[0]),  &((%.DopeVector3)[0]),  &((%.DopeVector4)[0]),  10,  1);
; CHECK:     + END LOOP
; CHECK:    END REGION

;Module Before HIR
; ModuleID = 'double-matrix-vector-mul.c'
source_filename = "double-matrix-vector-mul.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [1000 x [1000 x double]] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [1000 x double] zeroinitializer, align 16
@c = dso_local local_unnamed_addr global [1000 x double] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local void @matrixvector() local_unnamed_addr #0 {
entry:
  br label %for.cond3.preheader

for.cond3.preheader:                              ; preds = %entry, %for.cond.cleanup5
  %l1.035 = phi i32 [ 0, %entry ], [ %inc22, %for.cond.cleanup5 ]
  br label %for.cond8.preheader

for.cond.cleanup:                                 ; preds = %for.cond.cleanup5
  ret void

for.cond8.preheader:                              ; preds = %for.cond3.preheader, %for.cond.cleanup10
  %indvars.iv36 = phi i64 [ 0, %for.cond3.preheader ], [ %indvars.iv.next37, %for.cond.cleanup10 ]
  %arrayidx17 = getelementptr inbounds [1000 x double], ptr @c, i64 0, i64 %indvars.iv36, !intel-tbaa !3
  %arrayidx17.promoted = load double, ptr %arrayidx17, align 8, !tbaa !3
  br label %for.body11

for.cond.cleanup5:                                ; preds = %for.cond.cleanup10
  %inc22 = add nuw nsw i32 %l1.035, 1
  %exitcond39.not = icmp eq i32 %inc22, 100
  br i1 %exitcond39.not, label %for.cond.cleanup, label %for.cond3.preheader, !llvm.loop !8

for.cond.cleanup10:                               ; preds = %for.body11
  %add.lcssa = phi double [ %add, %for.body11 ]
  store double %add.lcssa, ptr %arrayidx17, align 8, !tbaa !3
  %indvars.iv.next37 = add nuw nsw i64 %indvars.iv36, 1
  %exitcond38.not = icmp eq i64 %indvars.iv.next37, 1000
  br i1 %exitcond38.not, label %for.cond.cleanup5, label %for.cond8.preheader, !llvm.loop !10

for.body11:                                       ; preds = %for.cond8.preheader, %for.body11
  %indvars.iv = phi i64 [ 0, %for.cond8.preheader ], [ %indvars.iv.next, %for.body11 ]
  %add3132 = phi double [ %arrayidx17.promoted, %for.cond8.preheader ], [ %add, %for.body11 ]
  %arrayidx13 = getelementptr inbounds [1000 x [1000 x double]], ptr @a, i64 0, i64 %indvars.iv36, i64 %indvars.iv, !intel-tbaa !11
  %0 = load double, ptr %arrayidx13, align 8, !tbaa !11
  %arrayidx15 = getelementptr inbounds [1000 x double], ptr @b, i64 0, i64 %indvars.iv, !intel-tbaa !3
  %1 = load double, ptr %arrayidx15, align 8, !tbaa !3
  %mul = fmul fast double %1, %0
  %add = fadd fast double %add3132, %mul
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond.not, label %for.cond.cleanup10, label %for.body11, !llvm.loop !13
}

attributes #0 = { nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cmov,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA1000_d", !5, i64 0}
!5 = !{!"double", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
!10 = distinct !{!10, !9}
!11 = !{!12, !5, i64 0}
!12 = !{!"array@_ZTSA1000_A1000_d", !4, i64 0}
!13 = distinct !{!13, !9}

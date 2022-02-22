; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-runtime-dd -hir-cost-model-throttling=0 -print-after=hir-runtime-dd -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -hir-cost-model-throttling=0  -S < %s 2>&1 | FileCheck %s

; Source:
;
; void fcn(unsigned *A, unsigned *B, unsigned *LoopCount) {
; 	for (unsigned j=0 ; j<100; j++) {
; 		for (unsigned i=0; i< LoopCount[j]; i++) {
; 			A[i] += 100;
; 		}
; 	}
; }

; HIR:
;
; <0>          BEGIN REGION { }
; <33>               + DO i1 = 0, 99, 1   <DO_LOOP>
; <3>                |   %0 = (%LoopCount)[i1];
; <5>                |   if (%0 != 0)
; <5>                |   {
; <34>               |      + UNKNOWN LOOP i2
; <11>               |      |   <i2 = 0>
; <11>               |      |   for.body4:
; <13>               |      |   %1 = (%A)[i2];
; <15>               |      |   (%A)[i2] = %1 + 100;
; <17>               |      |   %2 = (%LoopCount)[i1];
; <21>               |      |   if (i2 + 1 <u %2)
; <21>               |      |   {
; <21>               |      |      <i2 = i2 + 1>
; <22>               |      |      goto for.body4;
; <21>               |      |   }
; <34>               |      + END LOOP
; <5>                |   }
; <33>               + END LOOP
; <0>          END REGION

; unsigned case for the compatible-unknown-loop lit-test

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   %0 = (%LoopCount)[i1];
; CHECK:       |   if (%0 != 0)
; CHECK:       |   {
; CHECK:       |      %2 = (%LoopCount)[i1];
; CHECK:       |      %ub = umax(1, zext.i32.i64(%2)) + -1;
; CHECK:       |      %mv.test = &((%A)[%ub]) >=u &((%LoopCount)[i1]);
; CHECK:       |      %mv.test2 = &((%LoopCount)[i1]) >=u &((%A)[0]);
; CHECK:       |      %mv.and = %mv.test  &  %mv.test2;
; CHECK:       |      if (%mv.and == 0)
; CHECK:       |      {
; CHECK:       |         + DO i2 = 0, %ub, 1   <DO_LOOP>  <MVTag: 34>
; CHECK:       |         |   %1 = (%A)[i2];
; CHECK:       |         |   (%A)[i2] = %1 + 100;
; CHECK:       |         + END LOOP
; CHECK:       |      }
; CHECK:       |      else
; CHECK:       |      {
; CHECK:       |         + UNKNOWN LOOP i2  <MVTag: 34> <nounroll> <novectorize>
; CHECK:       |         |   <i2 = 0>
; CHECK:       |         |   for.body4.38:
; CHECK:       |         |   %1 = (%A)[i2];
; CHECK:       |         |   (%A)[i2] = %1 + 100;
; CHECK:       |         |   %2 = (%LoopCount)[i1];
; CHECK:       |         |   if (i2 + 1 <u %2)
; CHECK:       |         |   {
; CHECK:       |         |      <i2 = i2 + 1>
; CHECK:       |         |      goto for.body4.38;
; CHECK:       |         |   }
; CHECK:       |         + END LOOP
; CHECK:       |      }
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK: END REGION

;Module Before HIR
; ModuleID = 'unsigned-unknown-loop.c'
source_filename = "unsigned-unknown-loop.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @fcn(i32* nocapture %A, i32* nocapture readnone %B, i32* nocapture readonly %LoopCount) local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.cond.cleanup3
  %indvars.iv20 = phi i64 [ 0, %entry ], [ %indvars.iv.next21, %for.cond.cleanup3 ]
  %arrayidx = getelementptr inbounds i32, i32* %LoopCount, i64 %indvars.iv20
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %cmp216.not = icmp eq i32 %0, 0
  br i1 %cmp216.not, label %for.cond.cleanup3, label %for.body4.preheader

for.body4.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond.cleanup3.loopexit:                       ; preds = %for.body4
  br label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond.cleanup3.loopexit, %for.cond1.preheader
  %indvars.iv.next21 = add nuw nsw i64 %indvars.iv20, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next21, 100
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.cond1.preheader, !llvm.loop !7

for.body4:                                        ; preds = %for.body4.preheader, %for.body4
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body4 ], [ 0, %for.body4.preheader ]
  %arrayidx6 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx6, align 4, !tbaa !3
  %add = add i32 %1, 100
  store i32 %add, i32* %arrayidx6, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %3 = zext i32 %2 to i64
  %cmp2 = icmp ult i64 %indvars.iv.next, %3
  br i1 %cmp2, label %for.body4, label %for.cond.cleanup3.loopexit, !llvm.loop !9
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
!9 = distinct !{!9, !8}

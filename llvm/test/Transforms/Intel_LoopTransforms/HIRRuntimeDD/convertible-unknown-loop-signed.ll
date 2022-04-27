; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-runtime-dd -hir-cost-model-throttling=0 -print-after=hir-runtime-dd -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -hir-cost-model-throttling=0  -S < %s 2>&1 | FileCheck %s

; RuntimeDD Multiversion an unknown-loop if it is convertible and has only one
; Lval mem-ref. In the source below, the inner loop generates an HIR UNKNOWN
; LOOP because the upper bound is a memory reference (LoopCount[j]). This might
; alias with the lval mem-ref in the body, A[i]. Therefore no-alias checking
; needs to be done at runtime.

; Source:
;
; void fcn(int *A, int *B, int *LoopCount) {
;     for (int j=0 ; j<100; j++) {
;         for (int i=0; i< LoopCount[j]; i++) {
;             A[i] += 100;
;         }
;     }
; }

; HIR:
;
; <0>          BEGIN REGION { }
; <33>               + DO i1 = 0, 99, 1   <DO_LOOP>
; <3>                |   %0 = (%LoopCount)[i1];
; <5>                |   if (%0 > 0)
; <5>                |   {
; <34>               |      + UNKNOWN LOOP i2
; <11>               |      |   <i2 = 0>
; <11>               |      |   for.body4:
; <13>               |      |   %1 = (%A)[i2];
; <15>               |      |   (%A)[i2] = %1 + 100;
; <17>               |      |   %2 = (%LoopCount)[i1];
; <21>               |      |   if (i2 + 1 < %2)
; <21>               |      |   {
; <21>               |      |      <i2 = i2 + 1>
; <22>               |      |      goto for.body4;
; <21>               |      |   }
; <34>               |      + END LOOP
; <5>                |   }
; <33>               + END LOOP
; <0>          END REGION

; After transformation, a successful runtime-DD check ends up running a DO LOOP
; version of the input UNKNOWN LOOP. The else contains the unmodified UNKNOWN
; LOOP with additional attributes of '<nounroll>' and '<novectorize>'.

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   %0 = (%LoopCount)[i1];
; CHECK:       |   if (%0 > 0)
; CHECK:       |   {
; CHECK:       |      %2 = (%LoopCount)[i1];
; CHECK:       |      %ub = smax(1, sext.i32.i64(%2)) + -1;
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
; CHECK:       |         |   if (i2 + 1 < %2)
; CHECK:       |         |   {
; CHECK:       |         |      <i2 = i2 + 1>
; CHECK:       |         |      goto for.body4.38;
; CHECK:       |         |   }
; CHECK:       |         + END LOOP
; CHECK:       |      }
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK: END REGION

; ModuleID = 'unknownloop.cpp'
source_filename = "unknownloop.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
define dso_local void @_Z3fcnPiS_S_(i32* nocapture %A, i32* nocapture readnone %B, i32* nocapture readonly %LoopCount) local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.cond.cleanup3
  %indvars.iv19 = phi i64 [ 0, %entry ], [ %indvars.iv.next20, %for.cond.cleanup3 ]
  %arrayidx = getelementptr inbounds i32, i32* %LoopCount, i64 %indvars.iv19
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %cmp216 = icmp sgt i32 %0, 0
  br i1 %cmp216, label %for.body4.preheader, label %for.cond.cleanup3

for.body4.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond.cleanup3.loopexit:                       ; preds = %for.body4
  br label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond.cleanup3.loopexit, %for.cond1.preheader
  %indvars.iv.next20 = add nuw nsw i64 %indvars.iv19, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next20, 100
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.cond1.preheader, !llvm.loop !7

for.body4:                                        ; preds = %for.body4.preheader, %for.body4
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body4 ], [ 0, %for.body4.preheader ]
  %arrayidx6 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx6, align 4, !tbaa !3
  %add = add nsw i32 %1, 100
  store i32 %add, i32* %arrayidx6, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %3 = sext i32 %2 to i64
  %cmp2 = icmp slt i64 %indvars.iv.next, %3
  br i1 %cmp2, label %for.body4, label %for.cond.cleanup3.loopexit, !llvm.loop !9
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
!9 = distinct !{!9, !8}

; RUN: opt %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis -enable-compress-expand-idiom 2>&1 | FileCheck %s

; <0>          BEGIN REGION { }
; <25>               + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; <6>                |   if ((%a)[i1] != 0)
; <6>                |   {
; <13>               |      %1 = (%f)[%c.014];
; <10>               |      %c.014 = %c.014  +  1;
; <14>               |      %r.015 = %1  +  %r.015;
; <6>                |   }
; <25>               + END LOOP
; <0>          END REGION

; CHECK:      [Compress/Expand Idiom] Increment {sb:4}+1 detected: <10>         %c.014 = %c.014  +  1;
; CHECK-NOT:  [Compress/Expand Idiom] Increment rejected
; CHECK:      Idiom List
; CHECK-NEXT: CEIndexIncFirst: <10>         %c.014 = %c.014  +  1;
; CHECK-NEXT:   CELoad: (%f)[%c.014]
; CHECK-NEXT:     CELdStIndex: %c.014

source_filename = "compress3.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree nosync nounwind uwtable
define dso_local noundef i32 @_Z3fooPfiPi(float* noalias nocapture noundef readonly %f, i32 noundef %n, i32* noalias noundef %a) local_unnamed_addr #0 {
entry:
  call void @llvm.assume(i1 true) [ "align"(i32* %a, i64 32) ]
  %cmp13 = icmp sgt i32 %n, 0
  br i1 %cmp13, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count18 = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  %c.1.lcssa = phi i32 [ %c.1, %for.inc ]
  %r.1.lcssa = phi float [ %r.1, %for.inc ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %c.0.lcssa = phi i32 [ 0, %entry ], [ %c.1.lcssa, %for.cond.cleanup.loopexit ]
  %r.0.lcssa = phi float [ 0.000000e+00, %entry ], [ %r.1.lcssa, %for.cond.cleanup.loopexit ]
  %conv = sitofp i32 %c.0.lcssa to float
  %add4 = fadd fast float %r.0.lcssa, %conv
  %conv5 = fptosi float %add4 to i32
  ret i32 %conv5

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %r.015 = phi float [ 0.000000e+00, %for.body.preheader ], [ %r.1, %for.inc ]
  %c.014 = phi i32 [ 0, %for.body.preheader ], [ %c.1, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %tobool.not = icmp eq i32 %0, 0
  br i1 %tobool.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %inc = add nsw i32 %c.014, 1
  %idxprom1 = sext i32 %c.014 to i64
  %arrayidx2 = getelementptr inbounds float, float* %f, i64 %idxprom1
  %1 = load float, float* %arrayidx2, align 4, !tbaa !7
  %add = fadd fast float %1, %r.015
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %c.1 = phi i32 [ %inc, %if.then ], [ %c.014, %for.body ]
  %r.1 = phi float [ %add, %if.then ], [ %r.015, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count18
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !9
}

; Function Attrs: inaccessiblememonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #1

attributes #0 = { mustprogress nofree nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { inaccessiblememonly mustprogress nofree nosync nounwind willreturn }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"float", !5, i64 0}
!9 = distinct !{!9, !10, !11}
!10 = !{!"llvm.loop.mustprogress"}
!11 = !{!"llvm.loop.intel.vector.aligned"}

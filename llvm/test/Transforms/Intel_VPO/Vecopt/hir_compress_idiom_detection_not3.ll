; RUN: opt %s -mattr=+avx512f,+avx512vl -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis -enable-compress-expand-idiom 2>&1 | FileCheck %s

; <0>          BEGIN REGION { }
; <36>               + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; <2>                |   %k.034.out = %k.034;
; <6>                |   if ((%a)[i1] != 0)
; <6>                |   {
; <12>               |      %k.034 = %k.034  +  3;
; <15>               |      (%d1)[%k.034] = (%v1)[i1];
; <21>               |      (%v2)[i1] = (%d2)[%k.034.out + 4];
; <27>               |      (%d3)[%k.034.out + 6] = (%v3)[i1];
; <6>                |   }
; <36>               + END LOOP
; <0>          END REGION

; CHECK:      [Compress/Expand Idiom] Increment {sb:3}+3 detected: <12>         %k.034 = %k.034  +  3;
; CHECK:      [Compress/Expand Idiom] Inconsistent parent of dependency: <2>          %k.034.out = %k.034;
; CHECK-NEXT: [Compress/Expand Idiom] Increment rejected: <12>         %k.034 = %k.034  +  3;
; CHECK:      Idiom List
; CHECK-NEXT:   No idioms detected.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
define dso_local void @_Z3fooPiS_S_S_S_S_S_i(i32* noalias nocapture noundef readonly %a, i32* noalias nocapture noundef writeonly %d1, i32* noalias nocapture noundef readonly %v1, i32* noalias nocapture noundef readonly %d2, i32* noalias nocapture noundef writeonly %v2, i32* noalias nocapture noundef writeonly %d3, i32* noalias nocapture noundef readonly %v3, i32 noundef %n) local_unnamed_addr #0 {
entry:
  %cmp33 = icmp sgt i32 %n, 0
  br i1 %cmp33, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count36 = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %k.034 = phi i32 [ 0, %for.body.preheader ], [ %k.1, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %tobool.not = icmp eq i32 %0, 0
  br i1 %tobool.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx2 = getelementptr inbounds i32, i32* %v1, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx2, align 4, !tbaa !3
  %add = add nsw i32 %k.034, 3
  %idxprom3 = sext i32 %add to i64
  %arrayidx4 = getelementptr inbounds i32, i32* %d1, i64 %idxprom3
  store i32 %1, i32* %arrayidx4, align 4, !tbaa !3
  %add6 = add nsw i32 %k.034, 4
  %idxprom7 = sext i32 %add6 to i64
  %arrayidx8 = getelementptr inbounds i32, i32* %d2, i64 %idxprom7
  %2 = load i32, i32* %arrayidx8, align 4, !tbaa !3
  %arrayidx10 = getelementptr inbounds i32, i32* %v2, i64 %indvars.iv
  store i32 %2, i32* %arrayidx10, align 4, !tbaa !3
  %arrayidx13 = getelementptr inbounds i32, i32* %v3, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx13, align 4, !tbaa !3
  %add14 = add nsw i32 %k.034, 6
  %idxprom15 = sext i32 %add14 to i64
  %arrayidx16 = getelementptr inbounds i32, i32* %d3, i64 %idxprom15
  store i32 %3, i32* %arrayidx16, align 4, !tbaa !3
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %k.1 = phi i32 [ %add, %if.then ], [ %k.034, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count36
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !7
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}

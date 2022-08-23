; RUN: opt %s -mattr=+avx512f,+avx512vl -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis -enable-compress-expand-idiom 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   if ((%C1)[i1] == 0)
;       |   {
;       |      if ((%C2)[i1] == 0)
;       |      {
;       |         %2 = (%B)[%n2.025];
;       |         %n2.025 = %n2.025  +  1;
;       |         (%B)[%n2.025] = %2;
;       |      }
;       |   }
;       + END LOOP
; END REGION

; CHECK:      [Compress/Expand Idiom] Increment {sb:3}+1 detected: <17>         %n2.025 = %n2.025  +  1;
; CHECK-NEXT: [Compress/Expand Idiom] Data-dependency on redefined index found: (%B)[%n2.025]
; CHECK-NEXT: [Compress/Expand Idiom] Increment rejected: <17>         %n2.025 = %n2.025  +  1;
; CHECK-NEXT: Idiom List
; CHECK-NEXT:   No idioms detected.
  
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind uwtable
define dso_local void @_Z3fooPdS_S_PiS0_S_i(double* noalias nocapture noundef readnone %A1, double* noalias nocapture noundef readnone %A2, double* noalias nocapture noundef %B, i32* noalias nocapture noundef readonly %C1, i32* noalias nocapture noundef readonly %C2, double* noalias nocapture noundef readnone %C3, i32 noundef %N) local_unnamed_addr #0 {
entry:
  %cmp23 = icmp sgt i32 %N, 0
  br i1 %cmp23, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %N to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %n2.025 = phi i32 [ 0, %for.body.preheader ], [ %n2.1, %for.inc ]
  %n1.024 = phi i32 [ 0, %for.body.preheader ], [ %n1.1, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %C1, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %cmp1.not = icmp eq i32 %0, 0
  br i1 %cmp1.not, label %if.else, label %for.inc

if.else:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, i32* %C2, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx3, align 4, !tbaa !3
  %cmp4.not = icmp eq i32 %1, 0
  br i1 %cmp4.not, label %if.else6, label %for.inc

if.else6:                                         ; preds = %if.else
  %inc = add nsw i32 %n2.025, 1
  %idxprom7 = sext i32 %n2.025 to i64
  %arrayidx8 = getelementptr inbounds double, double* %B, i64 %idxprom7
  %2 = load double, double* %arrayidx8, align 8, !tbaa !7
  %inc9 = add nsw i32 %n1.024, 1
  %idxprom10 = sext i32 %inc to i64
  %arrayidx11 = getelementptr inbounds double, double* %B, i64 %idxprom10
  store double %2, double* %arrayidx11, align 8, !tbaa !7
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.else, %if.else6
  %n1.1 = phi i32 [ %n1.024, %for.body ], [ %n1.024, %if.else ], [ %inc9, %if.else6 ]
  %n2.1 = phi i32 [ %n2.025, %for.body ], [ %n2.025, %if.else ], [ %inc, %if.else6 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !9
}

attributes #0 = { argmemonly mustprogress nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"double", !5, i64 0}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}

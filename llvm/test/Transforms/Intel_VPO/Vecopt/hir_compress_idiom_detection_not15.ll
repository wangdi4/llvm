; RUN: opt %s -mattr=+avx512f,+avx512vl -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis -enable-compress-expand-idiom 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   if ((%C1)[i1] == 0)
;       |   {
;       |      %res = 2.000000e+00;
;       |      if ((%C2)[i1] == 0)
;       |      {
;       |         %3 = (%B)[%n2.033];
;       |         %n2.033 = %n2.033  +  1;
;       |         %res = %3;
;       |      }
;       |   }
;       |   else
;       |   {
;       |      (%B)[%k.031] = (%A1)[i1];
;       |      %k.031 = %k.031  +  1;
;       |      %res = 1.000000e+00;
;       |   }
;       + END LOOP
; END REGION

; CHECK:      [Compress/Expand Idiom] Increment {sb:3}+1 detected: <19>         %n2.033 = %n2.033  +  1;
; CHECK-NEXT: [Compress/Expand Idiom] Increment {sb:5}+1 detected: <29>         %k.031 = %k.031  +  1;
; CHECK-NEXT: [Compress/Expand Idiom] Data-dependency on other memory found: (%B)[%k.031]
; CHECK-NEXT: [Compress/Expand Idiom] Increment rejected: <19>         %n2.033 = %n2.033  +  1;
; CHECK-NEXT: [Compress/Expand Idiom] Data-dependency on other memory found: (%B)[%n2.033]
; CHECK-NEXT: [Compress/Expand Idiom] Increment rejected: <29>         %k.031 = %k.031  +  1;
; CHECK-NEXT: Idiom List
; CHECK-NEXT:   No idioms detected.
  
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind uwtable
define dso_local double @_Z3fooPdS_S_PiS0_S_i(double* noalias nocapture noundef readonly %A1, double* noalias nocapture noundef readnone %A2, double* noalias nocapture noundef %B, i32* noalias nocapture noundef readonly %C1, i32* noalias nocapture noundef readonly %C2, double* noalias nocapture noundef readnone %C3, i32 noundef %N) local_unnamed_addr #0 {
entry:
  %cmp30 = icmp sgt i32 %N, 0
  br i1 %cmp30, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %N to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %ret = phi double [ %res, %for.cond.cleanup.loopexit ], [ 0., %entry ]
  ret double %ret

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %n2.033 = phi i32 [ 0, %for.body.preheader ], [ %n2.1, %for.inc ]
  %n1.032 = phi i32 [ 0, %for.body.preheader ], [ %n1.1, %for.inc ]
  %k.031 = phi i32 [ 0, %for.body.preheader ], [ %k.1, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %C1, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %cmp1.not = icmp eq i32 %0, 0
  br i1 %cmp1.not, label %if.else, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds double, double* %A1, i64 %indvars.iv
  %1 = load double, double* %arrayidx3, align 8, !tbaa !7
  %inc = add nsw i32 %k.031, 1
  %idxprom4 = sext i32 %k.031 to i64
  %arrayidx5 = getelementptr inbounds double, double* %B, i64 %idxprom4
  store double %1, double* %arrayidx5, align 8, !tbaa !7
  br label %for.inc

if.else:                                          ; preds = %for.body
  %arrayidx7 = getelementptr inbounds i32, i32* %C2, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx7, align 4, !tbaa !3
  %cmp8.not = icmp eq i32 %2, 0
  br i1 %cmp8.not, label %if.else10, label %for.inc

if.else10:                                        ; preds = %if.else
  %inc11 = add nsw i32 %n2.033, 1
  %idxprom12 = sext i32 %n2.033 to i64
  %arrayidx13 = getelementptr inbounds double, double* %B, i64 %idxprom12
  %3 = load double, double* %arrayidx13, align 8, !tbaa !7
  %inc14 = add nsw i32 %n1.032, 1
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else, %if.else10
  %k.1 = phi i32 [ %inc, %if.then ], [ %k.031, %if.else ], [ %k.031, %if.else10 ]
  %n1.1 = phi i32 [ %n1.032, %if.then ], [ %n1.032, %if.else ], [ %inc14, %if.else10 ]
  %n2.1 = phi i32 [ %n2.033, %if.then ], [ %n2.033, %if.else ], [ %inc11, %if.else10 ]
  %res = phi double [ 1., %if.then ], [ 2., %if.else ], [ %3, %if.else10 ]
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

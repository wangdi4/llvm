; RUN: opt %s -mattr=+avx512f,+avx512vl -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   if ((%C1)[i1] == 0)
;       |   {
;       |      if ((%C2)[i1] == 0)
;       |      {
;       |         %4 = (%B)[%n2.046];
;       |         %n2.046 = %n2.046  +  1;
;       |         (%B)[%n1.045] = %4;
;       |         %n1.045 = %n1.045  +  1;
;       |      }
;       |      else
;       |      {
;       |         (%B)[0] = (%A2)[i1];
;       |      }
;       |   }
;       |   else
;       |   {
;       |      (%B)[%k.044] = (%A1)[i1];
;       |      %k.044 = %k.044  +  1;
;       |   }
;       |   (%C3)[i1] = (%B)[i1];
;       + END LOOP
; END REGION

; CHECK:      [Compress/Expand Idiom] Increment {sb:3}+1 detected: <19>         %n2.046 = %n2.046  +  1;
; CHECK-NEXT: [Compress/Expand Idiom] Increment {sb:4}+1 detected: <23>         %n1.045 = %n1.045  +  1;
; CHECK-NEXT: [Compress/Expand Idiom] Increment {sb:5}+1 detected: <36>         %k.044 = %k.044  +  1;
; CHECK-NEXT: [Compress/Expand Idiom] Data-dependency on other memory found: (%B)[%n1.045]
; CHECK-NEXT: [Compress/Expand Idiom] Increment rejected: <19>         %n2.046 = %n2.046  +  1;
; CHECK-NEXT: [Compress/Expand Idiom] Data-dependency on other memory found: (%B)[%n2.046]
; CHECK-NEXT: [Compress/Expand Idiom] Increment rejected: <23>         %n1.045 = %n1.045  +  1;
; CHECK-NEXT: [Compress/Expand Idiom] Data-dependency on other memory found: (%B)[%n2.046]
; CHECK-NEXT: [Compress/Expand Idiom] Increment rejected: <36>         %k.044 = %k.044  +  1;
; CHECK-NEXT: Idiom List
; CHECK-NEXT:   No idioms detected.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind uwtable
define dso_local void @_Z3fooPdS_S_PiS0_S_i(double* noalias nocapture noundef readonly %A1, double* noalias nocapture noundef readonly %A2, double* noalias nocapture noundef %B, i32* noalias nocapture noundef readonly %C1, i32* noalias nocapture noundef readonly %C2, double* noalias nocapture noundef writeonly %C3, i32 noundef %N) local_unnamed_addr #0 {
entry:
  %cmp43 = icmp sgt i32 %N, 0
  br i1 %cmp43, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %N to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %if.end21
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %if.end21
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %if.end21 ]
  %n2.046 = phi i32 [ 0, %for.body.preheader ], [ %n2.1, %if.end21 ]
  %n1.045 = phi i32 [ 0, %for.body.preheader ], [ %n1.1, %if.end21 ]
  %k.044 = phi i32 [ 0, %for.body.preheader ], [ %k.1, %if.end21 ]
  %arrayidx = getelementptr inbounds i32, i32* %C1, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %cmp1.not = icmp eq i32 %0, 0
  br i1 %cmp1.not, label %if.else, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds double, double* %A1, i64 %indvars.iv
  %1 = load double, double* %arrayidx3, align 8, !tbaa !7
  %inc = add nsw i32 %k.044, 1
  %idxprom4 = sext i32 %k.044 to i64
  %arrayidx5 = getelementptr inbounds double, double* %B, i64 %idxprom4
  store double %1, double* %arrayidx5, align 8, !tbaa !7
  br label %if.end21

if.else:                                          ; preds = %for.body
  %arrayidx7 = getelementptr inbounds i32, i32* %C2, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx7, align 4, !tbaa !3
  %cmp8.not = icmp eq i32 %2, 0
  br i1 %cmp8.not, label %if.else14, label %if.then9

if.then9:                                         ; preds = %if.else
  %arrayidx11 = getelementptr inbounds double, double* %A2, i64 %indvars.iv
  %3 = load double, double* %arrayidx11, align 8, !tbaa !7
  store double %3, double* %B, align 8, !tbaa !7
  br label %if.end21

if.else14:                                        ; preds = %if.else
  %inc15 = add nsw i32 %n2.046, 1
  %idxprom16 = sext i32 %n2.046 to i64
  %arrayidx17 = getelementptr inbounds double, double* %B, i64 %idxprom16
  %4 = load double, double* %arrayidx17, align 8, !tbaa !7
  %inc18 = add nsw i32 %n1.045, 1
  %idxprom19 = sext i32 %n1.045 to i64
  %arrayidx20 = getelementptr inbounds double, double* %B, i64 %idxprom19
  store double %4, double* %arrayidx20, align 8, !tbaa !7
  br label %if.end21

if.end21:                                         ; preds = %if.then9, %if.else14, %if.then
  %k.1 = phi i32 [ %inc, %if.then ], [ %k.044, %if.then9 ], [ %k.044, %if.else14 ]
  %n1.1 = phi i32 [ %n1.045, %if.then ], [ %n1.045, %if.then9 ], [ %inc18, %if.else14 ]
  %n2.1 = phi i32 [ %n2.046, %if.then ], [ %n2.046, %if.then9 ], [ %inc15, %if.else14 ]
  %arrayidx23 = getelementptr inbounds double, double* %B, i64 %indvars.iv
  %5 = load double, double* %arrayidx23, align 8, !tbaa !7
  %arrayidx25 = getelementptr inbounds double, double* %C3, i64 %indvars.iv
  store double %5, double* %arrayidx25, align 8, !tbaa !7
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

; RUN: opt %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis -enable-compress-expand-idiom 2>&1 | FileCheck %s

; <0>          BEGIN REGION { }
; <27>               + DO i1 = 0, 14, 1   <DO_LOOP>
; <6>                |   if ((%c)[i1] != 0)
; <6>                |   {
; <15>               |      %add = (%a)[i1]  +  (%b)[%k.013];
; <16>               |      (%b)[%k.013] = %add;
; <17>               |      %k.013 = %k.013  +  1;
; <6>                |   }
; <27>               + END LOOP
; <0>          END REGION

; CHECK:      [Compress/Expand Idiom] Increment {sb:3}+1 detected: <18>         %k.013 = %k.013  +  1;
; CHECK-NOT:  [Compress/Expand Idiom] Increment rejected
; CHECK:      Idiom List
; CHECK-NEXT: CEIndexIncFirst: <18>         %k.013 = %k.013  +  1;
; CHECK-DAG:    CELoad: (%b)[%k.013]
; CHECK-DAG:      CELdStIndex: %k.013
; CHECK-DAG:    CEStore: <17>         (%b)[%k.013] = %add;
; CHECK-DAG:      CELdStIndex: %k.013

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind uwtable
define dso_local void @_Z4vaddjPfS_Pi(i32 noundef %n, float* noalias nocapture noundef %a, float* noalias nocapture noundef readonly %b, i32* noalias nocapture noundef readonly %c) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ], !in.de.ssa !3
  %k.013 = phi i32 [ 0, %entry ], [ %k.1, %for.inc ]
  %k.013.out = call i32 @llvm.ssa.copy.i32(i32 %k.013), !out.de.ssa !4
  %arrayidx = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !5
  %tobool.not = icmp eq i32 %0, 0
  br i1 %tobool.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %idxprom1 = zext i32 %k.013.out to i64
  %arrayidx2 = getelementptr inbounds float, float* %b, i64 %idxprom1
  %1 = load float, float* %arrayidx2, align 4, !tbaa !9
  %arrayidx4 = getelementptr inbounds float, float* %a, i64 %indvars.iv
  %2 = load float, float* %arrayidx4, align 4, !tbaa !9
  %add = fadd fast float %2, %1
  store float %add, float* %arrayidx2, align 4, !tbaa !9
  %inc = add i32 %k.013, 1, !live.range.de.ssa !4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %k.1 = phi i32 [ %inc, %if.then ], [ %k.013, %for.body ], !live.range.de.ssa !4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 15
  %indvars.iv.in = call i64 @llvm.ssa.copy.i64(i64 %indvars.iv.next), !in.de.ssa !3
  br i1 %exitcond.not, label %for.end, label %for.body, !llvm.loop !11

for.end:                                          ; preds = %for.inc
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind readnone willreturn
declare i64 @llvm.ssa.copy.i64(i64 returned) #1

; Function Attrs: nocallback nofree nosync nounwind readnone willreturn
declare i32 @llvm.ssa.copy.i32(i32 returned) #1

attributes #0 = { argmemonly mustprogress nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind readnone willreturn }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!"indvars.iv.de.ssa"}
!4 = !{}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}
!9 = !{!10, !10, i64 0}
!10 = !{!"float", !7, i64 0}
!11 = distinct !{!11, !12}
!12 = !{!"llvm.loop.mustprogress"}

; When use cannot be moved before single rval definition of temp, we can try if single rval 
; definition of temp can be moved after use
;
; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -print-before=hir-temp-cleanup -print-after=hir-temp-cleanup 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>,hir-temp-cleanup,print<hir-framework>" 2>&1 | FileCheck %s

; RUN: opt < %s -opaque-pointers -hir-ssa-deconstruction -hir-temp-cleanup -print-before=hir-temp-cleanup -print-after=hir-temp-cleanup 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -passes="hir-ssa-deconstruction,print<hir-framework>,hir-temp-cleanup,print<hir-framework>" 2>&1 | FileCheck %s
;
;*** IR Dump Before HIR Temp Cleanup (hir-temp-cleanup) ***
;Function: sub
;
;<0>          BEGIN REGION { }
;<24>               + DO i1 = 0, 999, 1   <DO_LOOP>
;<2>                |   %m.010.out = %m.010;
;<4>                |   %0 = (%p)[i1];
;<6>                |   if (%0 > 0.000000e+00)
;<6>                |   {
;<10>               |      %m.010 = %m.010  +  1;
;<13>               |      %1 = (%q)[%m.010.out];
;<14>               |      %add = %1  +  1.000000e+00;
;<15>               |      (%q)[%m.010.out] = %add;
;<6>                |   }
;<24>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Temp Cleanup (hir-temp-cleanup) ***
;Function: sub
;
; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:           |   if ((%p)[i1] > 0.000000e+00)
; CHECK:           |   {
; CHECK:           |      %add = (%q)[%m.010]  +  1.000000e+00;
; CHECK:           |      (%q)[%m.010] = %add;
; CHECK:           |      %m.010 = %m.010  +  1;
; CHECK:           |   }
; CHECK:           + END LOOP
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @sub(float* noalias nocapture readonly %p, float* noalias nocapture %q) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc
  ret void

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %m.010 = phi i32 [ 0, %entry ], [ %m.1, %for.inc ]
  %arrayidx = getelementptr inbounds float, float* %p, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4, !tbaa !3
  %cmp1 = fcmp fast ogt float %0, 0.000000e+00
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %inc = add nsw i32 %m.010, 1
  %idxprom3 = sext i32 %m.010 to i64
  %arrayidx4 = getelementptr inbounds float, float* %q, i64 %idxprom3
  %1 = load float, float* %arrayidx4, align 4, !tbaa !3
  %add = fadd fast float %1, 1.000000e+00
  store float %add, float* %arrayidx4, align 4, !tbaa !3
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %m.1 = phi i32 [ %inc, %if.then ], [ %m.010, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !llvm.loop !7
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}

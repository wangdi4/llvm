; Although the temp %q.022 is a live-in temp and it is not in a safe-reduction chain,
; B[i1] can still be scalar replacement candidate. Thus, RTDD is triggered in this test case.
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
;*** IR Dump Before HIR RuntimeDD Multiversioning (hir-runtime-dd) ***
;Function: sub
;
;<0>          BEGIN REGION { }
;<18>               + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;<3>                |   %0 = (%B)[i1];
;<4>                |   %add = %0  +  %q.022;
;<6>                |   (%A)[i1] = %add;
;<7>                |   %1 = (%B)[i1];
;<8>                |   %add5 = %1  +  1.000000e+00;
;<9>                |   %mul = %add5  *  %add5;
;<10>               |   (%A)[i1] = %mul;
;<13>               |   %q.022 = %add5;
;<18>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR RuntimeDD Multiversioning (hir-runtime-dd) ***
;Function: sub
;
; CHECK:           %mv.test = &((%B)[zext.i32.i64(%N) + -1]) >=u &((%A)[0]);
; CHECK:           %mv.test1 = &((%A)[zext.i32.i64(%N) + -1]) >=u &((%B)[0]);
; CHECK:           %mv.and = %mv.test  &  %mv.test1;
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local i32 @sub(ptr nocapture %A, ptr nocapture readonly %B, i32 %N) local_unnamed_addr #0 {
entry:
  %cmp20 = icmp sgt i32 %N, 0
  br i1 %cmp20, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count23 = zext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %q.022 = phi float [ 0.000000e+00, %for.body.preheader ], [ %add5, %for.body ]
  %arrayidx = getelementptr inbounds float, ptr %B, i64 %indvars.iv
  %0 = load float, ptr %arrayidx, align 4, !tbaa !3
  %add = fadd fast float %0, %q.022
  %arrayidx2 = getelementptr inbounds float, ptr %A, i64 %indvars.iv
  store float %add, ptr %arrayidx2, align 4, !tbaa !3
  %1 = load float, ptr %arrayidx, align 4, !tbaa !3
  %add5 = fadd fast float %1, 1.000000e+00
  %mul = fmul fast float %add5, %add5
  store float %mul, ptr %arrayidx2, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count23
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !7

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret i32 undef
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="true" }

!3 = !{!4, !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}

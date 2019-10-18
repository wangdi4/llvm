; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-general-unroll -print-after=hir-general-unroll -hir-cg -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,hir-general-unroll,print<hir>,hir-cg" -S < %s 2>&1 | FileCheck %s
;
;*** IR Dump Before HIR General Unroll ***
;Function: sub
;
;<0>          BEGIN REGION { }
;<12>               + DO i1 = 0, sext.i32.i64(%M) + -1, 1   <DO_LOOP> <unroll = 2>
;<3>                |   %0 = (%A)[i1];
;<4>                |   %add = %0  +  1.000000e+00;
;<5>                |   (%A)[i1] = %add;
;<12>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR General Unroll ***
;Function: sub
;
; CHECK:     BEGIN REGION { modified }
; CHECK:           %tgu = (sext.i32.i64(%M))/u2;
;
; CHECK:           + DO i1 = 0, %tgu + -1, 1   <DO_LOOP> <nounroll>
; CHECK:           |   %0 = (%A)[2 * i1];
; CHECK:           |   %add = %0  +  1.000000e+00;
; CHECK:           |   (%A)[2 * i1] = %add;
; CHECK:           |   %0 = (%A)[2 * i1 + 1];
; CHECK:           |   %add = %0  +  1.000000e+00;
; CHECK:           |   (%A)[2 * i1 + 1] = %add;
; CHECK:           + END LOOP
;
; CHECK:           if (2 * %tgu <u sext.i32.i64(%M))
; CHECK:           {
; CHECK:              %0 = (%A)[2 * %tgu];
; CHECK:              %add = %0  +  1.000000e+00;
; CHECK:              (%A)[2 * %tgu] = %add;
; CHECK:           }
; CHECK:     END REGION

;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local i32 @sub(i32* nocapture readonly %X, i32 %N, i32 %M, float* nocapture %A) local_unnamed_addr #0 {
entry:
  %cmp25 = icmp sgt i32 %M, 0
  br i1 %cmp25, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %M to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds float, float* %A, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4, !tbaa !2
  %add = fadd float %0, 1.000000e+00
  store float %add, float* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body, !llvm.loop !6

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  switch i32 %N, label %sw.default [
    i32 0, label %sw.epilog
    i32 4, label %sw.bb
    i32 3, label %sw.bb3
    i32 2, label %sw.bb8
    i32 1, label %sw.bb13
  ]

sw.bb:                                            ; preds = %for.end
  %arrayidx1 = getelementptr inbounds i32, i32* %X, i64 3
  %1 = load i32, i32* %arrayidx1, align 4, !tbaa !8
  %cmp2 = icmp eq i32 %1, 1
  br i1 %cmp2, label %sw.epilog, label %sw.bb3

sw.bb3:                                           ; preds = %sw.bb, %for.end
  %arrayidx4 = getelementptr inbounds i32, i32* %X, i64 2
  %2 = load i32, i32* %arrayidx4, align 4, !tbaa !8
  %cmp5 = icmp eq i32 %2, 1
  br i1 %cmp5, label %sw.epilog, label %sw.bb8

sw.bb8:                                           ; preds = %sw.bb3, %for.end
  %arrayidx9 = getelementptr inbounds i32, i32* %X, i64 1
  %3 = load i32, i32* %arrayidx9, align 4, !tbaa !8
  %cmp10 = icmp eq i32 %3, 1
  br i1 %cmp10, label %sw.epilog, label %sw.bb13

sw.bb13:                                          ; preds = %sw.bb8, %for.end
  %4 = load i32, i32* %X, align 4, !tbaa !8
  %cmp15 = icmp eq i32 %4, 1
  br i1 %cmp15, label %sw.epilog, label %sw.default

sw.default:                                       ; preds = %sw.bb13, %for.end
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.bb13, %sw.bb8, %sw.bb3, %sw.bb, %sw.default, %for.end
  %i.0 = phi i32 [ 1, %sw.default ], [ 0, %sw.bb13 ], [ 0, %sw.bb8 ], [ 0, %sw.bb3 ], [ 0, %sw.bb ], [ 0, %for.end ]
  ret i32 %i.0
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"DPC++ Compiler 2021.1 (YYYY.8.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.unroll.count", i32 2}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !4, i64 0}

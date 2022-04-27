; Although live-in temp %q.029 is not in the safe-reduction chain, the loop can help scalar replacement.
; In this case, B[i1] and B[i1+1] are scalar replacement candidates. We can still make RTDD get triggered.
;
; RUN: opt -hir-ssa-deconstruction -hir-runtime-dd -print-after=hir-runtime-dd -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
;*** IR Dump Before HIR RuntimeDD Multiversioning (hir-runtime-dd) ***
;Function: sub
;
;<0>          BEGIN REGION { }
;<21>               + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>   <LEGAL_MAX_TC = 2147483647>
;<3>                |   %0 = (%B)[i1];
;<4>                |   %add = %0  +  %q.029;
;<6>                |   (%A)[i1] = %add;
;<9>                |   %1 = (%A)[i1 + 1];
;<11>               |   %2 = (%B)[i1 + 1];
;<12>               |   %add9 = %2  +  %1;
;<13>               |   %mul = %add9  *  %add9;
;<14>               |   (%A)[i1] = %mul;
;<16>               |   %q.029 = %add9;
;<21>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR RuntimeDD Multiversioning (hir-runtime-dd) ***
;Function: sub
;
; CHECK:    BEGIN REGION { }
; CHECK:           %mv.test = &((%B)[zext.i32.i64(%N)]) >=u &((%A)[0]);
; CHECK:           %mv.test1 = &((%A)[zext.i32.i64(%N)]) >=u &((%B)[0]);
; CHECK:           %mv.and = %mv.test  &  %mv.test1;
; CHECK:           if (%mv.and == 0)
; CHECK:           {
; CHECK:              + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>   <LEGAL_MAX_TC = 2147483647>  <MVTag: 21>
; CHECK:              |   %0 = (%B)[i1];
; CHECK:              |   %add = %0  +  %q.029;
; CHECK:              |   (%A)[i1] = %add;
; CHECK:              |   %1 = (%A)[i1 + 1];
; CHECK:              |   %2 = (%B)[i1 + 1];
; CHECK:              |   %add9 = %2  +  %1;
; CHECK:              |   %mul = %add9  *  %add9;
; CHECK:              |   (%A)[i1] = %mul;
; CHECK:              |   %q.029 = %add9;
; CHECK:              + END LOOP
; CHECK:           }
; CHECK:           else
; CHECK:           {
; CHECK:              + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>   <LEGAL_MAX_TC = 2147483647>  <MVTag: 21> <nounroll> <novectorize>
; CHECK:              |   %0 = (%B)[i1];
; CHECK:              |   %add = %0  +  %q.029;
; CHECK:              |   (%A)[i1] = %add;
; CHECK:              |   %1 = (%A)[i1 + 1];
; CHECK:              |   %2 = (%B)[i1 + 1];
; CHECK:              |   %add9 = %2  +  %1;
; CHECK:              |   %mul = %add9  *  %add9;
; CHECK:              |   (%A)[i1] = %mul;
; CHECK:              |   %q.029 = %add9;
; CHECK:              + END LOOP
; CHECK:           }
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local i32 @sub(float* nocapture %A, float* nocapture readonly %B, i32 %N) local_unnamed_addr #0 {
entry:
  %cmp27 = icmp sgt i32 %N, 0
  br i1 %cmp27, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count30 = zext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %q.029 = phi float [ 0.000000e+00, %for.body.preheader ], [ %add9, %for.body ]
  %arrayidx = getelementptr inbounds float, float* %B, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4, !tbaa !3
  %add = fadd fast float %0, %q.029
  %arrayidx2 = getelementptr inbounds float, float* %A, i64 %indvars.iv
  store float %add, float* %arrayidx2, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx5 = getelementptr inbounds float, float* %A, i64 %indvars.iv.next
  %1 = load float, float* %arrayidx5, align 4, !tbaa !3
  %arrayidx8 = getelementptr inbounds float, float* %B, i64 %indvars.iv.next
  %2 = load float, float* %arrayidx8, align 4, !tbaa !3
  %add9 = fadd fast float %2, %1
  %mul = fmul fast float %add9, %add9
  store float %mul, float* %arrayidx2, align 4, !tbaa !3
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count30
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !7

for.end.loopexit:                                 ; preds = %for.body
  %add9.lcssa = phi float [ %add9, %for.body ]
  %phi.cast = fptosi float %add9.lcssa to i32
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %q.0.lcssa = phi i32 [ 0, %entry ], [ %phi.cast, %for.end.loopexit ]
  ret i32 %q.0.lcssa
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="true" }

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

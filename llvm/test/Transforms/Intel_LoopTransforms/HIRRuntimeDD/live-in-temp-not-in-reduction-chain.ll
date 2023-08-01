; Avoid RTDD by checking the temp %q.020 which is a live-in temp and it is not in a safe-reduction chain,
; because RTDD will not help vectorization / parallelization in this case and should be avoided.
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
;*** IR Dump Before HIR RuntimeDD Multiversioning (hir-runtime-dd) ***
;Function: sub
;
;<0>          BEGIN REGION { }
;<20>               + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;<3>                |   %0 = (%B)[i1];
;<4>                |   %add = %0  +  %q.020;
;<6>                |   (%C)[i1] = %add;
;<8>                |   %mul = i1  *  i1;
;<9>                |   %conv = sitofp.i32.float(%mul);
;<10>               |   %mul3 = %conv  *  %conv;
;<12>               |   (%A)[i1] = %mul3;
;<15>               |   %q.020 = %conv;
;<20>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR RuntimeDD Multiversioning (hir-runtime-dd) ***
;Function: sub
;
; CHECK:       BEGIN REGION { }
; CHECK-NEXT:      + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK-NEXT:      |   %0 = (%B)[i1];
; CHECK-NEXT:      |   %add = %0  +  %q.020;
; CHECK-NEXT:      |   (%C)[i1] = %add;
; CHECK-NEXT:      |   %mul = i1  *  i1;
; CHECK-NEXT:      |   %conv = sitofp.i32.float(%mul);
; CHECK-NEXT:      |   %mul3 = %conv  *  %conv;
; CHECK-NEXT:      |   (%A)[i1] = %mul3;
; CHECK-NEXT:      |   %q.020 = %conv;
; CHECK-NEXT:      + END LOOP
; CHECK-NEXT:  END REGION
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local float @sub(ptr nocapture %A, ptr nocapture readonly %B, ptr nocapture %C, i32 %N) local_unnamed_addr #0 {
entry:
  %cmp18 = icmp sgt i32 %N, 0
  br i1 %cmp18, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count21 = zext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %q.020 = phi float [ 0.000000e+00, %for.body.preheader ], [ %conv, %for.body ]
  %arrayidx = getelementptr inbounds float, ptr %B, i64 %indvars.iv
  %0 = load float, ptr %arrayidx, align 4, !tbaa !3
  %add = fadd fast float %0, %q.020
  %arrayidx2 = getelementptr inbounds float, ptr %C, i64 %indvars.iv
  store float %add, ptr %arrayidx2, align 4, !tbaa !3
  %1 = trunc i64 %indvars.iv to i32
  %mul = mul nsw i32 %1, %1
  %conv = sitofp i32 %mul to float
  %mul3 = fmul fast float %conv, %conv
  %arrayidx5 = getelementptr inbounds float, ptr %A, i64 %indvars.iv
  store float %mul3, ptr %arrayidx5, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count21
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !7

for.end.loopexit:                                 ; preds = %for.body
  %conv.lcssa = phi float [ %conv, %for.body ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %q.0.lcssa = phi float [ 0.000000e+00, %entry ], [ %conv.lcssa, %for.end.loopexit ]
  ret float %q.0.lcssa
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

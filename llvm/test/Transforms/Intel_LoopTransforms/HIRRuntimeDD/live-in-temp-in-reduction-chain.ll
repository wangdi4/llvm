; Test RTDD gets triggered when live-in temp %q.017 is in the reduction chain.
;
; RUN: opt -hir-ssa-deconstruction -hir-runtime-dd -print-after=hir-runtime-dd -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
;*** IR Dump Before HIR RuntimeDD Multiversioning (hir-runtime-dd) ***
;Function: sub
;
;<0>          BEGIN REGION { }
;<15>               + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>   <LEGAL_MAX_TC = 2147483647>
;<3>                |   %0 = (%B)[i1];
;<5>                |   (%B)[i1] = %0 + 1;
;<8>                |   %q.017 = (%A)[i1]  +  %q.017;
;<15>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR RuntimeDD Multiversioning (hir-runtime-dd) ***
;Function: sub
;
; CHECK:    BEGIN REGION { }
; CHECK:           %mv.test = &((%B)[zext.i32.i64(%N) + -1]) >=u &((%A)[0]);
; CHECK:           %mv.test1 = &((%A)[zext.i32.i64(%N) + -1]) >=u &((%B)[0]);
; CHECK:           %mv.and = %mv.test  &  %mv.test1;                         
; CHECK:           if (%mv.and == 0)                                         
; CHECK:           {                                                         
; CHECK:              + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>   <LEGAL_MAX_TC = 2147483647>  <MVTag: 15>
; CHECK:              |   %0 = (%B)[i1];                                                                       
; CHECK:              |   (%B)[i1] = %0 + 1;                                                                   
; CHECK:              |   %1 = (%A)[i1];                                                                       
; CHECK:              |   %q.017 = %1  +  %q.017;                                                              
; CHECK:              + END LOOP                                                                               
; CHECK:           }                                                                                           
; CHECK:           else                                                                                        
; CHECK:           {                                                                                           
; CHECK:              + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>   <LEGAL_MAX_TC = 2147483647>  <MVTag: 15> <nounroll> <novectorize>                                                                                              
; CHECK:              |   %0 = (%B)[i1];                                                                           
; CHECK:              |   (%B)[i1] = %0 + 1;                                                                       
; CHECK:              |   %1 = (%A)[i1];                                                                           
; CHECK:              |   %q.017 = %1  +  %q.017;                                                                  
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
define dso_local i32 @sub(i32* nocapture readonly %A, i32* nocapture %B, i32 %N) local_unnamed_addr #0 {
entry:
  %cmp15 = icmp sgt i32 %N, 0
  br i1 %cmp15, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count18 = zext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %q.017 = phi i32 [ 0, %for.body.preheader ], [ %add5, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %add = add nsw i32 %0, 1
  store i32 %add, i32* %arrayidx, align 4, !tbaa !3
  %arrayidx4 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx4, align 4, !tbaa !3
  %add5 = add nsw i32 %1, %q.017
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count18
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !7

for.end.loopexit:                                 ; preds = %for.body
  %add5.lcssa = phi i32 [ %add5, %for.body ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %q.0.lcssa = phi i32 [ 0, %entry ], [ %add5.lcssa, %for.end.loopexit ]
  ret i32 %q.0.lcssa
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}

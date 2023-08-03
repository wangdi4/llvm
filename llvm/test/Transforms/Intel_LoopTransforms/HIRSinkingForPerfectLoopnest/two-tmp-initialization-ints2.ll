; Verify that the test allows multiple pre-loop copy inst candidates.
; In this test case, there are two pre-loop copy insts %4 = 0 and %5 = 0.
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Sinking For Perfect Loopnest (hir-sinking-for-perfect-loopnest) ***
;Function: matrix_mul_matrix
;
;<0>          BEGIN REGION { }
;<53>               + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;<54>               |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;<11>               |   |   %4 = 0;
;<12>               |   |   %5 = 0;
;<55>               |   |   
;<55>               |   |   + DO i3 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;<17>               |   |   |   %7 = (%A)[zext.i32.i64(%N) * i1 + i3];
;<21>               |   |   |   %10 = (%B)[i2 + zext.i32.i64(%N) * i3];
;<23>               |   |   |   %5 = %5  +  (%10 * %7);
;<26>               |   |   |   %12 = (%A)[zext.i32.i64(%N) * i2 + i3];
;<28>               |   |   |   %13 = (%B)[zext.i32.i64(%N) * i1 + i3];
;<30>               |   |   |   %4 = %4  +  (%12 * %13);
;<55>               |   |   + END LOOP
;<55>               |   |   
;<38>               |   |   (%C)[zext.i32.i64(%N) * i1 + i2] = %5;
;<39>               |   |   (%D)[zext.i32.i64(%N) * i1 + i2] = %4;
;<54>               |   + END LOOP
;<53>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Sinking For Perfect Loopnest (hir-sinking-for-perfect-loopnest) ***
;Function: matrix_mul_matrix
;
; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:           |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:           |   |   (%C)[zext.i32.i64(%N) * i1 + i2] = 0;
; CHECK:           |   |   (%D)[zext.i32.i64(%N) * i1 + i2] = 0;
; CHECK:           |   |   
; CHECK:           |   |   + DO i3 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:           |   |   |   %4 = (%D)[zext.i32.i64(%N) * i1 + i2];
; CHECK:           |   |   |   %5 = (%C)[zext.i32.i64(%N) * i1 + i2];
; CHECK:           |   |   |   %7 = (%A)[zext.i32.i64(%N) * i1 + i3];
; CHECK:           |   |   |   %10 = (%B)[i2 + zext.i32.i64(%N) * i3];
; CHECK:           |   |   |   %5 = %5  +  (%10 * %7);
; CHECK:           |   |   |   %12 = (%A)[zext.i32.i64(%N) * i2 + i3];
; CHECK:           |   |   |   %13 = (%B)[zext.i32.i64(%N) * i1 + i3];
; CHECK:           |   |   |   %4 = %4  +  (%13 * %12);
; CHECK:           |   |   |   (%C)[zext.i32.i64(%N) * i1 + i2] = %5;
; CHECK:           |   |   |   (%D)[zext.i32.i64(%N) * i1 + i2] = %4;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION
;
; Test undo sinking pass can hoist out the load insts and stores insts when loop interchange
; was not triggered.
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-distribute-loopnest,hir-loop-interchange,hir-undo-sinking-for-perfect-loopnest,print<hir>,print<hir>,print<hir>,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s --check-prefix=CHECK-UNDO
; 
;*** IR Dump After HIR Sinking For Perfect Loopnest (hir-sinking-for-perfect-loopnest) ***
;Function: matrix_mul_matrix
;
;<0>          BEGIN REGION { }
;<53>               + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;<54>               |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;<56>               |   |   (%C)[zext.i32.i64(%N) * i1 + i2] = 0;
;<58>               |   |   (%D)[zext.i32.i64(%N) * i1 + i2] = 0;
;<55>               |   |   
;<55>               |   |   + DO i3 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;<59>               |   |   |   %4 = (%D)[zext.i32.i64(%N) * i1 + i2];
;<57>               |   |   |   %5 = (%C)[zext.i32.i64(%N) * i1 + i2];
;<17>               |   |   |   %7 = (%A)[zext.i32.i64(%N) * i1 + i3];
;<21>               |   |   |   %10 = (%B)[i2 + zext.i32.i64(%N) * i3];
;<23>               |   |   |   %5 = %5  +  (%10 * %7);
;<26>               |   |   |   %12 = (%A)[zext.i32.i64(%N) * i2 + i3];
;<28>               |   |   |   %13 = (%B)[zext.i32.i64(%N) * i1 + i3];
;<30>               |   |   |   %4 = %4  +  (%13 * %12);
;<38>               |   |   |   (%C)[zext.i32.i64(%N) * i1 + i2] = %5;
;<39>               |   |   |   (%D)[zext.i32.i64(%N) * i1 + i2] = %4;
;<55>               |   |   + END LOOP
;<54>               |   + END LOOP
;<53>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Loop Distribution LoopNest (hir-loop-distribute-loopnest) ***
;Function: matrix_mul_matrix
;
;<0>          BEGIN REGION { }
;<62>               + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;<60>               |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;<56>               |   |   (%C)[zext.i32.i64(%N) * i1 + i2] = 0;
;<58>               |   |   (%D)[zext.i32.i64(%N) * i1 + i2] = 0;
;<60>               |   + END LOOP
;<62>               + END LOOP
;<62>               
;<63>               
;<63>               + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;<61>               |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;<55>               |   |   + DO i3 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;<59>               |   |   |   %4 = (%D)[zext.i32.i64(%N) * i1 + i2];
;<57>               |   |   |   %5 = (%C)[zext.i32.i64(%N) * i1 + i2];
;<17>               |   |   |   %7 = (%A)[zext.i32.i64(%N) * i1 + i3];
;<21>               |   |   |   %10 = (%B)[i2 + zext.i32.i64(%N) * i3];
;<23>               |   |   |   %5 = %5  +  (%10 * %7);
;<26>               |   |   |   %12 = (%A)[zext.i32.i64(%N) * i2 + i3];
;<28>               |   |   |   %13 = (%B)[zext.i32.i64(%N) * i1 + i3];
;<30>               |   |   |   %4 = %4  +  (%13 * %12);
;<38>               |   |   |   (%C)[zext.i32.i64(%N) * i1 + i2] = %5;
;<39>               |   |   |   (%D)[zext.i32.i64(%N) * i1 + i2] = %4;
;<55>               |   |   + END LOOP
;<61>               |   + END LOOP
;<63>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Loop Interchange (hir-loop-interchange) ***
;Function: matrix_mul_matrix
;
;<0>          BEGIN REGION { }
;<62>               + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;<60>               |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;<56>               |   |   (%C)[zext.i32.i64(%N) * i1 + i2] = 0;
;<58>               |   |   (%D)[zext.i32.i64(%N) * i1 + i2] = 0;
;<60>               |   + END LOOP
;<62>               + END LOOP
;<62>               
;<63>               
;<63>               + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;<61>               |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;<55>               |   |   + DO i3 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;<59>               |   |   |   %4 = (%D)[zext.i32.i64(%N) * i1 + i2];
;<57>               |   |   |   %5 = (%C)[zext.i32.i64(%N) * i1 + i2];
;<17>               |   |   |   %7 = (%A)[zext.i32.i64(%N) * i1 + i3];
;<21>               |   |   |   %10 = (%B)[i2 + zext.i32.i64(%N) * i3];
;<23>               |   |   |   %5 = %5  +  (%10 * %7);
;<26>               |   |   |   %12 = (%A)[zext.i32.i64(%N) * i2 + i3];
;<28>               |   |   |   %13 = (%B)[zext.i32.i64(%N) * i1 + i3];
;<30>               |   |   |   %4 = %4  +  (%13 * %12);
;<38>               |   |   |   (%C)[zext.i32.i64(%N) * i1 + i2] = %5;
;<39>               |   |   |   (%D)[zext.i32.i64(%N) * i1 + i2] = %4;
;<55>               |   |   + END LOOP
;<61>               |   + END LOOP
;<63>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Undo Sinking For Perfect Loopnest (hir-undo-sinking-for-perfect-loopnest) ***
;Function: matrix_mul_matrix
;
; CHECK-UNDO:    BEGIN REGION { }
; CHECK-UNDO:       + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK-UNDO:      |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK-UNDO:      |   |   (%C)[zext.i32.i64(%N) * i1 + i2] = 0;
; CHECK-UNDO:      |   |   (%D)[zext.i32.i64(%N) * i1 + i2] = 0;
; CHECK-UNDO:      |   + END LOOP
; CHECK-UNDO:      + END LOOP
;      
;               
; CHECK-UNDO:      + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK-UNDO:      |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK-UNDO:      |   |      %4 = 0;
; CHECK-UNDO:      |   |      %5 = 0;
; CHECK-UNDO:      |   |   + DO i3 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK-UNDO:      |   |   |   %7 = (%A)[zext.i32.i64(%N) * i1 + i3];
; CHECK-UNDO:      |   |   |   %10 = (%B)[i2 + zext.i32.i64(%N) * i3];
; CHECK-UNDO:      |   |   |   %5 = %5  +  (%10 * %7);
; CHECK-UNDO:      |   |   |   %12 = (%A)[zext.i32.i64(%N) * i2 + i3];
; CHECK-UNDO:      |   |   |   %13 = (%B)[zext.i32.i64(%N) * i1 + i3];
; CHECK-UNDO:      |   |   |   %4 = %4  +  (%13 * %12);
; CHECK-UNDO:      |   |   + END LOOP
; CHECK-UNDO:      |   |      (%C)[zext.i32.i64(%N) * i1 + i2] = %5;
; CHECK-UNDO:      |   |      (%D)[zext.i32.i64(%N) * i1 + i2] = %4;
; CHECK-UNDO:      |   + END LOOP
; CHECK-UNDO:      + END LOOP
; CHECK-UNDO:    END REGION
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @matrix_mul_matrix(i32 %N, ptr noalias nocapture %C, ptr noalias nocapture readonly %A, ptr noalias nocapture readonly %B, ptr noalias nocapture %D) local_unnamed_addr #0 {
entry:
  %cmp981 = icmp sgt i32 %N, 0
  br i1 %cmp981, label %for.cond1.preheader.preheader, label %for.end44

for.cond1.preheader.preheader:                    ; preds = %entry
  %0 = zext i32 %N to i64
  br label %for.body3.preheader

for.body3.preheader:                              ; preds = %for.inc42, %for.cond1.preheader.preheader
  %indvars.iv102 = phi i64 [ 0, %for.cond1.preheader.preheader ], [ %indvars.iv.next103, %for.inc42 ]
  %1 = mul nsw i64 %indvars.iv102, %0
  br label %for.body10.lr.ph

for.body10.lr.ph:                                 ; preds = %for.inc39, %for.body3.preheader
  %indvars.iv95 = phi i64 [ 0, %for.body3.preheader ], [ %indvars.iv.next96, %for.inc39 ]
  %2 = add nuw nsw i64 %indvars.iv95, %1
  %arrayidx = getelementptr inbounds i32, ptr %C, i64 %2
  %arrayidx7 = getelementptr inbounds i32, ptr %D, i64 %2
  %3 = mul nsw i64 %indvars.iv95, %0
  br label %for.body10

for.body10:                                       ; preds = %for.body10.lr.ph, %for.body10
  %indvars.iv = phi i64 [ 0, %for.body10.lr.ph ], [ %indvars.iv.next, %for.body10 ]
  %4 = phi i32 [ 0, %for.body10.lr.ph ], [ %add38, %for.body10 ]
  %5 = phi i32 [ 0, %for.body10.lr.ph ], [ %add24, %for.body10 ]
  %6 = add nuw nsw i64 %indvars.iv, %1
  %arrayidx14 = getelementptr inbounds i32, ptr %A, i64 %6
  %7 = load i32, ptr %arrayidx14, align 4, !tbaa !3
  %8 = mul nsw i64 %indvars.iv, %0
  %9 = add nuw nsw i64 %8, %indvars.iv95
  %arrayidx18 = getelementptr inbounds i32, ptr %B, i64 %9
  %10 = load i32, ptr %arrayidx18, align 4, !tbaa !3
  %mul19 = mul nsw i32 %10, %7
  %add24 = add nsw i32 %5, %mul19
  %11 = add nuw nsw i64 %indvars.iv, %3
  %arrayidx28 = getelementptr inbounds i32, ptr %A, i64 %11
  %12 = load i32, ptr %arrayidx28, align 4, !tbaa !3
  %arrayidx32 = getelementptr inbounds i32, ptr %B, i64 %6
  %13 = load i32, ptr %arrayidx32, align 4, !tbaa !3
  %mul33 = mul nsw i32 %13, %12
  %add38 = add nsw i32 %4, %mul33
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %0
  br i1 %exitcond.not, label %for.inc39, label %for.body10, !llvm.loop !7

for.inc39:                                        ; preds = %for.body10
  %add24.lcssa = phi i32 [ %add24, %for.body10 ]
  %add38.lcssa = phi i32 [ %add38, %for.body10 ]
  store i32 %add24.lcssa, ptr %arrayidx, align 4, !tbaa !3
  store i32 %add38.lcssa, ptr %arrayidx7, align 4, !tbaa !3
  %indvars.iv.next96 = add nuw nsw i64 %indvars.iv95, 1
  %exitcond101.not = icmp eq i64 %indvars.iv.next96, %0
  br i1 %exitcond101.not, label %for.inc42, label %for.body10.lr.ph, !llvm.loop !9

for.inc42:                                        ; preds = %for.inc39
  %indvars.iv.next103 = add nuw nsw i64 %indvars.iv102, 1
  %exitcond107.not = icmp eq i64 %indvars.iv.next103, %0
  br i1 %exitcond107.not, label %for.end44.loopexit, label %for.body3.preheader, !llvm.loop !10

for.end44.loopexit:                               ; preds = %for.inc42
  br label %for.end44

for.end44:                                        ; preds = %for.end44.loopexit, %entry
  ret void
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="true" }

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
!9 = distinct !{!9, !8}
!10 = distinct !{!10, !8}

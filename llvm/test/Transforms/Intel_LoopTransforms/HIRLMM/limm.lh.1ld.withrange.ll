; RUN: opt -passes="hir-ssa-deconstruction,hir-lmm,print<hir>" -hir-details -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s
;
;[Notes]
; - Although there is a !range metadata attached to the load of B[0][0], LIMM still triggers on the store of B[0][0]
;   since the pass is now instead using the first store ref (instead of first ref) to replicate for sinking.
;


;*** IR Dump Before HIR Loop Memory Motion (hir-lmm) ***
;Function: foo

;<0>          BEGIN REGION { }
;<27>               + DO i1 = 0, 99, 1   <DO_LOOP>
;<28>               |   + DO i2 = 0, 99, 1   <DO_LOOP>
;<5>                |   |   %0 = (@B)[0][0];
;<7>                |   |   %1 = (@A)[0][i1][i2];
;<9>                |   |   (@A)[0][i1][i2] = %0 + %1;
;<11>               |   |   %2 = (@A)[0][i2][i1];
;<13>               |   |   (@B)[0][0] = %2 + 1;
;<28>               |   + END LOOP
;<27>               + END LOOP
;<0>          END REGION

;*** IR Dump After HIR Loop Memory Motion (hir-lmm) ***
;Function: foo

;<0>          BEGIN REGION { modified }
;<27>               + DO i1 = 0, 99, 1   <DO_LOOP>
;<29>               |      %limm = (@B)[0][0];
;<28>               |   + DO i2 = 0, 99, 1   <DO_LOOP>
;<31>               |   |   %0 = %limm;
;<7>                |   |   %1 = (@A)[0][i1][i2];
;<9>                |   |   (@A)[0][i1][i2] = %0 + %1;
;<11>               |   |   %2 = (@A)[0][i2][i1];
;<32>               |   |   %limm = %2 + 1;
;<28>               |   + END LOOP
;<30>               |      (@B)[0][0] = %limm;
;<27>               + END LOOP
;<0>          END REGION

;*** IR Dump After HIR Loop Memory Motion (hir-lmm) ***
;Function: foo

; CHECK:     BEGIN REGION { modified }
;<27>               + Ztt: No
;<27>               + NumExits: 1
;<27>               + Innermost: No
;<27>               + HasSignedIV: Yes
;<27>               + LiveIn symbases: 5, 8
;<27>               + LiveOut symbases:
;<27>               + Loop metadata: !llvm.loop !5
;<27>               + DO i64 i1 = 0, 99, 1   <DO_LOOP>
; CHECK:            |      %limm = (@B)[0][0];
; CHECK:            |      <LVAL-REG> NON-LINEAR i32 %limm {sb:20}
; CHECK:            |      <RVAL-REG> {al:16}(LINEAR ptr @B)[i64 0][i64 0] inbounds  !tbaa !7 !range !12 {sb:18}
; CHECK:            |         <BLOB> LINEAR ptr @B {sb:5}
;<29>               |
;<28>               |   + Ztt: No
;<28>               |   + NumExits: 1
;<28>               |   + Innermost: Yes
;<28>               |   + HasSignedIV: Yes
;<28>               |   + LiveIn symbases: 5, 8, 20
;<28>               |   + LiveOut symbases: 20
;<28>               |   + Loop metadata: !llvm.loop !15
;<28>               |   + DO i64 i2 = 0, 99, 1   <DO_LOOP>
;<31>               |   |   %0 = %limm;
;<31>               |   |   <LVAL-REG> NON-LINEAR i32 %0 {sb:6}
;<31>               |   |   <RVAL-REG> NON-LINEAR i32 %limm {sb:20}
;<31>               |   |
;<7>                |   |   %1 = (@A)[0][i1][i2];
;<7>                |   |   <LVAL-REG> NON-LINEAR i32 %1 {sb:9}
;<7>                |   |   <RVAL-REG> {al:4}(LINEAR ptr @A)[i64 0][LINEAR i64 i1][LINEAR i64 i2] inbounds  !tbaa !13 {sb:19}
;<7>                |   |      <BLOB> LINEAR ptr @A {sb:8}
;<7>                |   |
;<9>                |   |   (@A)[0][i1][i2] = %0 + %1;
;<9>                |   |   <LVAL-REG> {al:4}(LINEAR ptr @A)[i64 0][LINEAR i64 i1][LINEAR i64 i2] inbounds  !tbaa !13 {sb:19}
;<9>                |   |      <BLOB> LINEAR ptr @A {sb:8}
;<9>                |   |   <RVAL-REG> NON-LINEAR i32 %0 + %1 {sb:2}
;<9>                |   |      <BLOB> NON-LINEAR i32 %0 {sb:6}
;<9>                |   |      <BLOB> NON-LINEAR i32 %1 {sb:9}
;<9>                |   |
;<11>               |   |   %2 = (@A)[0][i2][i1];
;<11>               |   |   <LVAL-REG> NON-LINEAR i32 %2 {sb:12}
;<11>               |   |   <RVAL-REG> {al:4}(LINEAR ptr @A)[i64 0][LINEAR i64 i2][LINEAR i64 i1] inbounds  !tbaa !13 {sb:19}
;<11>               |   |      <BLOB> LINEAR ptr @A {sb:8}
;<11>               |   |
;<32>               |   |   %limm = %2 + 1;
;<32>               |   |   <LVAL-REG> NON-LINEAR i32 %limm {sb:20}
;<32>               |   |   <RVAL-REG> NON-LINEAR i32 %2 + 1 {sb:2}
;<32>               |   |      <BLOB> NON-LINEAR i32 %2 {sb:12}
;<32>               |   |
;<28>               |   + END LOOP
; CHECK:            |      (@B)[0][0] = %limm;
; CHECK:            |      <LVAL-REG> {al:16}(LINEAR ptr @B)[i64 0][i64 0] inbounds  !tbaa !7 {sb:18}
; CHECK:            |         <BLOB> LINEAR ptr @B {sb:5}
; CHECK:            |      <RVAL-REG> NON-LINEAR i32 %limm {sb:20}
;<30>               |
;<27>               + END LOOP
;<0>         END REGION



target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = dso_local global [100 x i32] zeroinitializer, align 16
@A = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.cond.cleanup3
  %indvars.iv31 = phi i64 [ 0, %entry ], [ %indvars.iv.next32, %for.cond.cleanup3 ]
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond.cleanup3:                                ; preds = %for.body4
  %indvars.iv.next32 = add nuw nsw i64 %indvars.iv31, 1
  %exitcond33.not = icmp eq i64 %indvars.iv.next32, 100
  br i1 %exitcond33.not, label %for.cond.cleanup, label %for.cond1.preheader, !llvm.loop !3

for.body4:                                        ; preds = %for.cond1.preheader, %for.body4
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %0 = load   i32, ptr getelementptr inbounds ([100 x i32], ptr @B, i64 0, i64 0), align 16, !range !13,!tbaa !5
  %arrayidx6 = getelementptr inbounds [100 x [100 x i32]], ptr @A, i64 0, i64 %indvars.iv31, i64 %indvars.iv, !intel-tbaa !10
  %1 = load i32, ptr %arrayidx6, align 4, !tbaa !10
  %add = add nsw i32 %1, %0
  store i32 %add, ptr %arrayidx6, align 4, !tbaa !10
  %arrayidx14 = getelementptr inbounds [100 x [100 x i32]], ptr @A, i64 0, i64 %indvars.iv, i64 %indvars.iv31, !intel-tbaa !10
  %2 = load i32, ptr %arrayidx14, align 4, !tbaa !10
  %add15 = add nsw i32 %2, 1
  store   i32 %add15, ptr getelementptr inbounds ([100 x i32], ptr @B, i64 0, i64 0), align 16, !tbaa !5
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.cond.cleanup3, label %for.body4, !llvm.loop !12
}

attributes #0 = { nofree norecurse nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = distinct !{!3, !4}
!4 = !{!"llvm.loop.mustprogress"}
!5 = !{!6, !7, i64 0}
!6 = !{!"array@_ZTSA100_i", !7, i64 0}
!7 = !{!"int", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!11, !7, i64 0}
!11 = !{!"array@_ZTSA100_A100_i", !6, i64 0}
!12 = distinct !{!12, !4}
!13 = !{ i32 0, i32 2 }

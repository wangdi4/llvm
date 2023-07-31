; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-undo-sinking-for-perfect-loopnest,print<hir>" -hir-details -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
; This test checks updating the blob definition levels after undo sinking for perfect loopnest
;*** IR Dump Before HIR Sinking For Perfect Loopnest ***
;Function: foo
;
;<0>          BEGIN REGION { }
;<23>               + DO i64 i1 = 0, 99, 1   <DO_LOOP>
;<3>                |   %0 = (@B)[0][i1];
;<3>                |   <LVAL-REG> NON-LINEAR i8 %0
;<3>                |   <RVAL-REG> {al:1}(LINEAR [100 x i8]* @B)[i64 0][LINEAR i64 i1] inbounds  !tbaa !3
;<3>                |      <BLOB> LINEAR [100 x i8]* @B
;<3>                |
;<24>               |   + DO i64 i2 = 0, 99, 1   <DO_LOOP>
;<9>                |   |   (@A)[0][i1][i2] = %0;
;<9>                |   |   <LVAL-REG> {al:4}(LINEAR [100 x [100 x i32]]* @A)[i64 0][LINEAR i64 i1][LINEAR i64 i2] inbounds  !tbaa !10
;<9>                |   |      <BLOB> LINEAR [100 x [100 x i32]]* @A
;<9>                |   |   <RVAL-REG> LINEAR sext.i8.i32(%0){def@1}
;<9>                |   |      <BLOB> LINEAR i8 %0{def@1}
;<9>                |   |
;<24>               |   + END LOOP
;<23>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Sinking For Perfect Loopnest ***
;Function: foo
;
;<0>          BEGIN REGION { }
;<23>               + DO i64 i1 = 0, 99, 1   <DO_LOOP>
;<24>               |   + DO i64 i2 = 0, 99, 1   <DO_LOOP>
;<3>                |   |   %0 = (@B)[0][i1];
;<3>                |   |   <LVAL-REG> NON-LINEAR i8 %0
;<3>                |   |   <RVAL-REG> {al:1}(LINEAR [100 x i8]* @B)[i64 0][LINEAR i64 i1] inbounds  !tbaa !3
;<3>                |   |      <BLOB> LINEAR [100 x i8]* @B
;<3>                |   |
;<9>                |   |   (@A)[0][i1][i2] = %0;
;<9>                |   |   <LVAL-REG> {al:4}(LINEAR [100 x [100 x i32]]* @A)[i64 0][LINEAR i64 i1][LINEAR i64 i2] inbounds  !tbaa !10
;<9>                |   |      <BLOB> LINEAR [100 x [100 x i32]]* @A
;<9>                |   |   <RVAL-REG> NON-LINEAR sext.i8.i32(%0)
;<9>                |   |      <BLOB> NON-LINEAR i8 %0
;<9>                |   |
;<24>               |   + END LOOP
;<23>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Undo Sinking For Perfect Loopnest ***
;Function: foo
;
; CHECK:     BEGIN REGION { }
; CHECK:           + DO i64 i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |      %0 = (@B)[0][i1];
; CHECK:           |      <LVAL-REG> NON-LINEAR i8 %0
; CHECK:           |      <RVAL-REG> {al:1}(LINEAR ptr @B)[i64 0][LINEAR i64 i1] inbounds  !tbaa !3
; CHECK:           |         <BLOB> LINEAR ptr @B
;                  |
; CHECK:           |   + DO i64 i2 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   (@A)[0][i1][i2] = %0;
; CHECK:           |   |   <LVAL-REG> {al:4}(LINEAR ptr @A)[i64 0][LINEAR i64 i1][LINEAR i64 i2] inbounds  !tbaa !10
; CHECK:           |   |      <BLOB> LINEAR ptr @A
; CHECK:           |   |   <RVAL-REG> LINEAR sext.i8.i32(%0){def@1}
; CHECK:           |   |      <BLOB> LINEAR i8 %0{def@1}
;                  |   |
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = dso_local local_unnamed_addr global [100 x i8] zeroinitializer, align 16
@A = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo(ptr nocapture readnone %a) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.body:                                         ; preds = %entry, %for.cond.cleanup3
  %indvars.iv21 = phi i64 [ 0, %entry ], [ %indvars.iv.next22, %for.cond.cleanup3 ]
  %arrayidx = getelementptr inbounds [100 x i8], ptr @B, i64 0, i64 %indvars.iv21, !intel-tbaa !2
  %0 = load i8, ptr %arrayidx, align 1, !tbaa !2
  %conv = sext i8 %0 to i32
  br label %for.body4

for.cond.cleanup3:                                ; preds = %for.body4
  %indvars.iv.next22 = add nuw nsw i64 %indvars.iv21, 1
  %exitcond23.not = icmp eq i64 %indvars.iv.next22, 100
  br i1 %exitcond23.not, label %for.cond.cleanup, label %for.body, !llvm.loop !6

for.body4:                                        ; preds = %for.body, %for.body4
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body4 ]
  %arrayidx8 = getelementptr inbounds [100 x [100 x i32]], ptr @A, i64 0, i64 %indvars.iv21, i64 %indvars.iv, !intel-tbaa !8
  store i32 %conv, ptr %arrayidx8, align 4, !tbaa !8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.cond.cleanup3, label %for.body4, !llvm.loop !12
}

attributes #0 = { nofree norecurse nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.2.0 (YYYY.x.0.MMDD)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_c", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
!8 = !{!9, !11, i64 0}
!9 = !{!"array@_ZTSA100_A100_i", !10, i64 0}
!10 = !{!"array@_ZTSA100_i", !11, i64 0}
!11 = !{!"int", !4, i64 0}
!12 = distinct !{!12, !7}

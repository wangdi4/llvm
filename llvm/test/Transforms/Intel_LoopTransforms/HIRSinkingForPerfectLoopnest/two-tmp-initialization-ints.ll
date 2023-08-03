; Verify this test case allows multiple TmpInitalizationInst to be existed.
;Source code:
;int a[100][100];
;int b[100][100];
;int c[100][100];
;int d[100][100];
;void multiply(int M, int N, int L) {
;  for (int i = 0; i < M; i++) {
;    for (int j = 0; j < N; j++) {
;      int t = 0;
;      d[i][j] = 0;
;      for (int k = 0; k < L; k++) {
;        t += a[i][k] * b[k][j];
;        d[i][j] += a[i][k] * b[k][j];
;        c[i][j] = t;
;      }
;    }
;  }
;}
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Sinking For Perfect Loopnest (hir-sinking-for-perfect-loopnest) ***
;Function: multiply
;
;<0>          BEGIN REGION { }
;<54>               + DO i1 = 0, zext.i32.i64(%M) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
;<55>               |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
;<10>               |   |   (@d)[0][i1][i2] = 0;
;<56>               |   |   
;<17>               |   |      %add3268 = 0;
;<18>               |   |      %t.066 = 0;
;<56>               |   |   + DO i3 = 0, zext.i32.i64(%L) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
;<22>               |   |   |   %0 = (@a)[0][i1][i3];
;<24>               |   |   |   %1 = (@b)[0][i3][i2];
;<26>               |   |   |   %t.066 = (%1 * %0)  +  %t.066;
;<27>               |   |   |   %add3268 = %add3268  +  (%1 * %0);
;<56>               |   |   + END LOOP
;<35>               |   |      (@d)[0][i1][i2] = %add3268;
;<36>               |   |      (@c)[0][i1][i2] = %t.066;
;<55>               |   + END LOOP
;<54>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Sinking For Perfect Loopnest (hir-sinking-for-perfect-loopnest) ***
;Function: multiply
;
; CHECK:    BEGIN REGION { }
; CHECK:           + DO i1 = 0, zext.i32.i64(%M) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
; CHECK:           |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
; CHECK:           |   |   (@d)[0][i1][i2] = 0;
; CHECK:           |   |   (@c)[0][i1][i2] = 0;
; CHECK:           |   |   
; CHECK:           |   |   + DO i3 = 0, zext.i32.i64(%L) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
; CHECK:           |   |   |   %add3268 = (@d)[0][i1][i2];
; CHECK:           |   |   |   %t.066 = (@c)[0][i1][i2];
; CHECK:           |   |   |   %0 = (@a)[0][i1][i3];
; CHECK:           |   |   |   %1 = (@b)[0][i3][i2];
; CHECK:           |   |   |   %t.066 = (%1 * %0)  +  %t.066;
; CHECK:           |   |   |   %add3268 = %add3268  +  (%1 * %0);
; CHECK:           |   |   |   (@d)[0][i1][i2] = %add3268;
; CHECK:           |   |   |   (@c)[0][i1][i2] = %t.066;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@d = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@a = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@c = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @multiply(i32 %M, i32 %N, i32 %L) local_unnamed_addr #0 {
entry:
  %cmp865 = icmp sgt i32 %L, 0
  %cmp269 = icmp sgt i32 %N, 0
  %cmp71 = icmp sgt i32 %M, 0
  br i1 %cmp71, label %for.cond1.preheader.preheader, label %for.cond.cleanup

for.cond1.preheader.preheader:                    ; preds = %entry
  %wide.trip.count8082 = zext i32 %M to i64
  %wide.trip.count7683 = zext i32 %N to i64
  %wide.trip.count84 = zext i32 %L to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.cond.cleanup3
  %indvars.iv78 = phi i64 [ 0, %for.cond1.preheader.preheader ], [ %indvars.iv.next79, %for.cond.cleanup3 ]
  br i1 %cmp269, label %for.body4.preheader, label %for.cond.cleanup3

for.body4.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.cond.cleanup3.loopexit:                       ; preds = %for.cond.cleanup9
  br label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond.cleanup3.loopexit, %for.cond1.preheader
  %indvars.iv.next79 = add nuw nsw i64 %indvars.iv78, 1
  %exitcond81.not = icmp eq i64 %indvars.iv.next79, %wide.trip.count8082
  br i1 %exitcond81.not, label %for.cond.cleanup.loopexit, label %for.cond1.preheader, !llvm.loop !3

for.body4:                                        ; preds = %for.body4.preheader, %for.cond.cleanup9
  %indvars.iv74 = phi i64 [ %indvars.iv.next75, %for.cond.cleanup9 ], [ 0, %for.body4.preheader ]
  %arrayidx6 = getelementptr inbounds [100 x [100 x i32]], ptr @d, i64 0, i64 %indvars.iv78, i64 %indvars.iv74, !intel-tbaa !5
  store i32 0, ptr %arrayidx6, align 4, !tbaa !5
  %arrayidx36 = getelementptr inbounds [100 x [100 x i32]], ptr @c, i64 0, i64 %indvars.iv78, i64 %indvars.iv74
  br i1 %cmp865, label %for.body10.preheader, label %for.cond.cleanup9

for.body10.preheader:                             ; preds = %for.body4
  br label %for.body10

for.cond7.for.cond.cleanup9_crit_edge:            ; preds = %for.body10
  %add.lcssa = phi i32 [ %add, %for.body10 ]
  %add32.lcssa = phi i32 [ %add32, %for.body10 ]
  store i32 %add32.lcssa, ptr %arrayidx6, align 4, !tbaa !5
  store i32 %add.lcssa, ptr %arrayidx36, align 4, !tbaa !5
  br label %for.cond.cleanup9

for.cond.cleanup9:                                ; preds = %for.cond7.for.cond.cleanup9_crit_edge, %for.body4
  %indvars.iv.next75 = add nuw nsw i64 %indvars.iv74, 1
  %exitcond77.not = icmp eq i64 %indvars.iv.next75, %wide.trip.count7683
  br i1 %exitcond77.not, label %for.cond.cleanup3.loopexit, label %for.body4, !llvm.loop !11

for.body10:                                       ; preds = %for.body10.preheader, %for.body10
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body10 ], [ 0, %for.body10.preheader ]
  %add3268 = phi i32 [ %add32, %for.body10 ], [ 0, %for.body10.preheader ]
  %t.066 = phi i32 [ %add, %for.body10 ], [ 0, %for.body10.preheader ]
  %arrayidx14 = getelementptr inbounds [100 x [100 x i32]], ptr @a, i64 0, i64 %indvars.iv78, i64 %indvars.iv, !intel-tbaa !5
  %0 = load i32, ptr %arrayidx14, align 4, !tbaa !5
  %arrayidx18 = getelementptr inbounds [100 x [100 x i32]], ptr @b, i64 0, i64 %indvars.iv, i64 %indvars.iv74, !intel-tbaa !5
  %1 = load i32, ptr %arrayidx18, align 4, !tbaa !5
  %mul = mul nsw i32 %1, %0
  %add = add nsw i32 %mul, %t.066
  %add32 = add nsw i32 %add3268, %mul
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count84
  br i1 %exitcond.not, label %for.cond7.for.cond.cleanup9_crit_edge, label %for.body10, !llvm.loop !12
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = distinct !{!3, !4}
!4 = !{!"llvm.loop.mustprogress"}
!5 = !{!6, !8, i64 0}
!6 = !{!"array@_ZTSA100_A100_i", !7, i64 0}
!7 = !{!"array@_ZTSA100_i", !8, i64 0}
!8 = !{!"int", !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = distinct !{!11, !4}
!12 = distinct !{!12, !4}

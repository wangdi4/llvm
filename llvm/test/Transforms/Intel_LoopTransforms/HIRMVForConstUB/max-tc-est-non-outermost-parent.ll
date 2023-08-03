; RUN: opt -passes="hir-ssa-deconstruction,hir-mv-const-ub,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Test checks that we do multiversioning for MAC_TC_EST if innermost loop TC is defined on level 1.

;    BEGIN REGION { }
;        + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
;        |   %0 = (@b)[0][i1];
;        |
;        |   + DO i2 = 0, zext.i32.i64(%l) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
;        |   |   + DO i3 = 0, zext.i32.i64(%0) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>  <LEGAL_MAX_TC = 2147483647>
;        |   |   |   (@a)[0][i1][i2][i3] = i1 + i2 + i3;
;        |   |   + END LOOP
;        |   + END LOOP
;        |      %k.050 = smax(0, %0);
;        + END LOOP
;    END REGION

;CHECK:  BEGIN REGION { }
;CHECK:            + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
;CHECK:            |   %0 = (@b)[0][i1];
;CHECK:            |   if (%0 == 3)
;CHECK:            |   {
;CHECK:            |      + DO i2 = 0, zext.i32.i64(%l) + -1, 1   <DO_LOOP>
;CHECK:            |      |   + DO i3 = 0, 2, 1   <DO_LOOP>  <MAX_TC_EST = 3>  <LEGAL_MAX_TC = 2147483647>
;                  |      |   |   (@a)[0][i1][i2][i3] = i1 + i2 + i3;
;CHECK:            |      |   + END LOOP
;CHECK:            |      + END LOOP
;                  |         %k.050 = 3;
;CHECK:            |   }
;CHECK:            |   else
;CHECK:            |   {
;CHECK:            |      + DO i2 = 0, zext.i32.i64(%l) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
;CHECK:            |      |   + DO i3 = 0, zext.i32.i64(%0) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>  <LEGAL_MAX_TC = 2147483647>
;                  |      |   |   (@a)[0][i1][i2][i3] = i1 + i2 + i3;
;CHECK:            |      |   + END LOOP
;CHECK:            |      + END LOOP
;                  |         %k.050 = smax(0, %0);
;CHECK:            |   }
;CHECK:            + END LOOP
;CHECK:      END REGION



target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [100 x [100 x [3 x i32]]] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
define dso_local noundef i32 @_Z3fooiii(i32 noundef %n, i32 noundef %l, i32 noundef %m) local_unnamed_addr #0 {
entry:
  %cmp48 = icmp sgt i32 %n, 0
  br i1 %cmp48, label %for.body.lr.ph, label %for.end19

for.body.lr.ph:                                   ; preds = %entry
  %cmp245 = icmp sgt i32 %l, 0
  %wide.trip.count60 = zext i32 %n to i64
  %wide.trip.count56 = zext i32 %l to i64
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc17
  %indvars.iv58 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next59, %for.inc17 ]
  %k.050 = phi i32 [ undef, %for.body.lr.ph ], [ %k.1.lcssa, %for.inc17 ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @b, i64 0, i64 %indvars.iv58, !intel-tbaa !3
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !3
  br i1 %cmp245, label %for.cond4.preheader.lr.ph, label %for.inc17

for.cond4.preheader.lr.ph:                        ; preds = %for.body
  %cmp543 = icmp sgt i32 %0, 0
  %wide.trip.count = zext i32 %0 to i64
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond4.preheader.lr.ph, %for.inc14
  %indvars.iv53 = phi i64 [ 0, %for.cond4.preheader.lr.ph ], [ %indvars.iv.next54, %for.inc14 ]
  br i1 %cmp543, label %for.body6.lr.ph, label %for.inc14

for.body6.lr.ph:                                  ; preds = %for.cond4.preheader
  %1 = add nuw nsw i64 %indvars.iv58, %indvars.iv53
  br label %for.body6

for.body6:                                        ; preds = %for.body6.lr.ph, %for.body6
  %indvars.iv = phi i64 [ 0, %for.body6.lr.ph ], [ %indvars.iv.next, %for.body6 ]
  %2 = add nuw nsw i64 %1, %indvars.iv
  %arrayidx13 = getelementptr inbounds [100 x [100 x [3 x i32]]], ptr @a, i64 0, i64 %indvars.iv58, i64 %indvars.iv53, i64 %indvars.iv, !intel-tbaa !8
  %3 = trunc i64 %2 to i32
  store i32 %3, ptr %arrayidx13, align 4, !tbaa !8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.inc14.loopexit, label %for.body6, !llvm.loop !12

for.inc14.loopexit:                               ; preds = %for.body6
  br label %for.inc14

for.inc14:                                        ; preds = %for.inc14.loopexit, %for.cond4.preheader
  %indvars.iv.next54 = add nuw nsw i64 %indvars.iv53, 1
  %exitcond57.not = icmp eq i64 %indvars.iv.next54, %wide.trip.count56
  br i1 %exitcond57.not, label %for.inc17.loopexit, label %for.cond4.preheader, !llvm.loop !14

for.inc17.loopexit:                               ; preds = %for.inc14
  %smax = call i32 @llvm.smax.i32(i32 %0, i32 0)
  br label %for.inc17

for.inc17:                                        ; preds = %for.inc17.loopexit, %for.body
  %k.1.lcssa = phi i32 [ %k.050, %for.body ], [ %smax, %for.inc17.loopexit ]
  %indvars.iv.next59 = add nuw nsw i64 %indvars.iv58, 1
  %exitcond61.not = icmp eq i64 %indvars.iv.next59, %wide.trip.count60
  br i1 %exitcond61.not, label %for.end19.loopexit, label %for.body, !llvm.loop !15

for.end19.loopexit:                               ; preds = %for.inc17
  %k.1.lcssa.lcssa = phi i32 [ %k.1.lcssa, %for.inc17 ]
  br label %for.end19

for.end19:                                        ; preds = %for.end19.loopexit, %entry
  %k.0.lcssa = phi i32 [ undef, %entry ], [ %k.1.lcssa.lcssa, %for.end19.loopexit ]
  %sub = sub nsw i32 %n, %k.0.lcssa
  %idxprom20 = sext i32 %sub to i64
  %sub22 = sub nsw i32 %l, %n
  %idxprom23 = sext i32 %sub22 to i64
  %arrayidx25 = getelementptr inbounds [100 x [100 x [3 x i32]]], ptr @a, i64 0, i64 %idxprom20, i64 %idxprom23, i64 2, !intel-tbaa !8
  %4 = load i32, ptr %arrayidx25, align 4, !tbaa !8
  ret i32 %4
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare i32 @llvm.smax.i32(i32, i32) #1

attributes #0 = { mustprogress nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA100_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!9, !5, i64 0}
!9 = !{!"array@_ZTSA100_A100_A3_i", !10, i64 0}
!10 = !{!"array@_ZTSA100_A3_i", !11, i64 0}
!11 = !{!"array@_ZTSA3_i", !5, i64 0}
!12 = distinct !{!12, !13}
!13 = !{!"llvm.loop.mustprogress"}
!14 = distinct !{!14, !13}
!15 = distinct !{!15, !13}

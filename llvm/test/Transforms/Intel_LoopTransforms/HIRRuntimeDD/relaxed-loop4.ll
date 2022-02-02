; Test the case of splitting a ref group from the middle of refs.
;
; RUN: opt -hir-ssa-deconstruction -hir-runtime-dd -print-after=hir-runtime-dd -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
;[RTDD] Loop references:
;        (@vrow)[0][i1][(1 + sext.i32.i64(%other_channels)) * i2]
;        (@vrow)[0][i1][(1 + sext.i32.i64(%other_channels)) * i2 + 1]
;        (@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + i3]
;        (@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + i3 + 1]
;        (%prow)[i1 + sext.i32.i64(%channels) * i2 + i3]
;        (%prow)[i1 + sext.i32.i64(%channels) * i2 + i3]
;Ref groups after split:
; Group 0 contains (2) refs:
;(@vrow)[0][i1][(1 + sext.i32.i64(%other_channels)) * i2]
;(@vrow)[0][i1][(1 + sext.i32.i64(%other_channels)) * i2 + 1]
;Group 1 contains (2) refs:
;(%prow)[i1 + sext.i32.i64(%channels) * i2 + i3]
;(%prow)[i1 + sext.i32.i64(%channels) * i2 + i3]
;Group 2 contains (2) refs:
;(@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + i3]
;(@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + i3 + 1]
;
;*** IR Dump Before HIR RuntimeDD Multiversioning (hir-runtime-dd) ***
;Function: _Z19composite_referenceiiiiPvPhj
;
;<0>          BEGIN REGION { }
;<70>               + DO i1 = 0, zext.i32.i64(%height) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2000>
;<71>               |   + DO i2 = 0, zext.i32.i64(%width) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647> <LEGAL_MAX_TC = 2147483647>
;<14>               |   |   %2 = (@vrow)[0][i1][(1 + sext.i32.i64(%other_channels)) * i2];
;<18>               |   |   %3 = (@vrow)[0][i1][(1 + sext.i32.i64(%other_channels)) * i2 + 1];
;<20>               |   |   if (%channels > 0)
;<20>               |   |   {
;<72>               |   |      + DO i3 = 0, zext.i32.i64(%smax) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>
;<28>               |   |      |   %4 = (@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + i3];
;<34>               |   |      |   %5 = (@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + i3 + 1];
;<38>               |   |      |   %6 = (%prow)[i1 + sext.i32.i64(%channels) * i2 + i3];
;<43>               |   |      |   (%prow)[i1 + sext.i32.i64(%channels) * i2 + i3] = (zext.i8.i32(%3) + zext.i8.i32(%6) + zext.i8.i32(%5) + (zext.i8.i32(%2) * zext.i8.i32(%4)))/u255;
;<72>               |   |      + END LOOP
;<20>               |   |   }
;<71>               |   + END LOOP
;<70>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR RuntimeDD Multiversioning (hir-runtime-dd) ***
;Function: _Z19composite_referenceiiiiPvPhj
;
; CHECK:    BEGIN REGION { }
; CHECK:           + DO i1 = 0, zext.i32.i64(%height) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2000>
; CHECK:           |   if (%width > 0)
; CHECK:           |   {
; CHECK:           |      %mv.test = &((@vrow)[0][i1][-1 * smax(0, (1 + sext.i32.i64(%other_channels))) + (zext.i32.i64(%width) * smax(0, (1 + sext.i32.i64(%other_channels)))) + 1]) >=u &((%prow)[i1]);
; CHECK:           |      %mv.test5 = &((%prow)[i1 + zext.i32.i64(%smax) + -1 * sext.i32.i64(%channels) + (zext.i32.i64(%width) * sext.i32.i64(%channels)) + -1]) >=u &((@vrow)[0][i1][-1 * smin(0, (1 + sext.i32.i64(%other_channels))) + (zext.i32.i64(%width) * smin(0, (1 + sext.i32.i64(%other_channels))))]);
; CHECK:           |      %mv.and = %mv.test  &  %mv.test5;
; CHECK:           |      %mv.test6 = &((%prow)[i1 + zext.i32.i64(%smax) + -1 * sext.i32.i64(%channels) + (zext.i32.i64(%width) * sext.i32.i64(%channels)) + -1]) >=u &((@vrow)[0][i1][-1 * smin(0, sext.i32.i64(%other_channels)) + (zext.i32.i64(%width) * smin(0, sext.i32.i64(%other_channels)))]);
; CHECK:           |      %mv.test7 = &((@vrow)[0][i1][zext.i32.i64(%smax) + -1 * smax(0, sext.i32.i64(%other_channels)) + (zext.i32.i64(%width) * smax(0, sext.i32.i64(%other_channels)))]) >=u &((%prow)[i1]);
; CHECK:           |      %mv.and8 = %mv.test6  &  %mv.test7;
; CHECK:           |      if (%mv.and == 0 && %mv.and8 == 0)
; CHECK:           |      {
; CHECK:           |         + DO i2 = 0, zext.i32.i64(%width) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647> <MVTag: 71>
; CHECK:           |         |   %2 = (@vrow)[0][i1][(1 + sext.i32.i64(%other_channels)) * i2];
; CHECK:           |         |   %3 = (@vrow)[0][i1][(1 + sext.i32.i64(%other_channels)) * i2 + 1];
; CHECK:           |         |   if (%channels > 0)
; CHECK:           |         |   {
; CHECK:           |         |      + DO i3 = 0, zext.i32.i64(%smax) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>
; CHECK:           |         |      |   %4 = (@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + i3];
; CHECK:           |         |      |   %5 = (@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + i3 + 1];
; CHECK:           |         |      |   %6 = (%prow)[i1 + sext.i32.i64(%channels) * i2 + i3];
; CHECK:           |         |      |   (%prow)[i1 + sext.i32.i64(%channels) * i2 + i3] = (zext.i8.i32(%3) + zext.i8.i32(%6) + zext.i8.i32(%5) + (zext.i8.i32(%2) * zext.i8.i32(%4)))/u255;
; CHECK:           |         |      + END LOOP
; CHECK:           |         |   }
; CHECK:           |         + END LOOP
; CHECK:           |      }
; CHECK:           |      else
; CHECK:           |      {
; CHECK:           |         + DO i2 = 0, zext.i32.i64(%width) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647> <MVTag: 71> <nounroll> <novectorize>
; CHECK:           |         |   %2 = (@vrow)[0][i1][(1 + sext.i32.i64(%other_channels)) * i2];
; CHECK:           |         |   %3 = (@vrow)[0][i1][(1 + sext.i32.i64(%other_channels)) * i2 + 1];
; CHECK:           |         |   if (%channels > 0)
; CHECK:           |         |   {
; CHECK:           |         |      + DO i3 = 0, zext.i32.i64(%smax) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>
; CHECK:           |         |      |   %4 = (@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + i3];
; CHECK:           |         |      |   %5 = (@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + i3 + 1];
; CHECK:           |         |      |   %6 = (%prow)[i1 + sext.i32.i64(%channels) * i2 + i3];
; CHECK:           |         |      |   (%prow)[i1 + sext.i32.i64(%channels) * i2 + i3] = (zext.i8.i32(%3) + zext.i8.i32(%6) + zext.i8.i32(%5) + (zext.i8.i32(%2) * zext.i8.i32(%4)))/u255;
; CHECK:           |         |      + END LOOP
; CHECK:           |         |   }
; CHECK:           |         + END LOOP
; CHECK:           |      }
; CHECK:           |   }
; CHECK:           + END LOOP
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 't.cpp'
source_filename = "t.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@vrow = external dso_local local_unnamed_addr global [2000 x [8000 x i8]], align 16

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
define dso_local void @_Z19composite_referenceiiiiPvPhj(i32 %width, i32 %height, i32 %channels, i32 %other_channels, i8* nocapture readnone %highP, i8* nocapture %prow, i32 %t) local_unnamed_addr #0 {
entry:
  %cmp1473 = icmp sgt i32 %channels, 0
  %idx.ext = sext i32 %channels to i64
  %idx.ext33 = sext i32 %other_channels to i64
  %cmp575 = icmp sgt i32 %width, 0
  %cmp79 = icmp sgt i32 %height, 0
  br i1 %cmp79, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = icmp slt i32 %channels, 3
  %.sroa.speculated = select i1 %0, i32 %channels, i32 3
  %smax = call i32 @llvm.smax.i32(i32 %.sroa.speculated, i32 1)
  %wide.trip.count9092 = zext i32 %height to i64
  %wide.trip.count8593 = zext i32 %width to i64
  %1 = zext i32 %smax to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup6
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.cond.cleanup6
  %indvars.iv87 = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next88, %for.cond.cleanup6 ]
  br i1 %cmp575, label %for.body7.preheader, label %for.cond.cleanup6

for.body7.preheader:                              ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i8, i8* %prow, i64 %indvars.iv87
  %arrayidx1 = getelementptr inbounds [2000 x [8000 x i8]], [2000 x [8000 x i8]]* @vrow, i64 0, i64 %indvars.iv87, i64 0, !intel-tbaa !3
  br label %for.body7

for.cond.cleanup6.loopexit:                       ; preds = %for.cond.cleanup15
  br label %for.cond.cleanup6

for.cond.cleanup6:                                ; preds = %for.cond.cleanup6.loopexit, %for.body
  %indvars.iv.next88 = add nuw nsw i64 %indvars.iv87, 1
  %exitcond91.not = icmp eq i64 %indvars.iv.next88, %wide.trip.count9092
  br i1 %exitcond91.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !8

for.body7:                                        ; preds = %for.body7.preheader, %for.cond.cleanup15
  %indvars.iv82 = phi i64 [ 0, %for.body7.preheader ], [ %indvars.iv.next83, %for.cond.cleanup15 ]
  %other_row.078 = phi i8* [ %arrayidx1, %for.body7.preheader ], [ %add.ptr34, %for.cond.cleanup15 ]
  %this_row.077 = phi i8* [ %arrayidx3, %for.body7.preheader ], [ %add.ptr, %for.cond.cleanup15 ]
  %arrayidx9 = getelementptr inbounds i8, i8* %other_row.078, i64 %indvars.iv82
  %2 = load i8, i8* %arrayidx9, align 1, !tbaa !10
  %conv = zext i8 %2 to i32
  %indvars.iv.next83 = add nuw nsw i64 %indvars.iv82, 1
  %arrayidx11 = getelementptr inbounds i8, i8* %other_row.078, i64 %indvars.iv.next83
  %3 = load i8, i8* %arrayidx11, align 1, !tbaa !10
  %conv12 = zext i8 %3 to i32
  br i1 %cmp1473, label %for.body16.preheader, label %for.cond.cleanup15

for.body16.preheader:                             ; preds = %for.body7
  br label %for.body16

for.cond.cleanup15.loopexit:                      ; preds = %for.body16
  br label %for.cond.cleanup15

for.cond.cleanup15:                               ; preds = %for.cond.cleanup15.loopexit, %for.body7
  %add.ptr = getelementptr inbounds i8, i8* %this_row.077, i64 %idx.ext, !intel-tbaa !10
  %add.ptr34 = getelementptr inbounds i8, i8* %other_row.078, i64 %idx.ext33, !intel-tbaa !10
  %exitcond86.not = icmp eq i64 %indvars.iv.next83, %wide.trip.count8593
  br i1 %exitcond86.not, label %for.cond.cleanup6.loopexit, label %for.body7, !llvm.loop !11

for.body16:                                       ; preds = %for.body16.preheader, %for.body16
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body16 ], [ 0, %for.body16.preheader ]
  %arrayidx18 = getelementptr inbounds i8, i8* %other_row.078, i64 %indvars.iv
  %4 = load i8, i8* %arrayidx18, align 1, !tbaa !10
  %conv19 = zext i8 %4 to i32
  %mul = mul nuw nsw i32 %conv19, %conv
  %add20 = add nuw nsw i32 %mul, %conv12
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx23 = getelementptr inbounds i8, i8* %other_row.078, i64 %indvars.iv.next
  %5 = load i8, i8* %arrayidx23, align 1, !tbaa !10
  %conv24 = zext i8 %5 to i32
  %add25 = add nuw nsw i32 %add20, %conv24
  %arrayidx27 = getelementptr inbounds i8, i8* %this_row.077, i64 %indvars.iv
  %6 = load i8, i8* %arrayidx27, align 1, !tbaa !10
  %conv28 = zext i8 %6 to i32
  %add29 = add nuw nsw i32 %add25, %conv28
  %div = udiv i32 %add29, 255
  %conv30 = trunc i32 %div to i8
  store i8 %conv30, i8* %arrayidx27, align 1, !tbaa !10
  %exitcond.not = icmp eq i64 %indvars.iv.next, %1
  br i1 %exitcond.not, label %for.cond.cleanup15.loopexit, label %for.body16, !llvm.loop !12
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i32 @llvm.smax.i32(i32, i32) #1

attributes #0 = { mustprogress nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="true" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !6, i64 0}
!4 = !{!"array@_ZTSA2000_A8000_h", !5, i64 0}
!5 = !{!"array@_ZTSA8000_h", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
!10 = !{!6, !6, i64 0}
!11 = distinct !{!11, !9}
!12 = distinct !{!12, !9}

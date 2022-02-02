; Test that RTDD can happen before a relaxed non-perfect loopnest
;
; RUN: opt -hir-ssa-deconstruction -hir-runtime-dd -print-after=hir-runtime-dd -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
;*** IR Dump Before HIR RuntimeDD Multiversioning (hir-runtime-dd) ***                                             
;Function: _Z19composite_referenceiiiiPvPhS0_                                                                      
;
;<0>          BEGIN REGION { }
;<62>               + DO i1 = 0, zext.i32.i64(%height) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2000>
;<63>               |   + DO i2 = 0, %width + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;<12>               |   |   %2 = (%new_row)[3];                                                
;<14>               |   |   %sub = %2  ^  255;
;<15>               |   |   if (%channels > 0)
;<15>               |   |   {
;<64>               |   |      + DO i3 = 0, zext.i32.i64(%smax) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>
;<23>               |   |      |   %3 = (@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + i3];
;<27>               |   |      |   %4 = (%prow)[sext.i32.i64(%channels) * i2 + i3];
;<33>               |   |      |   (%prow)[sext.i32.i64(%channels) * i2 + i3] = ((zext.i8.i32(%2) * zext.i8.i32(%3)) + (zext.i8.i32(%4) * %sub))/u255;
;<64>               |   |      + END LOOP
;<15>               |   |   }
;<63>               |   + END LOOP
;<62>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR RuntimeDD Multiversioning (hir-runtime-dd) ***
;Function: _Z19composite_referenceiiiiPvPhS0_
;
; CHECK:   BEGIN REGION { }
; CHECK:           + DO i1 = 0, zext.i32.i64(%height) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2000>
; CHECK:           |   if (%width > 0)
; CHECK:           |   {
; CHECK:           |      %mv.test = &((%new_row)[3]) >=u &((%prow)[0]);
; CHECK:           |      %mv.test5 = &((%prow)[zext.i32.i64(%smax) + (sext.i32.i64((-1 + %width)) * sext.i32.i64(%channels)) + -1]) >=u &((%new_row)[3]);
; CHECK:           |      %mv.and = %mv.test  &  %mv.test5;
; CHECK:           |      %mv.test6 = &((@vrow)[0][i1][zext.i32.i64(%smax) + (sext.i32.i64((-1 + %width)) * smax(0, sext.i32.i64(%other_channels))) + -1]) >=u &((%prow)[0]);
; CHECK:           |      %mv.test7 = &((%prow)[zext.i32.i64(%smax) + (sext.i32.i64((-1 + %width)) * sext.i32.i64(%channels)) + -1]) >=u &((@vrow)[0][i1][(sext.i32.i64((-1 + %width)) * smin(0, sext.i32.i64(%other_channels)))]);
; CHECK:           |      %mv.and8 = %mv.test6  &  %mv.test7;
; CHECK:           |      if (%mv.and == 0 && %mv.and8 == 0)
; CHECK:           |      {
; CHECK:           |         + DO i2 = 0, %width + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647> <MVTag: 63>
; CHECK:           |         |   %2 = (%new_row)[3];
; CHECK:           |         |   %sub = %2  ^  255;
; CHECK:           |         |   if (%channels > 0)
; CHECK:           |         |   {
; CHECK:           |         |      + DO i3 = 0, zext.i32.i64(%smax) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>
; CHECK:           |         |      |   %3 = (@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + i3];
; CHECK:           |         |      |   %4 = (%prow)[sext.i32.i64(%channels) * i2 + i3];
; CHECK:           |         |      |   (%prow)[sext.i32.i64(%channels) * i2 + i3] = ((zext.i8.i32(%2) * zext.i8.i32(%3)) + (zext.i8.i32(%4) * %sub))/u255;
; CHECK:           |         |      + END LOOP
; CHECK:           |         |   }
; CHECK:           |         + END LOOP
; CHECK:           |      }
; CHECK:           |      else
; CHECK:           |      {
; CHECK:           |         + DO i2 = 0, %width + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647> <MVTag: 63> <nounroll> <novectorize>
; CHECK:           |         |   %2 = (%new_row)[3];
; CHECK:           |         |   %sub = %2  ^  255;
; CHECK:           |         |   if (%channels > 0)
; CHECK:           |         |   {
; CHECK:           |         |      + DO i3 = 0, zext.i32.i64(%smax) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>
; CHECK:           |         |      |   %3 = (@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + i3];
; CHECK:           |         |      |   %4 = (%prow)[sext.i32.i64(%channels) * i2 + i3];
; CHECK:           |         |      |   (%prow)[sext.i32.i64(%channels) * i2 + i3] = ((zext.i8.i32(%2) * zext.i8.i32(%3)) + (zext.i8.i32(%4) * %sub))/u255;
; CHECK:           |         |      + END LOOP
; CHECK:           |         |   }
; CHECK:           |         + END LOOP
; CHECK:           |      }
; CHECK:           |   }
; CHECK:           + END LOOP
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 'smallcompo3.cpp'
source_filename = "smallcompo3.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@vrow = external dso_local local_unnamed_addr global [2000 x [8000 x i8]], align 16

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
define dso_local void @_Z19composite_referenceiiiiPvPhS0_(i32 %width, i32 %height, i32 %channels, i32 %other_channels, i8* nocapture readnone %highP, i8* nocapture %prow, i8* nocapture readonly %new_row) local_unnamed_addr #0 {
entry:
  %arrayidx6 = getelementptr inbounds i8, i8* %new_row, i64 3
  %cmp853 = icmp sgt i32 %channels, 0
  %idx.ext = sext i32 %channels to i64
  %idx.ext21 = sext i32 %other_channels to i64
  %cmp355 = icmp sgt i32 %width, 0
  %cmp59 = icmp sgt i32 %height, 0
  br i1 %cmp59, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = icmp slt i32 %channels, 3
  %.sroa.speculated = select i1 %0, i32 %channels, i32 3
  %smax = call i32 @llvm.smax.i32(i32 %.sroa.speculated, i32 1)
  %wide.trip.count6668 = zext i32 %height to i64
  %1 = zext i32 %smax to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup4
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.cond.cleanup4
  %indvars.iv63 = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next64, %for.cond.cleanup4 ]
  br i1 %cmp355, label %for.body5.preheader, label %for.cond.cleanup4

for.body5.preheader:                              ; preds = %for.body
  %arrayidx1 = getelementptr inbounds [2000 x [8000 x i8]], [2000 x [8000 x i8]]* @vrow, i64 0, i64 %indvars.iv63, i64 0, !intel-tbaa !3
  br label %for.body5

for.cond.cleanup4.loopexit:                       ; preds = %for.cond.cleanup9
  br label %for.cond.cleanup4

for.cond.cleanup4:                                ; preds = %for.cond.cleanup4.loopexit, %for.body
  %indvars.iv.next64 = add nuw nsw i64 %indvars.iv63, 1
  %exitcond67.not = icmp eq i64 %indvars.iv.next64, %wide.trip.count6668
  br i1 %exitcond67.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !8

for.body5:                                        ; preds = %for.body5.preheader, %for.cond.cleanup9
  %other_row.058 = phi i8* [ %add.ptr22, %for.cond.cleanup9 ], [ %arrayidx1, %for.body5.preheader ]
  %x.057 = phi i32 [ %inc24, %for.cond.cleanup9 ], [ 0, %for.body5.preheader ]
  %this_row.056 = phi i8* [ %add.ptr, %for.cond.cleanup9 ], [ %prow, %for.body5.preheader ]
  %2 = load i8, i8* %arrayidx6, align 1, !tbaa !10
  %conv = zext i8 %2 to i32
  %sub = xor i32 %conv, 255
  br i1 %cmp853, label %for.body10.preheader, label %for.cond.cleanup9

for.body10.preheader:                             ; preds = %for.body5
  br label %for.body10

for.cond.cleanup9.loopexit:                       ; preds = %for.body10
  br label %for.cond.cleanup9

for.cond.cleanup9:                                ; preds = %for.cond.cleanup9.loopexit, %for.body5
  %add.ptr = getelementptr inbounds i8, i8* %this_row.056, i64 %idx.ext, !intel-tbaa !10
  %add.ptr22 = getelementptr inbounds i8, i8* %other_row.058, i64 %idx.ext21, !intel-tbaa !10
  %inc24 = add nuw nsw i32 %x.057, 1
  %exitcond62.not = icmp eq i32 %inc24, %width
  br i1 %exitcond62.not, label %for.cond.cleanup4.loopexit, label %for.body5, !llvm.loop !11

for.body10:                                       ; preds = %for.body10.preheader, %for.body10
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body10 ], [ 0, %for.body10.preheader ]
  %arrayidx12 = getelementptr inbounds i8, i8* %other_row.058, i64 %indvars.iv
  %3 = load i8, i8* %arrayidx12, align 1, !tbaa !10
  %conv13 = zext i8 %3 to i32
  %mul = mul nuw nsw i32 %conv13, %conv
  %arrayidx15 = getelementptr inbounds i8, i8* %this_row.056, i64 %indvars.iv
  %4 = load i8, i8* %arrayidx15, align 1, !tbaa !10
  %conv16 = zext i8 %4 to i32
  %mul17 = mul nuw nsw i32 %sub, %conv16
  %add = add nuw nsw i32 %mul17, %mul
  %div = udiv i32 %add, 255
  %conv18 = trunc i32 %div to i8
  store i8 %conv18, i8* %arrayidx15, align 1, !tbaa !10
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %1
  br i1 %exitcond.not, label %for.cond.cleanup9.loopexit, label %for.body10, !llvm.loop !12
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

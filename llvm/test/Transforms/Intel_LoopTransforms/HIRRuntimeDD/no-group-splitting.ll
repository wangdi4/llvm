; RUN: opt -hir-ssa-deconstruction -hir-runtime-dd -print-after=hir-runtime-dd -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; Spliting group does not happen in this lit test case. If we split group 0 into two groups which group 0 contains
; (@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + 3] and group 2 contains (@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + i3]
; and (@vrow)[0][i1][i1 + sext.i32.i64(%other_channels) * i2], we need to skip splitting because the refs in group2
; do not have constant distance, this group will not pass sort refs in groups check. If we continue to split group2, it
; will not pass refs existing in difference parent loop check, so it needs an early exit of the group spilting function.
;
; Group 0 contains (3) refs:
;(@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + 3]
;(@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + i3]
;(@vrow)[0][i1][i1 + sext.i32.i64(%other_channels) * i2]
;Group 1 contains (2) refs:
;(%prow)[i1 + sext.i32.i64(%channels) * i2 + i3]
;(%prow)[i1 + sext.i32.i64(%channels) * i2 + i3]
;
;*** IR Dump Before HIR RuntimeDD Multiversioning (hir-runtime-dd) ***
;Function: _Z19composite_referenceiiiiPvPhj
;
;<0>          BEGIN REGION { }
;<66>               + DO i1 = 0, zext.i32.i64(%height) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2000>
;<67>               |   + DO i2 = 0, %width + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;<14>               |   |   %2 = (@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + 3];
;<17>               |   |   if (%channels > 0)
;<17>               |   |   {
;<68>               |   |      + DO i3 = 0, zext.i32.i64(%smax) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>
;<25>               |   |      |   %3 = (@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + i3];
;<28>               |   |      |   %4 = (@vrow)[0][i1][i1 + sext.i32.i64(%other_channels) * i2];
;<32>               |   |      |   %5 = (%prow)[i1 + sext.i32.i64(%channels) * i2 + i3];
;<37>               |   |      |   (%prow)[i1 + sext.i32.i64(%channels) * i2 + i3] = (zext.i8.i32(%5) + zext.i8.i32(%4) + (zext.i8.i32(%2) * zext.i8.i32(%3)))/u255;
;<68>               |   |      + END LOOP
;<17>               |   |   }
;<67>               |   + END LOOP
;<66>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR RuntimeDD Multiversioning (hir-runtime-dd) ***
;Function: _Z19composite_referenceiiiiPvPhj
;
; CHECK:       BEGIN REGION { }
; CHECK:           + DO i1 = 0, zext.i32.i64(%height) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2000>
; CHECK:           |   + DO i2 = 0, %width + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:           |   |   %2 = (@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + 3];
; CHECK:           |   |   if (%channels > 0)
; CHECK:           |   |   {
; CHECK:           |   |      + DO i3 = 0, zext.i32.i64(%smax) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>
; CHECK:           |   |      |   %3 = (@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + i3];
; CHECK:           |   |      |   %4 = (@vrow)[0][i1][i1 + sext.i32.i64(%other_channels) * i2];
; CHECK:           |   |      |   %5 = (%prow)[i1 + sext.i32.i64(%channels) * i2 + i3];
; CHECK:           |   |      |   (%prow)[i1 + sext.i32.i64(%channels) * i2 + i3] = (zext.i8.i32(%5) + zext.i8.i32(%4) + (zext.i8.i32(%2) * zext.i8.i32(%3)))/u255;
; CHECK:           |   |      + END LOOP
; CHECK:           |   |   }
; CHECK:           |   + END LOOP
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
  %cmp1062 = icmp sgt i32 %channels, 0
  %idx.ext = sext i32 %channels to i64
  %idx.ext26 = sext i32 %other_channels to i64
  %cmp564 = icmp sgt i32 %width, 0
  %cmp68 = icmp sgt i32 %height, 0
  br i1 %cmp68, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = icmp slt i32 %channels, 3
  %.sroa.speculated = select i1 %0, i32 %channels, i32 3
  %smax = call i32 @llvm.smax.i32(i32 %.sroa.speculated, i32 1)
  %wide.trip.count7577 = zext i32 %height to i64
  %1 = zext i32 %smax to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup6
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.cond.cleanup6
  %indvars.iv72 = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next73, %for.cond.cleanup6 ]
  br i1 %cmp564, label %for.body7.preheader, label %for.cond.cleanup6

for.body7.preheader:                              ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i8, i8* %prow, i64 %indvars.iv72
  %arrayidx1 = getelementptr inbounds [2000 x [8000 x i8]], [2000 x [8000 x i8]]* @vrow, i64 0, i64 %indvars.iv72, i64 0, !intel-tbaa !3
  br label %for.body7

for.cond.cleanup6.loopexit:                       ; preds = %for.cond.cleanup11
  br label %for.cond.cleanup6

for.cond.cleanup6:                                ; preds = %for.cond.cleanup6.loopexit, %for.body
  %indvars.iv.next73 = add nuw nsw i64 %indvars.iv72, 1
  %exitcond76.not = icmp eq i64 %indvars.iv.next73, %wide.trip.count7577
  br i1 %exitcond76.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !8

for.body7:                                        ; preds = %for.body7.preheader, %for.cond.cleanup11
  %other_row.067 = phi i8* [ %add.ptr27, %for.cond.cleanup11 ], [ %arrayidx1, %for.body7.preheader ]
  %x.066 = phi i32 [ %inc29, %for.cond.cleanup11 ], [ 0, %for.body7.preheader ]
  %this_row.065 = phi i8* [ %add.ptr, %for.cond.cleanup11 ], [ %arrayidx3, %for.body7.preheader ]
  %arrayidx8 = getelementptr inbounds i8, i8* %other_row.067, i64 3
  %2 = load i8, i8* %arrayidx8, align 1, !tbaa !10
  %conv = zext i8 %2 to i32
  %arrayidx17 = getelementptr inbounds i8, i8* %other_row.067, i64 %indvars.iv72
  br i1 %cmp1062, label %for.body12.preheader, label %for.cond.cleanup11

for.body12.preheader:                             ; preds = %for.body7
  br label %for.body12

for.cond.cleanup11.loopexit:                      ; preds = %for.body12
  br label %for.cond.cleanup11

for.cond.cleanup11:                               ; preds = %for.cond.cleanup11.loopexit, %for.body7
  %add.ptr = getelementptr inbounds i8, i8* %this_row.065, i64 %idx.ext, !intel-tbaa !10
  %add.ptr27 = getelementptr inbounds i8, i8* %other_row.067, i64 %idx.ext26, !intel-tbaa !10
  %inc29 = add nuw nsw i32 %x.066, 1
  %exitcond71.not = icmp eq i32 %inc29, %width
  br i1 %exitcond71.not, label %for.cond.cleanup6.loopexit, label %for.body7, !llvm.loop !11

for.body12:                                       ; preds = %for.body12.preheader, %for.body12
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body12 ], [ 0, %for.body12.preheader ]
  %arrayidx14 = getelementptr inbounds i8, i8* %other_row.067, i64 %indvars.iv
  %3 = load i8, i8* %arrayidx14, align 1, !tbaa !10
  %conv15 = zext i8 %3 to i32
  %mul = mul nuw nsw i32 %conv15, %conv
  %4 = load i8, i8* %arrayidx17, align 1, !tbaa !10
  %conv18 = zext i8 %4 to i32
  %add = add nuw nsw i32 %mul, %conv18
  %arrayidx20 = getelementptr inbounds i8, i8* %this_row.065, i64 %indvars.iv
  %5 = load i8, i8* %arrayidx20, align 1, !tbaa !10
  %conv21 = zext i8 %5 to i32
  %add22 = add nuw nsw i32 %add, %conv21
  %div = udiv i32 %add22, 255
  %conv23 = trunc i32 %div to i8
  store i8 %conv23, i8* %arrayidx20, align 1, !tbaa !10
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %1
  br i1 %exitcond.not, label %for.cond.cleanup11.loopexit, label %for.body12, !llvm.loop !12
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

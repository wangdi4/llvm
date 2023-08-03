; Test that RTDD cannot be triggered at i2 loop, because i3 loop's upperbound ref is defined at i2 level
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
;*** IR Dump Before HIR RuntimeDD Multiversioning (hir-runtime-dd) ***
;Function: _Z19composite_referenceiiiiPvPhj
;
;<0>          BEGIN REGION { }
;<67>               + DO i1 = 0, zext.i32.i64(%height) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2000>
;<68>               |   + DO i2 = 0, %width + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;<14>               |   |   %0 = (@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + 3];
;<16>               |   |   %sub = %0  ^  255;
;<20>               |   |   if (umax((3 + %t), %sub) != 0)
;<20>               |   |   {
;<69>               |   |      + DO i3 = 0, sext.i32.i64(umax((3 + %t), %sub)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 8000>
;<29>               |   |      |   %1 = (@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + i3];
;<33>               |   |      |   %2 = (%prow)[i1 + sext.i32.i64(%channels) * i2 + i3];
;<38>               |   |      |   (%prow)[i1 + sext.i32.i64(%channels) * i2 + i3] = (zext.i8.i32(%2) + (zext.i8.i32(%0) * zext.i8.i32(%1)))/u255;
;<69>               |   |      + END LOOP
;<20>               |   |   }
;<68>               |   + END LOOP
;<67>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR RuntimeDD Multiversioning (hir-runtime-dd) ***
;Function: _Z19composite_referenceiiiiPvPhj
;
; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, zext.i32.i64(%height) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2000>
; CHECK:           |   + DO i2 = 0, %width + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:           |   |   %0 = (@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + 3];
; CHECK:           |   |   %sub = %0  ^  255;
; CHECK:           |   |   if (umax((3 + %t), %sub) != 0)
; CHECK:           |   |   {
; CHECK:           |   |      %mv.test = &((@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + sext.i32.i64(umax((3 + %t), %sub)) + -1]) >=u &((%prow)[i1 + sext.i32.i64(%channels) * i2]);
; CHECK:           |   |      %mv.test5 = &((%prow)[i1 + sext.i32.i64(%channels) * i2 + sext.i32.i64(umax((3 + %t), %sub)) + -1]) >=u &((@vrow)[0][i1][sext.i32.i64(%other_channels) * i2]);
; CHECK:           |   |      %mv.and = %mv.test  &  %mv.test5;
; CHECK:           |   |      if (%mv.and == 0)
; CHECK:           |   |      {
; CHECK:           |   |         + DO i3 = 0, sext.i32.i64(umax((3 + %t), %sub)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 8000>  <MVTag: 69>
; CHECK:           |   |         |   %1 = (@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + i3];
; CHECK:           |   |         |   %2 = (%prow)[i1 + sext.i32.i64(%channels) * i2 + i3];
; CHECK:           |   |         |   (%prow)[i1 + sext.i32.i64(%channels) * i2 + i3] = (zext.i8.i32(%2) + (zext.i8.i32(%0) * zext.i8.i32(%1)))/u255;
; CHECK:           |   |         + END LOOP
; CHECK:           |   |      }
; CHECK:           |   |      else
; CHECK:           |   |      {
; CHECK:           |   |         + DO i3 = 0, sext.i32.i64(umax((3 + %t), %sub)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 8000>  <MVTag: 69> <nounroll> <novectorize>
; CHECK:           |   |         |   %1 = (@vrow)[0][i1][sext.i32.i64(%other_channels) * i2 + i3];
; CHECK:           |   |         |   %2 = (%prow)[i1 + sext.i32.i64(%channels) * i2 + i3];
; CHECK:           |   |         |   (%prow)[i1 + sext.i32.i64(%channels) * i2 + i3] = (zext.i8.i32(%2) + (zext.i8.i32(%0) * zext.i8.i32(%1)))/u255;
; CHECK:           |   |         + END LOOP
; CHECK:           |   |      }
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
define dso_local void @_Z19composite_referenceiiiiPvPhj(i32 %width, i32 %height, i32 %channels, i32 %other_channels, ptr nocapture readnone %highP, ptr nocapture %prow, i32 %t) local_unnamed_addr #0 {
entry:
  %add = add i32 %t, 3
  %idx.ext = sext i32 %channels to i64
  %idx.ext23 = sext i32 %other_channels to i64
  %cmp558 = icmp sgt i32 %width, 0
  %cmp62 = icmp sgt i32 %height, 0
  br i1 %cmp62, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count6971 = zext i32 %height to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup6
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.cond.cleanup6
  %indvars.iv66 = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next67, %for.cond.cleanup6 ]
  br i1 %cmp558, label %for.body7.preheader, label %for.cond.cleanup6

for.body7.preheader:                              ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i8, ptr %prow, i64 %indvars.iv66
  %arrayidx1 = getelementptr inbounds [2000 x [8000 x i8]], ptr @vrow, i64 0, i64 %indvars.iv66, i64 0, !intel-tbaa !3
  br label %for.body7

for.cond.cleanup6.loopexit:                       ; preds = %for.cond.cleanup11
  br label %for.cond.cleanup6

for.cond.cleanup6:                                ; preds = %for.cond.cleanup6.loopexit, %for.body
  %indvars.iv.next67 = add nuw nsw i64 %indvars.iv66, 1
  %exitcond70.not = icmp eq i64 %indvars.iv.next67, %wide.trip.count6971
  br i1 %exitcond70.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !8

for.body7:                                        ; preds = %for.body7.preheader, %for.cond.cleanup11
  %x.061 = phi i32 [ %inc26, %for.cond.cleanup11 ], [ 0, %for.body7.preheader ]
  %this_row.060 = phi ptr [ %add.ptr, %for.cond.cleanup11 ], [ %arrayidx3, %for.body7.preheader ]
  %other_row.059 = phi ptr [ %add.ptr24, %for.cond.cleanup11 ], [ %arrayidx1, %for.body7.preheader ]
  %arrayidx8 = getelementptr inbounds i8, ptr %other_row.059, i64 3
  %0 = load i8, ptr %arrayidx8, align 1, !tbaa !10
  %conv = zext i8 %0 to i32
  %sub = xor i32 %conv, 255
  %cmp.i = icmp ult i32 %sub, %add
  %.sroa.speculated = select i1 %cmp.i, i32 %add, i32 %sub
  %cmp1056.not = icmp eq i32 %.sroa.speculated, 0
  br i1 %cmp1056.not, label %for.cond.cleanup11, label %for.body12.preheader

for.body12.preheader:                             ; preds = %for.body7
  %wide.trip.count = sext i32 %.sroa.speculated to i64
  br label %for.body12

for.cond.cleanup11.loopexit:                      ; preds = %for.body12
  br label %for.cond.cleanup11

for.cond.cleanup11:                               ; preds = %for.cond.cleanup11.loopexit, %for.body7
  %add.ptr = getelementptr inbounds i8, ptr %this_row.060, i64 %idx.ext, !intel-tbaa !10
  %add.ptr24 = getelementptr inbounds i8, ptr %other_row.059, i64 %idx.ext23, !intel-tbaa !10
  %inc26 = add nuw nsw i32 %x.061, 1
  %exitcond65.not = icmp eq i32 %inc26, %width
  br i1 %exitcond65.not, label %for.cond.cleanup6.loopexit, label %for.body7, !llvm.loop !11

for.body12:                                       ; preds = %for.body12.preheader, %for.body12
  %indvars.iv = phi i64 [ 0, %for.body12.preheader ], [ %indvars.iv.next, %for.body12 ]
  %arrayidx14 = getelementptr inbounds i8, ptr %other_row.059, i64 %indvars.iv
  %1 = load i8, ptr %arrayidx14, align 1, !tbaa !10
  %conv15 = zext i8 %1 to i32
  %mul = mul nuw nsw i32 %conv15, %conv
  %arrayidx17 = getelementptr inbounds i8, ptr %this_row.060, i64 %indvars.iv
  %2 = load i8, ptr %arrayidx17, align 1, !tbaa !10
  %conv18 = zext i8 %2 to i32
  %add19 = add nuw nsw i32 %mul, %conv18
  %div = udiv i32 %add19, 255
  %conv20 = trunc i32 %div to i8
  store i8 %conv20, ptr %arrayidx17, align 1, !tbaa !10
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup11.loopexit, label %for.body12, !llvm.loop !12
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="true" }

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

; We multiversion at the i1 loop level by splitting the memref group which
; includes (@vrow)[0][i1][i3] and (@vrow)[0][i1][(1 + sext.i32.i64(%other_channels)) * i2]
; into two memref groups.
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; Incoming HIR-
;
;<0>          BEGIN REGION { }
;<63>               + DO i1 = 0, zext.i32.i64(%height) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2000>  <LEGAL_MAX_TC = 2147483647>
;<64>               |   + DO i2 = 0, zext.i32.i64(%width) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;<13>               |   |   %2 = (@vrow)[0][i1][(1 + sext.i32.i64(%other_channels)) * i2];
;<15>               |   |   %sub = %2  ^  255;
;<65>               |   |
;<65>               |   |   + DO i3 = 0, zext.i32.i64(%smax) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>  <LEGAL_MAX_TC = 3>
;<24>               |   |   |   %3 = (@vrow)[0][i1][i3];
;<28>               |   |   |   %4 = (%prow)[sext.i32.i64(%channels) * i2 + i3];
;<34>               |   |   |   (%prow)[sext.i32.i64(%channels) * i2 + i3] = ((zext.i8.i32(%2) * zext.i8.i32(%3)) + (zext.i8.i32(%4) * %sub))/u255;
;<65>               |   |   + END LOOP
;<64>               |   + END LOOP
;<63>               + END LOOP
;<0>          END REGION
;
; Ref groups after split:
; Group 0 contains (1) refs:
;(@vrow)[0][i1][(1 + sext.i32.i64(%other_channels)) * i2]
;Group 1 contains (2) refs:
;(%prow)[sext.i32.i64(%channels) * i2 + i3]
;(%prow)[sext.i32.i64(%channels) * i2 + i3]
;Group 2 contains (1) refs:
;(@vrow)[0][i1][i3]
;
;*** IR Dump After HIR RuntimeDD Multiversioning (hir-runtime-dd) ***
;
; CHECK: %mv.test = &((@vrow)[0][zext.i32.i64(%height) + -1][-1 * smax(0, (1 + sext.i32.i64(%other_channels))) + (zext.i32.i64(%width) * smax(0, (1 + sext.i32.i64(%other_channels))))]) >=u &((%prow)[0]);
; CHECK: %mv.test5 = &((%prow)[zext.i32.i64(%smax) + -1 * sext.i32.i64(%channels) + (zext.i32.i64(%width) * sext.i32.i64(%channels)) + -1]) >=u &((@vrow)[0][0][-1 * smin(0, (1 + sext.i32.i64(%other_channels))) + (zext.i32.i64(%width) * smin(0, (1 + sext.i32.i64(%other_channels))))]);
; CHECK: %mv.and = %mv.test  &  %mv.test5;
; CHECK: %mv.test6 = &((%prow)[zext.i32.i64(%smax) + -1 * sext.i32.i64(%channels) + (zext.i32.i64(%width) * sext.i32.i64(%channels)) + -1]) >=u &((@vrow)[0][0][0]);
; CHECK: %mv.test7 = &((@vrow)[0][zext.i32.i64(%height) + -1][zext.i32.i64(%smax) + -1]) >=u &((%prow)[0]);
; CHECK: %mv.and8 = %mv.test6  &  %mv.test7;
; CHECK: if (%mv.and == 0 & %mv.and8 == 0)  <MVTag: 63>
; CHECK: {
; CHECK:    + DO i1 = 0, zext.i32.i64(%height) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2000>  <LEGAL_MAX_TC = 2147483647>  <MVTag: 63>
; CHECK:    |   + DO i2 = 0, zext.i32.i64(%width) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>  <MVTag: 64>
; CHECK:    |   |   %2 = (@vrow)[0][i1][(1 + sext.i32.i64(%other_channels)) * i2];
; CHECK:    |   |   %sub = %2  ^  255;
; CHECK:    |   |
; CHECK:    |   |   + DO i3 = 0, zext.i32.i64(%smax) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>  <LEGAL_MAX_TC = 3>  <MVTag: 65>
; CHECK:    |   |   |   %3 = (@vrow)[0][i1][i3];
; CHECK:    |   |   |   %4 = (%prow)[sext.i32.i64(%channels) * i2 + i3];
; CHECK:    |   |   |   (%prow)[sext.i32.i64(%channels) * i2 + i3] = ((zext.i8.i32(%2) * zext.i8.i32(%3)) + (zext.i8.i32(%4) * %sub))/u255;
; CHECK:    |   |   + END LOOP
; CHECK:    |   + END LOOP
; CHECK:    + END LOOP
; CHECK: }
; CHECK: else
; CHECK: {
; CHECK:    + DO i1 = 0, zext.i32.i64(%height) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2000>  <LEGAL_MAX_TC = 2147483647>  <MVTag: 63> <nounroll> <novectorize>
; CHECK:    |   + DO i2 = 0, zext.i32.i64(%width) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>  <MVTag: 64> <nounroll> <novectorize>
; CHECK:    |   |   %2 = (@vrow)[0][i1][(1 + sext.i32.i64(%other_channels)) * i2];
; CHECK:    |   |   %sub = %2  ^  255;
; CHECK:    |   |
; CHECK:    |   |   + DO i3 = 0, zext.i32.i64(%smax) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>  <LEGAL_MAX_TC = 3>  <MVTag: 65> <nounroll> <novectorize>
; CHECK:    |   |   |   %3 = (@vrow)[0][i1][i3];
; CHECK:    |   |   |   %4 = (%prow)[sext.i32.i64(%channels) * i2 + i3];
; CHECK:    |   |   |   (%prow)[sext.i32.i64(%channels) * i2 + i3] = ((zext.i8.i32(%2) * zext.i8.i32(%3)) + (zext.i8.i32(%4) * %sub))/u255;
; CHECK:    |   |   + END LOOP
; CHECK:    |   + END LOOP
; CHECK:    + END LOOP
; CHECK: }
;
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -hir-details < %s 2>&1 | FileCheck %s -check-prefix=CHECK-ALIAS
;
; CHECK-ALIAS: <RVAL-REG> {{.*}} @vrow)[{{.*}}1 + sext.i32.i64(%other_channels)) * i2]{{.*}} !alias.scope [[SCOPE1:.*]] !noalias [[SCOPE2:.*]] {
; CHECK-ALIAS: <RVAL-REG> {{.*}} @vrow)[{{.*}}i3]{{.*}} !alias.scope [[SCOPE1]] !noalias [[SCOPE2]] {
; CHECK-ALIAS: <RVAL-REG> {{.*}} %prow)[{{.*}}sext.i32.i64(%channels) * i2 + i3]{{.*}} !alias.scope [[SCOPE2:.*]] !noalias [[SCOPE1:.*]] {
;
;Module Before HIR
; ModuleID = 'smallcompo2.cpp'
source_filename = "smallcompo2.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@vrow = external dso_local local_unnamed_addr global [2000 x [8000 x i8]], align 16

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
define dso_local void @_Z19composite_referenceiiiiPvPh(i32 %width, i32 %height, i32 %channels, i32 %other_channels, i8* nocapture readnone %highP, i8* nocapture %prow) local_unnamed_addr #0 {
entry:
  %cmp958 = icmp sgt i32 %channels, 0
  %idx.ext = sext i32 %channels to i64
  %idx.ext24 = sext i32 %other_channels to i64
  %cmp360 = icmp sgt i32 %width, 0
  %cmp64 = icmp sgt i32 %height, 0
  br i1 %cmp64, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = icmp slt i32 %channels, 3
  %.sroa.speculated = select i1 %0, i32 %channels, i32 3
  %smax = call i32 @llvm.smax.i32(i32 %.sroa.speculated, i32 1)
  %wide.trip.count7577 = zext i32 %height to i64
  %wide.trip.count7078 = zext i32 %width to i64
  %1 = zext i32 %smax to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup4
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.cond.cleanup4
  %indvars.iv72 = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next73, %for.cond.cleanup4 ]
  br i1 %cmp360, label %for.body5.preheader, label %for.cond.cleanup4

for.body5.preheader:                              ; preds = %for.body
  %arrayidx1 = getelementptr inbounds [2000 x [8000 x i8]], [2000 x [8000 x i8]]* @vrow, i64 0, i64 %indvars.iv72, i64 0, !intel-tbaa !3
  br label %for.body5

for.cond.cleanup4.loopexit:                       ; preds = %for.cond.cleanup10
  br label %for.cond.cleanup4

for.cond.cleanup4:                                ; preds = %for.cond.cleanup4.loopexit, %for.body
  %indvars.iv.next73 = add nuw nsw i64 %indvars.iv72, 1
  %exitcond76.not = icmp eq i64 %indvars.iv.next73, %wide.trip.count7577
  br i1 %exitcond76.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !8

for.body5:                                        ; preds = %for.body5.preheader, %for.cond.cleanup10
  %indvars.iv67 = phi i64 [ 0, %for.body5.preheader ], [ %indvars.iv.next68, %for.cond.cleanup10 ]
  %other_row.063 = phi i8* [ %arrayidx1, %for.body5.preheader ], [ %add.ptr25, %for.cond.cleanup10 ]
  %this_row.061 = phi i8* [ %prow, %for.body5.preheader ], [ %add.ptr, %for.cond.cleanup10 ]
  %arrayidx7 = getelementptr inbounds i8, i8* %other_row.063, i64 %indvars.iv67
  %2 = load i8, i8* %arrayidx7, align 1, !tbaa !10
  %conv = zext i8 %2 to i32
  %sub = xor i32 %conv, 255
  br i1 %cmp958, label %for.body11.preheader, label %for.cond.cleanup10

for.body11.preheader:                             ; preds = %for.body5
  br label %for.body11

for.cond.cleanup10.loopexit:                      ; preds = %for.body11
  br label %for.cond.cleanup10

for.cond.cleanup10:                               ; preds = %for.cond.cleanup10.loopexit, %for.body5
  %add.ptr = getelementptr inbounds i8, i8* %this_row.061, i64 %idx.ext, !intel-tbaa !10
  %add.ptr25 = getelementptr inbounds i8, i8* %other_row.063, i64 %idx.ext24, !intel-tbaa !10
  %indvars.iv.next68 = add nuw nsw i64 %indvars.iv67, 1
  %exitcond71.not = icmp eq i64 %indvars.iv.next68, %wide.trip.count7078
  br i1 %exitcond71.not, label %for.cond.cleanup4.loopexit, label %for.body5, !llvm.loop !11

for.body11:                                       ; preds = %for.body11.preheader, %for.body11
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body11 ], [ 0, %for.body11.preheader ]
  %arrayidx15 = getelementptr inbounds [2000 x [8000 x i8]], [2000 x [8000 x i8]]* @vrow, i64 0, i64 %indvars.iv72, i64 %indvars.iv, !intel-tbaa !3
  %3 = load i8, i8* %arrayidx15, align 1, !tbaa !3
  %conv16 = zext i8 %3 to i32
  %mul = mul nuw nsw i32 %conv16, %conv
  %arrayidx18 = getelementptr inbounds i8, i8* %this_row.061, i64 %indvars.iv
  %4 = load i8, i8* %arrayidx18, align 1, !tbaa !10
  %conv19 = zext i8 %4 to i32
  %mul20 = mul nuw nsw i32 %sub, %conv19
  %add = add nuw nsw i32 %mul20, %mul
  %div = udiv i32 %add, 255
  %conv21 = trunc i32 %div to i8
  store i8 %conv21, i8* %arrayidx18, align 1, !tbaa !10
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %1
  br i1 %exitcond.not, label %for.cond.cleanup10.loopexit, label %for.body11, !llvm.loop !12
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

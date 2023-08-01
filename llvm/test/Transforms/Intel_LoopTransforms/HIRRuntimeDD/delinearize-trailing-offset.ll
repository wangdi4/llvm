; RUN: opt -aa-pipeline="scoped-noalias-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-runtime-dd,hir-unroll-and-jam,print<hir>,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output < %s 2>&1 | FileCheck %s

; Verify that refs with trailing offsets are delinearized for runtime DD tests
; and vectorization is enabled after unroll and jam.
;
; In the following example, runtimeDD uses delinearized forms of ref
; (%out)[zext.i32.i64(%0) * i1 + i2].1, which is
; (%out)[zext.i32.i64(%1) + -1][sext.i32.i64(%0) + -2].1
; This delinearilzation adds test, "zext.i32.i64(%0) > 1".
;
; After unroll and jam, output depences arise between the following two refs.
;  (%out)[2 * zext.i32.i64(%0) * i1 + i2].1
;  (%out)[2 * zext.i32.i64(%0) * i1 + i2 + zext.i32.i64(%0)].1
;
; The runtime test added from delinearization, "zext.i32.i64(%0) > 1",
; is used for calculating DV (< =) between the two refs.

; CHECK: Function: test_bad
;
; CHECK:         BEGIN REGION { modified }
; CHECK:               %mv.upper.base = &((i8*)(%out)[zext.i32.i64(%1) + -1][sext.i32.i64(%0) + -2].1);
; CHECK:               %mv.test = &((%3)[zext.i32.i64(%1) + -1][sext.i32.i64(%0) + -2]) >=u &((i8*)(%out)[0][0].1);
; CHECK:               %mv.test2 = &((%mv.upper.base)[3]) >=u &((%3)[-1][0]);
; CHECK:               %mv.and = %mv.test  &  %mv.test2;
; CHECK:               if (sext.i32.i64(%2) > 1 & sext.i32.i64(%0) + -2 < sext.i32.i64(%2) & zext.i32.i64(%0) > 1 & sext.i32.i64(%0) + -2 < zext.i32.i64(%0) & %mv.and == 0)  <MVTag: 34>
;                      {
;                         %tgu = (zext.i32.i64(%1))/u2;
;
; CHECK:                  + DO i1 = 0, %tgu + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1073741823>  <LEGAL_MAX_TC = 1073741823>  <MVTag: 34, Delinearized: %3, %out> <nounroll and jam>
; CHECK:                  |   + DO i2 = 0, sext.i32.i64(%0) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 2147483646>  <LEGAL_MAX_TC = 2147483646>  <MVTag: 35>
;                         |   |   %9 = (%3)[2 * sext.i32.i64(%2) * i1 + i2 + -1 * sext.i32.i64(%2)];
;                         |   |   %10 = (%3)[2 * sext.i32.i64(%2) * i1 + i2];
; CHECK:                  |   |   (%out)[2 * zext.i32.i64(%0) * i1 + i2].1 = zext.i8.i32(%9) + -1 * zext.i8.i32(%10);
;                         |   |   %9 = (%3)[2 * sext.i32.i64(%2) * i1 + i2];
;                         |   |   %10 = (%3)[2 * sext.i32.i64(%2) * i1 + i2 + sext.i32.i64(%2)];
; CHECK:                  |   |   (%out)[2 * zext.i32.i64(%0) * i1 + i2 + zext.i32.i64(%0)].1 = zext.i8.i32(%9) + -1 * zext.i8.i32(%10);
; CHECK:                  |   + END LOOP
; CHECK:                  + END LOOP
;
;
;                         + DO i1 = 2 * %tgu, zext.i32.i64(%1) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1>  <LEGAL_MAX_TC = 1>  <MVTag: 34, Delinearized: %3, %out> <nounroll> <nounroll and jam> <max_trip_count = 1>
;                         |   + DO i2 = 0, sext.i32.i64(%0) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 2147483646>  <LEGAL_MAX_TC = 2147483646>  <MVTag: 35>
;                         |   |   %9 = (%3)[sext.i32.i64(%2) * i1 + i2 + -1 * sext.i32.i64(%2)];
;                         |   |   %10 = (%3)[sext.i32.i64(%2) * i1 + i2];
;                         |   |   (%out)[zext.i32.i64(%0) * i1 + i2].1 = zext.i8.i32(%9) + -1 * zext.i8.i32(%10);
;                         |   + END LOOP
;                         + END LOOP
;                      }
;                      else
;                      {
;                         %tgu3 = (zext.i32.i64(%1))/u2;
;
;                         + DO i1 = 0, %tgu3 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1073741823>  <LEGAL_MAX_TC = 1073741823>  <MVTag: 34> <nounroll> <nounroll and jam> <novectorize>
;                         |   + DO i2 = 0, sext.i32.i64(%0) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 2147483646>  <LEGAL_MAX_TC = 2147483646>  <MVTag: 35> <nounroll> <novectorize>
;                         |   |   %9 = (%3)[2 * sext.i32.i64(%2) * i1 + i2 + -1 * sext.i32.i64(%2)];
;                         |   |   %10 = (%3)[2 * sext.i32.i64(%2) * i1 + i2];
;                         |   |   (%out)[2 * zext.i32.i64(%0) * i1 + i2].1 = zext.i8.i32(%9) + -1 * zext.i8.i32(%10);
;                         |   |   %9 = (%3)[2 * sext.i32.i64(%2) * i1 + i2];
;                         |   |   %10 = (%3)[2 * sext.i32.i64(%2) * i1 + i2 + sext.i32.i64(%2)];
;                         |   |   (%out)[2 * zext.i32.i64(%0) * i1 + i2 + zext.i32.i64(%0)].1 = zext.i8.i32(%9) + -1 * zext.i8.i32(%10);
;                         |   + END LOOP
;                         + END LOOP
;
;
;                         + DO i1 = 2 * %tgu3, zext.i32.i64(%1) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1>  <LEGAL_MAX_TC = 1>  <MVTag: 34> <nounroll> <nounroll and jam> <novectorize> <max_trip_count = 1>
;                         |   + DO i2 = 0, sext.i32.i64(%0) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 2147483646>  <LEGAL_MAX_TC = 2147483646>  <MVTag: 35> <nounroll> <novectorize>
;                         |   |   %9 = (%3)[sext.i32.i64(%2) * i1 + i2 + -1 * sext.i32.i64(%2)];
;                         |   |   %10 = (%3)[sext.i32.i64(%2) * i1 + i2];
;                         |   |   (%out)[zext.i32.i64(%0) * i1 + i2].1 = zext.i8.i32(%9) + -1 * zext.i8.i32(%10);
;                         |   + END LOOP
;                         + END LOOP
;                      }
;                END REGION
;
;
; Verify DVs of the unroll and jammed instances of memrefs.
;
; CHECK: (%out)[2 * zext.i32.i64(%0) * i1 + i2].1 --> (%out)[2 * zext.i32.i64(%0) * i1 + i2 + zext.i32.i64(%0)].1 OUTPUT (< =) (0 0)

; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.input = type { i32, i32, i32, ptr }
%struct.output = type { float, i32 }

; Function Attrs: nofree norecurse nosync nounwind memory(read, argmem: readwrite, inaccessiblemem: none) uwtable
define dso_local void @test_bad(ptr nocapture noundef readonly %in, ptr nocapture noundef writeonly %out) local_unnamed_addr {
entry:
  %0 = load i32, ptr %in, align 8
  %height2 = getelementptr inbounds %struct.input, ptr %in, i64 0, i32 1
  %1 = load i32, ptr %height2, align 4
  %cmp42 = icmp sgt i32 %1, 0
  br i1 %cmp42, label %for.cond4.preheader.lr.ph, label %for.cond.cleanup

for.cond4.preheader.lr.ph:                        ; preds = %entry
  %stride3 = getelementptr inbounds %struct.input, ptr %in, i64 0, i32 2
  %2 = load i32, ptr %stride3, align 8
  %cmp540 = icmp sgt i32 %0, 1
  %idx.ext = sext i32 %2 to i64
  br i1 %cmp540, label %for.cond4.preheader.lr.ph.split.us, label %for.cond.cleanup

for.cond4.preheader.lr.ph.split.us:               ; preds = %for.cond4.preheader.lr.ph
  %sub = add nsw i32 %0, -1
  %data = getelementptr inbounds %struct.input, ptr %in, i64 0, i32 3
  %3 = load ptr, ptr %data, align 8
  %4 = zext i32 %0 to i64
  %wide.trip.count52 = zext i32 %1 to i64
  %wide.trip.count = sext i32 %sub to i64
  br label %for.cond4.preheader.us

for.cond4.preheader.us:                           ; preds = %for.cond4.for.cond.cleanup6_crit_edge.us, %for.cond4.preheader.lr.ph.split.us
  %indvars.iv47 = phi i64 [ %indvars.iv.next48, %for.cond4.for.cond.cleanup6_crit_edge.us ], [ 0, %for.cond4.preheader.lr.ph.split.us ]
  %5 = add nsw i64 %indvars.iv47, -1
  %6 = mul nsw i64 %5, %idx.ext
  %arrayidx13.us = getelementptr inbounds i8, ptr %3, i64 %6
  %add.ptr.us = getelementptr inbounds i8, ptr %arrayidx13.us, i64 %idx.ext
  %7 = mul nsw i64 %indvars.iv47, %4
  br label %for.body7.us

for.body7.us:                                     ; preds = %for.cond4.preheader.us, %for.body7.us
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader.us ], [ %indvars.iv.next, %for.body7.us ]
  %8 = add nsw i64 %indvars.iv, %6
  %arrayidx.us = getelementptr inbounds i8, ptr %3, i64 %8
  %9 = load i8, ptr %arrayidx.us, align 1
  %conv.us = zext i8 %9 to i32
  %arrayidx15.us = getelementptr inbounds i8, ptr %add.ptr.us, i64 %indvars.iv
  %10 = load i8, ptr %arrayidx15.us, align 1
  %conv16.us = zext i8 %10 to i32
  %sub17.us = sub nsw i32 %conv.us, %conv16.us
  %11 = add nuw nsw i64 %indvars.iv, %7
  %a.us = getelementptr inbounds %struct.output, ptr %out, i64 %11, i32 1
  store i32 %sub17.us, ptr %a.us, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond4.for.cond.cleanup6_crit_edge.us, label %for.body7.us

for.cond4.for.cond.cleanup6_crit_edge.us:         ; preds = %for.body7.us
  %indvars.iv.next48 = add nuw nsw i64 %indvars.iv47, 1
  %exitcond53.not = icmp eq i64 %indvars.iv.next48, %wide.trip.count52
  br i1 %exitcond53.not, label %for.cond.cleanup.loopexit, label %for.cond4.preheader.us, !llvm.loop !15

for.cond.cleanup.loopexit:                        ; preds = %for.cond4.for.cond.cleanup6_crit_edge.us
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %for.cond4.preheader.lr.ph, %entry
  ret void
}

!15 = distinct !{!15, !16, !17}
!16 = !{!"llvm.loop.mustprogress"}
!17 = !{!"llvm.loop.unroll_and_jam.count", i32 2}

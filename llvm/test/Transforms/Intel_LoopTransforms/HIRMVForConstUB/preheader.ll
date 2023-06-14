; RUN: opt -passes="hir-ssa-deconstruction,hir-mv-const-ub,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Test checks that we extract preheader before multiversioning of the inner loop.

; HIR before optimization:
;            BEGIN REGION { }
;                  + DO i1 = 0, %0 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>  <LEGAL_MAX_TC = 4294967295>
;                  |   %i = @llvm.umin.i32(%0,  -1 * i1);
;                  |
;                  |      %wide.trip.count.i = zext.i32.i64(%i);
;                  |   + DO i2 = 0, %wide.trip.count.i + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>  <LEGAL_MAX_TC = 4294967295> <trip_counts = 1, 2>
;                  |   |   %1 = (%agg.result)[i2];
;                  |   |   (%agg.result)[0] = %1;
;                  |   + END LOOP
;                  + END LOOP
;            END REGION

; HIR after optimization:
; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, %0 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>  <LEGAL_MAX_TC = 4294967295>
; CHECK:           |   %i = @llvm.umin.i32(%0,  -1 * i1);
; CHECK:           |   if (%i != 0)
; CHECK:           |   {
; CHECK:           |      %wide.trip.count.i = zext.i32.i64(%i);
; CHECK:           |      if (%wide.trip.count.i + -1 == 0)
; CHECK:           |      {
; CHECK:           |         + DO i2 = 0, 0, 1   <DO_LOOP>
; CHECK:           |         |   %1 = (%agg.result)[i2];
; CHECK:           |         |   (%agg.result)[0] = %1;
; CHECK:           |         + END LOOP
; CHECK:           |      }
; CHECK:           |      else
; CHECK:           |      {
; CHECK:           |         if (%wide.trip.count.i + -1 == 1)
; CHECK:           |         {
; CHECK:           |            + DO i2 = 0, 1, 1   <DO_LOOP>
; CHECK:           |            |   %1 = (%agg.result)[i2];
; CHECK:           |            |   (%agg.result)[0] = %1;
; CHECK:           |            + END LOOP
; CHECK:           |         }
; CHECK:           |         else
; CHECK:           |         {
; CHECK:           |            + DO i2 = 0, %wide.trip.count.i + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>  <LEGAL_MAX_TC = 4294967295>
; CHECK:           |            |   %1 = (%agg.result)[i2];
; CHECK:           |            |   (%agg.result)[0] = %1;
; CHECK:           |            + END LOOP
; CHECK:           |         }
; CHECK:           |      }
; CHECK:           |   }
; CHECK:           + END LOOP
; CHECK:     END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind memory(argmem: readwrite)
define void @_ZNK4llvm5APIntmlERKS0_(ptr nocapture %agg.result, i32 %0) local_unnamed_addr #0 personality ptr null {
entry:
  %cmp.i242.not = icmp eq i32 %0, 0
  br i1 %cmp.i242.not, label %invoke.cont10, label %for.body.i.preheader

for.body.i.preheader:                             ; preds = %entry
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i.preheader, %_Z.exit
  %i.0.i3 = phi i32 [ %inc.i, %_Z.exit ], [ 0, %for.body.i.preheader ]
  %sub.i = sub i32 0, %i.0.i3
  %i = tail call i32 @llvm.umin.i32(i32 %0, i32 %sub.i)
  %cmp1.not.i = icmp eq i32 %i, 0
  br i1 %cmp1.not.i, label %_Z.exit, label %for.body.preheader.i

for.body.preheader.i:                             ; preds = %for.body.i
  %wide.trip.count.i = zext i32 %i to i64
  br label %for.body.i1

for.body.i1:                                      ; preds = %for.body.i1, %for.body.preheader.i
  %indvars.iv.i = phi i64 [ 0, %for.body.preheader.i ], [ %indvars.iv.next.i, %for.body.i1 ]
  %arrayidx38.i = getelementptr i64, ptr %agg.result, i64 %indvars.iv.i
  %1 = load i64, ptr %arrayidx38.i, align 8
  store i64 %1, ptr %agg.result, align 8
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %exitcond.not.i = icmp eq i64 %indvars.iv.next.i, %wide.trip.count.i
  br i1 %exitcond.not.i, label %_Z.exit.loopexit, label %for.body.i1, !llvm.loop !0

_Z.exit.loopexit: ; preds = %for.body.i1
  br label %_Z.exit

_Z.exit:   ; preds = %_Z.exit.loopexit, %for.body.i
  %inc.i = add nuw i32 %i.0.i3, 1
  %exitcond.not = icmp eq i32 %inc.i, %0
  br i1 %exitcond.not, label %invoke.cont10.loopexit, label %for.body.i

invoke.cont10.loopexit:                           ; preds = %_Z.exit
  br label %invoke.cont10

invoke.cont10:                                    ; preds = %invoke.cont10.loopexit, %entry
  ret void
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.umin.i32(i32, i32) #1

attributes #0 = { nofree nosync nounwind memory(argmem: readwrite) "pre_loopopt" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!0 = distinct !{!0, !1, !2}
!1 = !{!"llvm.loop.mustprogress"}
!2 = !{!"llvm.loop.intel.loopcount", i32 1, i32 2}

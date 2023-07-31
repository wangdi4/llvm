; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll,print<hir-framework>,hir-dead-store-elimination,print<hir-framework>" 2>&1 < %s | FileCheck %s

; Verify that the store of (%0)[i2] in the first i2 loop after complete unroll
; is not removed by dead store elimination because the base pointer is
; redefined in between the loops.

; Incoming HIR-
; + DO i1 = 0, 49, 1   <DO_LOOP>
; |   + DO i2 = 0, 1, 1   <DO_LOOP> <unroll = 2>
; |   |   %0 = (%B)[i2];
; |   |
; |   |   + DO i3 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; |   |   |   (%0)[i3] = i2;
; |   |   + END LOOP
; |   + END LOOP
; + END LOOP

; Dump after complete unroll of i2 loop

; CHECK: + DO i1 = 0, 49, 1   <DO_LOOP>
; CHECK: |   %0 = (%B)[0];
; CHECK: |
; CHECK: |   + DO i2 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK: |   |   (%0)[i2] = 0;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %0 = (%B)[1];
; CHECK: |
; CHECK: |   + DO i2 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK: |   |   (%0)[i2] = 1;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; Dump after dead store elimination

; CHECK: + DO i1 = 0, 49, 1   <DO_LOOP>
; CHECK: |   %0 = (%B)[0];
; CHECK: |
; CHECK: |   + DO i2 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK: |   |   (%0)[i2] = 0;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %0 = (%B)[1];
; CHECK: |
; CHECK: |   + DO i2 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK: |   |   (%0)[i2] = 1;
; CHECK: |   + END LOOP
; CHECK: + END LOOP



target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo(ptr noalias %A, ptr noalias %B, i32 %n) local_unnamed_addr {
entry:
  %cmp523 = icmp sgt i32 %n, 0
  br i1 %cmp523, label %for.cond1.preheader.us.preheader, label %for.end14

for.cond1.preheader.us.preheader:                 ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.cond1.preheader.us

for.cond1.preheader.us:                           ; preds = %for.inc12.us-lcssa.us.us, %for.cond1.preheader.us.preheader
  %k.027.us = phi i32 [ %inc13.us, %for.inc12.us-lcssa.us.us ], [ 0, %for.cond1.preheader.us.preheader ]
  br label %for.body3.us.us

for.body3.us.us:                                  ; preds = %for.cond4.for.inc9_crit_edge.us.us, %for.cond1.preheader.us
  %indvars.iv47 = phi i64 [ 0, %for.cond1.preheader.us ], [ %indvars.iv.next48, %for.cond4.for.inc9_crit_edge.us.us ]
  %arrayidx.us.us = getelementptr inbounds ptr, ptr %B, i64 %indvars.iv47
  %0 = load ptr, ptr %arrayidx.us.us, align 8, !tbaa !2
  %1 = trunc i64 %indvars.iv47 to i32
  br label %for.body6.us.us

for.cond4.for.inc9_crit_edge.us.us:               ; preds = %for.body6.us.us
  %indvars.iv.next48 = add nuw nsw i64 %indvars.iv47, 1
  %exitcond49 = icmp eq i64 %indvars.iv.next48, 2
  br i1 %exitcond49, label %for.inc12.us-lcssa.us.us, label %for.body3.us.us, !llvm.loop !6

for.body6.us.us:                                  ; preds = %for.body6.us.us, %for.body3.us.us
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body6.us.us ], [ 0, %for.body3.us.us ]
  %arrayidx8.us.us = getelementptr inbounds i32, ptr %0, i64 %indvars.iv
  store i32 %1, ptr %arrayidx8.us.us, align 4, !tbaa !8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond4.for.inc9_crit_edge.us.us, label %for.body6.us.us

for.inc12.us-lcssa.us.us:                         ; preds = %for.cond4.for.inc9_crit_edge.us.us
  %inc13.us = add nuw nsw i32 %k.027.us, 1
  %exitcond50 = icmp eq i32 %inc13.us, 50
  br i1 %exitcond50, label %for.end14.loopexit, label %for.cond1.preheader.us

for.end14.loopexit:                               ; preds = %for.inc12.us-lcssa.us.us
  br label %for.end14

for.end14:                                        ; preds = %for.end14.loopexit, %entry
  ret void
}


!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPi", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.unroll.count", i32 2}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !4, i64 0}

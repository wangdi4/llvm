; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,print<hir>" -disable-output 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-post-vec-complete-unroll,print<hir>"  -disable-output 2>&1 < %s | FileCheck %s

; This test case checks that the outer loops won't be marked as multi-exit
; after unrolling completely the inner loop. The reason is that the redundant
; condition 'i2 > 32' will be removed, which will remove the goto.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, %m, 1   <DO_MULTI_EXIT_LOOP>
;       |   + DO i2 = 0, %n + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   |   (%A)[0] = 0;
;       |   |
;       |   |   + DO i3 = 0, 3, 1   <DO_MULTI_EXIT_LOOP> <unroll>
;       |   |   |   if ((%A)[i3] == 2)
;       |   |   |   {
;       |   |   |      if (i3 > 32)
;       |   |   |      {
;       |   |   |         goto for.end.split.loopexit;
;       |   |   |      }
;       |   |   |      (%A)[i3] = 0;
;       |   |   |   }
;       |   |   + END LOOP
;       |   + END LOOP
;       + END LOOP
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, %m, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   |   (%A)[0] = 0;
; CHECK:       |   |   if ((%A)[0] == 2)
; CHECK:       |   |   {
; CHECK:       |   |      (%A)[0] = 0;
; CHECK:       |   |   }
; CHECK:       |   |   if ((%A)[1] == 2)
; CHECK:       |   |   {
; CHECK:       |   |      (%A)[1] = 0;
; CHECK:       |   |   }
; CHECK:       |   |   if ((%A)[2] == 2)
; CHECK:       |   |   {
; CHECK:       |   |      (%A)[2] = 0;
; CHECK:       |   |   }
; CHECK:       |   |   if ((%A)[3] == 2)
; CHECK:       |   |   {
; CHECK:       |   |      (%A)[3] = 0;
; CHECK:       |   |   }
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr nocapture %A, i32 %n, i32 %m) {
entry:
  %cmp16 = icmp sgt i32 %n, 0
  br i1 %cmp16, label %for.body.lr.ph, label %for.end8

for.body.lr.ph:                                   ; preds = %entry
  br label %for.outer.body

for.outer.body:
  %iv.outer = phi i32 [ 0, %for.body.lr.ph ], [ %inc.outer, %for.inc5 ]
  br label %for.body

for.body:                                         ; preds = %for.inc6, %for.body.lr.ph
  %i.017 = phi i32 [ 0, %for.outer.body ], [ %inc7, %for.inc6 ]
  store i32 0, ptr %A
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.inc8 ]
  %arrayidx4 = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx4
  %cmp2 = icmp eq i32 %0, 2
  br i1 %cmp2, label %if.then, label %for.inc8

if.then:
  %cmp3 = icmp sgt i64 %indvars.iv, 32
  br i1 %cmp3, label %for.end.split.loopexit, label %if.cont

if.cont:
  store i32 0, ptr %arrayidx4
  br label %for.inc8

for.inc8:
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.inc6, label %for.body3, !llvm.loop !0

for.inc6:                                         ; preds = %for.body3
  %inc7 = add nuw nsw i32 %i.017, 1
  %exitcond18 = icmp eq i32 %inc7, %n
  br i1 %exitcond18, label %for.inc5, label %for.body

for.inc5:                                         ; preds = %for.body3
  %inc.outer = add nuw nsw i32 %iv.outer, 1
  %exitcond2 = icmp eq i32 %iv.outer, %m
  br i1 %exitcond2, label %for.end8.loopexit, label %for.outer.body

for.end.split.loopexit:                                ; preds = %for.inc6
  unreachable

for.end8.loopexit:                                ; preds = %for.inc6
  br label %for.end8

for.end8:                                         ; preds = %for.end8.loopexit, %entry
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.full"}
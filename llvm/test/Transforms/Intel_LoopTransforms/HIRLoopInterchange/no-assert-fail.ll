; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-interchange,print<hir>" -debug-only=hir-loop-interchange < %s 2>&1 | FileCheck %s

; Make sure the inputs get interchanged without any issue. Notice that the loop level starts from 2 not 1.

; CHECK: Function: foo
; CHECK:          BEGIN REGION { }
; CHECK:                + DO i1 = 0, 0, 1   <DO_LOOP>
; CHECK:                |   %tmp = (undef)[0];
; CHECK:                |
; CHECK:                |   + DO i2 = 0, (smax(4, undef) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 536870911>
; CHECK:                |   |   + DO i3 = 0, 3, 1   <DO_LOOP>
; CHECK:                |   |   |   + DO i4 = 0, 2, 1   <DO_LOOP>
; CHECK:                |   |   |   |   %tmp25 = undef  -  (%tmp)[24 * i2 + i3 + 24 * (undef /u 4)];
; CHECK:                |   |   |   |   (undef)[0][i4] = undef;
; CHECK:                |   |   |   + END LOOP
; CHECK:                |   |   + END LOOP
; CHECK:                |   + END LOOP
; CHECK:                + END LOOP
; CHECK:          END REGION

; CHECK: Loopnest Interchanged: ( 2 3 4 ) --> ( 2 4 3 )

; CHECK: Function: foo
; CHECK:          BEGIN REGION { modified }
; CHECK:                + DO i1 = 0, 0, 1   <DO_LOOP>
; CHECK:                |   %tmp = (undef)[0];
; CHECK:                |
; CHECK:                |   + DO i2 = 0, (smax(4, undef) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 536870911>
; CHECK:                |   |   + DO i3 = 0, 2, 1   <DO_LOOP>
; CHECK:                |   |   |   + DO i4 = 0, 3, 1   <DO_LOOP>
; CHECK:                |   |   |   |   %tmp25 = undef  -  (%tmp)[24 * i2 + i4 + 24 * (undef /u 4)];
; CHECK:                |   |   |   |   (undef)[0][i3] = undef;
; CHECK:                |   |   |   + END LOOP
; CHECK:                |   |   + END LOOP
; CHECK:                |   + END LOOP
; CHECK:                + END LOOP
; CHECK:          END REGION

; ModuleID = 'bugpoint-reduced-simplified.ll'
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo() local_unnamed_addr #0 personality ptr @pluto {
bb:
  br label %bb1

bb1:                                              ; preds = %bb3, %bb
  %tmp = load ptr, ptr undef, align 8
  br label %bb5

bb2:                                              ; preds = %bb3
  ret void

bb3:                                              ; preds = %bb13
  %tmp4 = icmp eq i64 undef, undef
  br i1 %tmp4, label %bb2, label %bb1

bb5:                                              ; preds = %bb13, %bb1
  %tmp6 = phi i32 [ 0, %bb1 ], [ %tmp14, %bb13 ]
  %tmp7 = add nuw nsw i32 %tmp6, undef
  %tmp8 = lshr exact i32 %tmp7, 2
  %tmp9 = mul nuw nsw i32 %tmp8, 24
  br label %bb10

bb10:                                             ; preds = %bb16, %bb5
  %tmp11 = phi i32 [ 0, %bb5 ], [ %tmp17, %bb16 ]
  %tmp12 = add nuw i32 %tmp11, %tmp9
  br label %bb19

bb13:                                             ; preds = %bb16
  %tmp14 = add nuw nsw i32 %tmp6, 4
  %tmp15 = icmp slt i32 %tmp14, undef
  br i1 %tmp15, label %bb5, label %bb3

bb16:                                             ; preds = %bb19
  %tmp17 = add nuw nsw i32 %tmp11, 1
  %tmp18 = icmp eq i32 %tmp17, 4
  br i1 %tmp18, label %bb13, label %bb10

bb19:                                             ; preds = %bb19, %bb10
  %tmp20 = phi i64 [ 0, %bb10 ], [ %tmp27, %bb19 ]
  %tmp21 = add i32 %tmp12, 0
  %tmp22 = zext i32 %tmp21 to i64
  %tmp23 = getelementptr inbounds float, ptr %tmp, i64 %tmp22
  %tmp24 = load float, ptr %tmp23, align 4
  %tmp25 = fsub fast float undef, %tmp24
  %tmp26 = getelementptr inbounds [3 x double], ptr undef, i64 0, i64 %tmp20
  store double undef, ptr %tmp26, align 8
  %tmp27 = add nuw nsw i64 %tmp20, 1
  %tmp28 = icmp eq i64 %tmp27, 3
  br i1 %tmp28, label %bb16, label %bb19, !llvm.loop !0
}

declare dso_local i32 @pluto(...)

attributes #0 = { "use-soft-float"="false" }

!nvvm.annotations = !{}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.mustprogress"}

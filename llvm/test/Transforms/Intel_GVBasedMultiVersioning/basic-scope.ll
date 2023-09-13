; Test that multiversioning can be restricted to part of a function.
; This test currently only checks debug log until the transformation part of
; scoped multiversioning is implemented.
; RUN: opt -S -passes=gvbased-multiversioning -gvbased-multiversion-min-num-branches=3 -debug < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global1 = internal unnamed_addr global i1 false, align 1

declare void @unknown_side_effect() local_unnamed_addr #0

define void @basic_scope(ptr nocapture noundef %num) #0 {
; CHECK: GVMV analysis for basic_scope created invariant set: (global1 = 0) Scope: from   %.b1213 = load i1, ptr @global1, align 1 to   tail call void @unknown_side_effect()
entry:
  %num.1 = load i32, ptr %num, align 4, !tbaa !0
  %num.1.inc = add nsw i32 %num.1, 1
  store i32 %num.1.inc, ptr %num, align 4, !tbaa !0
  tail call void @unknown_side_effect()
; The scope should start below
  %.b1213 = load i1, ptr @global1, align 1
  br i1 %.b1213, label %if.end, label %if.end.thread

if.end.thread:                                    ; preds = %entry
  %0 = load i32, ptr %num, align 4, !tbaa !0
  br label %if.end4.thread

if.end:                                           ; preds = %entry
  tail call void @unknown_side_effect()
  %.b1114.pr = load i1, ptr @global1, align 1
  %num.2 = load i32, ptr %num, align 4, !tbaa !0
  %num.2.inc = add nsw i32 %num.2, 1
  store i32 %num.2.inc, ptr %num, align 4, !tbaa !0
  br i1 %.b1114.pr, label %if.end4, label %if.end4.thread

if.end4.thread:                                   ; preds = %if.end, %if.end.thread
  %1 = phi i32 [ %0, %if.end.thread ], [ %num.2.inc, %if.end ]
  %inc3 = add nsw i32 %1, 2
  store i32 %inc3, ptr %num, align 4, !tbaa !0
  br label %if.end7

if.end4:                                          ; preds = %if.end
  tail call void @unknown_side_effect()
  %.b15.pre = load i1, ptr @global1, align 1
  br i1 %.b15.pre, label %if.then6, label %if.end7

if.then6:                                         ; preds = %if.end4
  tail call void @unknown_side_effect()
  br label %if.end7

if.end7:                                          ; preds = %if.end4.thread, %if.then6, %if.end4
; Scope ends here
  tail call void @unknown_side_effect()
  %num.3 = load i32, ptr %num, align 4, !tbaa !0
  %num.3.inc = add nsw i32 %num.3, 1
  store i32 %num.3.inc, ptr %num, align 4, !tbaa !0
  ret void
}

; Scoped multiversioning will bail out because the target instruction appears
; in the middle of multiple loads of global1.
define void @not_profitable(ptr nocapture noundef %num) {
; CHECK: GVMV analysis for not_profitable bails out, unable to build a valid invariant set
entry:
  %num.1 = load i32, ptr %num, align 4, !tbaa !0
  %num.1.inc = add nsw i32 %num.1, 1
  store i32 %num.1.inc, ptr %num, align 4, !tbaa !0
  %.b1213 = load i1, ptr @global1, align 1
  br i1 %.b1213, label %if.end, label %if.end.thread

if.end.thread:                                    ; preds = %entry
  %0 = load i32, ptr %num, align 4, !tbaa !0
  br label %if.end4.thread

if.end:                                           ; preds = %entry
  tail call void @unknown_side_effect()
  %.b1114.pr = load i1, ptr @global1, align 1
  %num.2 = load i32, ptr %num, align 4, !tbaa !0
  %num.2.inc = add nsw i32 %num.2, 1
  store i32 %num.2.inc, ptr %num, align 4, !tbaa !0
  br i1 %.b1114.pr, label %if.end4, label %if.end4.thread

if.end4.thread:                                   ; preds = %if.end, %if.end.thread
  %1 = phi i32 [ %0, %if.end.thread ], [ %num.2.inc, %if.end ]
  %inc3 = add nsw i32 %1, 2
  store i32 %inc3, ptr %num, align 4, !tbaa !0
; This call blocks multiversioning
  tail call void @unknown_side_effect()
  br label %if.end7

if.end4:                                          ; preds = %if.end
  tail call void @unknown_side_effect()
  %.b15.pre = load i1, ptr @global1, align 1
  br i1 %.b15.pre, label %if.then6, label %if.end7

if.then6:                                         ; preds = %if.end4
  tail call void @unknown_side_effect()
  br label %if.end7

if.end7:                                          ; preds = %if.end4.thread, %if.then6, %if.end4
  %num.3 = load i32, ptr %num, align 4, !tbaa !0
  %num.3.inc = add nsw i32 %num.3, 1
  store i32 %num.3.inc, ptr %num, align 4, !tbaa !0
  ret void
}

!0 = !{!1, !1, i64 0}
!1 = !{!"int", !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C/C++ TBAA"}

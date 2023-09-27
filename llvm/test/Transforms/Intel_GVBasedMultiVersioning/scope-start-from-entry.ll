; Ensure scoped multiversioning work with scope starting from function entry.
; RUN: opt -S -passes=gvbased-multiversioning -gvbased-multiversion-min-num-branches=3 -debug < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global1 = internal global i1 false, align 1

declare void @unknown_side_effect() #0

; CHECK: GVMV analysis for scope_start_from_entry created invariant set: (global1 = 0) Scope: from   %num.1 = load i32, ptr %num, align 4 to   tail call void @unknown_side_effect()

define void @scope_start_from_entry(ptr nocapture noundef %num) #0 {
; CHECK-LABEL: define void @scope_start_from_entry(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[MV_SCOPED_LOAD_GLOBAL1:%.*]] = load i1, ptr @global1, align 1
; CHECK-NEXT:    [[TMP0:%.*]] = xor i1 [[MV_SCOPED_LOAD_GLOBAL1]], true
; CHECK-NEXT:    br i1 [[TMP0]], label %mv.scope.entry.clone, label %mv.scope.entry
; CHECK:       mv.scope.entry:
; CHECK-NEXT:    [[NUM_1:%.*]] = load i32, ptr %num, align 4
; CHECK-NEXT:    [[NUM_1_INC:%.*]] = add nsw i32 [[NUM_1]], 1
; CHECK-NEXT:    store i32 [[NUM_1_INC]], ptr %num, align 4
; CHECK-NEXT:    [[DOTB1213:%.*]] = load i1, ptr @global1, align 1
; CHECK-NEXT:    br i1 [[DOTB1213]], label %if.end, label %if.end.thread
; CHECK:       if.end.thread:
; CHECK-NEXT:    [[TMP1:%.*]] = load i32, ptr %num, align 4
; CHECK-NEXT:    br label %if.end4.thread
; CHECK:       if.end:
; CHECK-NEXT:    tail call void @unknown_side_effect()
; CHECK-NEXT:    [[DOTB1114_PR:%.*]] = load i1, ptr @global1, align 1
; CHECK-NEXT:    [[NUM_2:%.*]] = load i32, ptr %num, align 4
; CHECK-NEXT:    [[NUM_2_INC:%.*]] = add nsw i32 [[NUM_2]], 1
; CHECK-NEXT:    store i32 [[NUM_2_INC]], ptr %num, align 4
; CHECK-NEXT:    br i1 [[DOTB1114_PR]], label %if.end4, label %if.end4.thread
; CHECK:       if.end4.thread:
; CHECK-NEXT:    [[TMP2:%.*]] = phi i32 [ [[TMP1]], %if.end.thread ], [ [[NUM_2_INC]], %if.end ]
; CHECK-NEXT:    [[INC3:%.*]] = add nsw i32 [[TMP2]], 2
; CHECK-NEXT:    store i32 [[INC3]], ptr %num, align 4
; CHECK-NEXT:    br label %mv.scope.exit
; CHECK:       if.end4:
; CHECK-NEXT:    tail call void @unknown_side_effect()
; CHECK-NEXT:    [[DOTB15_PRE:%.*]] = load i1, ptr @global1, align 1
; CHECK-NEXT:    br i1 [[DOTB15_PRE]], label %if.then6, label %mv.scope.exit
; CHECK:       if.then6:
; CHECK-NEXT:    tail call void @unknown_side_effect()
; CHECK-NEXT:    br label %mv.scope.exit
; CHECK:       mv.scope.exit:
; CHECK-NEXT:    tail call void @unknown_side_effect()
; CHECK-NEXT:    [[NUM_3:%.*]] = load i32, ptr %num, align 4
; CHECK-NEXT:    [[NUM_3_INC:%.*]] = add nsw i32 [[NUM_3]], 1
; CHECK-NEXT:    store i32 [[NUM_3_INC]], ptr %num, align 4
; CHECK-NEXT:    ret void
; CHECK:       mv.scope.entry.clone:
; CHECK-NEXT:    [[NUM_1_CLONE:%.*]] = load i32, ptr %num, align 4
; CHECK-NEXT:    [[NUM_1_INC_CLONE:%.*]] = add nsw i32 [[NUM_1_CLONE]], 1
; CHECK-NEXT:    store i32 [[NUM_1_INC_CLONE]], ptr %num, align 4
; CHECK-NEXT:    [[TMP3:%.*]] = load i32, ptr %num, align 4
; CHECK-NEXT:    [[INC3_CLONE:%.*]] = add nsw i32 [[TMP3]], 2
; CHECK-NEXT:    store i32 [[INC3_CLONE]], ptr %num, align 4
; CHECK-NEXT:    br label %mv.scope.exit
;
entry:
  %num.1 = load i32, ptr %num, align 4
  %num.1.inc = add nsw i32 %num.1, 1
  store i32 %num.1.inc, ptr %num, align 4
  %.b1213 = load i1, ptr @global1, align 1
  br i1 %.b1213, label %if.end, label %if.end.thread

if.end.thread:
  %0 = load i32, ptr %num, align 4
  br label %if.end4.thread

if.end:
  tail call void @unknown_side_effect()
  %.b1114.pr = load i1, ptr @global1, align 1
  %num.2 = load i32, ptr %num, align 4
  %num.2.inc = add nsw i32 %num.2, 1
  store i32 %num.2.inc, ptr %num, align 4
  br i1 %.b1114.pr, label %if.end4, label %if.end4.thread

if.end4.thread:
  %1 = phi i32 [ %0, %if.end.thread ], [ %num.2.inc, %if.end ]
  %inc3 = add nsw i32 %1, 2
  store i32 %inc3, ptr %num, align 4
  br label %if.end7

if.end4:
  tail call void @unknown_side_effect()
  %.b15.pre = load i1, ptr @global1, align 1
  br i1 %.b15.pre, label %if.then6, label %if.end7

if.then6:
  tail call void @unknown_side_effect()
  br label %if.end7

if.end7:
  tail call void @unknown_side_effect()
  %num.3 = load i32, ptr %num, align 4
  %num.3.inc = add nsw i32 %num.3, 1
  store i32 %num.3.inc, ptr %num, align 4
  ret void
}

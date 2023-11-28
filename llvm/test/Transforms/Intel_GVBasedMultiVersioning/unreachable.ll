; RUN: opt -S -passes=gvbased-multiversioning -gvbased-multiversion-min-num-branches=3 < %s | FileCheck %s
; Check that scoped multiversioning can work with IR with unreachables in some cases.
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global1 = global i1 false, align 2
@num = external global i32, align 4

; The IR is generated from the following C code:
; int mv_with_unreachable(int n, int in) {
;   num += 1;
;   unknown_side_effect();
;   switch (n) {
;     case 0: {
;       if (global1) {
;         num += 1;
;         unknown_side_effect();
;       } else {
;         num += 2;
;       }
;       break;
;     }
;     case 1: {
;       if (global1) {
;         num += 1;
;         unknown_side_effect();
;       } else {
;         num += 2;
;       }
;       break;
;     }
;     case 2: {
;       if (global1) {
;         in |= 0xFFFF;
;         num += 1;
;         unknown_side_effect();
;       } else {
;         in &= 0xFFFF;
;         num += 2;
;       }
;       break;
;     }
;     default:
;       __builtin_unreachable();
;   }
;   unknown_side_effect();
;   num += 1;
;   return in;
; }
define i32 @mv_with_unreachable(i32 noundef %n, i32 noundef %in) {
; CHECK-LABEL: define i32 @mv_with_unreachable(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = load i32, ptr @num, align 4
; CHECK-NEXT:    [[ADD:%.*]] = add nsw i32 [[TMP0]], 1
; CHECK-NEXT:    store i32 [[ADD]], ptr @num, align 4
; CHECK-NEXT:    tail call void @unknown_side_effect()
; CHECK-NEXT:    [[MV_SCOPED_LOAD_GLOBAL1:%.*]] = load i1, ptr @global1, align 1
; CHECK-NEXT:    [[TMP1:%.*]] = xor i1 [[MV_SCOPED_LOAD_GLOBAL1]], true
; CHECK-NEXT:    br i1 [[TMP1]], label %mv.scope.entry.clone, label %mv.scope.entry
; CHECK:       mv.scope.entry:
; CHECK-NEXT:    switch i32 %n, label %sw.default [
; CHECK-NEXT:    i32 0, label %sw.bb
; CHECK-NEXT:    i32 1, label %sw.bb3
; CHECK-NEXT:    i32 2, label %sw.bb10
; CHECK-NEXT:    ]
; CHECK:       sw.bb10:
; CHECK-NEXT:    [[DOTB:%.*]] = load i1, ptr @global1, align 2
; CHECK-NEXT:    [[TMP4:%.*]] = load i32, ptr @num, align 4
; CHECK-NEXT:    br i1 [[DOTB]], label %if.then12, label %if.else14
; CHECK:       if.then12:
; CHECK-NEXT:    [[OR:%.*]] = or i32 %in, 65535
; CHECK-NEXT:    [[ADD13:%.*]] = add nsw i32 [[TMP4]], 1
; CHECK-NEXT:    store i32 [[ADD13]], ptr @num, align 4
; CHECK-NEXT:    tail call void @unknown_side_effect()
; CHECK-NEXT:    br label %mv.scope.exit
; CHECK:       if.else14:
; CHECK-NEXT:    [[AND:%.*]] = and i32 %in, 65535
; CHECK-NEXT:    [[ADD15:%.*]] = add nsw i32 [[TMP4]], 2
; CHECK-NEXT:    store i32 [[ADD15]], ptr @num, align 4
; CHECK-NEXT:    br label %mv.scope.exit
; CHECK:       sw.default:
; CHECK-NEXT:    unreachable
; CHECK:       mv.scope.exit:
; CHECK-NEXT:    [[IN_ADDR_0_MV_LIVEOUT:%.*]] = phi i32 [ [[OR]], %if.then12 ], [ [[AND]], %if.else14 ], [ %in, %if.then5 ], [ %in, %if.else7 ], [ %in, %if.then ], [ %in, %if.else ], [ [[AND_CLONE:%.*]], %sw.bb10.clone ], [ %in, %sw.bb3.clone ], [ %in, %sw.bb.clone ]
; CHECK-NEXT:    tail call void @unknown_side_effect()
; CHECK-NEXT:    [[TMP5:%.*]] = load i32, ptr @num, align 4
; CHECK-NEXT:    [[ADD17:%.*]] = add nsw i32 [[TMP5]], 1
; CHECK-NEXT:    store i32 [[ADD17]], ptr @num, align 4
; CHECK-NEXT:    ret i32 [[IN_ADDR_0_MV_LIVEOUT]]
; CHECK:       mv.scope.entry.clone:
; CHECK-NEXT:    switch i32 %n, label %sw.default.clone [
; CHECK-NEXT:    i32 0, label %sw.bb.clone
; CHECK-NEXT:    i32 1, label %sw.bb3.clone
; CHECK-NEXT:    i32 2, label %sw.bb10.clone
; CHECK-NEXT:    ]
; CHECK:       sw.bb10.clone:
; CHECK-NEXT:    [[TMP8:%.*]] = load i32, ptr @num, align 4
; CHECK-NEXT:    [[AND_CLONE]] = and i32 %in, 65535
; CHECK-NEXT:    [[ADD15_CLONE:%.*]] = add nsw i32 [[TMP8]], 2
; CHECK-NEXT:    store i32 [[ADD15_CLONE]], ptr @num, align 4
; CHECK-NEXT:    br label %mv.scope.exit
; CHECK:       sw.default.clone:
; CHECK-NEXT:    unreachable
;
entry:
  %0 = load i32, ptr @num, align 4
  %add = add nsw i32 %0, 1
  store i32 %add, ptr @num, align 4
  tail call void @unknown_side_effect()
  switch i32 %n, label %sw.default [
  i32 0, label %sw.bb
  i32 1, label %sw.bb3
  i32 2, label %sw.bb10
  ]

sw.bb:
  %.b21 = load i1, ptr @global1, align 2
  %1 = load i32, ptr @num, align 4
  br i1 %.b21, label %if.then, label %if.else

if.then:
  %add1 = add nsw i32 %1, 1
  store i32 %add1, ptr @num, align 4
  tail call void @unknown_side_effect()
  br label %sw.epilog

if.else:
  %add2 = add nsw i32 %1, 2
  store i32 %add2, ptr @num, align 4
  br label %sw.epilog

sw.bb3:
  %.b20 = load i1, ptr @global1, align 2
  %2 = load i32, ptr @num, align 4
  br i1 %.b20, label %if.then5, label %if.else7

if.then5:
  %add6 = add nsw i32 %2, 1
  store i32 %add6, ptr @num, align 4
  tail call void @unknown_side_effect()
  br label %sw.epilog

if.else7:
  %add8 = add nsw i32 %2, 2
  store i32 %add8, ptr @num, align 4
  br label %sw.epilog

sw.bb10:
  %.b = load i1, ptr @global1, align 2
  %3 = load i32, ptr @num, align 4
  br i1 %.b, label %if.then12, label %if.else14

if.then12:
  %or = or i32 %in, 65535
  %add13 = add nsw i32 %3, 1
  store i32 %add13, ptr @num, align 4
  tail call void @unknown_side_effect()
  br label %sw.epilog

if.else14:
  %and = and i32 %in, 65535
  %add15 = add nsw i32 %3, 2
  store i32 %add15, ptr @num, align 4
  br label %sw.epilog

sw.default:
  unreachable

sw.epilog:
  %in.addr.0 = phi i32 [ %or, %if.then12 ], [ %and, %if.else14 ], [ %in, %if.then5 ], [ %in, %if.else7 ], [ %in, %if.then ], [ %in, %if.else ]
  tail call void @unknown_side_effect()
  %4 = load i32, ptr @num, align 4
  %add17 = add nsw i32 %4, 1
  store i32 %add17, ptr @num, align 4
  ret i32 %in.addr.0
}

declare void @unknown_side_effect()

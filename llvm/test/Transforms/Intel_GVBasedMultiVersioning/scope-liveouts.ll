; Check that PHI instructions are generated for live-outs in scoped multiversioning.
; RUN: opt -S -passes=gvbased-multiversioning,verify -gvbased-multiversion-min-num-branches=3 < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global1 = global i1 false, align 2
@num = external global i32, align 4

; The IR is generated from the following C code:
; int basic_scope(int in) {
;   num += 1;
;   unknown_side_effect();
;   if (global1) {
;     num += 1;
;     unknown_side_effect();
;   } else {
;     num += 2;
;   }
;   if (global1) {
;     num += 1;
;     unknown_side_effect();
;   } else {
;     num += 2;
;   }
;   if (global1) {
;     in |= 0xFFFF;
;     num += 1;
;     unknown_side_effect();
;   } else {
;     in &= 0xFFFF;
;     num += 2;
;   }
;   unknown_side_effect();
;   num += 1;
;   return in;
; }
define i32 @basic_scope(i32 noundef %in) {
; CHECK-LABEL: define i32 @basic_scope(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[LOAD1:%.*]] = load i32, ptr @num, align 4
; CHECK-NEXT:    [[ADD1:%.*]] = add nsw i32 [[LOAD1]], 1
; CHECK-NEXT:    store i32 [[ADD1]], ptr @num, align 4
; CHECK-NEXT:    tail call void @unknown_side_effect()
; CHECK-NEXT:    [[MV_SCOPED_LOAD_GLOBAL1:%.*]] = load i1, ptr @global1, align 1
; CHECK-NEXT:    [[COND:%.*]] = xor i1 [[MV_SCOPED_LOAD_GLOBAL1]], true
; CHECK-NEXT:    br i1 [[COND]], label %mv.scope.entry.clone, label %mv.scope.entry
; CHECK:       mv.scope.entry:
; CHECK-NEXT:    {{.*}} = load i1, ptr @global1, align 2
; CHECK:       if.then4:
; CHECK-NEXT:    [[LOAD2:%.*]] = load i32, ptr @num, align 4
; CHECK-NEXT:    [[ADD2:%.*]] = add nsw i32 [[LOAD2]], 1
; CHECK-NEXT:    store i32 [[ADD2]], ptr @num, align 4
; CHECK-NEXT:    tail call void @unknown_side_effect()
; CHECK-NEXT:    [[DOTB_PRE:%.*]] = load i1, ptr @global1, align 2
; CHECK-NEXT:    br i1 [[DOTB_PRE]], label %if.then10, label %if.else12
; CHECK:       if.then10:
; CHECK-NEXT:    [[OR:%.*]] = or i32 %in, 65535
; CHECK:         tail call void @unknown_side_effect()
; CHECK-NEXT:    br label %mv.scope.exit
; CHECK:       if.else12:
; CHECK-NEXT:    [[AND:%.*]] = and i32 %in, 65535
; CHECK:        br label %mv.scope.exit
; CHECK:       mv.scope.exit:
; CHECK-NEXT:    [[IN_ADDR_0_MV_LIVEOUT:%.*]] = phi i32 [ [[AND_CLONE:%.*]], %mv.scope.entry.clone ], [ [[OR]], %if.then10 ], [ [[AND]], %if.else12 ]
; CHECK-NEXT:    tail call void @unknown_side_effect()
; CHECK-NEXT:    [[LOAD3:%.*]] = load i32, ptr @num, align 4
; CHECK-NEXT:    [[ADD3:%.*]] = add nsw i32 [[LOAD3]], 1
; CHECK-NEXT:    store i32 [[ADD3]], ptr @num, align 4
; CHECK-NEXT:    ret i32 [[IN_ADDR_0_MV_LIVEOUT]]
; CHECK:       mv.scope.entry.clone:
; CHECK-NEXT:    [[LOAD4:%.*]] = load i32, ptr @num, align 4
; CHECK-NEXT:    [[ADD4:%.*]] = add nsw i32 [[LOAD4]], 2
; CHECK-NEXT:    store i32 [[ADD4]], ptr @num, align 4
; CHECK-NEXT:    [[LOAD5:%.*]] = load i32, ptr @num, align 4
; CHECK-NEXT:    [[ADD5:%.*]] = add nsw i32 [[LOAD5]], 2
; CHECK-NEXT:    store i32 [[ADD5]], ptr @num, align 4
; CHECK-NEXT:    [[AND_CLONE]] = and i32 %in, 65535
; CHECK-NEXT:    [[LOAD6:%.*]] = load i32, ptr @num, align 4
; CHECK-NEXT:    [[ADD7:%.*]] = add nsw i32 [[LOAD6]], 2
; CHECK-NEXT:    store i32 [[ADD7]], ptr @num, align 4
; CHECK-NEXT:    br label %mv.scope.exit
;
entry:
  %0 = load i32, ptr @num, align 4
  %add = add nsw i32 %0, 1
  store i32 %add, ptr @num, align 4
  tail call void @unknown_side_effect()
  %.b19 = load i1, ptr @global1, align 2
  %1 = load i32, ptr @num, align 4
  br i1 %.b19, label %if.then, label %if.else

if.then:
  %add1 = add nsw i32 %1, 1
  store i32 %add1, ptr @num, align 4
  tail call void @unknown_side_effect()
  %.b18.pre = load i1, ptr @global1, align 2
  br i1 %.b18.pre, label %if.then4, label %if.else6

if.else:
  %add2 = add nsw i32 %1, 2
  store i32 %add2, ptr @num, align 4
  br label %if.else6

if.then4:
  %2 = load i32, ptr @num, align 4
  %add5 = add nsw i32 %2, 1
  store i32 %add5, ptr @num, align 4
  tail call void @unknown_side_effect()
  %.b.pre = load i1, ptr @global1, align 2
  br i1 %.b.pre, label %if.then10, label %if.else12

if.else6:
  %3 = load i32, ptr @num, align 4
  %add7 = add nsw i32 %3, 2
  store i32 %add7, ptr @num, align 4
  br label %if.else12

if.then10:
  %or = or i32 %in, 65535
  %4 = load i32, ptr @num, align 4
  %add11 = add nsw i32 %4, 1
  store i32 %add11, ptr @num, align 4
  tail call void @unknown_side_effect()
  br label %if.end14

if.else12:
  %and = and i32 %in, 65535
  %5 = load i32, ptr @num, align 4
  %add13 = add nsw i32 %5, 2
  store i32 %add13, ptr @num, align 4
  br label %if.end14

if.end14:
  %in.addr.0 = phi i32 [ %or, %if.then10 ], [ %and, %if.else12 ]
  tail call void @unknown_side_effect()
  %6 = load i32, ptr @num, align 4
  %add15 = add nsw i32 %6, 1
  store i32 %add15, ptr @num, align 4
  ret i32 %in.addr.0
}

declare void @unknown_side_effect()

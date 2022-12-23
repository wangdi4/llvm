; RUN: opt %s -passes=lowerswitch -S | FileCheck %s

; Check that the new dead blocks introduced by LowerSwitch pass (they were alive before the pass), are deleted.

; CHECK-LABEL: define i32 @f
; CHECK-NOT: out_of_switch_range
; CHECK-NOT: successor_of_unreachable

define i32 @f(i32 %c) {
chk65:
  %cmp = icmp sgt i32 %c, 65
  br i1 %cmp, label %return, label %chk0

chk0:                                             ; preds = %chk65
  %cmp1 = icmp slt i32 %c, 0
  br i1 %cmp, label %return, label %bb_if

bb_if:                                            ; preds = %chk0
  %ashr.val = ashr exact i32 %c, 2
  %cmp2 = icmp sgt i32 %ashr.val, 14
  br i1 %cmp2, label %bb_switch, label %return

bb_switch:                                        ; preds = %bb_if
; %ashr.val must be 15 or 16 here (analyzed by LazyValueInfo)
  switch i32 %ashr.val, label %out_of_switch_range [
    i32 15, label %return
    i32 16, label %another_ret
  ]

; unreachable default case
out_of_switch_range:                              ; preds = %bb_switch
  br label %successor_of_unreachable

; unreachable successors
successor_of_unreachable:                         ; preds = %out_of_switch_range
  br label %return

another_ret:                                      ; preds = %bb_switch
  br label %return

return:                                           ; preds = %another_ret, %successor_of_unreachable, %bb_switch, %bb_if, %chk0, %chk65
  %retval = phi i32 [ -1, %chk0 ], [ -1, %chk65 ], [ -1, %bb_if ], [ 42, %bb_switch ], [ 42, %another_ret ], [ -42, %successor_of_unreachable ]
  ret i32 %retval
}

; CHECK-LABEL: define i32 @g
; CHECK: bb_switch:
; CHECK-NEXT: br label %return
; CHECK-NOT: out_of_switch_range
define i32 @g(i32 %c) {
bb_switch:
  switch i32 %c, label %out_of_switch_range [
    i32 16, label %return
  ]

out_of_switch_range:                              ; preds = %bb_switch
  unreachable

return:                                           ; preds = %bb_switch
  ret i32 %c
}

; CHECK-LABEL: define i32 @h
; CHECK-NOT: out_of_switch_range
; CHECK-NOT: successor_of_unreachable
define i32 @h(i32 %c) {
chk65:
  %cmp = icmp sgt i32 %c, 65
  br i1 %cmp, label %return, label %chk0

chk0:                                             ; preds = %chk65
  %cmp1 = icmp slt i32 %c, 0
  br i1 %cmp, label %return, label %bb_if

bb_if:                                            ; preds = %chk0
  %ashr.val = ashr exact i32 %c, 2
  %cmp2 = icmp sgt i32 %ashr.val, 14
  br i1 %cmp2, label %bb_switch, label %return

bb_switch:                                        ; preds = %bb_if
; %ashr.val must be 15 or 16 here (analyzed by LazyValueInfo)
  switch i32 %ashr.val, label %out_of_switch_range [
    i32 15, label %return
    i32 16, label %another_ret
  ]

; unreachable default case
out_of_switch_range:                              ; preds = %bb_switch, %successor_of_unreachable
  br label %successor_of_unreachable

; unreachable successors
successor_of_unreachable:                         ; preds = %out_of_switch_range
  br label %out_of_switch_range

another_ret:                                      ; preds = %bb_switch
  br label %return

return:                                           ; preds = %another_ret, %bb_switch, %bb_if, %chk0, %chk65
  %retval = phi i32 [ -1, %chk0 ], [ -1, %chk65 ], [ -1, %bb_if ], [ 42, %bb_switch ], [ 42, %another_ret ]
  ret i32 %retval
}

; RUN: opt %s -lowerswitch -S | FileCheck %s
; RUN: opt %s -passes=lowerswitch -S | FileCheck %s

; CHECK-NOT: unreachable_with_succ
; CHECK-NOT: successor_of_unreachable

define i32 @f(i32 %c) {
chk65:
  %cmp = icmp sgt i32 %c, 65
  br i1 %cmp, label %return, label %chk0

chk0:
  %cmp1 = icmp slt i32 %c, 0
  br i1 %cmp, label %return, label %bb_if

bb_if:
  %ashr.val = ashr exact i32 %c, 2
  %cmp2 = icmp sgt i32 %ashr.val, 15
  br i1 %cmp2, label %bb_switch, label %return

bb_switch:
; %ashr.val must be 16 here (analyzed by LazyValueInfo)
  switch i32 %ashr.val, label %unreachable_with_succ [
    i32 16, label %return
  ]

; unreachable default case
unreachable_with_succ:
  br label %successor_of_unreachable

; unreachable successors
successor_of_unreachable:
  br label %return

return:
  %retval = phi i32 [-1, %chk0], [-1, %chk65], [-1, %bb_if], [42, %bb_switch], [-42, %successor_of_unreachable]
  ret i32 %retval
}

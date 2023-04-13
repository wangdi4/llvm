; RUN: opt -passes="simplifycfg" %s -S | FileCheck %s

; Verify that the out of range switch cases 4 and 8 are eliminated and switch is simplified into a select.

; CHECK: %sel = select i1 %cmp, i32 12, i32 %i
; CHECK-NOT: switch
; CHECK: %cond = icmp eq i32 %sel, 12
; CHECK: %[[SEL:.*]] = select i1 %cond, i32 3, i32 4 ;INTEL
; CHECK: ret i32 %[[SEL:.*]]                         ;INTEL

define i32 @test(i32 %i) {
entry:
  %cmp = icmp slt i32 %i, 12
  %sel = select i1 %cmp, i32 12, i32 %i
  switch i32 %sel, label %default [
    i32 4, label %bb0
    i32 8, label %bb1
    i32 12, label %bb2
  ]

bb0:
  ret i32 1

bb1:
  ret i32 2

bb2:
  ret i32 3

default:
  ret i32 4
}

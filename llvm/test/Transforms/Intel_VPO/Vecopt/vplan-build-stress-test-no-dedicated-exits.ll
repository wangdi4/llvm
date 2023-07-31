; RUN: opt -S -passes="vplan-vec" -vpo-vplan-build-stress-test < %s | FileCheck %s
; Test that we do not cause an assertion fail when stress testing VPlan builds
; for loop without dedicated exits.
; CHECK-LABEL: define void @test_multiple_exits_from_single_block(
; CHECK: ret
define void @test_multiple_exits_from_single_block(i8 %a, ptr %b.ptr) {
entry:
  switch i8 %a, label %loop [
    i8 0, label %exit.a
    i8 1, label %exit.b
  ]

loop:
  %b = load volatile i8, ptr %b.ptr
  switch i8 %b, label %loop [
    i8 0, label %exit.a
    i8 1, label %exit.b
    i8 2, label %loop
    i8 3, label %exit.a
    i8 4, label %loop
    i8 5, label %exit.a
    i8 6, label %loop
  ]

exit.a:
  ret void
exit.b:
  ret void
}

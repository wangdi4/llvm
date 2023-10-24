; Check that splitting of critical edges from switch instruction doesn't affect branch annotation.
; RUN: llc -cgp-split-switch-critical-edge=true < %s -print-after=finalize-isel -o /dev/null 2>&1 | FileCheck %s -check-prefix=SPLIT-SWITCH-CE


; Hexagon runs passes that renumber the basic blocks, causing this test
; to fail.
; XFAIL: target=hexagon-{{.*}}

declare void @foo()

; Make sure we have the correct weight attached to each successor.
define i32 @test2(i32 %x) nounwind uwtable readnone ssp {
; CHECK-LABEL: Machine code for function test2:
entry:
  %conv = sext i32 %x to i64
  switch i64 %conv, label %return [
    i64 0, label %sw.bb
    i64 1, label %sw.bb
    i64 4, label %sw.bb
    i64 5, label %sw.bb1
    i64 15, label %sw.bb
  ], !prof !0
; New BB is created on a critical edge, BB numbering is changed.
; SPLIT-SWITCH-CE: bb.0.entry:
; SPLIT-SWITCH-CE: successors: %bb.1(0x75f8ebf2), %bb.5(0x0a07140e)
; SPLIT-SWITCH-CE: bb.5.entry:
; SPLIT-SWITCH-CE: successors: %bb.2(0x60606068), %bb.6(0x1f9f9f98)
; SPLIT-SWITCH-CE: bb.6.entry:
; SPLIT-SWITCH-CE: successors: %bb.1(0x3cf3cf4b), %bb.7(0x430c30b5)
; SPLIT-SWITCH-CE: bb.7.entry:
; SPLIT-SWITCH-CE: successors: %bb.1(0x2e8ba2d7), %bb.3(0x51745d29)

sw.bb:
; this call will prevent simplifyCFG from optimizing the block away in ARM/AArch64.
  tail call void @foo()
  br label %return

sw.bb1:
  br label %return

return:
  %retval.0 = phi i32 [ 5, %sw.bb1 ], [ 1, %sw.bb ], [ 0, %entry ]
  ret i32 %retval.0
}

!0 = !{!"branch_weights", i32 7, i32 6, i32 4, i32 4, i32 64, i21 1000}


declare void @g(i32)
define void @left_leaning_weight_balanced_tree(i32 %x) {
entry:
  switch i32 %x, label %return [
    i32 0,  label %bb0
    i32 100, label %bb1
    i32 200, label %bb2
    i32 300, label %bb3
    i32 400, label %bb4
    i32 500, label %bb5
  ], !prof !1
bb0: tail call void @g(i32 0) br label %return
bb1: tail call void @g(i32 1) br label %return
bb2: tail call void @g(i32 2) br label %return
bb3: tail call void @g(i32 3) br label %return
bb4: tail call void @g(i32 4) br label %return
bb5: tail call void @g(i32 5) br label %return
return: ret void

; Check that we set branch weights on the pivot cmp instruction correctly.
; Cases {0,100,200,300} go on the left with weight 13; cases {400,500} go on the
; right with weight 20.
;
; CHECK-LABEL: Machine code for function left_leaning_weight_balanced_tree:
; CHECK: bb.0.entry:
; CHECK-NOT: Successors
; CHECK: successors: %bb.8(0x32d2d2d3), %bb.9(0x4d2d2d2d)
}

!1 = !{!"branch_weights",
  ; Default:
  i32 1,
  ; Case 0, 100, 200:
  i32 10, i32 1, i32 1,
  ; Case 300, 400, 500:
  i32 1, i32 10, i32 10}

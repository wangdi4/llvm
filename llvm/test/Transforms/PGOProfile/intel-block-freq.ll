; REQUIRES: asserts
; RUN: opt -pgo-instr-gen -debug-only=cfgmst -disable-output -S < %s 2>&1 | FileCheck %s

; Test that block frequencies are updated correctly
; by SplitIndirectCriticalEdges. Check that implicitly by dumping MST edges
; weights assigned during PGO instrumentation. During PGO instrumentation
; the critical edges from indirectbr are split, then MST is created and
; each edge is scaled by src BB frequency.

define i32 @f_blockfreq(i32 %c, i8* %addr) {
entry:
  indirectbr i8* %addr, [ label %sw.entry, label %sw.bb ], !prof !0

sw.entry:
  switch i32 %c, label %sw.epilog [
  i32 0, label %sw.bb
  i32 1, label %sw.bb
  ], !prof !1

sw.bb:
  %retval.0 = phi i32 [ 0, %entry ], [ 1, %sw.entry ], [ 1, %sw.entry ]
  br label %sw.epilog

sw.epilog:
  %retval.1 = phi i32 [ %retval.0, %sw.bb ], [ 2, %sw.entry ]
  ret i32 %retval.1
}

!0 = !{!"branch_weights", i32 15, i32 5}
!1 = !{!"branch_weights", i32 5, i32 5, i32 5}

; CHECK: Build Edge on f_blockfreq
; CHECK:   Edge: from fake node to entry w = 10
; CHECK:   Edge: from entry to sw.entry  w=7
; CHECK:   Edge: from entry to sw.bb  w=2
; CHECK:   Edge: from sw.entry to sw.epilog  w=2666
; CHECK:   Edge: from sw.entry to .split  w=5333
; CHECK:   Edge: from sw.entry to .split  w=5333
; CHECK:   Edge: from sw.bb to .split  w=0
; CHECK:   Edge: from .split to sw.epilog  w=8
; CHECK:   Edge: from sw.epilog to fake exit w = 10

; This test is to verify that the PGO metadata is updated when
; the simplifycfg pass eliminates redundant entries from switch tables.

; RUN: opt -S -simplifycfg %s | FileCheck %s
; RUN: opt -S -passes=simplify-cfg %s | FileCheck %s

declare void @func2()
declare void @func3()

define i32 @test2(i32 %i) !prof !0 {
entry:
  switch i32 %i, label %sw.default [
    i32 1, label %sw.bb     ; This case will be eliminated
    i32 2, label %sw.bb.1
    i32 3, label %sw.bb.2
    i32 4, label %sw.bb.3   ; This case will be eliminated
  ], !prof !1

sw.bb:
  br i1 false, label %if.true, label %sw.epilog

sw.bb.1:
  call void @func2();
  br label %sw.epilog

sw.bb.2:
  call void @func3();
  br label %sw.epilog

sw.bb.3:
    br label %sw.epilog

sw.default:
  br label %sw.epilog

if.true:
  unreachable

sw.epilog:
; Note: cases are only eliminated if the PHI result is same as the result
; produced by the default case.
  %a.0 = phi i32 [ 8, %sw.default ], [ 8, %sw.bb.3 ], [ 100, %sw.bb.2 ], [ 7, %sw.bb.1 ], [ 8, %sw.bb ]
  ret i32 %a.0
}

!0 = !{!"function_entry_count", i64 1000}
!1 = !{!"branch_weights", i32 800, i32 25, i32 50, i32 105, i32 20}

; The default value (first count) should be incremented based on the counts
; of the eliminated cases. Also, the rewriting of the switch instruction
; results in the cases being listed in a different order, the metadata
; should be updated appropriately.
; CHECK-LABEL: define i32 @test2
; CHECK:  switch i32 %i, label %sw.epilog [
; CHECK:    i32 3, label %sw.bb.2
; CHECK:    i32 2, label %sw.bb.1
; CHECK:  ], !prof !1
; CHECK: !1 = !{!"branch_weights", i32 845, i32 105, i32 50}

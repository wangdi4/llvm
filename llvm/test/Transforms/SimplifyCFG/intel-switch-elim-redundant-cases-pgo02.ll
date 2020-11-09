; This test is to verify that the PGO metadata is updated when
; the simplifycfg pass eliminates redundant entries from switch tables.
;
; In this case, applying the execution counts from the eliminated cases
; to the default case would result in integer overflow of a 32-bit value,
; so this case verifies the weights are normalized to 32-bits.

; RUN: opt -S -simplifycfg %s | FileCheck %s
; RUN: opt -S -passes=simplify-cfg %s | FileCheck %s

declare void @func2()
declare void @func3()

define i32 @test2(i32 %i) !prof !0 {
entry:
  switch i32 %i, label %sw.default [
    i32 1, label %sw.bb     ; This case will be eliminated
    i32 2, label %sw.bb.1
    i32 3, label %sw.bb.2   ; This case will be eliminated
    i32 4, label %sw.bb.3
  ], !prof !1

sw.bb:
  br i1 false, label %if.true, label %sw.epilog

sw.bb.1:
  call void @func2();
  br label %sw.epilog

sw.bb.2:
    br label %sw.epilog

sw.bb.3:
  call void @func3();
  br label %sw.epilog

sw.default:
  br label %sw.epilog

if.true:
  unreachable

sw.epilog:
; Note: cases are only eliminated if the PHI result is same as the result
; produced by the default case.
  %a.0 = phi i32 [ 8, %sw.default ], [ 9, %sw.bb.3 ], [ 8, %sw.bb.2 ], [ 7, %sw.bb.1 ], [ 8, %sw.bb ]
  ret i32 %a.0
}

!0 = !{!"function_entry_count", i64 4295032984}
!1 = !{!"branch_weights", i32 4294901760, i32 65535, i32 50, i32 65535, i32 104}

; The default value (first count) should be incremented based on the counts
; of the eliminated cases. Also, the rewriting of the switch instruction
; results in the cases being listed in a different order, the metadata
; should be updated appropriately. (note, metadata values are printed as
; signed in IR, but will be treated as unsigned)
; CHECK-LABEL: define i32 @test2
; CHECK:  switch i32 %i, label %sw.epilog [
; CHECK:    i32 4, label %sw.bb.3
; CHECK:    i32 2, label %sw.bb.1
; CHECK:  ], !prof !1
; CHECK: !1 = !{!"branch_weights", i32 -2147450881, i32 52, i32 25}

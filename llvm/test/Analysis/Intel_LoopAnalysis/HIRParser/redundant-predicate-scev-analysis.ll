; RUN: opt < %s -hir-create-function-level-region -passes="hir-ssa-deconstruction,print<hir-framework>" 2>&1 | FileCheck %s

; Verify that %cmp1 is deduced as 'false' using ScalarEvolution's isImplied() logic.
; Before change, the formed HIR looked like this-

; if (trunc.i32.i1(%t) == 0)
; {
;    if (trunc.i32.i1(%t) + -1 == 0)
;    {
;       ret 3;
;    }
;    else
;    {
;       ret 4;
;    }
; }
; else
; {
;    ret 2;
; }


; CHECK: if (trunc.i32.i1(%t) == 0)
; CHECK: {

; CHECK-NOT: if
; CHECK-NOT: ret 3

; CHECK:    ret 4;
; CHECK: }
; CHECK: else
; CHECK: {
; CHECK:    ret 2;
; CHECK: }

define i32 @foo(i32 %t) {
entry:
  %t.and = and i32 %t, 1
  %cmp = icmp eq i32 %t.and, 0
  br i1 %cmp, label %bb1, label %bb2

bb1:
  %t.xor = xor i32 %t, -1
  %t.xor.and = and i32 %t.xor, 1
  %cmp1 = icmp eq i32 %t.xor.and, 0
  br i1 %cmp1, label %bb3, label %bb4

bb2:
  ret i32 2

bb3:
  ret i32 3

bb4:
  ret i32 4
}
